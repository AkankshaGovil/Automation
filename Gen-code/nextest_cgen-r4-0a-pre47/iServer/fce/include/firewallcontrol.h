#ifndef __FIREWALLCONTROL_H
#define __FIREWALLCONTROL_H

#include "mfcp.h"

/**
 * This header file contains the API definitions for MFCE session management
 * and bundleId management
 */


#define FCE_ANY_IP      0
#define FCE_ANY_PORT    0


/**
 * this is the startup method that needs to be called to initialize the
 * firewall control and MFCP
 */
extern void FCEStart (void);

/**
 * this is to shutdown the firewall control thread, this gracefully closes all
 * holes it had opened up in the firewall
 */
extern void FCEStop (void);

/**
 * call if any of the configuration parameters have changed
 * i.e., ./iserver all reconfig is called
 *
 * existing pinholes could be cleared if some parameters have changed from their
 * previous values
 */
extern void FCEReconfig (void);

/**
 * allocates and assigns a unique bundleid to the given pointer and returns the MFCP session
 * pointer to which this bundleId belongs to
 */
extern MFCP_Session* FCEAllocateBundleId (unsigned int *bundleId);

extern MFCP_Session *FCEGetSession(void);

/**
 * allocates and returns the next available bundle id that belongs to the given MFCP session
 */
extern unsigned int FCEGetNextBundleId (MFCP_Session *sess);

void FCEAdjustBundleId(unsigned int bid);

/**
 * closes the given bundle, logs an error if some error happens
 */
extern void FCECloseBundle (MFCP_Session *sess, unsigned int bundleId);

/**
 * Allocates a resource asynchronously
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
                                         unsigned int dst_sym, void (*callback)(void *), void *appData);

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
                                         unsigned int dst_sym, void (*callback)(void *), void *appData);

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
					  unsigned int dst_sym);

/*
 * creates a resource asynchronously
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
                                           unsigned int dst_sym, void (*callback)(void *), void *appData);

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
                                           char *protocol, int dtmf_detect, int dtmf_detect_param,
                                           unsigned int dst_sym, void (*callback)(void *), void *appData);

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
											unsigned int dst_sym);

/**
 * closes the given resource, logs an error if some error happens
 */
extern void FCECloseResource (MFCP_Session *sess, unsigned int resourceId);

/**
 * to check if the FCE is enabled or not
 *
 * @return TRUE if FCE is enabled and operational or FALSE if not
 */
extern int IsFCEEnabled (void);

/**
 * this is the callback called when a redundant iserver switches over and
 * becomes inactive (VIP goes down)
 */
extern void FCEServerInactive (void);

/**
 * this is the callback called when a redundant iserver switches over and
 * becomes active (VIP is up)
 */
extern void FCEServerActive (void);

#endif
