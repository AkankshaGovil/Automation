#################################################################
#################################################################
##
##			Makefile
##
#################################################################
#################################################################

include $(BASE)/global.mk

OBJS = 	bridgefw.o bridgeutils.o bridgesip.o bridgeh323.o

lib install: $(LIBBRIDGE)

$(LIBBRIDGE) : $(OBJS)
	ar ruv $@ $^

clean:	
	$(RM) $(OBJS) 

depend:
	$(MAKEDEPEND) $(CFLAGS) *.c


# DO NOT DELETE
