#ifndef _GK_H_
#define _GK_H_

#include "gis.h"
#include "sccdefs.h"
#include "uh323inc.h"
#include "calldefs.h"
#include "sipcall.h"

/* This file has prototypes of faststart.c uh235.c uh323call.c rrq.c  
** callsmEventProcessor.c handleSetup.c callsmactions.c.
*/

/************************* faststart.c**************************************/
int ChannelCodec(char *name);
int createDataHandle(const H323RTPSet *pRtpSet);
int uh323SetFastStartParam(CallHandle *callHandle, RTPSet rtpSet[],int nrtpset);
int uh323CapSend(HCALL hsCall,H323RTPSet *pRtpSet,int ncodecs,int nodeId,
		                int ecaps1, int vendor);
int uh323NullCapSend2(HCALL hsCall);
int createDataHandleOLC(const H323RTPSet *pRtpSet);
void uh323FastStartSetupInit(const CallHandle *callHandle);
int uh323ExtractFSAudioRtpSet(H323RTPSet *pRtpSet,
		                int *pcount,cmFastStartChannel fsChannel[]);
int getCodecParam(int dataTypeHandle,int *pCodecType,int *pParam, int *pSS);
int getDataHandle(const H323RTPSet *pRtpSet);
int findNonStandardCodec(HPVT hVal, int nonsNodeId);
int toH323Direction(int direction);
int getLucentFaxDataHandle();
int getFaxDataHandle();
int getDtmfDataHandle();
int getDtmfStringDataHandle();
int getRfc2833DataHandle(void);
int createCodecTree(const H323RTPSet *pRtpSet,HPVT hVal,int nodeId);
int addFacilityFastStart(int nodeId, int openMsgId);
int decodeFacilityFastStartMsg(IN  HCALL hsCall, IN  int message,
		IN OUT cmFastStartMessage *fsMessage,IN OUT INT16 *lcnOut);
RVAPI int RVCALLCONV sendFacilityFastStart(IN HAPPCALL haCall, IN HCALL hsCall, 
		IN cmFastStartMessage *fsMessage, IN INT16 lcn);

/************************* faststart.c**************************************/


/************************* uh235.c *****************************************/
void uh235Init(void);
int getSetupTokenNodeId(HCALL hsCall);
int getLcfTokenNodeId(HRAS hsRas);
int grqAddAuthentication(HRAS hsRas);
int rrqAddAuthentication(HRAS hsRas,char senId[],char pass[]);
int arqAddAuthentication(HRAS hsRas,char senId[],char pass[]);
int getAcfTokenNodeId(HRAS hsRas);
int drqAddAuthentication(HRAS hsRas,char senId[],char pass[],CallHandle *callHandle);
int irrAddAuthentication(int nodeId,char senId[],char pass[]);
int __stringToBMP(const char* str,char* bmpStr);
void mkHash(long timeStamp,int genIDlen,char generalID[],int pwdlength,
		                char passwd[], unsigned char hash[]);
void getVTData179(char *callingpn,int *datalen,char databuff[]);

/************************* uh235.c *****************************************/


/************************* uh323call.c *************************************/
int GkCallExtractCallerPhone(HCALL hsCall, char *phone, int phoneLen, 
				unsigned char *cgpnType);
int GkOpenChannel(CallHandle *callHandle, ChanInfo *chanInfo,int sid);
int GkSetChannelAddress(CallHandle *callHandle, unsigned long ip,
		                unsigned short port, int sid);
int CallSetEvtFromH323StackState(int state, int origin, int *pevt);
int BillCallDropInferReason(HCALL   hsCall, CallHandle *callHandle,
		                int stateMode, cmReasonType reason, int cause);
int GkInitChannelDataType(CallHandle *callHandle, BOOL origin,
		                int dataTypeHandle, cmCapDataType dataType, int param,
				                int sid, int flags);
int GkInitChannelRTCPAddress(CallHandle *callHandle, BOOL origin,
		                unsigned long ip, unsigned short port, int sid);
int GkInitChannelHandle(CallHandle *callHandle, BOOL origin, HCHAN hsChannel,
		                int sid);
int cause2SCCError(int cause);
int BillCallDropReason(cmReasonType reason);

/************************* uh323call.c *************************************/


/************************* rrq.c *******************************************/
int GkHandleGCF(IN HAPPRAS haRas, IN HRAS hsRas);
int GkHandleRCF(IN HAPPRAS haRas, IN HRAS hsRas);
int GkHandleRAIResponse(IN HAPPRAS haRas, IN HRAS hsRas, char *msg);
int GkHandleGRJ(IN HAPPRAS haRas, IN HRAS hsRas, IN cmRASReason reason);
int GkHandleRRJ(IN HAPPRAS haRas, IN HRAS hsRas, IN cmRASReason reason);
int GkHandleGRQ(IN HRAS hsRas, IN HCALL hsCall, OUT LPHAPPRAS lphaRas,
		                IN cmTransportAddress *srcAddress, IN HAPPCALL haCall);
int GkHandleRRQ(IN HRAS hsRas,IN HCALL hsCall, OUT LPHAPPRAS lphaRas,
		                IN cmTransportAddress *srcAddress, IN HAPPCALL haCall);
int GkHandleURQ(IN HRAS hsRas, IN HCALL hsCall, OUT LPHAPPRAS lphaRas,
		                IN cmTransportAddress *srcAddress, IN HAPPCALL haCall);
int GkHandleRAI(IN HRAS hsRas, IN HCALL hsCall, OUT LPHAPPRAS lphaRas,
		                IN cmTransportAddress *srcAddress, IN HAPPCALL haCall);
int GkHandleBRQ(IN HRAS hsRas, IN HCALL hsCall, OUT LPHAPPRAS lphaRas,
		                IN cmTransportAddress *srcAddress, IN HAPPCALL haCall);
int GkHandleGkURQ(IN HRAS hsRas, IN cmTransportAddress *srcAddress);
int GkComputeSchedule(GkInfo *gkInfo);
int GkInsertGkID(IN HRAS hsRas, cmRASTrStage trstage, char *gkid);
int GkCompareGkID(HRAS hsRas);
int GkCheckAltGk(CacheGkInfo *gkInfo, int looparound);
int GkScheduled(GkInfo *gkInfo);
int GkSendURQ(NetoidInfoEntry *entry);

/************************* rrq.c *******************************************/


/************************* callsmEventProcessor.c **************************/
int SCC_QueueEvent(SCC_EventBlock *eventBlockPtr);
int SCC_DelegateEvent(SCC_EventBlock *eventBlockPtr);
int SCC_BridgeEventWorker(SCC_EventBlock *evtPtr);
int SCC_DelegateEventWorker(SCC_EventBlock *eventBlockPtr);
/************************* callsmEventProcessor.c **************************/


/************************* handleSetup.c ***********************************/
int GkAdmitCallFromSetup(HCALL hsCall, HAPPCALL haCall);

/************************* handleSetup.c ***********************************/


/************************* uh323callcb.c ***********************************/
int LogCalledLeg(IN HAPPCALL haCall, IN HCALL hsCall, char *fn);
RVAPI int RVCALLCONV sendFacilityFastStartOpenChannel(IN HAPPCALL haCall,
		IN HCALL hsCall, IN INT16 lcn);
RVAPI int RVCALLCONV sendFacilityFastStartCloseChannel(IN HAPPCALL haCall,
		IN HCALL hsCall, IN INT16 lcn);

/************************* uh323callcb.c ***********************************/


/************************* h323smutils.c ***********************************/
void H323FreeEvData(H323EventData *ptr);
void H323FreeEvent(SCC_EventBlock *evtPtr);
int h323HuntError(int callError,int cause);
int ise164(char phone[PHONE_NUM_LEN]);

/************************* h323smutils.c ***********************************/


/************************* callsmactions.c *********************************/
void saveEgressH323Id(char *confID, char *egressH323Id);
int SCC_CallStateIdle(SCC_EventBlock *evtPtr);
int SCCQueueEvent(SCC_EventBlock *evtPtr);
int updateNonStandardCodecType(CallHandle *callHandle, int sid, int isOut);
int processPendingEvents (CallHandle *callHandle);

/************************* callsmactions.c *********************************/

/************************* arq.c *******************************************/
int GkAdmitCall (HRAS hsRas, HCALL hsCall, ARQHandle *arqHandle, int newCallHandle,
		                int answerCall, CacheTableInfo *scacheInfo);
int GkSendARQ (PhoNode *rfphonodep, ARQHandle *arqHandle2, void *data, char *id);
int GkSendDRQ (ARQHandle *arqHandle);
int GkFindSrcForArq (HRAS hsRas, HCALL hsCall, int answerCall, int newCallHandle, 
		ARQHandle *arqHandle, CacheTableInfo *scacheInfo);

/************************* arq.c *******************************************/





/************************* finwait.c *******************************************/
void CleanHalfClose(void);

void initWaitSdEntry(int maxfds);

/************************* callsmEventProcessor.c *****************************/

int SCC_Init();
	



#endif /* _GK_H_ */ 
