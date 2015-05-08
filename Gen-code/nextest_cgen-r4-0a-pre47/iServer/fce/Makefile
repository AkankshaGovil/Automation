#################################################################
#################################################################
##
##			Makefile
##
#################################################################
#################################################################

include $(BASE)/global.mk


CFLAGS += -Wall -I../xml/xmlparse -I../xml/xmlwf 
TEJA_CFLAGS = -Wall -ggdb -c  -D_REENTRANT \
              -D_POSIX_PTHREAD_SEMANTICS -DTEJA_DEPRECATED_API_ON \
              -DTEJA_DOUBLE -DL2_TYPE=L2_ETH -DCLI_APP \
              -DTEJA_DEBUG -DTEJA_ARG_CHECKING=1
TEJA_OBJS = teja_cli.o teja_cli_app.o teja_cli_ui.o hknife_cli.o
HKNIFE_OBJS = $(TEJA_OBJS)

ifeq ("X$(PLATFORM)", "Xlinux")
OBJS=	hknifeglue.o
else
OBJS=	nsfglue.o
endif

OBJS += \
	fceutils.o \
	firewallcontrol.o \
	poolalloc.o \
	portalloc.o \
	mfcp.o \
	fclocal.o \
	lexor.o \
	$(HKNIFE_OBJS) 

TESTLIBS =  $(LIBMEM) $(LIBDB) $(LIBUTILS) $(LIBSRVR) $(LIBUTIL) $(LIBCOMMON) $(LIBOOSD)\
	$(LIBCRYPTO) $(LIBXML) $(LIBSCONFIG) $(LIBTAVL) $(LIBAVL) $(LIBSLOCKS) $(LIBENUM) \
    $(LIBFCE)  $(LIBLOCK) $(LIBTST) $(LIBGISRPC) $(LIBRADIUS) \
    $(LIBRADC) $(LIBEXECD) $(LIBXML) $(LIBSCONFIG) $(LIBFCE) $(LIBCOMMON) \
    $(LIBBRIDGE) $(LIBMEM) $(LIBUTILS) $(LIBTST) $(LIBCOMMON) $(LIBCRYPTO)  $(LIBTAVL) $(LIBAVL) $(LIBDB)  $(LIBSRVR) $(LIBSLOCKS)

TEST_PORT = portalloc


#CONFIGOBJS = fceconfig.o

TESTOBJ = test.o

LEXOR = lexor.c

lib install: $(LIBFCE)

$(LEXOR): lexor.l
	flex -Pmfcp -olexor.c ../lexor.l

$(LIBFCE): $(OBJS)
	ar ruv $@ $^
	$(CP) ../pools.xml $(BINDIR)

#$(LIBFCECONFIG): $(CONFIGOBJS)
#	ar ruv $@ $^

test: $(LIBFCE) test.o 
	$(CC) -o $@ test.o  $(CFLAGS) $(TESTLIBS) $(LDLIBS) -O2

mfcpd: $(LIBFCE) mfcpd.o
	$(CC) -o $@ mfcpd.o $(CFLAGS) $(TESTLIBS) $(LDLIBS) -O2

portalloc: portalloc.c
	$(CC) -O2 -o $@ -DTEST_PORT_ALLOC $(CFLAGS) $(TESTLIBS) $(BASE)/common/$(TARGET)/srvrlog.o ../portalloc.c 


$(TEJA_OBJS): $(TEJA_OBJS:%.o=%.c)
	$(CC) $(TEJA_CFLAGS) -I../../include -I../include -I../include/teja ../$(@:%.o=%.c)


clean:	
	$(RM) $(OBJS) $(LEXOR) $(CONFIGOBJS) $(LIBFCE) $(DLIB)/lipvnsp.a

depend:
	$(MAKEDEPEND) $(CFLAGS) *.c


# DO NOT DELETE











