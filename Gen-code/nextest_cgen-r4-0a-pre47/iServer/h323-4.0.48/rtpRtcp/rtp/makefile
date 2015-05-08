##############################################################
#### 		Conference Manager  Makefile
##############################################################

PROJECT_NAME = rtpRtcp
OBJECTS   = bitfield.o buffer.o payload.o rtcp.o rtp.o
TARGET    = rtp
PROGLIBS  = 
PROGFLAGS =  -I../../h323/ads -I../../h323/defs -I../../h323/lowunix -I../../h323/log
DISTFILES =
PROGDEFS  = 


#PROJECT_BASE := $(word 1, $(subst $(PROJECT_NAME)/,$(PROJECT_NAME) ,$(shell pwd)))
include $(PROJECT_BASE)/../make/unix/common.mk
LIBTARGET = $(libdir)/lib$(TARGET).a
all: $(LIBTARGET)
include $(PROJECT_BASE)/../make/unix/lib.mk
