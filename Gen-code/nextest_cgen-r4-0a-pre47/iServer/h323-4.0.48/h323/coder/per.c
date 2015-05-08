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
    per.c

    Ron S.
    14 May 1996

    ***************************************************
    *** Encoding Rules are ASN.1 PER BASIC - ALIGNED. ***
    ***************************************************


    Encoding/Decoding of the following ASN complex types:
    - Sequence
    - Sequence OF
    - Set OF
    - Choice

    Encoding: MSGV --> bitBuf
    Decoding: bitBuf --> MSGV

    Algorithm:
    ----------

    > Encoding(msgv, buf, tree)

    - Clear tmp and func lists.
    - Clear buf.
    - Add root node to func list.
    - while (not end_of_func_list) do
    - Clear tmp list.
    - tmp <- Encode head of func list.
    - func <- tmp + func
    - endwhile


    > EncodeFuncListNode(msgv, buf, tree, node)

    - switch (type of node)
    - Encode node by type:
    - Add encoding bits to buf.
    - Add child nodes to tmp list as needed.


    > Decoding(msgv, buf, tree)
    This is done much the same way aw the encoding procedure. Only the input is taken from
    the bit buffer and the value tree is generated.


  Notes:
  ------
  1. NULL object has no bit encoding and so there shall be no node of NULL type in
  the value tree.
  2. This is a non - reentrent code. Using tmp list as global variable for the encoding/
  decoding process.
  3. The output data structures shall have enough space to contain the encoding/decoding
  result.
  4. ALthough the process is recursive, it is implemented by a queue(func list) that
  holds an ordered list of the nodes to be processed.


  Parameters:
  -----------
  asnH: asn handler.
  deleted: number of bits deleted from bit buffer after last processing procedure.
  msgv: Message Values handler. The value tree.
  buf: bit buffer. Holds the message in ASN PER encoding(Decoding). Has sufficient space to
  hold the complete encoding(Encoding).
  tree: The syntax tree structure.
  type: Encode/Decode, determines which kind of operation to do.
  func: function node. The current node to process.
  parent: path to node in value tree.
  structEnum: enumeration of structure in the syntax tree.
  index: of child. Used for set - of nodes.
  path: path of structure in the syntax tree.
  vtPath: path of current node in value tree.
*/


#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <bb.h>
#include <copybits.h>
#include <per.h>
#include <perseq.h>
#include <perseqof.h>
#include <perchoic.h>
#include <perchrst.h>
#include <peroctst.h>
#include <perBitString.h>
#include <peroid.h>
#include <perintr.h>

#include <pvaltreeStackApi.h>
#include <psyntreeStackApi.h>
#include <ms.h>
#include <emanag.h>


/********************** MACROS **********************/

#define MAX_FUNC_NODES   30
#define IS_OPEN_TYPE(nodeId)(wasTypeResolvedInRunTime || pstGetIsOpenType(per->hSyn,(nodeId)))
#define UNDEFINED_TYPE -1905

/*** static variables ***/
static int         msa = 0;
static int         msaErr = 0;

/* Maximum size of buffers for encoding/decoding */
static int         perMaxBufSize = 0;


int perGetMsa(void)
{  return msa;}
int perGetMsaErr(void)
{  return msaErr;}



/********************************************************************************************
 * coderFreeThreadBuffer
 * An exit callback of a specific thread for CODER.
 * This function frees the allocation of the encode/decode buffer
 * context  - The pointer to the buffer to free
 ********************************************************************************************/
void RVCALLCONV coderFreeThreadBuffer(IN void*   context)
{
    THREAD_CoderLocalStorage* coderTls = (THREAD_CoderLocalStorage *)context;
    free(coderTls->buffer);
}


/************************************************************************
 * perGetBuffer
 * purpose: Returns the buffer to use for PER encode/decode procedures
 * input  : none
 * output : none
 * return : Pointer to the buffer on success
 *          NULL on failure
 ************************************************************************/
THREAD_CoderLocalStorage* perGetBuffer(void)
{
    THREAD_CoderLocalStorage*   coderTls;
    RvH323ThreadHandle          threadId;
    BOOL                        anyExitFunc = FALSE;

    threadId = RvH323ThreadGetHandle();
    coderTls = (THREAD_CoderLocalStorage *)THREAD_GetLocalStorage(threadId, tlsIntrCoder, sizeof(THREAD_CoderLocalStorage));
    if (coderTls == NULL) return NULL;

    if ((coderTls->buffer != NULL) && ((int)coderTls->bufferSize < perMaxBufSize))
    {
        /* Current allocation too small - make sure we fix this situation... */
        free(coderTls->buffer);
        coderTls->buffer = NULL;
        anyExitFunc = TRUE;
    }

    if (coderTls->buffer == NULL)
    {
        coderTls->bufferSize = perMaxBufSize;
        coderTls->buffer = (BYTE *)malloc((size_t)coderTls->bufferSize);

        if (coderTls->buffer == NULL)
            return NULL;
        else if (!anyExitFunc)
            H323ThreadSetExitFunction(threadId, coderFreeThreadBuffer, coderTls);
    }

    return coderTls;
}


/*_____________________________________________________________________*/
/*_________________________________INFORMATION_________________________*/
/*_____________________________________________________________________*/

/*
Desc: put bits in head of dest buffer.
Returns: Number of bits extracted from buffer.
*/
INT32
bbGet2Left(IN  HPER     hPer,
           IN  UINT32   fromBitPosition, /* in buffer */
           IN  UINT32   numOfBitsToGet,
           OUT UINT8*   dest) /* destination buffer */
{
    perStruct *per =(perStruct *)hPer;
    bbStruct *bb =(bbStruct *)(per->hBB);
    UINT32 actualLength;

    if (!bb || !dest)
        return RVERROR;
    actualLength = min(numOfBitsToGet, bb->bitsInUse - fromBitPosition);

    if (actualLength > per->buf->bufferSize * 8)
    {
        per->encodingDecodingErrorBitMask = per->encodingDecodingErrorBitMask | encDecErrorsMessageIsInvalid;
        return RVERROR;
    }

    memcpyb(dest, 0, bb->octets, fromBitPosition, actualLength);
    return actualLength;
}


/*
Desc: put bits in tail of dest buffer.
Note: Length of dest buffer is the minimum number of bytes needed to hold
the numOfBitsToGet bits.
Returns: Number of bits extracted from buffer.
*/
INT32
bbGet2Right(HPER hPer,
            IN UINT32 fromBitPosition, /* in buffer */
            IN UINT32 numOfBitsToGet,
            OUT UINT8 *dest) /* destination buffer */
{
    perStruct *per =(perStruct *)hPer;
    bbStruct *bb =(bbStruct *)(per->hBB);
    UINT32 actualLength;

    if (!bb || !dest)
        return RVERROR;
    actualLength = min(numOfBitsToGet, bb->bitsInUse - fromBitPosition);

    if (actualLength>MAX_INT_SIZE*8)
    {
        per->encodingDecodingErrorBitMask = per->encodingDecodingErrorBitMask | encDecErrorsMessageIsInvalid;
        return RVERROR;
    }

    memcpyb(dest, (8 - numOfBitsToGet%8)%8, bb->octets, fromBitPosition, actualLength);
    return actualLength;
}












/*______________________________low level____________________________________________*/


/*
Desc: Production of the complete encoding.
Padding the field - list encoding for octet alignment.
10.1

*/
int
perEncodeComplete(IN  HPER hPer, IN int bitsInUse)
{
    perStruct *per =(perStruct *)hPer;
    int  len;
    UINT8 ch = 0;

    /* -- 10.1.3 */
    if (bitsInUse>0)
        len =(int)((8 - bitsInUse%8)%8); /* bits for alignment */
    else
        len = 8;

    return bbAddTail(per->hBB, (UINT8 *)&ch, len, FALSE); /* align encoding */
}


/*
Desc: Encode buffer as an open type starting at offset.
10.2

*/
int
perEncodeOpenType(IN  HPER hPer,
                  IN  INT32 offset,  /* beginning of open type encoding */
                  IN  INT32 fieldId)
{
    perStruct *per =(perStruct *)hPer;
    UINT8 octets[64]; /* space for tmp encoding */
    INT32 encLen = bbBitsInUse(per->hBB) - offset; /* encoding length */
    HBB tmpBB; /* for length encoding */
    INT32 tmpBBlen;
    if (fieldId);

#ifdef RV_CODER_DEBUG
    if (bbGetAllocationSize(10) >(int)sizeof(octets))
    {
        msaPrintFormat(ErrPER, "perEncodeOpenType: Allocation space for length not enough [%d].",
            bbGetAllocationSize(10));
        return RVERROR;
    }
#endif  /* RV_CODER_DEBUG */
    tmpBB = bbConstructFrom(10, (char *)octets, sizeof(octets));

    /* -- 10.2.2: length in octets */
    if (perEncodeLen(perLenTypeUNCONSTRAINED, bbSetByte(encLen), 0, 0, tmpBB) <0)
    {
        msaPrintFormat(ErrPER, "perEncodeOpenType: length encoding failed for '%s' [%ld].",
            pstGetFieldNamePtr(per->hSyn, fieldId), encLen);
        return RVERROR;
    }

    /* -- add length before open field encoding */
    /* move encoding tmpBBlen bits ahead */
    tmpBBlen = bbBitsInUse(tmpBB);
    if (bbMove(per->hBB, offset, encLen, offset + tmpBBlen) < 0)
        return RVERROR;
    if (bbSet(per->hBB, offset, tmpBBlen, bbOctets(tmpBB)) >= 0)
    {
        msaPrintFormat(InfoPER, "Encoding OPENTYPE: intserted %d bits in [%d].", tmpBBlen, offset);
    }
    else
    {
        msaPrintFormat(ErrPER, "perEncodeOpenType: set failed for '%s' [%d].",
            pstGetFieldNamePtr(per->hSyn, fieldId), encLen);
        return RVERROR;
    }

    return TRUE;
}


/*____________________________________________________________________________*/
/*___________________________NODE_____________________________________________*/
/*____________________________________________________________________________*/

/* Desc: encode a node in the static syntax tree.
Logic: switch between types.
*/
int
perEncNode(IN  HPER hPer,
           IN  int synParent,
           IN  int valParent,
           IN  INT32 fieldId,
           IN  BOOL wasTypeResolvedInRunTime)
{
#ifndef NOLOGSUPPORT
    char* stringForPrint;
#endif
    perStruct *per =(perStruct *)hPer;
    INT32 value=-1;
    int ret = RVERROR;
    INT32 offset=-1;
    pstNodeType type;
    int  to,from;
    BOOL toAbsent,fromAbsent;


    if (((int)(type = pstGetNodeType(per->hSyn,synParent))) < 0)
    {
#ifndef NOLOGSUPPORT
        stringForPrint = pstGetFieldNamePtr(per->hSyn, fieldId);
        msaPrintFormat(ErrPER, "perEncodeNode: Illegal syntax node: [%s:%d].",
            nprn(stringForPrint), synParent);
#endif
        return RVERROR;
    }

    if (pvtGet(per->hVal, valParent, NULL, NULL, &value, NULL) <0)
    {
#ifndef NOLOGSUPPORT
        stringForPrint = pstGetFieldNamePtr(per->hSyn, fieldId);
        msaPrintFormat(ErrPER, "perEncodeNode: Illegal value node: [%s:%d].",
            nprn(stringForPrint), valParent);
#endif
        return RVERROR;
    }
    pstGetNodeRangeExt(per->hSyn,synParent,&from,&to,&fromAbsent,&toAbsent);
    per->isOuterMostValueEmpty = FALSE;
#ifndef NOLOGSUPPORT
    stringForPrint = pstGetTokenName(type);
    msaPrintFormat(InfoPER, "Encoding %s: %s [%d].",
        (fieldId >= 0) ? pstGetFieldNamePtr(per->hSyn, fieldId):"(null)",
        nprn(stringForPrint),
        bbBitsInUse(per->hBB));
#endif
    if (IS_OPEN_TYPE(synParent))
    {
        UINT8 ch = 0;
        if (bbAddTail(per->hBB, &ch, 0, TRUE) < 0) /* align buffer */
            return RVERROR;
        offset = bbBitsInUse(per->hBB);
    }

    switch (type)
    {
        case pstNull:
            per->isOuterMostValueEmpty = TRUE;
            ret = TRUE;
            break;
        case pstInteger:
            ret = perEncodeInt((UINT32)value, from,to, fromAbsent, toAbsent,
                pstGetIsExtended(per->hSyn,synParent), per->hBB);
            if (from == to && !fromAbsent && !toAbsent)
                per->isOuterMostValueEmpty = TRUE;
            break;
        case pstBoolean:
            ret = perEncodeBool(((value) ? (TRUE):(FALSE)), per->hBB);
            break;

        case pstUniversalString:
        case pstGeneralString:
        case pstBMPString:
        case pstIA5String:
        case pstNumericString:
        case pstPrintableString:
        case pstVisibleString:
            ret = perEncodeCharString(hPer, synParent, valParent, fieldId);
            break;
        case pstOctetString:
            ret = perEncodeOctetString(hPer, synParent, valParent, fieldId);
            break;

        case pstBitString:
            ret = perEncodeBitString(hPer, synParent, valParent, fieldId);
            break;

        case pstObjectIdentifier:
            ret = perEncodeOID(hPer, synParent, valParent, fieldId);
            break;

        case pstEnumeration:
        case pstChoice:
            ret = perEncodeChoice(hPer, synParent, valParent, fieldId);
            break;

        case pstSet:
        case pstSequence:
            ret = perEncodeSequece(hPer, synParent, valParent, fieldId);
            break;

        case pstSetOf:
        case pstSequenceOf:
            ret = perEncodeSequeceOF(hPer, synParent, valParent, fieldId);
            break;
        default:
#ifndef NOLOGSUPPORT
            stringForPrint = pstGetTokenName(type);
            msaPrintFormat(ErrPER, "perEncodeNode: %s TYPE unrecognized: %s.",
                pstGetFieldNamePtr(per->hSyn, fieldId),
                nprn(stringForPrint));
#endif
            return RVERROR;
    }

    if (ret >= 0)
        if (IS_OPEN_TYPE(synParent))
        {
            perEncodeComplete(hPer, (int)bbBitsInUse(per->hBB) - offset);
            ret = perEncodeOpenType(hPer, offset, fieldId);
        }
        if (ret < 0)
        {
#ifndef NOLOGSUPPORT
            stringForPrint = pstGetTokenName(type);
            msaPrintFormat(ErrPER, "perEncodeNode: '%s' %s encoding Error!.",
                pstGetFieldNamePtr(per->hSyn, fieldId),
                nprn(stringForPrint));
#endif
            return RVERROR;
        }


        return TRUE;
}



int perPushElemToArrayOfSpecialNodes(
                                     IN HPER hPer,
                                     IN int element
                                     )
{
    perStruct *per =(perStruct *)hPer;
    if (per->currentPositionInArrayOfSpecialNodes == MAX_SPECIAL_NODES)
        return RVERROR;
    per->arrayOfSpecialNodes[per->currentPositionInArrayOfSpecialNodes++] = element;
    return TRUE;
}


int perPopElemFromArrayOfSpecialNodes(
                                      IN HPER hPer
                                      )
{
    perStruct *per =(perStruct *)hPer;
    if (per->currentPositionInArrayOfSpecialNodes == 0)
        return RVERROR;
    per->currentPositionInArrayOfSpecialNodes--;
    return TRUE;
}


int perGetDependentTypeForDecoding(
                                   IN HPER hPer,
                                   IN int specialTypeNodeId, /* nodeid of "special" type */
                                   OUT int* dependentTypeNodeId /* nodeid of "real" type */
                                   )
{
    int i, objectId;
    pstTypeFromConstraint specialType;
    pstChild field;
    pstConstrainingField constrainingField;
    pstFieldOfObjectReference fieldOfObject;
    perStruct *per =(perStruct *)hPer;

    /* getting special type */
    pstGetTypeFromConstraint(per->hSyn, specialTypeNodeId, &specialType);

    /* getting constraining field */
    pstGetConstrainingField(per->hSyn, specialTypeNodeId, &constrainingField);

    /* getting field itself(node) */
    /* here very intimate knowledge of the internal database is used*/
    /* it is known that fields are placed at nodeId+fieldIndex position */

    pstGetChild(per->hSyn, 0, constrainingField.fieldId, &field);

    if (pvtFindObject(per->hVal,
           per->arrayOfSpecialNodes[per->currentPositionInArrayOfSpecialNodes - 1 -
                                    constrainingField.relativeIndex],
           per->hSyn, specialType.objectSetId, &objectId) == RVERROR)      /* finding object */
           /* object was not found */
           return (UNDEFINED_TYPE);


    for (i = 1;; i++)                                                           /* finding field of object */
    {
        if (pstGetFieldOfObjectReference(per->hSyn, objectId, i, &fieldOfObject) < 0)
            return RVERROR;
        if (fieldOfObject.fieldInClassNumber == specialType.fieldInClassNumber)
            break;
    }

    if (dependentTypeNodeId)
        *dependentTypeNodeId = fieldOfObject.settingId;
    return fieldOfObject.settingId;
}



int
perDecNode(IN  HPER hPer,
           IN  int synParent,
           IN  pstFieldSpeciality speciality,
           IN  int valParent,
           IN  INT32 fieldId)
{
#ifndef NOLOGSUPPORT
    char* stringForPrint;
#endif
    perStruct *per =(perStruct *)hPer;
    UINT32 value = 0;
    int i, ret = RVERROR,  vtPath=-1, startListLength, finishListLength;
    UINT32 dec = 0;
    BOOL boola = FALSE, wasTypeResolvedInRunTime = FALSE;
    INT32 saveLocation=-1;
    UINT32 fieldLen = 0;
    pstNodeType type;
    int  to,from;
    BOOL toAbsent,fromAbsent;
    int origSynParent = synParent;

    if (valParent < 0)
    {
        per->encodingDecodingErrorBitMask |= encDecErrorsMessageIsInvalid;
        return RVERROR;
    }

    if (speciality == pstDependingDependent || speciality == pstDependent)
    {
        if ((synParent = perGetDependentTypeForDecoding(hPer, synParent, &synParent)) == RVERROR)
            return RVERROR;
        wasTypeResolvedInRunTime = TRUE;

        if (synParent == UNDEFINED_TYPE)
        {
            /* Seems like H.450 message contains parts we can't understand. */
            msaPrintFormat(ErrPER,
                "perDecodeNode: Uknown dependent type: [%s:%d].",
                pstGetFieldNamePtr(per->hSyn, fieldId), origSynParent);
        }
    }
    pstGetNodeRangeExt(per->hSyn,synParent,&from,&to,&fromAbsent,&toAbsent);
    type=pstGetNodeType(per->hSyn,synParent);
    if (synParent != UNDEFINED_TYPE)
    {
        if (((int)type) < 0)
        {
#ifndef NOLOGSUPPORT
            msaPrintFormat(ErrPER, "perDecodeNode: Illegal syntax node: [%s:%d].",
                pstGetFieldNamePtr(per->hSyn, fieldId), synParent);
#endif
            return RVERROR;
        }
#ifndef NOLOGSUPPORT
        stringForPrint = pstGetTokenName(type);
        msaPrintFormat(InfoPER, "Decoding %s: %s [%ld].",
            pstGetFieldNamePtr(per->hSyn, fieldId), nprn(stringForPrint),
            per->decodingPosition);
#endif
    }

    if (IS_OPEN_TYPE(synParent))
    {
        if (perDecodeLen(perLenTypeUNCONSTRAINED, &fieldLen, 0, 0, hPer, per->decodingPosition, &dec) <0)
        {
#ifndef NOLOGSUPPORT
            msaPrintFormat(ErrPER, "perDecNode: open type field length missing [%s].",
                pstGetFieldNamePtr(per->hSyn, fieldId));
#endif
            return RVERROR;
        }
        msaPrintFormat(InfoPER, " Decoding open type.");
        per->decodingPosition += dec;
        saveLocation = per->decodingPosition;
    }

    if (synParent != UNDEFINED_TYPE)
        switch (type)
    {
        case pstNull:
            vtPath=valParent;
            if (fieldId!=RVERROR)
            ret =  vtPath =pvtAdd(per->hVal, valParent, fieldId, -123, NULL, NULL);
            break;
        case pstInteger:
            ret = perDecodeInt(&value, from, to, fromAbsent, toAbsent,
                                pstGetIsExtended(per->hSyn,synParent),
                                hPer, per->decodingPosition, &dec, (char*)"integer");
            vtPath=valParent;
            if ((fieldId!=RVERROR) && (ret >= 0))
                ret = vtPath=pvtAdd(per->hVal, valParent, fieldId, (INT32)value, NULL, NULL);
            per->decodingPosition += dec;
            break;
        case pstBoolean:
            ret = perDecodeBool(&boola, hPer, per->decodingPosition, &dec);
            vtPath=valParent;
            if ((fieldId!=RVERROR) && (ret >= 0))
                ret = vtPath=pvtAdd(per->hVal, valParent, fieldId, boola,  NULL, NULL);
            per->decodingPosition += dec;
            break;

        case pstUniversalString:
        case pstGeneralString:
        case pstBMPString:
        case pstIA5String:
        case pstNumericString:
        case pstPrintableString:
        case pstVisibleString:
            ret = perDecodeCharString(hPer, synParent, valParent, fieldId);
            break;

        case pstOctetString:
            ret = perDecodeOctetString(hPer, synParent, valParent, fieldId);
            break;

        case pstBitString:
            ret = perDecodeBitString(hPer, synParent, valParent, fieldId);
            break;
        case pstObjectIdentifier:
            ret = perDecodeOID(hPer, synParent, valParent, fieldId);
            break;
        case pstEnumeration:
        case pstChoice:
            startListLength = per->currentPositionInArrayOfSpecialNodes;
            ret = perDecodeChoice(hPer, synParent, valParent, fieldId);
            finishListLength = per->currentPositionInArrayOfSpecialNodes;
            for (i = 0; i < finishListLength - startListLength; i++)
                perPopElemFromArrayOfSpecialNodes(hPer);
            break;
        case pstSet:
        case pstSequence:
            startListLength = per->currentPositionInArrayOfSpecialNodes;
            ret = perDecodeSequece(hPer, synParent, valParent, fieldId);
            finishListLength = per->currentPositionInArrayOfSpecialNodes;
            for (i = 0; i < finishListLength - startListLength; i++)
                perPopElemFromArrayOfSpecialNodes(hPer);
            break;
        case pstSequenceOf:
        case pstSetOf:
            ret = perDecodeSequeceOF(hPer, synParent, valParent, fieldId);
            break;
        default:
#ifndef NOLOGSUPPORT
            stringForPrint = pstGetTokenName(type);
            msaPrintFormat(ErrPER, "perDecodeNode: %s TYPE unrecognized: %s.",
                pstGetFieldNamePtr(per->hSyn, fieldId),
                nprn(stringForPrint));
#endif
            return RVERROR;
    } /* end of switch */
    else /* synParent == UNDEFINED_TYPE */
    {
        vtPath=valParent;
        /*if (fieldId!=RVERROR)
        ret = pvtAdd(per->hVal, valParent, fieldId, 555, NULL, NULL);*/
    }


    if (IS_OPEN_TYPE(synParent))
        per->decodingPosition = saveLocation + fieldLen*8; /* skip field */

    if (synParent != UNDEFINED_TYPE && ret < 0)
    {
#ifndef NOLOGSUPPORT
        stringForPrint = pstGetTokenName(type);
        msaPrintFormat(ErrPER, "perDecodeNode: '%s' %s decoding Error!.",
            pstGetFieldNamePtr(per->hSyn, fieldId),
            nprn(stringForPrint));
#endif
        switch (ret)
        {
            case RVERROR:
                per->encodingDecodingErrorBitMask = per->encodingDecodingErrorBitMask | encDecErrorsMessageIsInvalid;
                break;
            case RESOURCES_PROBLEM:per->encodingDecodingErrorBitMask = per->encodingDecodingErrorBitMask |
                                       encDecErrorsResourcesProblem;
                break;
        }


        return RVERROR;
    }

    if (speciality == pstDependingDependent || speciality == pstDepending)
        perPushElemToArrayOfSpecialNodes(hPer, ret);

    if (synParent == UNDEFINED_TYPE)
        per->encodingDecodingErrorBitMask = per->encodingDecodingErrorBitMask | encDecErrorsObjectWasNotFound;

    return TRUE;
}





/************************************************************************
 * perConstruct
 * purpose: Construct PER information needed by the encode/decode manager.
 * input  : maxBufSize  - Maximum size of buffer supported (messages larger
 *                        than this size in bytes cannot be decoded/encoded).
 * output : none
 * return : none
 ************************************************************************/
void perConstruct(IN int maxBufSize)
{
    emTypeOfEncoding eSys;
    static BOOL FirstTime = TRUE;

    eSys.Encode = perEncode;
    eSys.Decode = perDecode;

    if (FirstTime)
    {
        FirstTime = FALSE;
        msa = msaRegister(0, "PER", "PER Encoder/Decoder");
        msaErr = msaRegister(0, "PERERR", "PER Error Messages");
    }

    emSetEncoding(emPER, &eSys);

    /* Make sure we create a buffer with sufficient size for this thread */
    if (perMaxBufSize < maxBufSize)
        perMaxBufSize = maxBufSize;
    perGetBuffer();
}


/*
Desc: Free per allocations.
Free func list and structure.
*/
void perDestruct(void) {}
/*
Desc: Encode a message from root.
*/
int
perEncode(
          IN  HPVT  valH,            /* encoding from value tree */
          IN  int    valNodeId,       /* root of encoding */
          OUT UINT8  *buffer,         /* encoding to this buffer */
          IN  int    bufferLength,    /* in bytes */
          OUT int*   encoded)         /* bytes encoded to buffer */
{
    perStruct per;
    UINT8 octets[32];
    HBB bbH;
    int synNodeId;
    int ret;

#ifdef RV_CODER_DEBUG
    if (bbGetAllocationSize(0) >(int)sizeof(octets))
    {
        msaPrintFormat(ErrPER, "perEncode: Allocation space for length not enough [%d].",
            bbGetAllocationSize(0));
        return RVERROR;
    }
#endif  /* RV_CODER_DEBUG */
    bbH = bbConstructFrom(0, (char *)octets, sizeof(octets));
    bbSetOctets(bbH, bufferLength, 0, buffer);

    if (!buffer || !bbH)
        return RVERROR;

    memset((void *)&per, 0, sizeof(per));
    per.hSyn = pvtGetSynTree(valH, valNodeId);
    per.hVal = valH;
    per.isOuterMostValueEmpty = FALSE;
    per.hBB = bbH;
    per.encodingDecodingErrorBitMask = 0x0;
    per.buf = perGetBuffer();

    if (pvtGet(valH, valNodeId, NULL, &synNodeId, NULL, NULL) <0)
    {
        msaPrintFormat(ErrPER, "perEncode: value tree root id is illagal [%d].", valNodeId);
        return RVERROR;
    }

    ret = perEncNode((HPER)&per, synNodeId, valNodeId, -1, FALSE);
    if (ret == RVERROR &&((bbStruct*)(per.hBB))->isOverflowOfBuffer == TRUE)
        msaPrintFormat(ErrPER, "Encoding error: Overflow of buffer.");
    if (encoded)
        *encoded = bbBytesInUse(per.hBB);
    perEncodeComplete((HPER)&per, (int)bbBitsInUse(per.hBB));
    return ret;
}



/*
Desc: Decode a message from root.
*/
int
perDecode(
          OUT HPVT valH,         /* decoding to value tree */
          IN  int   valNodeId,    /* root of encoding */
          IN  INT32 fieldId,      /* root field Id */
          IN  UINT8 *buffer,      /* decoding from this buffer */
          IN  int   bufferLength, /* in bytes */
          OUT int*  decoded)     /* number of BYTES successfully decoded from buffer */
{
    perStruct per;
    UINT8 octets[64];
    HBB bbH;
    int synRootId;
    pstChildExt child;
    pstFieldSpeciality speciality;
    int index;

    bbH = bbConstructFrom(10, (char *)octets, sizeof(octets));
    bbSetOctets(bbH, bufferLength, bufferLength*8, buffer);

    if (!buffer || !bbH)
        return RVERROR;

    memset((void *)&per, 0, sizeof(per));
    per.hSyn = pvtGetSynTree(valH, valNodeId);
    per.hVal = valH;
    per.isOuterMostValueEmpty = FALSE;
    per.hBB = bbH;
    per.currentPositionInArrayOfSpecialNodes = 0;
    per.encodingDecodingErrorBitMask = 0x0;
    per.buf = perGetBuffer();

    pvtGet(valH, valNodeId, NULL, &synRootId, NULL, NULL);
    if ((index=pstGetFieldIndex(per.hSyn, synRootId, fieldId)) >=0)
    {
        pstGetChildExt(per.hSyn, synRootId,index,&child);
        speciality = child.speciality;
        synRootId = child.nodeId;
    }
    else
        speciality = pstNotSpecial;

    if (perDecNode((HPER)&per, synRootId, speciality, valNodeId, fieldId) == RVERROR  &&
        ((bbStruct*)(per.hBB))->isOverflowOfBuffer == TRUE)
    {
        msaPrintFormat(ErrPER, "Decoding error: Overflow of buffer.");
    }

    if (decoded)
        *decoded = bbSetByte(per.decodingPosition);

    if (per.encodingDecodingErrorBitMask)
    {
        if (per.encodingDecodingErrorBitMask &  encDecErrorsResourcesProblem)
            return RESOURCES_PROBLEM;

        if (per.encodingDecodingErrorBitMask &  encDecErrorsMessageIsInvalid)
            return RVERROR;

        /* We might have an H.450 message with parts we can't understand. In such a case,
           we should still think of the message as decodable without any real errors
        if (per.encodingDecodingErrorBitMask &  encDecErrorsObjectWasNotFound)
            return OBJECT_WAS_NOT_FOUND;*/
    }

    return TRUE;
}



#ifdef __cplusplus
}
#endif



