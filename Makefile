SUBDIR=	libadjust client server

CFLAGS+=	-I../libadjust

test:
	bundle exec cucumber

.include <bsd.subdir.mk>
