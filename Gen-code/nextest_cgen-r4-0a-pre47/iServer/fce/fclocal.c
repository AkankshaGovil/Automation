/***************************************************************************
                          fclocal.c  -  description
                             -------------------
    begin                : Thu Jun 19 2003
    copyright            : (C) 2003 by bruce mattie
    email                : bmattie@nextone.com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   Copyright Nextone Communications                                      *
 *   2003                                                                  *
 *                                                                         *
 ***************************************************************************/

/* fclocal.c - extract arguments from the requests and execute local NSF */


#include <stdio.h>
#include <strings.h>
#include <netinet/in.h>

#define MFCP_PRIVATE_INTERFACE
#include "mfcp.h"
#include "mfcpproto.h"
#include "fclocal.h"
#include "nsfglue.h"

int
fc_dorequest (MFCP_Request * rPtr)
{
  in_addr_t dAddr,sAddr;
  uint16_t dPort,sPort;
  uint16_t dPool, sPool;
  unsigned int bundleId;
  unsigned int resourceId, peerResourceId;
  char *strProto;
  in_addr_t retNatSrcAddr, retNatDestAddr;
  uint16_t retNatSrcPort, retNatDestPort;
  uint32_t retResId;
  char saddrString[80], daddrString[80];
  MFCP_Value *val_p;
  int	optype = 0; 		/* default value */
  int dstSym = 0;

  mfcp_req_display(rPtr, "fc_dorequest");

  switch (rPtr->type) {
    /********************************* CRR ***********************************************/
  case MFCP_REQ_CRR:           /* create resource */

    if ((mfcp_get_addrport
         (mfcp_value_getstr (mfcp_req_getval (rPtr, MFCP_PARAM_DEST)),
          &dAddr, &dPort)) != MFCP_RET_OK) {
      mfcp_logit ("Bad dest address in MFCP_REQ_CRR");
      mfcp_send_failure(rPtr->sess,rPtr,400,"Bad dest address in MFCP_REQ_CRR");
      return (MFCP_RET_NOTOK);
    }

    if ((mfcp_get_addrport
         (mfcp_value_getstr (mfcp_req_getval (rPtr, MFCP_PARAM_SRC)),
          &sAddr, &sPort)) != MFCP_RET_OK) {
      mfcp_logit ("Bad src address in MFCP_REQ_CRR");
      mfcp_send_failure(rPtr->sess,rPtr,400,"Bad src address in MFCP_REQ_CRR");
      return (MFCP_RET_NOTOK);
    }

    dstSym = mfcp_dst_sym_str_to_int( mfcp_value_getstr (mfcp_req_getval (rPtr, MFCP_PARAM_DEST_SYM)));

    if ((strProto =
         mfcp_value_getstr (mfcp_req_getval (rPtr, MFCP_PARAM_PROTOCOL))) ==
        NULL) {
      mfcp_logit ("Bad or missing protocol in MFCP_REQ_CRR");
      mfcp_send_failure(rPtr->sess,rPtr,400,"Bad or missing protol in MFCP_REQ_CRR");
      return (MFCP_RET_NOTOK);
    }

    if ((optype = mfcp_value_getint (mfcp_req_getval (rPtr, MFCP_PARAM_OPTYPE))) ==
        (int)(MFCP_RET_BADINT)) {
		optype = 0;	/* Default value */
    }

    if ((mfcp_get_addrport
         (mfcp_value_getstr (mfcp_req_getval (rPtr, MFCP_PARAM_NAT_SRC)),
          &retNatSrcAddr, &retNatSrcPort)) != MFCP_RET_OK) {
      retNatSrcAddr = 0;
      retNatSrcPort = 0;
    }

    if ((mfcp_get_addrport
         (mfcp_value_getstr (mfcp_req_getval (rPtr, MFCP_PARAM_NAT_DEST)),
          &retNatDestAddr, &retNatDestPort)) != MFCP_RET_OK) {
      retNatDestAddr = 0;
      retNatDestPort = 0;
    }

    bundleId = (unsigned int) mfcp_value_getint (mfcp_req_getval (rPtr, MFCP_PARAM_BUNDLE_ID));
    dPool = (unsigned int) mfcp_value_getint (mfcp_req_getval (rPtr, MFCP_PARAM_DEST_POOL));
    sPool = (unsigned int) mfcp_value_getint (mfcp_req_getval (rPtr, MFCP_PARAM_SRC_POOL));
    if ( (val_p = mfcp_req_getval (rPtr, MFCP_PARAM_PEER_RESOURCE_ID)) != NULL) {
    	peerResourceId = (unsigned int) mfcp_value_getint (val_p);
	}
	else {
		peerResourceId = 0;
	}

    if (nsfGlueOpenResource (bundleId,
			     dAddr,
			     dPort,
			     dPool,
			     sPool,
			     strProto,
				 peerResourceId,
			     &retNatSrcAddr,
			     &retNatSrcPort,
			     &retNatDestAddr,
			     &retNatDestPort,
			     &retResId,
			     dstSym,
			     optype) ) {

      mfcp_logit("Error from nsfGlueOpenResource");
      mfcp_send_failure(rPtr->sess,rPtr,400,"Error from nsfGlueOpenResource");
      return(MFCP_RET_NOTOK);
    }

    /* build up the response for this request and queue it back */

    mfcp_res_add_param (rPtr, MFCP_PARAM_NAT_DEST,
			mfcp_value_new_s (mfcp_make_addrstring
					  (saddrString, retNatDestAddr, retNatDestPort)));

    mfcp_res_add_param (rPtr, MFCP_PARAM_NAT_SRC,
			mfcp_value_new_s (mfcp_make_addrstring
					  (saddrString, retNatSrcAddr, retNatSrcPort)));

    mfcp_res_add_param(rPtr, MFCP_PARAM_RESOURCE_ID,
		       mfcp_value_new_i(retResId));
    break;
    /********************************* CHR ***********************************************/
  case MFCP_REQ_CHR:           /* change resource */

    resourceId = mfcp_value_getint(mfcp_req_getval(rPtr,MFCP_PARAM_RESOURCE_ID)); 
    bundleId = (unsigned int) mfcp_value_getint (mfcp_req_getval (rPtr, MFCP_PARAM_BUNDLE_ID));

    if ((strProto =
         mfcp_value_getstr (mfcp_req_getval (rPtr, MFCP_PARAM_PROTOCOL))) ==
        NULL) {
      mfcp_logit ("Bad or missing protocol in MFCP_REQ_CHR");
      mfcp_send_failure(rPtr->sess,rPtr,400,"Bad or missing protol in MFCP_REQ_CHR");
      return (MFCP_RET_NOTOK);
    }

    if ((mfcp_get_addrport
         (mfcp_value_getstr (mfcp_req_getval (rPtr, MFCP_PARAM_DEST)),
          &dAddr, &dPort)) != MFCP_RET_OK) {
      mfcp_logit ("Bad dest address in MFCP_REQ_CHR");
      mfcp_send_failure(rPtr->sess,rPtr,400,"Bad dest address in MFCP_REQ_CHR");
      return (MFCP_RET_NOTOK);
    }

    if ((mfcp_get_addrport
         (mfcp_value_getstr (mfcp_req_getval (rPtr, MFCP_PARAM_NAT_SRC)),
          &retNatSrcAddr, &retNatSrcPort)) != MFCP_RET_OK) {
      retNatSrcAddr = 0;
      retNatSrcPort = 0;
    }

    if ((mfcp_get_addrport
         (mfcp_value_getstr (mfcp_req_getval (rPtr, MFCP_PARAM_NAT_DEST)),
          &retNatDestAddr, &retNatDestPort)) != MFCP_RET_OK) {
      retNatDestAddr = 0;
      retNatDestPort = 0;
    }

    sPool = (unsigned int)mfcp_value_getint (mfcp_req_getval (rPtr, MFCP_PARAM_SRC_POOL));

    if ((mfcp_get_addrport
         (mfcp_value_getstr (mfcp_req_getval (rPtr, MFCP_PARAM_SRC)),
          &sAddr, &sPort)) != MFCP_RET_OK) {
      mfcp_logit ("Bad src address in MFCP_REQ_MDR");
      mfcp_send_failure(rPtr->sess,rPtr,400,"Bad src address in MFCP_REQ_MDR");
      return (MFCP_RET_NOTOK);
    }

    dPool = (unsigned int)mfcp_value_getint (mfcp_req_getval (rPtr, MFCP_PARAM_DEST_POOL));
    dstSym = mfcp_dst_sym_str_to_int( mfcp_value_getstr (mfcp_req_getval (rPtr, MFCP_PARAM_DEST_SYM)));

    if ( (val_p = mfcp_req_getval (rPtr, MFCP_PARAM_PEER_RESOURCE_ID)) != NULL) {
    	peerResourceId = (unsigned int) mfcp_value_getint (val_p);
	}
	else {
		peerResourceId = 0;
	}

    if ((optype = mfcp_value_getint (mfcp_req_getval (rPtr, MFCP_PARAM_OPTYPE))) ==
        (int)(MFCP_RET_BADINT)) {
		optype = 0;	/* Default value */
    }

	if (nsfGlueModifyResource (	bundleId,
			   					resourceId,
							   	dAddr,
							   	dPort,
							   	dPool,
								sPool,
							   	peerResourceId,
			     					&retNatSrcAddr,
			 				        &retNatSrcPort,
				       			        &retNatDestAddr,
			     					&retNatDestPort,
								dstSym,
							   	optype)) {
		mfcp_logit("Error from nsfGlueModifyResource");
		mfcp_send_failure(rPtr->sess,rPtr,400,"Error from nsfGlueModifyResource");
		return(MFCP_RET_NOTOK);
	}

    /* build up the response for this request and queue it back */

	mfcp_res_add_param (rPtr, MFCP_PARAM_NAT_SRC,
			  mfcp_value_new_s (mfcp_make_addrstring
					    (saddrString, retNatSrcAddr, retNatSrcPort)));

	mfcp_res_add_param (rPtr, MFCP_PARAM_NAT_DEST,
			  mfcp_value_new_s (mfcp_make_addrstring
					    (daddrString, retNatDestAddr, retNatDestPort)));

	mfcp_res_add_param(rPtr, MFCP_PARAM_RESOURCE_ID,
			 mfcp_value_new_i(resourceId));
      

    break;
    /********************************* DLB ***********************************************/
  case MFCP_REQ_DLB:           /* delete bundle */
    bundleId = mfcp_value_getint(mfcp_req_getval(rPtr,MFCP_PARAM_BUNDLE_ID)); 

    if (nsfGlueCloseBundle (bundleId)) {
      mfcp_logit("Error from nsfGlueCloseBundle");
      mfcp_send_failure(rPtr->sess,rPtr,400,"Error from nsfGlueCloseBundle");
      return(MFCP_RET_NOTOK);
    }
    mfcp_res_add_param(rPtr, MFCP_PARAM_RESOURCE_ID,
		       mfcp_value_new_i(bundleId));

    break;

  case MFCP_REQ_DLR:           /* delete a resource */
    resourceId = mfcp_value_getint (mfcp_req_getval (rPtr, MFCP_PARAM_RESOURCE_ID));

    if (nsfGlueCloseResource (resourceId)) {
      mfcp_logit("Error from nsfGlueCloseResource");
      mfcp_send_failure(rPtr->sess,rPtr,400,"Error from nsfGlueCloseResource");
      return(MFCP_RET_NOTOK);
    }

    mfcp_res_add_param(rPtr, MFCP_PARAM_RESOURCE_ID,
		       mfcp_value_new_i(resourceId));

    break;
    /********************************* CRS ***********************************************/
  case MFCP_REQ_CRS:           /* create a session */
    break;
    /********************************* DLS ***********************************************/
  case MFCP_REQ_DLS:           /* delete a session */
    break;

  default:
    mfcp_send_failure(rPtr->sess,rPtr,400,"Unknown mfcp action");
    return(MFCP_RET_NOTOK);
  }

  /* call the callback or send the packet*/

  mfcp_sess_send_res(rPtr->sess,rPtr);


  return MFCP_RET_OK;
}


/****************************************************************************
 * Call back function for responses 
 * Once the request is completed this function will be called to 
 * either unlock the sync mutex for syn calls or call the user supplied 
 * function with the rPtr as a argument. 
 ***************************************************************************/

int
fc_doresponse(MFCP_Request *rPtr) {

  mfcp_req_display(rPtr, "fc_doresponse");
  if (rPtr->func) { /* if the func is a valid function then we assume is a async call */
    NETDEBUG(MFCE,NETLOG_DEBUG4,("fc_doresponse: call the callback\n"));
    (rPtr->func)((void *)rPtr);
  } else { /* Here if the request was a sync call */
    NETDEBUG(MFCE,NETLOG_DEBUG4,("fc_doresponse: unlock the request pointer mutex\n"));
    pthread_mutex_unlock(&rPtr->syncLock); /* let the caller continue */
  }

  return MFCP_RET_OK;
}







