/***********************************************************************************************
  Notice:
  This document contains information that is proprietary to RADVISION LTD..
  No part of this publication may be reproduced in any form whatsoever without
  written prior approval by RADVISION LTD..

    RADVISION LTD. reserves the right to revise this publication and make changes
    without obligation to notify any person of such revisions or changes.

*************************************************************************************************/
 
/********************************************************************************************
*                                TAP_security.c
*
* This file contains all the functions which enable the use of security modes
*
*
*      Written by                        Version & Date                        Change
*     ------------                       ---------------                      --------
*                                         10-Jan-2001
*
********************************************************************************************/
/*****************************************************************************************/

/*****************************************************************************************/


#ifdef __cplusplus
extern "C" {
#endif

#include "TAP_security.h"

#ifdef USE_SECURITY

#ifdef USE_SEC_HASH
#include "sec_hash.h"
#endif

#include "h235_api.h"

#endif  /* USE_SECURITY */



static BOOL secInitialized = FALSE;


/********************************************************************************************
 * How to get the security params
 ********************************************************************************************
 * In order to get string variables from the TCL, all you have to do, is call the following
 * Command:
 *
 * TclGetVariable("<varname>");
 *
 * this command will return to you a string (no need to allocate nor free it) which is the
 * contents of the variable. variable may be:
 *
 * app(options,secMode)   - will return the security mode ("None" or "Procedure1")
 * app(options,senderID)  - will return the sender ID (e.g. "MyName")
 * app(options,generalID) - will return the general ID (e.g. "RVGK")
 * app(options,password)  - will return the password (e.g. "Ken Sent Me")
 ********************************************************************************************/

#ifdef USE_SECURITY
h235AppFailReply_t RVCALLCONV h235Failure(IN   HAPP hApp,
                IN  H235HAPP    h235HApp,
                IN  H235HMSG    h235HMsg,
                IN  int         messageRoot,
                IN  entityId_t  *senderId,
                IN h235Reason_t reason);

h235AppSuccessReply_t RVCALLCONV h235Success(IN    HAPP hApp,
                IN  H235HAPP    h235HApp,
                IN  H235HMSG    h235HMsg,
                IN  int         messageRoot,
                IN  entityId_t  *senderId);


int RVCALLCONV hash_SH1_96Callback(IN H235HKEY     keyHandle ,
                        IN OUT void *buf,
                        IN int bufLen,
                        OUT unsigned char* digest,
                        OUT int *len);

static int stringToBMP(IN const char* str,OUT char* bmpStr);
H235HAPP h235HApp;
HAPP saved_lphApp;

#endif  /* USE_SECURITY */


int SEC_set(HAPP lphApp)
{
    int retVal = 0;
#ifdef USE_SECURITY

    static  BOOL secInstanceExist = FALSE;
    static char key[20];
    char   *tclVar = 0;
    h235Mode_t secMode = h235ModeNone;
    BOOL isCheckComing = TRUE;
    char text[64];


    if (lphApp)
        saved_lphApp = lphApp;

    /********************************************/
    /* secure mode                               */
    /********************************************/
    tclVar = TclGetVariable("app(options,secMode)");
    if (strcmp(tclVar,"None") == 0) {
        secMode = h235ModeNone;
        sprintf(text,"SEC: Setting secure mode - None");
        TclExecute("test:Log {%s}", text);
    }
    else if (strcmp(tclVar,"Procedure1") == 0) {
        secMode = h235ModeProcedure1;
        sprintf(text,"SEC: Setting secure mode - Procedure1");
        TclExecute("test:Log {%s}", text);
    }

    /********************************************/
    /* check incoming                           */
    /********************************************/
    tclVar = TclGetVariable("app(options,checkIncoming)");
    if (strcmp(tclVar,"0") == 0)
        isCheckComing = FALSE;
    else if (strcmp(tclVar,"1") == 0)
        isCheckComing = TRUE;
    /********************************************/
    /* New H235 instance                        */
    /********************************************/
    if ((secMode != h235ModeNone || isCheckComing ) &&
                            secInstanceExist == FALSE) {
        h235EventHandlers_T eh;
        if (h235Init(saved_lphApp,&h235HApp) >= 0)
            secInitialized = TRUE;
        else
            secInitialized = FALSE;

        memset(&eh,0,sizeof(h235EventHandlers_T));
        eh.evFailure                        = h235Failure;
        eh.evSuccess                        = h235Success;
        h235SetEventHandlers(h235HApp,&eh);
        h235SetEncodingCallback(h235HApp,hash_SH1_96Callback,OID_U_STRING);
		h235SetTimeSync(h235HApp,TRUE,86500); /*allow max 24 hours diff */


        secInstanceExist = TRUE;
    }

    /********************************************/
    /* delete H235 instance                     */
    /********************************************/
    if ((secMode == h235ModeNone && isCheckComing == FALSE) &&
                            secInstanceExist == TRUE) {
        h235End(h235HApp);
        secInstanceExist = FALSE;
    }

    /***********************************/
    /* key                             */
    /***********************************/
    memset(key,0,20);
    strcpy(key,TclGetVariable("app(options,password)"));


    if (h235HApp) {
        entityId_t    gkId;
        entityId_t    senderId;

        h235SetSecureMode(h235HApp,secMode);
        h235SetCheckIncomingMessages(h235HApp,isCheckComing);


        /***********************************/
        /* generalID                       */
        /***********************************/
        tclVar = TclGetVariable("app(options,generalID)");
        gkId.length = stringToBMP(tclVar,(char*)gkId.data);
        h235SetGatekeeperId(h235HApp,&gkId,(H235HKEY) key);

        /***********************************/
        /* senderId                        */
        /***********************************/
        tclVar = TclGetVariable("app(options,senderID)");
        senderId.length = stringToBMP(tclVar,(char*)senderId.data);
        h235SetSenderId(h235HApp,&senderId);
    }
#endif /* USE_SECURITY */
    return retVal;
}




/********************************************************************************************
 * SEC_Init
 * purpose : This function initializes the security sepplementay services. does nothing if
 *           USE_SECURITY is not defined.
 * input   : lphApp - pointer to the application handle
 * output  : none
 * return  : negative on error
 ********************************************************************************************/
int SEC_init(HAPP lphApp)
{
    int retVal = 0;
#ifdef USE_SECURITY
    SEC_set(lphApp);

#endif /* USE_SECURITY */
    return retVal;
}


/********************************************************************************************
 * SEC_IsInitialized
 * purpose : Indicates if SECURITY package is working
 * input   : none
 * output  : none
 * return  : TRUE if SECURITY package can be used
 ********************************************************************************************/
BOOL SEC_IsInitialized(void)
{
    return secInitialized;
}


/********************************************************************************************
 * SEC_notifyModeChange
 * purpose : This procedure will be called when the "Set" button is presed.
 * syntax  : SEC.notifyModeChange
 * input   : none
 * output  : none
 * return  : TCL_OK on ok, TCL_ERROR when not ok.
 ********************************************************************************************/
int SEC_notifyModeChange(ClientData clientData, Tcl_Interp *interp,int argc, char *argv[])
{
    int retVal = TCL_OK;
#ifdef USE_SECURITY

    SEC_set(0);

#endif /* USE_SECURITY */
    return retVal;
}



/*----------------------------------------------------------------------*/
#ifdef USE_SECURITY
h235AppFailReply_t RVCALLCONV  h235Failure(IN   HAPP hApp,
                IN  H235HAPP    h235HApp,
                IN  H235HMSG    h235HMsg,
                IN  int         messageRoot,
                IN  entityId_t  *senderId,
                IN h235Reason_t reason)
{
    h235AppFailReply_t reply;
    char msgName[164];
    sprintf(msgName,"%s",cmGetProtocolMessageName(saved_lphApp,messageRoot));

    switch (reason) {
        case h235ReasonNoSecurityInfo:  reply = h235FailReplyForceOk;               break;
        case h235ReasonMessageError:    reply = h235FailReplyDeleteMessage;         break;
        case h235ReasonGeneralIDIncorrect:  reply = h235FailReplyForceOk;           break;
        case h235ReasonUnknownSender:   reply = h235FailReplyContinue;         		break;
        case h235ReasonProcNotSupported:reply = h235FailReplyContinue;              break;
        case h235ReasonTimeStampMissing:reply = h235FailReplyContinue;              break;
        case h235ReasonTimeStampFail:   reply = h235FailReplyContinue;              break;
        case h235ReasonTimeRandomFail:  reply = h235FailReplyContinue;              break;
        case h235ReasonIncorrectAuth:   reply = h235FailReplyDeleteMessage;         break;
        case h235ReasonGeneralError:    reply = h235FailReplyDeleteMessage;         break;
        default: reply = h235FailReplyContinue;
    }

    /*****************************************
     * The following reject messages might be
     * because of security denial, therefore
     * we will not fail them on security
     *****************************************/
    if (strcmp(msgName,"registrationReject") == 0   ||
        strcmp(msgName,"admissionReject") == 0      ||
        strcmp(msgName,"disengageReject") == 0 )
        reply = h235FailReplyForceOk;

    /*************************************/
    /* log messages                      */
    /*************************************/
    {
        char text[1128],smalltext[24];
        char reason_text[32];
        sprintf(reason_text,"%s",h235ReasonName(reason));

        {
                sprintf(text,"SEC: incoming message \'%s\' failed security - reason %s",msgName,reason_text);
                /*GK_NOTIFY(text,GEN_NA,20);*/
                TclExecute("test:Log {%s}", text);
                switch (reply) {
                    case h235FailReplyContinue:     sprintf(smalltext,"continue"); break;
                    case h235FailReplyDeleteMessage:sprintf(smalltext,"delete incoming message"); break;
                    case h235FailReplyForceOk:      sprintf(smalltext,"ignore security failure"); break;
                    default: sprintf(smalltext,"bug"); /* this option is impossible */
                }
                TclExecute("test:Log {SEC: application reply: %s}", smalltext);
                /*sprintf(text,"SEC: application reply: %s", smalltext);
                GK_NOTIFY(text,GEN_NA,20);*/
        }
    }

    return reply;
}

h235AppSuccessReply_t RVCALLCONV h235Success(IN    HAPP hApp,
                IN  H235HAPP    h235HApp,
                IN  H235HMSG    h235HMsg,
                IN  int         messageRoot,
                IN  entityId_t  *senderId)
{
    h235AppSuccessReply_t reply;

    reply = h235SuccessReplyForceFail;
    reply = h235SuccessReplyDeleteMessage;
    reply = h235SuccessReplyContinue;
    {
        char msgName[164],text[1128];
        sprintf(msgName,"%s",cmGetProtocolMessageName(saved_lphApp,messageRoot));
        sprintf(text,"SEC: incoming message %s passed security",msgName);
        TclExecute("test:Log {%s}", text);
        /*GK_NOTIFY(text,GEN_NA,20);*/
    }
    return reply;
}
/*-----------------------------------------------------------------------*/

#ifndef USE_SEC_HASH
void hmac( IN char* data,
           IN int dataLength,
           IN char *key,
           IN int keyLen,
           OUT unsigned char *digest,
           IN int digestLength) 
{
	/*********************************************************/
	/* here you should implement your hmac-sha1-96 algorithm */
	/* The current implementation is just a dummy one for    */
	/* presentations purpose and it returns the key value    */
	/* instead of the authenticator.                         */
	/*********************************************************/
	memset(digest,0,digestLength);
	memcpy(digest,key,(digestLength < keyLen)?digestLength:keyLen);
}

#endif

int RVCALLCONV hash_SH1_96Callback(IN H235HKEY keyHandle ,
                        IN OUT void *buf,
                        IN int bufLen,
                        OUT unsigned char* digest,
                        OUT int *len)

{
    char *key1;
    key1 = (char *) keyHandle;
/*  h235HashFunc1(key1,buf,bufLen,digest);*/
    hmac((char*)buf,bufLen,key1,20,digest,20);
    *len = 12;  /* 96 bits length in bytes */
    return 0;
}



/***********************************************************************************
 * Routine Name:  stringToBMP
 * Description :  converst a string to a BMP String
 * Input: str - a regula string
 * Output: bmpStr - the result BMP String
 * Return: The length of the BMP String.
 ***********************************************************************************/

static int stringToBMP(IN const char* str,OUT char* bmpStr)
{
    int len=strlen(str);
    int i;
    for (i=0;i<len;i++) {
        bmpStr[i*2]=0;
        bmpStr[i*2+1]=str[i];
    }
    return i*2;
}

#endif  /* USE_SECURITY */



#ifdef __cplusplus
}
#endif
