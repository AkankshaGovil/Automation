##
## Makefile
##

include $(BASE)/global.mk

OBJS =  \
	tsmapi.o \
	smInvite_S.o \
	smInvite_C.o \
	smInfo_S.o \
	smInfo_C.o \
	smByeCancel_S.o \
	smByeCancel_C.o \
	tsmprocessor.o \
	tsmutils.o \
	tsmactions.o \
	tsmsend.o \
	tsmtimer.o \
	tsmicmp.o \
	tsmsdp.o

lib install: $(LIBSIPTSM)

clean:	
	rm -f $(OBJS) *.a $(LIBSIPTSM)

$(LIBSIPTSM):	$(OBJS)
	$(AR) ruv $@ $^

