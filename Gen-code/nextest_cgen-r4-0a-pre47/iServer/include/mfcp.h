
/*
 * Copyright (c) 2003 NExtone Communications, Inc.
 * All rights reserved.
 */

#ifndef _LIBMFCP_H
#define _LIBMFCP_H

#include <netinet/in.h>


/*****************************************************************************
 *
 *  Typedef Definitions
 *
*****************************************************************************/
#ifndef MFCP_PRIVATE_INTERFACE
typedef void MFCP_Session;
typedef void MFCP_Request;
#endif


typedef unsigned int MFCP_ResourceId;

/*****************************************************************************
 *
 *  Enumerations
 *
*****************************************************************************/
typedef enum {
  MFCP_RET_OK = 0,               /* Success */
  MFCP_RET_NOTOK = -1,           /* Generic failure */ 
  MFCP_RET_CONERR = -2,          /* Connection error */ 
  MFCP_RET_BINDERR = -3,         /* bind error */
  MFCP_RET_GETSOCKERR = -4,      /* getsock error */
  MFCP_RET_NODISP = -5,          /* error in dispatch */
  MFCP_RET_SNDQFULL = -6,        /* snd q is full */
  MFCP_RET_BADINT = 0xffffffff   /* generic bad integer */
} MFCP_Return;

/* type of request */
typedef enum
{
  MFCP_REQ_NONE = 0,
  MFCP_REQ_CRS = 1,
  MFCP_REQ_MDS = 2,
  MFCP_REQ_CRB = 3,
  MFCP_REQ_CRR = 4,
  MFCP_REQ_MDR = 5,
  MFCP_REQ_DLR = 6,
  MFCP_REQ_DLB = 7,
  MFCP_REQ_DLS = 8,
  MFCP_REQ_AUS = 9,
  MFCP_REQ_HLP = 10,
  MFCP_REQ_PNG = 11,
  MFCP_REQ_NTF = 12,
  MFCP_REQ_RQT = 13,
  MFCP_REQ_RSP = 14,
  MFCP_REQ_CHR = 15,
  MFCP_REQ_END = 16
} MFCP_RequestType;

char * mfcp_req_str (MFCP_RequestType type);

/* Parameter Data - if a param is added then you must modify the mfcp_param_str function */
typedef enum
{
  MFCP_PARAM_NONE = 0,
  MFCP_PARAM_SESSION_ID = 1,
  MFCP_PARAM_BUNDLE_ID = 2,
  MFCP_PARAM_RESOURCE_ID = 3,
  MFCP_PARAM_REPORT_ERRORS = 4,
  MFCP_PARAM_SHUTDOWN_INTERVAL = 5,
  MFCP_PARAM_ID = 6,
  MFCP_PARAM_SRC = 7,
  MFCP_PARAM_DEST = 8,
  MFCP_PARAM_PROTOCOL = 9,
  MFCP_PARAM_SRC_POOL = 10,
  MFCP_PARAM_DEST_POOL = 11,
  MFCP_PARAM_TYPE = 12,
  MFCP_PARAM_BANDWIDTH = 13,
  MFCP_PARAM_DATA = 14,
  MFCP_PARAM_PEER_RESOURCE_ID = 16,
  MFCP_PARAM_OPTYPE = 17,
  MFCP_PARAM_DTMFDETECT = 18,
  MFCP_PARAM_DTMFDETECTPARAM = 19,
  MFCP_PARAM_NAT_SRC = 20,
  MFCP_PARAM_NAT_DEST = 21,
  MFCP_PARAM_DEST_SYM = 22,
  MFCP_PARAM_END = 23
} MFCP_ParameterTypes;

typedef enum 
{
  MFCP_RSTATUS_OK =200,
  MFCP_RSTATUS_NOTOK=400
} MFCP_Status;

/* values that the dst_sym parameter in CRR can take */
typedef enum
{
  MFCP_DEST_SYM_NONE = 0,
  MFCP_DEST_SYM_DISC = 1,
  MFCP_DEST_SYM_FORCE = 2
} MFCP_DestSymType;

/*****************************************************************************
 *
 *  Function Prototypes
 *
*****************************************************************************/

#ifdef MFCP_PRIVATE_INTERFACE
#include "mfcplocal.h"
#endif

typedef MFCP_Session *pMFCP_Session;

  /* Session Methods: */
int mfcp_init(void);

void mfcp_reconfig(MFCP_Session *sess);

MFCP_Session *mfcp_sess_new (void);
MFCP_Session *mfcp_sess_free (MFCP_Session * sess);
MFCP_Session *mfcp_sess_connect (unsigned long int addr,
                                 unsigned int timeout,
				 void (*asyncFunc)(void *rPtr));

int mfcp_sess_close (MFCP_Session * sess);

/* interface routines */
MFCP_Return  mfcp_req_dlr (MFCP_Session * sess, int bundleId, int resourceId,
			   void (*func)(void *), void *appData);

MFCP_Request *mfcp_sreq_dlr (MFCP_Session * sess, int bundleId, int resourceId);

MFCP_Return  mfcp_req_crr (MFCP_Session * sess,
			   int bundleId,
			   MFCP_ResourceId peerResourceId,
			   unsigned int saddr, unsigned short sport,
			   unsigned int daddr, unsigned short dport, 
			   unsigned int nat_saddr, unsigned short nat_sport,
			   unsigned int nat_daddr, unsigned short nat_dport, 
			   unsigned int ingressPool, unsigned int egressPool,
			   char *protocol, int dtmf_detect, int dtmf_detect_param, unsigned int dst_sym, 
			   int	optype,
			   void (*func)(void *), void *appData);

MFCP_Request  *mfcp_sreq_crr (MFCP_Session * sess,
			   int bundleId,
			   MFCP_ResourceId peerResourceId,
			   unsigned int saddr, unsigned short sport,
			   unsigned int daddr, unsigned short dport, 
			   unsigned int nat_saddr, unsigned short nat_sport,
			   unsigned int nat_daddr, unsigned short nat_dport, 
			   unsigned int ingressPool, unsigned int egressPool,
			   char *protocol, int dtmf_detect, int dtmf_detect_param, unsigned int dst_sym);

MFCP_Return mfcp_req_chr(MFCP_Session *sess, 
			 int bundleId,
			 MFCP_ResourceId resourceId, 
			 MFCP_ResourceId peerResourceId, 
			 unsigned int saddr, unsigned short sport,
			 unsigned int daddr, unsigned short dport, 
			 unsigned int nat_saddr, unsigned short nat_sport,
			 unsigned int nat_daddr, unsigned short nat_dport, 
			 unsigned int ingressPool, unsigned int egressPool,
			 char *protocol, int dtmf_detect, int dtmf_detect_param, 
			 int	optype,
			 void (*func)(void *), void *appData);

MFCP_Request *mfcp_sreq_chr(MFCP_Session *sess, 
			 int bundleId,
			 MFCP_ResourceId resourceId, 
			 MFCP_ResourceId peerResourceId, 
			 unsigned int saddr, unsigned short sport,
			 unsigned int daddr, unsigned short dport, 
			 unsigned int nat_saddr, unsigned short nat_sport,
			 unsigned int nat_daddr, unsigned short nat_dport, 
			 unsigned int ingressPool, unsigned int egressPool,
			 char *protocol, int dtmf_detect, int dtmf_detect_param);
     
MFCP_Return mfcp_req_dlb (MFCP_Session * sess, int bundleId,
		  void (*func)(void *), void *appData);

MFCP_Request *mfcp_sreq_dlb (MFCP_Session * sess, int bundleId);


MFCP_Return mfcp_req_dls (MFCP_Session * sess,
		  void (*func)(void *), void *appData);

MFCP_Request *mfcp_sreq_dls (MFCP_Session * sess);

/* Request data extraction routines */

unsigned int mfcp_get_dest_addr(MFCP_Request *rPtr);
int mfcp_get_dest_port(MFCP_Request *rPtr);

unsigned int mfcp_get_nat_dest_addr(MFCP_Request *rPtr);
int mfcp_get_nat_dest_port(MFCP_Request *rPtr);

unsigned int mfcp_get_nat_src_addr(MFCP_Request *rPtr);
int mfcp_get_nat_src_port(MFCP_Request *rPtr);

int mfcp_get_int(MFCP_Request *rPtr, MFCP_ParameterTypes param);
char *mfcp_get_str(MFCP_Request *rPtr, MFCP_ParameterTypes param);

MFCP_Session *mfcp_get_sess(MFCP_Request *rPtr);
MFCP_Status mfcp_get_res_status(MFCP_Request *rPtr);
MFCP_RequestType mfcp_get_res_type(MFCP_Request *rPtr);
char *mfcp_get_res_estring(MFCP_Request *rPtr);
void *mfcp_get_res_appdata(MFCP_Request *rPtr);

/* destroy request and associated data */
MFCP_Request *mfcp_req_free (MFCP_Request *rPtr);
int mfcp_dst_sym_str_to_int (const char *val);
#endif /* _LIBMFCP_H */


