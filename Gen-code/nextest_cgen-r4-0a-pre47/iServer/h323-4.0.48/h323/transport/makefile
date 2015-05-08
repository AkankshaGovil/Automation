##############################################################
#### 		Middle layer Makefile
##############################################################

PROJECT_NAME = h323
OBJECTS = 	annexe.o	\
		iannerw.o	\
		ianneses.o	\
		iannetmr.o	\
		iannexe.o	\
		tpkt.o		\
		transnet.o	\
		transport.o	\
		transStates.o	\
		transutil.o

TARGET = trans
PROGFLAGS = -I../lowunix -I../ads -I../h245 -I../log -I../pvt_pst -I../coder -I../cm -I../config -I../cat -I../ras -I../asn_objects -I../utils -I../defs -I../mib
DISTFILES =
PROGS =
PROGLIBS =
PROGDEFS = 

PROJECT_BASE := $(word 1, $(subst $(PROJECT_NAME)/,$(PROJECT_NAME) ,$(shell pwd)))
include $(PROJECT_BASE)/../make/unix/common.mk
LIBTARGET = $(libdir)/lib$(TARGET).a
all: $(LIBTARGET)
include $(mkdir)/lib.mk
