#ifdef __cplusplus
extern "C" {
#endif



/*

NOTICE:
This document contains information that is proprietary to RADVISION LTD.
No part of this publication may be reproduced in any form whatsoever without
written prior approval by RADVISION LTD.

RADVISION LTD. reserves the right to revise this publication and make changes
without obligation to notify any person of such revisions or changes.

*/

#ifndef __PAYLOAD_H
#define __PAYLOAD_H



typedef enum
{
    PCMU  =  0,
    G7231 =  4,
    PCMA  =  8,
    G722  =  9,
    G728  = 15,
    G729  = 18,
    H261  = 31,
    H263  = 34
} payload_e;



/* == G.711 U-Law == */

RVAPI
INT32 RVCALLCONV rtpPCMUPack(
        IN  void *      buf,
        IN  INT32       len,
        IN  rtpParam *  p,
        IN  void     *  param);

RVAPI
INT32 RVCALLCONV rtpPCMUUnpack(
        OUT  void *      buf,
        IN   INT32       len,
        OUT  rtpParam *  p,
        OUT  void *      param);

RVAPI
INT32 RVCALLCONV rtpPCMUGetHeaderLength(void);


/* == G.711 A-Law == */

RVAPI
INT32 RVCALLCONV rtpPCMAPack(
        IN  void *      buf,
        IN  INT32       len,
        IN  rtpParam *  p,
        IN  void     *  param);

RVAPI
INT32 RVCALLCONV rtpPCMAUnpack(
        OUT  void *      buf,
        IN   INT32       len,
        OUT  rtpParam *  p,
        OUT  void *      param);

RVAPI
INT32 RVCALLCONV rtpPCMAGetHeaderLength(void);


/* == G.722 == */

RVAPI
INT32 RVCALLCONV rtpG722Pack(
        IN  void *      buf,
        IN  INT32       len,
        IN  rtpParam *  p,
        IN  void     *  param);


RVAPI
INT32 RVCALLCONV rtpG722Unpack(
        OUT  void *      buf,
        IN   INT32       len,
        OUT  rtpParam *  p,
        OUT  void *      param);

RVAPI
INT32 RVCALLCONV rtpG722GetHeaderLength(void);

/* == G.728 == */

RVAPI
INT32 RVCALLCONV rtpG728Pack(
        IN  void *      buf,
        IN  INT32       len,
        IN  rtpParam *  p,
        IN  void     *  param);


RVAPI
INT32 RVCALLCONV rtpG728Unpack(
        OUT  void *      buf,
        IN   INT32       len,
        OUT  rtpParam *  p,
        OUT  void *      param);

RVAPI
INT32 RVCALLCONV rtpG728GetHeaderLength(void);

/* == G.729 == */

RVAPI
INT32 RVCALLCONV rtpG729Pack(
        IN  void *      buf,
        IN  INT32       len,
        IN  rtpParam *  p,
        IN  void     *  param);


RVAPI
INT32 RVCALLCONV rtpG729Unpack(
        OUT  void *      buf,
        IN   INT32       len,
        OUT  rtpParam *  p,
        OUT  void *      param);

RVAPI
INT32 RVCALLCONV rtpG729GetHeaderLength(void);


/* == G.723.1 == */
RVAPI
INT32 RVCALLCONV rtpG7231Pack(
        IN  void *      buf,
        IN  INT32       len,
        IN  rtpParam *  p,
        IN  void     *  param);

RVAPI
INT32 RVCALLCONV rtpG7231Unpack(
        OUT  void *      buf,
        IN   INT32       len,
        OUT  rtpParam *  p,
        OUT  void *      param);

RVAPI
INT32 RVCALLCONV rtpG7231GetHeaderLength(void);


/* == H.261 == */

typedef struct
{
    int sBit;
    int eBit;
    int i;
    int v;
    int gobN;
    int mbaP;
    int quant;
    int hMvd;
    int vMvd;
} H261param;

RVAPI
INT32 RVCALLCONV rtpH261Pack(
        IN  void *      buf,
        IN  INT32       len,
        IN  rtpParam *  p,
        IN  void     *  param);

RVAPI
INT32 RVCALLCONV rtpH261Unpack(
        OUT  void*buf,
        IN   INT32       len,
        OUT  rtpParam*p,
        OUT  void *      param);

RVAPI
INT32 RVCALLCONV rtpH261GetHeaderLength(void);


/* == H.263 == */

typedef struct
{
    int f;
    int p;
    int sBit;
    int eBit;
    int src;
    int i;
    int a;
    int s;
    int dbq;
    int trb;
    int tr;
    int gobN;
    int mbaP;
    int quant;
    int hMv1;
    int vMv1;
    int hMv2;
    int vMv2;
} H263param;

RVAPI
INT32 RVCALLCONV rtpH263Pack(
        IN  void *      buf,
        IN  INT32       len,
        IN  rtpParam *  p,
        IN  void     *  param);

RVAPI
INT32 RVCALLCONV rtpH263Unpack(
        OUT  void *      buf,
        IN   INT32       len,
        OUT  rtpParam *  p,
        OUT  void *      param);


RVAPI
INT32 RVCALLCONV rtpH263GetHeaderLength(void);



/* == H.263a == */

typedef struct
{
    int f;
    int p;
    int sBit;
    int eBit;
    int src;
    int i;
    int u;
    int a;
    int s;
    int dbq;
    int trb;
    int tr;
    int gobN;
    int mbaP;
    int quant;
    int hMv1;
    int vMv1;
    int hMv2;
    int vMv2;
} H263aparam;

RVAPI
INT32 RVCALLCONV rtpH263aPack(
        IN  void *      buf,
        IN  INT32       len,
        IN  rtpParam *  p,
        IN  void     *  param);

RVAPI
INT32 RVCALLCONV rtpH263aUnpack(
        OUT  void *      buf,
        IN   INT32       len,
        OUT  rtpParam *  p,
        OUT  void *      param);


RVAPI
INT32 RVCALLCONV rtpH263aGetHeaderLength(void);




#endif  /* __PAYLOAD_H */
#ifdef __cplusplus
}
#endif



