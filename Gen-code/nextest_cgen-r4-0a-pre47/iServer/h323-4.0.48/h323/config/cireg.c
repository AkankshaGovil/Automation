#ifdef __cplusplus
extern "C" {
#endif




/*

NOTICE:
This document contains information that is proprietary to RADVISION LTD.
No part of this publication may be reproduced in any form whatsoever without
written prior approval by RADVISION LTD.

RADVISION LTD reserves the right to revise this publication and make changes
without obligation to notify any person of such revisions or changes.

*/

/***************************************************************************

  ciRegistry.c  --  Configuration load/save functions - Registry version

  Module Author: Oz Solomonovich
  This Comment:  1-Jan-1996

  Abstract:      CI routines for loading and saving the configuration to
                 and from the Windows registry.

  Platforms:     Windows 95, Windows NT only.

  Known Bugs:    1. Untested under NT.
                 2. Remote registry services untested.


  Specifying The Registry Key
  ---------------------------

  CI identifies registry requests by searching for "::" inside the source
  string.  The full format for the registry source string is:

    <::<\\remote machine name\><root key\><user name\>subkey

  remote machine name - an optional name of a remote machine that supports
                        remote registry access services.

  root key -            can be one of the following:
                        o HKEY_LOCAL_MACHINE
                        o HKEY_CURRENT_USER
                        o HKEY_USERS

                        If omitted, HKEY_LOCAL_MACHINE is
                        used by default.

  user name -           If root key is set as HKEY_USERS, it must be
                        followed by a user name.

  subkey -              The subkey to use for the configuration.  This is
                        not a direct subkey under the root key, but instead
                        a subkey under:
                          <root key>\Software\RADVISION\H323 Configuration\

  NOTE:  User and remote machine names cannot excede 255 characters!

  Examples:

  "::default"
    Will instruct CI to locate the data in the local machine registry under
    the key:
      HKEY_LOCAL_MACHINE\Software\RADVISION\H323 Configuration\default

  "::HKEY_LOCAL_MACHINE\default"
    Same as above.

  "::HKEY_USERS\Bill Gates\Gateway"
    Will instruct CI to locate the data in the local machine registry under
    the key:
      HKEY_USERS\Bill Gates\Software\RADVISION\H323 Configuration\Gateway

  "::\\foobar\HKEY_CURRENT_USER\Config1"
    Will instruct CI to locate the data in a remote machine named foobar,
    under the key
      HKEY_CURRENT_USER\Software\RADVISION\H323 Configuration\Config1

***************************************************************************/
#ifdef WIN32

#define NOGDI
#define NOIME
#define NOMCX
#define NONLS
#define NOUSER
#define NOHELP
#define NOSYSPARAMSINFO

#pragma warning (push,3)
#define  STRICT
#include <windows.h>
#include <winreg.h>
#pragma warning (pop)

#include <string.h>
#include <tchar.h>

#include <rvinternal.h>
#include <rtree.h>
#include <rpool.h>
#include <strutils.h>

#ifdef UNDER_CE
WCHAR * StrToWStr(char * StrIn, WCHAR * wStrOut);
char *  WStrToStr(WCHAR * wStrIn, char * StrOut);

void    CharToWChar(char CharIn, WCHAR * wCharOut);
void    WCharToChar(WCHAR wCharIn, char * CharOut);

void    DisplayLastError(void);     // debug only
#endif

#include "ci.h"
#include "cisupp.h"

#define KEY_NAME_BUFFER_SIZE  64   /* more than should be needed */

#define SAM_READ   KEY_QUERY_VALUE | KEY_ENUMERATE_SUB_KEYS
#define SAM_WRITE  KEY_WRITE

/* _T defines string literal as a wide-character-string if */
/* UNICODE is in effect, or otherwise as a regular string  */
#define VALNAME_STR    _T("Str")
#define VALNAME_INT    _T("Value")


#define DEFAULT_KEY    HKEY_LOCAL_MACHINE
#define KEY_NAME_BASE  "Software\\RADVISION\\H323 Configuration"



static void deleteSubKeys    (HKEY key);

static int  openRegistryKey  (const char *source, REGSAM sam,
                              PHKEY key, int create);

static int  estimateCfgSize  (HKEY key, int *nodes, int *data);

static void buildFromRegistry(HKEY key, HRPOOL pool, HRTREE tree,
                              int nodeID, char *name, int level);

static int  outputToRegistry (HKEY key, HRTREE tree, int nodeID, HRPOOL pool);



#define REG_ID_STRING  "::"
#define REG_ID_LEN     2

/* should start with "::" */
int ciIDRegistry(const char *source)
{
  return (source  &&  strlen(source) > REG_ID_LEN  &&
          !strncmp(source, REG_ID_STRING, REG_ID_LEN));
}


int ciEstimateCfgSizeRegistry(const char *source, int *nodes, int *data)
{
  HKEY key;
  int retVal;

  if (openRegistryKey(source, SAM_READ, &key, 0) != ERROR_SUCCESS)
    return ERR_CI_REGOPENKEY;

  *nodes = 0;
  *data  = 0;

  retVal = estimateCfgSize(key, nodes, data);

  RegCloseKey(key);

  return retVal;
}

int ciBuildFromRegistry(const char *source, HRTREE tree, HRPOOL pool)
{
  HKEY key;

  if (openRegistryKey(source, SAM_READ, &key, 0) != ERROR_SUCCESS)
    return ERR_CI_REGOPENKEY;

  buildFromRegistry(key, pool, tree, rtRoot(tree), 0, 0);

  RegCloseKey(key);

  return 0;
}


int ciOutputToRegistry(char *target, HRTREE tree, HRPOOL pool)
{
  HKEY key;
  int retVal;

  if (openRegistryKey(target, SAM_WRITE, &key, 0) == ERROR_SUCCESS)
  {
    deleteSubKeys(key);
    RegCloseKey(key);
  }

  if (openRegistryKey(target, SAM_WRITE, &key, 1) != ERROR_SUCCESS)
    return ERR_CI_REGCREATE;

  retVal = outputToRegistry(key, tree, -1, pool);

  RegCloseKey(key);

  return retVal;
}


/* recursivly deletes a key and all it's decendents */
static void deleteSubKeys(HKEY key)
{
  HKEY child;
  TCHAR name_buf[KEY_NAME_BUFFER_SIZE];
  int i = 0;
  DWORD buf_size;
  FILETIME file_time;

  /* process the children */
    i = 0;
    buf_size = KEY_NAME_BUFFER_SIZE;
    while (RegEnumKeyEx(key, i, name_buf, &buf_size, NULL, NULL, NULL, &file_time) == ERROR_SUCCESS)
    {
        if (RegOpenKeyEx(key, name_buf, 0, SAM_READ, &child) != ERROR_SUCCESS)
          return;

        deleteSubKeys(child);
        RegCloseKey(child);
        if (RegDeleteKey(key, name_buf) != ERROR_SUCCESS)
            i++;
        buf_size = KEY_NAME_BUFFER_SIZE;
    }

    return;
}


static int estimateCfgSize(HKEY key, int *nodes, int *data)
{
    HKEY child;
    DWORD i;
    DWORD keyType, bufSize = KEY_NAME_BUFFER_SIZE;
    TCHAR name_buf[KEY_NAME_BUFFER_SIZE];
    FILETIME file_time;
    int len = 0;
    
    /* see if this key has a string parameter */
    if (RegQueryValueEx(key, VALNAME_STR, NULL, &keyType, 0, &i) ==
        ERROR_SUCCESS  &&  keyType == REG_BINARY)
    {
        *data += i;
    }
    
    /* process the children */
    i = 0;
    
    while (RegEnumKeyEx(key, i, name_buf, &bufSize, NULL, NULL, NULL, &file_time) == ERROR_SUCCESS)
    {
        if (RegOpenKeyEx(key, name_buf, 0, SAM_READ, &child) != ERROR_SUCCESS)
        {
            return ERR_CI_REGOPENKEY;
        }
        (*nodes)++;
        
#ifdef UNICODE
        len = (wcslen(name_buf) + 1) * 2;        /* wide character = 2 bytes */
        (*data) += (len + CONFIG_RPOOL_BLOCK_SIZE - (len % CONFIG_RPOOL_BLOCK_SIZE));
#else
        len = strlen(name_buf) + 1;
        (*data) += (len + CONFIG_RPOOL_BLOCK_SIZE - (len % CONFIG_RPOOL_BLOCK_SIZE));
#endif
        
        estimateCfgSize(child, nodes, data);
        RegCloseKey(child);
        
        i++;
        
        bufSize = KEY_NAME_BUFFER_SIZE;
    }
    
    return 0;
}


static void buildFromRegistry(HKEY key, HRPOOL pool, HRTREE tree,
                              int nodeID, char *name, int level)
{
  cfgValue cfgVal;
  HKEY child;
  int newNodeID;
  DWORD i, keyType, bufSize = KEY_NAME_BUFFER_SIZE;
  TCHAR name_buf[KEY_NAME_BUFFER_SIZE];
  FILETIME file_time;

  if (level)
  {
    cfgVal.name = rpoolAllocCopyExternal(pool, (void *)name, strlen(name));

    if (RegQueryValueEx(key, VALNAME_STR, 0, &keyType, 0,
      (LPDWORD)&cfgVal.value) == ERROR_SUCCESS  &&  keyType == REG_BINARY)
    {
        char buff[MAX_CONFIG_TEMP_BUFFER_SIZE];
        RegQueryValueEx(key, VALNAME_STR, 0, &keyType, (LPBYTE)buff, (LPDWORD)&cfgVal.value);
        cfgVal.isString = 1;
        cfgVal.str      = rpoolAllocCopyExternal(pool, (void *)buff, cfgVal.value);
    }
    else /* an int */
    {
      cfgVal.isString = 0;
      i = sizeof(cfgVal.value);

      RegQueryValueEx(key, VALNAME_INT, 0, &keyType, (LPBYTE)&cfgVal.value,
        &i);
    }

    newNodeID = rtAddTail(tree, nodeID, &cfgVal);
    nodeID = newNodeID;
  }

  /* process the children */
  i = 0;
  
  while (RegEnumKeyEx(key, i, name_buf, &bufSize, NULL, NULL, NULL, &file_time) == ERROR_SUCCESS)
  {
      if (RegOpenKeyEx(key, name_buf, 0, SAM_READ, &child) != ERROR_SUCCESS)
          return;  /* will fail miserably */
      buildFromRegistry(child, pool, tree, nodeID, (char *)name_buf,
          level + 1);
      RegCloseKey(child);
      
      i++;
      bufSize = KEY_NAME_BUFFER_SIZE;
  }
  
}

static int outputToRegistry(HKEY key, HRTREE tree, int nodeID, HRPOOL pool)
{
    pcfgValue cfgVal;
    HKEY child;
    int rtChild, retVal;
    DWORD disposition;
    char buff[MAX_CONFIG_TEMP_BUFFER_SIZE];
#ifdef UNICODE
    WCHAR wstr[256];
#endif
    
    if (nodeID >= 0)
    {
        cfgVal = (pcfgValue)rtGetByPath(tree, nodeID);
        if(cfgVal->str)
        {
            rpoolCopyToExternal(pool, (void*)buff, cfgVal->str, 0, MAX_CONFIG_TEMP_BUFFER_SIZE);
        }

        if (cfgVal->isString)
            RegSetValueEx(key, VALNAME_STR, 0, REG_BINARY,
                (CONST BYTE*)(cfgVal->str? buff : ""), cfgVal->value);
        else  /* an int */
            RegSetValueEx(key, VALNAME_INT, 0, REG_DWORD,
                (CONST BYTE*)&(cfgVal->value), sizeof(cfgVal->value));
  }
  else
    nodeID = rtRoot(tree);

  /* process children */
  rtChild = rtHead(tree, nodeID);
  while (rtChild >= 0)
  {
    cfgVal = (pcfgValue)rtGetByPath(tree, rtChild);
    rpoolCopyToExternal(pool, (void*)buff, cfgVal->name, 0, MAX_CONFIG_TEMP_BUFFER_SIZE);
    buff[rpoolChunkSize(pool, cfgVal->name)] = 0;
    
#ifdef UNICODE
    wprintf(wstr, L"%hs", buff);
    if (RegCreateKeyEx(key, wstr, 0, 0, REG_OPTION_NON_VOLATILE,
      SAM_WRITE, 0, &child, &disposition) < 0)
#else
    if (RegCreateKeyEx(key, buff, 0, 0, REG_OPTION_NON_VOLATILE,
      SAM_WRITE, 0, &child, &disposition) < 0)
#endif
    {
      return ERR_CI_REGCREATE;
    }
    if ((retVal = outputToRegistry(child, tree, rtChild, pool)) < 0)
      return retVal;

    rtChild = rtBrother(tree, rtChild);
  }

  return 0;
}


#define KEY_DEF(x)   #x, x

typedef struct
{
  const char *keyName;
  HKEY       key;
} presetKey;

presetKey presetKeys[] =
{
  KEY_DEF(HKEY_CURRENT_USER),
  KEY_DEF(HKEY_USERS),
  KEY_DEF(HKEY_LOCAL_MACHINE),
};


/* handles name mapping.  see note at beginning of file. */
static int openRegistryKey(const char *source, REGSAM sam, PHKEY key,
                           int create)
{
  const char *p, *src = source + REG_ID_LEN;
  DWORD i;
  unsigned int len, retVal;
#ifndef UNDER_CE
  int isRemote = 0;
#endif
  int closeSourceKey = 0, isUsers = 0;
  HKEY sourceKey = DEFAULT_KEY, baseKey;
  char name_buffer[256];
#ifdef UNICODE
    WCHAR wstr [256];
#endif

  if (!strncmp((char *)src, "\\\\", 2))
  {
#ifdef UNDER_CE
    /* remote registery not supported by Windows CE */
      return ERROR_PATH_NOT_FOUND;
#else

     src += 2;
    p = strchr(src + 1, '\\');
    if (!p)
      return ERROR_PATH_NOT_FOUND;
    len = p - src;
    strncpy(name_buffer, src, len);
    name_buffer[len] = '\0';
    isRemote = 1;
    src = p;
#endif
  }

  if (*src == '\\')
    src++;

  /* try to match first token with a preset key name */
  p = strchr(src + 1, '\\');
  if (p)
  {
    len = p - src;

    for (i = 0; i < sizeof(presetKeys) / sizeof(presetKey); i++)
    {
      if (!strncmp_ci(presetKeys[i].keyName, src, len)  &&
          strlen(presetKeys[i].keyName) == len)
      {
        sourceKey = presetKeys[i].key;
        src = p + 1;
        break;
      }
    }
  }

  isUsers = (sourceKey == HKEY_USERS);

#ifndef UNDER_CE
  if (isRemote)
  {
    if ((retVal = RegConnectRegistry(name_buffer, sourceKey, &baseKey)) !=
      ERROR_SUCCESS)
    {
      goto exit;
    }
    sourceKey = baseKey;
    closeSourceKey = 1;
  }
#endif

  if (isUsers)
  {
    /* extract user name */
    p = strchr(src + 1, '\\');
    if (!p)
    {
      retVal = ERROR_PATH_NOT_FOUND;
      goto exit;
    }
    len = p - src;
    strncpy(name_buffer, src, len);
    name_buffer[len] = '\0';
    src = p + 1;

#ifdef UNICODE
    wprintf(wstr, L"%hs", name_buffer);
    if ((retVal = RegOpenKeyEx(sourceKey, wstr, 0, sam, &baseKey)) !=
#else
    if ((retVal = RegOpenKeyEx(sourceKey, name_buffer, 0, sam, &baseKey)) !=
#endif
        ERROR_SUCCESS)
    {
      goto exit;
    }
    if (closeSourceKey)  /* in case of remote connection */
      RegCloseKey(sourceKey);
    else
      closeSourceKey = 1;
    sourceKey = baseKey;
  }

#ifdef UNDER_CE
  wsprintf (wstr, L"%hs", KEY_NAME_BASE);
  retVal = create?
    RegCreateKeyEx(sourceKey, wstr, 0, 0, REG_OPTION_NON_VOLATILE,
                   SAM_WRITE, 0, &baseKey, &i) :
    RegOpenKeyEx(sourceKey, wstr, 0, sam, &baseKey);
#else
  retVal = create?
    RegCreateKeyEx(sourceKey, KEY_NAME_BASE, 0, 0, REG_OPTION_NON_VOLATILE,
                   SAM_WRITE, 0, &baseKey, &i) :
    RegOpenKeyEx(sourceKey, KEY_NAME_BASE, 0, sam, &baseKey);
#endif

  if (retVal != ERROR_SUCCESS)
    goto exit;


#ifdef UNICODE
  wsprintf (wstr, L"%hs", src);
  retVal = create?
    RegCreateKeyEx(baseKey, wstr, 0, 0, REG_OPTION_NON_VOLATILE,
                   SAM_WRITE, 0, key, &i) :
    RegOpenKeyEx(baseKey, wstr, 0, sam, key);
#else
  retVal = create?
    RegCreateKeyEx(baseKey, src, 0, 0, REG_OPTION_NON_VOLATILE,
                   SAM_WRITE, 0, key, &i) :
    RegOpenKeyEx(baseKey, src, 0, sam, key);
#endif

  RegCloseKey(baseKey);

exit:

  if (closeSourceKey);
    RegCloseKey(sourceKey);

  return retVal;
}
#endif

#ifdef __cplusplus
}
#endif
