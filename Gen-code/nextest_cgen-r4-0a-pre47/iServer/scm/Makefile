##
## Makefile
##

include $(BASE)/global.mk

INCDIRS += -I$(SCMDIR)/include
OBJS =  \
	scm_monitor.o \
	scm_queue.o \
	scm_call.o \
	scm_replicator.o

lib install: $(LIBSCM)

clean:	
	rm -f $(OBJS) *.a $(LIBSCM) 

$(LIBSCM):	$(OBJS)
	$(AR) ruv $@ $^
