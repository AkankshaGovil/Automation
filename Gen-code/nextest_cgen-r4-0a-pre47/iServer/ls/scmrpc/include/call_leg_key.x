/* moved from ssip/include/sipkey.h */
#ifdef RPC_HDR
%#include "header_url.h"
#endif

struct _SipCallLegKey
{
	string			callid<>;	/* allocated */
	header_url 		*local;		/* allocated */
	header_url 		*remote;	/* allocated */

};

typedef struct _SipCallLegKey SipCallLegKey;
