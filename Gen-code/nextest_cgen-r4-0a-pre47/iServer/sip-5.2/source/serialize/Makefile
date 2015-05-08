########################################################################
#
#		This Makefile Creates Serialization Library: libsipserialize.a
#		
########################################################################
include ../../MakeVars

COMMONSERSRC = 	serialize.c\
				byteordering.c\
				initsipmessage.c\
				serializeHeaders.c

	
INCLUDES =  -Ih -I../common/h -Ibison/cfiles\
	-I../common/rpr/h/\
	-I../common/dcs/h/  -Idcs/h\
	-I../accessor/h\
	-I../parser/h\
	-Ih 

COMMONSEROBJS = $(COMMONSERSRC:%.c=../../obj/serialize/%.o) 

OBJECTS = $(COMMONSEROBJS) 

all: genlib

gencode:
	cd autogen; ./generator

genlib: libsipserialize 

libsipserialize: ../../lib/libsipserialize.a
../../lib/libsipserialize.a: $(OBJECTS)
	$(AR)  ../../lib/libsipserialize.a $(OBJECTS)

../../obj/serialize/%.o: src/%.c
	$(CC) $(CFLAGS) $(INCLUDES) -c $< -o $@

allclean:
	rm -f ../../obj/serialize/*
	rm -f ../../lib/libsipserialize.a
	
