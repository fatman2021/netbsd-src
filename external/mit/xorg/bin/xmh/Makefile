#	$NetBSD: Makefile,v 1.7 2015/07/19 23:15:06 mrg Exp $

.include <bsd.own.mk>

PROG=	xmh
SRCS=	bbox.c command.c compfuncs.c folder.c init.c main.c \
	menu.c mlist.c msg.c pick.c popup.c screen.c toc.c \
	tocfuncs.c tocutil.c tsource.c util.c viewfuncs.c \
	miscfuncs.c

CPPFLAGS+=	-I${X11SRCDIR.${PROG}}/../include
CPPFLAGS+=	-DHAS_VFORK -DXVENDORNAME=${XVENDORNAME} \
		-DXORG_RELEASE=${XORG_RELEASE}

APPDEFS=Xmh

LDADD+=	-lXaw -lXmu -lXt -lSM -lICE -lXpm -lXext -lX11
DPADD+=	${LIBXAW} ${LIBXMU} ${LIBXT} ${LIBSM} ${LIBICE} \
	${LIBXPM} ${LIBXEXT} ${LIBX11}

.PATH:	${X11SRCDIR.${PROG}} ${X11SRCDIR.${PROG}}/app-defaults
.PATH:	${X11SRCDIR.${PROG}}/man

CWARNFLAGS.clang+=	-Wno-format

.include <bsd.x11.mk>
.include <bsd.prog.mk>
