#ifndef OSD_H

#define OSD_H
#include <netdb.h>
#include "osdconfig.h"
#include "nxosdtypes.h"

// header includes for rpcent structure and getrpcbyname_r definition
#if HAVE_FUNC_GETRPCBYNAME_R_4
#if HAVE_RPC_RPCENT_H
#include <rpc/rpcent.h>
#endif
#elif HAVE_FUNC_GETRPCBYNAME_R_5
#if HAVE_RPC_NETDB_H
#include <rpc/netdb.h>
#endif
#endif


extern size_t nx_strlcpy (char *, const char *, size_t);

extern struct hostent* nx_gethostbyname_r (const char *, struct hostent *, 
char *, int, int *h_errnop);

extern hrtime_t nx_gethrtime();

extern size_t nx_strlcat(char *dst, const char *src, size_t dstsize);

extern int nx_sig2str(int signum, char *str, size_t buflen);

extern  struct  rpcent*  nx_getrpcbyname_r(const  char  *name,   struct
					   rpcent *result, char *buffer, int buflen);

extern int nx_mkdirp(const char *path, mode_t mode);

extern void nx_thread_set_rt(int32_t ns_quantum);

//SVCTCP_CREATE
#if HAVE_SVC_CREATE
#define nx_svctcp_create(dispatch, prognum,  versnum) \
  svc_create( dispatch, prognum, versnum, "tcp") 
	
#elif HAVE_SVCTCP_CREATE
	
#define nx_svctcp_create(dispatch, prognum,  versnum) \
  svc_register(svctcp_create(RPC_ANYSOCK, 0, 0), prognum, versnum, dispatch, IPPROTO_TCP)

#else
#error no fallback implementation for creating rpc service using tcp found
#endif

//SVCUDP_CREATE
#if HAVE_SVC_CREATE
#define nx_svcudp_create(dispatch, prognum,  versnum) \
  svc_create( dispatch, prognum, versnum, "udp") 
	
#elif HAVE_SVCTCP_CREATE
	
#define nx_svcudp_create(dispatch, prognum,  versnum) \
  svc_register(svcudp_create(RPC_ANYSOCK), prognum, versnum, dispatch, IPPROTO_UDP)

#else
#error no fallback implementation for creating rpc service using udp found
#endif


#endif/* OSD_H */

