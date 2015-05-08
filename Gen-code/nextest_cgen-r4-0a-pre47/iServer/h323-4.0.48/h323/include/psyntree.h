/*
***********************************************************************************

NOTICE:
This document contains information that is proprietary to RADVISION LTD..
No part of this publication may be reproduced in any form whatsoever without
written prior approval by RADVISION LTD..

RADVISION LTD. reserves the right to revise this publication and make changes
without obligation to notify any person of such revisions or changes.

***********************************************************************************
*/


/*
  psynTree.h

  Syntax tree interface.
  Provides access to a single compilation unit of asn modules.

  Ron S.
  31 Oct. 1996


  Parameters:
   - synH: syntax tree handler.
   - nodeId: id of node in the syntax tree. unique identifier.
   - node: node information unit. Allocated by user.
   - child: field under node information unit. Allocated by user.
   - index: >=1.

  Notes:
   - Field names are identical to names in asn text file. The names are NULL terminated.
   - Output pointer parameters may be null. It means that they are ignored.

  */

#ifndef _PSYNTREE_
#define _PSYNTREE_

#ifdef __cplusplus
extern "C" {
#endif

#include <rvcommon.h>


typedef enum {

    /*** Simple types ***/
    pstInteger =1,
    pstNull,
    pstBoolean,
    pstEnumeration,
    pstObjectIdentifier,
    pstOctetString,

    /*** Strings ***/
    pstBitString,
    pstGeneralString,
    pstUniversalString,
    pstBMPString,
    pstIA5String,
    pstVisibleString,
    pstNumericString,
    pstPrintableString,

    /*** Complex types ***/
    pstChoice,
    pstSequence,
    pstSet,
    pstOf,
    pstSequenceOf,
    pstSetOf

} pstNodeType;


typedef enum {
    pstTagUniversal=1,
    pstTagApplication,
    pstTagPrivate,
    pstTagEmpty
} pstTagClass;

/* child record type */
typedef struct {
    int   index;           /* number of child, starting at 1.
                              RVERROR if invalid */
    int   nodeId;          /* child's node id */
    INT32 fieldId;         /* child's field id */
    BOOL  isOptional;      /* TRUE if the child is optional */
} pstChild;




typedef void (*stPrintFuncP)(int type, const char*line, ...); /* print function prototype */


DECLARE_OPAQUE(HPST); /* handler */

/************************************************************************
 * pstConstruct
 * purpose: Create a PST handle for a type in an ASN.1 definition.
 *          This function uses dynamic allocation when called.
 * input  : syntax      - Syntax buffer to use (the output of the ASN.1
 *                        compiler)
 *                        The buffer is the parameter returned by
 *                        cmEmGetCommonSyntax(), cmEmGetH245Syntax() and
 *                        cmEmGetQ931Syntax()
 *          rootName    - Root name of the type to create a PST handle.
 * output : none
 * return : PST handle on success
 *          NULL on failure
 * examples:
 *      hPst = pstConstruct(cmEmGetQ931Syntax(), (char*)"AliasAddress");
 *      hPst = pstConstruct(cmEmGetH245Syntax(), (char*)"Setup-UUIE.fastStart");
 ************************************************************************/
RVAPI HPST RVCALLCONV
pstConstruct(
    IN  unsigned char*  syntax,
    IN  char*           rootName);


/************************************************************************
 * pstDestruct
 * purpose: Destruct a PST handle created by pstConstruct().
 * input  : hSyn    - PST handle to destruct
 * output : none
 * return : Non-negative value on success
 *          Negative value on failure
 ************************************************************************/
RVAPI int RVCALLCONV
pstDestruct(IN HPST hSyn);


/************************************************************************
 * pstPrintNode
 * purpose: Print the information of a syntax tree node to a buffer
 * input  : hSyn    - Syntax tree handle
 *          nodeId  - Node to print
 *          buf     - Result buffer to use
 *          len     - Length of buffer
 * output : none
 * return : Number of characters printed
 ************************************************************************/
int pstPrintNode(
    IN  HPST            hSyn,
    IN  int             nodeId,
    IN  char*           buf,
    IN  int             len);


RVAPI pstNodeType RVCALLCONV /* type of node */
pstGetNodeType(
     IN  HPST    synH,
     IN  int     nodeId
     );

/* for Sequence Of XXX/Set Of XXX nodes - returns the id of XXX */
RVAPI int RVCALLCONV
pstGetNodeOfId(
     IN  HPST    synH,
     IN  int     nodeId
     );

RVAPI int RVCALLCONV /* type of node */
pstGetNodeRange(
      IN  HPST    synH,
      IN  int     nodeId,
      OUT int *   from,
      OUT int *   to
      );

RVAPI int RVCALLCONV /*the tag of the node or RVERROR*/
pstGetTag(
     IN  HPST        synH,
     IN  int         nodeId,
     OUT pstTagClass* tagClass);

RVAPI int RVCALLCONV /*is not extended or RVERROR*/
pstGetIsExtended(
     IN  HPST        synH,
     IN  int         nodeId);

RVAPI int RVCALLCONV /*is not extended or RVERROR*/
pstGetIsOpenType(
     IN  HPST        synH,
     IN  int         nodeId);

RVAPI INT32 RVCALLCONV /* Field enumeration or RVERROR */
pstGetFieldId(
        /* convert field name to internal id */
    IN  HPST        synH,
        IN  const char *fieldName /* null terminated string */
        );

RVAPI int RVCALLCONV /* Real length of field name or RVERROR */
pstGetFieldName(
      /* convert field internal id to field name */
      IN  HPST synH,
      IN  INT32 fieldId,
      IN  int fieldNameLength, /* num. of bytes in string allocation */
      OUT char* fieldName /* user allocated */
      );

RVAPI char* RVCALLCONV /* null terminated token name */
pstGetTokenName(
      IN pstNodeType type);

RVAPI  int RVCALLCONV /* actual length of the fromString or RVERROR */
pstGetFROMString(
       /* Get the character constraints of the syntax node */
       IN  HPST synH,
       IN  int nodeId,
       IN  int fromStringLength, /* num. of bytes in string allocation */
       OUT char* fromString /* null terminated. user allocated */
       );

RVAPI /*INT32*/int RVCALLCONV /* Internal node id or RVERROR */
pstGetNodeIdByPath(
         /* get internal node id from specified node path.
        Path to node should start at root, and correspond to the
        ASN module syntax structure. */
         IN  HPST        synH,
         IN  const char *path  /* format: "a.b.c" */
         );


RVAPI int RVCALLCONV /* the length of the root name or RVERROR. */
pstGetRootName(
     /* note: if bufferLength is smaller than the name length,
        the correct length is returned, but the buffer will
        not be valid. */
     IN  HPST    synH,
     IN  int     bufferLength,   /* target buffer length */
     OUT char *  buffer          /* target buffer for root name */
     );

RVAPI int RVCALLCONV
pstGetNumberOfChildren(
         IN  HPST    synH,
         IN  int     nodeId
         );


RVAPI int RVCALLCONV
pstGetChild(
      IN  HPST        synH,
      IN  int         nodeId,      /* node id of parent */
      IN  int         childIndex,   /* index of child */
      OUT pstChild *  child
      );


RVAPI BOOL RVCALLCONV /* true if node is of one of the string types: */
/*  pstObjectIdentifier, pstOctetString, pstBitString, pstGeneralString,
  pstUniversalString, pstBMPString, pstIA5String, pstVisibleString,
  pstNumericString, pstPrintableString
*/
pstIsStringNode(
      IN  HPST     synH,
      IN  int      nodeId);


#ifdef __cplusplus
}
#endif

#endif


