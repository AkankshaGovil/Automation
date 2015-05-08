##
## Makefile
##

include $(BASE)/global.mk

OBJS =  \
	tsmcb.o \
	uasm.o \
	uabsm.o \
	uactions.o \
	uaevp.o	\
	uautils.o \
	uascm.o \
	uauth.tab.o \
	lex.uauth.o \
	regevp.o \
	regactions.o \
	reg.o \
	regsm.o

ALLOBJS = $(OBJS) \
	uabridge.o

lib install: $(LIBSIPUA) $(LIBSIPUA2)

uauth.tab.c: uauth.y
	bison -v -t -d -p uauth -o $@ $^

lex.uauth.c: uauth.l
	flex -i -Cr -Puauth $^

clean:	
	rm -f $(ALLOBJS) *.a $(LIBSIPUA) $(LIBSIPUA2)

$(LIBSIPUA):	$(ALLOBJS)
	$(AR) ruv $@ $^

$(LIBSIPUA2):	$(OBJS)
	$(AR) ruv $@ $^

