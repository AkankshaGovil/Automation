#
# SSLeay/crypto/dh/Makefile
#

DIR=	dh
TOP=	../..
CC=	cc
INCLUDES= -I.. -I../../include
CFLAG=-g
INSTALLTOP=/usr/local/ssl
MAKE=		make -f Makefile.ssl
MAKEDEPEND=	makedepend -f Makefile.ssl
MAKEFILE=	Makefile.ssl
AR=		ar r

CFLAGS= $(INCLUDES) $(CFLAG)

ERR=dh
ERRC=dh_err
GENERAL=Makefile
TEST= dhtest.c
APPS=

LIB=$(TOP)/libcrypto.a
LIBSRC= dh_gen.c dh_key.c dh_lib.c dh_check.c $(ERRC).c
LIBOBJ= dh_gen.o dh_key.o dh_lib.o dh_check.o $(ERRC).o

SRC= $(LIBSRC)

EXHEADER= dh.h
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
	/bin/rm -f *.o */*.o *.obj lib tags core .pure .nfs* *.old *.bak fluff

errors:
	perl $(TOP)/util/err-ins.pl $(ERR).err $(ERR).h
	perl ../err/err_genc.pl -s $(ERR).h $(ERRC).c

# DO NOT DELETE THIS LINE -- make depend depends on it.

dh_gen.o: /usr/include/stdio.h /usr/include/libio.h /usr/include/features.h
dh_gen.o: /usr/include/sys/cdefs.h /usr/include/gnu/stubs.h
dh_gen.o: /usr/include/_G_config.h /usr/include/gnu/types.h
dh_gen.o: /usr/lib/gcc-lib/i386-redhat-linux/2.7.2.3/include/stddef.h
dh_gen.o: /usr/lib/gcc-lib/i386-redhat-linux/2.7.2.3/include/stdarg.h
dh_gen.o: /usr/include/stdio_lim.h ../cryptlib.h /usr/include/stdlib.h
dh_gen.o: /usr/include/sys/types.h /usr/include/time.h /usr/include/endian.h
dh_gen.o: /usr/include/bytesex.h /usr/include/sys/select.h
dh_gen.o: /usr/include/selectbits.h /usr/include/alloca.h
dh_gen.o: /usr/include/string.h ../../include/e_os.h /usr/include/unistd.h
dh_gen.o: /usr/include/posix_opt.h /usr/include/confname.h ../crypto.h
dh_gen.o: ../../include/stack.h ../../include/buffer.h ../../include/bio.h
dh_gen.o: ../../include/err.h /usr/include/errno.h /usr/include/errnos.h
dh_gen.o: /usr/include/linux/errno.h /usr/include/asm/errno.h
dh_gen.o: ../../include/bn.h dh.h
dh_key.o: /usr/include/stdio.h /usr/include/libio.h /usr/include/features.h
dh_key.o: /usr/include/sys/cdefs.h /usr/include/gnu/stubs.h
dh_key.o: /usr/include/_G_config.h /usr/include/gnu/types.h
dh_key.o: /usr/lib/gcc-lib/i386-redhat-linux/2.7.2.3/include/stddef.h
dh_key.o: /usr/lib/gcc-lib/i386-redhat-linux/2.7.2.3/include/stdarg.h
dh_key.o: /usr/include/stdio_lim.h ../cryptlib.h /usr/include/stdlib.h
dh_key.o: /usr/include/sys/types.h /usr/include/time.h /usr/include/endian.h
dh_key.o: /usr/include/bytesex.h /usr/include/sys/select.h
dh_key.o: /usr/include/selectbits.h /usr/include/alloca.h
dh_key.o: /usr/include/string.h ../../include/e_os.h /usr/include/unistd.h
dh_key.o: /usr/include/posix_opt.h /usr/include/confname.h ../crypto.h
dh_key.o: ../../include/stack.h ../../include/buffer.h ../../include/bio.h
dh_key.o: ../../include/err.h /usr/include/errno.h /usr/include/errnos.h
dh_key.o: /usr/include/linux/errno.h /usr/include/asm/errno.h
dh_key.o: ../../include/bn.h ../../include/rand.h dh.h
dh_lib.o: /usr/include/stdio.h /usr/include/libio.h /usr/include/features.h
dh_lib.o: /usr/include/sys/cdefs.h /usr/include/gnu/stubs.h
dh_lib.o: /usr/include/_G_config.h /usr/include/gnu/types.h
dh_lib.o: /usr/lib/gcc-lib/i386-redhat-linux/2.7.2.3/include/stddef.h
dh_lib.o: /usr/lib/gcc-lib/i386-redhat-linux/2.7.2.3/include/stdarg.h
dh_lib.o: /usr/include/stdio_lim.h ../cryptlib.h /usr/include/stdlib.h
dh_lib.o: /usr/include/sys/types.h /usr/include/time.h /usr/include/endian.h
dh_lib.o: /usr/include/bytesex.h /usr/include/sys/select.h
dh_lib.o: /usr/include/selectbits.h /usr/include/alloca.h
dh_lib.o: /usr/include/string.h ../../include/e_os.h /usr/include/unistd.h
dh_lib.o: /usr/include/posix_opt.h /usr/include/confname.h ../crypto.h
dh_lib.o: ../../include/stack.h ../../include/buffer.h ../../include/bio.h
dh_lib.o: ../../include/err.h /usr/include/errno.h /usr/include/errnos.h
dh_lib.o: /usr/include/linux/errno.h /usr/include/asm/errno.h
dh_lib.o: ../../include/bn.h dh.h
dh_check.o: /usr/include/stdio.h /usr/include/libio.h /usr/include/features.h
dh_check.o: /usr/include/sys/cdefs.h /usr/include/gnu/stubs.h
dh_check.o: /usr/include/_G_config.h /usr/include/gnu/types.h
dh_check.o: /usr/lib/gcc-lib/i386-redhat-linux/2.7.2.3/include/stddef.h
dh_check.o: /usr/lib/gcc-lib/i386-redhat-linux/2.7.2.3/include/stdarg.h
dh_check.o: /usr/include/stdio_lim.h ../cryptlib.h /usr/include/stdlib.h
dh_check.o: /usr/include/sys/types.h /usr/include/time.h
dh_check.o: /usr/include/endian.h /usr/include/bytesex.h
dh_check.o: /usr/include/sys/select.h /usr/include/selectbits.h
dh_check.o: /usr/include/alloca.h /usr/include/string.h ../../include/e_os.h
dh_check.o: /usr/include/unistd.h /usr/include/posix_opt.h
dh_check.o: /usr/include/confname.h ../crypto.h ../../include/stack.h
dh_check.o: ../../include/buffer.h ../../include/bio.h ../../include/err.h
dh_check.o: /usr/include/errno.h /usr/include/errnos.h
dh_check.o: /usr/include/linux/errno.h /usr/include/asm/errno.h
dh_check.o: ../../include/bn.h dh.h
dh_err.o: /usr/include/stdio.h /usr/include/libio.h /usr/include/features.h
dh_err.o: /usr/include/sys/cdefs.h /usr/include/gnu/stubs.h
dh_err.o: /usr/include/_G_config.h /usr/include/gnu/types.h
dh_err.o: /usr/lib/gcc-lib/i386-redhat-linux/2.7.2.3/include/stddef.h
dh_err.o: /usr/lib/gcc-lib/i386-redhat-linux/2.7.2.3/include/stdarg.h
dh_err.o: /usr/include/stdio_lim.h ../../include/err.h /usr/include/errno.h
dh_err.o: /usr/include/errnos.h /usr/include/linux/errno.h
dh_err.o: /usr/include/asm/errno.h dh.h
