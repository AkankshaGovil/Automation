/* imoved from include/mem.h */
#ifdef RPC_HDR
%#include "ipc.h"
%#include "key.h"
%
%
%extern bool_t xdr_Address_e (XDR *, Address_e *);
#endif

/* Realm information which will be useful in doing call source lookups,
** like realmId, passing to FC code, like pool ids, inter/intra realm mr
** usually a subset of the realm entry in the database */
struct _CallRealmInfo {
    unsigned long   realmId;  

	/* all other information can be looked up */
    unsigned long   rsa;     

    unsigned int    sPoolId;
    unsigned int    mPoolId;
    Address_e       addrType;

    unsigned short interRealm_mr;
    unsigned short intraRealm_mr;
	string		   sipdomain<>;
};

typedef struct _CallRealmInfo CallRealmInfo;
