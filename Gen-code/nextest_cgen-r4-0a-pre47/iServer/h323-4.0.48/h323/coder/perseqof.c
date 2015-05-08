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
  perSequenceOf.c

  Ron S.
  16 May 1996


  ____________________________________________________________________________
  ___________________________SEQUENCE_OF______________________________________
  ____________________________________________________________________________


  format:

  +----------+--------+---------+       +---------+
  | ext. bit | length | field-1 |  ...  | field-n |
  +----------+--------+---------+       +---------+

  */


#include <stdio.h>
#include <perintr.h>


/*
  Desc: Encode a sequence OF node.
  According to clause `9.
  Node value contains the number of components in the set.


*/
int
perEncodeSequeceOF(IN  HPER hPer,
           IN  int synParent,
           IN  int valParent, /* this is me */
           IN  INT32 fieldId)
{
    perStruct *per =(perStruct *)hPer;
    int vtPath=-1, numOfComponents=-1;
    BOOL IsOK = TRUE, isExtended = FALSE;
    int  to,from;
    BOOL toAbsent,fromAbsent;
    int ret = 0;

    /* -- get number of components */
    numOfComponents = pvtNumChilds(per->hVal, valParent);
    if (numOfComponents <0)
    {
        msaPrintFormat(ErrPER, "perEncodeSequeceOF: Value node not exist. [%s]",
            pstGetFieldNamePtr(per->hSyn, fieldId));
        return RVERROR;
    }
    pstGetNodeRangeExt(per->hSyn,synParent,&from,&to,&fromAbsent,&toAbsent);

    if (pstGetIsExtended(per->hSyn,synParent) == FALSE) /* -- validate numOfComponents */
    {
        if (!fromAbsent && !toAbsent &&
            from == to &&  numOfComponents != from) IsOK = FALSE; /* no length */
        if ((!toAbsent && numOfComponents > to) || (!fromAbsent && numOfComponents <from))
            IsOK = FALSE;

        if (!IsOK)
        {
            msaPrintFormat(ErrPER, "perEncodeSequeceOF:%s: Illegal number of elements for set. [%d]",
                pstGetFieldNamePtr(per->hSyn, fieldId), numOfComponents);
            return RVERROR;
        }
    }
    else /* -- extension */
    {
        if ((!fromAbsent && numOfComponents < from) || (!toAbsent && to < numOfComponents))
        {
            isExtended = TRUE;
            perEncodeBool(TRUE, per->hBB); /* adding extension bit. */
        }
        else
            perEncodeBool(FALSE, per->hBB);

            /*
            msaPrintFormat(ErrPER, "perEncodeSequeceOF: NO EXTENSION SUPPORT [%s].",
            pstGetFieldNamePtr(per->hSyn, fieldId));
            return RVERROR;
        */
    }

    /* -- length: 19.7 */
    if ((fromAbsent && toAbsent) || isExtended)
        /* encode length as unconstrained mode. */
        perEncodeLen(perLenTypeUNCONSTRAINED, numOfComponents, 0, 0, per->hBB);
    else
        if (!fromAbsent && !toAbsent) /* encode length as constrained. */
            perEncodeLen(perLenTypeCONSTRAINED, numOfComponents, from, to, per->hBB);
        else
            if (!fromAbsent && toAbsent)
                perEncodeLen(perLenTypeUNCONSTRAINED, numOfComponents, from, 0, per->hBB);

    /* -- components encoding */
    vtPath=pvtChild(per->hVal,valParent);
    while ((vtPath >= 0) && (ret >= 0))
    {
        ret = perEncNode(hPer, pstGetNodeOfId(per->hSyn, synParent), vtPath, fieldId, FALSE);
        vtPath=pvtBrother(per->hVal,vtPath);
    }

    return ret;
}


/*
  Desc: Decode a sequence OF node.
  According to clause `9.
  Node value contains the number of components in the set.

*/
int
perDecodeSequeceOF(IN  HPER hPer,
           IN  int synParent, /* parent in syntax tree */
           IN  int valParent, /* field parent in value tree */
           IN  INT32 fieldId)   /* enum of current field */
{
  BOOL isExtended=FALSE;
  perStruct *per = (perStruct *)hPer;

    UINT32 dec = 0; /* decoded bits */
    int numOfComponent=0;
    int vtPath=-1,i;
    UINT32 pos_from = per->decodingPosition; /* internal position */

    int  to,from;
    BOOL toAbsent,fromAbsent;
    BOOL extended= pstGetIsExtended(per->hSyn, synParent);

    /* -- extension */
    if (extended == TRUE)
    {
        perDecodeBool( &isExtended, hPer, pos_from, &dec); /* decoding extension bit. Sergey M. */
        pos_from+=dec;
        /*
            msaPrintFormat(ErrPER, "perDecodeSequeceOF: NO EXTENSION SUPPORT [%s].",
            pstGetFieldNamePtr(per->hSyn, fieldId));
                return RVERROR;
        */
    }
    pstGetNodeRangeExt(per->hSyn,synParent,&from,&to,&fromAbsent,&toAbsent);

    /* -- number of components: 19.7 */
    /* -- length: 19.7 */
    if (toAbsent || isExtended)   /* encode length as unconstrained mode. */
    {
        if(perDecodeLen(perLenTypeUNCONSTRAINED, (UINT32 *)&numOfComponent, 0, 0, hPer, pos_from, &dec)<0) return RVERROR;
        else pos_from+=dec;
    }
    else
    if (!fromAbsent && !toAbsent) /* encode length as constrained. */
    {
        if(from != to)
        {
            if (perDecodeLen(perLenTypeCONSTRAINED, (UINT32 *)&numOfComponent,
                from, to, hPer, pos_from, &dec) <0) return RVERROR;
            else pos_from+=dec;
        }
        else numOfComponent = from;
    }

    if (extended==FALSE &&
        ((!toAbsent && (numOfComponent > to)) ||
         (!fromAbsent && (numOfComponent < from))))
     {
       msaPrintFormat(ErrPER, "perDecodeSequeceOF: %s: Illegal number of elements for set. [%d]",
              pstGetFieldNamePtr(per->hSyn, fieldId), numOfComponent);
       return RVERROR;
     }




    per->decodingPosition = pos_from;

  /* -- components decoding */
    vtPath=valParent;
    if (fieldId!=RVERROR)
    vtPath = pvtAdd(per->hVal, valParent, fieldId, numOfComponent, NULL, NULL);

    for (i=0; i<(int)numOfComponent; i++)
    if (perDecNode(hPer, pstGetNodeOfId(per->hSyn,synParent), pstNotSpecial ,vtPath, -33) <0)
        return RVERROR;

  return vtPath;
}

#ifdef __cplusplus
}
#endif



