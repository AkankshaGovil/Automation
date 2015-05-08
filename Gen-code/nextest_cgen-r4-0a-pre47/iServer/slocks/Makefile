#################################################################
#################################################################
##
##			Makefile
##
#################################################################
#################################################################

include $(BASE)/global.mk

VPATH = ..

CFLAGS += -I.

OBJS = 	pt-machine.o \
	pthread.o \
	spinlock.o \
	mutex.o

install: lib

lib : $(OBJS)
	ar ruv $(LIBSLOCKS) $(OBJS)

clean:	
	$(RM) $(OBJS) 

depend:
	$(MAKEDEPEND) $(CFLAGS) *.c


# DO NOT DELETE
