#
# SSLeay/crypto/bio/Makefile
#

DIR=	bio
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

ERR=bio
ERRC=bio_err
GENERAL=Makefile
TEST=
APPS=

LIB=$(TOP)/libcrypto.a
LIBSRC= bio_lib.c bio_cb.c $(ERRC).c \
	bss_mem.c bss_null.c bss_fd.c \
	bss_file.c bss_sock.c bss_conn.c \
	bf_null.c bf_buff.c b_print.c b_dump.c \
	b_sock.c bss_acpt.c bf_nbio.c
LIBOBJ= bio_lib.o bio_cb.o $(ERRC).o \
	bss_mem.o bss_null.o bss_fd.o \
	bss_file.o bss_sock.o bss_conn.o \
	bf_null.o bf_buff.o b_print.o b_dump.o \
	b_sock.o bss_acpt.o bf_nbio.o

SRC= $(LIBSRC)

EXHEADER= bio.h bss_file.c
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
	@for i in $(EXHEADER) bss_file.c ; \
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

bio_lib.o: /usr/include/stdio.h /usr/include/libio.h /usr/include/features.h
bio_lib.o: /usr/include/sys/cdefs.h /usr/include/gnu/stubs.h
bio_lib.o: /usr/include/_G_config.h /usr/include/gnu/types.h
bio_lib.o: /usr/lib/gcc-lib/i386-redhat-linux/2.7.2.3/include/stddef.h
bio_lib.o: /usr/lib/gcc-lib/i386-redhat-linux/2.7.2.3/include/stdarg.h
bio_lib.o: /usr/include/stdio_lim.h /usr/include/errno.h
bio_lib.o: /usr/include/errnos.h /usr/include/linux/errno.h
bio_lib.o: /usr/include/asm/errno.h ../crypto.h ../../include/stack.h
bio_lib.o: ../cryptlib.h /usr/include/stdlib.h /usr/include/sys/types.h
bio_lib.o: /usr/include/time.h /usr/include/endian.h /usr/include/bytesex.h
bio_lib.o: /usr/include/sys/select.h /usr/include/selectbits.h
bio_lib.o: /usr/include/alloca.h /usr/include/string.h ../../include/e_os.h
bio_lib.o: /usr/include/unistd.h /usr/include/posix_opt.h
bio_lib.o: /usr/include/confname.h ../../include/buffer.h bio.h
bio_lib.o: ../../include/err.h
bio_cb.o: /usr/include/stdio.h /usr/include/libio.h /usr/include/features.h
bio_cb.o: /usr/include/sys/cdefs.h /usr/include/gnu/stubs.h
bio_cb.o: /usr/include/_G_config.h /usr/include/gnu/types.h
bio_cb.o: /usr/lib/gcc-lib/i386-redhat-linux/2.7.2.3/include/stddef.h
bio_cb.o: /usr/lib/gcc-lib/i386-redhat-linux/2.7.2.3/include/stdarg.h
bio_cb.o: /usr/include/stdio_lim.h /usr/include/string.h
bio_cb.o: /usr/include/stdlib.h /usr/include/sys/types.h /usr/include/time.h
bio_cb.o: /usr/include/endian.h /usr/include/bytesex.h
bio_cb.o: /usr/include/sys/select.h /usr/include/selectbits.h
bio_cb.o: /usr/include/alloca.h ../cryptlib.h ../../include/e_os.h
bio_cb.o: /usr/include/unistd.h /usr/include/posix_opt.h
bio_cb.o: /usr/include/confname.h ../crypto.h ../../include/stack.h
bio_cb.o: ../../include/buffer.h bio.h ../../include/err.h
bio_cb.o: /usr/include/errno.h /usr/include/errnos.h
bio_cb.o: /usr/include/linux/errno.h /usr/include/asm/errno.h
bio_err.o: /usr/include/stdio.h /usr/include/libio.h /usr/include/features.h
bio_err.o: /usr/include/sys/cdefs.h /usr/include/gnu/stubs.h
bio_err.o: /usr/include/_G_config.h /usr/include/gnu/types.h
bio_err.o: /usr/lib/gcc-lib/i386-redhat-linux/2.7.2.3/include/stddef.h
bio_err.o: /usr/lib/gcc-lib/i386-redhat-linux/2.7.2.3/include/stdarg.h
bio_err.o: /usr/include/stdio_lim.h ../../include/err.h /usr/include/errno.h
bio_err.o: /usr/include/errnos.h /usr/include/linux/errno.h
bio_err.o: /usr/include/asm/errno.h bio.h ../crypto.h ../../include/stack.h
bss_mem.o: /usr/include/stdio.h /usr/include/libio.h /usr/include/features.h
bss_mem.o: /usr/include/sys/cdefs.h /usr/include/gnu/stubs.h
bss_mem.o: /usr/include/_G_config.h /usr/include/gnu/types.h
bss_mem.o: /usr/lib/gcc-lib/i386-redhat-linux/2.7.2.3/include/stddef.h
bss_mem.o: /usr/lib/gcc-lib/i386-redhat-linux/2.7.2.3/include/stdarg.h
bss_mem.o: /usr/include/stdio_lim.h /usr/include/errno.h
bss_mem.o: /usr/include/errnos.h /usr/include/linux/errno.h
bss_mem.o: /usr/include/asm/errno.h ../cryptlib.h /usr/include/stdlib.h
bss_mem.o: /usr/include/sys/types.h /usr/include/time.h /usr/include/endian.h
bss_mem.o: /usr/include/bytesex.h /usr/include/sys/select.h
bss_mem.o: /usr/include/selectbits.h /usr/include/alloca.h
bss_mem.o: /usr/include/string.h ../../include/e_os.h /usr/include/unistd.h
bss_mem.o: /usr/include/posix_opt.h /usr/include/confname.h ../crypto.h
bss_mem.o: ../../include/stack.h ../../include/buffer.h bio.h
bss_mem.o: ../../include/err.h
bss_null.o: /usr/include/stdio.h /usr/include/libio.h /usr/include/features.h
bss_null.o: /usr/include/sys/cdefs.h /usr/include/gnu/stubs.h
bss_null.o: /usr/include/_G_config.h /usr/include/gnu/types.h
bss_null.o: /usr/lib/gcc-lib/i386-redhat-linux/2.7.2.3/include/stddef.h
bss_null.o: /usr/lib/gcc-lib/i386-redhat-linux/2.7.2.3/include/stdarg.h
bss_null.o: /usr/include/stdio_lim.h /usr/include/errno.h
bss_null.o: /usr/include/errnos.h /usr/include/linux/errno.h
bss_null.o: /usr/include/asm/errno.h ../cryptlib.h /usr/include/stdlib.h
bss_null.o: /usr/include/sys/types.h /usr/include/time.h
bss_null.o: /usr/include/endian.h /usr/include/bytesex.h
bss_null.o: /usr/include/sys/select.h /usr/include/selectbits.h
bss_null.o: /usr/include/alloca.h /usr/include/string.h ../../include/e_os.h
bss_null.o: /usr/include/unistd.h /usr/include/posix_opt.h
bss_null.o: /usr/include/confname.h ../crypto.h ../../include/stack.h
bss_null.o: ../../include/buffer.h bio.h ../../include/err.h
bss_fd.o: bss_sock.c /usr/include/stdio.h /usr/include/libio.h
bss_fd.o: /usr/include/features.h /usr/include/sys/cdefs.h
bss_fd.o: /usr/include/gnu/stubs.h /usr/include/_G_config.h
bss_fd.o: /usr/include/gnu/types.h
bss_fd.o: /usr/lib/gcc-lib/i386-redhat-linux/2.7.2.3/include/stddef.h
bss_fd.o: /usr/lib/gcc-lib/i386-redhat-linux/2.7.2.3/include/stdarg.h
bss_fd.o: /usr/include/stdio_lim.h /usr/include/errno.h /usr/include/errnos.h
bss_fd.o: /usr/include/linux/errno.h /usr/include/asm/errno.h ../cryptlib.h
bss_fd.o: /usr/include/stdlib.h /usr/include/sys/types.h /usr/include/time.h
bss_fd.o: /usr/include/endian.h /usr/include/bytesex.h
bss_fd.o: /usr/include/sys/select.h /usr/include/selectbits.h
bss_fd.o: /usr/include/alloca.h /usr/include/string.h ../../include/e_os.h
bss_fd.o: /usr/include/unistd.h /usr/include/posix_opt.h
bss_fd.o: /usr/include/confname.h ../crypto.h ../../include/stack.h
bss_fd.o: ../../include/buffer.h bio.h ../../include/err.h
bss_file.o: /usr/include/stdio.h /usr/include/libio.h /usr/include/features.h
bss_file.o: /usr/include/sys/cdefs.h /usr/include/gnu/stubs.h
bss_file.o: /usr/include/_G_config.h /usr/include/gnu/types.h
bss_file.o: /usr/lib/gcc-lib/i386-redhat-linux/2.7.2.3/include/stddef.h
bss_file.o: /usr/lib/gcc-lib/i386-redhat-linux/2.7.2.3/include/stdarg.h
bss_file.o: /usr/include/stdio_lim.h /usr/include/errno.h
bss_file.o: /usr/include/errnos.h /usr/include/linux/errno.h
bss_file.o: /usr/include/asm/errno.h ../cryptlib.h /usr/include/stdlib.h
bss_file.o: /usr/include/sys/types.h /usr/include/time.h
bss_file.o: /usr/include/endian.h /usr/include/bytesex.h
bss_file.o: /usr/include/sys/select.h /usr/include/selectbits.h
bss_file.o: /usr/include/alloca.h /usr/include/string.h ../../include/e_os.h
bss_file.o: /usr/include/unistd.h /usr/include/posix_opt.h
bss_file.o: /usr/include/confname.h ../crypto.h ../../include/stack.h
bss_file.o: ../../include/buffer.h bio.h ../../include/err.h
bss_sock.o: /usr/include/stdio.h /usr/include/libio.h /usr/include/features.h
bss_sock.o: /usr/include/sys/cdefs.h /usr/include/gnu/stubs.h
bss_sock.o: /usr/include/_G_config.h /usr/include/gnu/types.h
bss_sock.o: /usr/lib/gcc-lib/i386-redhat-linux/2.7.2.3/include/stddef.h
bss_sock.o: /usr/lib/gcc-lib/i386-redhat-linux/2.7.2.3/include/stdarg.h
bss_sock.o: /usr/include/stdio_lim.h /usr/include/errno.h
bss_sock.o: /usr/include/errnos.h /usr/include/linux/errno.h
bss_sock.o: /usr/include/asm/errno.h ../cryptlib.h /usr/include/stdlib.h
bss_sock.o: /usr/include/sys/types.h /usr/include/time.h
bss_sock.o: /usr/include/endian.h /usr/include/bytesex.h
bss_sock.o: /usr/include/sys/select.h /usr/include/selectbits.h
bss_sock.o: /usr/include/alloca.h /usr/include/string.h ../../include/e_os.h
bss_sock.o: /usr/include/unistd.h /usr/include/posix_opt.h
bss_sock.o: /usr/include/confname.h ../crypto.h ../../include/stack.h
bss_sock.o: ../../include/buffer.h bio.h ../../include/err.h
bss_conn.o: /usr/include/stdio.h /usr/include/libio.h /usr/include/features.h
bss_conn.o: /usr/include/sys/cdefs.h /usr/include/gnu/stubs.h
bss_conn.o: /usr/include/_G_config.h /usr/include/gnu/types.h
bss_conn.o: /usr/lib/gcc-lib/i386-redhat-linux/2.7.2.3/include/stddef.h
bss_conn.o: /usr/lib/gcc-lib/i386-redhat-linux/2.7.2.3/include/stdarg.h
bss_conn.o: /usr/include/stdio_lim.h /usr/include/errno.h
bss_conn.o: /usr/include/errnos.h /usr/include/linux/errno.h
bss_conn.o: /usr/include/asm/errno.h ../cryptlib.h /usr/include/stdlib.h
bss_conn.o: /usr/include/sys/types.h /usr/include/time.h
bss_conn.o: /usr/include/endian.h /usr/include/bytesex.h
bss_conn.o: /usr/include/sys/select.h /usr/include/selectbits.h
bss_conn.o: /usr/include/alloca.h /usr/include/string.h ../../include/e_os.h
bss_conn.o: /usr/include/unistd.h /usr/include/posix_opt.h
bss_conn.o: /usr/include/confname.h ../crypto.h ../../include/stack.h
bss_conn.o: ../../include/buffer.h bio.h ../../include/err.h
bf_null.o: /usr/include/stdio.h /usr/include/libio.h /usr/include/features.h
bf_null.o: /usr/include/sys/cdefs.h /usr/include/gnu/stubs.h
bf_null.o: /usr/include/_G_config.h /usr/include/gnu/types.h
bf_null.o: /usr/lib/gcc-lib/i386-redhat-linux/2.7.2.3/include/stddef.h
bf_null.o: /usr/lib/gcc-lib/i386-redhat-linux/2.7.2.3/include/stdarg.h
bf_null.o: /usr/include/stdio_lim.h /usr/include/errno.h
bf_null.o: /usr/include/errnos.h /usr/include/linux/errno.h
bf_null.o: /usr/include/asm/errno.h ../cryptlib.h /usr/include/stdlib.h
bf_null.o: /usr/include/sys/types.h /usr/include/time.h /usr/include/endian.h
bf_null.o: /usr/include/bytesex.h /usr/include/sys/select.h
bf_null.o: /usr/include/selectbits.h /usr/include/alloca.h
bf_null.o: /usr/include/string.h ../../include/e_os.h /usr/include/unistd.h
bf_null.o: /usr/include/posix_opt.h /usr/include/confname.h ../crypto.h
bf_null.o: ../../include/stack.h ../../include/buffer.h bio.h
bf_null.o: ../../include/err.h ../../include/evp.h ../../include/md2.h
bf_null.o: ../../include/md5.h ../../include/sha.h ../../include/ripemd.h
bf_null.o: ../../include/des.h ../../include/rc4.h ../../include/rc2.h
bf_null.o: ../../include/rc5.h ../../include/blowfish.h ../../include/cast.h
bf_null.o: ../../include/idea.h ../../include/mdc2.h ../../include/rsa.h
bf_null.o: ../../include/bn.h ../../include/dsa.h ../../include/dh.h
bf_null.o: ../../include/objects.h ../../include/asn1.h
bf_buff.o: /usr/include/stdio.h /usr/include/libio.h /usr/include/features.h
bf_buff.o: /usr/include/sys/cdefs.h /usr/include/gnu/stubs.h
bf_buff.o: /usr/include/_G_config.h /usr/include/gnu/types.h
bf_buff.o: /usr/lib/gcc-lib/i386-redhat-linux/2.7.2.3/include/stddef.h
bf_buff.o: /usr/lib/gcc-lib/i386-redhat-linux/2.7.2.3/include/stdarg.h
bf_buff.o: /usr/include/stdio_lim.h /usr/include/errno.h
bf_buff.o: /usr/include/errnos.h /usr/include/linux/errno.h
bf_buff.o: /usr/include/asm/errno.h ../cryptlib.h /usr/include/stdlib.h
bf_buff.o: /usr/include/sys/types.h /usr/include/time.h /usr/include/endian.h
bf_buff.o: /usr/include/bytesex.h /usr/include/sys/select.h
bf_buff.o: /usr/include/selectbits.h /usr/include/alloca.h
bf_buff.o: /usr/include/string.h ../../include/e_os.h /usr/include/unistd.h
bf_buff.o: /usr/include/posix_opt.h /usr/include/confname.h ../crypto.h
bf_buff.o: ../../include/stack.h ../../include/buffer.h bio.h
bf_buff.o: ../../include/err.h ../../include/evp.h ../../include/md2.h
bf_buff.o: ../../include/md5.h ../../include/sha.h ../../include/ripemd.h
bf_buff.o: ../../include/des.h ../../include/rc4.h ../../include/rc2.h
bf_buff.o: ../../include/rc5.h ../../include/blowfish.h ../../include/cast.h
bf_buff.o: ../../include/idea.h ../../include/mdc2.h ../../include/rsa.h
bf_buff.o: ../../include/bn.h ../../include/dsa.h ../../include/dh.h
bf_buff.o: ../../include/objects.h ../../include/asn1.h
b_print.o: /usr/include/stdio.h /usr/include/libio.h /usr/include/features.h
b_print.o: /usr/include/sys/cdefs.h /usr/include/gnu/stubs.h
b_print.o: /usr/include/_G_config.h /usr/include/gnu/types.h
b_print.o: /usr/lib/gcc-lib/i386-redhat-linux/2.7.2.3/include/stddef.h
b_print.o: /usr/lib/gcc-lib/i386-redhat-linux/2.7.2.3/include/stdarg.h
b_print.o: /usr/include/stdio_lim.h ../cryptlib.h /usr/include/stdlib.h
b_print.o: /usr/include/sys/types.h /usr/include/time.h /usr/include/endian.h
b_print.o: /usr/include/bytesex.h /usr/include/sys/select.h
b_print.o: /usr/include/selectbits.h /usr/include/alloca.h
b_print.o: /usr/include/string.h ../../include/e_os.h /usr/include/unistd.h
b_print.o: /usr/include/posix_opt.h /usr/include/confname.h ../crypto.h
b_print.o: ../../include/stack.h ../../include/buffer.h bio.h
b_print.o: ../../include/err.h /usr/include/errno.h /usr/include/errnos.h
b_print.o: /usr/include/linux/errno.h /usr/include/asm/errno.h
b_dump.o: /usr/include/stdio.h /usr/include/libio.h /usr/include/features.h
b_dump.o: /usr/include/sys/cdefs.h /usr/include/gnu/stubs.h
b_dump.o: /usr/include/_G_config.h /usr/include/gnu/types.h
b_dump.o: /usr/lib/gcc-lib/i386-redhat-linux/2.7.2.3/include/stddef.h
b_dump.o: /usr/lib/gcc-lib/i386-redhat-linux/2.7.2.3/include/stdarg.h
b_dump.o: /usr/include/stdio_lim.h ../cryptlib.h /usr/include/stdlib.h
b_dump.o: /usr/include/sys/types.h /usr/include/time.h /usr/include/endian.h
b_dump.o: /usr/include/bytesex.h /usr/include/sys/select.h
b_dump.o: /usr/include/selectbits.h /usr/include/alloca.h
b_dump.o: /usr/include/string.h ../../include/e_os.h /usr/include/unistd.h
b_dump.o: /usr/include/posix_opt.h /usr/include/confname.h ../crypto.h
b_dump.o: ../../include/stack.h ../../include/buffer.h bio.h
b_dump.o: ../../include/err.h /usr/include/errno.h /usr/include/errnos.h
b_dump.o: /usr/include/linux/errno.h /usr/include/asm/errno.h
b_sock.o: /usr/include/stdio.h /usr/include/libio.h /usr/include/features.h
b_sock.o: /usr/include/sys/cdefs.h /usr/include/gnu/stubs.h
b_sock.o: /usr/include/_G_config.h /usr/include/gnu/types.h
b_sock.o: /usr/lib/gcc-lib/i386-redhat-linux/2.7.2.3/include/stddef.h
b_sock.o: /usr/lib/gcc-lib/i386-redhat-linux/2.7.2.3/include/stdarg.h
b_sock.o: /usr/include/stdio_lim.h /usr/include/stdlib.h
b_sock.o: /usr/include/sys/types.h /usr/include/time.h /usr/include/endian.h
b_sock.o: /usr/include/bytesex.h /usr/include/sys/select.h
b_sock.o: /usr/include/selectbits.h /usr/include/alloca.h
b_sock.o: /usr/include/errno.h /usr/include/errnos.h
b_sock.o: /usr/include/linux/errno.h /usr/include/asm/errno.h ../cryptlib.h
b_sock.o: /usr/include/string.h ../../include/e_os.h /usr/include/unistd.h
b_sock.o: /usr/include/posix_opt.h /usr/include/confname.h ../crypto.h
b_sock.o: ../../include/stack.h ../../include/buffer.h bio.h
b_sock.o: ../../include/err.h
bss_acpt.o: /usr/include/stdio.h /usr/include/libio.h /usr/include/features.h
bss_acpt.o: /usr/include/sys/cdefs.h /usr/include/gnu/stubs.h
bss_acpt.o: /usr/include/_G_config.h /usr/include/gnu/types.h
bss_acpt.o: /usr/lib/gcc-lib/i386-redhat-linux/2.7.2.3/include/stddef.h
bss_acpt.o: /usr/lib/gcc-lib/i386-redhat-linux/2.7.2.3/include/stdarg.h
bss_acpt.o: /usr/include/stdio_lim.h /usr/include/errno.h
bss_acpt.o: /usr/include/errnos.h /usr/include/linux/errno.h
bss_acpt.o: /usr/include/asm/errno.h ../cryptlib.h /usr/include/stdlib.h
bss_acpt.o: /usr/include/sys/types.h /usr/include/time.h
bss_acpt.o: /usr/include/endian.h /usr/include/bytesex.h
bss_acpt.o: /usr/include/sys/select.h /usr/include/selectbits.h
bss_acpt.o: /usr/include/alloca.h /usr/include/string.h ../../include/e_os.h
bss_acpt.o: /usr/include/unistd.h /usr/include/posix_opt.h
bss_acpt.o: /usr/include/confname.h ../crypto.h ../../include/stack.h
bss_acpt.o: ../../include/buffer.h bio.h ../../include/err.h
bf_nbio.o: /usr/include/stdio.h /usr/include/libio.h /usr/include/features.h
bf_nbio.o: /usr/include/sys/cdefs.h /usr/include/gnu/stubs.h
bf_nbio.o: /usr/include/_G_config.h /usr/include/gnu/types.h
bf_nbio.o: /usr/lib/gcc-lib/i386-redhat-linux/2.7.2.3/include/stddef.h
bf_nbio.o: /usr/lib/gcc-lib/i386-redhat-linux/2.7.2.3/include/stdarg.h
bf_nbio.o: /usr/include/stdio_lim.h /usr/include/errno.h
bf_nbio.o: /usr/include/errnos.h /usr/include/linux/errno.h
bf_nbio.o: /usr/include/asm/errno.h ../cryptlib.h /usr/include/stdlib.h
bf_nbio.o: /usr/include/sys/types.h /usr/include/time.h /usr/include/endian.h
bf_nbio.o: /usr/include/bytesex.h /usr/include/sys/select.h
bf_nbio.o: /usr/include/selectbits.h /usr/include/alloca.h
bf_nbio.o: /usr/include/string.h ../../include/e_os.h /usr/include/unistd.h
bf_nbio.o: /usr/include/posix_opt.h /usr/include/confname.h ../crypto.h
bf_nbio.o: ../../include/stack.h ../../include/buffer.h bio.h
bf_nbio.o: ../../include/err.h ../../include/rand.h ../../include/evp.h
bf_nbio.o: ../../include/md2.h ../../include/md5.h ../../include/sha.h
bf_nbio.o: ../../include/ripemd.h ../../include/des.h ../../include/rc4.h
bf_nbio.o: ../../include/rc2.h ../../include/rc5.h ../../include/blowfish.h
bf_nbio.o: ../../include/cast.h ../../include/idea.h ../../include/mdc2.h
bf_nbio.o: ../../include/rsa.h ../../include/bn.h ../../include/dsa.h
bf_nbio.o: ../../include/dh.h ../../include/objects.h ../../include/asn1.h
