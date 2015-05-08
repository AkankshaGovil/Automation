#ifndef _sipkey_h_
#define _sipkey_h_

#include "call_leg_key.h"

/* For caller, local is From, remote is To.
 * For callee, local is To, remote is From.
 * In other words, 
 * for caller/callee, from is local when it originates a req
 * and from is remote when it responds to a request.
 */
#define SipTranKeyCallid(keyptr) (((keyptr)->callLeg).callid)
#define SipTranKeyLocal(keyptr) (((keyptr)->callLeg).local)
#define SipTranKeyRemote(keyptr) (((keyptr)->callLeg).remote)
#define SipTranKeyCseqno(keyptr) ((keyptr)->cseqno)
#define SipTranKeyMethod(keyptr) ((keyptr)->method)
#define SipTranKeyType(keyptr) ((keyptr)->type)

#if 0 /* moved to ls/rpc/call_leg_key.h */
typedef struct
{
	char			*callid;	/* allocated */
	header_url 		*local;		/* allocated */
	header_url 		*remote;	/* allocated */

} SipCallLegKey;
#endif

typedef struct
{
	/* Maintain this as the first entry */
	SipCallLegKey	callLeg;

	int				cseqno;
	char 			*method;	/* allocated */

#define SIPTRAN_UAC	0
#define SIPTRAN_UAS	1
	int				type;		/* trans type */

} SipTransKey;

#endif /* _sipkey_h_ */
