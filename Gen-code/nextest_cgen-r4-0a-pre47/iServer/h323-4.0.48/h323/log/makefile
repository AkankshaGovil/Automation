##############################################################
#### 		Middle layer Makefile
##############################################################

PROJECT_NAME = h323
OBJECTS = msprint.o log.o
TARGET = log
PROGFLAGS = -I../lowunix -I../ads -I../defs
DISTFILES =
PROGS =
PROGLIBS =
PROGDEFS = 

PROJECT_BASE := $(word 1, $(subst $(PROJECT_NAME)/,$(PROJECT_NAME) ,$(shell pwd)))
include $(PROJECT_BASE)/../make/unix/common.mk
LIBTARGET = $(libdir)/lib$(TARGET).a
all: $(LIBTARGET)
include $(mkdir)/lib.mk
