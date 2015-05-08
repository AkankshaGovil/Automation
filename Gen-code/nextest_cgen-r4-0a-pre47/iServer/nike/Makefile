
##
## Makefile for Nike NexTone IKE
## Medhavi, 02/99
##

## Default project.
##
include $(BASE)/global.mk

#INCDIRS = -I/usr/src/linux/include -I$(DRINC) -I../../include 

CFLAGS += -c -Di386

LIB = libnike.a

OBJS =  initiator.o  \
	responder.o \
	header.o \
	service.o \
	sa_utils.o \
	crypto.o \
	utils.o \
	isadb.o \
	cookie.o \
	oakley.o \
	id.o \
	hash.o \
	nonce.o \
	sig.o \
	cert.o \
	process.o \
	misc.o \
	sa.o \
	ke.o \
	ikmpd.o \
	params.o

all:	lib

install:	lib
	cp $(LIB) $(LIBNIKE)

$(LIB) lib:	$(OBJS)
	$(AR) ruv $(LIB) $(OBJS)

initiator.o:	initiator.c
	$(CC) $(CFLAGS) -c $^

responder.o:	responder.c
	$(CC) $(CFLAGS) -c $^

header.o:	header.c
	$(CC) $(CFLAGS) -c $^

ikmpd.o:	ikmpd.c
	$(CC) $(CFLAGS) -c $^

test: libnike.a test.o
	$(CC) -o $@ $^

## Administrative targets.

clean:	
	rm -f $(OBJS)

deps:
	makedepend $(CFLAGS) *.c

params.o:	params.c
	$(CC) $(CFLAGS) -c $^

dbg: params.o
	$(CC) params.o /home/medhavi/ssl/openssl-0.9.1c/libcrypto.a -o test
##
# DO NOT DELETE
