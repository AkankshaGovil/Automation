#
# SSLeay/crypto/err/Makefile
#

DIR=	err
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

GENERAL=Makefile
TEST=
APPS=

LIB=$(TOP)/libcrypto.a
LIBSRC=err.c err_all.c err_prn.c
LIBOBJ=err.o err_all.o err_prn.o

SRC= $(LIBSRC)

EXHEADER= err.h
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

err.o: /usr/include/stdio.h /usr/include/libio.h /usr/include/features.h
err.o: /usr/include/sys/cdefs.h /usr/include/gnu/stubs.h
err.o: /usr/include/_G_config.h /usr/include/gnu/types.h
err.o: /usr/lib/gcc-lib/i386-redhat-linux/2.7.2.3/include/stddef.h
err.o: /usr/lib/gcc-lib/i386-redhat-linux/2.7.2.3/include/stdarg.h
err.o: /usr/include/stdio_lim.h ../../include/lhash.h ../crypto.h
err.o: ../../include/stack.h ../cryptlib.h /usr/include/stdlib.h
err.o: /usr/include/sys/types.h /usr/include/time.h /usr/include/endian.h
err.o: /usr/include/bytesex.h /usr/include/sys/select.h
err.o: /usr/include/selectbits.h /usr/include/alloca.h /usr/include/string.h
err.o: ../../include/e_os.h /usr/include/unistd.h /usr/include/posix_opt.h
err.o: /usr/include/confname.h ../../include/buffer.h ../../include/bio.h
err.o: err.h /usr/include/errno.h /usr/include/errnos.h
err.o: /usr/include/linux/errno.h /usr/include/asm/errno.h
err_all.o: /usr/include/stdio.h /usr/include/libio.h /usr/include/features.h
err_all.o: /usr/include/sys/cdefs.h /usr/include/gnu/stubs.h
err_all.o: /usr/include/_G_config.h /usr/include/gnu/types.h
err_all.o: /usr/lib/gcc-lib/i386-redhat-linux/2.7.2.3/include/stddef.h
err_all.o: /usr/lib/gcc-lib/i386-redhat-linux/2.7.2.3/include/stdarg.h
err_all.o: /usr/include/stdio_lim.h ../../include/asn1.h /usr/include/time.h
err_all.o: ../../include/bn.h ../../include/stack.h ../../include/buffer.h
err_all.o: ../../include/bio.h ../crypto.h ../../include/rsa.h
err_all.o: ../../include/dh.h ../../include/dsa.h ../../include/evp.h
err_all.o: ../../include/md2.h ../../include/md5.h ../../include/sha.h
err_all.o: ../../include/ripemd.h ../../include/des.h ../../include/rc4.h
err_all.o: ../../include/rc2.h ../../include/rc5.h ../../include/blowfish.h
err_all.o: ../../include/cast.h ../../include/idea.h ../../include/mdc2.h
err_all.o: ../../include/objects.h ../../include/pem.h ../../include/x509.h
err_all.o: ../../include/x509_vfy.h ../../include/pkcs7.h
err_all.o: ../../include/conf.h ../../include/lhash.h err.h
err_all.o: /usr/include/errno.h /usr/include/errnos.h
err_all.o: /usr/include/linux/errno.h /usr/include/asm/errno.h
err_prn.o: /usr/include/stdio.h /usr/include/libio.h /usr/include/features.h
err_prn.o: /usr/include/sys/cdefs.h /usr/include/gnu/stubs.h
err_prn.o: /usr/include/_G_config.h /usr/include/gnu/types.h
err_prn.o: /usr/lib/gcc-lib/i386-redhat-linux/2.7.2.3/include/stddef.h
err_prn.o: /usr/lib/gcc-lib/i386-redhat-linux/2.7.2.3/include/stdarg.h
err_prn.o: /usr/include/stdio_lim.h ../../include/lhash.h ../crypto.h
err_prn.o: ../../include/stack.h ../cryptlib.h /usr/include/stdlib.h
err_prn.o: /usr/include/sys/types.h /usr/include/time.h /usr/include/endian.h
err_prn.o: /usr/include/bytesex.h /usr/include/sys/select.h
err_prn.o: /usr/include/selectbits.h /usr/include/alloca.h
err_prn.o: /usr/include/string.h ../../include/e_os.h /usr/include/unistd.h
err_prn.o: /usr/include/posix_opt.h /usr/include/confname.h
err_prn.o: ../../include/buffer.h ../../include/bio.h err.h
err_prn.o: /usr/include/errno.h /usr/include/errnos.h
err_prn.o: /usr/include/linux/errno.h /usr/include/asm/errno.h
