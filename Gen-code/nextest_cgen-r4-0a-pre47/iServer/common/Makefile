
##
## Makefile for DSP Driver.
## Sridhar , 03/98.
##
## Revisions for Linux, 07/13/98.

## Default project.
##
include $(BASE)/global.mk

LIB = libcommon.a
SHAREDLIB = libcommon.so

DESTDIR = ../lib

OBJS =  queue.o  \
	list.o \
	qmsg.o \
	codemap.o \
	signals.o \
	packets.o \
	uiio.o \
	pef.o \
	srvrlog.o \
	ifs.o \
	ifconf.o \
	get_ifi_info.o \
	pids.o \
	phonode.o \
	fdsets.o \
	conn.o \
	timerlists.o \
	timerutils.o \
	ipcerror.o \
	ipcutils.o \
	license.o \
	ipcerror.o \
	phonefunc.o \
	lm_gethwaddr.o \
	lm_utils.o \
	lm_verify.o \
	shm.o \
	shmapp.o \
	sendpoll.o \
	icmpd.o \
	args.o \
	thutils.o \
	fileutils.o \
	sysutils.o

ifneq ($(PLATFORM), linux)
	ALLOBJS = $(OBJS) \
		strace.o
else
	ALLOBJS = $(OBJS)
endif



SHAREDOBJS =	pids.o \
		sendpoll.o

all:	lib

lib install: $(LIBCOMMON)

$(LIBCOMMON):	$(ALLOBJS)
	$(AR) ruv $@ $^
	$(CP) ../codemap_[0-9][0-9][0-9].txt $(BINDIR)
	$(CP) ../codemap_[0-9][0-9][0-9].dat $(BINDIR)

shared-lib: $(SHAREDLIB) $(SHAREDOBJS) Makefile

$(SHAREDLIB): $(SHAREDOBJS)
	$(CCSHARED) $(LDFLAGS) -o $@ $(SHAREDOBJS)

## Administrative targets.

clean:	
	rm -f $(ALLOBJS) $(LIBCOMMON)

deps:
	makedepend $(CFLAGS) *.c

tim: timer.o $(LIBUTILS)
	$(CC) -o $@ $^

## Following objects are not included in the build
example: example.o
	gcc -Wall -I${BASE}/include -o $@ ${BASE}/common/example.c

##
# DO NOT DELETE
