##############################################################
#### 		Example of Library Makefile
##############################################################

PROJECT_NAME = example
OBJECTS = ex1.o
TARGET = ex
PROGFLAGS = -I../ex1
DISTFILES =
PROGS =
PROGLIBS =
PROGDEFS = 


PROJECT_BASE := $(word 1, $(subst $(PROJECT_NAME)/,$(PROJECT_NAME) ,$(shell pwd)))
include $(PROJECT_BASE)/make/common.mk
LIBTARGET = $(libdir)/lib$(TARGET).a
all: $(LIBTARGET)
include $(PROJECT_BASE)/make/lib.mk
