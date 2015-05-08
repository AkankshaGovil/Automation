#
# SSLeay/crypto/dsa/Makefile
#

DIR=	dsa
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

ERR=dsa
ERRC=dsa_err
GENERAL=Makefile
TEST=dsatest.c
APPS=

LIB=$(TOP)/libcrypto.a
LIBSRC= dsa_gen.c dsa_key.c dsa_lib.c dsa_vrf.c dsa_sign.c $(ERRC).c
LIBOBJ= dsa_gen.o dsa_key.o dsa_lib.o dsa_vrf.o dsa_sign.o $(ERRC).o

SRC= $(LIBSRC)

EXHEADER= dsa.h
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

dsa_gen.o: /usr/include/stdio.h /usr/include/libio.h /usr/include/features.h
dsa_gen.o: /usr/include/sys/cdefs.h /usr/include/gnu/stubs.h
dsa_gen.o: /usr/include/_G_config.h /usr/include/gnu/types.h
dsa_gen.o: /usr/lib/gcc-lib/i386-redhat-linux/2.7.2.3/include/stddef.h
dsa_gen.o: /usr/lib/gcc-lib/i386-redhat-linux/2.7.2.3/include/stdarg.h
dsa_gen.o: /usr/include/stdio_lim.h /usr/include/time.h
dsa_gen.o: /usr/include/timebits.h ../cryptlib.h /usr/include/stdlib.h
dsa_gen.o: /usr/include/sys/types.h /usr/include/endian.h
dsa_gen.o: /usr/include/bytesex.h /usr/include/sys/select.h
dsa_gen.o: /usr/include/selectbits.h /usr/include/alloca.h
dsa_gen.o: /usr/include/string.h ../../include/e_os.h /usr/include/unistd.h
dsa_gen.o: /usr/include/posix_opt.h /usr/include/confname.h ../crypto.h
dsa_gen.o: ../../include/stack.h ../../include/buffer.h ../../include/bio.h
dsa_gen.o: ../../include/err.h /usr/include/errno.h /usr/include/errnos.h
dsa_gen.o: /usr/include/linux/errno.h /usr/include/asm/errno.h
dsa_gen.o: ../../include/sha.h ../../include/bn.h dsa.h ../../include/rand.h
dsa_key.o: /usr/include/stdio.h /usr/include/libio.h /usr/include/features.h
dsa_key.o: /usr/include/sys/cdefs.h /usr/include/gnu/stubs.h
dsa_key.o: /usr/include/_G_config.h /usr/include/gnu/types.h
dsa_key.o: /usr/lib/gcc-lib/i386-redhat-linux/2.7.2.3/include/stddef.h
dsa_key.o: /usr/lib/gcc-lib/i386-redhat-linux/2.7.2.3/include/stdarg.h
dsa_key.o: /usr/include/stdio_lim.h /usr/include/time.h
dsa_key.o: /usr/include/timebits.h ../cryptlib.h /usr/include/stdlib.h
dsa_key.o: /usr/include/sys/types.h /usr/include/endian.h
dsa_key.o: /usr/include/bytesex.h /usr/include/sys/select.h
dsa_key.o: /usr/include/selectbits.h /usr/include/alloca.h
dsa_key.o: /usr/include/string.h ../../include/e_os.h /usr/include/unistd.h
dsa_key.o: /usr/include/posix_opt.h /usr/include/confname.h ../crypto.h
dsa_key.o: ../../include/stack.h ../../include/buffer.h ../../include/bio.h
dsa_key.o: ../../include/err.h /usr/include/errno.h /usr/include/errnos.h
dsa_key.o: /usr/include/linux/errno.h /usr/include/asm/errno.h
dsa_key.o: ../../include/sha.h ../../include/bn.h dsa.h ../../include/rand.h
dsa_lib.o: /usr/include/stdio.h /usr/include/libio.h /usr/include/features.h
dsa_lib.o: /usr/include/sys/cdefs.h /usr/include/gnu/stubs.h
dsa_lib.o: /usr/include/_G_config.h /usr/include/gnu/types.h
dsa_lib.o: /usr/lib/gcc-lib/i386-redhat-linux/2.7.2.3/include/stddef.h
dsa_lib.o: /usr/lib/gcc-lib/i386-redhat-linux/2.7.2.3/include/stdarg.h
dsa_lib.o: /usr/include/stdio_lim.h ../cryptlib.h /usr/include/stdlib.h
dsa_lib.o: /usr/include/sys/types.h /usr/include/time.h
dsa_lib.o: /usr/include/timebits.h /usr/include/endian.h
dsa_lib.o: /usr/include/bytesex.h /usr/include/sys/select.h
dsa_lib.o: /usr/include/selectbits.h /usr/include/alloca.h
dsa_lib.o: /usr/include/string.h ../../include/e_os.h /usr/include/unistd.h
dsa_lib.o: /usr/include/posix_opt.h /usr/include/confname.h ../crypto.h
dsa_lib.o: ../../include/stack.h ../../include/buffer.h ../../include/bio.h
dsa_lib.o: ../../include/err.h /usr/include/errno.h /usr/include/errnos.h
dsa_lib.o: /usr/include/linux/errno.h /usr/include/asm/errno.h
dsa_lib.o: ../../include/bn.h dsa.h ../../include/asn1.h
dsa_vrf.o: /usr/include/stdio.h /usr/include/libio.h /usr/include/features.h
dsa_vrf.o: /usr/include/sys/cdefs.h /usr/include/gnu/stubs.h
dsa_vrf.o: /usr/include/_G_config.h /usr/include/gnu/types.h
dsa_vrf.o: /usr/lib/gcc-lib/i386-redhat-linux/2.7.2.3/include/stddef.h
dsa_vrf.o: /usr/lib/gcc-lib/i386-redhat-linux/2.7.2.3/include/stdarg.h
dsa_vrf.o: /usr/include/stdio_lim.h ../cryptlib.h /usr/include/stdlib.h
dsa_vrf.o: /usr/include/sys/types.h /usr/include/time.h
dsa_vrf.o: /usr/include/timebits.h /usr/include/endian.h
dsa_vrf.o: /usr/include/bytesex.h /usr/include/sys/select.h
dsa_vrf.o: /usr/include/selectbits.h /usr/include/alloca.h
dsa_vrf.o: /usr/include/string.h ../../include/e_os.h /usr/include/unistd.h
dsa_vrf.o: /usr/include/posix_opt.h /usr/include/confname.h ../crypto.h
dsa_vrf.o: ../../include/stack.h ../../include/buffer.h ../../include/bio.h
dsa_vrf.o: ../../include/err.h /usr/include/errno.h /usr/include/errnos.h
dsa_vrf.o: /usr/include/linux/errno.h /usr/include/asm/errno.h
dsa_vrf.o: ../../include/bn.h dsa.h ../../include/rand.h ../../include/asn1.h
dsa_vrf.o: ../../include/asn1_mac.h ../../include/x509.h ../../include/rsa.h
dsa_vrf.o: ../../include/dh.h ../../include/evp.h ../../include/md2.h
dsa_vrf.o: ../../include/md5.h ../../include/sha.h ../../include/ripemd.h
dsa_vrf.o: ../../include/des.h ../../include/rc4.h ../../include/rc2.h
dsa_vrf.o: ../../include/rc5.h ../../include/blowfish.h ../../include/cast.h
dsa_vrf.o: ../../include/idea.h ../../include/mdc2.h ../../include/objects.h
dsa_vrf.o: ../../include/x509_vfy.h ../../include/pkcs7.h
dsa_sign.o: /usr/include/stdio.h /usr/include/libio.h /usr/include/features.h
dsa_sign.o: /usr/include/sys/cdefs.h /usr/include/gnu/stubs.h
dsa_sign.o: /usr/include/_G_config.h /usr/include/gnu/types.h
dsa_sign.o: /usr/lib/gcc-lib/i386-redhat-linux/2.7.2.3/include/stddef.h
dsa_sign.o: /usr/lib/gcc-lib/i386-redhat-linux/2.7.2.3/include/stdarg.h
dsa_sign.o: /usr/include/stdio_lim.h ../cryptlib.h /usr/include/stdlib.h
dsa_sign.o: /usr/include/sys/types.h /usr/include/time.h
dsa_sign.o: /usr/include/timebits.h /usr/include/endian.h
dsa_sign.o: /usr/include/bytesex.h /usr/include/sys/select.h
dsa_sign.o: /usr/include/selectbits.h /usr/include/alloca.h
dsa_sign.o: /usr/include/string.h ../../include/e_os.h /usr/include/unistd.h
dsa_sign.o: /usr/include/posix_opt.h /usr/include/confname.h ../crypto.h
dsa_sign.o: ../../include/stack.h ../../include/buffer.h ../../include/bio.h
dsa_sign.o: ../../include/err.h /usr/include/errno.h /usr/include/errnos.h
dsa_sign.o: /usr/include/linux/errno.h /usr/include/asm/errno.h
dsa_sign.o: ../../include/bn.h dsa.h ../../include/rand.h
dsa_sign.o: ../../include/asn1.h
dsa_err.o: /usr/include/stdio.h /usr/include/libio.h /usr/include/features.h
dsa_err.o: /usr/include/sys/cdefs.h /usr/include/gnu/stubs.h
dsa_err.o: /usr/include/_G_config.h /usr/include/gnu/types.h
dsa_err.o: /usr/lib/gcc-lib/i386-redhat-linux/2.7.2.3/include/stddef.h
dsa_err.o: /usr/lib/gcc-lib/i386-redhat-linux/2.7.2.3/include/stdarg.h
dsa_err.o: /usr/include/stdio_lim.h ../../include/err.h /usr/include/errno.h
dsa_err.o: /usr/include/errnos.h /usr/include/linux/errno.h
dsa_err.o: /usr/include/asm/errno.h dsa.h ../../include/bn.h
