#ifndef _usipif_h_
#define	_usipif_h_

/* First include sip stack header files */

#include "portlayer.h"
#include "sipdecode.h"
#include "sipstruct.h"
#include "sipsendmessage.h"
#include "siplist.h"
#include "sipinit.h"
#include "sipfree.h"
#include "general.h"
#include "request.h"
#include "response.h"
#include "entity.h"
#include "ccp.h"
#include "rpr.h"
#include "header.h"
#include "sipstatistics.h"
#include "siptimer.h"
#include "siptrace.h"

#include "connP.h"
#include "smP.h"
#include "usipsmp.h"

extern char sipdomainname[];
#define SipGetLocalDomain() (sipdomainname)

#define SIP_REGISTRAR_PORT 5060

/* Other prototypes */
int smSIPStartCall(UCC_EventBlock *event);
int smSIPPrintCaller (UCC_EventBlock * event);
int smSIPReceiveInvite(UCC_EventBlock * event);
int smSIPRingFxoOrFxs (UCC_EventBlock * event);
int smSIPProcess1xxInfo(UCC_EventBlock * event);
int smSIPProcess180Ringing(UCC_EventBlock * event);
int smSIPProcess183SessionProgress(UCC_EventBlock * event);
int smSIPCallConnect(UCC_EventBlock * event);
int smSIPSetCurrentParty(UCC_EventBlock * event);
int smSIPProcess200Success(UCC_EventBlock * event);
int smSIPSendAck(UCC_EventBlock * event);
int smSIPProcessAck(UCC_EventBlock * event);
int smSIPSendBye(UCC_EventBlock * event);
int smSIPCallClose2(PartyEntry *party);
int smSIPCallClose(UCC_EventBlock * event);
int smSIPProcessCancel(UCC_EventBlock * event);
int smSIPProcessBye(UCC_EventBlock * event);
int smSIPRejectInvite(UCC_EventBlock * event);
int smSIPProcess3xxRedirect(UCC_EventBlock * event);
int smSIPProcessRequestFailure(UCC_EventBlock * event);
int smSIPProcess30xMovedOrProxy(UCC_EventBlock * event);
int smSIPHandleTransfer(UCC_EventBlock * event);
int smSIPHandleHoldPressed(UCC_EventBlock * event);
int smSIPHandleUnHold(UCC_EventBlock * event);
int smSIPHandleHoldOrig(UCC_EventBlock * event);
int smSIPProcessInvite(UCC_EventBlock * event);
void sip_indicateInvite ( SipMessage *s, SipEventContext *context );
void sip_indicateRegister(SipMessage *s, SipEventContext *context) ;
void sip_indicateCancel(SipMessage *s, SipEventContext *context) ;
void sip_indicateOptions(SipMessage *s, SipEventContext *context) ;
void sip_indicateBye(SipMessage *s, SipEventContext *context) ;
void sip_indicateAck(SipMessage *s, SipEventContext *context) ;
void sip_indicateUnknownRequest(SipMessage *s, SipEventContext *context) ;
void sip_indicateInformational(SipMessage *s, SipEventContext *context) ;
void sip_indicateFinalResponse(SipMessage *s, SipEventContext *context) ;
void sip_indicateTimeOut( SipEventContext *context);
void sip_indicateInfo(SipMessage *message, SipEventContext *context);
void sip_indicatePropose(SipMessage *message, SipEventContext *context);
void sip_indicatePrack(SipMessage *message, SipEventContext *context);
SipBool sip_formInvite(SipMessage **invmsg, UCC_EventBlock *event, SipError *err);
SipBool sip_form_reqline_inrequest(SipMessage *msg, PartyEntry * party, char * method, SipError *err);
SipBool sip_form_tohdr_inmsg(SipMessage *msg, header_url *to, SipError *err);
SipBool sip_form_fromhdr_inmsg(SipMessage *msg, header_url *from, SipError *err);
SipBool sip_form_viahdr_inreq(SipMessage *m, int protocol, SipError *err);
SipBool sip_form_callid_inmsg(SipMessage *msg, SIP_S8bit *callid, SipError *err);
SipBool sip_form_cseqnum_inmsg(SipMessage *msg, int seqnum, char * method, SipError *err);
SipBool sip_form_contacthdr_inmsg(SipMessage *msg, header_url *contact, int port, SipError *err);
SipBool sip_form_contenttypehdr_inmsg(SipMessage *m, SipError *err);
SipBool sip_form_sdpmessage(SdpMessage *sdp_message1, int type, PartyEntry * party, SipError * err);
SipBool sip_form_sdpmsg_inmsg(SipMessage *m, int type, PartyEntry * party, SipError *err);
SipBool sip_form_sdpmedia_inmsg(SdpMessage *sdp_message , int type, PartyEntry * party, SipError *err);
SipBool sdp_insert_media_attributes(SdpMedia * media, int codec_type);
SipBool sip_send_ack(SipMessage *reqmsg, PartyEntry *party);
SipBool sip_form_reqline_inAck(SipMessage * req, SipMessage * resp, header_url * req_uri, SipError * err);
SipBool sip_form_cseqnum_inAck(SipMessage * req, SipMessage * resp, SipError * err);
SipBool sip_send_bye(PartyEntry *party);
SipBool sip_form_response_body ( SipMessage *s, char * method, SIP_U32bit status_code, SIP_S8bit *reason, SipMessage *sip_response, PartyEntry * party, SipError *err);
SipBool form_basic_headers( SipMessage *s, SipMessage *sip_response, char * method, PartyEntry * party, SipError *err);
SipBool sip_setProperValueInTranspAddr(SipMessage *message, PartyEntry * party, SipError *err);
SipBool sip_add_sdpdescription(SipMessage * sip_response, PartyEntry * party, SipError * err);
SipBool sip_send_sipresponse(SipMessage * msg, char * method, int status_code, char * reason, PartyEntry * party);
int USIP_Init (void);
int sip_InitNet (void);
int GetSipControlFd (void);
int GetSipSigFd (void);
int CloseSipControlFd (void);
int CloseSipSigFd (void);
int sip_SigConnReceive (int csock, FD_MODE rw, void *data);
int sip_ControlPacketReceive (int csock, FD_MODE rw, void *data);
int sipInitiateControl(UCC_EventBlock  * event);
int SIP_ConnConnected(int rc, void *data);
int SIP_ConnErrorOrTimeout(int rc, void *data);
int SendSipRequest (PartyEntry *party);
SIPTimerEntry * SipSearchTimerInList(SipTimerKey * key);
int sipTimerExpiry(tid t);
SipBool ExtractFromToAndReqUriHeaders(SipMessage *m, header_url **to, header_url **from, header_url **req_uri, SipError *err);
SipBool GetSipCallId(SipMessage *s, SIP_S8bit **callId, SipError *err) ;
SipBool GetSipCseq(SipMessage *s, SIP_U32bit *cseq, SipError *err) ;
SipBool GetSipCseqMethod(SipMessage *s, char **method, SipError *err) ;
SipBool SipInitUrlInParty(header_url **url, char *name, char * host);
SipBool SipInitRemoteAddrInParty(SipTranspAddr *raddr, char *addr, int port);
PartyEntry * SearchPartyByCallIdOnPort(List list, char * callid);
PartyEntry * SearchPartyByCallIdOnPortAndFromTo(List list, char * callid, header_url * from, header_url * to);
PartyEntry * SearchPartyListByCallId(char * callid);
PartyEntry * SearchPartyListByCallIdAndFromTo(char * callid, header_url * from, header_url * to);
SipBool SipDetermineSdpParams(SipMessage *msg, SipSdpParams * params);
SipBool SipProcessContact(SipMessage * msg, PartyEntry * party);
UCC_SIPData * GetNewSIPCallData(void);
void sipFreePartyData(PartyEntry * party);
SipBool ExtractFromAndToHeaders(SipMessage *m, header_url **to, header_url **from, SipError *err);
int print_sip_timer_key(SipTimerKey * key);
int SipCompareUrlNameandHost(header_url * src, header_url * dst);
void sip_delete_timer_from_list(char * callid, char * method);
SipBool form_response_basic_headers( SipMessage *s, SipMessage *sip_response, PartyEntry * party, SipError *err);
int smSipFreeMessage(UCC_EventBlock * event);
SipBool sip_send_cancel(PartyEntry *party);
int smSIPSendCancel(UCC_EventBlock * event);
void sip_getconfiguredcodecs(char * codecptr);

PartyEntry *SipSearchRegistrations(char *callID);
SipBool sip_form_record_route_inmsg(SipMessage * respmsg, SipMessage * reqmsg, SipError * err);
SipBool sip_store_route(SipMessage * msg, PartyEntry * party, SipError * err);
SipBool sip_set_route_inmsg(SipMessage * msg, PartyEntry * party, SipError * err);
SipBool sip_set_remote_party_context(PartyEntry * party, SipAddrSpec * addrspec);
SipBool sip_getmaddr_fromaddrspec(PartyEntry * party, SipAddrSpec * addrspec);

#endif /* _usipif_h_ */
