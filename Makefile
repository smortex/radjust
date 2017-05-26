SUBDIR=	libadjust client server

CFLAGS+=	-I../libadjust

test:
	bundle install --jobs=3 --retry=3
	bundle exec cucumber

.include <bsd.subdir.mk>
