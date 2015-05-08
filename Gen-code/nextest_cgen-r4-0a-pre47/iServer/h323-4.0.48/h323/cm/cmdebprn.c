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

#include <rvinternal.h>
#include <ms.h>
#include <cmintr.h>

void cmiAPIEnter(HAPP hApp,const char*line,...)
{
#ifndef NOLOGSUPPORT
    cmElem* app = (cmElem *)hApp;

    if (logIsSelected(app->logAPI, RV_DEBUG))
    {
        char buf[1024];
        va_list v;
        va_start(v, line);
        vsprintf(buf, line, v);

        cmLock(hApp);
        app->level++;
        logPrintFormat(app->logAPI, RV_DEBUG,
                       "%d >>>%*c%s", app->level, app->level * 3, ' ', buf);
        cmUnlock(hApp);
    }
#else
	if (hApp || line);
#endif
}
void cmiAPIExit(HAPP hApp,const char*line,...)
{
#ifndef NOLOGSUPPORT
	cmElem* app = (cmElem *)hApp;

    if (logIsSelected(app->logAPI, RV_DEBUG))
    {
        char buf[1024];
        va_list v;
        va_start(v, line);
        vsprintf(buf, line, v);

        cmLock(hApp);
        logPrintFormat(app->logAPI, RV_DEBUG,
                       "%d <<<%*c%s", app->level, app->level * 3, ' ', buf);
        app->level--;
        cmUnlock(hApp);
    }
#else
	if (hApp || line);
#endif
}
void cmiCBEnter(HAPP hApp,const char*line,...)
{
#ifndef NOLOGSUPPORT
	cmElem* app = (cmElem *)hApp;

    if (logIsSelected(app->logCB, RV_DEBUG))
    {
        char buf[1024];
        va_list v;
        va_start(v, line);
        vsprintf(buf, line, v);

        cmLock(hApp);
        app->level++;
        logPrintFormat(app->logCB, RV_DEBUG,
                       "%d >>>%*c%s", app->level, app->level * 3, ' ', buf);
        cmUnlock(hApp);
    }
#else
	if (hApp || line);
#endif
}
void cmiCBExit(HAPP hApp,const char*line,...)
{
#ifndef NOLOGSUPPORT
	cmElem* app = (cmElem *)hApp;

    if (logIsSelected(app->logCB, RV_DEBUG))
    {
        char buf[1024];
        va_list v;
        va_start(v, line);
        vsprintf(buf, line, v);

        cmLock(hApp);
        logPrintFormat(app->logCB, RV_DEBUG,
                       "%d <<<%*c%s", app->level, app->level * 3, ' ', buf);
        app->level--;
        cmUnlock(hApp);
    }
#else
	if (hApp || line);
#endif
}

#ifdef __cplusplus
}
#endif

