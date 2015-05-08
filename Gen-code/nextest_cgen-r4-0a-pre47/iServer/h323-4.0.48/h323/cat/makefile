##############################################################
#### 		Conference Manager  Makefile
##############################################################

PROJECT_NAME = h323
OBJECTS   = cat.o
TARGET    = cat
PROGLIBS  = 
PROGFLAGS =  -I../cm/  -I../ads -I../coder -I../pvt_pst  -I../lowunix -I../utils -I../h245 -I../ras -I../config -I../log -I../defs

DISTFILES =
PROGDEFS  = 


PROJECT_BASE := $(word 1, $(subst $(PROJECT_NAME)/,$(PROJECT_NAME) ,$(shell pwd)))
include $(PROJECT_BASE)/../make/unix/common.mk
LIBTARGET = $(libdir)/lib$(TARGET).a
all: $(LIBTARGET)
include $(mkdir)/lib.mk
