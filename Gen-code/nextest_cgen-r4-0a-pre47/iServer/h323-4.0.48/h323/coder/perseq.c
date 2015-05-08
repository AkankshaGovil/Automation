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
  perSequence.c

  Extension encoding:
  -------------------
   - encode field.
   - complete encoding (padding).
   - open type encoding (length prefix)

   ------------------------
   | length | field | pad |
   ------------------------
   | 1/2oc. |   n octets  |
   ------------------------


  Ron S.
  14 May 1996

  ____________________________________________________________________________
  ___________________________SEQUENCE_________________________________________
  ____________________________________________________________________________


  Format:

  +----------+-------------------+---------+     +---------+
  | ext. bit | n bits (optional) | field-1 | ... | field-n |
  +----------+-------------------+---------+     +---------+

  Extentions:

  +------------+---------------------+----------+       +----------+
  | length (n) | n bits (extentions) | Efield-1 |  ...  | Efield-n |
  +------------+---------------------+----------+       +----------+

  Efield-i: open type complete encoding.

  +-----+--------+----------+-----+
  | pad | length | encoding | pad |
  +-----+--------+----------+-----+


  */

#include <stdio.h>
#include <perintr.h>





/*
  Return: number of extended childs that actually exist in value tree.
  */
int
perGetNumOfExtendedChilds(IN  HPER hPer,
              IN  int synParent,
              IN  int valParent)
{
  perStruct *per = (perStruct *)hPer;
    pstChildExt synChild;
    int i, count=0;
    int children;

    children=pstGetNumberOfChildren(per->hSyn,synParent);

    for (i=0; i<children; i++)
    {
        pstGetChildExt(per->hSyn, synParent, i+1, &synChild);
        if (synChild.isExtended &&  /* extended */
            pvtGetChild(per->hVal, valParent, synChild.fieldId, NULL) >=0) /* exist */
      count++;
  }

  return count;
}


/*
  Desc: Encode the extended fields in a sequence type.

  -- length: number of extended fields.
  -- bit field: bit foreach extended field. true if exist.
  -- list of extended fields, each as open type.
  */
int
perEncodeSequenceExt(IN  HPER hPer,
         IN  int synParent,
         IN  int valParent,
         IN  INT32 fieldId)
{
  char *stringForPrint;
  perStruct *per = (perStruct *)hPer;
    pstChildExt synChild;
    int i, vtPath=-1, numOfExtendedChilds=0;
    INT32 offset=-1;
    UINT8 ch=0;
    int children;

    /* -- checks */
    if (!per)
    {
        stringForPrint=pstGetFieldNamePtr(per->hSyn, fieldId);
        msaPrintFormat(ErrPER, "perEncodeSequenceExt: %s: Invalid parameters.",
            nprn(stringForPrint));
        return RVERROR;
    }

    children=pstGetNumberOfChildren(per->hSyn,synParent);

    /* -- length: 18.8: normally small length */
    for (numOfExtendedChilds=0, i=0; i<children; i++)
    {
        pstGetChildExt(per->hSyn, synParent, i+1, &synChild);
        if (synChild.isExtended) numOfExtendedChilds++;
    }
    if (perEncodeLen(perLenTypeNORMALLY_SMALL, numOfExtendedChilds, 0, 0, per->hBB) < 0)
        return RVERROR;

    /* -- bit field:18.7 */
    for (i=0; i<children; i++)
    {
        pstGetChildExt(per->hSyn, synParent, i+1, &synChild);
        if (synChild.isExtended)
        { /* extended */
            perEncodeBool(
                (pvtGetChild(per->hVal, valParent, synChild.fieldId, NULL) >=0)  /* exist */,
                    per->hBB);
        }
    }

  /* -- field encoding: 18.9 */
    for (i=0; i<children; i++)
    {
        pstGetChildExt(per->hSyn, synParent, i+1, &synChild);
        if (synChild.isExtended)
        { /* extended */
            if ((vtPath=pvtGetChild(per->hVal, valParent, synChild.fieldId,NULL)) >=0)
            {  /* exist */
                int childSynNodeId;
                pstFieldSpeciality speciality=synChild.speciality;
                BOOL wasTypeResolvedInRunTime=FALSE;
                if(bbAddTail(per->hBB, &ch, 0, TRUE)<0) /* align buffer */
                return RVERROR;
                offset = bbBitsInUse(per->hBB);

                if(speciality==pstDependingDependent || speciality==pstDependent)
                {
                    pvtGet(per->hVal, vtPath, NULL, &childSynNodeId, NULL, NULL);
                    wasTypeResolvedInRunTime=TRUE;
                }
                else
                    childSynNodeId=synChild.nodeId;

                if (perEncNode(hPer, childSynNodeId, vtPath, synChild.fieldId, wasTypeResolvedInRunTime) < 0)
                    return RVERROR;
                perEncodeComplete(hPer,(int)bbBitsInUse(per->hBB)-offset);
                if (perEncodeOpenType(hPer, offset, synChild.fieldId)<0)
                    return RVERROR;
            }
            else
            { /* should be optional */
                if ((synChild.isOptional == FALSE) && (synChild.isDefault == FALSE))
                { /* but is not */
                    stringForPrint=pstGetFieldNamePtr(per->hSyn, fieldId);
                    msaPrintFormat(ErrPER, "perEncodeSequenceExt: non optional Child not found [%d]%s->%s.",
                        valParent, nprn(stringForPrint), pstGetFieldNamePtr(per->hSyn, synChild.fieldId));
                    return RVERROR;
                }
            }
        }
  }
  return TRUE;
}



/*
  Desc: Encode a sequence node.
  According to clause 18.
  Node value has no meaning.
*/
int
perEncodeSequece(IN  HPER hPer,
         IN  int synParent,
         IN  int valParent,
         IN  INT32 fieldId)
{
    char *stringForPrint;
    perStruct *per = (perStruct *)hPer;
    pstChildExt synChild;
    int i, vtPath=-1, numOfExtChilds=0;
    int children;

  /* -- checks */
    if (!per)
    {
        stringForPrint=pstGetFieldNamePtr(per->hSyn, fieldId);
        msaPrintFormat(ErrPER, "perEncodeSequence: Invalid parameters. [%s]",
            nprn(stringForPrint));
        return RVERROR;
    }

    /* -- extension bit 18.1 */
    if (pstGetIsExtended(per->hSyn,synParent) == TRUE)
    { /* node has extension mark */
        numOfExtChilds=perGetNumOfExtendedChilds(hPer, synParent, valParent);
        perEncodeBool((numOfExtChilds>0) , per->hBB);  /* has extended childs */
    }
    children=pstGetNumberOfChildren(per->hSyn,synParent);

    /* -- preemble 18.2-18.3: bit for each optional child */
    for (i=0; i<children; i++)
    {
        pstGetChildExt(per->hSyn, synParent, i+1, &synChild);

        if (synChild.isExtended == FALSE && /* not extended */
            ((synChild.isOptional == TRUE) || (synChild.isDefault == TRUE))) /* optional child */
        {
            vtPath=pvtGetChild(per->hVal, valParent, synChild.fieldId, NULL);
            perEncodeBool((vtPath>=0), per->hBB);/* exists */
        }
    }

    /* -- preemble 18.4-18.6: encode fields */
    for (i=0; i<children; i++)
    {
        pstGetChildExt(per->hSyn, synParent, i+1, &synChild);

        if (synChild.isExtended == FALSE)
        { /* not extended */
            if ((vtPath = pvtGetChild(per->hVal, valParent, synChild.fieldId, NULL)) >=0)
            {/* exists */
                int childSynNodeId;
                BOOL wasTypeResolvedInRunTime=FALSE;

                if (synChild.speciality==pstDependingDependent ||
                    synChild.speciality==pstDependent)
                {
                    pvtGet(per->hVal, vtPath, NULL, &childSynNodeId, NULL, NULL);
                    wasTypeResolvedInRunTime=TRUE;
                }
                else
                    childSynNodeId=synChild.nodeId;
                if (perEncNode(hPer, childSynNodeId, vtPath, synChild.fieldId, wasTypeResolvedInRunTime) < 0)
                    return RVERROR;
            }
            else
            {
                if ((synChild.isOptional == FALSE) && (synChild.isDefault == FALSE))
                { /* non-optional not found */
                    stringForPrint = pstGetFieldNamePtr(per->hSyn, synChild.fieldId);
                    msaPrintFormat(ErrPER, "perEncodeSequence: Child not found %d->%s.",
                        valParent, nprn(stringForPrint));
                    return RVERROR;
                }
            }
        } /* not extended */
  }

  /* -- Extensions 18.7-18.9 */
  if (numOfExtChilds >0)
    if (perEncodeSequenceExt(hPer, synParent, valParent, fieldId)< 0)
      return RVERROR;

  return TRUE;
}


/*_____________________________________decoding.........................................*/


/*
  Desc: Decode a sequence node.
  According to clause 18.
  Node value has no meaning.

*/
int
perDecodeSequece(IN  HPER hPer,
         IN  int synParent, /* parent in syntax tree */
         IN  int valParent, /* field parent in value tree */
         IN  INT32 fieldId)   /* enum of current field */
{
    char *stringForPrint;
    perStruct *per = (perStruct *)hPer;
    pstChildExt synChild;

    BOOL isExtended=FALSE; /* true if extensions exist */
    BOOL optionalExist=FALSE;
    BOOL isExist=FALSE;

    UINT32 dec = 0; /* decoded bits */
    int i, vtPath, numOfOptionalChilds=0, extChild=-1;
    UINT32 from = per->decodingPosition; /* internal position */
    INT32 saveLocation=-1;
    UINT32 extLen=0, fieldLen=0;
    int children;

    /* -- checks */
    if (!per)
    {
        msaPrintFormat(ErrPER, "perDecodeSequece: Invalid parameters.");
        return RVERROR;
    }

    /* -- add sequence value node */
    vtPath=valParent;
    if (fieldId!=RVERROR)
    vtPath = pvtAdd(per->hVal, valParent, fieldId, -111, NULL, NULL);

    /* -- first bit 18.1 */
    if (pstGetIsExtended(per->hSyn,synParent) == TRUE)
    { /* node has extension mark */
        if (perDecodeBool(&isExtended, hPer, from, &dec) <0)
        {
            stringForPrint=pstGetFieldNamePtr(per->hSyn, fieldId);
            msaPrintFormat(ErrPER, "perDecodeSequece: extension bit missing [%s].",
                nprn(stringForPrint));
            return RVERROR;
        }
        else from+=dec; /* advance decoding position */
    }

    /* -- preemble 18.2-18.6: bit for each optional child */
    numOfOptionalChilds = pstGetNumberOfOptionalBeforeExtension(per->hSyn, synParent);
    per->decodingPosition = from+numOfOptionalChilds; /* advance to end of bit field */
    children= pstGetNumberOfChildren(per->hSyn,synParent);

    for (i=0; i<children; i++)
    {
        pstGetChildExt(per->hSyn, synParent, i+1, &synChild);

        if (synChild.isExtended == FALSE)
        { /* not extended */
            if ((synChild.isOptional == TRUE) || (synChild.isDefault == TRUE))
            { /* optional child */
                perDecodeBool(&optionalExist, hPer, from++, &dec);
                if (optionalExist)
                    if (perDecNode(hPer, synChild.nodeId, synChild.speciality, vtPath, synChild.fieldId) <0)
                        return RVERROR;
            }
            else /* not optional */
                if (perDecNode(hPer, synChild.nodeId, synChild.speciality, vtPath, synChild.fieldId) <0) return RVERROR;
        }
    } /* for */

    /* -- extensions prefix: 18.7-18.8 */
    if (!isExtended) return vtPath; /* no extensions */
    from = per->decodingPosition;

    /* -- length */
    if (perDecodeLen(perLenTypeNORMALLY_SMALL, &extLen, 0, 0, hPer, from, &dec) <0)
    {
        stringForPrint=pstGetFieldNamePtr(per->hSyn, fieldId);
        msaPrintFormat(ErrPER, "perDecodeSequece: extension length missing [%s].",
            nprn(stringForPrint));
        return RVERROR;
    }
    else from+=dec;

    /* -- extension fields */
    per->decodingPosition = from+extLen; /* skip bit field */

    extChild = pstGetFirstExtendedChild(per->hSyn, synParent);
    if (extChild<0) /* report WARNING */
    {
        stringForPrint=pstGetFieldNamePtr(per->hSyn, fieldId);
        msaPrintFormat(ErrPER, "perDecodeSequece: NO extensions for this version [%s].",
            nprn(stringForPrint));
    }

    /* -- decode extension fields: open type */
    for (i=0; i<(int)extLen; i++)
    {
        perDecodeBool(&isExist, hPer, from+i, &dec);
        if (isExist)
        {  /* extended child exist */

            if (perDecodeLen(perLenTypeUNCONSTRAINED, &fieldLen, 0, 0, hPer,per->decodingPosition, &dec) <0)
            {
                stringForPrint=pstGetFieldNamePtr(per->hSyn, fieldId);
                msaPrintFormat(ErrPER, "perDecodeSequece: extended field length missing [%s/%d].",
                           nprn(stringForPrint), i);
                return RVERROR;
            }

            per->decodingPosition+=dec;
            saveLocation = per->decodingPosition;

            if (extChild >0 && extChild+i <= children)
            {  /* valid extended child */
                pstGetChildExt(per->hSyn, synParent, extChild+i, &synChild);
                if (perDecNode(hPer, synChild.nodeId, synChild.speciality, vtPath, synChild.fieldId) <0) return RVERROR;
            }
            else
            {
                stringForPrint=pstGetFieldNamePtr(per->hSyn, fieldId);
                msaPrintFormat(ErrPER, "perDecodeSequece: New extension (#%d) under [%s].",
                                        i, nprn(stringForPrint));
            }

            per->decodingPosition = saveLocation + fieldLen*8; /* skip field */
        }

    }
    return vtPath;
}

#ifdef __cplusplus
}
#endif



