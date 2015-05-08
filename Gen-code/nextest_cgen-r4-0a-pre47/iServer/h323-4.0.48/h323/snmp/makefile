##############################################################
#### 		SNMP  Makefile
##############################################################

PROJECT_NAME = h323
OBJECTS   = 	cfgparam.o 	\
		h225callTable.o	\
		h245table.o	\
		rasnmp.o	\
		snmp.o		\
		snmpapi.o	\
		snmputil.o	\
		table.o

TARGET    = snmp
PROGLIBS  = 
PROGFLAGS = -I. -I../ads -I../pvt_pst -I../lowunix -I../coder -I../defs \
	-I../cm -I../config -I../log -I../cat -I../utils -I../h245 -I../ras \
        -I../transport -I../asn_objects -I../mib
DISTFILES =
PROGDEFS  = 

PROJECT_BASE := $(word 1, $(subst $(PROJECT_NAME)/,$(PROJECT_NAME) ,$(shell pwd)))
include $(PROJECT_BASE)/../make/unix/common.mk
LIBTARGET = $(libdir)/lib$(TARGET).a
all: $(LIBTARGET)
include $(mkdir)/lib.mk
