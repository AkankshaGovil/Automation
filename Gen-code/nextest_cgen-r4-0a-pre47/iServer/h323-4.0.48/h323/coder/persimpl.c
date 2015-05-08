#ifdef __cplusplus
extern "C"
{
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
    perSimple.c

    Encoding Rules are ASN.1 PER BASIC - ALIGNED.
    Encoding/Decoding of ASN simple types.
    Integer, length, null, boolean, octet string.

    Note: Open type fields are encoded into a separate bit buffer and then added
    to the complete encoding as an octet string with unconstrained length.

    Note: Bit order should be: Most Significant Bit(MSB) first.
    Byte order ...

    Encoding: Add to tail of buffer.
    Decoding: Decode from position in buffer. Returns the number of bits being decoded.
    Does not change the buffer.
    Uses user allocated memory for integers and strings(sra).


    Parameters:
    n: number.
    lb: lower bound. -1 if no constraint.
    ub: upper bound. -1 if no constraint.
    from: position in bit buffer from which to decode.
    decoded: number of bits successfully decoded from buffer.
    bbH: bit buffer handler.

*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <rvinternal.h>
#include <perintr.h> /* for msa */
#include <ms.h>

#include <persimpl.h>



#define K64    65536lu     /* 64K */
#define BIT24  16777216lu /* 2^24 */
#define BIT32  4294967295lu /* 2^32 - 1 */

#define DELCHECK(ret)((ret < 0) ? (0):(ret))

#ifdef _PERINTMAIN_
    static void TestPeek(bitBuf *buf1, bitBuf *buf2, int len);
#endif

    int
        perEncodeSemiConstInt(IN  UINT32 n, /* the number */
              IN  INT32 lb,
              OUT HBB bbH,
              OUT UINT32 *length);  /* in octets */


    int
        perDecodeSemiConstInt(IN  UINT32 *n,
              IN  INT32 lb,
              IN  INT32 length, /* in octets */
              IN  HPER hPer,
              IN  UINT32 from,
              OUT UINT32 *dec);


              /* Description: Returns the exact number of bits for a constrained
              integer with defined range.
              Inupt: range- ub - lb + 1
              Returns: The number of bits for the integer. or -1 if range too
              large to conclude the size;
    */

    static BYTE
        perIntBits(UINT32 range)
    {
        static const char lut[]=
        {
            0,
                0, 1, 2, 2, 3, 3, 3, 3, 4, 4, 4, 4, 4, 4, 4, 4,
                5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5,
                6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6,
                6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6,
                7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7,
                7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7,
                7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7,
                7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7,
                8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8,
                8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8,
                8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8,
                8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8,
                8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8,
                8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8,
                8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8,
                8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8
        };





        if (range <= 256)
            return lut[range];

        if (range <= K64)
            return 2*8;
        if (range <= BIT24)
            return 3*8;
        return 4*8;
    }



    /*___________________________________________________________________________*/
    /*________________________Constraigned_whole_Integer_________________________*/
    /*___________________________________________________________________________*/
    /*
    Desc: Encodes an integer and puts it in TAIL of bit buffer.
    Returns: Failure if cannnot encode n.
    */
    int
        perEncodeInt(IN  UINT32 n,
        IN  UINT32 lb,
        IN  UINT32 ub,
        IN  BOOL isFromAbsent,
        IN  BOOL isToAbsent,
        IN  BOOL isExtension,
        OUT HBB bbH) /* Should be initialized with sufficient size.
                 It has to have at least 4 bytes size. */
    {
        BOOL isExtended = FALSE;
        UINT32 num = n - lb;
        UINT32 tmpNum;
        UINT32 range = ub - lb + 1;
        unsigned char ch; /* for case 10.5.7.1 */
        unsigned char numOct[MAX_INT_SIZE]; /*num converted to array in BIG ENDIAN*/
        int bits;

        if (lb == 0 && ub == BIT32)
            range = (UINT32)BIT32;

        if (!bbH || bbFreeBytes(bbH) < MAX_INT_SIZE)
        {
            msaPrintFormat(ErrPER, "per:EncodeInt: Buffer allocation error");
            return RVERROR; /* Buffer allocation error */
        }

        if (isExtension)
        {
            if (((!isFromAbsent) && n<lb) ||((!isToAbsent) && ub < n))
            {
                isExtended = TRUE;
                perEncodeBool(TRUE, bbH); /* adding extension bit. Sergey M. */
            }
            else
                perEncodeBool(FALSE, bbH);
        }



            /* Unconstrained or Semiconstrained INTEGER. Sergey M. */
            if (isToAbsent != FALSE || isExtended)
            {
                if (n == 0)
                    bits = 1;
                else
                    bits = perIntBits(n + 1);
                if (isFromAbsent != FALSE && (bits==8 || (bits==16 && (n&0x8000)) || (bits==24 && (n&0x800000))))
                    bits++;
                ch =(unsigned char)bbSetByte(bits);

                perEncodeLen(perLenTypeUNCONSTRAINED, (UINT32)ch, 0, 0, bbH);

                if (isFromAbsent != FALSE || isExtended) /* Unconstrained INTEGER. Sergey M. */
                {
                    /* Convert the number to 4 bytes in BIG ENDIAN order */
                    tmpNum = n; /* 4 bytes integer */
                    numOct[0] =(unsigned char)(tmpNum >> 24); /* n / 2^24 */
                    tmpNum -= numOct[0] << 24;
                    numOct[1] =(unsigned char)(tmpNum >> 16); /* n / 2^16 */
                    tmpNum -= numOct[1] << 16;
                    numOct[2] =(unsigned char)(tmpNum >> 8); /* n / 2^8 */
                    tmpNum -= numOct[2] << 8;
                    numOct[3] =(unsigned char)tmpNum;
                    return bbAddTail(bbH, (UINT8 *)(numOct + (MAX_INT_SIZE - ch)), ch*8, TRUE);
                }
                else
                {
                    if (n < lb)
                    {
                        msaPrintFormat(ErrPER, "per:EncodeInt: Integer out of range [%d <= %d ", lb, n);
                        return RVERROR; /* Range Error */
                    }
                    return perEncodeSemiConstInt(n, (INT32)lb, bbH, NULL);
                }
            } /* End of unconstrained or semiconstrained INTEGER. Sergey M. */

            if (ub == lb)
                return TRUE; /* no encoding needed */



            /* Irina * changes for negative numbers */
  if (/*lb > ub || n < lb ||  n > ub */ n-lb > ub-lb) {
    if ((int)ub>(int)lb)
    {
        msaPrintFormat(ErrPER, "per:EncodeInt: Integer out of range [%d <= %d <= %d]", lb, n, ub);
    }
    else
        msaPrintFormat(ErrPER, "per:EncodeInt: Integer out of range [%u <= %u <= %u]", lb, n, ub);
                return RVERROR; /* Range Error */
            }


            bits = perIntBits(range);

            /* range <= 255: 10.5.7.1 */
            if (range <= 255)
            {
                ch =(unsigned char)num;
                ch =(unsigned char)(ch <<(8 - bits));
                return bbAddTail(bbH, (UINT8 *)&ch, bits, FALSE);
            }

            /* range = 256: 10.5.7.2 */
            if (range == 256)
            {
                ch =(unsigned char)num;
                return bbAddTail(bbH, (UINT8 *)&ch, 8, TRUE);
            }

            /* Convert the number to 4 bytes in BIG ENDIAN order */
            tmpNum = num; /* 4 bytes integer */
            numOct[0] =(unsigned char)(tmpNum >> 24); /* n / 2^24 */
            tmpNum -= numOct[0] << 24;
            numOct[1] =(unsigned char)(tmpNum >> 16); /* n / 2^16 */
            tmpNum -= numOct[1] << 16;
            numOct[2] =(unsigned char)(tmpNum >> 8); /* n / 2^8 */
            tmpNum -= numOct[2] << 8;
            numOct[3] =(unsigned char)tmpNum;

            /* range <=K64: 10.5.7.3; 2 octets aligned */
            if (range <= K64)
            {
                /* 256 < num <= 65536 */
                if (bits/8 < 0 || bits/8 > MAX_INT_SIZE)
                    return RVERROR;
                return bbAddTail(bbH, (UINT8 *)(numOct + (MAX_INT_SIZE - bits/8)), bits, TRUE);
            }

            /* range > K64: 10.5.7.4: indefinite length case */
            /* These cases require additional length field */
            /* The length shall be encoded according to clause 10.9.3.6 and be treated
            as an octet - aligned - bit - field. The length is <= 127.  */

            bits = perIntBits(num);
            ch =(unsigned char)bbSetByte(bits);
            if (ch == 0)
                ch = 1; /* at least 1 octet */

            /* length: 12.2.6a */
            perEncodeLen(perLenTypeCONSTRAINED, (UINT32)ch, 1, bbSetByte(perIntBits(ub)), bbH);
            return bbAddTail(bbH, (UINT8 *)(numOct + (MAX_INT_SIZE - ch)), ch*8, TRUE); /* number */
}



int /* TRUE or RVERROR */
perDecodeInt(
             /* Decodes an integer from a bit buffer. */
             OUT UINT32 *n, /* decoded number */
             IN  UINT32 lb,
             IN  UINT32 ub,
             IN  BOOL isFromAbsent,
             IN  BOOL isToAbsent,
             IN  BOOL isExtension,
             IN  HPER hPer,
             IN  UINT32 from, /* position in buffer */
             OUT UINT32 *decoded,
             IN  char* desc /* short description(optional) */
             )
{
    BOOL isExtended = FALSE;
    perStruct *per =(perStruct *)hPer;

    UINT32 num;
    UINT32 extract, bits, bytes;
    UINT32 range = ub - lb + 1;
    UINT8 numOct[MAX_INT_SIZE]; /*num converted to array in BIG ENDIAN*/
    UINT32 myFrom = from;

    memset((char *)numOct, 0, MAX_INT_SIZE);

    if (lb == 0 && ub == BIT32)
        range = (UINT32)BIT32;

    if (!n || !decoded)
    {
        msaPrintFormat(ErrPER, "per:DecodeInt:%s: parameters (n,del) not allocated.", nprn(desc));
        return RVERROR;
    }
    *n = 0;
    *decoded = 0;

    if (!(per->hBB))
    {
        msaPrintFormat(ErrPER, "per:DecodeInt:%s: Buffer allocation error.", nprn(desc));
        return RVERROR; /* Buffer allocation error */
    }

    if (isExtension)
    {
        perDecodeBool(&isExtended, hPer, from, decoded); /* decoding extension bit. Sergey M. */
        myFrom += *decoded;
    }

    /* Unconstrained or Semiconstrained INTEGER. Sergey M. */
    if (isToAbsent !=  FALSE || isExtended)
    {
        perDecodeLen(perLenTypeUNCONSTRAINED, (UINT32 *)&bytes, 0, 0, hPer, myFrom, &extract);
        bits = bytes*8; /* set # bits to extract */
        *decoded += extract;
        myFrom += extract;
        *decoded += bbAlignBits(per->hBB, (INT32)myFrom);
        myFrom += bbAlignBits(per->hBB, (INT32)myFrom); /* move to alignment */
        if (myFrom + bits > bbBitsInUse(per->hBB))
        {
            msaPrintFormat(ErrPER, "per:DecodeInt:%s: Integer too large to decode from buffer [len=%ld]",
                nprn(desc), bits);
            return RVERROR;
        }

        if (isFromAbsent !=  FALSE || isExtended) /* Unconstrained INTEGER. Sergey M. */
        {
            if ((INT32)(extract = bbGet2Right(hPer, myFrom, bits, (UINT8 *)(numOct + (MAX_INT_SIZE - bytes)))) <0)
            {
                msaPrintFormat(ErrPER, "per:DecodeInt:%s: Extracting failed [%d]", nprn(desc), extract);
                return RVERROR;
            }

            num = numOct[3] +((UINT32)numOct[2] << 8) +((UINT32)numOct[1] << 16) +((UINT32)numOct[0] << 24);
            /*num += lb; */

            *decoded += extract;

            *n = num;

            msaPrintFormat(InfoPER, " >> [%lu..%lu] (%lu:%lu) => %lu \t \t %s.",
                lb, ub, from, *decoded, *n, nprn(desc));
            return TRUE;
        }
        else /* Semiconstrained INTEGER. Sergey M. */
        {
            perDecodeSemiConstInt(n, (INT32)lb, (INT32)bytes, hPer, myFrom, &extract);
            *decoded += extract;
            msaPrintFormat(InfoPER, " >> [%lu....] (%lu:%lu) => %lu \t \t %s.",
                lb, from, *decoded, *n, nprn(desc));
            return TRUE;
        }
    } /* end of Unconstrainted or Semiconstrained INTEGER. Sergey M. */


    if (ub == lb) /* no encoding needed */
    {
        *n = lb;
        return TRUE;
    }


    bits = perIntBits(range);
    bytes = bbSetByte(bits);
    if (bytes > MAX_INT_SIZE)
    {
        msaPrintFormat(ErrPER, "per:DecodeInt:%s: Number of bits in integer error [%ld]", nprn(desc), bits);
        return RVERROR;
    }

    if (bits > 16)
    {
        /* Extract length */
        /* 12.2.6a */
        perDecodeLen(perLenTypeCONSTRAINED, (UINT32 *)&bytes, 1, bbSetByte(perIntBits(ub)), hPer, myFrom, &extract);
        if (bytes > MAX_INT_SIZE)
        {
            msaPrintFormat(ErrPER, "perDecodeInt:%s: Integer value too large to decode [%ld].", nprn(desc), bytes);
            return RVERROR;
        }
        bits = bytes*8; /* set # bits to extract */
        *decoded += extract;
        myFrom += extract;
    }

    if (range >= 256)
    {
        *decoded += bbAlignBits(per->hBB, (INT32)myFrom);
        myFrom += bbAlignBits(per->hBB, (INT32)myFrom); /* move to alignment */
    }

    if (myFrom + bits > bbBitsInUse(per->hBB))
    {
        msaPrintFormat(ErrPER, "per:DecodeInt:%s: Integer too large to decode from buffer [len=%ld]",
            nprn(desc), bits);
        return RVERROR;
    }

    if ((INT32)(extract = bbGet2Right(hPer, myFrom, bits, (UINT8 *)(numOct + (MAX_INT_SIZE - bytes)))) <0)
    {
        msaPrintFormat(ErrPER, "per:DecodeInt:%s: Extracting failed [%d]", nprn(desc), extract);
        return RVERROR;
    }

    num = numOct[3] +((UINT32)numOct[2] << 8) +((UINT32)numOct[1] << 16) +((UINT32)numOct[0] << 24);
    num += lb;

    if (num - lb > ub - lb)
    {
        msaPrintFormat(ErrPER, "per:DecodeInt:%s: Decoded number out of range [%ld]", nprn(desc), num);
        return RVERROR;
    }

    *decoded += extract;

    *n = num;

    msaPrintFormat(InfoPER, " >> [%lu..%lu] (%lu:%lu) => %lu \t \t %s.",
        lb, ub, from, *decoded, *n, nprn(desc));
    return TRUE;
}




/*___________________________________________________________________________*/
/*________________________Constraigned_non_negative_integer__________________*/
/*___________________________________________________________________________*/
/*
Desc: Encodes an integer into b bits and puts it in TAIL of bit buffer.
Returns: Failure if cannnot encode n.
Note: n <=2^32
*/
int
perEncodeNumber(IN  UINT32 n,
                IN  UINT32 b,  /* number of bits to hold the encoding */
                OUT HBB bbH) /* Should be initialized with sufficient size. */
{
    UINT32 tmpNum;
    unsigned char numOct[MAX_INT_SIZE]; /*num converted to array in BIG ENDIAN*/

    if (b>MAX_INT_SIZE*8)
    {
        msaPrintFormat(ErrPER, "perDecodeNumber: Encoding not supported for %d bits.", b);
        return RVERROR;
    }

    if (!bbH || bbFreeBits(bbH) < (INT32)b)
    {
        msaPrintFormat(ErrPER, "perEncodeNumber: Buffer allocation error. [%d bits]", b);
        return RVERROR; /* Buffer allocation error */
    }

    /* Convert the number to 4 bytes in BIG ENDIAN order */
    tmpNum = n; /* 4 bytes integer */
    numOct[0] =(unsigned char)(tmpNum >> 24); /* n / 2^24 */
    tmpNum -= numOct[0] << 24;
    numOct[1] =(unsigned char)(tmpNum >> 16); /* n / 2^16 */
    tmpNum -= numOct[1] << 16;
    numOct[2] =(unsigned char)(tmpNum >> 8); /* n / 2^8 */
    tmpNum -= numOct[2] << 8;
    numOct[3] =(unsigned char)(tmpNum);

    return bbAddTailFrom(bbH, (UINT8 *)numOct, MAX_INT_SIZE*8 - b, b, FALSE);
}



/*
Desc: Decodes an integer from a bit buffer.
Returns: RVERROR ot TRUE.
*/
int
perDecodeNumber(OUT UINT32 *n, /* decoded number */
                IN  UINT32 b,  /* number of bits to hold the encoding */
                IN  HPER hPer,
                IN  UINT32 from, /* position in buffer */
                OUT UINT32 *decoded)
{
    perStruct *per =(perStruct *)hPer;

    UINT8 numOct[MAX_INT_SIZE]; /*num converted to array in BIG ENDIAN*/
    unsigned int bytes;

    memset((char *)numOct, 0, MAX_INT_SIZE);

    if (!n || !decoded)
    {
        msaPrintFormat(ErrPER, "perDecodeNumber: parameters (n,dec) not allocated.");
        return RVERROR;
    }
    *n = 0;
    *decoded = 0;

    if (b>MAX_INT_SIZE*8)
    {
        msaPrintFormat(ErrPER, "perDecodeNumber: Encoding not supported for %d bits.", b);
        return RVERROR;
    }

    if (!(per->hBB))
    {
        msaPrintFormat(ErrPER, "perDecodeNumber: Buffer allocation error.");
        return RVERROR; /* Buffer allocation error */
    }

    if (from + b > bbBitsInUse(per->hBB))
    {
        msaPrintFormat(ErrPER, "perDecodeNumber: Number too large to decode from buffer [len=%d]",
            b);
        return RVERROR;
    }

    bytes = bbSetByte(b);
    if ((UINT32)bbGet2Right(hPer, from, b, (UINT8 *)(numOct + (MAX_INT_SIZE - bytes))) !=b)
    {
        msaPrintFormat(ErrPER, "per:DecodeInt: Extracting failed [%d]", b);
        return RVERROR;
    }

    *n = numOct[3] +(numOct[2] << 8) +(numOct[1] << 16) +(numOct[0] << 24);
    *decoded = b;

    msaPrintFormat(InfoPER, " >> [%d] (%ld:%ld) => %d. \t \t number", b, from, *decoded, *n);
    return TRUE;
}





/*___________________________________________________________________________*/
/*________________________________normally small_____________________________*/
/*___________________________________________________________________________*/


/*
clause '10.7'
*/
int
perEncodeSemiConstInt(IN  UINT32 n, /* the number */
                      IN  INT32 lb,
                      OUT HBB bbH,
                      OUT UINT32 *length)  /* in octets */
{
    UINT32 num, tmpNum, len;
    unsigned char numOct[MAX_INT_SIZE]; /*num converted to array in BIG ENDIAN*/

    if ((UINT32)lb >n)
        return RVERROR;
    num = n - lb; /* to be encoded */

    /* 10.3: octet - aligned - bit - field as non - negative - binary - integer. */

    /* Convert the number to 4 bytes in BIG ENDIAN order */
    tmpNum = num; /* 4 bytes integer */
    numOct[0] =(unsigned char)(tmpNum >> 24); /* n / 2^24 */
    tmpNum -= numOct[0] << 24;
    numOct[1] =(unsigned char)(tmpNum >> 16); /* n / 2^16 */
    tmpNum -= numOct[1] << 16;
    numOct[2] =(unsigned char)(tmpNum >> 8); /* n / 2^8 */
    tmpNum -= numOct[2] << 8;
    numOct[3] =(unsigned char)tmpNum;

    len = bbSetByte(num);
    if (len == 0)
        len = 1; /* at least 1 octet */
    if (length)
        *length = len;
    return bbAddTail(bbH, (UINT8 *)(numOct + (MAX_INT_SIZE - len)), len*8, TRUE);
}


int
perDecodeSemiConstInt(IN  UINT32 *n,
                      IN  INT32 lb,
                      IN  INT32 length, /* in octets */
                      IN  HPER hPer,
                      IN  UINT32 from,
                      OUT UINT32 *dec)
{
    UINT8 numOct[MAX_INT_SIZE]; /*num converted to array in BIG ENDIAN*/
    UINT32 num;
    INT32 extract;

    if (length>MAX_INT_SIZE)
    {
        return RVERROR;
    }

    memset((char *)numOct, 0, MAX_INT_SIZE);

    if ((extract = bbGet2Right(hPer, from, length*8, (UINT8 *)(numOct + (MAX_INT_SIZE - length)))) <0)
    {
        msaPrintFormat(ErrPER, "perDecodeSemiConstInt: Extracting failed [%d]", extract);
        return RVERROR;
    }

    num = numOct[3] +(numOct[2] << 8) +(numOct[1] << 16) +(numOct[0] << 24);
    num += lb;
    *n = num;
    *dec = extract;
    return TRUE;
}




/*___________________________________________________________________________*/
/*________________________________normally small_____________________________*/
/*___________________________________________________________________________*/

/*
clause '10.6'
*/
int
perEncodeNormallySmallInt(IN  UINT32 n, /* the number */
                          OUT HBB bbH)
{
    UINT8 octets[32]; /* space for tmp encoding */
    UINT32 length=0;
    INT32 offset;
    HBB tmpBB; /* for length encoding */
    INT32 tmpBBlen;

    if (n <= 63)
    {
        /* 10.6.1 */

        perEncodeInt(0, 0, 1, FALSE, FALSE, FALSE, bbH);
        return perEncodeInt(n, 0, 63, FALSE, FALSE, FALSE, bbH);
    }

    /* 10.6.2:  ... | 1 | length | n | */
#ifdef RV_CODER_DEBUG
    if (bbGetAllocationSize(10) > (int)sizeof(octets))
    {
        msaPrintFormat(ErrPER, "perEncodeNormallySmallInt: Allocation space for length not enough [%d].",
            bbGetAllocationSize(10));
        return RVERROR;
    }
#endif  /* RV_CODER_DEBUG */
    tmpBB = bbConstructFrom(10, (INT8 *)octets, 32);

    perEncodeInt(1, 0, 1, FALSE, FALSE, FALSE, bbH);
    offset = bbBitsInUse(bbH);
    perEncodeSemiConstInt(n, 0, bbH, &length);

    if (perEncodeLen(perLenTypeNORMALLY_SMALL, length, 0, 0, tmpBB) <0)
    {
        msaPrintFormat(ErrPER, "perEncodeNormallySmallInt: length encoding failed for '%d'.", length);
        return RVERROR;
    }

    tmpBBlen = bbBitsInUse(tmpBB);
    if (bbMove(bbH, offset, length, offset + tmpBBlen) < 0)
        return RVERROR;
    if (bbSet(bbH, offset, tmpBBlen, bbOctets(tmpBB)) <0)
    {
        msaPrintFormat(ErrPER, "perEncodeNormallySmallInt: set failed for '%d'.", length);
        return RVERROR;
    }

    return TRUE;
}


int
perDecodeNormallySmallInt(OUT UINT32 *n, /* the number */
                          IN  HPER hPer,
                          IN  UINT32 from,
                          OUT UINT32 *dec)
{
    BOOL boola = FALSE;
    UINT32 myDec = FALSE;
    UINT32 length;

    perDecodeBool(&boola, hPer, from, &myDec);
    *dec = myDec;
    if (boola == FALSE)
    {
        /* 10.6.1 */
        perDecodeInt(n, 0, 63, FALSE, FALSE, FALSE, hPer, from + myDec, &myDec, (char*)"normally small int");
        *dec += myDec;
        return TRUE;
    }

    /* 10.6.2 */
    perDecodeLen(perLenTypeNORMALLY_SMALL, &length, 0, 0, hPer, from, &myDec);
    *dec += myDec;
    perDecodeSemiConstInt(n, 0, (INT32)length, hPer, from + (*dec), &myDec);
    *dec += myDec;
    return TRUE;
}




/*___________________________________________________________________________*/
/*________________________________Length_____________________________________*/
/*___________________________________________________________________________*/
/*
Desc: Encodes length and puts it in the TAIL of bit buffer.
Returns: Failure if cannnot encode n.
Note: clause 10.9
as CONSTRAINED: ub <= 64K.
as UNCONSTRAINED: n < 16K.
*/
int
perEncodeLen(IN  perLenType type,  /* CONSTRAINED, NORMALLY_SMALL, UNCONSTRAINED */
             IN  UINT32 n,  /* the length */
             IN  UINT32 lb,  /* only for constrained type */
             IN  UINT32 ub,  /* only for constrained type */
                             OUT HBB bbH) /* Should be initialized with sufficient size.
                             It has to have at least 4 bytes size. */
{
    if (bbFreeBytes(bbH) <MAX_INT_SIZE)
    {
        msaPrintFormat(ErrPER, "per:EncodeLen: Buffer allocation error.");
        return RVERROR; /* Buffer allocation error */
    }

    if (type == perLenTypeCONSTRAINED && ub < K64) /* 10.9.3.3 */
        return perEncodeInt(n, lb, ub, FALSE, FALSE, FALSE, bbH);

    if (type == perLenTypeNORMALLY_SMALL) /* 10.9.3.4 extension addition bitmap length of set or sequence type */
    {
        if (n <= 64)
        {
            perEncodeInt(0, 0, 1, FALSE, FALSE, FALSE, bbH);
            return perEncodeInt(n - 1, 0, 63, FALSE, FALSE, FALSE, bbH);
        }
        else
            perEncodeInt(1, 0, 1, FALSE, FALSE, FALSE, bbH);
    }


        /* unconstrained length 10.9.3.5 */
        lb = 0;

        if (n <= 127) /* 10.9.3.6 */
            return perEncodeInt(n, 0, 255, FALSE, FALSE, FALSE, bbH); /* 1 octet aligned. 10.5.7.2 */
        if (n < 16384)  /* 10.9.3.7 */
            return perEncodeInt(n + 0x8000, 0, (UINT32)(K64 - 1), FALSE, FALSE, FALSE, bbH); /* 2 octet aligned */


        /* fragmentation procedure */
        if (n >= 16384)
        {
            msaPrintFormat(ErrPER, "perEncodeLen: fragmentation procedure not available [%d]", n);
        }

        /* not reached */
        return RVERROR;
}


/*
Desc: Dencodes length from the bit buffer.(at position).
Returns: RVERROR or TRUE.
Note: clause 10.9
*/
int
perDecodeLen(IN  perLenType type, /* CONSTRAINED, NORMALLY_SMALL, UNCONSTRAINED */
             OUT UINT32 *n, /* the length */
             IN  UINT32 lb,
             IN  UINT32 ub,
             OUT HPER hPer,
             IN  UINT32 from, /* position in buffer */
             OUT UINT32 *decoded)
{
    INT32 num;
    int ret;
    UINT32 flag;
    UINT32 dec = 0;

    if (!n || !decoded)
    {
        msaPrintFormat(ErrPER, "per:DecodeLen: parameters (n,del) not allocated.");
        return RVERROR;
    }
    *n = 0;
    *decoded = 0;

    if (type == perLenTypeCONSTRAINED && ub < K64) /* 10.9.3.3 */
    {
        return perDecodeInt(n, lb, ub, FALSE, FALSE, FALSE, hPer, from, decoded, (char*)"constrained length");
    }

    if (type == perLenTypeNORMALLY_SMALL)
    {
        ret = perDecodeInt(&flag, 0, 1, FALSE, FALSE, FALSE, hPer, from, &dec, (char*)"normally small length (1st bit)");

        *decoded += dec;
        from += dec;
        if (ret < 0)
            return ret; /* error decoding int. */

        if (!flag)
        {
            /* 10.9.3.4: n <= 64 */

            ret = perDecodeInt(n, 0, 63, FALSE, FALSE, FALSE, hPer, from, &dec, (char*)"normally small length");

            *decoded += dec;
            from += dec;
            (*n)++;
            return ret;
        }
    }

    /* unconstrained length 10.9.3.5 */

    ret = perDecodeInt(&flag, 0, 255, FALSE, FALSE, FALSE, hPer, from, &dec, (char*)"unconstrained length (1st byte)");  /* decode 1 aligned octet */

    *decoded += dec;
    from += dec;
    if (ret < 0)
        return ret;
    if (!(flag & 1 << 7))
    {
        /* 1st bit =0 ==> 1 octet aligned. 10.5.7.2 */
        *n = flag;
        return TRUE;
    }
    if (!(flag & 1 << 6))
    {
        /* bits = 10 ==> 10.9.3.7: n < 16384 */
        num = flag & ~(1 << 7);
        num = num << 8;

        ret = perDecodeInt(&flag, 0, 255, FALSE, FALSE, FALSE, hPer, from, &dec, (char*)"unconstrained length (2 bytes)");  /* decode 1 aligned octet */

        *decoded += dec;
        from += dec;
        *n = num + flag;
        return TRUE;
    }

    msaPrintFormat(ErrPER, "perDecodeLen: fragmentation procedure not available [%d]", *n);
    return RVERROR;
}

/*___________________________________________________________________________*/
/*________________________________Boolean____________________________________*/
/*___________________________________________________________________________*/
/*
Desc: Encodes boolean and puts it in the TAIL of bit buffer.
Returns: Failure if cannnot encode n.
Note: clause 11
*/
int
perEncodeBool(IN  BOOL n,
OUT HBB bbH) /* Should be initialized with sufficient size.
It has to have at least 1 byte size. */
{
    if (bbFreeBytes(bbH) < MAX_INT_SIZE)
    {
        msaPrintFormat(ErrPER, "per:EncodeBool: Buffer allocation error.");
        return RVERROR; /* Buffer allocation error */
    }

    if (n == TRUE)

        return perEncodeInt(1, 0, 1, FALSE, FALSE, FALSE, bbH);

    else if (n == FALSE)
        return perEncodeInt(0, 0, 1, FALSE, FALSE, FALSE, bbH);

    else
        msaPrintFormat(ErrPER, "perEncodeBool: Illegal value %d", n);
    return RVERROR;
}


/*
Desc: Decodes boolean from buffer(at position).
Returns: RVERROR or positive number.
Note: clause 10.9
Note: clause 11
*/
int
perDecodeBool(OUT BOOL *n,
              IN  HPER hPer,
              IN  UINT32 from,
              OUT UINT32 *decoded)
{
    UINT32 num;
    int ret;

    if (!n)
        return RVERROR;

    ret = perDecodeInt(&num, 0, 1, FALSE, FALSE, FALSE, hPer, from, decoded, (char*)"boolean");

    if (num)
        *n = TRUE;
    else
        *n = FALSE;

    return ret;
}

/*___________________________________________________________________________*/
/*________________________________NULL_______________________________________*/
/*___________________________________________________________________________*/
int
perEncodeNull(HBB bbH)
{
    if (bbH)
        ;

    return TRUE;
}

int
perDecodeNull(HBB bbH)
{
    if (bbH)
        ;

    return TRUE;
}
#ifdef __cplusplus
}
#endif



