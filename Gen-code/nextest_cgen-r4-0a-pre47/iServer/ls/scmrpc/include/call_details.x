/* file created from gk/include/calldefs.h */
#ifdef RPC_HDR
%#include "mfcp.h"
#endif /* RPC_HDR */

#ifdef RPC_XDR
%extern bool_t xdr_pMFCP_Session(register XDR *xdrs, pMFCP_Session *objp);
#endif

#if defined(RPC_HDR)
%
%struct _FCEStatusStruct {
%	unsigned char rval	:3;
%#define	FCE_INPROGRESS		1
%#define	FCE_SUCCESS			2
%#define	FCE_FAILURE			3
%	unsigned char op	:3;
%// Same as fwop - noop/modify/open/close
%	unsigned char err	:2;
%#define	FCE_LICENSE_FAILURE	1
%#define	FCE_GENERAL_FAILURE	2
%#define	FCE_NATT_LICENSE_FAILURE	3
%};
%
%typedef struct _FCEStatusStruct FCEStatusStruct;
%
%extern bool_t xdr_FCEStatusStruct(register XDR *xdrs, FCEStatusStruct *objp);
%#define FCE_STATUS_IS_NULL(x) (*((unsigned char *)(&x)) == 0)
%
#endif

struct _CallDetails
{
	int callError;
	int lastEvent;
	int flags;
#ifdef RPC_HDR
%	#define HUNT_TRIGGER	0x1
%	#define REDIRECT	0x2
#endif

	int	callType;		/* either CAP_SIP or CAP_H323 */
	pMFCP_Session	fceSession;	/* MFCP session where the following resource resides */
	unsigned int	fceResource;	/* resource id associated with media holes on this call */
	unsigned int	fceDstIp;
	unsigned short	fceDstPort;
	unsigned int	fceSrcIp;
	unsigned short	fceSrcPort;
	FCEStatusStruct	fceStatus;		/* either FCE_SUCCESS or FCE_GENERAL_FAILURE */
	char 			*relatedCallID;

	/* H.323 */
	int	cause;			/* ISDN CC */
	int	h225Reason;		
	int	rasReason;	

	/* SIP */
	int	responseCode;

};

typedef struct _CallDetails CallDetails;
