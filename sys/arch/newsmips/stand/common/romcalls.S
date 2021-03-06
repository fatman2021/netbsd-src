/*	$NetBSD: romcalls.S,v 1.5 2003/04/18 18:52:27 tsutsui Exp $	*/

/*-
 * Copyright (C) 1999 Tsubai Masanari.  All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. The name of the author may not be used to endorse or promote products
 *    derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 * OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 * IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
 * NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include <mips/asm.h>
#include <mips/cpuregs.h>
#include <machine/romcall.h>

	.text
	.align	2
	.set	noreorder

LEAF_NOPROFILE(rom_halt)
	addu	sp, sp, -16
	li	a0, 8		# RB_HALT
	sw	a0, 0(sp)
	li	a0, SYS_reboot
	move	a1, sp
	syscall
	nop
END(rom_halt)

LEAF_NOPROFILE(rom_open)
	addu	sp, sp, -32
	sw	ra, 28(sp)

	sw	a0, 16(sp)
	sw	a1, 20(sp)

	li	a0, SYS_open
	addu	a1, sp, 16
	syscall
	nop

	lw	ra, 28(sp)
	addu	sp, sp, 32
	j	ra
	nop
END(rom_open)

LEAF_NOPROFILE(rom_close)
	addu	sp, sp, -32
	sw	ra, 28(sp)

	sw	a0, 16(sp)

	li	a0, SYS_close
	addu	a1, sp, 16
	syscall
	nop

	lw	ra, 28(sp)
	addu	sp, sp, 32
	j	ra
	nop
END(rom_close)

LEAF_NOPROFILE(rom_read)
	addu	sp, sp, -32
	sw	ra, 28(sp)

	sw	a0, 16(sp)
	sw	a1, 20(sp)
	sw	a2, 24(sp)

	li	a0, SYS_read
	addu	a1, sp, 16
	syscall
	nop

	lw	ra, 28(sp)
	addu	sp, sp, 32
	j	ra
	nop
END(rom_read)

LEAF_NOPROFILE(rom_write)
	addu	sp, sp, -32
	sw	ra, 28(sp)

	sw	a0, 16(sp)
	sw	a1, 20(sp)
	sw	a2, 24(sp)

	li	a0, SYS_write
	addu	a1, sp, 16
	syscall
	nop

	lw	ra, 28(sp)
	addu	sp, sp, 32
	j	ra
	nop
END(rom_write)

LEAF_NOPROFILE(rom_lseek)
	addu	sp, sp, -32
	sw	ra, 28(sp)

	sw	a0, 16(sp)
	sw	a1, 20(sp)
	sw	a2, 24(sp)

	li	a0, SYS_lseek
	addu	a1, sp, 16
	syscall
	nop

	lw	ra, 28(sp)
	addu	sp, sp, 32
	j	ra
	nop
END(rom_lseek)

LEAF_NOPROFILE(rom_ioctl)
	addu	sp, sp, -32
	sw	ra, 28(sp)

	sw	a0, 16(sp)
	sw	a1, 20(sp)
	sw	a2, 24(sp)

	li	a0, SYS_ioctl
	addu	a1, sp, 16
	syscall
	nop

	lw	ra, 28(sp)
	addu	sp, sp, 32
	j	ra
	nop
END(rom_ioctl)
