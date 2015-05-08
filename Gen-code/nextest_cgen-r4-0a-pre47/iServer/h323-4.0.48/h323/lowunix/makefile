##############################################################
#### 		LOWUNIX Makefile
##############################################################

PROJECT_NAME = h323
OBJECTS = 	iplist.o \
		li.o \
		logging.o \
		mei.o \
		msg.o \
		mti.o \
		pi.o \
		seli.o \
                sigcatch.o \
		termin.o \
		Threads.o

TARGET = low
PROGFLAGS = -I../ads -I../log -I../utils -I../defs
DISTFILES =
PROGS =
PROGLIBS =
PROGDEFS = 


PROJECT_BASE := $(word 1, $(subst $(PROJECT_NAME)/,$(PROJECT_NAME) ,$(shell pwd)))

include $(PROJECT_BASE)/../make/unix/common.mk
LIBTARGET = $(libdir)/lib$(TARGET).a
all: $(LIBTARGET)
include $(mkdir)/lib.mk
