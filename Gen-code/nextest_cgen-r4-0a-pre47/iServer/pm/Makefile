##
## Makefile for Process Manager 
##
## Default project.
##
include $(BASE)/global.mk


LIBS =  $(LIBSRVR) $(LIBSCONFIG) $(LIBCOMMON) $(LIBOSD) $(LIBLOCK) $(LIBCOMMON) $(LIBTST)

DESTDIR = ../bin

PMOBJS = pm.o\
		poll.o\
		udpserver.o


all:	pm	

exec install:	pm	
	cp pm $(BINDIR)
	cp ../pmcore $(BINDIR)
	cp ../core.cfg $(BINDIR)

pm:	$(PMOBJS) $(LIBS)
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $+  $(LDLIBS)


## Administrative targets.

clean:	
	rm -f $(PMOBJS)

deps:
	makedepend $(CFLAGS) *.c


##
# DO NOT DELETE
