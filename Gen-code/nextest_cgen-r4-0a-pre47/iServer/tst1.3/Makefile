# Generated automatically from Makefile.in by configure.

INSTALL = ./install-sh -c
INSTALL_PROGRAM = ${INSTALL}

srcdir = .
prefix = /usr/local
execprefix = ${prefix}
includedir = ${prefix}/include
libdir = ${exec_prefix}/lib
mandir = ${prefix}/man

#### End of system configuration section. ####

include $(BASE)/global.mk

SHELL = /bin/sh

.SUFFIXES:
.SUFFIXES: .c .o

#CC = gcc
#CFLAGS = -DSUNOS -D_REENTRANT -DXML_BYTE_ORDER=21 -DNX_BYTE_ORDER=21 -fPIC -DSIP_SOLARIS -DANSI_PROTO -DSIP_BY_REFERENCE -DTHIRTY_TWO_BIT -DALLOW_ISERVER_H323 -D_POSIX_PTHREAD_SEMANTICS -ggdb -O3 -DNOTMRLOG -DNOTHRDLOG -DNOFDLOG -DNORESLOG - nextone
CFLAGS += -D_TST_PRIV_
AR = ar -cr
OBJECTS = tst_cleanup.o tst_delete.o tst_grow_node_free_list.o tst_init.o \
          tst_insert.o tst_search.o tst_match_set.o tst_walk.o
MANPAGES = tst_cleanup.8 tst_delete.8 tst_init.8 tst_insert.8 tst_search.8

all:	libtst.a
install lib: all
	cp libtst.a $(LIBTST)

installdirs:
	${SHELL} ${srcdir}/mkinstalldirs ${libdir} ${includedir} ${mandir}/man8

#install:	all installdirs  -- nextone
#	${INSTALL_PROGRAM} ${srcdir}/libtst.a ${libdir}/libtst.a
#	${INSTALL_PROGRAM} ${srcdir}/tst.h ${includedir}/tst.h
#	for i in ${MANPAGES}; do \
#	 ${INSTALL_PROGRAM} $$i ${mandir}/man8/$$i; \
#	done
libtst.a: ${OBJECTS}
	${AR} libtst.a ${OBJECTS}

#tst_cleanup.o:	tst_cleanup.c tst.h
#	${CC} -c ${CFLAGS} tst_cleanup.c

#tst_delete.o:	tst_delete.c tst.h
#	${CC} -c ${CFLAGS} tst_delete.c

#tst_grow_node_free_list.o:	tst_grow_node_free_list.c tst.h
#	${CC} -c ${CFLAGS} tst_grow_node_free_list.c

#tst_init.o:	tst_init.c tst.h
#	${CC} -c ${CFLAGS} tst_init.c

#tst_insert.o:	tst_insert.c tst.h
#	${CC} -c ${CFLAGS} tst_insert.c

#tst_search.o:	tst_search.c tst.h
#	${CC} -c ${CFLAGS} tst_search.c

#tst_match_set.o:	tst_match_set.c tst.h
#	${CC} -c ${CFLAGS} tst_match_set.c

clean:
	rm -f ${OBJECTS} libtst.a
