##############################################################
#### 		Conference Manager  Makefile
##############################################################

PROJECT_NAME = h323
OBJECTS   = 	caputils.o		\
		cmchan.o 		\
		cmChanGetByXXX.o	\
		cmChannelsCollection.o	\
		cmChanOperations.o	\
		cmctrl.o		\
		cmCtrlCap.o		\
		cmCtrlMMode.o		\
		cmCtrlMpoint.o		\
		cmCtrlMSD.o		\
		cmCtrlRMode.o		\
		cmCtrlRTD.o		\
		conf.o			\
		faststart.o		\
		redencod.o		\
		transpcap.o		\
		userinput.o


TARGET    = h245
PROGLIBS  = 
PROGFLAGS =  -I../cm -I../pvt_pst -I../ads -I../coder -I../log -I../transport -I../lowunix -I../utils -I../asn_objects -I../config -I../cat -I../ras -I../mib -I../defs
DISTFILES =
PROGDEFS  = 


PROJECT_BASE := $(word 1, $(subst $(PROJECT_NAME)/,$(PROJECT_NAME) ,$(shell pwd)))
include $(PROJECT_BASE)/../make/unix/common.mk
LIBTARGET = $(libdir)/lib$(TARGET).a
all: $(LIBTARGET)
include $(mkdir)/lib.mk
