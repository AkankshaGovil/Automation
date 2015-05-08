/*

NOTICE:
This document contains information that is proprietary to RADVISION LTD..
No part of this publication may be reproduced in any form whatsoever without
written prior approval by RADVISION LTD..

RADVISION LTD. reserves the right to revise this publication and make changes
without obligation to notify any person of such revisions or changes.

*/


#ifndef _LOG_H
#define _LOG_H

#ifdef __cplusplus
extern "C" {
#endif


#include <rvinternal.h>
#include <stdarg.h>

#include <ms.h>


/************************************************************************
 *                  Debugging MACRO definitions
 * Make sure all debug definitions are processed on debug modes
 ************************************************************************/
#ifdef RV_DEBUG_ALL
#define RV_RA_DEBUG
#define RV_EMA_DEBUG
#define RV_RAS_DEBUG
#define RV_CODER_DEBUG
#endif




/************************************************************************
 * Log manager handle declaration
 * This handle is used to create and control HLOG handles. There should
 * be a single log manager per stack instance.
 ************************************************************************/
DECLARE_OPAQUE(RVHLOGMGR);


/************************************************************************
 * Log handle declaration
 * This handle is used to send messages to the log.
 * There are several such handles inside a single log manager. Their
 * information can be filtered in different forms.
 ************************************************************************/
DECLARE_OPAQUE(RVHLOG);




/************************************************************************
 * Severity of a LOG message
 * RV_DEBUG - Detailed deubg information
 * RV_INFO  - Debug information
 * RV_WARN  - Warning about a possible problem
 * RV_ERROR - Cannot continue with current operation
 * RV_EXCEP - Possible bug in the code
 ************************************************************************/
typedef enum {
    RV_DEBUG = 0,
    RV_INFO,
    RV_WARN,
    RV_ERROR,
    RV_EXCEP
} RV_LOG_MessageType;

#ifndef NOLOGSUPPORT





/************************************************************************
 * logInitialize
 * purpose: Create a log manager for use by a stack instance
 * input  : logMgr      - Log manager to destroy
 * output : none
 * return : none
 ************************************************************************/
RVAPI RVHLOGMGR RVCALLCONV logInitialize(void);


/************************************************************************
 * logEnd
 * purpose: Destroys a log manager
 * input  : none
 * output : none
 * return : none
 ************************************************************************/
RVAPI void RVCALLCONV logEnd(
    IN RVHLOGMGR    logMgr);


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
    IN const char*  description);


/************************************************************************
 * logUnregister
 * purpose: Unregister a log source
 * input  : hLog        - Handle to the log to unregister
 * output : none
 * return : none
 ************************************************************************/
RVAPI void RVCALLCONV logUnregister(IN RVHLOG   hLog);


/************************************************************************
 * logGetSource
 * purpose: Return the log source by its name
 * input  : name    - Name of source to get
 * output : none
 * return : Source handle for the given name on success
 *          NULL on failure
 ************************************************************************/
RVHLOG logGetSource(IN const char* name);


/************************************************************************
 * logGetName
 * purpose: Return the name of the log source
 * input  : hLog        - Handle to the log to use
 * output : none
 * return : Pointer to the name of the logger
 ************************************************************************/
const char* logGetName(IN RVHLOG hLog);


/************************************************************************
 * logGetDebugLevel
 * purpose: Check the debug level of messages in the stack
 * input  : hLog        - Handle to the log to use
 * output : none
 * return : Debug level of messages (0-4)
 ************************************************************************/
#if 0
int logGetDebugLevel(IN RVHLOG  hLog);
#else
#define logGetDebugLevel(hLog) \
    msGetDebugLevel()
#endif



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
        IN RV_LOG_MessageType    mType);



/************************************************************************
 * logPrint
 * purpose: Print a message to the log according to the configuration
 *          parameters.
 * input  : hLog        - Handle to the log to use for this module
 *          mType       - Type of message to print
 *          printParams - Printing parameters.
 *                        These parameters should be inside brackets.
 * output : none
 * return : none
 * example: logPrint((RVHLOG)hLog, RV_EXCEP,
 *                   ((RVHLOG)hLog, RV_EXCEP, "Example %d", i));
 ************************************************************************/
#define logPrint(hLog,mType,printParams) \
    if (logIsSelected(hLog, mType))             \
        logPrintFormat printParams;             \
    else;



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
                    IN const char*         printParams, ...);


#else  /* NOLOGSUPPORT */

/* Make sure all the functions are empty macros - we're not using a log */
#define logInitialize()                                 (RVHLOGMGR)1
#define logEnd(logMgr)                                  
#define logRegister(logMgr,name,desc)                   (RVHLOG)0
#define logUnregister(hLog)                             
#define logGetDebugLevel(hLog)                          0
#define logSetDebugLevel(hLog,level)
#define logGetName(hLog)                                ((char*)"")
#define logIsSelected(hLog,mType)                       0
#define logSetSelection(hLog,sink,mType,on)
#define logSetGlobalSelection(hLogMgr,sink,mType,on)
#define logPrint(hLog,mType,printParams)
#define logPrintFormat(hLog,mType,printParams)

#endif  /* NOLOGSUPPORT */

#ifdef __cplusplus
}
#endif

#endif  /* _LOG_H */



