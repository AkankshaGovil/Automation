
/*

NOTICE:
This document contains information that is proprietary to RADVISION LTD..
No part of this publication may be reproduced in any form whatsoever without
written prior approval by RADVISION LTD..

RADVISION LTD. reserves the right to revise this publication and make changes
without obligation to notify any person of such revisions or changes.

*/

#ifdef __cplusplus
extern "C" {
#endif


#include <bb.h>
#include <perintr.h>
#include <rasdecoder.h>



/************************************************************************
 * rasDecoderInit
 * purpose: Initialize the decoder. Checks out the information inside
 *          the PST tree to allow faster runtime decoding of the first
 *          2 fields.
 * input  : ras             - RAS instance to use
 * output : none
 * return : Non-negative value on success
 *          Negative value on failure
 ************************************************************************/
int rasDecoderInit(IN  rasModule*  ras)
{
    pstChild synMsg, synField;
    UINT32 i;
    int rootId, j;

    ras->decoder.firstExtField = 26;

    /* Find out the number of messsages */
    rootId = pstGetNodeIdByPath(ras->synMessage, NULL);
    ras->decoder.numOfMessages = pstGetNumberOfChildren(ras->synMessage, rootId);

    /* Allocate a place for the number of OPTIONAL fields per message */
    ras->decoder.numOfOptFields = (UINT32*)malloc(sizeof(UINT32) * ras->decoder.numOfMessages);

    for (i = 0; i < ras->decoder.numOfMessages; i++)
    {
        ras->decoder.numOfOptFields[i] = 0;
        pstGetChild(ras->synMessage, rootId, (int)(i + 1), &synMsg);

        /* Find out the number of OPTINAL fields for this specific message */
        for (j = 0; j < pstGetNumberOfChildren(ras->synMessage, synMsg.nodeId); j++)
        {
            /* We count only fields that are not after ... that are OPTIONAL */
            pstGetChild(ras->synMessage, synMsg.nodeId, j + 1, &synField);
            if ((pstChildIsExtended(ras->synMessage, synMsg.nodeId, j + 1) == FALSE) &&
                (synField.isOptional == TRUE))
                ras->decoder.numOfOptFields[i]++;
        }
    }

    return 0;
}


/************************************************************************
 * rasDecoderEnd
 * purpose: Deinitialize the decoder.
 * input  : ras             - RAS instance to use
 * output : none
 * return : Non-negative value on success
 *          Negative value on failure
 ************************************************************************/
int rasDecoderEnd(IN  rasModule*  ras)
{
    if (ras->decoder.numOfOptFields != NULL)
        free(ras->decoder.numOfOptFields);
    return 0;
}



/************************************************************************
 * rasDecodePart
 * purpose: Decode the first 2 fields of an incoming message. This way
 *          We know the message's type and its sequence number.
 * input  : ras             - RAS instance to use
 *          messageBuffer   - The encoded message to check
 *          messageSize     - The size of the message in bytes
 * output : index           - Index of the RAS message
 *          requestSeqNum   - Request sequence number of the message
 * return : Non-negative value on success
 *          Negative value on failure
 ************************************************************************/
int rasDecodePart(
    IN  rasModule*  ras,
    IN  BYTE*       messageBuffer,
    IN  UINT32      messageSize,
    OUT UINT32*     index,
    OUT UINT32*     requestSeqNum)
{
    UINT8       octets[64]; /* Octets we need for the bitblock allocation (bbH) */
    HBB         bbH; /* Handle to the bitblock we'll use while decoding the message */
    UINT32      from = 0, dec = 0; /* Positions inside PER buffer */
    perStruct   per; /* We need this struct to decode... */
    HPER        hPer = (HPER)&per;

    UINT32      fieldLen; /* Length of an open type field - just because we must */
    BOOL        isExtended = FALSE; /* Indication if the CHOICE value is after ... */


    /* Allocate a bit-block for the decoder for the first 2 fields.
     * We will create this decoder and then reset its buffer to point at the buffer we
     * have instead of the empty one we just created with 10 bytes
     */
    if (bbGetAllocationSize(10) > (int)sizeof(octets))
    {
        logPrint(ras->log, RV_EXCEP,
                 (ras->log, RV_EXCEP, "rasDecodePart: Allocation space for first 2 fields in RAS is not enough"));
        return RVERROR;
    }
    bbH = bbConstructFrom(10, (char *)octets, sizeof(octets));
    bbSetOctets(bbH, (int)messageSize, (INT32)messageSize * 8, messageBuffer);

    /* Set-up PER for decoding the first fields */
    memset((void *)&per, 0, sizeof(per));
    per.hBB = bbH;

    /* Decode first boolean field. This is the extension mark for the RAS Message CHOICE */
    if (perDecodeBool(&isExtended, hPer, from, &dec) < 0)
        return RVERROR;
    from += dec; /* Advance with the decoder's position */

    /* Find out the type of message */
    if (!isExtended)
    {
        /* Field before extension mark (...) - get the type of message */
        if (perDecodeInt(index, 0, ras->decoder.firstExtField - 2, FALSE, FALSE, FALSE,
                         hPer, from, &dec, (char*)"choice") < 0)
            return RVERROR;
    }
    else
    {
        /* Field after extension mark (...) - get the type of message */
        if (perDecodeNormallySmallInt(index, hPer, from, &dec) < 0)
            return RVERROR;
        (*index) += ras->decoder.firstExtField - 1;
    }
    from += dec; /* Advance with the decoder's position */
    per.decodingPosition = from;

    if (isExtended)
    {
        /* We're after ... - this kind of field is an OPEN TYPE, os we should decode
           the length first */
        if (perDecodeLen(perLenTypeUNCONSTRAINED, &fieldLen, 0, 0, hPer, from, &dec) < 0)
            return RVERROR;
        from += dec;
    }

    /* Skip the extension indication field on the SEQUENCE */
    from++;

    /* Fix the positioning. We're now inside a SEQUENCE and we want to skip the bitfield
       that holds all the OPTINAL field indications */
    from += ras->decoder.numOfOptFields[*index];
    per.decodingPosition = from;

    /* Decode the requestSeqNum - it's just an INTEGER */
    if (perDecodeInt(requestSeqNum, 1, 65535, FALSE, FALSE, FALSE, hPer,
                     from, &dec, (char*)"reqSeqNum") < 0)
        return RVERROR;

    logPrint(ras->log, RV_DEBUG,
             (ras->log, RV_DEBUG,
             "rasDecodePart: index=%d, seqNum=%d", *index, *requestSeqNum));

    return 0;
}





#ifdef __cplusplus
}
#endif


