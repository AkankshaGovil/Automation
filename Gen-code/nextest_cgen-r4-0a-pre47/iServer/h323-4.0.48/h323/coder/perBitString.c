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
  perBitString.c

  Irina S.
  14 Dec 1997

  ____________________________________________________________________________
  _____________________________BIT STRING_____________________________________
  ____________________________________________________________________________


  format:

  +----------+--------+---------+
  | ext. bit | length | string  |
  +----------+--------+---------+

  */

#include <stdio.h>
#include <perintr.h>


/*
  Desc: Encodes a bit string into TAIL of buffer.
  clause '15'
  */
int
perEncodeBitString(IN  HPER hPer,
             IN  int synParent,
             IN  int valParent, /* this is me */
             IN  INT32 fieldId)
{
  char *stringForPrint;
  perStruct *per = (perStruct *)hPer;
  int length=0;
  UINT8 *data=per->buf->buffer;
  perLenType lenType;
  int from,to;

#ifdef NamedBit
  UINT32 newLength;
  UINT32 byteN;
#endif

  pstGetNodeRange(per->hSyn,synParent,&from,&to);

  length=(UINT32)pvtGetBitString(per->hVal, valParent, 0, NULL);

  if (bbFreeBytes(per->hBB) < (INT32)bbSetByte(length)) {
    stringForPrint=pstGetFieldNamePtr(per->hSyn, fieldId);
    msaPrintFormat(ErrPER, "perEncodeBitString: %s: buffer allocation error.",
           nprn(stringForPrint));
    return RVERROR;
  }


  pvtGetString(per->hVal, valParent,length/8+1,(INT8*)data);
  /* -- extension: 15.5 */
  if (pstGetIsExtended(per->hSyn,synParent) == TRUE) {
    if (length > to || length < from) { /* not within range */
      perEncodeBool(TRUE, per->hBB);
      perEncodeLen(perLenTypeUNCONSTRAINED, length, 0, 0, per->hBB);
      return bbAddTail(per->hBB, data, length /* *8 */ , TRUE);
    }
    else
      perEncodeBool(FALSE, per->hBB);
  }


  /* -- length: 15.6 - 15.10 */
  if (from == to && from == 0) { /* no constraint */
    lenType = perLenTypeUNCONSTRAINED;

#ifdef NamedBit
    /* 15.2 */

    while (!(data[length/8] & (1 << (7 - (length%8)))) && (length >= 0))
      length--;
    length++;

#endif

  }
  else {
    lenType = perLenTypeCONSTRAINED;

#ifdef NamedBit
    /* 15.3 */
    newLength = length;

    /* length less than lb */
    if (newLength < from) {
      newLength = from;

      /* add trailing 0 bits */
      data[length/8] &= ipow2(8) - ipow2(8 - length%8);
      for (byteN = length/8+1; byteN < newLength/8; byteN++)
    data[byteN] &= 0;

      length = newLength;
    }

    /* remove trailing 0 bits */
    while (!(data[length/8] & (1 << (7 - (length%8)))) && (length >=  from))
      length--;
    length++;

    if (length > to){
#else
      /* not within range */
      if (length > to || length < from){
#endif
    stringForPrint=pstGetFieldNamePtr(per->hSyn, fieldId);
    msaPrintFormat(ErrPER, "perEncodeBitString: %s length not within range: %d<=%d<=%d.",
             nprn(stringForPrint), from, length, to);
      return RVERROR;
    }
  }


  if (lenType == perLenTypeCONSTRAINED && to == 0) return TRUE;  /* 15.7 */

  if (lenType == perLenTypeCONSTRAINED) {
    if (from == to) {  /* no length determinant */
      if (length <=16)  /* 15.8 */
    return bbAddTail(per->hBB, data, length/* *8 */, FALSE); /* no alignment */
      else /* 15.9 */
    return bbAddTail(per->hBB, data, length/* *8 */, TRUE); /* with alignment */
    }
  }

  /* with length determinant: 15.10 */
  perEncodeLen(lenType, length, from, to, per->hBB);
  return bbAddTail(per->hBB, data, length /* *8 */, length > 0); /* Align only of length > 0: 10.9.3.3 note 2 */

}


/*
  Desc: Decodes a bit string from buffer (at position).
  Note: clause '15'
*/
int
perDecodeBitString(IN  HPER hPer,
             IN  int synParent, /* parent in syntax tree */
             IN  int valParent, /* field parent in value tree */
             IN  INT32 fieldId)   /* enum of current field */
{
  char* stringForPrint;
  perStruct *per = (perStruct *)hPer;

  int length=0,ret=RVERROR;
  BOOL isExtended = FALSE;
  UINT32 dec = 0; /* decoded bits */
  int vtPath;
  UINT8 *data=per->buf->buffer;
  perLenType lenType;
  int from,to;


  pstGetNodeRange(per->hSyn,synParent,&from,&to);


 /* -- extension: 16.3 */
  if (pstGetIsExtended(per->hSyn,synParent) == TRUE) {
    perDecodeBool(&isExtended, hPer, per->decodingPosition, &dec);
  }
  per->decodingPosition += dec;
  if (isExtended == TRUE) {   /* not within extension root range */
    perDecodeLen(perLenTypeUNCONSTRAINED, (UINT32 *)&length, 0, 0, hPer, per->decodingPosition, &dec);
    per->decodingPosition += dec;
    vtPath=valParent;
    if (fieldId!=RVERROR)
    if ((ret=vtPath=pvtAdd(per->hVal, valParent, fieldId, length, (char*)"", NULL)) <0) {
      stringForPrint=pstGetFieldNamePtr(per->hSyn, fieldId);
      msaPrintFormat(ErrPER, "perDecodeBitString: cannot add string to value tree [%s:%d].",
             nprn(stringForPrint), length);
      return RVERROR;
    }

    if ((UINT32)per->decodingPosition + length > bbBitsInUse(per->hBB)) {
      stringForPrint=pstGetFieldNamePtr(per->hSyn, fieldId);
      msaPrintFormat(ErrPER, "perDecodeBitString1: Not enough space in buffer to decode string [%s:%d].",
             nprn(stringForPrint), length);
      return RVERROR;
    }
    bbGet2Left(hPer, per->decodingPosition, length/* *8 */, data);

    if (pvtSet(per->hVal, vtPath, -1, length, (char*)data) <0) {
      stringForPrint=pstGetFieldNamePtr(per->hSyn, fieldId);
      msaPrintFormat(ErrPER, "perDecodeBitString: cannot add string to value tree [%s:%d].",
             nprn(stringForPrint), length);
      return RVERROR;
    }


   per->decodingPosition += (length/* *8 */);
    return ret;
  }
  /* -- not extended */

  /* -- length */
  if (from == to && from == 0) /* no constraint */
    lenType = perLenTypeUNCONSTRAINED;
  else {
    lenType = perLenTypeCONSTRAINED;

    if (from == to) {  /* no length determinant */
      length = from;
        vtPath=valParent;
        if (fieldId!=RVERROR)
      if ((ret=vtPath=pvtAdd(per->hVal, valParent, fieldId, length, (char*)"", NULL)) <0) {
    stringForPrint=pstGetFieldNamePtr(per->hSyn, fieldId);
    msaPrintFormat(ErrPER, "perDecodeBitString:1: cannot add string to value tree [%s:%d].",
               nprn(stringForPrint), length);
    return RVERROR;
      }
      if (from > 16)  /* 15.9 */
      {
    per->decodingPosition += bbAlignBits(per->hBB, (INT32)(per->decodingPosition)); /* alignment */

    if ((UINT32)per->decodingPosition + length > bbBitsInUse(per->hBB)) {
      stringForPrint=pstGetFieldNamePtr(per->hSyn, fieldId);
      msaPrintFormat(ErrPER, "perDecodeBitString2: Not enough space in buffer to decode string [%s:%d].",
             nprn(stringForPrint), length);
      return RVERROR;
    }
      }
      /* 15.8 *//* 15.9 */
      bbGet2Left(hPer, per->decodingPosition, length/* *8 */, data); /* no alignment */
      if (pvtSet(per->hVal, vtPath, -1, length, (char*)data) <0) {
    stringForPrint=pstGetFieldNamePtr(per->hSyn, fieldId);
    msaPrintFormat(ErrPER, "perDecodeBitString:1: cannot add string to value tree [%s:%d].",
               nprn(stringForPrint), length);
    return RVERROR;
      }
      per->decodingPosition += (length/* *8 */);
      return ret;
    }
  }

  /* -- get length: 15.10 */
  if (perDecodeLen(lenType, (UINT32 *)&length, from, to,
           hPer, per->decodingPosition, &dec) <0)
    return RVERROR;
  per->decodingPosition+=dec;

  if (lenType == perLenTypeCONSTRAINED &&
      (length > to || length < from)) { /* not within extension root range */
    stringForPrint=pstGetFieldNamePtr(per->hSyn, fieldId);
    msaPrintFormat(ErrPER, "perDecodeBitString: %s length not within extension root range: %d<=%d<=%d.",
           nprn(stringForPrint), from, length, to);
    return RVERROR;
  }


  /* -- add value node */
    vtPath=valParent;
    if (fieldId!=RVERROR)
  if ((ret=vtPath=pvtAdd(per->hVal, valParent, fieldId, length, (char*)"", NULL)) <0) {
    stringForPrint=pstGetFieldNamePtr(per->hSyn, fieldId);
    msaPrintFormat(ErrPER, "perDecodeBitString:1: cannot add string to value tree [%s:%d].",
           nprn(stringForPrint), length);
    return RVERROR;
  }
  if (length > 0)
  {
    /* Align only if length > 0: 10.9.3.3 note 2 */
    per->decodingPosition += bbAlignBits(per->hBB, (INT32)(per->decodingPosition));
  }
  if ((UINT32)per->decodingPosition + length > bbBitsInUse(per->hBB)) {
    stringForPrint=pstGetFieldNamePtr(per->hSyn, fieldId);
    msaPrintFormat(ErrPER, "perDecodeBitString3: Not enough space in buffer to decode string [%s:%d].",
           nprn(stringForPrint), length);
    return RVERROR;
  }

  bbGet2Left(hPer, per->decodingPosition, length/* *8 */, data);
  if (pvtSet(per->hVal, vtPath, -1, length, (char*)data) <0) {
    stringForPrint=pstGetFieldNamePtr(per->hSyn, fieldId);
    msaPrintFormat(ErrPER, "perDecodeBitString:1: cannot add string to value tree [%s:%d].",
           nprn(stringForPrint), length);
    return RVERROR;
  }
  per->decodingPosition += (length/* *8 */);
  return TRUE;
}
#ifdef __cplusplus
}
#endif



