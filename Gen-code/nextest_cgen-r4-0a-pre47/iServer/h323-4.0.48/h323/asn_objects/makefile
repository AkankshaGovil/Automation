##############################################################
#### 		Conference Manager  Makefile
##############################################################

PROJECT_NAME = h323
OBJECTS   = q931asn1.o h245.o commonasn.o ciscoasn.o
TARGET    = asn
PROGLIBS  = 
PROGFLAGS =  -I../pvt_pst  -I../lowunix -I../defs

DISTFILES =
PROGDEFS  = 


PROJECT_BASE := $(word 1, $(subst $(PROJECT_NAME)/,$(PROJECT_NAME) ,$(shell pwd)))
include $(PROJECT_BASE)/../make/unix/common.mk
LIBTARGET = $(libdir)/lib$(TARGET).a
all: $(LIBTARGET)
include $(mkdir)/lib.mk
