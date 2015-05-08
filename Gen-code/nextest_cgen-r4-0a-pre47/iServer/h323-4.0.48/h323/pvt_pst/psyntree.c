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
  psyntree.c

  Ron S.
  31 Oct. 1996

  */


#include <rvinternal.h>
#include <psyntreeDef.h>
#include <psyntreeStackApi.h>
#include <psyntreeDb.h>
#include <psyntree.h>

/*
  synTree.c

  Ron S.
  2 May 1996

  Provides access to syntax tree array.

  Syntax tree format:
  ===================

   -- Header:
    - idSize (small) #bytes for id
    - syntax tree offset (id)
    - field array offset (id)
    - from array offset (id)
    - total size (id)

   -- Syntax tree:
    -- list of nodes:
     -- node:
      - type (small). e.g. SEQUENCE, SET, CHOICE.
      - tag (int)
      - tag class (small)
      - extension exists (bool).
      - number of childs (small).
      - from (int)
      - to (int)
      - OF (id). from node list.
      - FROM (id). from from string array.

      - list of childs:
       -- child:
        - field id (id). from field name array.
    - struct id (id). from node list.
    - is optional (bool)
    - is extended (bool)

   -- Field array:
    - list of strings, each terminated by NULL.

   -- from array:
    - list of strings, each terminated by NULL.

  */

#include <stdio.h>

#include <intutils.h>
#include <strutils.h>
#include <token.h>


#define RV_MAX_PATH 300

/* define all static offsets */

#define stROOT_NODE_SIZE 2*4
#define stNODE_SIZE      9*4
#define stCHILD_SIZE     4*4
#define stROOT_SIZE      2*4


#define stGET(offset, size, num)  { UINT32 __tmp;bigE2Int(syn->syntax + (offset), (size), (UINT32 *)&(__tmp)); num=__tmp; }

#define stGET_BYTE(offset, num)  { num = *(UINT8*)(syn->syntax + (offset)); }








#ifdef PLATFORM_BIG_ENDIAN
#define mnBig 0x42415300ul      /*'BAS\x0'*/
#define mnLittle 0x4c415300ul   /*'LAS\x0'*/
#else
#define mnBig 0x534142ul        /*'\000SAB'*/
#define mnLittle 0x53414cul     /*'\000SAL'*/
#endif


char *
pstGetTagClassName(pstTagClass tagClass)
{
    static char const* const asnTags[]=
    {
        "UNIVERSAL", "APPLICATION", "PRIVATE",  "EMPTY"
    };
    if (tagClass<=0 || tagClass > (int)(sizeof(asnTags)/sizeof(*asnTags)))
        return NULL;
    return (char *)asnTags[tagClass-1];
}


RVAPI char* RVCALLCONV /* null terminated token name */
pstGetTokenName(
      IN pstNodeType type)
{
    static char const* const asnTokens[]=
    {
        "INTEGER", "NULL",      "BOOLEAN",  "ENUMERATED",
        "OBJECT IDENTIFIER",    "OCTET STRING",

        "BIT STRING",           "GeneralString",
        "UniversalString",      "BMPString",
        "IA5String",            "VisibleString",
        "NumericString",        "PrintableString",

        "CHOICE",  "SEQUENCE",  "SET",      "OF",
        "SET OF",               "SEQUENCE OF"
    };
    if ((type <= 0) || (type > (int)(sizeof(asnTokens)/sizeof(*asnTokens))))
        return NULL;
    return (char *)asnTokens[type-1];
}





/*_____________________________________child_______________________________________*/


static int /* child index or RVERROR */
stGetField(
       /* get child by field id (offset) */
       IN  HPST hSyn,
       IN  int nodeId,
       IN  INT32 fieldId, /* offset of child name */
       OUT stChildExt **child
       )
{
    stNodeExt* elementBuffer;
    int childNodeId, index;

    if(hSyn==NULL || fieldId<0) return RVERROR;

    elementBuffer = (stNodeExt *)stGetNodeDataByNodeId(hSyn, nodeId);

    index = stGetChildByNodeAndFieldName(hSyn, nodeId, (int)m_numOfChilds(elementBuffer), fieldId, &childNodeId);
    if (index < 0) return index;

    if (child != NULL)
    {
        *child = (stNodeExt *)stGetNodeDataByNodeId(hSyn, childNodeId);
        if (*child == NULL)
            return RVERROR;
    }

    return index;
}






/*______________________________display_________________________*/

#ifndef NOLOGSUPPORT
/************************************************************************
 * pstPrintNode
 * purpose: Print the information of a syntax tree node to a buffer
 * input  : hSyn    - Syntax tree handle
 *          nodeId  - Node to print
 *          buf     - Result buffer to use
 *          len     - Length of buffer
 * output : none
 * return : Number of characters printed
 ************************************************************************/
int pstPrintNode(
    IN  HPST            hSyn,
    IN  int             nodeId,
    IN  char*           buf,
    IN  int             len)
{
    stNodeExt*  node;
    char*       ptr = buf;
    char*       fromString;
    stNodeExt*  ofNode;

    /* Get the node itself */
    node = (stNodeExt *)stGetNodeDataByNodeId(hSyn, nodeId);
    if (node == NULL) return 0;

    /* 1. GENERAL TYPE */
    ptr += sprintf(ptr, "%s ", nprn(pstGetTokenName((pstNodeType)(m_type(node)))));

    /* 2. TAG */
    if (m_tag(node) >= 0)
      ptr += sprintf(ptr, "[%s %d] ", nprn(pstGetTagClassName((pstTagClass)m_tagClass(node))), m_tag(node));

    /* 3. FROM-TO */
    if ((m_from(node) > 0) || (m_to(node) > 0))
    {
        /* Check for negative boundaries */
        if (m_from(node) <= m_to(node))
            ptr += sprintf(ptr, "(%d..%d) ", m_from(node), m_to(node));
        else
            ptr += sprintf(ptr, "(%u..%u)or(%d..%d) ", m_from(node), m_to(node), m_from(node), m_to(node));
    }

    /* 4. FROM CHARS */
    if (m_fromId(node) >= 0)
    {
        fromString = stGetNameByNameId(hSyn, m_fromId(node), NULL);
        if (fromString[0])
            ptr += sprintf(ptr, "FROM '%s' ", fromString);
    }

    /* 5. EXTENSION */
    if (m_isExtension(node))
    {
        strcpy(ptr, "... ");
        ptr += 4;
    }

    /* 6. SEQUENCE OF recursion */
    switch (m_type(node))
    {
        case pstOf:
        case pstSequenceOf:
        case pstSetOf:
            ofNode = (stNodeExt *)stGetNodeDataByNodeId(hSyn, m_ofId(node));
            if (ofNode != NULL)
            {
                int printedChars = (ptr - buf);
                return printedChars + pstPrintNode(hSyn, m_ofId(node), ptr, len - printedChars);
            }
        default:
            break;
    }

    return (ptr - buf);
}
#endif

/*______________________________tree_________________________*/


/************************************************************************
 * pstConstruct
 * purpose: Create a PST handle for a type in an ASN.1 definition.
 *          This function uses dynamic allocation when called.
 * input  : syntax      - Syntax buffer to use (the output of the ASN.1
 *                        compiler)
 *                        The buffer is the parameter returned by
 *                        cmEmGetCommonSyntax(), cmEmGetH245Syntax() and
 *                        cmEmGetQ931Syntax()
 *          rootName    - Root name of the type to create a PST handle.
 * output : none
 * return : PST handle on success
 *          NULL on failure
 * examples:
 *      hPst = pstConstruct(cmEmGetQ931Syntax(), (char*)"AliasAddress");
 *      hPst = pstConstruct(cmEmGetH245Syntax(), (char*)"Setup-UUIE.fastStart");
 ************************************************************************/
RVAPI HPST RVCALLCONV
pstConstruct(
    IN  unsigned char*  syntax,
    IN  char*           rootName)
{
    synStruct *syn;
    char rootType[128];
    char *path=NULL;
    int nameId;

    if (!syntax || !rootName) return NULL;

    syn = (synStruct *)calloc(sizeof(synStruct), 1);
    if (!syn) return NULL;

    syn->syntax = (fmt2Struct *)syntax;

    /* handle internal root definition (path handling) */
    if ((path = strchr(rootName, '.')))
    {
        strncpy(rootType, rootName, path-rootName+1);
        rootType[path-rootName] = 0;
        path++;
    }
    else
    {
        strcpy(rootType, rootName);
    }

    syn->rootNodeId = stGetNodeIdByName((HPST)syn, rootType);
    if (syn->rootNodeId >= 0)
        syn->rootNameId = stGetNameIdByNodeId((HPST)syn, syn->rootNodeId);

    if (path)
    {
        syn->rootNodeId = pstGetNodeIdByPath((HPST)syn, path);
        if (syn->rootNodeId >= 0)
        {
            nameId = stGetNameIdByNodeId((HPST)syn, syn->rootNodeId);
            syn->rootNameId = pstGetFieldId((HPST)syn, stGetNameByNameId((HPST)syn, nameId, NULL));
        }
    }

    return (HPST)syn;
}


/************************************************************************
 * pstDestruct
 * purpose: Destruct a PST handle created by pstConstruct().
 * input  : hSyn    - PST handle to destruct
 * output : none
 * return : Non-negative value on success
 *          Negative value on failure
 ************************************************************************/
RVAPI int RVCALLCONV
pstDestruct(IN HPST hSyn)
{
    synStruct *syn = (synStruct *)hSyn;

    if (!syn) return RVERROR;
    free(syn);
    return TRUE;
}


RVAPI pstNodeType RVCALLCONV /* type of node */
pstGetNodeType(
           IN  HPST hSyn,
           IN  int nodeId
           )
{
    stNodeExt* node;

    if (hSyn != NULL)
    {
        node = (stNodeExt *)stGetNodeDataByNodeId(hSyn, nodeId);
        if (node != NULL)
            return (pstNodeType)m_type(node);
    }

    return (pstNodeType)RVERROR;
}


RVAPI int RVCALLCONV /* type of node */
pstGetNodeOfId(
           IN  HPST hSyn,
           IN  /*INT32*/int nodeId
           )
{
    stNodeExt* node;

    node = (stNodeExt *)stGetNodeDataByNodeId(hSyn, nodeId);
    if (node != NULL)
        return m_ofId(node);
    else
        return RVERROR;
}

RVAPI int RVCALLCONV /*the tag of the node or RVERROR*/
pstGetTag(
         IN  HPST        hSyn,
         IN  int         nodeId,
         OUT pstTagClass* tagClass
         )
{
    stNodeExt* node;

    node = (stNodeExt *)stGetNodeDataByNodeId(hSyn, nodeId);
    if (node != NULL)
    {
        if (tagClass)
            *tagClass = (pstTagClass)m_tagClass(node);
        return m_tag(node);
    }
    else
        return RVERROR;
}

RVAPI int RVCALLCONV /*is not extended or RVERROR*/
pstGetIsExtended(
     IN  HPST        hSyn,
     IN  int         nodeId)
{
    stNodeExt* node;

    node = (stNodeExt *)stGetNodeDataByNodeId(hSyn, nodeId);
    if (node != NULL)
        return m_isExtension(node);
    else
        return RVERROR;
}

RVAPI int RVCALLCONV /*is not extended or RVERROR*/
pstGetIsOpenType(
     IN  HPST        hSyn,
     IN  int         nodeId)
{
    stNodeExt* node;

    node = (stNodeExt *)stGetNodeDataByNodeId(hSyn, nodeId);
    if (node != NULL)
        return m_flags(node) & isOpenType;
    else
        return RVERROR;
}


RVAPI int RVCALLCONV /* type of node */
pstGetNodeRange(
            IN  HPST hSyn,
            IN  int nodeId,
                OUT int *from,
                OUT int *to
           )
{
    stNodeExt* node;

    node = (stNodeExt *)stGetNodeDataByNodeId(hSyn, nodeId);
    if (node == NULL) return RVERROR;

    if (from)  *from = m_from(node);
    if (to)    *to   = m_to(node);

    return (m_to(node) == 0)? RVERROR : 0;
}


RVAPI INT32 RVCALLCONV /* Field enumeration or RVERROR */
pstGetFieldId(
          /* convert field name to internal id */
          IN  HPST hSyn,
          IN  const char *fieldName /* null terminated string */
          )
{
    if (hSyn == NULL) return RVERROR;

    return stGetNameIdByName(hSyn, (char *)fieldName);
}

RVAPI int RVCALLCONV /* Real length of field name (exluding null) or RVERROR */
pstGetFieldName(
        /* convert field internal id to field name */
        IN  HPST hSyn,
        IN  INT32 fieldId,
        IN  int fieldNameLength, /* num. of bytes in string allocation */
        OUT char* fieldName /* null terminated. user allocated */
        )
{
    char* string;
    int   length;

    string = stGetNameByNameId(hSyn, fieldId, &length);

    /* Check if we've got a valid string to pass on */
    if ((fieldName != NULL) && (fieldNameLength > 0)) fieldName[0] = 0;
    if (string == NULL) return RVERROR;

    /* Copy the result to the user's buffer */
    if ((fieldName != NULL) && (fieldNameLength > 0))
        strncpy(fieldName, string, fieldNameLength);

    return length;
}



RVAPI  int RVCALLCONV /* actual length of the fromString or RVERROR */
pstGetFROMString(
         /* Get the character constraints of the syntax node */
         IN  HPST hSyn,
         IN  int nodeId,
         IN  int fromStringLength, /* num. of bytes in string allocation */
         OUT char* fromString /* null terminated. user allocated */
          )
{
    char *string;
    int actualLength;

    if (fromString && fromStringLength>0) fromString[0]=0;
    string=pstGetFROMStringPtr(hSyn, nodeId, &actualLength);
    if (!string || actualLength<0) return RVERROR;

    if (fromString && fromStringLength>0)
        strncpy(fromString, string, fromStringLength);
    return actualLength;
}


RVAPI /*INT32*/int RVCALLCONV /* Internal node id or RVERROR */
pstGetNodeIdByPath(
           /* get internal node id from specified node path.
              Path to node should start at root, and correspond to the
              ASN module syntax structure. */
           IN  HPST hSyn,
           IN  const char *path  /* format: "a.b.c" */
           )
{
    char name[RV_MAX_PATH];
    char *ptr=NULL, *nameptr=name;
    INT32 fieldEnum;
    stChildExt *child = NULL;
    stNodeExt *sNode;
    int sNodeId;

    if (!hSyn) return RVERROR;
    if (!path) return pstGetRoot(hSyn);
    strncpy(name, path, RV_MAX_PATH);
    sNodeId = pstGetRoot(hSyn);

    for(;;)
    {
        ptr=strchr(nameptr,'.');
        if (ptr != NULL) *ptr=0;

        if (nameptr[0] != '\0')
        {
            if (isNumber(nameptr))
            {   /* sequence of index */
                sNode = (stNodeExt *)stGetNodeDataByNodeId(hSyn, sNodeId);
                sNodeId = m_ofId(sNode);
            }
            else
            {  /* other */
                if ( (fieldEnum=pstGetFieldId(hSyn, nameptr)) <0) return RVERROR;
                if (stGetField(hSyn, sNodeId, fieldEnum, &child) <0) return RVERROR;
                sNodeId=m_structId(child);
            }
        }

        nameptr=ptr+1;
        if (ptr == NULL)
            break;
    }

    return sNodeId;
}

RVAPI int RVCALLCONV /* get root name */
pstGetRootName(
               IN  HPST  hSyn,
               IN  int   bufferLength,
               OUT char *buffer)
{
    char* string;
    int length;
    synStruct *syn = (synStruct *)hSyn;

    if (syn == NULL) return RVERROR;
    string = stGetNameByNameId(hSyn, syn->rootNameId, &length);

    if (string == NULL)
    {
        if ((buffer != NULL) && (bufferLength > 0)) buffer[0] = 0;
        return RVERROR;
    }

    if ((buffer != NULL) && (bufferLength > 0))
        strncpy(buffer, string, bufferLength);

    return length;
}

RVAPI int RVCALLCONV
pstGetNumberOfChildren(
                    IN HPST  hSyn,
                    IN int   nodeId)
{
    stNodeExt* node;

    node = (stNodeExt *)stGetNodeDataByNodeId(hSyn, nodeId);
    if (node != NULL)
        return m_numOfChilds(node);
    else
        return RVERROR;
}


RVAPI int RVCALLCONV
pstGetChild(
            IN  HPST      hSyn,
            IN  int       nodeId,      /* node id of parent */
            IN  int       childIndex,  /* index of child */
            OUT pstChild *child
           )
{
    stChildExt* fieldInfo;
    int err;

    if (child)
        child->index = RVERROR;  /* assume the worst */

    /* Get the information */
    err = stGetChildByIndex(hSyn, nodeId, childIndex, &fieldInfo);
    if (err < 0) return err;

    /* Convert it to something readable */
    if (child)
    {
        child->index      = childIndex;
        child->nodeId     = m_structId(fieldInfo);
        child->fieldId    = m_fieldId(fieldInfo);
        child->isOptional = m_isOptional(fieldInfo);
    }

    return err;
}


RVAPI BOOL RVCALLCONV
pstIsStringNode(
           IN  HPST     hSyn,
           IN  int      nodeId
           )
{
    pstNodeType type = pstGetNodeType(hSyn, nodeId);
    if (((int)type) < 0)
        return FALSE;

    switch (type)
    {
        case pstObjectIdentifier:
        case pstOctetString:
        case pstBitString:
        case pstGeneralString:
        case pstUniversalString:
        case pstBMPString:
        case pstIA5String:
        case pstVisibleString:
        case pstNumericString:
        case pstPrintableString:  return TRUE;
        default:                  break;
    }
    return FALSE;
}




#ifdef __cplusplus
}
#endif



