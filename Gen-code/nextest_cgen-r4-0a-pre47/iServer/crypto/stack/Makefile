#
# SSLeay/crypto/stack/Makefile
#

DIR=	stack
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
LIBSRC=stack.c
LIBOBJ=stack.o

SRC= $(LIBSRC)

EXHEADER= stack.h
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

stack.o: /usr/include/stdio.h /usr/include/libio.h /usr/include/features.h
stack.o: /usr/include/sys/cdefs.h /usr/include/gnu/stubs.h
stack.o: /usr/include/_G_config.h /usr/include/gnu/types.h
stack.o: /usr/lib/gcc-lib/i386-redhat-linux/2.7.2.3/include/stddef.h
stack.o: /usr/lib/gcc-lib/i386-redhat-linux/2.7.2.3/include/stdarg.h
stack.o: /usr/include/stdio_lim.h ../cryptlib.h /usr/include/stdlib.h
stack.o: /usr/include/sys/types.h /usr/include/time.h /usr/include/endian.h
stack.o: /usr/include/bytesex.h /usr/include/sys/select.h
stack.o: /usr/include/selectbits.h /usr/include/alloca.h
stack.o: /usr/include/string.h ../../include/e_os.h /usr/include/unistd.h
stack.o: /usr/include/posix_opt.h /usr/include/confname.h ../crypto.h stack.h
stack.o: ../../include/buffer.h ../../include/bio.h ../../include/err.h
stack.o: /usr/include/errno.h /usr/include/errnos.h
stack.o: /usr/include/linux/errno.h /usr/include/asm/errno.h
