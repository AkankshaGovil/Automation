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

#include <cmConfig.h>



/************************************************************************
 *
 *                          Public functions
 *
 ************************************************************************/


/************************************************************************
 * cmiCreateLog
 * purpose: Create a log manager for use by a stack instance
 *          This function reads the information from a configuration
 *          manager and initializes the log manager with it.
 * input  : app     - Stack's application handle
 * output : none
 * return : Log manager handle on success
 *          NULL on failure
 ************************************************************************/
RVHLOGMGR RVCALLCONV cmiCreateLog(IN cmElem* app)
{
    /* Initialize a log manager */
    app->logMgr = logInitialize();

    if (app->logMgr != NULL)
    {
        /* Create sources for the CM's log */
        if (meiGlobalInit());
        app->log = logRegister(app->logMgr, "CM", "Conference Management");
        app->logAPI = logRegister(app->logMgr, "CMAPI", "Conference Management API");
        app->logCB = logRegister(app->logMgr, "CMAPICB", "Conference Management CallBack");
        app->logERR = logRegister(app->logMgr, "CMERR", "Conference Management Errors");
        app->logTPKT = logRegister(app->logMgr, "TPKTCHAN", "TPKT Messages");
        app->logConfig = logRegister(app->logMgr, "CONFIG", "Configuragtion settings");
        app->logAppl = logRegister(app->logMgr, "APPL", "User instigated messages");
    }

    return app->logMgr;
}


/************************************************************************
 * pvtLoadFromConfig
 * purpose: Convert information from a configuration instance (HCFG) to
 *          a value tree of a given syntax.
 * input  : hCfg        - Configuration instance handle
 *          hVal        - Value tree handle
 *          name        - Name of root from configuration to convert
 *          rootNodeId  - Syntax of configuration
 *          log         - Log handle to use for debug messages
 * output : none
 * return : Root ID of configuration created on success
 *          Negative value on failure
 ************************************************************************/
int pvtLoadFromConfig(HCFG hCfg, HPVT hVal, char* name, int rootNodeId, RVHLOG log)
{
    static char tempBuff[1024];
    int tempLen;

    int length = strlen(name);
    char next[256], svalue[256], *str;
    INT32 value;
    ci_str_type strtype;
    if (log);

    strncpy(next, name, sizeof(next));

    for (;;)
    {
        int nodeId;

        if (ciNext(hCfg, next, next, sizeof(next)) == ERR_CI_NOTFOUND)
            break;

        if (strncmp(name, next, length))
            break;

        ciGetValueExt(hCfg, next, &strtype, &value);

        tempLen=sprintf(tempBuff,"%s= (%d)", nprn(next), value);

        if (strtype != ci_str_not)
        {
            switch (strtype)
            {
                case ci_str_regular:
                {
                    ciGetString(hCfg, next, svalue, sizeof(svalue));
                    tempLen+=sprintf(tempBuff+tempLen, " '%*.*s'", value, value, nprn(svalue));
                    break;
                }

                case ci_str_bit:
                {
                    ciGetBitString(hCfg, next, svalue, sizeof(svalue),
                        (UINT32 *)&value);
                    tempLen+=sprintf(tempBuff+tempLen, " bitstring, bits=%d", value);
                    break;
                }
            default:
                break;
            }
            str = (char *)svalue;
        }
        else
        {
            str = NULL;
        }

        if ((nodeId = pvtBuildByPath(hVal, rootNodeId, strchr(next, '.') , value, str)) < 0)
        {
            logPrint(log, RV_ERROR,
                (log, RV_ERROR, "Cannot create: %s", tempBuff));
        }

        logPrint(log, RV_DEBUG,
             (log, RV_DEBUG, "%s", tempBuff));
    }
    /*vtCheckTree(hVal, rootNodeId, msa);*/
    return 0;
}


/************************************************************************
 * cmGetLogMgr
 * purpose: Get the log manager instance used
 * input  : hApp     - Stack instance handle
 * output : none
 * return : Log manager used on success
 *          NULL on failure
 ************************************************************************/
RVAPI RVHLOGMGR RVCALLCONV cmGetLogMgr(IN  HAPP        hApp)
{
    cmElem* app=(cmElem*)hApp;

    if (app)
        return (app->logMgr);
    return 0;
}




#ifdef __cplusplus
}
#endif



