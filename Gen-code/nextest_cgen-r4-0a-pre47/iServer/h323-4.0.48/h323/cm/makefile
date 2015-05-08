##############################################################
#### 		Conference Manager  Makefile
##############################################################

PROJECT_NAME = h323
OBJECTS   = 	callobj.o \
                cmCall.o \
	    	cmCallHighLevel.o \
		cmCallMessages.o \
		cmCallMPoint.o \
		cmcfgrt.o \
		cmConfig.o \
		cmControlTransport.o \
		cmCrossReference.o \
		cmdebprn.o \
		cmGeneral.o \
		cmhook.o \
		cmnew.o \
		cmparam.o \
		cmQ931.o \
		cmsize.o \
		stkutils.o \
		cmAutoRas.o \
		cmAutoRasCall.o \
		cmAutoRasEP.o \
		cmrasinit.o \
		cmutils.o \
		getcid.o


TARGET    = cm
PROGLIBS  = 
PROGFLAGS =  -I../h245 -I../pvt_pst -I../ads -I../coder -I../log -I../transport -I../lowunix -I../utils -I../asn_objects -I../config -I../cat -I../ras -I../mib -I../defs
DISTFILES =
PROGDEFS  = 


PROJECT_BASE := $(word 1, $(subst $(PROJECT_NAME)/,$(PROJECT_NAME) ,$(shell pwd)))
include $(PROJECT_BASE)/../make/unix/common.mk
LIBTARGET = $(libdir)/lib$(TARGET).a
all: $(LIBTARGET)
include $(mkdir)/lib.mk
