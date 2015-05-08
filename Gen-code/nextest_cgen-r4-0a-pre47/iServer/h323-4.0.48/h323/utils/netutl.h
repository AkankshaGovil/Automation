/*

NOTICE:
This document contains information that is proprietary to RADVISION LTD.
No part of this publication may be reproduced in any form whatsoever without
written prior approval by RADVISION LTD.

RADVISION LTD reserves the right to revise this publication and make changes
without obligation to notify any person of such revisions or changes.

*/

/****************************************************************************

  netutils.h  --  Network utilities interface

  Module Author: Oz Solomonovich
  This Comment:  26-Dec-1996

  Abstract:      IP and network byte order conversion utilities.

  Platforms:     All platforms supported.  The network byte order conversion
                 utilities determine the host byte order at run-time.

  Known Bugs:    None.

****************************************************************************/


#ifndef __NETUTILS_H
#define __NETUTILS_H


#ifdef __cplusplus
extern "C" {
#endif


#include <rvinternal.h>


#define CONVERSION_ERROR    0xffffffff  /* used by ip conversion routines */

BOOL isMulticastIP(UINT32 ip);

/* converts host to network byte order, UINT32 values.
   returns CONVERSION_ERROR on error. */
UINT32 rv_htonl(UINT32 hostlong);

/* converts network to host byte order, UINT32 values.
   returns CONVERSION_ERROR on error. */
UINT32 rv_ntohl(UINT32 netlong);

/* converts host to network byte order, UINT16 values.
   returns CONVERSION_ERROR on error. */
UINT16 rv_htons(UINT16 hostshort);

/* converts network to host byte order, UINT16 values.
   returns CONVERSION_ERROR on error. */
UINT16 rv_ntohs(UINT16 netshort);

/* converts a character ip representation to an network order integer.
   returns CONVERSION_ERROR on error. */
UINT32 ip_to_uint32(const char *cp);

/* converts a network order integer ip representation to a string.
   returns CONVERSION_ERROR on error. */
void uint32_to_ip(UINT32 addr, char *ip_buffer);



#ifdef __cplusplus
}
#endif

#endif  /* __NETUTILS_H */


