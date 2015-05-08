#
#	RPC Makefile
#
#  		File Naming conventions
#
#			Input Files :
#
#				$(APPN).x           	- RPCGEN input, XDR protocol definition
#
#				ispd_svc.c				- Server routines for initialization and
#										   running of server.
#
#				ispd_svc_proc.c			- Server routines called by
#											Server stub
#
#				ispd_clnt_proc.c		- Client routines which call
#											client stub
#
#  			RPCGEN Generated files :
#
#    			$(APPN).h				- Misc includes generated
#    			$(APPN)_clnt.c			- Client stub
#    			$(APPN)_svc.c			- Server stub
#    			$(APPN)_xdr.c			- Client/Server XDR conversion routines
#

include $(BASE)/global.mk

APPN			= ispd_rpc
CLIENT			= ../$(TARGET)/test_client
SERVER			= ../$(TARGET)/test_server

CC				= gcc

CFLAGS 	=	-I$(BASE)/include -I$(BASE)/tavl -I$(BASE)/avl-1.4.0\
			-I$(BASE)/tst1.3 -I$(BASE)/ls/include				\
			-I$(BASE)/pd/include								\
			-I$(BASE)/pd/$(TARGET)								\
			-I$(BASE)/ls/scmrpc/include							\
			-I$(OSDDIR)/include 								\
			-D_POSIX_PTHREAD_SEMANTICS -D_REENTRANT				\
			-Wall -Wno-unused-variable -Wno-parentheses

ifeq ("X$(PLATFORM)", "Xlinux")
CFLAGS += -ggdb -DNETOID_LINUX
else
CFLAGS += -ggdb -DSUNOS -DSOLARIS_REL=$(SOLARIS_REL)
endif

dbmalloc = $(DBMALLOC)

ifeq (1,$(dbmalloc))
	CFLAGS 	+= -D_DEBUG_MALLOC_
endif

OBJS	= 	../$(TARGET)/ispd_clnt_proc.o		\
			../$(TARGET)/ispd_rpc_svc.o			\
			../$(TARGET)/ispd_svc.o				\
			../$(TARGET)/gis_svc.o				\
			../$(TARGET)/dbsync_svc.o			\
			../$(TARGET)/test_client.o			\
			../$(TARGET)/ispd_rpc_clnt.o		\
			../$(TARGET)/ispd_rpc_xdr.o			\
			../$(TARGET)/ispd_svc_proc.o		\
			../$(TARGET)/gis_svc_proc.o			\
			../$(TARGET)/dbsync_svc_proc.o		\
			../$(TARGET)/test_server.o

RPCGEN			= rpcgen
RPCGEN_FLAGS	= -m

RPCGEN_TARGETS	= 	../$(TARGET)/$(APPN).h 		\
					../$(TARGET)/$(APPN)_clnt.c	\
					../$(TARGET)/$(APPN)_svc.c	\
					../$(TARGET)/$(APPN)_xdr.c

LDLIBS			= -L$(BASE)/lib/$(TARGET) -lsocket -lnsl -lrt -lpthread \
				  -ldb -lutils -lxml -lsconfig -lsrvr -lfce -llock -lcommon -losd

all					:	$(RPCGEN_TARGETS) $(OBJS)

rpcgen_targets		:	$(RPCGEN_TARGETS)

test				:	$(CLIENT) $(SERVER)


../$(TARGET)/$(APPN)_svc.c		: $(APPN).x Makefile
	$(RPCGEN) -C -m -o $@ $<

../$(TARGET)/$(APPN)_clnt.c		: $(APPN).x Makefile
	$(RPCGEN) -C -l -o $@ $<

../$(TARGET)/$(APPN)_xdr.c		: $(APPN).x Makefile
	$(RPCGEN) -C -c -o $@ $<

../$(TARGET)/$(APPN).h			: $(APPN).x Makefile
	$(RPCGEN) -C -h -o $@ $<


../$(TARGET)/ispd_clnt_proc.o	: ispd_clnt_proc.c
	$(CC) $(CFLAGS) $(PROFFLAGS) -c $< -o $@

../$(TARGET)/ispd_svc.o	: ispd_svc.c
	$(CC) $(CFLAGS) $(PROFFLAGS) -c $< -o $@

../$(TARGET)/gis_svc.o	: gis_svc.c
	$(CC) $(CFLAGS) $(PROFFLAGS) -c $< -o $@

../$(TARGET)/dbsync_svc.o	: dbsync_svc.c
	$(CC) $(CFLAGS) $(PROFFLAGS) -c $< -o $@

../$(TARGET)/test_client.o	: test_client.c
	$(CC) $(CFLAGS) $(PROFFLAGS) -c $< -o $@

../$(TARGET)/ispd_rpc_clnt.o	: ../$(TARGET)/ispd_rpc_clnt.c
	$(CC) $(CFLAGS) $(PROFFLAGS) -c $< -o $@

../$(TARGET)/ispd_rpc_xdr.o	: ../$(TARGET)/ispd_rpc_xdr.c
	$(CC) $(CFLAGS) $(PROFFLAGS) -c $< -o $@

../$(TARGET)/ispd_svc_proc.o	: ispd_svc_proc.c
	$(CC) $(CFLAGS) $(PROFFLAGS) -c $< -o $@

../$(TARGET)/gis_svc_proc.o	: gis_svc_proc.c
	$(CC) $(CFLAGS) $(PROFFLAGS) -c $< -o $@

../$(TARGET)/dbsync_svc_proc.o	: dbsync_svc_proc.c
	$(CC) $(CFLAGS) $(PROFFLAGS) -c $< -o $@

../$(TARGET)/test_server.o	: test_server.c
	$(CC) $(CFLAGS) $(PROFFLAGS) -c $< -o $@

$(CLIENT)			:	../$(TARGET)/$(APPN).h					\
						../$(TARGET)/ispd_clnt_proc.o			\
						../$(TARGET)/$(APPN)_clnt.o				\
						../$(TARGET)/$(APPN)_xdr.o				\
						../$(TARGET)/test_client.o
	$(CC) $(CFLAGS) $(PROFFLAGS) -o $(CLIENT) ../$(TARGET)/test_client.o 	\
						../$(TARGET)/ispd_clnt_proc.o			\
						../$(TARGET)/$(APPN)_clnt.o				\
						../$(TARGET)/$(APPN)_xdr.o $(LDLIBS)

$(SERVER)			:	../$(TARGET)/$(APPN).h					\
						../$(TARGET)/ispd_svc_proc.o			\
						../$(TARGET)/$(APPN)_svc.o				\
						../$(TARGET)/$(APPN)_xdr.o				\
						../$(TARGET)/ispd_svc.o					\
						../$(TARGET)/test_server.o
	$(CC) $(CFLAGS) $(PROFFLAGS) -o $(SERVER) ../$(TARGET)/test_server.o		\
						../$(TARGET)/ispd_svc.o					\
						../$(TARGET)/ispd_svc_proc.o			\
						../$(TARGET)/$(APPN)_svc.o				\
						../$(TARGET)/$(APPN)_xdr.o $(LDLIBS) -lrpcsvc

%.cpo				:   %.c Makefile
	$(CC) $(CFLAGS) $(PROFFLAGS) -E $< >$@ 2>&1

clean	:
	rm -f $(RPCGEN_TARGETS) $(OBJS) $(SERVER) $(CLIENT) core.* *.cpo

