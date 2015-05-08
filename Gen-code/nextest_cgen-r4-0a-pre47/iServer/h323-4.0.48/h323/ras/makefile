##############################################################
#### 		Conference Manager  Makefile
##############################################################

PROJECT_NAME = h323
OBJECTS   = 	cmiras.o 	\
		cmras.o		\
		rasdecoder.o	\
		rasin.o		\
		rasirr.o	\
		rasout.o	\
		rasparams.o	\
		rasutils.o

TARGET    = ras
PROGLIBS  = 
PROGFLAGS = -I. -I../ads -I../pvt_pst -I../lowunix -I../coder -I../defs \
	-I../cm -I../config -I../log -I../cat -I../utils -I../h245 \
        -I../transport -I../asn_objects -I../mib
DISTFILES =
PROGDEFS  = 


PROJECT_BASE := $(word 1, $(subst $(PROJECT_NAME)/,$(PROJECT_NAME) ,$(shell pwd)))
include $(PROJECT_BASE)/../make/unix/common.mk
LIBTARGET = $(libdir)/lib$(TARGET).a
all: $(LIBTARGET)
include $(mkdir)/lib.mk
