##############################################################
#### 		ADS Makefile
##############################################################

PROJECT_NAME = h323
OBJECTS = statistic.o cmmib.o
TARGET = mib
PROGFLAGS = -I../defs -I../ads -I../pvt_pst -I../utils -I../log -I../lowunix -I../coder -I../cm -I../config -I../ras -I../h245 -I../mib -I../cat -I../transport -I../asn_objects
DISTFILES =
PROGS =
PROGLIBS =
PROGDEFS = 


PROJECT_BASE := $(word 1, $(subst $(PROJECT_NAME)/,$(PROJECT_NAME) ,$(shell pwd)))
include $(PROJECT_BASE)/../make/unix/common.mk
LIBTARGET = $(libdir)/lib$(TARGET).a
all: $(LIBTARGET)
include $(mkdir)/lib.mk
