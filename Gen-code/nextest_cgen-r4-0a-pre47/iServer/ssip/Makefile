##
## Makefile
##

include $(BASE)/global.mk

OBJS =  \
	ssip.o \
	ssipcb.o \
	ssiptimers.o \
	ack.o \
	refer.o \
	via.o \
	send.o \
	loop.o \
	response.o \
	cancel.o \
	bye.o \
	utils.o \
	recordroute.o \
	hashmd5.o \
	mddriver.o \
	authenticate.o \
	branch.o \
	sipformmessage.o

ALLOBJS = $(OBJS) \
	register.o \
	invite.o

lib install: $(LIBSIP) $(LIBSIP2)

clean:	
	rm -f $(ALLOBJS) *.a $(LIBSIP) $(LIBSIP2)

$(LIBSIP):	$(ALLOBJS)
	$(AR) ruv $@ $^

$(LIBSIP2):	$(OBJS)
	$(AR) ruv $@ $^

