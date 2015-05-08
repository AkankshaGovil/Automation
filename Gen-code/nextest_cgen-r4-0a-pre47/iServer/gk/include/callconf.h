#ifndef _callconf_h_
#define _callconf_h_

#include "calldefs.h"


typedef struct
{
	char 			confID[CONF_ID_LEN];
	time_t			iTime, rTime;

	int			ncalls;
	char			callID[MAX_CONF_CALLS][CALL_ID_LEN];
	SCC_CallState		state; //bridge state
	ConfType			confType; // Conference type sip2h323, sip2sip etc 
	int					subState; // sub state with in the state
	int					earlyH245Connected; // set on successfully completing 245 Channel
											// in early h245 procedure
	int					mediaOnHold;
#define	MEDIA_ON_HOLD_REINV_TX  1
#define	MEDIA_ON_HOLD_200Ok_RX	2
#define MEDIA_RESUME_200Ok_RX   3
#define MEDIA_RESUME_OLCACK_RX	4
	SCC_EventBlock		*sipEvt;/* pending event */	
	List				h323EvtList; 	/* FIFO queue of pending H323 events */
#define IWF_H323QUEUE_SIZE 			5 	/* Queue size for H323 events  */		
	
	int			mediaRouted;  // this call is media routed
#define CONF_MEDIA_TYPE_VOICE		0
#define CONF_MEDIA_TYPE_VIDEO		1
#define CONF_MEDIA_TYPE_DATA		2
	int			h323MediaType;	// 0 - voice, 1 - video, 2 - data. Default voice

	int			inviteNoSDP;	// Set if SIP invite has no SDP
	int			olcFromCisco;	// Set if OLC is from Cisco with media port 4000

	char		h323ConfId[GUID_LEN];	// 16 byte globally unique H323 conf ID

	// The following fields are stored here because they
	// need to be maintained through hunts to alternate endpoints.
	// They are sent in the Setup to an alternate endpoint.

	char        egressH323Id[H323ID_LEN];	// Our H323-ID as configured on GK 
	int			egressTokenNodeId;	// Egress ACF/LCF token node ID 
	int			setupQ931NodeId;	// For passing bearer capability if we hunt

} ConfHandle;

#define ConfSetConfID(conf, _confID_)	(memcpy((conf)->confID, \
		_confID_, CONF_ID_LEN))

struct CallHandleStruct;

ConfHandle * GisAllocConfHandle(void);
void GisFreeConfHandle(void *ptr);
int lockConfHandle(ConfHandle *cfHandle);
int unlockConfHandle(ConfHandle *cfHandle);
//int lockConfHandle(char confID[CONF_ID_LEN);
//int unlockConfHandle(char confID[CONF_ID_LEN);
int GisDeleteCallFromConf(char *callID, char *confID);
#define GisAddCallToConf(ch) GisAddCallToConfGetConf(ch, (ConfHandle **) NULL)
int GisAddCallToConfGetConf(struct CallHandleStruct *callHandle, ConfHandle **confHandlep);
void GisDisableConfHandle(void *ptr);
#endif /* _callconf_h_ */
