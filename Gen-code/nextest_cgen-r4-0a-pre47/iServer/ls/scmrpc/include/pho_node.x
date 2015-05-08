/* moved from include/ipc.h */
/* IP address type */
#ifdef RPC_HDR
%#include <sys/types.h>
%
%
%#define REG_ID_LEN	68
%#define PHONE_NUM_LEN	64
%#define VPN_LEN	PHONE_NUM_LEN
%#define PASSWORD_LEN	16
%
%
%typedef union
%{
%	unsigned long		l;
%	unsigned char 		uc[4];
%} IPaddr;
%
%
%extern bool_t xdr_IPaddr (XDR *, IPaddr *);
%
#endif

/*
 * Client credentials.
 * Also a Phonode structure. A phonode structure identifies
 * a phone endpoint on a node.
 */
struct _PhoNode
{
	char 			regid[REG_ID_LEN];
	unsigned long	uport;
	IPaddr			ipaddress;	/* Physical IP Address */
	unsigned long	realmId;	/* realmid of the phonode */

	char			phone[PHONE_NUM_LEN];

  	char 			vpnPhone[VPN_LEN]; 	/* Vpn Id - sent to the vpns only */
  	unsigned long 	vpnExtLen;
  	unsigned short 	cap;			/* Capabilities of the terminal */

	unsigned short 	sflags;			/* Indicates what all is set in this */
	unsigned long	clientState;

#if 0	/* password field eliminated in 2.1 */
	/* Pin or password, either encrypted or plain-text */
	/* Encryption is TBD. */
	char			password[PASSWORD_LEN];	
#endif

};

typedef struct _PhoNode PhoNode; 
typedef struct _PhoNode ClientCredo;
