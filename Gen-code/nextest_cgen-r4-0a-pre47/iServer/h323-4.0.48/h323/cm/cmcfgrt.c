#ifdef __cplusplus
extern "C" {
#endif



/*

NOTICE:
This document contains information that is proprietary to RADVISION LTD.
No part of this publication may be reproduced in any form whatsoever without
written prior approval by RADVISION LTD.

RADVISION LTD. reserves the right to revise this publication and make changes
without obligation to notify any person of such revisions or changes.

*/


/****************************************************************************

  cmcfgrt.c  --  Configuration Roots

  This Comment:   25-May-1997
  Module Author:  Oz Solomonovich

  Abstract:       Interface to the configuration root names.

  Platforms:      All.

  Known Bugs:     None.

****************************************************************************/


#include <rvinternal.h>
#include <cm.h>
#include <cmcfgrt.h>
#include <cmdebprn.h>


static
const char *asnList[] =
{
    "SystemConfiguration",
    "RASConfiguration",
    "Q931Configuration",
    "H245Configuration",

    "H450SystemConfiguration",
    "CTConfig",
    "CDConfig",
    "CHConfig",
    "CPConfig",
    "CWConfig",
    "MWIConfig",
    "CCConfig",
    "COConfig",
    "CIConfig"
};

static
BYTE*
    (RVCALLCONV*syntaxList[])(void) =
{
    cmEmGetCommonSyntax,
    cmEmGetQ931Syntax,
    cmEmGetQ931Syntax,
    cmEmGetH245Syntax,

    cmEmGetCommonSyntax,
    cmEmGetCommonSyntax,
    cmEmGetCommonSyntax,
    cmEmGetCommonSyntax,
    cmEmGetCommonSyntax,
    cmEmGetCommonSyntax,
    cmEmGetCommonSyntax,
    cmEmGetCommonSyntax,
    cmEmGetCommonSyntax,
    cmEmGetCommonSyntax
};

static
const char *cfgList[] =
{
    "system",
    "RAS",
    "Q931",
    "h245",
    "h450system",
    "h4502",
    "h4503",
    "h4504",
    "h4505",
    "h4506",
    "h4507",
    "h4509",
    "h45010",
    "h45011"
};


RVAPI void RVCALLCONV
cmGetCfgRootNameList(
                    IN   HAPP               hApp,
                    OUT  int *              count,
                    OUT  const char ***     list)
{
  if (!hApp) return;
  cmiAPIEnter(hApp, "cmGetCfgRootNameList: hApp=0x%lx",hApp);

    if (count)
        *count = sizeof(cfgList)/sizeof(char *);

    if (list)
        *list = cfgList;
  cmiAPIExit(hApp, "cmGetCfgRootNameList: hApp=0x%lx count=%d",hApp,*count);

}

RVAPI const unsigned char * RVCALLCONV
cmGetSyntaxFromCfgName(
                    IN   HAPP               hApp,
                    IN   const char *       rootName)
{
    int i;

    if (!hApp) return NULL;
    cmiAPIEnter(hApp, "cmGetSyntaxFromCfgName: hApp=0x%lx,rootName %s",hApp,nprn(rootName));

    for (i = 0; i < (int)(sizeof(asnList)/sizeof(char *)); i++)
    {
        if (!strcmp(cfgList[i], rootName))
        {
            cmiAPIExit(hApp, "cmGetSyntaxFromCfgName: hApp=0x%lx",hApp);
            return syntaxList[i]();
        }
    }
    cmiAPIExit(hApp, "cmGetSyntaxFromCfgName: hApp=0x%lx rootName null",hApp);
    return NULL;
}

RVAPI const char * RVCALLCONV
cmGetAsnNameFromCfgName(
                    IN   HAPP               hApp,
                    IN   const char *       rootName)
{
    int i;

    if (!hApp) return NULL;
    cmiAPIEnter(hApp, "cmGetAsnNameFromCfgName: hApp=0x%lx,rootName %s",hApp,nprn(rootName));

    for (i = 0; i < (int)(sizeof(asnList)/sizeof(char *)); i++)
    {
        if (!strcmp(cfgList[i], rootName))
        {
            cmiAPIExit(hApp, "cmGetAsnNameFromCfgName: hApp=0x%lx asnName %s",hApp,nprn(asnList[i]));
            return asnList[i];
        }
    }
    cmiAPIExit(hApp, "cmGetAsnNameFromCfgName: hApp=0x%lx rootName null",hApp);
    return NULL;
}


RVAPI const char * RVCALLCONV
cmGetCfgNameFromAsnName(
                    IN   HAPP               hApp,
                    IN   const char *       asnName)
{
    int i;

    if (!hApp) return NULL;
    cmiAPIEnter(hApp, "cmGetCfgNameFromAsnName: hApp=0x%lx asnName %s",hApp,nprn(asnName));

    for (i = 0; i < (int)(sizeof(asnList)/sizeof(char *)); i++)
    {
        if (!strcmp(asnList[i], asnName))
        {
            cmiAPIExit(hApp, "cmGetCfgNameFromAsnName: hApp=0x%lx cfgName %s",hApp,nprn(cfgList[i]));
            return cfgList[i];
        }
    }
    cmiAPIExit(hApp, "cmGetCfgNameFromAsnName: hApp=0x%lx cfgName null",hApp);
    return NULL;
}
#ifdef __cplusplus
}
#endif



