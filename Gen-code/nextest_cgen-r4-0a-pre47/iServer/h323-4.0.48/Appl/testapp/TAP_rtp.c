/************************************************************************************************************************

Notice:
This document contains information that is proprietary to RADVISION LTD..
No part of this publication may be reproduced in any form whatsoever without
written prior approval by RADVISION LTD..

RADVISION LTD. reserves the right to revise this publication and make changes
without obligation to notify any person of such revisions or changes.

*************************************************************************************************************************/


/********************************************************************************************
 *                                TAP_rtp.c
 *
 * This file is used for RTP/RTCP support in the test application.
 *
 *
 *
 *
 *      Written by                        Version & Date                        Change
 *     ------------                       ---------------                      --------
 *     Tsahi Levent-Levi                    02-Aug-2000
 *
 ********************************************************************************************/



#ifdef __cplusplus
extern "C" {
#endif


#ifdef USE_RTP

#ifdef WIN32
#include <windows.h>
#else
#include <time.h>
#define GetCurrentTime() time(NULL)
#endif

#include <rtp.h>
#include <rtcp.h>
#include <payload.h>

#endif

#include "TAP_general.h"
#include "TAP_rtp.h"

#ifdef RV_USE_R0LOOP
#include <sysrtplp.h>
#endif

#ifdef RV_USE_R0
RVAPI INT32 RVCALLCONV RTP_loadDriver(void);
RVAPI void  RVCALLCONV RTP_unloadDriver(void);
#endif

#ifdef RV_USE_R0LOOP
RVAPI INT32 RVCALLCONV RTP_loadDriverLP(void);
RVAPI void  RVCALLCONV RTP_unloadDriverLP(void);
#endif

#ifdef USE_RTP

#define MAX_SESSIONS (20)


/* Initialized ? */
static BOOL rtp_initialized = FALSE;


/* Current time in milliseconds. */
static UINT32 rtp_curTime;


/* Number of next allocation position for RTP control structure */
static UINT32 rtp_NextAlloc;


/* RTP sessions we can handle */
static RTPCtrl rtp_Ctrl[MAX_SESSIONS];


#endif  /* USE_RTP */


/********************************************************************************************
 *                                                                                          *
 *                                  Private functions                                       *
 *                                                                                          *
 ********************************************************************************************/


#ifdef USE_RTP


/********************************************************************************************
 * rtp_eventHandler
 * purpose : Handle RTP events
 * input   : hRTP       - Handle of the RTP session
 *           context    - Context used on registration for the event
 * output  : none
 * return  : none
 ********************************************************************************************/
void CALLCONVC rtp_eventHandler(HRTPSESSION hRTP, void* context)
{
    static char buff[8000];
    RTPCtrl*    rC = (RTPCtrl*)context;
    rtpParam    p;
    int         error;

    /* Read RTP buffer */
    error = rtpReadEx(hRTP, buff, sizeof(buff), (GetCurrentTime() - rtp_curTime) * 8,&p);

    if (error) {
        TclExecute("test:Log {rtp_eventHandler: rtpRead returned error}");
        return;
    }

    if (rC != NULL)
    {
        /* Add the bytes we received to the session's information */
        rC->bytes += p.len - p.sByte - ((p.payload == H261) ? 4 : 0);

        /* Make sure our own sequence number is synchronized, but make it a little bit
           different than that of the other side. */
        p.sequenceNumber += 29000;

        /* Loop back what we received... */
        if ((rC->channels == 2) &&
            ((rC->action == RTP_Replay) || (rC->action == RTP_RecordReplay)))
        {
            /*if (writeToDisk)
            {
                write(rC->file,&(p.len),4);
                write(rC->file,buff,p.len);
            }*/

            error = rtpWrite(hRTP, buff, p.len, &p);

            if (error < 0)
            {
                TclExecute("test:Log {rtp_eventHandler: rtpWrite returned error}");
            }
        }
    }
}


void RVCALLCONV rtcpRecvCallback(IN HRTCPSESSION hrtcp, IN void* context, IN UINT32 ssrc)
{
    RTCPINFO info;

    rtcpGetSourceInfo(hrtcp, ssrc, &info);
}

#endif  /* USE_RTP */





/********************************************************************************************
 *                                                                                          *
 *                                  Public functions                                        *
 *                                                                                          *
 ********************************************************************************************/


/********************************************************************************************
 * RTP_TestInit
 * purpose : Initialize the RTP/RTCP package of the test application
 * input   : none
 * output  : none
 * return  : none
 ********************************************************************************************/
void RTP_TestInit(void)
{
#ifdef USE_RTP
    int i;

#ifdef RV_USE_R0
    if (RTP_loadDriver() < 0)
    {
        rtp_initialized = FALSE;
        TclExecute("test:Log {!!! RTP driver failed to load !!!}");
        return;
    }

#endif

#ifdef RV_USE_R0LOOP
    if (RTP_loadDriverLP() < 0)
    {
        rtp_initialized = FALSE;
        TclExecute("test:Log {!!! RTP loop driver failed to load !!!}");
        return;
    }

#endif


    /* Initialize RTP and RTCP packages */
    if (rtpInit() < 0)
    {
        rtp_initialized = FALSE;
        TclExecute("test:Log {!!! RTP initialization failed !!!}");
        return;
    }
    if (rtcpInit() < 0)
    {
        rtp_initialized = FALSE;
        rtpEnd();
        TclExecute("test:Log {!!! RTCP initialization failed !!!}");
        return;
    }

    rtp_initialized = TRUE;

    rtp_NextAlloc = 0;
    rtp_curTime = GetCurrentTime();

    for (i = 0; i < MAX_SESSIONS; i++)
        rtp_Ctrl[i].allocated = FALSE;
#endif
}


/********************************************************************************************
 * RTP_TestEnd
 * purpose : Deinitialize the RTP/RTCP package of the test application
 * input   : none
 * output  : none
 * return  : none
 ********************************************************************************************/
void RTP_TestEnd(void)
{
#ifdef USE_RTP
    /* Deinitialize RTP and RTCP packages */
    if (rtp_initialized)
    {
        rtcpEnd();
        rtpEnd();

#ifdef RV_USE_R0LOOP
     RTP_unloadDriverLP();
#endif

#ifdef RV_USE_R0
     RTP_unloadDriver();
#endif
        rtp_initialized = FALSE;
    }
#endif
}


/********************************************************************************************
 * RTP_TestOpen
 * purpose : Open an RTP channel
 * input   : name       - Name to use for the connection
 * output  : none
 * return  : Session number allocated
 *           Negative number on failure
 ********************************************************************************************/
int RTP_TestOpen(const char* name)
{
#ifdef USE_RTP
    BOOL    loop = FALSE;
    int     s, rtpPort;
    HRTPSESSION hRTP = NULL, althRTP = NULL;
    void*   hartcp=NULL;

    /* Find an empty session to allocate */
    while (rtp_Ctrl[rtp_NextAlloc].allocated)
    {
        rtp_NextAlloc++;
        if (rtp_NextAlloc == MAX_SESSIONS)
        {
            rtp_NextAlloc = 0;
            if (loop) return -1; /* Couldn't find an empty session... */
            loop = TRUE;
        }
    }

    s = rtp_NextAlloc;
    rtp_Ctrl[rtp_NextAlloc].allocated = TRUE;
    rtp_NextAlloc = (rtp_NextAlloc + 1) % MAX_SESSIONS;

    /* Add a channel to the session */
    RTP_TestRate(s);

    /* Create an RTP handle for the session */
    rtpPort = 1;
    while ((rtpPort % 2) != 0)
    {
        /* We make sure we've got an even number for the opened port.
           This way of doing it isn't encouraged */
        althRTP = hRTP;
        hRTP = rtpOpenEx((UINT16)0, 1, 0xff, NULL);
        if(hRTP == NULL)
            return RVERROR;
        rtpPort = rtpGetPort(hRTP);
        if(althRTP != NULL)
            rtpClose(althRTP);
    }
    rtp_Ctrl[s].hRTP = hRTP;
    rtp_Ctrl[s].action = RTP_None;

    rtpUseSequenceNumber(rtp_Ctrl[s].hRTP);

    if (rtp_Ctrl[s].hRTCP == NULL)
    {
        /* Create RTCP handle for the session */
        rtp_Ctrl[s].hRTCP = rtcpOpen(rtpGetSSRC(rtp_Ctrl[s].hRTP),
                                     (UINT16)(RTP_TestGetLocalPort(s)+1),
                                     (char*)name);
        if(rtp_Ctrl[s].hRTCP == NULL) return -1;
        rtpSetRTCPSession(rtp_Ctrl[s].hRTP, rtp_Ctrl[s].hRTCP);


#ifdef RV_USE_R0LOOP

#else
        rtcpSetRTCPRecvEventHandler(rtp_Ctrl[s].hRTCP, rtcpRecvCallback, hartcp);
#endif
    }

    rtp_Ctrl[s].channels = 1;

    /* Set the event handler for RTP */
#ifdef RV_USE_R0LOOP
    rtpStartMaintenanceLoop(rtp_Ctrl[s].hRTP, NULL);
#else
    rtpSetEventHandler(rtp_Ctrl[s].hRTP, rtp_eventHandler, NULL);
#endif

    /* Return the port used */
    return s;
#else
    return 0;
#endif
}




/********************************************************************************************
 * RTP_IsInitialized
 * purpose : Indicates if RTP/RTCP package is working
 * input   : none
 * output  : none
 * return  : TRUE if RTP/RCTP package can be used
 ********************************************************************************************/
BOOL RTP_IsInitialized(void)
{
#ifdef USE_RTP
    return rtp_initialized;
#else
    return FALSE;
#endif
}




/********************************************************************************************
 * RTP_TestOpenSecondChannel
 * purpose : Open a second RTP channel on the same connection
 * input   : session    - Session number to use
 * output  : none
 * return  : none
 ********************************************************************************************/
void RTP_TestOpenSecondChannel(int session)
{
#ifdef USE_RTP
    if (session < 0) return;

    rtp_Ctrl[session].channels++;
#endif
}




/********************************************************************************************
 * RTP_TestClose
 * purpose : Close an RTP channel
 * input   : session    - Session number to use
 * output  : none
 * return  : none
 ********************************************************************************************/
void RTP_TestClose(int session)
{
#ifdef USE_RTP
    if (session < 0) return;
    if (rtp_Ctrl[session].channels == 0) return;

    /* Remove a channel from the session */
    rtp_Ctrl[session].channels--;

    if (rtp_Ctrl[session].channels == 0)
    {
        /* Close the session */
        if(rtp_Ctrl[session].hRTP != NULL)
        {
            rtpClose(rtp_Ctrl[session].hRTP);
            rtp_Ctrl[session].hRTP  = NULL;
            rtp_Ctrl[session].hRTCP = NULL;
        }
        rtp_Ctrl[session].allocated = FALSE;
    }
#endif
}




/********************************************************************************************
 * RTP_TestGetLocalPort
 * purpose : Return the local port used by the session
 * input   : session    - Session number to use
 * output  : none
 * return  : Port number used
 ********************************************************************************************/
int RTP_TestGetLocalPort(int session)
{
#ifdef USE_RTP
    if ((session >= 0) && (rtp_Ctrl[session].hRTP != NULL))
        return rtpGetPort(rtp_Ctrl[session].hRTP);
    else
        return 0;
#else
    return 2326;
#endif
}



/********************************************************************************************
 * RTP_TestSetRemoteRTP
 * purpose : Open an RTP channel
 * input   : session        - Session number to use
 *           ip             - IP number of remote side
 *           port           - Port number of remote side
 *           isMulticast    - Is it a multicast address or not
 * output  : none
 * return  : none
 ********************************************************************************************/
void RTP_TestSetRemoteRTP(int session, UINT32 ip, UINT16 port, BOOL isMulticast)
{
#ifdef USE_RTP
    if (session < 0) return;

    rtpSetRemoteAddress(rtp_Ctrl[session].hRTP, ip, port);
    if (isMulticast)
        rtcpSetGroupAddress(rtp_Ctrl[session].hRTCP, ip);
#ifdef RV_USE_R0LOOP
    rtpStartMaintenanceLoop(rtp_Ctrl[session].hRTP, &rtp_Ctrl[session]);
#else
    rtpSetEventHandler(rtp_Ctrl[session].hRTP, rtp_eventHandler, &rtp_Ctrl[session]);
#endif

#endif
}



/********************************************************************************************
 * RTP_TestSetRemoteRTCP
 * purpose : Open an RTP channel
 * input   : session        - Session number to use
 *           ip             - IP number of remote side
 *           port           - Port number of remote side
 *           isMulticast    - Is it a multicast address or not
 * output  : none
 * return  : none
 ********************************************************************************************/
void RTP_TestSetRemoteRTCP(int session, UINT32 ip, UINT16 port, BOOL isMulticast)
{
#ifdef USE_RTP
    if (session < 0) return;

    if (rtp_Ctrl[session].hRTCP != NULL)
    {
        rtcpSetRemoteAddress(rtp_Ctrl[session].hRTCP, ip, port);
        if (isMulticast)
            rtcpSetGroupAddress(rtp_Ctrl[session].hRTCP, ip);
    }
#endif
}



/********************************************************************************************
 * RTP_TestSetAction
 * purpose : Set an action on the given session: playback, record, replay, etc.
 * input   : session    - Session number to use
 *           action     - The action to set
 * output  : none
 * return  : none
 ********************************************************************************************/
void RTP_TestSetAction(int session, RTP_Action action)
{
#ifdef USE_RTP
    if (session < 0) return;

    rtp_Ctrl[session].action = action;
#endif
}



/********************************************************************************************
 * RTP_TestRate
 * purpose : Return the rate of a session
 * input   : none
 * output  : none
 * return  : Rate of the session
 ********************************************************************************************/
UINT32 RTP_TestRate(int session)
{
#ifdef USE_RTP
    UINT32  rate, curtime;

    if (session < 0) return 0;

    curtime = GetCurrentTime();

    /* Calculate the rate */
    rate = (rtp_Ctrl[session].bytes * 8) / (curtime - rtp_Ctrl[session].lastTime + 1);

    /* Set session information */
    rtp_Ctrl[session].lastTime = curtime;
    rtp_Ctrl[session].bytes = 0;

    return rate;
#else
    return 0;
#endif
}



#ifdef __cplusplus
}
#endif



