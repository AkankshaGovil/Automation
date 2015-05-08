##############################################################
#### 		Configuration Makefile
##############################################################

PROJECT_NAME = h323
OBJECTS = ci.o cibuf.o cifile.o cisupp.o
TARGET = conf
PROGFLAGS = -I../ads -I../utils -I../lowunix -I../log -I../defs
DISTFILES =
PROGS =
PROGLIBS =
PROGDEFS = -DBSD_COMP
#PROGDEFS = -DBSD_COMP -D__LINK_CONFIG_AND_RAW__

PROJECT_BASE := $(word 1, $(subst $(PROJECT_NAME)/,$(PROJECT_NAME) ,$(shell pwd)))
include $(PROJECT_BASE)/../make/unix/common.mk
LIBTARGET = $(libdir)/lib$(TARGET).a
all: $(LIBTARGET)
include $(mkdir)/lib.mk
