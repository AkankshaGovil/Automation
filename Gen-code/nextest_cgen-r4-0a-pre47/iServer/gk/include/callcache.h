#ifndef _callcache_h_
#define _callcache_h_

#include "calldefs.h"

#ifdef _MAINTAIN_SEPARATE_


/* Call Structure */
typedef struct
{
	 /* KEY - CALL-ID */
	 char		callID[CALL_ID_LEN];	/* Printable, includes NULL */
	 int		callIDLen;
	 int		callModel;
	 int		callState;
	 int		inCRV;
	 int		outCRV;

	 PhoNode		phonode;
	 unsigned long 	rasip;
	 unsigned short	rasport;
 	 int            realm_id;

	 /* Remote Information */
	 PhoNode		rphonode;
	 unsigned long 	rrasip;
	 unsigned short	rrasport;
	 int            rrealm_id;

	 /* Creation and refresh time */
	 time_t			iTime;
	 time_t			rTime;

	 char			flags[4];
	 int			callNo;

} CallHandle;

#define CallSetCallID(call, _callID_)	(memcpy((call)->callID, _callID_, CALL_ID_LEN))
#define CallCallModel(call)				((call)->callModel)
#define	CallSetInCRV(call, _CRV_)		((call)->inCRV = _CRV_)
#define	CallSetOutCRV(call, _CRV_)		((call)->outCRV = _CRV_)

extern int ncalls;

#endif /* MAINTAIN_SEPARATE */

#include "arq.h"
#endif /* _callcache_h_ */ 
