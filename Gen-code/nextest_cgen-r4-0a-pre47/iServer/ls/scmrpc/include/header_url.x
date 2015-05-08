/* moved from ssip/include/ssip.h */
#ifdef RPC_HDR
%#include "sipcommon.h"
#endif 

struct _SipUrlParameter
{
	string name<>;
	string value<>;
};

typedef struct _SipUrlParameter SipUrlParameter;
typedef struct _SipUrlParameter SipPrivacyParams;

#ifdef RPC_HDR
%#define SIP_MAXURLPARMS	10
#endif
#ifdef RPC_XDR
typedef unsigned short SIP_U16bit;
#endif

struct _header_url
{
    string name<>;
    string host<>;
    string tag<>;
	string display_name<>;
    string maddr<>;
	SipUrlParameter url_parameters[SIP_MAXURLPARMS];

	SIP_U16bit port;
	SIP_U16bit type;	/* SIP or H.323 */
	int realmId;
	SipPrivacyParams priv_params[SIP_MAXURLPARMS];
	string header<>;
};

typedef struct _header_url  header_url;
typedef struct _header_url  SipHeaderUrl;
typedef struct _header_url  SipHeaderUri;

typedef struct _header_url_list header_url_list;

#ifdef RPC_HDR
%typedef header_url_list *pheader_url_list;
#endif


struct _header_url_list 
{
	header_url_list *prev;
	header_url_list *next;
	header_url *url;
};
