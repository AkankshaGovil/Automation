
##
## Makefile for TAVL library.
## Gagan Bajpai , 06/2004.
##
##
include $(BASE)/global.mk

LIB = libtavl.a

DESTDIR = ../lib

OBJS =  tavlinit.o \
	tavl_ins.o \
	taavlrebl.o \
	tavlfind.o \
	tavldall.o \
	tavl_del.o \
	tavlfree.o \
	tavl_gdtc.o \
	tavlpred.o \
	tavl_rst.o \
	tavl_sdt.o \
	tavlsucc.o

ALLOBJS = $(OBJS)


all:	lib

lib install: $(LIBTAVL)

$(LIBTAVL):	$(ALLOBJS)
	$(AR) ruv $@ $^


## Administrative targets.

clean:	
	rm -f $(ALLOBJS) $(LIBTAVL)


## DO NOT DELETE
