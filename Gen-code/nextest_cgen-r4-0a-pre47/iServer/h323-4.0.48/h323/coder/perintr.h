#ifdef __cplusplus
extern "C" {
#endif



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
  perInternal.h

  for PER internal usage.

  Ron S.
  14 May 1996

  */

#ifndef _PERINTERNAL_
#define _PERINTERNAL_

#include <per.h>
#include <persimpl.h>
#include <psyntreeStackApi.h>
#include <tls.h>
#include <rlist.h>
#include <bb.h>

#include <ms.h>

#define ErrPER          (perGetMsaErr())
#define InfoPER         (perGetMsa())

#define MAX_SPECIAL_NODES 20
#define MAX_INT_SIZE  4


typedef enum
{
  encDecErrorsObjectWasNotFound =0x1, /* 001 */
  encDecErrorsMessageIsInvalid  =0x2, /* 010 */
  encDecErrorsResourcesProblem  =0x4 /* 100 */
} encDecErrors;


/************************************************************************
 * THREAD_CoderLocalStorage
 * Thread specific information for the coder.
 * bufferSize   - Size of allocated buffer
 * buffer       - Encode/decode buffer to use for the given thread
 *                We've got a single buffer for each thread - this allows us to
 *                work will less memory resources, and dynamic size of buffers.
 ************************************************************************/
typedef struct
{
    UINT32  bufferSize;
    BYTE*   buffer;
} THREAD_CoderLocalStorage;



typedef struct
{
  HPST          hSyn;  /* syntax tree */
  HPVT          hVal;  /* value tree */
  HBB           hBB;   /* encoded buffer */
  int           arrayOfSpecialNodes[MAX_SPECIAL_NODES]; /* list for "special" fields */
  unsigned int  currentPositionInArrayOfSpecialNodes;
  /* --- Encoding parameters --- */
  BOOL          isOuterMostValueEmpty; /* see 10.1.3 for complete encoding */

  /* --- decoding parameters --- */
  unsigned int  encodingDecodingErrorBitMask;
  UINT32        decodingPosition; /* currently decoding... */
  /* last decoded node */
  int           synParent;
  int           valParent;
  INT32         fieldId; /* reference from last parent. Debug only */

  THREAD_CoderLocalStorage* buf; /* Buffer for encode/decode */
} perStruct;



int perGetMsa(void);
int perGetMsaErr(void);

int perEncNode(IN  HPER hPer,
           IN  int synParent,
           IN  int valParent,
           IN  INT32 fieldId,
           IN  BOOL wasTypeResolvedInRunTime);

int perDecNode(IN  HPER hPer,
           IN  int synParent,
           IN  pstFieldSpeciality speciality,
           IN  int valParent,
           IN  INT32 fieldId);

int perEncodeComplete(IN  HPER hPer, IN int bitsInUse);

int perEncodeOpenType(IN  HPER hPer,
              IN  INT32 offset,  /* beginning of open type encoding */
              IN  INT32 fieldId);

#endif
#ifdef __cplusplus
}
#endif



