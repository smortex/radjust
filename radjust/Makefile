OS!=		uname -s

PROG=		radjust
WITHOUT_MAN=
CFLAGS+=	-I../libadjust
.if ${OS} == "FreeBSD"
LDFLAGS+=	-lmd
.endif
LDADD+=		../libadjust/libadjust.a
.if ${OS} == "Linux"
CFLAGS+=	-D_GNU_SOURCE
LDADD+=		-lcrypto
.endif

.include <bsd.prog.mk>

coin:
	scp Makefile radjust.c schnocdlu:radjust/radjust/
	ssh schnocdlu cd radjust/radjust '&&' fmake '&&' fmake install
