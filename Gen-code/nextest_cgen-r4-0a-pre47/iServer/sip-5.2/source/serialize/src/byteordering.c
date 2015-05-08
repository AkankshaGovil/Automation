#include <portlayer.h>
#include <sipcommon.h>
#include <byteordering.h>

SIP_U32bit SIP_htonl
#ifdef ANSI_PROTO
(SIP_U32bit hostlong)
#else
(hostlong)
SIP_U32bit hostlong;
#endif
{
	return(htonl(hostlong));
}

SIP_U32bit SIP_ntohl
#ifdef ANSI_PROTO
(SIP_U32bit netlong)
#else
(netlong)
SIP_U32bit netlong;
#endif
{
       return(ntohl(netlong));
}

SIP_U16bit SIP_htons
#ifdef ANSI_PROTO
(SIP_U16bit hostshort)
#else
(hostshort)
SIP_U16bit hostshort;
#endif
{
       return(htons(hostshort));
}

SIP_U16bit SIP_ntohs
#ifdef ANSI_PROTO
(SIP_U16bit netshort)
#else
(netshort)
SIP_U16bit netshort;
#endif
{
      return(ntohs(netshort));
}
