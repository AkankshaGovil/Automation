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
  perCharString.c

  Ron S.
  20 May 1996


  clause '26':
  Encoding the restricted character string type

  ____________________________________________________________________________
  ___________________________CHARACTER STRING_________________________________
  ____________________________________________________________________________


  format:

  +----------+--------+---------+
  | ext. bit | length | string  |
  +----------+--------+---------+

  */


#include <stdio.h>
#include <perintr.h>
#include <psyntree.h>
#include <intutils.h>

static int /* number of bytes for a known-multiplier character according to type. */
perCharSize(pstNodeType type)
  /* 26.5.3 */
{
    switch (type)
    {
        case pstUniversalString:    return 4;
        case pstBMPString:          return 2;
        case pstIA5String:          return 1;
        case pstGeneralString:      return 1;
        case pstVisibleString:      return 1;
        case pstNumericString:      return 1;
        case pstPrintableString:    return 1;
        default: break;
    }

    return -1;
}


/*
   Returns number of bits needed to encode alphabet.
   26.5.2
   */
static int
perAlphabetBits(int N) /* number of characters in the effective permitted alphabet */
{
    if (N<=0)
    {
        msaPrintFormat(ErrPER, "perAlphabetBits: Illegal parameter N=%d.", N);
        return 0;
    }

    if (N <= 2)             return 1;
    else if (N <= 4)        return 2;
    else if (N <= 16)       return 4;
    else if (N <= 256)      return 8;
    else if (N <= 0x10000)  return 16;
    return 32;
}

static UINT32
perAlphabetMaxValue(char *alphabet,
            int alphabetSize,
            int charSize)
{
  UINT32 v=0;
  int i;

  if (charSize >4) return 0;

  for (i=0; i<charSize; i++)
  {
      v <<= 8;
      v += alphabet[(alphabetSize-1)*charSize + i];
  }
   /*  ((UINT8 *)&v)[4-charSize+i] = alphabet[(alphabetSize-1)*charSize + i]; */
  return v;
}


/* return position of letter in alphabet. */
static int
perAlphabetIndex(char *alphabet,
         int alphabetSize,
         int charSize,
         UINT8 *letter)
{
  int i, j;

  for (i=0; i<alphabetSize; i+=charSize) {
    for (j=0; j<charSize; j++)
      if ((UINT8)alphabet[i+j] == letter[j]) break;;
    if (j<charSize) return i/charSize;
  }
  return -1;
}




/*
  Desc: Encodes an character string into TAIL of buffer.
  clause '26'
  */
int
perEncodeCharString(IN  HPER hPer,
                    IN  int synParent,
                    IN  int valParent, /* this is me */
                    IN  INT32 fieldId)
{
    perStruct *per = (perStruct *)hPer;
    int length=0;
    INT32 index;
    UINT8 *data=per->buf->buffer;
    int numOfChars, charSize, b, i;
    UINT8 ch=0;
    INT32 ub; /* largest value in permitted alphabet */
    char *fromString=NULL;
    int fromStringSize=0;
    pstNodeType type;
    int to,from;
    
    type=pstGetNodeType(per->hSyn, synParent);
    pstGetNodeRange(per->hSyn, synParent,&from,&to);

    if (type != pstNumericString)
    {
        /* Get the "FROM" string from the syntax */
        fromString = pstGetFROMStringPtr(per->hSyn, synParent,&fromStringSize);
    }
    else
    {
        /* For NumericString types we know the FROM already... */
        fromString = (char*)" 0123456789";
        fromStringSize = 11;
    }

    pvtGet(per->hVal, valParent, NULL, NULL, (INT32*)&length, NULL);
    pvtGetString(per->hVal, valParent,length,(INT8*)data);
    if (bbFreeBytes(per->hBB) < (INT32)length)
    {
        msaPrintFormat(ErrPER, "perEncodeCharString: %s: buffer allocation error.",
            pstGetFieldNamePtr(per->hSyn, fieldId));
        return RVERROR;
    }
    
    /* -- not known-multiplier strings: 26.6 */
    if (perCharSize(type) <0)
    { /* not known-multiplier type */
        perEncodeLen(perLenTypeUNCONSTRAINED, length, 0, 0, per->hBB); /* length: 26.6.3 */
        return bbAddTail(per->hBB, data, length*8, TRUE); /* BER: 26.6.2 */
    }
    
    /* -- extension: 26.4 */
    if (pstGetIsExtended(per->hSyn, synParent) == TRUE)
    {
        if (length > to || length < from)
        { /* not within extension root range */
            perEncodeBool(TRUE, per->hBB);
            from = 0;
            to = 0 ; /* no effective size constraint */
            fromStringSize = 0; /* no effective PermittedAlphabet constraint */
        }
        else
            perEncodeBool(FALSE, per->hBB);
    }
    
    /* -- length: 26.5.6 - 26.5.7 */
    charSize = perCharSize(type);
    numOfChars = (int)(length / charSize);
    if (to > 0)
    {  /* size constraint exist */
        if (numOfChars > to || numOfChars < from)
        { /* not within extension root range */
            msaPrintFormat(ErrPER, "perEncodeCharString:%s length not within extension root range: %d<=%d<=%d.",
                pstGetFieldNamePtr(per->hSyn, fieldId), from, numOfChars, to);
            return RVERROR;
        }
    }
    
    /* if (length == 0) return TRUE;*/  /* 26.5 */
    
    if (to == 0) /* 26.5.7: unconstrained length. aub unset. */
        perEncodeLen(perLenTypeUNCONSTRAINED, numOfChars, 0, 0, per->hBB);
    if (from < to)  /* 26.5.7: length determinant. aub set */
        perEncodeLen(perLenTypeCONSTRAINED, numOfChars, from, to, per->hBB);
    
    /* -- string encoding */
    if (fromStringSize<1) b=8;
    else
        b = perAlphabetBits(fromStringSize); /* 26.5.2 */
    
    if ((numOfChars != 0) && /* 10.9.3.3 note 2 - no alignment if length of string is 0 */
        ((to*b > 16) || /* 26.5.6 alignment rule and part of 26.5.7 alignment rule */
        ((to*b == 16) && (from < to)))) /* 26.5.7 alignment rule */
    {
        /* Alignment is needed for this string value */
        if (bbAddTail(per->hBB, (UINT8 *)&ch, 0, TRUE)<0) /* align */
            return RVERROR;
    }

    if (fromStringSize <1)   /* no FROM restriction */
        return bbAddTail(per->hBB, data, numOfChars*charSize*8, FALSE);
    
    /* -- calc from */
    ub = perAlphabetMaxValue(fromString, fromStringSize, charSize);
    
    if (ub <= ipow2(b))  /* 26.5.4.a */
        return bbAddTail(per->hBB, data, length*8, FALSE);
    
    /* -- 26.5.4.b */
    for (i=0; i<numOfChars; i++)
    {
        index = perAlphabetIndex(fromString, fromStringSize, charSize, data+i*charSize);
        if (index <0)
        {
            msaPrintFormat(ErrPER, "perEncodeCharString: String character out of alphabet: '%.100s':%d [%s].",
                nprn(((char*)data)), i, nprn(fromString));
            return RVERROR;
        }
        perEncodeNumber(index, b, per->hBB);
    }
    
    return TRUE;
}



/*
  Desc: Decodes an character string from buffer (at position).
  Note: clause '26'
*/
int
perDecodeCharString(IN  HPER hPer,
                    IN  int synParent, /* parent in syntax tree */
                    IN  int valParent, /* field parent in value tree */
                    IN  INT32 fieldId)   /* enum of current field */
{
    perStruct *per = (perStruct *)hPer;
    
    INT32 ub;
    int length, numOfChars=0;
    BOOL isExtended = FALSE;
    UINT32 dec = 0; /* decoded bits */
    int vtPath;
    UINT8 *data=per->buf->buffer;
    int charSize, b, pos, j;
    int i;
    UINT32 index;
    char *fromString=NULL;
    int fromStringSize=0;
    pstNodeType type;
    int to,from;
    int extension;
    
    type=pstGetNodeType(per->hSyn, synParent);
    pstGetNodeRange(per->hSyn, synParent,&from,&to);

    if (type != pstNumericString)
    {
        /* Get the "FROM" string from the syntax */
        fromString = pstGetFROMStringPtr(per->hSyn, synParent,&fromStringSize);
    }
    else
    {
        /* For NumericString types we know the FROM already... */
        fromString = (char*)" 0123456789";
        fromStringSize = 11;
    }

    charSize = perCharSize(type);
    
    extension=pstGetIsExtended(per->hSyn,synParent);
    
    /* -- not known-multiplier strings: 26.6 */
    if (perCharSize(type) <0)
    {   /* not known-multiplier type */
        /* -- length: 26.6.3 */
        perDecodeLen(perLenTypeUNCONSTRAINED, (UINT32 *)&numOfChars, 0, 0, hPer, per->decodingPosition, &dec);
        per->decodingPosition += dec;
        length = numOfChars;
        
        if(bbGet2Left(hPer, per->decodingPosition, length*8, data)<0)
        {
            msaPrintFormat(ErrPER, "perDecodeCharString: bbGet2Left failed [%s]",
                pstGetFieldNamePtr(per->hSyn, fieldId));
            return RVERROR;
        }
        /* -- data in BER: 26.6.2 */
        vtPath=valParent;
        if (fieldId!=RVERROR)
        {
            if ((vtPath=pvtAdd(per->hVal, valParent, fieldId, length, (char*)data, NULL)) <0)
            {
                msaPrintFormat(ErrPER, "perDecodeCharString: cannot add string to value tree [%s:%d].",
                    pstGetFieldNamePtr(per->hSyn, fieldId), length);
                return RVERROR;
            }
        }
        per->decodingPosition += (length*8);
        return vtPath;
    }

    /* -- extension: 26.3 */
    if (extension == TRUE)
    {
        perDecodeBool(&isExtended, hPer, per->decodingPosition, &dec);
    }
    per->decodingPosition += dec;
    
    if (isExtended == TRUE)
    {   /* not within extension root range */
        from = 0;
        to = 0; /* no effective size constraint */
        fromStringSize = 0; /* no effective PermittedAlphabet constraint */
    }
    
    /* -- length */
    if (from == to && from >0) /* no length determinant */
        numOfChars = from;
    else
    { /* there is length */
        if (to == 0) /* unconstrained */
            perDecodeLen(perLenTypeUNCONSTRAINED,(UINT32 *)&numOfChars, 0, 0, hPer, per->decodingPosition, &dec);
        else
            perDecodeLen(perLenTypeCONSTRAINED,(UINT32 *)&numOfChars,
            from, to, hPer, per->decodingPosition, &dec);
        
        per->decodingPosition+= dec;
    }
    
    if (to>0 && (numOfChars > to|| numOfChars < from))
    { /* not within extension root range */
        msaPrintFormat(ErrPER, "perDecodeCharString: %s length not within extension root range: %d<=%d<=%d.",
            pstGetFieldNamePtr(per->hSyn, fieldId), from, numOfChars, to);
        return RVERROR;
    }
    
    
    /* -- string */
    if (fromStringSize<1)   b = 8;
    else                    b = perAlphabetBits(fromStringSize); /* 26.5.2 */
    
    if ((numOfChars != 0) && /* 10.9.3.3 note 2 - no alignment if length of string is 0 */
        ((to*b > 16) || /* 26.5.6 alignment rule and part of 26.5.7 alignment rule */
        ((to*b == 16) && (from < to)))) /* 26.5.7 alignment rule */
        per->decodingPosition += bbAlignBits(per->hBB, (INT32)(per->decodingPosition)); /* alignment */
        
    /* -- add value node */
    length = numOfChars*charSize;
    if (fromStringSize <1)
    {  /* no FROM restriction */
        if(bbGet2Left(hPer, per->decodingPosition, length*8, data)<0)
        {
            msaPrintFormat(ErrPER, "perDecodeCharString2: bbGet2Left failed [%s]",
                pstGetFieldNamePtr(per->hSyn, fieldId));
            return RVERROR;
        }
        per->decodingPosition += (length*8);
    }
    else
    {
        /* -- calc. from */
        ub = perAlphabetMaxValue(fromString, fromStringSize, charSize);
        
        if (ub <= ipow2(b))
        { /* 26.5.4.a */
            if(bbGet2Left(hPer, per->decodingPosition, length*8, data)<0)
            {
                msaPrintFormat(ErrPER, "perDecodeCharString3: bbGet2Left failed [%s]",
                    pstGetFieldNamePtr(per->hSyn, fieldId));
                return RVERROR;
            }
            per->decodingPosition += (length*8);
        }
        else
        {
            /* -- 26.5.4.b */
            pos=0;
            for (i=0; i<numOfChars; i++)
            {
                if (perDecodeNumber(&index, b, hPer, per->decodingPosition, &dec) <0)
                    return RVERROR;
                per->decodingPosition+=dec;
                for (j=0; j<charSize; j++)   /* get real character */
                    data[pos++] = fromString[index*charSize+j];
            }
        }
    }
    vtPath=valParent;
    if (fieldId!=RVERROR)
    {
        if ((vtPath= pvtAdd(per->hVal, valParent, fieldId, length, (char*)data, NULL)) <0)
        {
            msaPrintFormat(ErrPER, "perDecodeCharString:1: cannot add string to value tree [%s:%d].",
                pstGetFieldNamePtr(per->hSyn, fieldId), length);
            return RVERROR;
        }
    }
    return vtPath;
}

#ifdef __cplusplus
}
#endif



