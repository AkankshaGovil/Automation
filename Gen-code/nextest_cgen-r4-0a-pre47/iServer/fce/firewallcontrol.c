/**
 * this file contains code for firewall control
 */

#include <stdio.h>
#include <errno.h>
#include <pthread.h>
#include <unistd.h>
#include <stdlib.h>
#include <netdb.h>
#include <string.h>

#include "lock.h"
#include "srvrlog.h"
#include "lsconfig.h"
#include "firewallcontrol.h"
#include "nsfglue.h"
#include "fceutils.h"
#include "licenseIf.h"
#include "net.h"
#include "mfcp.h"
#include "fcemacro.h" /* For IS_NFS_FW */
#include "common.h"

#define TRUE	1
#define FALSE	0

/* maximum bundle id note: snow shore can only handle 16bit BUNDLE ids */
#define MAX_BUNDLE_ID  0xfffe

/* config parameters */
static char fwName[128] = {0};

static MFCP_Session *session;
static int lastBundleId = 0;
static int fceInited = FALSE;

extern unsigned int fceFirewallAddresses[40];
extern int     fceFirewallNumber;

/**
 * initialize the local config variables
 */
static void
VarInit ()
{
  strcpy(fwName, "none");
  session = NULL;
  lastBundleId = 0;
  fceInited = FALSE;
}


/**
 * this is the initialization method that needs to be called to start the
 * firewall control thread
 */
void
FCEStart (void)
{

  if (!fceEnabled())
    return; /* nothing to do, FCE license not enabled */

  if (strcmp(fceConfigFwName, "none") == 0)
    return; /* nothing to do, no firewall configured */

  if (fceInited == TRUE) {
    NETERROR(MFCE, ("FCEStart: fce already inited!!\n"));
    return;
  }

  /* initialize our local copy of the config variables */
  VarInit();

  /* store config variables into our local copy */
  strcpy(fwName, fceConfigFwName);

  
  // initialize firewall connections/specific function pointers and start FCE thread
  if( IS_LOCAL_FW )
    {
      mfcp_init();
      session = mfcp_sess_connect(0, 0, NULL);
      fceInited = TRUE;
      NETDEBUG(MFCE, NETLOG_DEBUG2, ("FCEStart: successfull to NSF\n"));
    } 
  else if (strcasecmp(fwName,"mfcp") == 0) 
    {
      mfcp_init();
      /* for now allow only one firewall, the first one specified */
      if ((session = mfcp_sess_connect(fceFirewallAddresses[0], 0, NULL)) != NULL) 
	{
	  fceInited = TRUE;
	  NETDEBUG(MFCE, NETLOG_DEBUG2, ("FCEStart: successfull to MFCP\n"));
	}
      else
	{
	  NETERROR(MFCE, ("FCEStart: Cannot create session to address 0x%0x\n", fceFirewallAddresses[0]));
	}
    }
  else
    {
      NETERROR(MFCE, ("FCEStart: unknown Firewall Name (%s), will not initialize FCE\n", fwName));
      VarInit();
    }
}

/**
 * this is to shutdown the firewall control thread, this gracefully closes all
 * holes it had opened up in the firewall
 */
void
FCEStop (void)
{
  /* shutdown firewall connections */
  if (fceInited == TRUE)
  {
    mfcp_sess_close(session);
    VarInit();
    NETDEBUG(MFCE, NETLOG_DEBUG2, ("FCEStop: successfull\n"));
  }
  else
  {
    NETDEBUG(MFCE, NETLOG_DEBUG4, ("FCEStop: Attempting to shutdown FCE - it never started\n"));
  }
}

/**
 * call if any of the configuration parameters have changed
 * i.e., ./iserver all reconfig is called
 *
 * existing pinholes would be cleared if some parameters have changed from their
 * previous values
 * 
 */
void
FCEReconfig (void)
{
  NETDEBUG(MFCE, NETLOG_DEBUG4, ("FCEReconfig: reconfig request received\n"));
  /* if thread already running, disturb him only if the config have changed */

  if (fceInited == FALSE) {
    NETDEBUG(MFCE, NETLOG_DEBUG4, ("FCEReconfig: configuring firewall\n"));
    FCEStart();
  } else if (!fceEnabled()) {
      FCEStop();
      return; /* nothing to do, FCE license not enabled */
  } else {
    mfcp_reconfig(session);
  }
}

/**
 * allocates and returns the next available bundle id that belongs to the given MFCP session
 */
unsigned int
FCEGetNextBundleId (MFCP_Session *sess)
{
  // TODO - when managing multiple session, use the session passed in
  // for allocating the bundleId

  // *********************************************************************
  // we are not locking here, because the caller (from bridgefw.c) always
  // has a lock before calling in here
  // *********************************************************************

  //  LockGetLock(&fceLock, 0, 0);

  lastBundleId++;

  /* potential bug, when rollover happens, this session ID could still be used
     by someone else */
  if ((lastBundleId <= 0) || (lastBundleId > MAX_BUNDLE_ID))
    lastBundleId = 1;

  //  LockReleaseLock(&fceLock);

  return lastBundleId;
}

void
FCEAdjustBundleId (unsigned int bid)
{
	lastBundleId = (lastBundleId > bid) ? lastBundleId : bid;
}

MFCP_Session *
FCEGetSession(void)
{
	return(session);
}

/**
 * allocates and assigns a unique bundleid to the given pointer and returns the MFCP session
 * pointer to which this bundleId belongs to
 */
MFCP_Session*
FCEAllocateBundleId (unsigned int *bundleId)
{
  *bundleId = 0;
  if (fceInited == FALSE)
  {
    NETERROR(MFCE, ("FCEAllocateBundleId: cannot allocate, FCE not inited\n"));
    return NULL;
  }

  *bundleId = FCEGetNextBundleId(session);

  return session;
}

static void
FCECloseBundleCallback (MFCP_Request *ptr)
{
  if (mfcp_get_res_status(ptr) == MFCP_RSTATUS_OK)
  {
    NETDEBUG(MFCE, NETLOG_DEBUG4, ("FCECloseBundleCallback: closed bundle %p\n", 
				mfcp_get_res_appdata(ptr)));
  }
  else
  {
    NETERROR(MFCE, ("FCECloseBundleCallback: unable to close bundle %p: %s\n", 
				mfcp_get_res_appdata(ptr), mfcp_get_res_estring(ptr)));
  }

  mfcp_req_free(ptr);
}

/**
 * closes the given bundle, logs an error if some error happens
 */
void
FCECloseBundle (MFCP_Session *sess, unsigned int bundleId)
{
  if (fceInited == TRUE)
  {
    if (mfcp_req_dlb(sess, bundleId, FCECloseBundleCallback, NX_INT_TO_POINTER(bundleId)) != MFCP_RET_OK)
    {
      NETERROR(MFCE, ("FCECloseBundle: unable to close bundle: %d\n", bundleId));
    }
  }
  else
  {
    NETDEBUG(MFCE, NETLOG_DEBUG4, ("FCECloseBundle: cannot close bundle %d, fce not inited\n", bundleId));
  }
}

/**
 * opens a resource asynchronously
 */
extern MFCP_Return FCEOpenResourceAsync (MFCP_Session *sess,
                                         int bundleId,
                                         int peerResourceId, 
                                         unsigned int saddr, unsigned short sport,
                                         unsigned int daddr, unsigned short dport,
                                         unsigned int nat_saddr, unsigned short nat_sport,
                                         unsigned int nat_daddr, unsigned short nat_dport,
                                         unsigned int ingressPool, unsigned int egressPool,
                                         char *protocol, int dtmf_detect, int dtmf_detect_param,
                                         unsigned int dst_sym, void (*callback)(void *), void *appData)
{
  if (fceInited == TRUE)
  {
    return mfcp_req_crr(sess,
		   				bundleId,
					   	peerResourceId,
					   	saddr,
					   	sport,
					   	daddr,
					   	dport,
					   	nat_saddr,
					   	nat_sport,
					   	nat_daddr,
					   	nat_dport,
					   	ingressPool,
					   	egressPool,
					   	protocol,
					   	dtmf_detect,
					   	dtmf_detect_param,
						dst_sym,
					   	ALLOCATE_CREATE,
					   	callback,
					   	appData);
  }

  NETDEBUG(MFCE, NETLOG_DEBUG4, ("FCEOpenResourceAsync: cannot open bundle %d, fce not inited\n", bundleId));
  return MFCP_RET_NOTOK;
}

/**
 * opens a resource synchronously
 */
extern MFCP_Request* FCEOpenResourceSync (MFCP_Session *sess,
                                          int bundleId,
                                          int peerResourceId, 
                                          unsigned int saddr, unsigned short sport,
                                          unsigned int daddr, unsigned short dport,
                                          unsigned int nat_saddr, unsigned short nat_sport,
                                          unsigned int nat_daddr, unsigned short nat_dport,
                                          unsigned int ingressPool, unsigned int egressPool,
                                          char *protocol, int dtmf_detect, int dtmf_detect_param,
					  unsigned int dst_sym)
{
  if (fceInited == TRUE)
  {
    return mfcp_sreq_crr(sess,
		   				bundleId,
					   	peerResourceId,
					   	saddr,
					   	sport,
					   	daddr,
					   	dport,
					   	saddr,
					   	sport,
					   	daddr,
					   	dport,
					   	ingressPool,
					   	egressPool,
					   	protocol,
					   	dtmf_detect,
					   	dtmf_detect_param,
						dst_sym);
  }

  NETDEBUG(MFCE, NETLOG_DEBUG4, ("FCEOpenResourceSync: cannot open bundle %d, fce not inited\n", bundleId));
  return NULL;
}

/**
 * allocates a resource asynchronously (doesn't open it)
 */
extern MFCP_Return FCEAllocResourceAsync (MFCP_Session *sess,
                                         int bundleId,
                                         int peerResourceId, 
                                         unsigned int saddr, unsigned short sport,
                                         unsigned int daddr, unsigned short dport,
                                         unsigned int nat_saddr, unsigned short nat_sport,
                                         unsigned int nat_daddr, unsigned short nat_dport,
                                         unsigned int ingressPool, unsigned int egressPool,
                                         char *protocol, int dtmf_detect, int dtmf_detect_param,
                                         unsigned int dst_sym, void (*callback)(void *), void *appData)
{
  if (fceInited == TRUE)
  {
    return mfcp_req_crr(sess,
		   				bundleId,
					   	peerResourceId,
					   	saddr,
					   	sport,
					   	daddr, 
						dport, 
					   	nat_saddr,
					   	nat_sport,
					   	nat_daddr, 
						nat_dport, 
						ingressPool,
					   	egressPool,
					   	protocol,
					   	dtmf_detect,
					   	dtmf_detect_param,
						dst_sym,
					   	ALLOCATE_ONLY,
					   	callback,
					   	appData);
  }

  NETDEBUG(MFCE, NETLOG_DEBUG4, ("FCEAllocResourceAsync: cannot open bundle %d, fce not inited\n", bundleId));
  return MFCP_RET_NOTOK;
}

/**
 * creates a resource asynchronously (which has been allocated before)
 */
extern MFCP_Return FCECreateResourceAsync (MFCP_Session *sess,
                                         int bundleId,
                                         int resourceId, 
                                         int peerResourceId, 
                                         unsigned int saddr, unsigned short sport,
                                         unsigned int daddr, unsigned short dport,
                                         unsigned int nat_saddr, unsigned short nat_sport,
                                         unsigned int nat_daddr, unsigned short nat_dport,
                                         unsigned int ingressPool, unsigned int egressPool,
                                         char *protocol, int dtmf_detect, int dtmf_detect_param,
                                         unsigned int dst_sym, void (*callback)(void *), void *appData)
{
  if (fceInited == TRUE)
  {
    return mfcp_req_chr(sess,
					   	bundleId,
		  			 	resourceId,
		  			 	peerResourceId,
					   	saddr,
					   	sport,
					   	daddr,
					   	dport,
					   	nat_saddr,
					   	nat_sport,
					   	nat_daddr,
					   	nat_dport,
					   	ingressPool,
					   	egressPool,
					   	protocol,
					   	dtmf_detect,
					   	dtmf_detect_param,
					   	CREATE_ONLY,
					   	callback,
					   	appData);
  }

  NETDEBUG(MFCE, NETLOG_DEBUG4, ("FCECreateResourceAsync: cannot create bundle %d, fce not inited\n", bundleId));
  return MFCP_RET_NOTOK;
}


/*
 * modifies a resource asynchronously
 */
extern MFCP_Return FCEModifyResourceAsync (MFCP_Session *sess,
                                           int bundleId,
                                           int resourceId, 
                                           int peerResourceId, 
                                           unsigned int saddr, unsigned short sport,
                                           unsigned int daddr, unsigned short dport, 
                                           unsigned int nat_saddr, unsigned short nat_sport,
                                           unsigned int nat_daddr, unsigned short nat_dport, 
                                           unsigned int ingressPool, unsigned int egressPool,
                                           char *protocol,  int dtmf_detect, int dtmf_detect_param,
                                           unsigned int dst_sym, void (*callback)(void *), void *appData)
{
  if (fceInited == TRUE)
  {
    return mfcp_req_chr(sess,
					   	bundleId,
		  			 	resourceId,
		  			 	peerResourceId,
					   	saddr,
					   	sport,
					   	daddr,
					   	dport,
					   	nat_saddr,
					   	nat_sport,
					   	nat_daddr,
					   	nat_dport,
					   	ingressPool,
					   	egressPool,
					   	protocol,
					   	dtmf_detect,
					   	dtmf_detect_param,
					   	ALLOCATE_CREATE,
					   	callback,
					   	appData);
  }

  NETDEBUG(MFCE, NETLOG_DEBUG4, ("FCEModifyResourceAsync: cannot modify bundle %d, fce not inited\n", bundleId));
  return MFCP_RET_NOTOK;
}

/*
 * modifies a resource synchronously
 */
extern MFCP_Request* FCEModifyResourceSync (MFCP_Session *sess, 
                                            int bundleId,
                                            int resourceId, 
                                            int peerResourceId, 
                                            unsigned int saddr, unsigned short sport,
                                            unsigned int daddr, unsigned short dport, 
                                            unsigned int nat_saddr, unsigned short nat_sport,
                                            unsigned int nat_daddr, unsigned short nat_dport, 
                                            unsigned int ingressPool, unsigned int egressPool,
                                            char *protocol, int dtmf_detect, int dtmf_detect_param,
											unsigned int dst_sym)
{
  if (fceInited == TRUE)
  {
    return mfcp_sreq_chr(sess,
					   	bundleId,
                                            	resourceId, 
                                            	peerResourceId, 
					   	saddr,
					   	sport,
					   	daddr,
					   	dport,
					   	nat_saddr,
					   	nat_sport,
					   	nat_daddr,
					   	nat_dport,
					   	ingressPool,
					   	egressPool,
					   	protocol,
					   	dtmf_detect,
					   	dtmf_detect_param);
  }

  NETDEBUG(MFCE, NETLOG_DEBUG4, ("FCEModifyResourceSync: cannot modify bundle %d, fce not inited\n", bundleId));
  return NULL;
}

static void
FCECloseResourceCallback (MFCP_Request *ptr)
{
  if (mfcp_get_res_status(ptr) == MFCP_RSTATUS_OK)
  {
    NETDEBUG(MFCE, NETLOG_DEBUG4, ("FCECloseResourceCallback: closed resource %d\n", mfcp_get_int(ptr, MFCP_PARAM_RESOURCE_ID)));
  }
  else
  {
    NETERROR(MFCE, ("FCECloseResourceCallback: unable to close resource %d: %s\n", mfcp_get_int(ptr, MFCP_PARAM_RESOURCE_ID), mfcp_get_res_estring(ptr)));
  }

  mfcp_req_free(ptr);
}

/**
 * closes the given resource, logs an error if some error happens
 */
void
FCECloseResource (MFCP_Session *sess, unsigned int resourceId)
{
  if (fceInited == TRUE)
  {
    if (mfcp_req_dlr(sess, 0, resourceId, FCECloseResourceCallback, NULL) != MFCP_RET_OK)
    {
      NETERROR(MFCE, ("FCECloseResource: unable to close resource: %d\n", resourceId));
    }
  }
  else
  {
    NETDEBUG(MFCE, NETLOG_DEBUG4, ("FCECloseResource: cannot close resource %d, fce not inited\n", resourceId));
  }
}

/**
 * to check if the FCE is enabled or not
 *
 * @return TRUE if FCE is enabled and operational or FALSE if not
 */
int
IsFCEEnabled ()
{
  return fceInited;
}

/**
 * this is the callback called when a redundant iserver switches over and
 * becomes inactive (VIP goes down)
 */
void FCEServerInactive ()
{
  if (IsFCEEnabled() == TRUE)
  {
    NETINFOMSG(MFCE, ("FCEServerInactive: Indicating to the firewalls that the server is now inactive\n"));
    //    (*ServerInactiveFunc)();
  }
}


/**
 * this is the callback called when a redundant iserver switches over and
 * becomes active (VIP is up)
 */
void FCEServerActive ()
{
  if (IsFCEEnabled() == TRUE)
  {
    NETINFOMSG(MFCE, ("FCEServerActive: Indicating to the firewalls that the server is now active\n"));
    //    (*ServerActiveFunc)();
  }
}
