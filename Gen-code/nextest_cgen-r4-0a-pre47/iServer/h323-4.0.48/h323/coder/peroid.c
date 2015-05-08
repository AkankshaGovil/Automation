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
  perOID.c

  Ron S.
  16 May 1996


  ____________________________________________________________________________
  ___________________________OBJECT IDENTIFIER________________________________
  ____________________________________________________________________________


  format:

  +--------+-----------+
  | length | BER - OID |
  +--------+-----------+

  */


#include <stdio.h>
#include <perintr.h>


/*___________________________________________________________________________*/
/*________________________________Object_Identifier__________________________*/
/*___________________________________________________________________________*/
/*
  Desc: Encodes an object identifier into TAIL of buffer.
  clause '21'

  */
int
perEncodeOID(IN  HPER hPer,
         IN  int synParent,
         IN  int valParent, /* this is me */
         IN  INT32 fieldId)
{
  perStruct *per = (perStruct *)hPer;
  INT32 length=0;
  char *data=(char*)per->buf->buffer;
  if(synParent);

  pvtGet(per->hVal, valParent, NULL, NULL, &length, NULL);
  if (length < 0)
  {
      msaPrintFormat(ErrPER, "perEncodeOID: %s: buffer allocation error.",
           nprn(pstGetFieldNamePtr(per->hSyn, fieldId)));
      return RVERROR;
  }

  pvtGetString(per->hVal, valParent,length,data);
  if (bbFreeBytes(per->hBB) < length) {
    msaPrintFormat(ErrPER, "perEncodeOID: %s: buffer allocation error.",
           nprn(pstGetFieldNamePtr(per->hSyn, fieldId)));
    return RVERROR;
  }

  /* with length determinant: 16.8 */
  perEncodeLen(perLenTypeUNCONSTRAINED, length, 0, 0, per->hBB);
  return bbAddTail(per->hBB, (UINT8 *)data, length*8, TRUE);
}


/*
  Desc: Decodes an object identifier from buffer (at position).
  Returns: RVERROR in case of fault or positive number.
  Note: Currently an octet string.
  */
int
perDecodeOID(IN  HPER hPer,
         IN  int synParent, /* parent in syntax tree */
         IN  int valParent, /* field parent in value tree */
         IN  INT32 fieldId)   /* enum of current field */
{
  char *stringForPrint;
  perStruct *per = (perStruct *)hPer;
  UINT32 length=0;
  UINT32 dec = 0; /* decoded bits */

  int vtPath;
  char *data=(char*)per->buf->buffer;
  if(synParent);

  /* -- get length: 16.8 */
  if (perDecodeLen(perLenTypeUNCONSTRAINED, &length, 0, 0, hPer, per->decodingPosition, &dec) <0)
    return RVERROR;
  per->decodingPosition+=dec;

 if (length>0x1fffffff || per->decodingPosition+length*8 > (UINT32)bbBitsInUse(per->hBB))
    {
      msaPrintFormat(ErrPER, "per:DecodeOID: Object Identifier too large to decode from buffer [len=%ld]",
             length*8);
      return RVERROR;
    }




  /* -- add value node */
    vtPath=valParent;
    if (fieldId!=RVERROR)
  if ((vtPath=pvtAdd(per->hVal, valParent, fieldId, (INT32)length, (char*)"", NULL)) <0) {
    stringForPrint=pstGetFieldNamePtr(per->hSyn, fieldId);
    msaPrintFormat(ErrPER, "perDecodeOID: cannot add string to value tree [%s:%d].",
           nprn(stringForPrint), length);
    return RVERROR;
  }
  per->decodingPosition += bbAlignBits(per->hBB, (INT32)(per->decodingPosition)); /* alignment */
  if(bbGet2Left(hPer, per->decodingPosition, length*8, (UINT8 *)data)<0)
    return RVERROR;

  if (pvtSet(per->hVal, vtPath, -1, (INT32)length, data) <0) {
    stringForPrint=pstGetFieldNamePtr(per->hSyn, fieldId);
    msaPrintFormat(ErrPER, "perDecodeOID: cannot add string to value tree [%s:%d].",
           nprn(stringForPrint), length);
    return RVERROR;
  }
  per->decodingPosition += (length*8);
  return TRUE;
}

#ifdef __cplusplus
}
#endif



