
#################################################################
#################################################################
##
##			Makefile
##
#################################################################
#################################################################

include $(BASE)/global.mk

CFLAGS += -I../../h323/sse -I../../h323/supserve -g -DNO_LIC $(DBMALLOCINC) -O3 -DHAVE_SIGLONGJMP

ifeq ("X$(PLATFORM)", "Xlinux")
LIBS= -Wl,--start-group 
else
LIBS=
endif


LIBS += $(LIBSCONFIG) $(LIBFCE) $(LIBARAVOX) $(LIBMEM) $(LIBDB) $(LIBUTILS) $(LIBSRVR) $(LIBUTILS) $(LIBCOMMON) $(LIBOSD)\
	$(LIBCRYPTO) $(LIBXML) $(LIBTAVL) $(LIBAVL) $(LIBSLOCKS) $(LIBMEM) $(LIBXML) $(LIBTST) $(DBMALLOCLIB) 

LIBEXPECT = -ltclstub8.4 
ifeq ("X$(PLATFORM)", "Xlinux")
LIBS+= -Wl,--end-group 
LIBEXPECT+= -lexpect5.40
else
LIBEXPECT+= -lexpect5.32
endif

SIPLIBS= $(LIBSIPTSM) $(LIBSIPUA2) $(LIBSIPTSM) $(LIBSIPUA2) $(LIBSIP2) $(LIBSIPTSM) $(LIBSIPUA2) 

LIBSUPS=$(DLIB)/libsups.a
LIBSSE=$(DLIB)/libsse.a

all: gen sgen mgen

#gen: cgen.o build.o finwait.o ixia.o $(LIBH323) $(LIBCOMMON) $(LIBOSD)
gen: cgen.o build.o finwait.o ixia.o sgenstubs.o $(LIBH323) $(LIBCOMMON) $(LIBOSD)
	$(CC) -O3 $(LDFLAGS) -o $@ $^ $(LIBS) $(LDLIBS) $(LIBEXPECT)

cgen: cgen.o build.o $(LIBH323) $(LIBCOMMON) $(LIBOSD)
	$(CC) -O3 $(LDFLAGS) -o gen $^ $(LDLIBS) 

mgen: mgen.o vars.o $(LIBCOMMON) $(LIBOSD)
	$(CC) $(LDFLAGS) -o $@ $^ $(LIBS) $(LDLIBS) 

sgen: vars.o register.o invite.o uabridge.o sgen.o build.o sgenstubs.o ixia.o
	$(CC) -O3 $(LDFLAGS) -o $@ $(LIBSIPAPI) $(LIBSIPCORE) $^ $(SIPLIBS) \
	$(LIBSIPAPI) $(LIBSIPCORE) $(SIPLIBS) $(LIBS) $(LDLIBS) $(LIBEXPECT)

build.c: .BUILD
	printf "%s" "char buildDate[64]=\"" > build.c
	printf "%s"  `date +%m/%d/%Y` >> build.c
	printf "%s" "\";" >> build.c
	printf "%s"  "long hostid=0x" >> build.c
	printf "%s"  `/usr/bin/hostid|cut -d'x' -f2` >> build.c
	printf "%s\n"  ";" >> build.c

.o:	.c
	$(CC) $(CFLAGS) -o $@ $^

clean:	
	rm -f *.o gen sgen mgen

depend:
	$(MAKEDEPEND) $(CFLAGS) *.c

install: all
.BUILD:

# DO NOT DELETE
