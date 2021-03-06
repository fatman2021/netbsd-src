//===-- tsan_sync.cc ------------------------------------------------------===//
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This file is a part of ThreadSanitizer (TSan), a race detector.
//
//===----------------------------------------------------------------------===//
#include "sanitizer_common/sanitizer_placement_new.h"
#include "tsan_sync.h"
#include "tsan_rtl.h"
#include "tsan_mman.h"

namespace __tsan {

void DDMutexInit(ThreadState *thr, uptr pc, SyncVar *s);

SyncVar::SyncVar()
    : mtx(MutexTypeSyncVar, StatMtxSyncVar) {
  Reset(0);
}

void SyncVar::Init(ThreadState *thr, uptr pc, uptr addr, u64 uid) {
  this->addr = addr;
  this->uid = uid;
  this->next = 0;

  creation_stack_id = 0;
  if (kCppMode)  // Go does not use them
    creation_stack_id = CurrentStackId(thr, pc);
  if (common_flags()->detect_deadlocks)
    DDMutexInit(thr, pc, this);
}

void SyncVar::Reset(ThreadState *thr) {
  uid = 0;
  creation_stack_id = 0;
  owner_tid = kInvalidTid;
  last_lock = 0;
  recursion = 0;
  is_rw = 0;
  is_recursive = 0;
  is_broken = 0;
  is_linker_init = 0;

  if (thr == 0) {
    CHECK_EQ(clock.size(), 0);
    CHECK_EQ(read_clock.size(), 0);
  } else {
    clock.Reset(&thr->clock_cache);
    read_clock.Reset(&thr->clock_cache);
  }
}

MetaMap::MetaMap() {
  atomic_store(&uid_gen_, 0, memory_order_relaxed);
}

void MetaMap::AllocBlock(ThreadState *thr, uptr pc, uptr p, uptr sz) {
  u32 idx = block_alloc_.Alloc(&thr->block_cache);
  MBlock *b = block_alloc_.Map(idx);
  b->siz = sz;
  b->tid = thr->tid;
  b->stk = CurrentStackId(thr, pc);
  u32 *meta = MemToMeta(p);
  DCHECK_EQ(*meta, 0);
  *meta = idx | kFlagBlock;
}

uptr MetaMap::FreeBlock(ThreadState *thr, uptr pc, uptr p) {
  MBlock* b = GetBlock(p);
  if (b == 0)
    return 0;
  uptr sz = RoundUpTo(b->siz, kMetaShadowCell);
  FreeRange(thr, pc, p, sz);
  return sz;
}

void MetaMap::FreeRange(ThreadState *thr, uptr pc, uptr p, uptr sz) {
  u32 *meta = MemToMeta(p);
  u32 *end = MemToMeta(p + sz);
  if (end == meta)
    end++;
  for (; meta < end; meta++) {
    u32 idx = *meta;
    *meta = 0;
    for (;;) {
      if (idx == 0)
        break;
      if (idx & kFlagBlock) {
        block_alloc_.Free(&thr->block_cache, idx & ~kFlagMask);
        break;
      } else if (idx & kFlagSync) {
        DCHECK(idx & kFlagSync);
        SyncVar *s = sync_alloc_.Map(idx & ~kFlagMask);
        u32 next = s->next;
        s->Reset(thr);
        sync_alloc_.Free(&thr->sync_cache, idx & ~kFlagMask);
        idx = next;
      } else {
        CHECK(0);
      }
    }
  }
}

MBlock* MetaMap::GetBlock(uptr p) {
  u32 *meta = MemToMeta(p);
  u32 idx = *meta;
  for (;;) {
    if (idx == 0)
      return 0;
    if (idx & kFlagBlock)
      return block_alloc_.Map(idx & ~kFlagMask);
    DCHECK(idx & kFlagSync);
    SyncVar * s = sync_alloc_.Map(idx & ~kFlagMask);
    idx = s->next;
  }
}

SyncVar* MetaMap::GetOrCreateAndLock(ThreadState *thr, uptr pc,
                              uptr addr, bool write_lock) {
  return GetAndLock(thr, pc, addr, write_lock, true);
}

SyncVar* MetaMap::GetIfExistsAndLock(uptr addr) {
  return GetAndLock(0, 0, addr, true, false);
}

SyncVar* MetaMap::GetAndLock(ThreadState *thr, uptr pc,
                             uptr addr, bool write_lock, bool create) {
  u32 *meta = MemToMeta(addr);
  u32 idx0 = *meta;
  u32 myidx = 0;
  SyncVar *mys = 0;
  for (;;) {
    u32 idx = idx0;
    for (;;) {
      if (idx == 0)
        break;
      if (idx & kFlagBlock)
        break;
      DCHECK(idx & kFlagSync);
      SyncVar * s = sync_alloc_.Map(idx & ~kFlagMask);
      if (s->addr == addr) {
        if (myidx != 0) {
          mys->Reset(thr);
          sync_alloc_.Free(&thr->sync_cache, myidx);
        }
        if (write_lock)
          s->mtx.Lock();
        else
          s->mtx.ReadLock();
        return s;
      }
      idx = s->next;
    }
    if (!create)
      return 0;
    if (*meta != idx0) {
      idx0 = *meta;
      continue;
    }

    if (myidx == 0) {
      const u64 uid = atomic_fetch_add(&uid_gen_, 1, memory_order_relaxed);
      myidx = sync_alloc_.Alloc(&thr->sync_cache);
      mys = sync_alloc_.Map(myidx);
      mys->Init(thr, pc, addr, uid);
    }
    mys->next = idx0;
    if (atomic_compare_exchange_strong((atomic_uint32_t*)meta, &idx0,
        myidx | kFlagSync, memory_order_release)) {
      if (write_lock)
        mys->mtx.Lock();
      else
        mys->mtx.ReadLock();
      return mys;
    }
  }
}

void MetaMap::MoveMemory(uptr src, uptr dst, uptr sz) {
  // src and dst can overlap,
  // there are no concurrent accesses to the regions (e.g. stop-the-world).
  CHECK_NE(src, dst);
  CHECK_NE(sz, 0);
  uptr diff = dst - src;
  u32 *src_meta = MemToMeta(src);
  u32 *dst_meta = MemToMeta(dst);
  u32 *src_meta_end = MemToMeta(src + sz);
  uptr inc = 1;
  if (dst > src) {
    src_meta = MemToMeta(src + sz) - 1;
    dst_meta = MemToMeta(dst + sz) - 1;
    src_meta_end = MemToMeta(src) - 1;
    inc = -1;
  }
  for (; src_meta != src_meta_end; src_meta += inc, dst_meta += inc) {
    CHECK_EQ(*dst_meta, 0);
    u32 idx = *src_meta;
    *src_meta = 0;
    *dst_meta = idx;
    // Patch the addresses in sync objects.
    while (idx != 0) {
      if (idx & kFlagBlock)
        break;
      CHECK(idx & kFlagSync);
      SyncVar *s = sync_alloc_.Map(idx & ~kFlagMask);
      s->addr += diff;
      idx = s->next;
    }
  }
}

void MetaMap::OnThreadIdle(ThreadState *thr) {
  block_alloc_.FlushCache(&thr->block_cache);
  sync_alloc_.FlushCache(&thr->sync_cache);
}

}  // namespace __tsan
