##############################################################
#### 		Middle layer Makefile
##############################################################

PROJECT_NAME = h323
OBJECTS = intutils.o prnutils.o oidutils.o netutl.o strutils.o
TARGET = utils
PROGFLAGS = -I../ads -I../defs -I../lowunix -I../log
DISTFILES =
PROGS =
PROGLIBS =
PROGDEFS = 

PROJECT_BASE := $(word 1, $(subst $(PROJECT_NAME)/,$(PROJECT_NAME) ,$(shell pwd)))
include $(PROJECT_BASE)/../make/unix/common.mk
LIBTARGET = $(libdir)/lib$(TARGET).a
all: $(LIBTARGET)
include $(mkdir)/lib.mk
