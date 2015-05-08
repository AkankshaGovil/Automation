#
# SSLeay/crypto/buffer/Makefile
#

DIR=	buffer
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

ERR=buffer
ERRC=buf_err
GENERAL=Makefile
TEST=
APPS=

LIB=$(TOP)/libcrypto.a
LIBSRC= buffer.c $(ERRC).c
LIBOBJ= buffer.o $(ERRC).o

SRC= $(LIBSRC)

EXHEADER= buffer.h
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
	$(TOP)/util/point.sh Makefile.ssl Makefile;
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
	perl $(TOP)/util/err-ins.pl $(ERR).err $(ERR).h
	perl ../err/err_genc.pl -s $(ERR).h $(ERRC).c

# DO NOT DELETE THIS LINE -- make depend depends on it.

buffer.o: /usr/include/stdio.h /usr/include/libio.h /usr/include/features.h
buffer.o: /usr/include/sys/cdefs.h /usr/include/gnu/stubs.h
buffer.o: /usr/include/_G_config.h /usr/include/gnu/types.h
buffer.o: /usr/lib/gcc-lib/i386-redhat-linux/2.7.2.3/include/stddef.h
buffer.o: /usr/lib/gcc-lib/i386-redhat-linux/2.7.2.3/include/stdarg.h
buffer.o: /usr/include/stdio_lim.h ../cryptlib.h /usr/include/stdlib.h
buffer.o: /usr/include/sys/types.h /usr/include/time.h /usr/include/endian.h
buffer.o: /usr/include/bytesex.h /usr/include/sys/select.h
buffer.o: /usr/include/selectbits.h /usr/include/alloca.h
buffer.o: /usr/include/string.h ../../include/e_os.h /usr/include/unistd.h
buffer.o: /usr/include/posix_opt.h /usr/include/confname.h ../crypto.h
buffer.o: ../../include/stack.h buffer.h ../../include/bio.h
buffer.o: ../../include/err.h /usr/include/errno.h /usr/include/errnos.h
buffer.o: /usr/include/linux/errno.h /usr/include/asm/errno.h
buf_err.o: /usr/include/stdio.h /usr/include/libio.h /usr/include/features.h
buf_err.o: /usr/include/sys/cdefs.h /usr/include/gnu/stubs.h
buf_err.o: /usr/include/_G_config.h /usr/include/gnu/types.h
buf_err.o: /usr/lib/gcc-lib/i386-redhat-linux/2.7.2.3/include/stddef.h
buf_err.o: /usr/lib/gcc-lib/i386-redhat-linux/2.7.2.3/include/stdarg.h
buf_err.o: /usr/include/stdio_lim.h ../../include/err.h /usr/include/errno.h
buf_err.o: /usr/include/errnos.h /usr/include/linux/errno.h
buf_err.o: /usr/include/asm/errno.h buffer.h
