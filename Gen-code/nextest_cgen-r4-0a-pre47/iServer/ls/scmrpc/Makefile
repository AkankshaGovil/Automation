include $(BASE)/global.mk

.SUFFIXES: .x

XDR_OBJS = \
	call_handle_xdr.o \
	call_details_xdr.o \
	codecs_xdr.o \
	sipcall_xdr.o \
	header_url_xdr.o \
	call_leg_key_xdr.o \
	pho_node_xdr.o \
	call_realm_info_xdr.o \
	scmrpcprog_xdr.o

OBJS = \
	norepl.o \
	scm_clnt.o \
	scm_clnt_api.o \
	scm_serv.o \
	scm_serv_api.o


CFLAGS += -DRPC_SPECIFIC
CFLAGS += -Wall -Wno-unused-variable

lib install: $(LIBSCMRPC)

$(LIBSCMRPC): $(XDR_OBJS) $(OBJS)
	ar ruv $@ $^

.x.h:
#	rpcgen -NCh -o ../$@ $<
	rpcgen -NCh  $< | awk -f ../rpc_specific.awk > ../$@

include/scmrpcprog.h: include/scmrpcprog.x
	rpcgen -NCh -o ../$@ $< 

call_handle_xdr.c: include/call_handle.h \
	include/pho_node.h include/call_realm_info.h include/header_url.h \
	include/call_leg_key.h include/codecs.h include/call_details.h \
	include/sipcall.h
	rpcgen -NCc -o $@ ../include/call_handle.x 

call_details_xdr.c: include/call_details.h
	rpcgen -NCc -o $@ ../include/call_details.x 

codecs_xdr.c: include/codecs.h
	rpcgen -NCc -o $@ ../include/codecs.x

call_leg_key_xdr.c: include/call_leg_key.h \
	include/header_url.h
	rpcgen -NCc -o $@ ../include/call_leg_key.x 

sipcall_xdr.c: include/sipcall.h include/codecs.h \
	include/call_leg_key.h include/header_url.h include/pho_node.h \
	include/call_realm_info.h
	rpcgen -NCc -o $@ ../include/sipcall.x 

header_url_xdr.c: include/header_url.h
	rpcgen -NCc -o $@ ../include/header_url.x 

pho_node_xdr.c: include/pho_node.h
	rpcgen -NCc -o $@ ../include/pho_node.x 

call_realm_info_xdr.c: include/call_realm_info.h
	rpcgen -NCc -o $@ ../include/call_realm_info.x 

scmrpcprog_xdr.c: include/scmrpcprog.h \
	include/call_details.h include/call_handle.h
	rpcgen -NCc -o $@ ../include/scmrpcprog.x 

XDR_SRCS=$(XDR_OBJS:%.o=%.c)
XDR_HDRS=$(XDR_OBJS:%_xdr.o=../include/%.h)

clean:
	$(RM) core $(XDR_OBJS) $(XDR_SRCS) $(XDR_HDRS) $(OBJS) \
	$(LIBSCMRPC)
