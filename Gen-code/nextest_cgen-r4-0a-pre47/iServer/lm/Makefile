
##
## Makefile for DSP Driver.
## Sridhar , 03/98.
##
## Revisions for Linux, 07/13/98.

## Default project.
##
include $(BASE)/global.mk

ifeq ("X$(PLATFORM)", "Xlinux")
LIBSOCK =
else
LIBSOCK = -lsocket
endif


LIBS =  $(LIBCRYPTO)
LCONVLIBS = $(LIBCOMMON) $(LIBS) $(LIBXML)
LSTATLIBS =  $(LIBCRYPTO) $(LIBCOMMON) $(LIBOSD) $(LIBSRVR) $(LIBSCONFIG) $(LIBLOCK)

DESTDIR = ../bin

LGENOBJS = lm_lgen.o ex_data.o

LGENXOBJS = lm_lgenx.o ex_data.o

LCONVOBJS = lconv.o ex_data.o

LSTATOBJS = lstat.o\

all:	lgen lgenx lconv # lreadck

exec install:	lgen lgenx lconv
	cp lgen $(BINDIR)
	cp lgenx $(BINDIR)
	cp lconv $(BINDIR)

lconv:	$(LCONVOBJS) $(LCONVLIBS)
	$(CC) $(PROFFLAGS) $(LDFLAGS) -o $@ $^ $(SFIOLIBS) $(LIBSOCK) 

lgen :	$(LGENOBJS) $(LIBS) 
	$(CC) $(PROFFLAGS) $(LDFLAGS) -o $@ $^ $(SFIOLIBS)

lgenx :	$(LGENXOBJS) $(LCONVLIBS) 
	$(CC) $(PROFFLAGS) $(LDFLAGS) -o $@ $^ $(SFIOLIBS) $(LIBSOCK) 

#lstat :	$(LSTATOBJS) $(LSTATLIBS) -lsocket ../../common/$(TARGET)/lm_utils.o
lstat :	$(LSTATOBJS) $(LSTATLIBS)
	$(CC) $(PROFFLAGS) $(LDFLAGS) -o $@ $^ $(LSTATLIBS)

## Administrative targets.

clean:	
	rm -f $(LGENOBJS) $(LCONVOBJS) $(LGENXOBJS) lgen lgenx lconv

deps:
	makedepend $(CFLAGS) *.c


##
# DO NOT DELETE
