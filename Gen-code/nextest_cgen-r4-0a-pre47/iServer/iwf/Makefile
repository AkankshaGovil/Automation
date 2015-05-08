#################################################################
#################################################################
##
##			Makefile
##
#################################################################
#################################################################

include $(BASE)/global.mk

OBJS = 	iwfsm.o \
		iwfsmactions.o \
		iwfsmdriver.o \
		iwfutils.o	\
		cause2err.o	\


lib install: $(LIBIWF)

$(LIBIWF) : $(OBJS)
	ar ruv $@ $^

clean:	
	$(RM) $(OBJS) 

.o:	.c
	$(CC) $(CFLAGS) -o $@ $^

depend:
	$(MAKEDEPEND) $(CFLAGS) *.c


# DO NOT DELETE
