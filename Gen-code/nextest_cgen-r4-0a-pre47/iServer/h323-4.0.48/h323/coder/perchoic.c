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
  perChoice.c

  Ron S.
  15 May 1996

  ____________________________________________________________________________
  ___________________________CHOICE___________________________________________
  ____________________________________________________________________________


  format:

  +----------+-------+-------+
  | ext. bit | index | field |
  +----------+-------+-------+

  */


#include <stdio.h>
#include <perintr.h>

/*
  Desc: Encode a choice node.
  According to clause 22.
  Node value contains the choice: fieldId (enumeration).

*/
int
perEncodeChoice(IN  HPER hPer,
        IN  int synParent,
        IN  int valParent, /* this is me */
        IN  INT32 fieldId)
{
  char *stringForPrint;
    int speciality;
    perStruct *per = (perStruct *)hPer;
    pstChildExt synChild;
    INTPTR choiceFieldId=-1;
    INT32 offset=-1;
    int vtPath;
    UINT8 ch=0;
    int childNodeId=-1;
    int extChild;
    int index; /* child index under syntax parent */
    int from,to;
    int hasExtensions;

    pstGetNodeRange(per->hSyn,synParent,&from,&to);

    /* -- get choice value */
    if ( (childNodeId = pvtChild(per->hVal, valParent)) <0)
    {
        stringForPrint=pstGetFieldNamePtr(per->hSyn, fieldId);
        msaPrintFormat(ErrPER, "perEncodeChoice: Value node not exist. [%s]",
               nprn(stringForPrint));
        return RVERROR;
    }
    pvtGet(per->hVal, childNodeId, &choiceFieldId, NULL, NULL, NULL);

    /* -- get syntax child */
    if ((index=pstGetFieldExt(per->hSyn, synParent, choiceFieldId, &synChild)) <0)
    {
        stringForPrint=pstGetFieldNamePtr(per->hSyn, choiceFieldId);
        msaPrintFormat(ErrPER, "perEncodeChoice: Syntax choice node not exist. [<%ld>%s]",
               choiceFieldId, nprn(stringForPrint));
        return RVERROR;
    }

    /* -- extension bit 22.5 */
    hasExtensions = pstGetIsExtended(per->hSyn,synParent);
    if (hasExtensions)
    {
        /* node has extension mark */
        /* and this is extended child */
        perEncodeBool(pstChildIsExtended(per->hSyn,synParent, index), per->hBB);

        extChild = pstGetFirstExtendedChild(per->hSyn, synParent);

    }
    else
    {
        /* No extensions - " simulate" first extended child by the number of nodes */
        extChild = pstGetNumberOfChildren(per->hSyn, synParent) + 1;
    }

    /* -- index */
    if (pstGetIsExtended(per->hSyn,synParent) == FALSE ||  /* 22.6 */
      pstChildIsExtended(per->hSyn,synParent, index) == FALSE)   /* 22.7 */
    {
        if (perEncodeInt(index-1, 0, (extChild<0)?(pstGetNumberOfChildren(per->hSyn,synParent)-1):(extChild-2), FALSE, FALSE, FALSE, per->hBB)< 0)
            return RVERROR;
    }
    else
    { /* 22.8 */
        if (extChild<0) /* report WARNING */
        {
            stringForPrint=pstGetFieldNamePtr(per->hSyn, fieldId);
            msaPrintFormat(ErrPER, "perEncodeChoice: No extension fields for this node [%s].",
                 nprn(stringForPrint));
        }
        if (perEncodeNormallySmallInt(index-extChild, per->hBB)<0)
            return RVERROR;
    }

    /* -- field encoding */
    if ((vtPath=pvtGetChild(per->hVal, valParent, choiceFieldId, NULL)) <0)
    {
        stringForPrint=pstGetFieldNamePtr(per->hSyn, choiceFieldId);
        msaPrintFormat(ErrPER, "perEncodeChoice: Value choice node not exist. [%s]",
            nprn(stringForPrint));
        return RVERROR;
    }
    speciality=pstChildIsSpecial(per->hSyn,synParent, index);

    if (pstChildIsExtended(per->hSyn,synParent, index) == FALSE)
    {
        int childSynNodeId;
        BOOL wasTypeResolvedInRunTime=FALSE;
        if(speciality==pstDependingDependent || speciality==pstDependent)
        {
            pvtGet(per->hVal, vtPath, NULL, &childSynNodeId, NULL, NULL);
            wasTypeResolvedInRunTime=TRUE;
        }
        else
            childSynNodeId=synChild.nodeId;

        if (perEncNode(hPer, childSynNodeId, vtPath, synChild.fieldId, wasTypeResolvedInRunTime)<0)
            return RVERROR;
    }
    else
    {  /* open type encoding */
        int childSynNodeId;
        BOOL wasTypeResolvedInRunTime=FALSE;
        if(bbAddTail(per->hBB, &ch, 0, TRUE)<0) /* align buffer */
            return RVERROR;
        offset = bbBitsInUse(per->hBB);

        if (speciality==pstDependingDependent || speciality==pstDependent)
        {
            pvtGet(per->hVal, vtPath, NULL, &childSynNodeId, NULL, NULL);
            wasTypeResolvedInRunTime=TRUE;
        }
        else
            childSynNodeId=synChild.nodeId;

        if (perEncNode(hPer, childSynNodeId, vtPath, synChild.fieldId, wasTypeResolvedInRunTime)<0)
            return RVERROR;
        perEncodeComplete(hPer,(int)bbBitsInUse(per->hBB)-offset);
        if (perEncodeOpenType(hPer, offset, synChild.fieldId)<0)
            return RVERROR;
    }

  return TRUE;
}



/*
  Desc: Decode a choice node.
  According to clause 22.
  Node value contains the choice: fieldId (enumeration).
  */
int
perDecodeChoice(IN  HPER hPer,
        IN  int synParent, /* parent in syntax tree */
        IN  int valParent, /* field parent in value tree */
        IN  INT32 fieldId)   /* enum of current field */
{
    perStruct *per = (perStruct *)hPer;
    pstChildExt synChild;

    BOOL isExtended=FALSE; /* true if extensions exist */
    BOOL skipChoiceValue = FALSE; /* true if we want to skip the decoding of the choice's value */

    UINT32 index=0;
    UINT32 children=0;
    UINT32 dec = 0; /* decoded bits */
    UINT32 fieldLen=0;
    int vtPath=-1;
    UINT32 from = per->decodingPosition; /* internal position */
    int extChild;
    BOOL extended=pstGetIsExtended(per->hSyn, synParent) ;
    int rv;

    /* -- extension bit 22.5 */
    if (extended== TRUE)
    { /* node has extension mark */
        if (perDecodeBool(&isExtended, hPer, from, &dec) <0)
        {
            msaPrintFormat(ErrPER, "perDecodeChoice: extension bit missing [%s].",
                pstGetFieldNamePtr(per->hSyn, fieldId));
            return RVERROR;
        }
        else from+=dec; /* advance decoding position */
    }

    children = pstGetNumberOfChildren(per->hSyn, synParent);
    if (extended)
    {
        extChild = pstGetFirstExtendedChild(per->hSyn, synParent);
        if (extChild < 0)
        { /* report an exception */
            msaPrintFormat(ErrPER, "perDecodeChoice: No extension fields for this node [%s].",
                pstGetFieldNamePtr(per->hSyn, fieldId));
            return RVERROR;
        }
    }
    else
        extChild = children + 1;

    /* -- index */
    if (extended == FALSE || /* 22.6 */ isExtended == FALSE /* 22.7 */)
    {
        if (perDecodeInt(&index, 0, (extChild<0)?(children-1):(extChild-2), FALSE, FALSE, FALSE, hPer, from, &dec, (char*)"choice") <0) return RVERROR;
    }
    else
    { /* 22.8 */ /* Extended child */
        if (perDecodeNormallySmallInt(&index, hPer, from, &dec) <0)
            return RVERROR;
        index+=extChild-1;
    }

    from+=dec;
    per->decodingPosition = from;

    /* -- field decoding */
    if (index + 1 > children)
    {
        if (isExtended == FALSE)
        {
            /* We got a child who's out of range */
            msaPrintFormat(ErrPER, "perDecodeChoice: field index does not exist %s [%d].",
                pstGetFieldNamePtr(per->hSyn, fieldId), index+1);
            return RVERROR;
        }
        else
        {
            /* We have an out of range field, but this choice has "..." in it */
            msaPrintFormat(ErrPER, "perDecodeChoice: field index extension not within range %s [%d].",
                pstGetFieldNamePtr(per->hSyn, fieldId), index+1);
            skipChoiceValue = TRUE;
        }

        /* The memset is here to remove a compilation warning */
        memset(&synChild, 0, sizeof(synChild));

        /* Add the choice's root even if we won't be using it. */
        vtPath = pvtAdd(per->hVal, valParent, fieldId, -111, NULL, NULL);
    }
    else
    {
        /* -- add choice value node */
        rv = pstGetChildExt(per->hSyn, synParent, (int)(index+1), &synChild);
        if (rv < 0)
        {
            msaPrintFormat(ErrPER, "perDecodeChoice: Error getting child information for %s [%d].",
                pstGetFieldNamePtr(per->hSyn, fieldId), index+1);
            return RVERROR;
        }

        vtPath=valParent;
        if (fieldId!=RVERROR)
            vtPath = pvtAdd(per->hVal, valParent, fieldId, synChild.fieldId, NULL, NULL);
    }

    if (isExtended == FALSE)
    { /* normal decoding */
        if (perDecNode(hPer, synChild.nodeId, synChild.speciality, vtPath,
                synChild.fieldId) <0)
            return RVERROR;
    }
    else
    {  /* extended child decoding: open type */
        if (perDecodeLen(perLenTypeUNCONSTRAINED, &fieldLen, 0, 0, hPer, from, &dec) <0)
        {
            msaPrintFormat(ErrPER, "perDecodeChoice: extended field length missing [%s].",
                pstGetFieldNamePtr(per->hSyn, fieldId));
            return RVERROR;
        }
        from+=dec;
        per->decodingPosition = from;

        if (skipChoiceValue == FALSE)
        {
            /* valid child */
            if (perDecNode(hPer, synChild.nodeId, synChild.speciality, vtPath,
                    synChild.fieldId) <0)
                return RVERROR;
        }

        per->decodingPosition = from +fieldLen*8; /* skip field */
    }

    return vtPath;
}


#ifdef __cplusplus
}
#endif



