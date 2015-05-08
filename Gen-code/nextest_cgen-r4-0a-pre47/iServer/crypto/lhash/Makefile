#
# SSLeay/crypto/lhash/Makefile
#

DIR=	lhash
TOP=	../..
CC=	cc
INCLUDES=
CFLAG=-g
INSTALLTOP=/usr/local/ssl
MAKE=		make -f Makefile.ssl
MAKEDEPEND=	makedepend -f Makefile.ssl
MAKEFILE=	Makefile.ssl
AR=		ar r

CFLAGS= $(INCLUDES) $(CFLAG)

GENERAL=Makefile
TEST=
APPS=

LIB=$(TOP)/libcrypto.a
LIBSRC=lhash.c lh_stats.c
LIBOBJ=lhash.o lh_stats.o

SRC= $(LIBSRC)

EXHEADER= lhash.h
HEADER=	$(EXHEADER)

ALL=    $(GENERAL) $(SRC) $(HEADER)

top:
	(cd ../..; $(MAKE) DIRS=crypto SDIRS=$(DIR) sub_all)

all:	lib

lib:	$(LIBOBJ)
	$(AR) $(LIB) $(LIBOBJ)
	sh $(TOP)/util/ranlib.sh $(LIB)
	@touch lib

files:
	perl $(TOP)/util/files.pl Makefile.ssl >> $(TOP)/MINFO

links:
	/bin/rm -f Makefile
	$(TOP)/util/point.sh Makefile.ssl Makefile ;
	$(TOP)/util/mklink.sh ../../include $(EXHEADER)
	$(TOP)/util/mklink.sh ../../test $(TEST)
	$(TOP)/util/mklink.sh ../../apps $(APPS)

install:
	@for i in $(EXHEADER) ; \
	do  \
	(cp $$i $(INSTALLTOP)/include/$$i; \
	chmod 644 $(INSTALLTOP)/include/$$i ); \
	done;

tags:
	ctags $(SRC)

tests:

lint:
	lint -DLINT $(INCLUDES) $(SRC)>fluff

depend:
	$(MAKEDEPEND) $(INCLUDES) $(PROGS) $(LIBSRC)

dclean:
	perl -pe 'if (/^# DO NOT DELETE THIS LINE/) {print; exit(0);}' $(MAKEFILE) >Makefile.new
	mv -f Makefile.new $(MAKEFILE)

clean:
	/bin/rm -f *.o *.obj lib tags core .pure .nfs* *.old *.bak fluff

errors:

# DO NOT DELETE THIS LINE -- make depend depends on it.

lhash.o: /usr/include/stdio.h /usr/include/libio.h /usr/include/features.h
lhash.o: /usr/include/sys/cdefs.h /usr/include/gnu/stubs.h
lhash.o: /usr/include/_G_config.h /usr/include/gnu/types.h
lhash.o: /usr/lib/gcc-lib/i386-redhat-linux/2.7.2.3/include/stddef.h
lhash.o: /usr/lib/gcc-lib/i386-redhat-linux/2.7.2.3/include/stdarg.h
lhash.o: /usr/include/stdio_lim.h /usr/include/string.h /usr/include/stdlib.h
lhash.o: /usr/include/sys/types.h /usr/include/time.h /usr/include/endian.h
lhash.o: /usr/include/bytesex.h /usr/include/sys/select.h
lhash.o: /usr/include/selectbits.h /usr/include/alloca.h ../crypto.h
lhash.o: ../../include/stack.h lhash.h
lh_stats.o: /usr/include/stdio.h /usr/include/libio.h /usr/include/features.h
lh_stats.o: /usr/include/sys/cdefs.h /usr/include/gnu/stubs.h
lh_stats.o: /usr/include/_G_config.h /usr/include/gnu/types.h
lh_stats.o: /usr/lib/gcc-lib/i386-redhat-linux/2.7.2.3/include/stddef.h
lh_stats.o: /usr/lib/gcc-lib/i386-redhat-linux/2.7.2.3/include/stdarg.h
lh_stats.o: /usr/include/stdio_lim.h /usr/include/string.h
lh_stats.o: /usr/include/stdlib.h /usr/include/sys/types.h
lh_stats.o: /usr/include/time.h /usr/include/endian.h /usr/include/bytesex.h
lh_stats.o: /usr/include/sys/select.h /usr/include/selectbits.h
lh_stats.o: /usr/include/alloca.h ../cryptlib.h ../../include/e_os.h
lh_stats.o: /usr/include/unistd.h /usr/include/posix_opt.h
lh_stats.o: /usr/include/confname.h ../crypto.h ../../include/stack.h
lh_stats.o: ../../include/buffer.h ../../include/bio.h ../../include/err.h
lh_stats.o: /usr/include/errno.h /usr/include/errnos.h
lh_stats.o: /usr/include/linux/errno.h /usr/include/asm/errno.h lhash.h
