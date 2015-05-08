#
# SSLeay/crypto/rand/Makefile
#

DIR=	rand
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
TEST= randtest.c
APPS=

LIB=$(TOP)/libcrypto.a
LIBSRC=md_rand.c randfile.c rand_lib.c
LIBOBJ=md_rand.o randfile.o rand_lib.o

SRC= $(LIBSRC)

EXHEADER= rand.h
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

md_rand.o: /usr/include/stdio.h /usr/include/libio.h /usr/include/features.h
md_rand.o: /usr/include/sys/cdefs.h /usr/include/gnu/stubs.h
md_rand.o: /usr/include/_G_config.h /usr/include/gnu/types.h
md_rand.o: /usr/lib/gcc-lib/i386-redhat-linux/2.7.2.3/include/stddef.h
md_rand.o: /usr/lib/gcc-lib/i386-redhat-linux/2.7.2.3/include/stdarg.h
md_rand.o: /usr/include/stdio_lim.h ../cryptlib.h /usr/include/stdlib.h
md_rand.o: /usr/include/sys/types.h /usr/include/time.h /usr/include/endian.h
md_rand.o: /usr/include/bytesex.h /usr/include/sys/select.h
md_rand.o: /usr/include/selectbits.h /usr/include/alloca.h
md_rand.o: /usr/include/string.h ../../include/e_os.h /usr/include/unistd.h
md_rand.o: /usr/include/posix_opt.h /usr/include/confname.h ../crypto.h
md_rand.o: ../../include/stack.h ../../include/buffer.h ../../include/bio.h
md_rand.o: ../../include/err.h /usr/include/errno.h /usr/include/errnos.h
md_rand.o: /usr/include/linux/errno.h /usr/include/asm/errno.h
md_rand.o: ../../include/md5.h rand.h
randfile.o: /usr/include/stdio.h /usr/include/libio.h /usr/include/features.h
randfile.o: /usr/include/sys/cdefs.h /usr/include/gnu/stubs.h
randfile.o: /usr/include/_G_config.h /usr/include/gnu/types.h
randfile.o: /usr/lib/gcc-lib/i386-redhat-linux/2.7.2.3/include/stddef.h
randfile.o: /usr/lib/gcc-lib/i386-redhat-linux/2.7.2.3/include/stdarg.h
randfile.o: /usr/include/stdio_lim.h ../cryptlib.h /usr/include/stdlib.h
randfile.o: /usr/include/sys/types.h /usr/include/time.h
randfile.o: /usr/include/endian.h /usr/include/bytesex.h
randfile.o: /usr/include/sys/select.h /usr/include/selectbits.h
randfile.o: /usr/include/alloca.h /usr/include/string.h ../../include/e_os.h
randfile.o: /usr/include/unistd.h /usr/include/posix_opt.h
randfile.o: /usr/include/confname.h ../crypto.h ../../include/stack.h
randfile.o: ../../include/buffer.h ../../include/bio.h ../../include/err.h
randfile.o: /usr/include/errno.h /usr/include/errnos.h
randfile.o: /usr/include/linux/errno.h /usr/include/asm/errno.h
randfile.o: /usr/include/sys/stat.h /usr/include/statbuf.h rand.h
rand_lib.o: /usr/include/stdio.h /usr/include/libio.h /usr/include/features.h
rand_lib.o: /usr/include/sys/cdefs.h /usr/include/gnu/stubs.h
rand_lib.o: /usr/include/_G_config.h /usr/include/gnu/types.h
rand_lib.o: /usr/lib/gcc-lib/i386-redhat-linux/2.7.2.3/include/stddef.h
rand_lib.o: /usr/lib/gcc-lib/i386-redhat-linux/2.7.2.3/include/stdarg.h
rand_lib.o: /usr/include/stdio_lim.h ../cryptlib.h /usr/include/stdlib.h
rand_lib.o: /usr/include/sys/types.h /usr/include/time.h
rand_lib.o: /usr/include/endian.h /usr/include/bytesex.h
rand_lib.o: /usr/include/sys/select.h /usr/include/selectbits.h
rand_lib.o: /usr/include/alloca.h /usr/include/string.h ../../include/e_os.h
rand_lib.o: /usr/include/unistd.h /usr/include/posix_opt.h
rand_lib.o: /usr/include/confname.h ../crypto.h ../../include/stack.h
rand_lib.o: ../../include/buffer.h ../../include/bio.h ../../include/err.h
rand_lib.o: /usr/include/errno.h /usr/include/errnos.h
rand_lib.o: /usr/include/linux/errno.h /usr/include/asm/errno.h rand.h
