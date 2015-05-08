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


#include <stdlib.h>

#include <rvinternal.h>
#include <cm.h>
#include <cmutils.h>
#include <cmictrl.h>
#include <netutl.h>
#include <q931asn1.h>
#include <transport.h>
#include <cmdebprn.h>
#include <cmCall.h>

void getGoodAddressForCall(HCALL hCall, cmTransportAddress* ta)
{
    callElem* call=(callElem*)hCall;

    /* if we already have a full address, we assume it's good, and just return */
    if ((ta->type == cmTransportTypeIP) && (ta->ip != 0) && (ta->port != 0))
        return;

    if (call)
    {
        UINT16 port = ta->port;
        cmTransGetGoodAddressForH245(call->hsTransSession,ta);
        if(port)
            ta->port = port;
    }
}

void getGoodAddressForCtrl(HCONTROL ctrl, cmTransportAddress* ta)
{
    getGoodAddressForCall(cmiGetByControl(ctrl), ta);
}

RVAPI int RVCALLCONV
cmCallCreateControlSession(
          IN     HCALL           hsCall,
          IN OUT cmTransportAddress* addr)
{
    int ret=0;
    callElem* call=(callElem*)hsCall;
    HAPP hApp=(HAPP)emaGetInstance((EMAElement)call);
    if (!hsCall || !hApp) return RVERROR;

    cmiAPIEnter(hApp,"cmCallCreateControlSession hsCall=0x%x",hsCall);
    if (emaLock((EMAElement)call))
    {
		if (addr)
	        ret=cmTransCreateControlSession(call->hsTransSession, addr, (!addr->ip || !addr->port), FALSE);
		else
		{
			cmTransportAddress dummyAddr;
	        ret=cmTransCreateControlSession(call->hsTransSession, &dummyAddr, FALSE, TRUE);
		}
        emaUnlock((EMAElement)call);
    }
    cmiAPIExit(hApp,"cmCallCreateControlSession [%d]",ret);
    return ret;
}


RVAPI int RVCALLCONV
cmCallCloseControlSession(
              /* Close the H.245 session for the call */
              IN     HCALL               hsCall)
{
    callElem*call=(callElem*)hsCall;
    HAPP hApp=(HAPP)emaGetInstance((EMAElement)call);
    if (!hsCall || !hApp) return RVERROR;

    cmiAPIEnter(hApp,"cmCallCloseControlSession: hsCall=0x%lx",hsCall);
    if (emaLock((EMAElement)call))
    {
        if (cmTransHasControlSession(call->hsTransSession))
        {
            if (m_callget(call,control))
            {
                stopControl(cmiGetControl(hsCall));
                m_callset(call,control,FALSE);
            }
            closeChannels(cmiGetControl(hsCall));
        }
        cmTransCloseControlSession(call->hsTransSession);
        emaUnlock((EMAElement)call);
    }
    cmiAPIExit(hApp,"cmCallCloseControlSession: [1]");
    return TRUE;
}


RVAPI int RVCALLCONV
cmCallHasControlSession(
              /* Does call has the H.245 protocol? */
              IN     HCALL               hsCall)
{
    callElem*call=(callElem*)hsCall;
    HAPP hApp=(HAPP)emaGetInstance((EMAElement)call);
    int has=0;
    if (!hsCall || !hApp) return RVERROR;

    cmiAPIEnter(hApp,"cmCallHasControlSession: hsCall=0x%lx",hsCall);
    if (emaLock((EMAElement)call))
    {
        has=cmTransHasControlSession(call->hsTransSession);
        emaUnlock((EMAElement)call);
    }
    cmiAPIExit(hApp,"cmCallHasControlSession: (hsCall=0x%lx)=%d",hsCall,has);
    return has;
}


RVAPI
int RVCALLCONV cmCallConnectControl(
                IN      HCALL               hsCall
                                  )
{
    callElem* call=(callElem*)hsCall;
    HAPP hApp=(HAPP)emaGetInstance((EMAElement)call);
    if (!hsCall || !hApp) return RVERROR;

    cmiAPIEnter(hApp,(char*)"cmCallConnectControl(hsCall=0x%lx)",hsCall);
    if (emaLock((EMAElement)call))
    {
        if ((call->state == cmCallStateInit) && (m_callget(call,control)) &&
            (controlGetState(cmiGetControl(hsCall)) == ctrlInitialized))
        {
            /* dummy call already has control - fix for MC: tunneling rejected, reset the control */
            initControl(cmiGetControl(hsCall), call->lcnOut);
        }
        cmTransEstablishControl (call->hsTransSession);
        emaUnlock((EMAElement)call);
    }
    cmiAPIExit(hApp,(char*)"cmCallConnectControl=0");
    return 0;
}

RVAPI int RVCALLCONV
cmCallSw2SeparateH245(IN      HCALL               hsCall)
{
    callElem*call=(callElem*)hsCall;
    HAPP hApp=(HAPP)emaGetInstance((EMAElement)call);
    if (!hsCall || !hApp) return RVERROR;

    cmiAPIEnter(hApp,(char*)"cmCallSw2SeparateH245(hsCall=0x%lx)",hsCall);
    if (emaLock((EMAElement)call))
    {
        cmTransSwitchToSeparate (call->hsTransSession);
        emaUnlock((EMAElement)call);
    }
    cmiAPIExit(hApp,(char*)"cmCallSw2SeparateH245 [OK]");
    return TRUE;
}

RVAPI
int RVCALLCONV cmCallSetControlRemoteAddress(
                IN      HCALL               hsCall,
                IN      cmTransportAddress  *addr
                                  )
{
    callElem* call=(callElem*)hsCall;
    HAPP hApp=(HAPP)emaGetInstance((EMAElement)call);
    TRANSERR transErr = cmTransErr;
    int res;

    if (!hsCall || !hApp) return RVERROR;

    cmiAPIEnter(hApp,(char*)"cmCallSetControlRemoteAddress(hsCall=0x%lx)",hsCall);
    if (emaLock((EMAElement)call))
    {
        HSTRANSHOST host = NULL;

        cmTransGetSessionParam(call->hsTransSession, 
                               cmTransParam_H245Connection, 
                               (void *)&host);
        if (host)
            transErr = cmTransSetHostParam(host, 
                                           cmTransHostParam_remoteAddress, 
                                           (void *)addr,
                                           FALSE);
        else
            transErr = cmTransErr;

        emaUnlock((EMAElement)call);
    }

    if (transErr != cmTransOK)
        res = RVERROR;
    else
        res = 0;

    cmiAPIExit(hApp,(char*)"cmCallSetControlRemoteAddress=%d", res);
    return res;
}

#ifdef __cplusplus
}
#endif
