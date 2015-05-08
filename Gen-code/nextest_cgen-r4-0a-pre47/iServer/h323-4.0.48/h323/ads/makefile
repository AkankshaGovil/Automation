##############################################################
#### 		ADS Makefile
##############################################################

PROJECT_NAME = h323
OBJECTS = 	bb.o \
		bheap.o \
	    	copybits.o \
	  	ema.o \
		etimer.o \
		hash.o \
		memfunc.o \
		mlist.o \
		ra.o \
		rid.o \
		rlist.o \
		rpool.o \
		rtree.o \
		timer.o \
		tls.o \
		token.o \
		ts.o

TARGET = ads
PROGFLAGS = -I../lowunix -I../log -I../utils -I../defs
DISTFILES =
PROGS =
PROGLIBS =
PROGDEFS = 


PROJECT_BASE := $(word 1, $(subst $(PROJECT_NAME)/,$(PROJECT_NAME) ,$(shell pwd)))

include $(PROJECT_BASE)/../make/unix/common.mk
LIBTARGET = $(libdir)/lib$(TARGET).a
all: $(LIBTARGET)
include $(mkdir)/lib.mk
