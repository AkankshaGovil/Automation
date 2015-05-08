
#################################################################
#################################################################
##
##			Makefile
##
#################################################################
#################################################################

include $(BASE)/global.mk

CFLAGS 			+=	-I$(BASE)/include -I$(BASE)/tavl -I$(BASE)/avl-1.4.0\
					-I$(BASE)/tst1.3 -I$(BASE)/ls/include		\
					-I$(BASE)/ls/scmrpc/include					\
					-I$(BASE)/pd/include						\
					-I$(BASE)/pd/$(TARGET)						\
					-I$(OSDDIR)/include 						\
					-D_POSIX_PTHREAD_SEMANTICS -D_REENTRANT		\
					-Wall -Wno-unused-variable -Wno-parentheses

ifeq ("X$(PLATFORM)", "Xlinux")
LIBSOCK =
CFLAGS += -ggdb -DNETOID_LINUX
else
LIBSOCK = -lsocket
CFLAGS += -ggdb -DSUNOS -DSOLARIS_REL=$(SOLARIS_REL)
endif

dbmalloc = $(DBMALLOC)

ifeq (1,$(dbmalloc))

	##########################
	## Start debug malloc defs
	##########################

	CFLAGS 			+=	-D_DEBUG_MALLOC_
	
	SYSTEM_LIBS		= $(SFIOLIBS) $(LIBSOCK) -lnsl -lrt -lpthread -lm -ldbmalloc

	##########################
	## End debug malloc defs
	##########################
else

	##########################
	## Start normal defs
	##########################

	SYSTEM_LIBS		= $(DBMALLOCLIB) $(SFIOLIBS) $(LIBSOCK) -lnsl -lrt -lpthread -lm -ldl

	##########################
	## End normal defs
	##########################
endif

LIBDEPS 		=  	$(LIBDB) $(LIBUTILS) $(LIBXML) $(LIBSCONFIG) \
					$(LIBCOMMON) $(LIBOSD) $(LIBFCE) $(LIBSRVR) $(LIBLOCK)

NEXTONE_LIBS 	= 	-ldb -lutils -lxml -lsconfig -lsrvr -lfce \
					-llock -lcommon -losd

LIBRSDRPC	= $(BASE)/lib/$(TARGET)/librsdrpc.a

VPATH = ..:$(BASE)/lib/$(TARGET)

LDFLAGS += -L$(BASE)/lib/$(TARGET)

ISPDOBJDIR = .

ISPDOBJS =	$(ISPDOBJDIR)/ispd.o				\
			$(ISPDOBJDIR)/icmp_echo_utils.o		\
			$(ISPDOBJDIR)/ispd_queue.o			\
			$(ISPDOBJDIR)/get_iface_info.o		\
			$(ISPDOBJDIR)/ispd_svc.o			\
			$(ISPDOBJDIR)/ispd_rpc_svc.o		\
			$(ISPDOBJDIR)/ispd_svc_proc.o		\
			$(ISPDOBJDIR)/ispd_clnt_proc.o		\
			$(ISPDOBJDIR)/ispd_rpc_clnt.o		\
			$(ISPDOBJDIR)/ispd_rpc_xdr.o

LIBRSDRPC_OBJS =	$(BASE)/pd/$(TARGET)/dbsync_svc.o 		\
					$(BASE)/pd/$(TARGET)/dbsync_svc_proc.o	\
					$(BASE)/pd/$(TARGET)/ispd_rpc_svc.o		\
					$(BASE)/pd/$(TARGET)/ispd_rpc_xdr.o

LIBGISRPC_OBJS =	$(BASE)/pd/$(TARGET)/gis_svc.o 			\
					$(BASE)/pd/$(TARGET)/gis_svc_proc.o		\
					$(BASE)/pd/$(TARGET)/ispd_rpc_svc.o		\
					$(BASE)/pd/$(TARGET)/ispd_rpc_xdr.o

all			:	.rpc $(LIBRSDRPC) $(LIBGISRPC) $(BINISPD)

.rpc		:
	( cd ../rpc ; make )

##
## Manager level programs.
##
$(BINISPD)	:	$(ISPDOBJS) $(LIBDEPS)
	$(CC) $(LDFLAGS) -o $@ $(ISPDOBJS) $(NEXTONE_LIBS) $(SYSTEM_LIBS)

$(LIBRSDRPC)	:	$(LIBRSDRPC_OBJS)
	$(AR) ruv $@ $^

$(LIBGISRPC)	:	$(LIBGISRPC_OBJS)
	$(AR) ruv $@ $^

test		:
	$(CC) $(CFLAGS) $(PROFFLAGS) $(LDFLAGS) test.c

testapp		: testappc.o $(LIBH323)
	$(CC) $(LDFLAGS) -o $@ $^ $(NEXTONE_LIBS) $(SYSTEM_LIBS)

.o			:	.c Makefile .rpc
	$(CC) -ggdb $(CFLAGS) $(PROFFLAGS) -o $@ $^

%.cpo		:	%.c Makefile
	$(CC) $(CFLAGS) $(PROFFLAGS) -E $< >$@ 2>&1

../rpc/ispd_rpc.h	:
	(cd ../rpc; make)

clean		:	
	(cd ../rpc && make clean)
	$(RM) -f	*.o *.cpo $(BINISPD) core.* .rpc		\
				malloc.log malloc.inuse $(LIBRSDRPC)	\
				$(LIBGISRPC)

depend		:
	$(MAKEDEPEND) $(CFLAGS) $(PROFFLAGS) *.c

exec		:	all

# DO NOT DELETE
