##
## Makefile for Process Manager 
##
## Default project.
##
include $(BASE)/global.mk

#CFLAGS += -O0
RANLIB	= ranlib

LIBS =  $(LIBSRVR) $(LIBSCONFIG) $(LIBCOMMON) $(LIBLOCK) $(LIBCOMMON) $(LIBOSD) $(LIBTST)

DESTDIR = $(BASE)/bin
LIBEXECD = libexecd.a
LIBEXECD_NAME = $(BASE)/lib/$(TARGET)/$(LIBEXECD)

LIBEXECD_OBJS = comm.o

EXECDOBJS = main.o

TESTOBJS = snd.o

all:	execd snd	

exec install:	execd	
	cp execd $(BINDIR)

lib:	$(LIBEXECD)

execd :	$(EXECDOBJS) $(LIBS)
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $+  $(LDLIBS)

snd:	$(TESTOBJS) $(LIBEXECD_NAME) $(LIBS)
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $+  $(LDLIBS)

libexecd.a:	$(LIBEXECD_OBJS)
			ar rv ${LIBEXECD_NAME} $?
			${RANLIB} ${LIBEXECD_NAME} 

## Administrative targets.
clean:	
	rm -f $(EXECDOBJS) $(TESTOBJS) $(LIBEXECD_OBJS) execd snd

deps:
	makedepend $(CFLAGS) *.c

##
# DO NOT DELETE
