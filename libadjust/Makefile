OS!=	uname -s

LIB=	adjust
SRCS=	error.c \
	file.c \
	file_info.c \
	networking.c

.if ${OS} == "Linux"
CFLAGS+=	-D_GNU_SOURCE
.endif

.include <bsd.lib.mk>
