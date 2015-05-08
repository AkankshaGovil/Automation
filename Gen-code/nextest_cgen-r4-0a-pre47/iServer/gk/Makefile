#################################################################
#################################################################
##
##			Makefile
##
#################################################################
#################################################################

include $(BASE)/global.mk

OBJS = 	uh323.o \
		handleSetup.o \
		uh323cb.o \
		lrq.o \
		rrq.o \
		callsmactions.o \
		arq.o \
		uh323cbage.o \
		uh323protocol.o \
		uh323callcb.o \
		uh323call.o \
		callsm.o \
		callsmEventProcessor.o \
		finwait.o \
		faststart.o \
		h323smutils.o \
		callstats.o \
		uh235.o \
		callutils.o \
		h323realm.o \

AGEOBJS = uh323cbage.o uh323age.o

lib install: $(LIBGK) $(LIBAGEGK) $(GKCONF) $(GKAGECONF)

$(LIBGK) : $(OBJS)
	ar ruv $@ $^

$(LIBAGEGK) : $(AGEOBJS)
	ar ruv $@ $^

$(GKCONF) : h323cfg-gktemplate.val
	cp $^ $@

$(GKAGECONF) : h323cfg-age.val
	cp $^ $@

testgk: testcgk.o
	$(CC) $(LDFLAGS) testcgk.o $(LIBH323) $(LDLIBS)

clean:	
	$(RM) $(OBJS) $(AGEOBJS) $(LIBGK)

depend:
	$(MAKEDEPEND) $(CFLAGS) *.c


# DO NOT DELETE
