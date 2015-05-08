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


#ifndef CMSIZE_H
#define CMSIZE_H

#include <cm.h>


RVAPI int RVCALLCONV
cmSizeCurProtocols(HAPP hApp);

RVAPI int RVCALLCONV
cmSizeMaxProtocols(HAPP hApp);

RVAPI int RVCALLCONV
cmSizeCurProcs(HAPP hApp);

RVAPI int RVCALLCONV
cmSizeMaxProcs(HAPP hApp);

RVAPI int RVCALLCONV
cmSizeCurEvents(HAPP hApp);

RVAPI int RVCALLCONV
cmSizeMaxEvents(HAPP hApp);

RVAPI int RVCALLCONV
cmSizeCurTimers(HAPP hApp);

RVAPI int RVCALLCONV
cmSizeMaxTimers(HAPP hApp);

RVAPI int RVCALLCONV
cmSizeCurUdpChans(HAPP hApp);

RVAPI int RVCALLCONV
cmSizeMaxUdpChans(HAPP hApp);

RVAPI int RVCALLCONV
cmSizeCurTpktChans(HAPP hApp);

RVAPI int RVCALLCONV
cmSizeMaxTpktChans(HAPP hApp);

RVAPI int RVCALLCONV
cmSizeCurChannels(HAPP hApp);

RVAPI int RVCALLCONV
cmSizeMaxChannels(HAPP hApp);

RVAPI int RVCALLCONV
cmSizeCurMessages(HAPP hApp);

RVAPI int RVCALLCONV
cmSizeMaxMessages(HAPP hApp);

RVAPI int RVCALLCONV
cmSizeCurChanDescs(HAPP hApp);

RVAPI int RVCALLCONV
cmSizeMaxChanDescs(HAPP hApp);



#endif

#ifdef __cplusplus
}
#endif

