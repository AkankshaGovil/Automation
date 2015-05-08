#ifdef __cplusplus
extern "C" {
#endif



/*

NOTICE:
This document contains information that is proprietary to RADVISION LTD..
No part of this publication may be reproduced in any form whatsoever without
written prior approval by RADVISION LTD..

RADVISION LTD. reserves the right to revise this publication and make changes
without obligation to notify any person of such revisions or changes.

*/

#include <rvinternal.h>
#include <emanag.h>
#include <q931.h>
#include <per.h>

/* Syntax definitions */
#include <commonasn.h>
#include <q931asn1.h>
#include <h245.h>
#include <ciscoasn.h> /* Nextone */


#include <pvaltree.h>
RVAPI
int RVCALLCONV cmEmEncode(
                        IN  HPVT    valH,
                        IN  int     vNodeId,
                        OUT BYTE*   buffer,
                        IN  int     length,
                        OUT int*    encoded)

{
    if (vNodeId >= 0)
        return emEncode (valH,vNodeId,buffer,length,encoded);
    return vNodeId;

}


RVAPI
int RVCALLCONV cmEmDecode(
                        IN  HPVT    valH,
                        IN  int     vNodeId,
                        IN  BYTE*   buffer,
                        IN  int     length,
                        OUT int*    decoded)
{
    if (vNodeId >= 0)
        return emDecode(valH,vNodeId,buffer,length,decoded);
    return vNodeId;
}



/************************************************************************
 * cmEmInstall
 * purpose: Initialize the encode/decode manager.
 *          This function should be called by applications that want to
 *          encode and decode ASN.1 messages, but don't want to initialize
 *          and use the CM for that purpose.
 * input  : maxBufSize  - Maximum size of buffer supported (messages larger
 *                        than this size in bytes cannot be decoded/encoded).
 * output : none
 * return : Non-negative value on success
 *          Negative value on failure
 ************************************************************************/
RVAPI
int RVCALLCONV cmEmInstall(IN int maxBufSize)
{
    q931Install();
    perConstruct(maxBufSize);
    return 0;
}


/************************************************************************
 * cmEmGetCommonSyntax
 * purpose: Returns the syntax of common.asn.
 *          This syntax holds the system's configuration syntax
 *          The syntax is the buffer passed to pstConstruct as syntax.
 * input  : none
 * output : none
 * return : Syntax of common.asn
 ************************************************************************/
RVAPI
BYTE* RVCALLCONV cmEmGetCommonSyntax(void)
{
    return commonasnGetSyntax();
}


/************************************************************************
 * cmEmGetQ931Syntax
 * purpose: Returns the syntax of Q931/H225 ASN.1
 *          This syntax holds the Q931 and H225 standard's ASN.1 syntax
 *          The syntax is the buffer passed to pstConstruct as syntax.
 * input  : none
 * output : none
 * return : Syntax of Q931/H225 ASN.1
 ************************************************************************/
RVAPI
BYTE* RVCALLCONV cmEmGetQ931Syntax(void)
{
    return q931asn1GetSyntax();
}


/************************************************************************
 * cmEmGetH245Syntax
 * purpose: Returns the syntax of h245.asn.
 *          This syntax holds the H245 standard's ASN.1 syntax
 *          The syntax is the buffer passed to pstConstruct as syntax.
 * input  : none
 * output : none
 * return : Syntax of h245.asn
 ************************************************************************/
RVAPI
BYTE* RVCALLCONV cmEmGetH245Syntax(void)
{
    return h245GetSyntax();
}

/* nextone */
/************************************************************************
 * cmEmGetCiscoSyntax
 * purpose: Returns the syntax of ciscoasn.asn.
 *          This syntax holds the Cisco Non standard ASN.1 syntax
 *          The syntax is the buffer passed to pstConstruct as syntax.
 * input  : none
 * output : none
 * return : Syntax of ciscoasn.asn
 ************************************************************************/
RVAPI
BYTE* RVCALLCONV cmEmGetCiscoSyntax(void)
{
    return ciscoasnGetSyntax();
}
/* nextone */

#ifdef __cplusplus
}
#endif
