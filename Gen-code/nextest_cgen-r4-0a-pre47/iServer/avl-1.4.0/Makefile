# Generated automatically from Makefile.in by configure.
# Makefile.in generated automatically by automake 1.4 from Makefile.am

# Copyright (C) 1994, 1995-8, 1999 Free Software Foundation, Inc.
# This Makefile.in is free software; the Free Software Foundation
# gives unlimited permission to copy and/or distribute it,
# with or without modifications, as long as this notice is preserved.

# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY, to the extent permitted by law; without
# even the implied warranty of MERCHANTABILITY or FITNESS FOR A
# PARTICULAR PURPOSE.

include $(BASE)/global.mk

SHELL = /bin/sh

CC = gcc
RANLIB = ranlib
VERSION = 1.4.0

include_HEADERS = avl.h avlt.h avltr.h rb.h

lib_LIBRARIES = libavl.a
libavl_a_SOURCES = avl.c avlt.c avltr.c rb.c

EXTRA_PROGRAMS = avl-test avlt-test avltr-test thread-test rb-test
DISTCLEANFILES = $(EXTRA_PROGRAMS) $(BUILT_SOURCES)

BUILT_SOURCES = avl-test.c avlt-test.c avltr-test.c rb-test.c

TESTS = avl-test avlt-test avltr-test thread-test rb-test

LIBRARIES =  $(lib_LIBRARIES)


DEFS =  -DNDEBUG -DPACKAGE=\"avl\" -DVERSION=\"1.4.0\"  -I. -I$(srcdir) 
CPPFLAGS = 
LDFLAGS = 
LIBS = 
libavl_a_LIBADD = 
libavl_a_OBJECTS =  avl.o avlt.o avltr.o rb.o
AR = ar
avl_test_SOURCES = avl-test.c
avl_test_OBJECTS =  avl-test.o
avl_test_LDADD = $(LDADD)
avl_test_DEPENDENCIES = 
avl_test_LDFLAGS = 
avlt_test_SOURCES = avlt-test.c
avlt_test_OBJECTS =  avlt-test.o
avlt_test_LDADD = $(LDADD)
avlt_test_DEPENDENCIES = 
avlt_test_LDFLAGS = 
avltr_test_SOURCES = avltr-test.c
avltr_test_OBJECTS =  avltr-test.o
avltr_test_LDADD = $(LDADD)
avltr_test_DEPENDENCIES = 
avltr_test_LDFLAGS = 
thread_test_SOURCES = thread-test.c
thread_test_OBJECTS =  thread-test.o
thread_test_DEPENDENCIES =  libavl.a
thread_test_LDFLAGS = 
rb_test_SOURCES = rb-test.c
rb_test_OBJECTS =  rb-test.o
rb_test_LDADD = $(LDADD)
rb_test_DEPENDENCIES = 
rb_test_LDFLAGS = 
#CFLAGS = -g -O2
COMPILE = $(CC) $(DEFS) $(INCLUDES) $(AM_CPPFLAGS) $(CPPFLAGS) $(AM_CFLAGS) $(CFLAGS)
CCLD = $(CC)
LINK = $(CCLD) $(AM_CFLAGS) $(CFLAGS) $(LDFLAGS) -o $@
TEXI2DVI = texi2dvi
INFO_DEPS = avl.info
DVIS = avl.dvi
TEXINFOS = avl.texinfo
DATA =  $(noinst_DATA)

HEADERS =  $(include_HEADERS)

DIST_COMMON =  README AUTHORS COPYING ChangeLog INSTALL Makefile.am \
Makefile.in NEWS THANKS TODO aclocal.m4 configure configure.in \
install-sh missing mkinstalldirs texinfo.tex


DISTFILES = $(DIST_COMMON) $(SOURCES) $(HEADERS) $(TEXINFOS) $(EXTRA_DIST)

TAR = tar
GZIP_ENV = --best
SOURCES = $(libavl_a_SOURCES) avl-test.c avlt-test.c avltr-test.c thread-test.c rb-test.c
OBJECTS = $(libavl_a_OBJECTS) avl-test.o avlt-test.o avltr-test.o thread-test.o rb-test.o

lib install: $(LIBRARIES)
	cp libavl.a $(LIBAVL)

.c.o:
	$(COMPILE) -c $<

.s.o:
	$(COMPILE) -c $<

.S.o:
	$(COMPILE) -c $<

mostlyclean-compile:
	-rm -f *.o core *.core

clean-compile:

distclean-compile:
	-rm -f *.tab.c

maintainer-clean-compile:

libavl.a: $(libavl_a_OBJECTS) $(libavl_a_DEPENDENCIES)
	-rm -f libavl.a
	$(AR) cru libavl.a $(libavl_a_OBJECTS) $(libavl_a_LIBADD)
	$(RANLIB) libavl.a

avl-test: $(avl_test_OBJECTS) $(avl_test_DEPENDENCIES)
	@rm -f avl-test
	$(LINK) $(avl_test_LDFLAGS) $(avl_test_OBJECTS) $(avl_test_LDADD) $(LIBS)

avlt-test: $(avlt_test_OBJECTS) $(avlt_test_DEPENDENCIES)
	@rm -f avlt-test
	$(LINK) $(avlt_test_LDFLAGS) $(avlt_test_OBJECTS) $(avlt_test_LDADD) $(LIBS)

avltr-test: $(avltr_test_OBJECTS) $(avltr_test_DEPENDENCIES)
	@rm -f avltr-test
	$(LINK) $(avltr_test_LDFLAGS) $(avltr_test_OBJECTS) $(avltr_test_LDADD) $(LIBS)

thread-test: $(thread_test_OBJECTS) $(thread_test_DEPENDENCIES)
	@rm -f thread-test
	$(LINK) $(thread_test_LDFLAGS) $(thread_test_OBJECTS) $(thread_test_LDADD) $(LIBS)

rb-test: $(rb_test_OBJECTS) $(rb_test_DEPENDENCIES)
	@rm -f rb-test
	$(LINK) $(rb_test_LDFLAGS) $(rb_test_OBJECTS) $(rb_test_LDADD) $(LIBS)

clean:
	rm -f $(LIBAVL)
	rm -f *.o libavl.a

avl-test.c avlt-test.c avltr-test.c rb-test.c:
	rm -f $@
	echo '#define SELF_TEST 1' > $@
	echo '#include "'`echo $@ | sed s/-test//`'"' >> $@
