## Makefile.am -- Process this file with automake to produce Makefile.in
if MAKE_TESTS
SUBDIRS = src tests
else
SUBDIRS = src
endif

EXTRA_DIST=config include

ACLOCAL_AMFLAGS = -I config

dist-hook:
	rm -rf `find $(distdir)/config -name CVS`
	rm -rf `find $(distdir)/include -name CVS`