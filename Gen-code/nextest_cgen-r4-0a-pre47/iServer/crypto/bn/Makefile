#
# SSLeay/crypto/bn/Makefile
#

DIR=	bn
TOP=	..
CC=	cc
INCLUDES= -I.. -I../../include
CFLAG=-g
INSTALLTOP=/usr/local/ssl
MAKE=		make -f Makefile.ssl
MAKEDEPEND=	makedepend -f Makefile.ssl
MAKEFILE=	Makefile.ssl
AR=		ar r

BN_ASM=		bn_asm.o
# or use
#BN_ASM=	bn86-elf.o

CFLAGS= $(INCLUDES) $(CFLAG)

ERR=bn
ERRC=bn_err
GENERAL=Makefile
TEST=bntest.c exptest.c
APPS=

LIB=$(TOP)/libcrypto.a
LIBSRC=	bn_add.c bn_div.c bn_exp.c bn_lib.c bn_mul.c \
	bn_print.c bn_rand.c bn_shift.c bn_word.c bn_blind.c \
	bn_gcd.c bn_prime.c $(ERRC).c bn_sqr.c bn_asm.c bn_recp.c bn_mont.c \
	bn_mpi.c bn_exp2.c

LIBOBJ=	bn_add.o bn_div.o bn_exp.o bn_lib.o bn_mul.o \
	bn_print.o bn_rand.o bn_shift.o bn_word.o bn_blind.o \
	bn_gcd.o bn_prime.o $(ERRC).o bn_sqr.o $(BN_ASM) bn_recp.o bn_mont.o \
	bn_mpi.o bn_exp2.o

SRC= $(LIBSRC)

EXHEADER= bn.h
HEADER=	bn_lcl.h bn_prime.h $(EXHEADER)

ALL=    $(GENERAL) $(SRC) $(HEADER)

top:
	(cd ../..; $(MAKE) DIRS=crypto SDIRS=$(DIR) sub_all)

all:	lib

knuth: bn_knuth.c
	cc -pg -I.. -I../../include bn_knuth.c -o knuth $(LIB) #../../../libefence.a

knuth.fast: bn_knuth.c
	cc -pg -fast -I.. -I../../include bn_knuth.c -o knuth $(LIB) #../../../libefence.a


lib:	$(LIBOBJ)
	$(AR) $(LIB) $(LIBOBJ)
	sh $(TOP)/util/ranlib.sh $(LIB)
	@touch lib

# elf
asm/bn86-elf.o: asm/bn86unix.cpp
	$(CPP) -DELF asm/bn86unix.cpp | as -o asm/bn86-elf.o

asm/co86-elf.o: asm/co86unix.cpp
	$(CPP) -DELF asm/co86unix.cpp | as -o asm/co86-elf.o

# solaris
asm/bn86-sol.o: asm/bn86unix.cpp
	$(CC) -E -DSOL asm/bn86unix.cpp | sed 's/^#.*//' > asm/bn86-sol.s
	as -o asm/bn86-sol.o asm/bn86-sol.s
	rm -f asm/bn86-sol.s

asm/co86-sol.o: asm/co86unix.cpp
	$(CC) -E -DSOL asm/co86unix.cpp | sed 's/^#.*//' > asm/co86-sol.s
	as -o asm/co86-sol.o asm/co86-sol.s
	rm -f asm/co86-sol.s

# a.out
asm/bn86-out.o: asm/bn86unix.cpp
	$(CPP) -DOUT asm/bn86unix.cpp | as -o asm/bn86-out.o

asm/co86-out.o: asm/co86unix.cpp
	$(CPP) -DOUT asm/co86unix.cpp | as -o asm/co86-out.o

# bsdi
asm/bn86bsdi.o: asm/bn86unix.cpp
	$(CPP) -DBSDI asm/bn86unix.cpp | sed 's/ :/:/' | as -o asm/bn86bsdi.o

asm/co86bsdi.o: asm/co86unix.cpp
	$(CPP) -DBSDI asm/co86unix.cpp | sed 's/ :/:/' | as -o asm/co86bsdi.o

asm/bn86unix.cpp:
	(cd asm; perl bn-586.pl cpp >bn86unix.cpp )

asm/co86unix.cpp:
	(cd asm; perl co-586.pl cpp >co86unix.cpp )

# MIPS 64 bit assember 
asm/mips3.o: asm/mips3.s
	/usr/bin/as -mips3 -O2 -o asm/mips3.o asm/mips3.s            

# MIPS 32 bit assember
asm/mips1.o: asm/mips1.s
	/usr/bin/as -O2 -o asm/mips1.o asm/mips1.s

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

exptest:
	/bin/rm -f exptest
	gcc -I../../include -g2 -ggdb -o exptest exptest.c ../../libcrypto.a

div:
	/bin/rm -f a.out
	gcc -I.. -g div.c ../../libcrypto.a

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
	/bin/rm -f *.o */*.o *.obj lib tags core .pure .nfs* *.old *.bak fluff bn_asm.s

errors:
	perl $(TOP)/util/err-ins.pl $(ERR).err $(ERR).org # special case .org
	perl $(TOP)/util/err-ins.pl $(ERR).err $(ERR).h
	perl ../err/err_genc.pl -s $(ERR).h $(ERRC).c

# DO NOT DELETE THIS LINE -- make depend depends on it.

bn_add.o: /usr/include/stdio.h /usr/include/libio.h /usr/include/features.h
bn_add.o: /usr/include/sys/cdefs.h /usr/include/gnu/stubs.h
bn_add.o: /usr/include/_G_config.h /usr/include/gnu/types.h
bn_add.o: /usr/lib/gcc-lib/i386-redhat-linux/2.7.2.3/include/stddef.h
bn_add.o: /usr/lib/gcc-lib/i386-redhat-linux/2.7.2.3/include/stdarg.h
bn_add.o: /usr/include/stdio_lim.h ../cryptlib.h /usr/include/stdlib.h
bn_add.o: /usr/include/sys/types.h /usr/include/time.h /usr/include/endian.h
bn_add.o: /usr/include/bytesex.h /usr/include/sys/select.h
bn_add.o: /usr/include/selectbits.h /usr/include/alloca.h
bn_add.o: /usr/include/string.h ../../include/e_os.h /usr/include/unistd.h
bn_add.o: /usr/include/posix_opt.h /usr/include/confname.h ../crypto.h
bn_add.o: ../../include/stack.h ../../include/buffer.h ../../include/bio.h
bn_add.o: ../../include/err.h /usr/include/errno.h /usr/include/errnos.h
bn_add.o: /usr/include/linux/errno.h /usr/include/asm/errno.h bn_lcl.h bn.h
bn_div.o: /usr/include/stdio.h /usr/include/libio.h /usr/include/features.h
bn_div.o: /usr/include/sys/cdefs.h /usr/include/gnu/stubs.h
bn_div.o: /usr/include/_G_config.h /usr/include/gnu/types.h
bn_div.o: /usr/lib/gcc-lib/i386-redhat-linux/2.7.2.3/include/stddef.h
bn_div.o: /usr/lib/gcc-lib/i386-redhat-linux/2.7.2.3/include/stdarg.h
bn_div.o: /usr/include/stdio_lim.h ../cryptlib.h /usr/include/stdlib.h
bn_div.o: /usr/include/sys/types.h /usr/include/time.h /usr/include/endian.h
bn_div.o: /usr/include/bytesex.h /usr/include/sys/select.h
bn_div.o: /usr/include/selectbits.h /usr/include/alloca.h
bn_div.o: /usr/include/string.h ../../include/e_os.h /usr/include/unistd.h
bn_div.o: /usr/include/posix_opt.h /usr/include/confname.h ../crypto.h
bn_div.o: ../../include/stack.h ../../include/buffer.h ../../include/bio.h
bn_div.o: ../../include/err.h /usr/include/errno.h /usr/include/errnos.h
bn_div.o: /usr/include/linux/errno.h /usr/include/asm/errno.h bn_lcl.h bn.h
bn_exp.o: /usr/include/stdio.h /usr/include/libio.h /usr/include/features.h
bn_exp.o: /usr/include/sys/cdefs.h /usr/include/gnu/stubs.h
bn_exp.o: /usr/include/_G_config.h /usr/include/gnu/types.h
bn_exp.o: /usr/lib/gcc-lib/i386-redhat-linux/2.7.2.3/include/stddef.h
bn_exp.o: /usr/lib/gcc-lib/i386-redhat-linux/2.7.2.3/include/stdarg.h
bn_exp.o: /usr/include/stdio_lim.h ../cryptlib.h /usr/include/stdlib.h
bn_exp.o: /usr/include/sys/types.h /usr/include/time.h /usr/include/endian.h
bn_exp.o: /usr/include/bytesex.h /usr/include/sys/select.h
bn_exp.o: /usr/include/selectbits.h /usr/include/alloca.h
bn_exp.o: /usr/include/string.h ../../include/e_os.h /usr/include/unistd.h
bn_exp.o: /usr/include/posix_opt.h /usr/include/confname.h ../crypto.h
bn_exp.o: ../../include/stack.h ../../include/buffer.h ../../include/bio.h
bn_exp.o: ../../include/err.h /usr/include/errno.h /usr/include/errnos.h
bn_exp.o: /usr/include/linux/errno.h /usr/include/asm/errno.h bn_lcl.h bn.h
bn_lib.o: /usr/include/stdio.h /usr/include/libio.h /usr/include/features.h
bn_lib.o: /usr/include/sys/cdefs.h /usr/include/gnu/stubs.h
bn_lib.o: /usr/include/_G_config.h /usr/include/gnu/types.h
bn_lib.o: /usr/lib/gcc-lib/i386-redhat-linux/2.7.2.3/include/stddef.h
bn_lib.o: /usr/lib/gcc-lib/i386-redhat-linux/2.7.2.3/include/stdarg.h
bn_lib.o: /usr/include/stdio_lim.h ../cryptlib.h /usr/include/stdlib.h
bn_lib.o: /usr/include/sys/types.h /usr/include/time.h /usr/include/endian.h
bn_lib.o: /usr/include/bytesex.h /usr/include/sys/select.h
bn_lib.o: /usr/include/selectbits.h /usr/include/alloca.h
bn_lib.o: /usr/include/string.h ../../include/e_os.h /usr/include/unistd.h
bn_lib.o: /usr/include/posix_opt.h /usr/include/confname.h ../crypto.h
bn_lib.o: ../../include/stack.h ../../include/buffer.h ../../include/bio.h
bn_lib.o: ../../include/err.h /usr/include/errno.h /usr/include/errnos.h
bn_lib.o: /usr/include/linux/errno.h /usr/include/asm/errno.h bn_lcl.h bn.h
bn_mul.o: /usr/include/stdio.h /usr/include/libio.h /usr/include/features.h
bn_mul.o: /usr/include/sys/cdefs.h /usr/include/gnu/stubs.h
bn_mul.o: /usr/include/_G_config.h /usr/include/gnu/types.h
bn_mul.o: /usr/lib/gcc-lib/i386-redhat-linux/2.7.2.3/include/stddef.h
bn_mul.o: /usr/lib/gcc-lib/i386-redhat-linux/2.7.2.3/include/stdarg.h
bn_mul.o: /usr/include/stdio_lim.h ../cryptlib.h /usr/include/stdlib.h
bn_mul.o: /usr/include/sys/types.h /usr/include/time.h /usr/include/endian.h
bn_mul.o: /usr/include/bytesex.h /usr/include/sys/select.h
bn_mul.o: /usr/include/selectbits.h /usr/include/alloca.h
bn_mul.o: /usr/include/string.h ../../include/e_os.h /usr/include/unistd.h
bn_mul.o: /usr/include/posix_opt.h /usr/include/confname.h ../crypto.h
bn_mul.o: ../../include/stack.h ../../include/buffer.h ../../include/bio.h
bn_mul.o: ../../include/err.h /usr/include/errno.h /usr/include/errnos.h
bn_mul.o: /usr/include/linux/errno.h /usr/include/asm/errno.h bn_lcl.h bn.h
bn_print.o: /usr/include/stdio.h /usr/include/libio.h /usr/include/features.h
bn_print.o: /usr/include/sys/cdefs.h /usr/include/gnu/stubs.h
bn_print.o: /usr/include/_G_config.h /usr/include/gnu/types.h
bn_print.o: /usr/lib/gcc-lib/i386-redhat-linux/2.7.2.3/include/stddef.h
bn_print.o: /usr/lib/gcc-lib/i386-redhat-linux/2.7.2.3/include/stdarg.h
bn_print.o: /usr/include/stdio_lim.h /usr/include/ctype.h
bn_print.o: /usr/include/endian.h /usr/include/bytesex.h ../cryptlib.h
bn_print.o: /usr/include/stdlib.h /usr/include/sys/types.h
bn_print.o: /usr/include/time.h /usr/include/sys/select.h
bn_print.o: /usr/include/selectbits.h /usr/include/alloca.h
bn_print.o: /usr/include/string.h ../../include/e_os.h /usr/include/unistd.h
bn_print.o: /usr/include/posix_opt.h /usr/include/confname.h ../crypto.h
bn_print.o: ../../include/stack.h ../../include/buffer.h ../../include/bio.h
bn_print.o: ../../include/err.h /usr/include/errno.h /usr/include/errnos.h
bn_print.o: /usr/include/linux/errno.h /usr/include/asm/errno.h bn_lcl.h bn.h
bn_rand.o: /usr/include/stdio.h /usr/include/libio.h /usr/include/features.h
bn_rand.o: /usr/include/sys/cdefs.h /usr/include/gnu/stubs.h
bn_rand.o: /usr/include/_G_config.h /usr/include/gnu/types.h
bn_rand.o: /usr/lib/gcc-lib/i386-redhat-linux/2.7.2.3/include/stddef.h
bn_rand.o: /usr/lib/gcc-lib/i386-redhat-linux/2.7.2.3/include/stdarg.h
bn_rand.o: /usr/include/stdio_lim.h /usr/include/time.h ../cryptlib.h
bn_rand.o: /usr/include/stdlib.h /usr/include/sys/types.h
bn_rand.o: /usr/include/endian.h /usr/include/bytesex.h
bn_rand.o: /usr/include/sys/select.h /usr/include/selectbits.h
bn_rand.o: /usr/include/alloca.h /usr/include/string.h ../../include/e_os.h
bn_rand.o: /usr/include/unistd.h /usr/include/posix_opt.h
bn_rand.o: /usr/include/confname.h ../crypto.h ../../include/stack.h
bn_rand.o: ../../include/buffer.h ../../include/bio.h ../../include/err.h
bn_rand.o: /usr/include/errno.h /usr/include/errnos.h
bn_rand.o: /usr/include/linux/errno.h /usr/include/asm/errno.h bn_lcl.h bn.h
bn_rand.o: ../../include/rand.h
bn_shift.o: /usr/include/stdio.h /usr/include/libio.h /usr/include/features.h
bn_shift.o: /usr/include/sys/cdefs.h /usr/include/gnu/stubs.h
bn_shift.o: /usr/include/_G_config.h /usr/include/gnu/types.h
bn_shift.o: /usr/lib/gcc-lib/i386-redhat-linux/2.7.2.3/include/stddef.h
bn_shift.o: /usr/lib/gcc-lib/i386-redhat-linux/2.7.2.3/include/stdarg.h
bn_shift.o: /usr/include/stdio_lim.h ../cryptlib.h /usr/include/stdlib.h
bn_shift.o: /usr/include/sys/types.h /usr/include/time.h
bn_shift.o: /usr/include/endian.h /usr/include/bytesex.h
bn_shift.o: /usr/include/sys/select.h /usr/include/selectbits.h
bn_shift.o: /usr/include/alloca.h /usr/include/string.h ../../include/e_os.h
bn_shift.o: /usr/include/unistd.h /usr/include/posix_opt.h
bn_shift.o: /usr/include/confname.h ../crypto.h ../../include/stack.h
bn_shift.o: ../../include/buffer.h ../../include/bio.h ../../include/err.h
bn_shift.o: /usr/include/errno.h /usr/include/errnos.h
bn_shift.o: /usr/include/linux/errno.h /usr/include/asm/errno.h bn_lcl.h bn.h
bn_word.o: /usr/include/stdio.h /usr/include/libio.h /usr/include/features.h
bn_word.o: /usr/include/sys/cdefs.h /usr/include/gnu/stubs.h
bn_word.o: /usr/include/_G_config.h /usr/include/gnu/types.h
bn_word.o: /usr/lib/gcc-lib/i386-redhat-linux/2.7.2.3/include/stddef.h
bn_word.o: /usr/lib/gcc-lib/i386-redhat-linux/2.7.2.3/include/stdarg.h
bn_word.o: /usr/include/stdio_lim.h ../cryptlib.h /usr/include/stdlib.h
bn_word.o: /usr/include/sys/types.h /usr/include/time.h /usr/include/endian.h
bn_word.o: /usr/include/bytesex.h /usr/include/sys/select.h
bn_word.o: /usr/include/selectbits.h /usr/include/alloca.h
bn_word.o: /usr/include/string.h ../../include/e_os.h /usr/include/unistd.h
bn_word.o: /usr/include/posix_opt.h /usr/include/confname.h ../crypto.h
bn_word.o: ../../include/stack.h ../../include/buffer.h ../../include/bio.h
bn_word.o: ../../include/err.h /usr/include/errno.h /usr/include/errnos.h
bn_word.o: /usr/include/linux/errno.h /usr/include/asm/errno.h bn_lcl.h bn.h
bn_blind.o: /usr/include/stdio.h /usr/include/libio.h /usr/include/features.h
bn_blind.o: /usr/include/sys/cdefs.h /usr/include/gnu/stubs.h
bn_blind.o: /usr/include/_G_config.h /usr/include/gnu/types.h
bn_blind.o: /usr/lib/gcc-lib/i386-redhat-linux/2.7.2.3/include/stddef.h
bn_blind.o: /usr/lib/gcc-lib/i386-redhat-linux/2.7.2.3/include/stdarg.h
bn_blind.o: /usr/include/stdio_lim.h ../cryptlib.h /usr/include/stdlib.h
bn_blind.o: /usr/include/sys/types.h /usr/include/time.h
bn_blind.o: /usr/include/endian.h /usr/include/bytesex.h
bn_blind.o: /usr/include/sys/select.h /usr/include/selectbits.h
bn_blind.o: /usr/include/alloca.h /usr/include/string.h ../../include/e_os.h
bn_blind.o: /usr/include/unistd.h /usr/include/posix_opt.h
bn_blind.o: /usr/include/confname.h ../crypto.h ../../include/stack.h
bn_blind.o: ../../include/buffer.h ../../include/bio.h ../../include/err.h
bn_blind.o: /usr/include/errno.h /usr/include/errnos.h
bn_blind.o: /usr/include/linux/errno.h /usr/include/asm/errno.h bn_lcl.h bn.h
bn_gcd.o: /usr/include/stdio.h /usr/include/libio.h /usr/include/features.h
bn_gcd.o: /usr/include/sys/cdefs.h /usr/include/gnu/stubs.h
bn_gcd.o: /usr/include/_G_config.h /usr/include/gnu/types.h
bn_gcd.o: /usr/lib/gcc-lib/i386-redhat-linux/2.7.2.3/include/stddef.h
bn_gcd.o: /usr/lib/gcc-lib/i386-redhat-linux/2.7.2.3/include/stdarg.h
bn_gcd.o: /usr/include/stdio_lim.h ../cryptlib.h /usr/include/stdlib.h
bn_gcd.o: /usr/include/sys/types.h /usr/include/time.h /usr/include/endian.h
bn_gcd.o: /usr/include/bytesex.h /usr/include/sys/select.h
bn_gcd.o: /usr/include/selectbits.h /usr/include/alloca.h
bn_gcd.o: /usr/include/string.h ../../include/e_os.h /usr/include/unistd.h
bn_gcd.o: /usr/include/posix_opt.h /usr/include/confname.h ../crypto.h
bn_gcd.o: ../../include/stack.h ../../include/buffer.h ../../include/bio.h
bn_gcd.o: ../../include/err.h /usr/include/errno.h /usr/include/errnos.h
bn_gcd.o: /usr/include/linux/errno.h /usr/include/asm/errno.h bn_lcl.h bn.h
bn_prime.o: /usr/include/stdio.h /usr/include/libio.h /usr/include/features.h
bn_prime.o: /usr/include/sys/cdefs.h /usr/include/gnu/stubs.h
bn_prime.o: /usr/include/_G_config.h /usr/include/gnu/types.h
bn_prime.o: /usr/lib/gcc-lib/i386-redhat-linux/2.7.2.3/include/stddef.h
bn_prime.o: /usr/lib/gcc-lib/i386-redhat-linux/2.7.2.3/include/stdarg.h
bn_prime.o: /usr/include/stdio_lim.h /usr/include/time.h ../cryptlib.h
bn_prime.o: /usr/include/stdlib.h /usr/include/sys/types.h
bn_prime.o: /usr/include/endian.h /usr/include/bytesex.h
bn_prime.o: /usr/include/sys/select.h /usr/include/selectbits.h
bn_prime.o: /usr/include/alloca.h /usr/include/string.h ../../include/e_os.h
bn_prime.o: /usr/include/unistd.h /usr/include/posix_opt.h
bn_prime.o: /usr/include/confname.h ../crypto.h ../../include/stack.h
bn_prime.o: ../../include/buffer.h ../../include/bio.h ../../include/err.h
bn_prime.o: /usr/include/errno.h /usr/include/errnos.h
bn_prime.o: /usr/include/linux/errno.h /usr/include/asm/errno.h bn_lcl.h bn.h
bn_prime.o: ../../include/rand.h bn_prime.h
bn_err.o: /usr/include/stdio.h /usr/include/libio.h /usr/include/features.h
bn_err.o: /usr/include/sys/cdefs.h /usr/include/gnu/stubs.h
bn_err.o: /usr/include/_G_config.h /usr/include/gnu/types.h
bn_err.o: /usr/lib/gcc-lib/i386-redhat-linux/2.7.2.3/include/stddef.h
bn_err.o: /usr/lib/gcc-lib/i386-redhat-linux/2.7.2.3/include/stdarg.h
bn_err.o: /usr/include/stdio_lim.h ../../include/err.h /usr/include/errno.h
bn_err.o: /usr/include/errnos.h /usr/include/linux/errno.h
bn_err.o: /usr/include/asm/errno.h bn.h
bn_sqr.o: /usr/include/stdio.h /usr/include/libio.h /usr/include/features.h
bn_sqr.o: /usr/include/sys/cdefs.h /usr/include/gnu/stubs.h
bn_sqr.o: /usr/include/_G_config.h /usr/include/gnu/types.h
bn_sqr.o: /usr/lib/gcc-lib/i386-redhat-linux/2.7.2.3/include/stddef.h
bn_sqr.o: /usr/lib/gcc-lib/i386-redhat-linux/2.7.2.3/include/stdarg.h
bn_sqr.o: /usr/include/stdio_lim.h ../cryptlib.h /usr/include/stdlib.h
bn_sqr.o: /usr/include/sys/types.h /usr/include/time.h /usr/include/endian.h
bn_sqr.o: /usr/include/bytesex.h /usr/include/sys/select.h
bn_sqr.o: /usr/include/selectbits.h /usr/include/alloca.h
bn_sqr.o: /usr/include/string.h ../../include/e_os.h /usr/include/unistd.h
bn_sqr.o: /usr/include/posix_opt.h /usr/include/confname.h ../crypto.h
bn_sqr.o: ../../include/stack.h ../../include/buffer.h ../../include/bio.h
bn_sqr.o: ../../include/err.h /usr/include/errno.h /usr/include/errnos.h
bn_sqr.o: /usr/include/linux/errno.h /usr/include/asm/errno.h bn_lcl.h bn.h
bn_asm.o: /usr/include/stdio.h /usr/include/libio.h /usr/include/features.h
bn_asm.o: /usr/include/sys/cdefs.h /usr/include/gnu/stubs.h
bn_asm.o: /usr/include/_G_config.h /usr/include/gnu/types.h
bn_asm.o: /usr/lib/gcc-lib/i386-redhat-linux/2.7.2.3/include/stddef.h
bn_asm.o: /usr/lib/gcc-lib/i386-redhat-linux/2.7.2.3/include/stdarg.h
bn_asm.o: /usr/include/stdio_lim.h ../cryptlib.h /usr/include/stdlib.h
bn_asm.o: /usr/include/sys/types.h /usr/include/time.h /usr/include/endian.h
bn_asm.o: /usr/include/bytesex.h /usr/include/sys/select.h
bn_asm.o: /usr/include/selectbits.h /usr/include/alloca.h
bn_asm.o: /usr/include/string.h ../../include/e_os.h /usr/include/unistd.h
bn_asm.o: /usr/include/posix_opt.h /usr/include/confname.h ../crypto.h
bn_asm.o: ../../include/stack.h ../../include/buffer.h ../../include/bio.h
bn_asm.o: ../../include/err.h /usr/include/errno.h /usr/include/errnos.h
bn_asm.o: /usr/include/linux/errno.h /usr/include/asm/errno.h bn_lcl.h bn.h
bn_recp.o: /usr/include/stdio.h /usr/include/libio.h /usr/include/features.h
bn_recp.o: /usr/include/sys/cdefs.h /usr/include/gnu/stubs.h
bn_recp.o: /usr/include/_G_config.h /usr/include/gnu/types.h
bn_recp.o: /usr/lib/gcc-lib/i386-redhat-linux/2.7.2.3/include/stddef.h
bn_recp.o: /usr/lib/gcc-lib/i386-redhat-linux/2.7.2.3/include/stdarg.h
bn_recp.o: /usr/include/stdio_lim.h ../cryptlib.h /usr/include/stdlib.h
bn_recp.o: /usr/include/sys/types.h /usr/include/time.h /usr/include/endian.h
bn_recp.o: /usr/include/bytesex.h /usr/include/sys/select.h
bn_recp.o: /usr/include/selectbits.h /usr/include/alloca.h
bn_recp.o: /usr/include/string.h ../../include/e_os.h /usr/include/unistd.h
bn_recp.o: /usr/include/posix_opt.h /usr/include/confname.h ../crypto.h
bn_recp.o: ../../include/stack.h ../../include/buffer.h ../../include/bio.h
bn_recp.o: ../../include/err.h /usr/include/errno.h /usr/include/errnos.h
bn_recp.o: /usr/include/linux/errno.h /usr/include/asm/errno.h bn_lcl.h bn.h
bn_mont.o: /usr/include/stdio.h /usr/include/libio.h /usr/include/features.h
bn_mont.o: /usr/include/sys/cdefs.h /usr/include/gnu/stubs.h
bn_mont.o: /usr/include/_G_config.h /usr/include/gnu/types.h
bn_mont.o: /usr/lib/gcc-lib/i386-redhat-linux/2.7.2.3/include/stddef.h
bn_mont.o: /usr/lib/gcc-lib/i386-redhat-linux/2.7.2.3/include/stdarg.h
bn_mont.o: /usr/include/stdio_lim.h ../cryptlib.h /usr/include/stdlib.h
bn_mont.o: /usr/include/sys/types.h /usr/include/time.h /usr/include/endian.h
bn_mont.o: /usr/include/bytesex.h /usr/include/sys/select.h
bn_mont.o: /usr/include/selectbits.h /usr/include/alloca.h
bn_mont.o: /usr/include/string.h ../../include/e_os.h /usr/include/unistd.h
bn_mont.o: /usr/include/posix_opt.h /usr/include/confname.h ../crypto.h
bn_mont.o: ../../include/stack.h ../../include/buffer.h ../../include/bio.h
bn_mont.o: ../../include/err.h /usr/include/errno.h /usr/include/errnos.h
bn_mont.o: /usr/include/linux/errno.h /usr/include/asm/errno.h bn_lcl.h bn.h
bn_mpi.o: /usr/include/stdio.h /usr/include/libio.h /usr/include/features.h
bn_mpi.o: /usr/include/sys/cdefs.h /usr/include/gnu/stubs.h
bn_mpi.o: /usr/include/_G_config.h /usr/include/gnu/types.h
bn_mpi.o: /usr/lib/gcc-lib/i386-redhat-linux/2.7.2.3/include/stddef.h
bn_mpi.o: /usr/lib/gcc-lib/i386-redhat-linux/2.7.2.3/include/stdarg.h
bn_mpi.o: /usr/include/stdio_lim.h ../cryptlib.h /usr/include/stdlib.h
bn_mpi.o: /usr/include/sys/types.h /usr/include/time.h /usr/include/endian.h
bn_mpi.o: /usr/include/bytesex.h /usr/include/sys/select.h
bn_mpi.o: /usr/include/selectbits.h /usr/include/alloca.h
bn_mpi.o: /usr/include/string.h ../../include/e_os.h /usr/include/unistd.h
bn_mpi.o: /usr/include/posix_opt.h /usr/include/confname.h ../crypto.h
bn_mpi.o: ../../include/stack.h ../../include/buffer.h ../../include/bio.h
bn_mpi.o: ../../include/err.h /usr/include/errno.h /usr/include/errnos.h
bn_mpi.o: /usr/include/linux/errno.h /usr/include/asm/errno.h bn_lcl.h bn.h
bn_exp2.o: /usr/include/stdio.h /usr/include/libio.h /usr/include/features.h
bn_exp2.o: /usr/include/sys/cdefs.h /usr/include/gnu/stubs.h
bn_exp2.o: /usr/include/_G_config.h /usr/include/gnu/types.h
bn_exp2.o: /usr/lib/gcc-lib/i386-redhat-linux/2.7.2.3/include/stddef.h
bn_exp2.o: /usr/lib/gcc-lib/i386-redhat-linux/2.7.2.3/include/stdarg.h
bn_exp2.o: /usr/include/stdio_lim.h ../cryptlib.h /usr/include/stdlib.h
bn_exp2.o: /usr/include/sys/types.h /usr/include/time.h /usr/include/endian.h
bn_exp2.o: /usr/include/bytesex.h /usr/include/sys/select.h
bn_exp2.o: /usr/include/selectbits.h /usr/include/alloca.h
bn_exp2.o: /usr/include/string.h ../../include/e_os.h /usr/include/unistd.h
bn_exp2.o: /usr/include/posix_opt.h /usr/include/confname.h ../crypto.h
bn_exp2.o: ../../include/stack.h ../../include/buffer.h ../../include/bio.h
bn_exp2.o: ../../include/err.h /usr/include/errno.h /usr/include/errnos.h
bn_exp2.o: /usr/include/linux/errno.h /usr/include/asm/errno.h bn_lcl.h bn.h
