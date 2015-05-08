/*

NOTICE:
This document contains information that is proprietary to RADVISION LTD.
No part of this publication may be reproduced in any form whatsoever without
written prior approval by RADVISION LTD.

RADVISION LTD reserves the right to revise this publication and make changes
without obligation to notify any person of such revisions or changes.

*/

/***************************************************************************

  ciFile.c  --  Configuration load/save functions - file version

  Module Author: Oz Solomonovich
  This Comment:  18-Dec-1996

  Abstract:      CI routines for loading and saving the configuration to
                 and from files.

  Platforms:     All.

  Known Bugs:    None.

***************************************************************************/

#ifdef __cplusplus
extern "C" {
#endif


#ifndef NOFILESYSTEM

#ifdef WIN32
#pragma warning (push,3)
#define  STRICT
#include <windows.h>
#pragma warning (pop)
#endif

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>

#include <rvinternal.h>
#include <rtree.h>
#include <rpool.h>
#include <oidutils.h>
#include <strutils.h>
#include <netutl.h>

#include "ci.h"
#include "cisupp.h"

#define FILE_BUF_SIZE 1024

static void buildFromFile(FILE *inf, char *fileBuffer, HRPOOL pool,
                          HRTREE tree, int nodeID, int level, char *assistBuff);

static void outputToFile(FILE *outf, HRTREE tree, int nodeID, int level, HRPOOL pool);

static int parseLine(IN int currLevel, IN char *line,
                     OUT char **name, OUT char **rawValue);



int ciIDFile(const char *source)
{
  return (source  &&  source[0]);
}


void ciGetModuleDirectoryFileName(const char* name, char* szPath, int szPathSize)
{
    char *p;
    p = strchr(name, '\\');
    strncpy(szPath, name, szPathSize);
#if defined(WIN32) && !defined(UNDER_CE)
    if (!p)
    {
        GetModuleFileName(GetModuleHandle(NULL), szPath, szPathSize);
        p = strrchr(szPath, '\\');
        if (p)
        {
            p++;
            strcpy(p, name);
        }
    }
#endif
}


int ciEstimateCfgSizeFile(const char *source, int *nodes, int *data)
{
  FILE *inf;
  char sourceWithPath[300];
  char *fileBuffer = (char*)malloc(FILE_BUF_SIZE);
  char *name, *value;

  if (!fileBuffer)
    return ERR_CI_ALLOCERROR;

  ciGetModuleDirectoryFileName(source, sourceWithPath, sizeof(sourceWithPath));

  if ( (inf = fopen(sourceWithPath, "rb")) == NULL)
  {
    free(fileBuffer);
    return ERR_CI_FILEOPEN;
  }

  *nodes = 0;
  *data  = 0;

  while (fgets(fileBuffer, FILE_BUF_SIZE, inf))
  {
    parseLine(0, fileBuffer, &name, &value);
    if (name)
    {
        int len = strlen(name);
        (*nodes)++;
        (*data) += (len + CONFIG_RPOOL_BLOCK_SIZE - (len % CONFIG_RPOOL_BLOCK_SIZE));
    }
    if (value)
    {
        int len = strlen(value);
        (*data) += (len + CONFIG_RPOOL_BLOCK_SIZE - (len % CONFIG_RPOOL_BLOCK_SIZE));
    }
  }

  fclose(inf);
  free(fileBuffer);

  return 0;
}

int ciBuildFromFile(const char *source, HRTREE tree, HRPOOL pool)
{
  FILE *inf;
  char sourceWithPath[300];
  char *fileBuffer = (char*)calloc(FILE_BUF_SIZE + 1, 1);
  char *assistBuff = (char*)calloc(2*FILE_BUF_SIZE, 1);

  if (!fileBuffer)
    return ERR_CI_ALLOCERROR;

  ciGetModuleDirectoryFileName(source, sourceWithPath,
                               sizeof(sourceWithPath));
  if ( (inf = fopen(sourceWithPath, "rb")) == NULL)
  {
    free(fileBuffer);
    return ERR_CI_FILEOPEN;
  }

  buildFromFile(inf, fileBuffer, pool, tree, rtRoot(tree), 0, assistBuff);
  fclose(inf);
  free(fileBuffer);
  free(assistBuff);

  return 0;
}


int ciOutputToFile(const char *target, HRTREE tree, HRPOOL pool)
{
  FILE *outf;
  char targetWithPath[300];
  ciGetModuleDirectoryFileName(target, targetWithPath,
                               sizeof(targetWithPath));

  if ( (outf = fopen(targetWithPath, "wb")) == NULL)
    return ERR_CI_FILEOPEN;

  /* output standard header */
  fprintf(outf,
    "# RADVISION H.323 Stack Configuration File\n"
    "#\n"
    "# Value encodings:\n"
    "#    ''   -  String (and asciiz is not appended)\n"
    "#    \"\"   -  BMP string of ASCII charactes\n"
    "#    []   -  Hex octet string\n"
    "#    <>   -  IP\n"
    "#    {}   -  Object ID\n"
    "#    %%   -  Bit string\n"
    "#   Other -  Integer\n"
    "\n");

  outputToFile(outf, tree, rtRoot(tree), 0, pool);
  fclose(outf);

  return 0;
}


static char *skipWhite(char *s)
{
  if (s)
    for (; *s  &&  isspace((int)*s); s++);

  return s;
}

static char *findMatchingQuote(char *s, char quote)
{
  /* stops only at '\n' */
  for (; *s != '\n'  &&  *s != quote; s++);

  return (*s == '\n') ? (char*)NULL : s;
}

/* parseLine() returns the new level, or -1 on parse error

   line format:
   [level] name [= value]
   lines beginning with # are a comment.

   level:  a number (the level number), '+' (advance to next level),
           +Number (advance Number levels), ditto for '-'.
           when no level is specified, the same level is intended
*/
static int parseLine(IN int currLevel, IN char *line,
                     OUT char **name, OUT char **rawValue)
{
    static char name_buf[256];
    static char val_buf[256];

    int newLevel, i;
    char sign, *p;

    *name     = NULL;
    *rawValue = NULL;

    line = skipWhite(line);
    if (!line[0]  ||  line[0] == '#')  /* comment or end of line */
    {
        return -1;
    }

    /* first section (optional): the level */
    if (isdigit((int)line[0]))
    {
        newLevel = 0;
        while (line[0]  &&  isdigit((int)line[0]))
        {
            newLevel = (newLevel * 10) + (line[0] - '0');
            line++;
        }
    }
    else
    {
        /* '+' or '-' */
        newLevel = currLevel;
        sign = line[0];
        if (sign == '+'  ||  sign == '-')
        {
            line++;
            if (!line[0])
            {
                return -1;
            }
            if (isdigit((int)line[0]))
            {
                i = 0;
                while (line[0]  && isdigit((int)line[0]))
                {
                    i = (i * 10) + (line[0] - '0');
                    line++;
                }
            }
            else
            {
                i = 1;
            }

            newLevel = newLevel + ((sign == '+')? i : -i);
        }
    }

    line = skipWhite(line);
    if (!line[0]  ||  line[0] == '#')  /* comment or end of line */
    {
        return -1;
    }

    /* second section: the name */
    p = (char *)name_buf;
    while (line[0]  &&  !isspace((int)line[0])  &&  line[0] != '=')
    {
        *p = line[0];
        line++;
        p++;
    }
    *p = '\0';
    *name = (char *)name_buf;
    /* simulate a value of zero in case there is no value */
    strcpy((char *)val_buf, "0");
    *rawValue = (char *)val_buf;

    line = skipWhite(line);
    if (!line[0]  ||  line[0] == '#'  ||  line[0] != '=')
    {
        return newLevel;
    }
    ++line;
    line = skipWhite(line);
    if (!line[0]  ||  line[0] == '#')
    {
        return newLevel;
    }

    /* third section: the value */
    p = (char *)val_buf;
    while (line[0])
    {
        *p = line[0];
        line++;
        p++;
    }
    *p = '\0';

    return newLevel;
}




/* this macro scans the buffer for the matching quote character, then
   updates the following variables:
     p              - beginning of string between quotes
     p2             - end of string (points to the asciiz)
     length         - length of string
     cfgVal->string - allocated variable string
   the macro will break from the loop if the string is null
*/
#define MATCH_QUOTE(quote_char)                                             \
  p2 = findMatchingQuote(p + 1, quote_char);                                \
  if (!p2  ||  ((p2 - p) < 1))                                              \
    break;                                                                  \
  p++;                                                                      \
  *p2 = '\0';                                                               \
  length = p2 - p;                                                          \
  assistLen = length;                                                       \
  assistBuff[length] = '\0';


/* Check that a parent doesn't have a node with a given name.
   If it does - return that node, so we can use it instead of adding a new one
   with the same name */
static int getSameNode(IN HRTREE tree, IN int parent, IN HRPOOL pool, IN const void* nodeName)
{
    int         nodeId;
    int         len;
    pcfgValue   cfgVal;

    nodeId = rtHead(tree, parent);
    if (nodeId < 0)
        return nodeId;

    len = rpoolChunkSize(pool, (void *)nodeName);
    cfgVal = (pcfgValue)rtGetByPath(tree, nodeId);

    while (rpoolChunkSize(pool, cfgVal->name) != len  ||
           rpoolCompareInternal(pool, cfgVal->name, (void *)nodeName, len))
    {
        nodeId = rtBrother(tree, nodeId);
        if (nodeId < 0)
            return RVERROR;
        cfgVal = (pcfgValue)rtGetByPath(tree, nodeId);
    }

    return nodeId;
}

static void buildFromFile(FILE *inf, char *fileBuffer, HRPOOL pool,
                          HRTREE tree, int nodeID, int level, char *assistBuff)
{
    static int lastReadLevel;
    int readLevel, newNodeID, nameCounter = 1;
    UINT32 length;
    cfgValue cfgVal;
    char *p, *p2, *value=NULL, *name=NULL;
    long int pos=0;
    int assistLen;
    
    if (level == 0)
    {
        lastReadLevel = 0;
    }

    for (;;)
    {
        readLevel = -2;
        
        memset(fileBuffer, 0, FILE_BUF_SIZE);
        
        while ((readLevel < 0))
        {
            pos = ftell(inf);
            
            if (fgets(fileBuffer, FILE_BUF_SIZE, inf)==NULL)
                return;
            readLevel = parseLine(lastReadLevel, fileBuffer, &name, &value);
        }
        
        if (readLevel < level)
        {
            /* a higher level, we must exit */
            fseek(inf, pos, SEEK_SET);
            return;
        }
        
        lastReadLevel = readLevel;
        
        /* will resolve '*' names here */
        if (!strcmp(name, "*"))
        {
            if (readLevel != level)
                sprintf(name, "%d", 1);
            else
                sprintf(name, "%d", ++nameCounter);
        }
        else
            if (atoi(name))
                nameCounter = atoi(name);
            
        cfgVal.name = (char *)rpoolAllocCopyExternal(pool, name, (int)strlen(name));
        cfgVal.isString = TRUE;    /* assume string */
        cfgVal.value    = 0;
        cfgVal.str      = NULL;
        
        /* parse the value */
        p = value;
        
        switch (*p)
        {
            case '\'':   /* string */
                MATCH_QUOTE('\'');
                cfgVal.str = rpoolAllocCopyExternal(pool, (void*)p, assistLen);
                cfgVal.value = assistLen;
                break;
            
            case '[':    /* hex string */
            {
                BOOL bIsHighByte;
                char c;
                BYTE i;
                
                MATCH_QUOTE(']');
                p2 = assistBuff;
                cfgVal.value = (assistLen + 1) >> 1;
                memset(assistBuff, 0, cfgVal.value);
                bIsHighByte = !(length & 1);
                for (; length; length--, p++)
                {
                    c = (char)toupper((char)*p);
                    i = (char)((c > '9')? (c - 'A' + (char)10) : (c - '0'));
                    if (bIsHighByte)
                    {
                        *p2 = (char)(i << (char)4);
                    }
                    else
                    {
                        *p2 = (char)(*p2 + i);
                        p2++;
                    }
                    bIsHighByte = !bIsHighByte;
                }
                cfgVal.str = rpoolAllocCopyExternal(pool, (void*)assistBuff, cfgVal.value);
                break;
            }
            
            case '"':   /* BMP string */
                p2 = findMatchingQuote(p + 1, '"');
                if (!p2  ||  ((p2 - p) < 1))
                    break;
                p++;
                *p2 = '\0';
                length = (p2 - p);
                cfgVal.value = length * 2;
                {
                    chrn2bmp(p, (int)length, (BYTE*)assistBuff);
                    cfgVal.str = rpoolAllocCopyExternal(pool, (void *)assistBuff, cfgVal.value);
                }
                break;
            
            case '%':    /* bit string */
            {
                char  buf[1024];
                char  bitBuf[1152];
                int   i, val;
                
                p2 = findMatchingQuote(p + 1, '%');
                if (!p2  ||  ((p2 - p) < 1))
                    break;
                p++;
                *p2 = '\0';
                
                /* str -> bits */
                p2 = buf;
                val = 0;
                i = length = 0;
                while (*p)
                {
                    if (*p != '1'  &&  *p != '0')
                        goto bitstr_out;
                    val += (((unsigned)(*p - '0')) << (7 - i));
                    if (++i > 7)
                    {
                        *p2 = (char)val;
                        i = val = 0;
                        p2++;
                    }
                    p++;
                    length++;  /* count bits */
                }
                
                if (i)
                    *p2 = (char)val;
                
                cfgVal.value = ciBuildBitString(buf, (int)length, bitBuf);
                cfgVal.str   = rpoolAllocCopyExternal(pool, (void *)bitBuf, cfgVal.value);
bitstr_out:
                break;
            }
            
            case '{':    /* object ID */
                MATCH_QUOTE('}');
                /* the target string should be big enough */
                cfgVal.value = oidEncodeOID((int)length, assistBuff, p);
                cfgVal.str   = rpoolAllocCopyExternal(pool, (void *)assistBuff, cfgVal.value);
                break;
            
            case '<':    /* IP */
                MATCH_QUOTE('>');
                length = ip_to_uint32(p);
                cfgVal.value = 4;
                cfgVal.str   = rpoolAllocCopyExternal((HRPOOL)pool, (void *)&length, cfgVal.value);
                break;
            
            default:
                cfgVal.isString = FALSE;
                cfgVal.value    = atoi(p);
        }

        if (level == readLevel)
        {
            newNodeID = getSameNode(tree, rtParent(tree, nodeID), (HRPOOL)pool, cfgVal.name);
            if (newNodeID <= 0)
            {
                /* Such a node doesn't exist - add it in */
                newNodeID = rtAddBrother(tree, nodeID, &cfgVal);
                nodeID = newNodeID;
            }
            else
            {
                /* Already have such a node - ignore this new one */
                if (cfgVal.name != NULL)
                    rpoolFree(pool, cfgVal.name);
                if (cfgVal.str != NULL)
                    rpoolFree(pool, cfgVal.str);
            }
        }
        else
            newNodeID = rtAddTail(tree, nodeID, &cfgVal);
        
        if (nodeID != newNodeID)  /* nest to next level */
            buildFromFile(inf, fileBuffer, pool, tree, newNodeID, level + 1, assistBuff);
    }
}


#define IS_PRINTABLE(c) ((c) >= ' '  &&  (c) < 127)

static void outputToFile(FILE *outf, HRTREE tree, int nodeID, int level, HRPOOL pool)
{
    pcfgValue cfgVal;
    int child, i, printable, isBMP;
    static int lastOutputLevel;
    char buff[MAX_CONFIG_TEMP_BUFFER_SIZE];

    if (level == 0)
        lastOutputLevel = 0;

    for (;;)
    {
        if (level != 0)
        {
            cfgVal = (pcfgValue)rtGetByPath(tree, nodeID);

            /* make the file legable by adding some extra blank lines */
            if (level == 1)
                fprintf(outf, "\n\n");
            else
                if (lastOutputLevel > level + 2)
                    putc('\n', outf);


            /* output the level */
            if (level == 1)  /* always print number on first level */
                fputc('1', outf);
            else
                if (level == lastOutputLevel)
                    fputc(' ', outf);
                else
                    if (level == lastOutputLevel + 1)
                        fputc('+', outf);
                    else
                        if (level == lastOutputLevel - 1)
                            fputc('-', outf);
                        else
                            fprintf(outf, "%d", level);

            /* output the name, padded with spaces on the left */
            rpoolCopyToExternal(pool, (void*)buff, cfgVal->name, 0, MAX_CONFIG_TEMP_BUFFER_SIZE);
            buff[rpoolChunkSize(pool, cfgVal->name)] = 0;
            fprintf(outf, "%*c%s = ", level, ' ',
                atoi(buff) ? "*" : buff);

            if (cfgVal->isString)
            {
                char buff[MAX_CONFIG_TEMP_BUFFER_SIZE];
                rpoolCopyToExternal(pool, (void*)buff, cfgVal->str, 0, MAX_CONFIG_TEMP_BUFFER_SIZE);
                
                /* see if we can print it as a BMP string */

                isBMP = 1; /* assume so */

                if (cfgVal->value % 2 == 0) /* even length */
                {
                    for (i = 0; i < cfgVal->value; i += 2)
                    {
                        if (buff[i] != '\0' ||
                            !IS_PRINTABLE(buff[i + 1]))
                        {
                            isBMP = 0;
                            break;
                        }
                    }
                }
                else
                    isBMP = 0;

                if (isBMP)
                {
                    fputc('"', outf);
                    for (i = 0; i < cfgVal->value; i += 2)
                    {
                        int iChar = buff[i + 1];
                        fputc(iChar, outf);
                    }
                    fprintf(outf, "\"\n");
                }
                else  /* non BMP strings */
                {
                    /* see if string is printable */
                    printable = 1;

                    for (i = 0; i < cfgVal->value; i++)
                    {
                        if (!IS_PRINTABLE(buff[i]))
                        {
                            printable = 0;
                        }
                    }

                    if (printable)
                    {
                        fputc('\'', outf);
                        fwrite(buff, cfgVal->value, 1, outf);
                        fputc('\'', outf);
                        fputc('\n', outf);
                    }
                    else
                    {
                        /* output octet string */
                        fputc('[', outf);
                        for (i = 0; i < cfgVal->value; i++)
                            fprintf(outf, "%02x",
                                    (unsigned char)buff[i]);
                        fprintf(outf, "]\n");
                    }
                }
            }
            else  /* an int */
                fprintf(outf, "%d\n", cfgVal->value);
        }

        lastOutputLevel = level;

        /* process children */
        child = rtHead(tree, nodeID);
        if (child >= 0)
            outputToFile(outf, tree, child, level + 1, pool);

        /* move to brother */
        nodeID = rtBrother(tree, nodeID);
        if (nodeID < 0)
            return;
    }
}
#endif

#ifdef __cplusplus
}
#endif



