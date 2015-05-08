##############################################################
#### 		Conference Manager  Makefile
##############################################################

PROJECT_NAME = h323
OBJECTS   =  psyntree.o pvaltree.o psyntreeDb.o psyntreeStackApi.o pvaltreeStackApi.o

TARGET    = pvtpst
PROGLIBS  = 
PROGFLAGS =  -I../ads -I../log -I../lowunix -I../utils -I../defs
DISTFILES =
PROGDEFS  = 


PROJECT_BASE := $(word 1, $(subst $(PROJECT_NAME)/,$(PROJECT_NAME) ,$(shell pwd)))
include $(PROJECT_BASE)/../make/unix/common.mk
LIBTARGET = $(libdir)/lib$(TARGET).a
all: $(LIBTARGET)
include $(mkdir)/lib.mk
