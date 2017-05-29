OS!=		uname -s

PROG=		server
WITHOUT_MAN=
CFLAGS+=	-I../libadjust
.if ${OS} == "Linux"
LDFLAGS+=	-lcrypto
.elif ${OS} == "FreeBSD"
LDFLAGS+=	-lmd
.endif
LDADD+=		../libadjust/libadjust.a

.include <bsd.prog.mk>
