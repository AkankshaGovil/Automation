AUTOMAKE_OPTIONS = gnits 

include_HEADERS = avl.h avlt.h avltr.h rb.h

lib_LIBRARIES = libavl.a
libavl_a_SOURCES = avl.c avlt.c avltr.c rb.c

info_TEXINFOS = avl.texinfo
noinst_DATA = avl.html avl.text

avl.text : avl.texinfo
	$(MAKEINFO) $(srcdir)/avl.texinfo -o avl.text --no-headers

avl.html: avl.texinfo
	-texi2html -monolithic $(srcdir)/avl.texinfo

EXTRA_PROGRAMS = avl-test avlt-test avltr-test thread-test rb-test
DISTCLEANFILES = $(EXTRA_PROGRAMS) $(BUILT_SOURCES)

BUILT_SOURCES = avl-test.c avlt-test.c avltr-test.c rb-test.c

TESTS = avl-test avlt-test avltr-test thread-test rb-test
avl-test.c avlt-test.c avltr-test.c rb-test.c:
	rm -f $@
	echo '#define SELF_TEST 1' > $@
	echo '#include "'`echo $@ | sed s/-test//`'"' >> $@

thread_test_LDADD = libavl.a
