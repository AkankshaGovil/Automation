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

#ifndef CODER_H
#define CODER_H
#include <pvaltree.h>

RVAPI
int RVCALLCONV cmEmEncode(
            IN      HPVT            valH,
            IN      int             vNodeId,
            OUT     BYTE*           buffer,
            IN      int             length,
            OUT     int*            encoded);

RVAPI
int RVCALLCONV cmEmDecode(
            IN      HPVT            valH,
            IN      int             vNodeId,
            IN      BYTE*           buffer,
            IN      int             length,
            OUT     int*            decoded);


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
int RVCALLCONV cmEmInstall(IN int maxBufSize);



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
BYTE* RVCALLCONV cmEmGetCommonSyntax(void);


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
BYTE* RVCALLCONV cmEmGetQ931Syntax(void);


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
BYTE* RVCALLCONV cmEmGetH245Syntax(void);

/* Nextone */
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
BYTE* RVCALLCONV cmEmGetCiscoSyntax(void);
/* Nextone */

#endif

#ifdef __cplusplus
}
#endif
