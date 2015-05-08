#include "gis.h"
#include <malloc.h>
#include "ua.h"

/* Transaction m/c send msg to Call SM */
int
SipTransRecvMsgHandle(SipAppMsgHandle *appMsgHandle)
{
	char fn[] = "SipTransRecvMsgHandle():";
	SipMsgHandle        *msgHandle = NULL;
	SipEventHandle		*evHandle = NULL;
	int event, rc = -1, smtype;

	NETDEBUG(MSIP, NETLOG_DEBUG4, ("%s Entering\n", fn));

	/* We cook up the event we need to feed into the CSM */
	if (appMsgHandle == NULL)
	{
		NETERROR(MSIP, ("%s appMsgHandle is NULL\n", fn));
		return -1;
	}

	msgHandle = appMsgHandle->msgHandle;

	if (msgHandle == NULL)
	{
		NETERROR(MSIP, ("%s msgHandle is NULL\n", fn));
		return -1;
	}

	event = SipGetEventFromMsgHandle(appMsgHandle, &smtype);

	if (event < 0)
	{
		// This is an event we dont handle
		NETDEBUG(MSIP, NETLOG_DEBUG4,
			("%s Dont need to handle this event\n", fn));

		return -1;
	}

	evHandle = SipAllocEventHandle();

	evHandle->type = Sip_eNetworkEvent;
	evHandle->event = event;
	evHandle->handle = appMsgHandle;

	if (SipValidateSipEventHandle(evHandle) < 0)
	{
		return -1;
	}

	switch (smtype)
	{
	case SIPEVENT_CALLSM:
		/* Fill up the application callid */
		SipFindAppCallID(msgHandle, appMsgHandle->callID, appMsgHandle->confID);

		rc = SipUAProcessEvent(evHandle);
		break;
	case SIPEVENT_REGSM:
		/* Fill up the application callid */
		SipRegFindAppCallID(msgHandle, appMsgHandle->callID);

		rc = SipRegProcessEvent(evHandle);
		break;
	default:
		NETERROR(MSIP, 
			("%s Cannot determine s/m type %d \n", fn, smtype));
		break;
	}

	if (rc < 0)
	{
		NETDEBUG(MSIP, NETLOG_DEBUG1,
			("%s SipUAProcessEvent returned error\n", fn));
	}

	return rc;
}

int
SipGetEventFromMsgHandle(SipAppMsgHandle *appMsgHandle, int *smtype)
{
	int event = -1;
	SipMsgHandle *msgHandle = NULL;

	msgHandle = appMsgHandle->msgHandle;

	*smtype = SIPEVENT_CALLSM;

	if (!strcmp(msgHandle->method, "INVITE"))
	{
		/* We will handle errors only for INVITE */
		if(appMsgHandle->tsmError == tsmError_NO_RESPONSE)
		{
			event = Sip_eNetworkNoResponseError;
		}
		else if (appMsgHandle->tsmError != 0)
		{
			event = Sip_eNetworkError;
		}
		else if (msgHandle->msgType == SipMessageRequest)
		{
			event = Sip_eNetworkInvite;	
		}
		else if (msgHandle->responseCode == 200)
		{
			event = Sip_eNetwork200;
		}
		else if (msgHandle->responseCode < 200)
		{
			event = Sip_eNetwork1xx;
		}
		else if (msgHandle->responseCode < 400)
		{
			event = Sip_eNetwork3xx;
		}
		else
		{
			event = Sip_eNetworkFinalResponse;
		}
	}
	else if (!strcmp(msgHandle->method, "REGISTER"))
	{
		*smtype = SIPEVENT_REGSM;

		if (appMsgHandle->tsmError != 0)
		{
			event = SipReg_eNetworkRegisterFinalResponse;
		}
		else if (msgHandle->msgType == SipMessageRequest)
		{
			// event = SipReg_eNetworkRegister;
		}
		else if (msgHandle->responseCode == 200)
		{
			event = SipReg_eNetworkRegister200;
		}
		else if (msgHandle->responseCode < 400)
		{
			event = SipReg_eNetworkRegister3xx;
		}
		else if (msgHandle->responseCode >= 400)
		{
			event = SipReg_eNetworkRegisterFinalResponse;
		}
	}
	else if (!strcmp(msgHandle->method, "INFO"))
	{
		if (appMsgHandle->tsmError != 0)
		{
			event = Sip_eNetworkError;
		}
		else if (msgHandle->msgType == SipMessageRequest)
		{
			event = Sip_eNetworkInfo;	
		}
		else if (msgHandle->responseCode >= 200)
		{
			event = Sip_eNetworkInfoFinalResponse;
		}
		else if (msgHandle->responseCode < 200)
		{
			event = Sip_eNetwork1xx;
		}
		else
		{
			event = Sip_eNetworkError;
		}

	}
	else if(!strcmp(msgHandle->method,"REFER") &&
		msgHandle->msgType == SipMessageResponse )
	{
		if(msgHandle->responseCode < 200 )
		{
			event = Sip_eNetwork1xx;
		}
		else if(msgHandle->responseCode == 202 )
		{
			event = Sip_eNetwork202;
		}
		else 
		{
			event = Sip_eNetworkError;
		}
	}
	else if ((appMsgHandle->tsmError == 0) &&
			(msgHandle->msgType == SipMessageRequest))
	{
		if (!strcmp(msgHandle->method, "ACK"))
		{
			event = Sip_eNetworkAck;
		}
		else if (!strcmp(msgHandle->method, "BYE"))
		{
			event = Sip_eNetworkBye;
		}
		else if (!strcmp(msgHandle->method, "CANCEL"))
		{
			event = Sip_eNetworkCancel;
		}
		else if(!strcmp(msgHandle->method, "REFER"))
		{
			event = Sip_eNetworkRefer;
		}
		else if(!strcmp(msgHandle->method, "NOTIFY"))
		{
			event = Sip_eNetworkNotify;
		}
	}

	return event;
}

