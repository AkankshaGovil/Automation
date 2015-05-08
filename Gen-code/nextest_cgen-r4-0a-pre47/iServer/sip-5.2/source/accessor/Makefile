########################################################################
# 		This is the Accessor Makefile for the SipStack   	
########################################################################
#
#		Creates libsipapi 
#		
########################################################################

include ../../MakeVars

ACCESSORSRC = sipinternal.c\
	sipclone.c\
	response.c\
	entity.c \
	header.c\
	sdp.c\
	sdpstring.c\
	sipvalidate.c \
	sipbcptinternal.c \
	request.c\
	general.c\
	bcpt.c\
		impp.c\
		imppinternal.c\
	sipstring.c
ACCESSORCCPSRC = ccp.c \
		ccpinternal.c
ACCESSORRPRSRC = rpr.c \
		rprinternal.c
ACCESSORDCSSRC = dcs.c \
			dcsclone.c \
			dcsintrnl.c \
			dcsstring.c

INCLUDES =  -I../parser/h -Ih -Irpr/h  \
		-Idcs/h -I../common/dcs/h \
			-I../common/h -I../common/rpr/h 

ACCESSOROBJS = $(ACCESSORSRC:%.c=../../obj/accessor/%.o) 
ACCESSORCCPOBJS = $(ACCESSORCCPSRC:%.c=../../obj/accessor/ccp/%.o) 
ACCESSORRPROBJS = $(ACCESSORRPRSRC:%.c=../../obj/accessor/rpr/%.o)
ACCESSORDCSOBJS = $(ACCESSORDCSSRC:%.c=../../obj/accessor/dcs/%.o)
OBJECTS = $(ACCESSOROBJS)  \
		$(ACCESSORDCSOBJS) \
			$(ACCESSORRPROBJS)

all: libsipapi

libsipapi: ../../lib/libsipapi.a
../../lib/libsipapi.a: $(OBJECTS)
	$(AR) ../../lib/libsipapi.a $(OBJECTS)

include .depend
../../obj/accessor/%.o: src/%.c
	$(CC) $(CFLAGS)  $(INCLUDES) -c $< -o $@
../../obj/accessor/rpr/%.o: rpr/src/%.c
	$(CC) $(CFLAGS)   $(INCLUDES) -c $< -o $@
../../obj/accessor/dcs/%.o: dcs/src/%.c
	$(CC) $(CFLAGS)   $(INCLUDES) -c $< -o $@
