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

#ifndef NOLOGSUPPORT

#include <rvinternal.h>
#include <time.h>
#include <ti.h>
#include <tls.h>
#include <log.h>
#include <msg.h>


typedef struct {
  int       inUse;
  char      name[11];
  char      description[51];
} msInfo_t;


#define MAX_INFO  90
static msInfo_t msInfo[MAX_INFO];
static int logActive = 0;

#define START 21


/* Message types */
static const char* LOG_mTypeStr[] =
    {"DEBUG", "INFO", "WARN", "ERROR", "EXCEP"};




/************************************************************************
 * logInitialize
 * purpose: Create a log manager for use by a stack instance
 * input  : none
 * output : none
 * return : Log manager handle on success
 *          NULL on failure
 ************************************************************************/
RVAPI RVHLOGMGR RVCALLCONV logInitialize(void)
{
    logActive++;

    if (logActive == 1)
    {
        memset(msInfo, 0, sizeof(msInfo));
        msInfo[0].inUse = TRUE;
        strcpy(msInfo[0].name, "UNREG");
        msInfo[1].inUse = TRUE;
        strcpy(msInfo[1].name, "MS");

        msOpen();
    }

    return (RVHLOGMGR)logActive;
}



/************************************************************************
 * logEnd
 * purpose: Destroys a log manager
 * input  : none
 * output : none
 * return : none
 ************************************************************************/
RVAPI void RVCALLCONV logEnd(
    IN RVHLOGMGR    logMgr)
{
    if (logMgr);

    logActive--;
    if (logActive == 0)
        msClose();
}


/************************************************************************
 * logRegister
 * purpose: Register a new type of log.
 *          This will create a new log handle, handling all the messages
 *          under the given 'name' variable.
 * input  : logMgr      - Log manager to use
 *          name        - Name of log to create
 *                        If the name already exists in this log manager
 *                        instance, its handle will be returned.
 *          description - Description of the log
 * output : none
 * return : Log handle on success
 *          NULL on failure
 ************************************************************************/
RVAPI RVHLOG RVCALLCONV logRegister(
    IN RVHLOGMGR    logMgr,
    IN const char*  name,
    IN const char*  description)
{
    int i;
    int firstFree=RVERROR;

    if (logMgr == NULL) return NULL;

    for (i = 0; i < MAX_INFO; i++)
    {
        if (msInfo[i].inUse && !strcmp(name,msInfo[i].name))
        {
            msInfo[i].inUse++;
            return (RVHLOG)(i-1);
        }
        if (!msInfo[i].inUse)
            firstFree=i;
    }

    if (firstFree>=0)
    {
        strncpy(msInfo[firstFree].name,name,sizeof(msInfo[firstFree].name)-1);
        strncpy(msInfo[firstFree].description,description,sizeof(msInfo[firstFree].description)-1);
        msInfo[firstFree].inUse=TRUE;

        logPrint((RVHLOG)(firstFree-1), RV_DEBUG,
                 ((RVHLOG)(firstFree-1), RV_DEBUG, "Registered %-11.11s %s",name,description));
        return (RVHLOG)(firstFree-1);
    }

    return NULL;
}


/************************************************************************
 * logUnregister
 * purpose: Unregister a log source
 * input  : hLog        - Handle to the log to unregister
 * output : none
 * return : none
 ************************************************************************/
RVAPI void RVCALLCONV logUnregister(IN RVHLOG   hLog)
{
   if (msInfo[(int)hLog+1].inUse>0)
    msInfo[(int)hLog+1].inUse--;
}


/************************************************************************
 * logGetSource
 * purpose: Return the log source by its name
 * input  : name    - Name of source to get
 * output : none
 * return : Source handle for the given name on success
 *          NULL on failure
 ************************************************************************/
RVHLOG logGetSource(IN const char* name)
{
    int i;
    for (i = MAX_INFO-1; i > 0; i--)
        if ((msInfo[i].inUse > 0) && !strcmp(name, msInfo[i].name))
            return (RVHLOG)(i-1);

    return (RVHLOG)NULL;
}


/************************************************************************
 * logGetName
 * purpose: Return the name of the log source
 * input  : hLog        - Handle to the log to use
 * output : none
 * return : Pointer to the name of the logger
 ************************************************************************/
const char* logGetName(IN RVHLOG hLog)
{
    return msInfo[(int)hLog+1].name;
}


/************************************************************************
 * logIsSelected
 * purpose: Check if the given type of message should be logged or not
 * input  : hLog        - Handle to the log to use
 *          mType       - Type of message to check
 * output : none
 * return : TRUE if message should be added to log
 *          FALSE otherwise
 ************************************************************************/
RVAPI BOOL RVCALLCONV logIsSelected(
        IN RVHLOG                hLog,
        IN RV_LOG_MessageType    mType)
{
    int level = msGetDebugLevel();


    if (level == 0) return FALSE; /* never print */
    if (level > 3) return TRUE; /* always print */

    if ((mType == RV_DEBUG) && (level == 1)) return FALSE; /* Don't print debug in level 1 */
    if (((mType == RV_ERROR) || (mType == RV_EXCEP)) && (level > 1)) return TRUE; /* Print errors when debug level higher than 1 */
    if ((mType == RV_WARN) && (level > 2)) return TRUE; /* Print warnings when debug level higher than 2 */

    return msIsSelected((int)hLog);
}


/************************************************************************
 * logPrintFormat
 * purpose: Print a message to the log without checking any configuration
 *          parameters. Should be used only if logIsSelected() function
 *          was called previously and returned TRUE.
 * input  : hLog        - Handle to the log to use for this module
 *          mType       - Type of message to print
 *          printParams - Printing parameters.
 *                        These parameters should be inside brackets.
 * output : none
 * return : none
 * example: logPrintFormat((RVHLOG)hLog, RV_EXCEP, "Example %d", i);
 ************************************************************************/
RVAPI void RVCALLCONV logPrintFormat(
                    IN RVHLOG              hLog,
                    IN RV_LOG_MessageType  mType,
                    IN const char*         printParams, ...)
{
    THREAD_LogLocalStorage* tls;
    va_list v;
    char* ptr;
    int length;
    time_t ti;
    struct tm t;

    if (hLog == NULL) return;

    /* Make sure we've got a buffer to work with */
    tls = THREAD_GetLogLocalStorage(NULL);
    if (tls == NULL) return ;
    ptr = tls->printedMessage;

    /* Get the current time */
    ti = timerGetTimeInSeconds();
#ifdef _MTS_
    t = *localtime_r(&ti, &t);
#else
    t = *localtime(&ti);
#endif

#ifndef NOTHREADS
    /* Insert thread's index */
    ptr += sprintf(ptr, "T%6d ", RvH323ThreadGetIndex());
#endif

    /* Format the time, source and message type */
    ptr += sprintf(ptr, "%2.2u:%2.2u:%2.2u %-10.10s: %-5.5s - ",
                   t.tm_hour, t.tm_min, t.tm_sec,
                   msInfo[(int)hLog+1].name, LOG_mTypeStr[mType]);

    /* Format the given line with the arguments */
    va_start(v, printParams);
    length = vsprintf(ptr, printParams, v);
    va_end(v);

    /* Remove ENDLINE character - we need to allow this for backward compatability
       with pvtPrint() that allows using the log to print partial lines */
    if (ptr[length-1] == '\n')
    {
        ptr[length-1] = '\0';
    }

    /* Do the printing itself */
    msPrint((int)hLog, tls->printedMessage);
}


#else  /* NOLOGSUPPORT */

int RV_dummyLOG = 0;

#endif  /* NOLOGSUPPORT */


#ifdef __cplusplus
}
#endif

