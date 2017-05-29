OS!=		uname -s

PROG=		server
WITHOUT_MAN=
CFLAGS+=	-I../libadjust
.if ${OS} == "FreeBSD"
LDFLAGS+=	-lmd
.endif
LDADD+=		../libadjust/libadjust.a
.if ${OS} == "Linux"
LDADD+=		-lcrypto
.endif

.include <bsd.prog.mk>
