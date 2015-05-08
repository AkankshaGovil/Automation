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
  perOctString.c

  Ron S.
  16 May 1996


  ____________________________________________________________________________
  ___________________________OCTET STRING_____________________________________
  ____________________________________________________________________________


  format:

  +----------+--------+---------+
  | ext. bit | length | string  |
  +----------+--------+---------+

  */


#include <stdio.h>
#include <perintr.h>

/*
  Desc: Encodes an octet string into TAIL of buffer.
  clause '16'
  */
int
perEncodeOctetString(IN  HPER hPer,
             IN  int synParent,
             IN  int valParent, /* this is me */
             IN  INT32 fieldId)
{
  perStruct *per = (perStruct *)hPer;
    int length=0;
    UINT8 *data=per->buf->buffer;
    perLenType lenType;
    pstNodeType type;
    int to,from;

    type=pstGetNodeType(per->hSyn, synParent);
    pstGetNodeRange(per->hSyn, synParent,&from,&to);

    pvtGet(per->hVal, valParent, NULL, NULL,(INT32*)&length, NULL);
    pvtGetString(per->hVal, valParent,(INT32)length,(char*)data);
    if (bbFreeBytes(per->hBB) < (INT32)length)
    {
        msaPrintFormat(ErrPER, "perEncodeOctetString: %s: buffer allocation error.",
            pstGetFieldNamePtr(per->hSyn, fieldId));
        return RVERROR;
    }

    /* -- extension: 16.3 */
    if (pstGetIsExtended(per->hSyn, synParent) == TRUE)
    {
        if (length > to || length < from)
        { /* not within range */
            perEncodeBool(TRUE, per->hBB);
            perEncodeLen(perLenTypeUNCONSTRAINED, length, 0, 0, per->hBB);
            return bbAddTail(per->hBB, data, length*8, TRUE);
        }
        else
          perEncodeBool(FALSE, per->hBB);
    }

    /* -- length: 16.5 - 16.8*/
    if (from == to && from == 0) /* no constraint */
        lenType = perLenTypeUNCONSTRAINED;
    else
    {
        lenType = perLenTypeCONSTRAINED;
        if (length > to || length < from)
        { /* not within range */
            msaPrintFormat(ErrPER, "perEncodeOctetString: %s length not within range: %d<=%d<=%d.",
                pstGetFieldNamePtr(per->hSyn, fieldId), from, length, to);
            return RVERROR;
        }
    }

    if (lenType == perLenTypeCONSTRAINED && to == 0) return TRUE;  /* 16.5 */

    if (lenType == perLenTypeCONSTRAINED)
    if (from == to)
    {  /* no length determinant */
      if (length <=2)  /* 16.6 */
          return bbAddTail(per->hBB, data, length*8, FALSE); /* no alignment */
      else /* 16.7 */
          return bbAddTail(per->hBB, data, length*8, TRUE); /* with alignment */
    }

    /* with length determinant: 16.8 */
    perEncodeLen(lenType, length, from, to, per->hBB);
    return bbAddTail(per->hBB, data, length*8, length > 0); /* Align only if length > 0: 10.9.3.3 note 2 */
}



/*
  Desc: Decodes an octet string from buffer (at position).
  Note: clause '16'
*/
int
perDecodeOctetString(IN  HPER hPer,
             IN  int synParent, /* parent in syntax tree */
             IN  int valParent, /* field parent in value tree */
             IN  INT32 fieldId)   /* enum of current field */
{
    perStruct *per =(perStruct *)hPer;

    int length = 0;
    BOOL isExtended = FALSE;
    UINT32 dec = 0; /* decoded bits */

    int vtPath;
    UINT8 *data = per->buf->buffer;
    perLenType lenType;
    pstNodeType type;
    int to,from;

    type=pstGetNodeType(per->hSyn, synParent);
    pstGetNodeRange(per->hSyn, synParent,&from,&to);


    /* -- extension: 16.3 */
    if (pstGetIsExtended(per->hSyn, synParent) == TRUE)
    {
        perDecodeBool(&isExtended, hPer, per->decodingPosition, &dec);
        per->decodingPosition += dec;
    }

    if (isExtended == TRUE)
    {
        /* not within extension root range */
        perDecodeLen(perLenTypeUNCONSTRAINED, (UINT32 *)&length, 0, 0, hPer,
            per->decodingPosition, &dec);
        per->decodingPosition += dec;

        if ((UINT32)per->decodingPosition + length*8 > bbBitsInUse(per->hBB))
        {
            msaPrintFormat(ErrPER, "perDecodeOctetString1: Not enough space in buffer to decode string [%s:%d].",
                pstGetFieldNamePtr(per->hSyn, fieldId), length);
            length = 0;
            data[0] = '\0';
        }
        else
        {
            if (bbGet2Left(hPer, per->decodingPosition, length*8, data) < 0)
            {
                msaPrintFormat(ErrPER, "perDecodeOctetString1: bbGet2Left failed [%s]",
                    pstGetFieldNamePtr(per->hSyn, fieldId));
                length = 0;
                data[0] = '\0';
            }
        }
        vtPath=valParent;
        if (fieldId!=RVERROR)
        if ((vtPath=pvtAdd(per->hVal, valParent, fieldId, length, (char*)data, NULL)) < 0)
        {
            msaPrintFormat(ErrPER, "perDecodeOctetString: cannot add string to value tree [%s:%d].",
                pstGetFieldNamePtr(per->hSyn, fieldId), length);
            return RVERROR;
        }

        per->decodingPosition +=(length*8);
        return length? vtPath : RVERROR;
    }

    /* -- not extended */

    /* -- length */
    if (from == to && from == 0) /* no constraint */
        lenType = perLenTypeUNCONSTRAINED;
    else
    {
        lenType = perLenTypeCONSTRAINED;

        if (from == to)
        {
            /* no length determinant */
            length = from;
/*            vtGetString(per->hVal, vtPath, length, (char*)data); */
            if (from > 2)
            {
                /* 16.7 */
                per->decodingPosition += bbAlignBits(per->hBB, (int)(per->decodingPosition)); /* alignment */

                if ((UINT32)per->decodingPosition + length*8 > bbBitsInUse(per->hBB))
                {
                    msaPrintFormat(ErrPER, "perDecodeOctetString2: Not enough space in buffer to decode string [%s:%d].",
                        pstGetFieldNamePtr(per->hSyn, fieldId), length);
                    data[0] = '\0';
                    length = 0;
                }
            }
            if (length)
            {
                if (bbGet2Left(hPer, per->decodingPosition, length*8, data) < 0) /* no alignment */
                {
                    msaPrintFormat(ErrPER, "perDecodeOctetString2: bbGet2Left failed [%s]",
                        pstGetFieldNamePtr(per->hSyn, fieldId));
                    data[0] = '\0';
                    length = 0;
                }
            }
            per->decodingPosition +=(length*8);
            vtPath=valParent;
            if (fieldId!=RVERROR)
            if ((vtPath = pvtAdd(per->hVal, valParent, fieldId, length, (char*)data, NULL)) <0)
            {
                msaPrintFormat(ErrPER, "perDecodeOctetString2: cannot add string to value tree [%s:%d].",
                    pstGetFieldNamePtr(per->hSyn, fieldId), length);
                return RVERROR;
            }

            return length? vtPath : RVERROR;
        }
    }

    /* -- get length: 16.8 */
    if (perDecodeLen(lenType, (UINT32 *)&length, from, to, hPer, per->decodingPosition, &dec) <0)
        return RVERROR;
    per->decodingPosition += dec;

    if (lenType == perLenTypeCONSTRAINED && (length > to || length < from))
    {
        /* not within extension root range */
        msaPrintFormat(ErrPER, "perDecodeOctetString: %s length not within extension root range: %d<=%d<=%d.",
            pstGetFieldNamePtr(per->hSyn, fieldId), from, length, to);
        return RVERROR;
    }


    /* -- add value node */
    if (length > 0)
    {
        /* Align only if the length > 0: 10.9.3.3 note 2 */
        per->decodingPosition += bbAlignBits(per->hBB, (INT32)(per->decodingPosition));
    }

    if ((UINT32)per->decodingPosition + length*8 > bbBitsInUse(per->hBB))
    {
        msaPrintFormat(ErrPER, "perDecodeOctetString3: Not enough space in buffer to decode string [%s:%d].",
            pstGetFieldNamePtr(per->hSyn, fieldId), length);
        length = 0;
        data[0] = '\0';
    }
    else
    {
        if (bbGet2Left(hPer, per->decodingPosition, length*8, data) < 0)
        {
            msaPrintFormat(ErrPER, "perDecodeOctetString3: bbGet2Left failed [%s]",
                pstGetFieldNamePtr(per->hSyn, fieldId));
            length = 0;
            data[0] = '\0';
        }
    }
    vtPath=valParent;
    if (fieldId!=RVERROR)
    if ((vtPath = pvtAdd(per->hVal, valParent, fieldId, length, (char*)data, NULL)) <0)
    {
        msaPrintFormat(ErrPER, "perDecodeOctetString:1: cannot add string to value tree [%s:%d].",
            pstGetFieldNamePtr(per->hSyn, fieldId), length);
        return RVERROR;
    }

    per->decodingPosition +=(length*8);
    return TRUE;
}

#ifdef __cplusplus
}
#endif



