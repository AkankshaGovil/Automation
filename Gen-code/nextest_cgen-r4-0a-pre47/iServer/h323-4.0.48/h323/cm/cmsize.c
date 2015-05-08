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


#include <cm.h>
#include <cmintr.h>


RVAPI UINT32 RVCALLCONV mtimerGetCurTimers(void);
RVAPI UINT32 RVCALLCONV mtimerGetMaxTimers(void);



RVAPI int RVCALLCONV cmSizeCurProtocols(HAPP hApp){   if (hApp);return RVERROR;}
RVAPI int RVCALLCONV cmSizeMaxProtocols(HAPP hApp){   if (hApp);return RVERROR;}
RVAPI int RVCALLCONV cmSizeCurProcs(HAPP hApp)    {   if (hApp);return RVERROR;}
RVAPI int RVCALLCONV cmSizeMaxProcs(HAPP hApp)    {   if (hApp);return RVERROR;}
RVAPI int RVCALLCONV cmSizeCurEvents(HAPP hApp)   {   if (hApp);return RVERROR;}
RVAPI int RVCALLCONV cmSizeMaxEvents(HAPP hApp)   {   if (hApp);return RVERROR;}
RVAPI int RVCALLCONV cmSizeCurUdpChans(HAPP hApp) {   if (hApp);return RVERROR;}
RVAPI int RVCALLCONV cmSizeMaxUdpChans(HAPP hApp) {   if (hApp);return RVERROR;}

RVAPI int RVCALLCONV cmSizeCurTimers(HAPP hApp)
{
    if (hApp);
    return mtimerGetCurTimers();
}

RVAPI int RVCALLCONV cmSizeMaxTimers(HAPP hApp)
{
    if (hApp);
    return mtimerGetMaxTimers();
}

RVAPI int RVCALLCONV cmSizeCurTpktChans(HAPP hApp)
{   
    cmElem* app=(cmElem*)hApp;
	int curNum;

	if (!hApp)
		return RVERROR;
	if (cmTransGetHostsStats(app->hTransport, &curNum, NULL) != cmTransOK)
		return RVERROR;
	else
		return curNum;
}

RVAPI int RVCALLCONV cmSizeMaxTpktChans(HAPP hApp)
{   
    cmElem* app=(cmElem*)hApp;
	int maxNum;

	if (!hApp)
		return RVERROR;
	if (cmTransGetHostsStats(app->hTransport, NULL, &maxNum) != cmTransOK)
		return RVERROR;
	else
		return maxNum;
}

RVAPI int RVCALLCONV cmSizeCurMessages(HAPP hApp)
{   
    cmElem* app=(cmElem*)hApp;
	int curNum;

	if (!hApp)
		return RVERROR;
	if (cmTransGetMessagesStats(app->hTransport, &curNum, NULL) != cmTransOK)
		return RVERROR;
	else
		return curNum;
}

RVAPI int RVCALLCONV cmSizeMaxMessages(HAPP hApp) 
{	
    cmElem* app=(cmElem*)hApp;
	int maxNum;

	if (!hApp)
		return RVERROR;
	if (cmTransGetMessagesStats(app->hTransport, NULL, &maxNum) != cmTransOK)
		return RVERROR;
	else
		return maxNum;
}

RVAPI int RVCALLCONV cmSizeCurChannels(HAPP hApp) {   if (hApp);return RVERROR;}
RVAPI int RVCALLCONV cmSizeMaxChannels(HAPP hApp) {   if (hApp);return RVERROR;}
RVAPI int RVCALLCONV cmSizeCurChanDescs(HAPP hApp){   if (hApp);return RVERROR;}
RVAPI int RVCALLCONV cmSizeMaxChanDescs(HAPP hApp){   if (hApp);return RVERROR;}

#ifdef __cplusplus
}
#endif

