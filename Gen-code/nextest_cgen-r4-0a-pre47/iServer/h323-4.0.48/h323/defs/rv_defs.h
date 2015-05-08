/*

NOTICE:
This document contains information that is proprietary to RADVISION LTD..
No part of this publication may be reproduced in any form whatsoever without
written prior approval by RADVISION LTD..

RADVISION LTD. reserves the right to revise this publication and make changes
without obligation to notify any person of such revisions or changes.

*/

#ifndef _RV_DEFS_H
#define _RV_DEFS_H

#ifdef __cplusplus
extern "C" {
#endif



/* --- transport address --- */

typedef enum {
  cmTransportTypeIP, /* No route */
  cmTransportTypeIPStrictRoute,
  cmTransportTypeIPLooseRoute,
  cmTransportTypeNSAP
} cmTransportType;

#define cmRouteSize (7) /* for luck */

typedef enum {
  cmDistributionUnicast,
  cmDistributionMulticast
} cmDistribution;

typedef struct
{
  UINT16          length; /* length in bytes of route */
  UINT32          ip;
  UINT16          port;
  cmTransportType type;
  UINT32          route[cmRouteSize];
  cmDistribution  distribution;
} cmTransportAddress;

typedef enum
{
    cmQ931setup =5,
    cmQ931callProceeding=2,
    cmQ931connect=7,
    cmQ931alerting=1,
    cmQ931releaseComplete=90,
    cmQ931status=125,
    cmQ931facility=98,
    cmQ931statusEnquiry=117,
    cmQ931statusInquiry=117,
    cmQ931progress=3,
    cmQ931setupAck=13,
    cmQ931setupAcknowledge=13,
    cmQ931information=123,
    cmQ931notify=110

}cmCallQ931MsgType;

/* The stage at which it is allowed to transfer the H.245 address to the remote */
typedef enum {
    cmTransH245Stage_setup,             /* Earliest H.245 possible, may send/act on addresses in all messages */
    cmTransH245Stage_callProceeding,    /* The remote can send the address in the C.P. message */
    cmTransH245Stage_alerting,          /* Only in the alerting message can the remote send the address */
    cmTransH245Stage_connect,           /* The remote must wait for the connect message (normal) */
    cmTransH245Stage_early,             /* our early H.245, sends addresses in setup and connect only */
    cmTransH245Stage_facility,          /* There will be no automatic sending of the address */
    cmTransH245Stage_noH245             /* no support for H.245, sends facility NoH245  */
} cmH245Stage;

/* the way to use annex E connection for Q.931 */
typedef enum {
    cmTransUseAnnexE,                   /* Force the use of annex E */
    cmTransNoAnnexE,                    /* Force the use of TPKT */
    cmTransPreferedAnnexE,              /* Initiate a both annex E
                                           and TPKT with preference to annex E*/
    cmTransPreferedTPKT                 /* Initiate a both annex E
                                           and TPKT with preference to TPKT*/
} cmAnnexEUsageMode;



#ifdef __cplusplus
}
#endif


#endif  /* _RV_DEFS_H */
