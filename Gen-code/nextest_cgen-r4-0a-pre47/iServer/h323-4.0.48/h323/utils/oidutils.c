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

#include <ctype.h>
#include <rvinternal.h>
#include <oidutils.h>

char* node[]={(char*)"itu-t",(char*)"iso",(char*)"joint-iso-itu-t",(char*)"ccitt", (char*)"joint-iso-ccitt",(char*)NULL};
char* node0[]={(char*)"recommendation",(char*)"question",(char*)"administration",
        (char*)"network-operator",(char*)"identified-organization",(char*)NULL};
char* node1[]={(char*)"standard",(char*)"1",(char*)"member-body",(char*)"identified-organization",(char*)NULL};
char* series[] = {
    (char*)"",  (char*)"a", (char*)"b", (char*)"c",
    (char*)"d", (char*)"e", (char*)"f", (char*)"g",
    (char*)"h", (char*)"i", (char*)"j", (char*)"k",
    (char*)"l", (char*)"m", (char*)"n", (char*)"o",
    (char*)"p", (char*)"q", (char*)"r", (char*)"s",
    (char*)"t", (char*)"u", (char*)"v", (char*)"w",
    (char*)"x", (char*)"y", (char*)"z", (char*)NULL};


char** nodes[]={node0,node1,NULL};
int sizeNodes[] = {(int)(sizeof(node0) / sizeof(char*)), (int)(sizeof(node1) / sizeof(char*)), -1};

static int findName(char** names,char* name, unsigned nameSize)
{
    int i=0;
    while(names[i])
    {
    if (!strncmp(names[i], name, nameSize) && strlen(names[i])==nameSize)
        return i;
    i++;
    }
    return RVERROR;
}

static
void eatWhiteSpaces(char**startPtr)
{
    while(isspace((int)**startPtr))
        (*startPtr)++;
}

BOOL isNonFirstNameChar(char c)
{
    return isalnum((int)c) || c=='-';
}

static
void eatName(char**startPtr)
{
    while(isNonFirstNameChar(**startPtr))
        (*startPtr)++;
}

static
int eatNumber(char**startPtr)
{
    int number=0;
    while(isdigit((int)**startPtr))
        number=number*10+(*((*startPtr)++)-'0');
    return number;
}

static
int addComp(int* buffSize,char** buff,unsigned number)
{
    int bytes=0;
    int i;
    char c;
    for (i=0;i<5;i++)
    {
        c=(char)(number>>(i?25:28));
        if (*buff)
            **buff=c;
        number<<=(i?7:4);
        if (c||bytes||i==4)
        {
            if (*buff)
            {
                if (*buffSize <= 0)
                    return -1;
                if (i!=4)
                    **buff+='\x80';
                (*buff)++;
            }
            (*buffSize)--;
            bytes++;
        }
    }
    return bytes;
}

/* == oidEncodeOID ==

   oidSize  size of target oid buffer
   oid      target oid buffer, or null
   buff     buffer of string data to encode

   Returns the size of the output string or -1 if there was not enough space.
   If (oid==NULL), the function doesn't write it's output to the oid buffer,
   but the size of the output string is still returned for valid inputs.
   oidSize is ignored in this case.
*/
int oidEncodeOID(int oidSize,char *oid, char *buff)
{
    char *startPtr=buff;
    int size =oidSize;
    int number;
    int compNum=1;
    int comps[2];

    if (oid == NULL)
        oidSize = 0;

    eatWhiteSpaces(&startPtr);
    while(*startPtr && *startPtr!='}')
    {
        char *nexttoken;
        if (isalpha((int)*startPtr))
        {
            char*stopPtr;
            nexttoken=startPtr+1;
            eatName(&nexttoken);
            stopPtr=nexttoken;
            eatWhiteSpaces(&nexttoken);
            if (*nexttoken=='(') /*NameAndNumberForm*/
            {
                nexttoken++;
                eatWhiteSpaces(&nexttoken);
                number=eatNumber(&nexttoken);
                eatWhiteSpaces(&nexttoken);
                if (*nexttoken==')')  nexttoken++;
            }
            else /*NameForm*/
            {
                if (compNum==3 && !comps[0] && !comps[1])
                    number=findName(series, startPtr, (unsigned)(stopPtr-startPtr));
                else
                    number=findName((compNum>1)?nodes[comps[0]]:node, startPtr, (unsigned)(stopPtr-startPtr));
                if (compNum==1) number%=3;
            }
        }
        else if (isdigit((int)*startPtr)) /*NumberForm*/
        {
            nexttoken=startPtr;
            eatWhiteSpaces(&nexttoken);
            number=eatNumber(&nexttoken);
        }
/* !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!! */
        else if (*startPtr == '(')
        {
            nexttoken=++startPtr;
            eatWhiteSpaces(&nexttoken);
            number=eatNumber(&nexttoken);
            eatWhiteSpaces(&nexttoken);
            if (*nexttoken != ')')
                return -1;
            nexttoken++;
        }
        else
            return -1;

        if (compNum<3) comps[compNum-1]=number;
        if (compNum==2)
        {
            number=comps[0]*40+comps[1];
        }

        if (compNum>=2)
            if (addComp(&oidSize,&oid,number) < 0)
                return -1;
        compNum++;
        startPtr=nexttoken;
        eatWhiteSpaces(&startPtr);
    }
    return size-oidSize;
}

int getComp(int *oidSize, unsigned char** oid)
{
    int number=0;
    do
    {
        number<<=7;
        number+=(**oid)&0x7f;
    --(*oidSize);
    }
    while(*oidSize && *((*oid)++)>=0x80);
    return number;
}

#define IF_SPACE_OK(checklen)                   \
{                                               \
    addlen = (checklen);                        \
    if (*buff)                                  \
    {                                           \
        (*buffSize) -= addlen;                  \
        if (*buffSize > 0)

#define END_IF_SPACE                            \
    }                                           \
    len += addlen;                              \
}



int putComp(int* buffSize, char**buff, int num, char**nodes, int numNodes, form f)
{
    char nBuf[10];
    int i=10, len = 0, addlen;
    int saveNum=num;
    nBuf[9]=0;

    /* Make sure the user's number here is within boundaries of the nodes array, otherwise,
       we won't try to write down the node's name since we don't know it. Please note that
       if numNodes < 0, we treat it as if the value is always within the range. */
    if ((numNodes < 0) || (saveNum < numNodes))
    {
        if ((nodes != NULL) && nodes[saveNum] && f!=numberForm)
        {
            IF_SPACE_OK(strlen(nodes[saveNum]) + (f!=nameForm));
            {
                strcpy(*buff,nodes[saveNum]);
                *buff+=strlen(nodes[saveNum]);
                if (f!=nameForm)
                    *((*buff)++)='(';
            }
            END_IF_SPACE;
        }
    }


    do
    {
        nBuf[--i]=(char)((num%10)+'0');
        num/=10;
    }
    while(num);

    if (f != nameForm  ||  !(nodes  &&  nodes[saveNum]))
    {
        IF_SPACE_OK(10 - i);
        {
            memcpy(*buff, nBuf+i, 10-i);
            *buff+=(10-i);
        }
        END_IF_SPACE;
    }

    if (nodes && nodes[saveNum] && f==nameAndNumberForm)
    {
        IF_SPACE_OK(1);
        {
            *((*buff)++)=')';
        }
        END_IF_SPACE;
    }

    IF_SPACE_OK(1);
    {
        *((*buff)++)=' ';
    }
    END_IF_SPACE;

    return len;
}

/* == oidEncodeOID ==

   oidSize  size of source oid buffer
   oid      source oid buffer
   buffSize size of target buffer
   buff     target buffer for decoded oid
   form     the format for the decoded oid

   Returns the size of the output string or -1 if there was not enough space.
   If (buff==NULL), the function doesn't write it's output to the buffer,
   but the size of the output string is still returned for valid inputs.
   buffSize is ignored in this case.
*/

int oidDecodeOID(int oidSize, char* oid, int buffSize, char *buff,form f)
{
    int compNum=2, num, root=0, len = 0;

    if (oidSize < 0)
    {
        if (buff != NULL)
            *buff = 0;
        return RVERROR;
    }

    while(oidSize)
    {
        num = getComp(&oidSize,(unsigned char **)&oid);
        if (num < 0)
        {
            /* Bad OID - number is out of range */
            if (buff != NULL)
                *buff = 0;
            return RVERROR;
        }

        if (compNum>2)
        {
            char** curNodes = NULL;
            int numNodes = 0;

            if (!root && (compNum == 3))
            {
                curNodes = series;
                numNodes = (int)(sizeof(series) / sizeof(char*));
            }
            len += putComp(&buffSize, &buff, num, curNodes, numNodes, f);
        }
        else
        {
            root = num / 40;
            if (root > 2)
            {
                /* joint-iso-itu-t may have more than 40 sub-arcs */
                root = 2;
                num -= 80;
            }
            else
            {
                num %= 40; /* the second level has up to 40 arcs */
            }

            len += putComp(&buffSize, &buff, root, node, (int)(sizeof(node) / sizeof(char*)), f);
            len += putComp(&buffSize, &buff, num, nodes[root], sizeNodes[root], f);
        }
        compNum++;
    }

    if (buff)
        *buff=0;

    return (buffSize >= 0)? len : buffSize;
}

RVAPI int RVCALLCONV
utlDecodeOIDInt(int oidSize, char* oid, int buffSize, int *buff)
{
    int compNum=2, num, root=0, len = 0;

    if (oidSize<0) return RVERROR;

    while(oidSize || buffSize>len)
    {
        num = getComp(&oidSize,(unsigned char **)&oid);
        if (num < 0)
        {
            /* Bad OID - number is out of range */
            return RVERROR;
        }

        if (compNum>2)
        {
            if (buffSize!=len) buff[len++]=num;
        }
        else
        {
            root = num / 40;
            if (root > 2)
            {
                /* joint-iso-itu-t may have more than 40 sub-arcs */
                root = 2;
                num -= 80;
            }
            else
            {
                num %= 40; /* the second level has up to 40 arcs */
            }

            if (buffSize!=len) buff[len++]=root;
            if (buffSize!=len) buff[len++]=num;
        }
        compNum++;
    }
    return len;
}

#ifdef __cplusplus
}
#endif



