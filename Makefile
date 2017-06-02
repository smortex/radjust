SUBDIR=	libadjust client server radjust fakessh

CFLAGS+=	-I../libadjust

style:
	find . -name "*.[ch]" -exec perl -pi -e 's/[ \t]+$$//' {} \;
	find . -name "*.[ch]" -exec astyle --style=linux \
	    --indent=force-tab-x \
	    --lineend=linux {} \;

test:
	bundle install --jobs=3 --retry=3
	bundle exec cucumber

.include <bsd.subdir.mk>
