#ifndef _siputils_h_
#define _siputils_h_

SipEventHandle * SipAllocEventHandle(void);

SipMsgHandle * SipAllocMsgHandle(void);

int
SipUASetSipMsgHandleForMsg(SipMessage *s, SipMsgHandle *handle);

int
SipUAGetAppCallIDForMsg(SipMsgHandle *handle, char *callID);

SipMsgHandle *
SipBridgeCreateMsgHandle(CallHandle *callHandle, char *, int, int);

CallHandle *
SipBridgeCreateCallHandle(SipEventHandle *evb);

int
SipBridgeInitCallLeg(SipAppMsgHandle *appMsgHandle, SipCallLegKey *leg);

int
SipBridgeInitCallHandle(SipEventHandle *evHandle, CallHandle *callHandle);

CallHandle * SipBridgeUpdateCallHandle(SipEventHandle *evb);

char *SipGenerateCallID(char *, void *(*malloc)(size_t));

CallHandle *
SipNetworkCreateCallHandle(SipEventHandle *evb);

int
SipNetworkInitCallLeg(SipMsgHandle *msgHandle, SipCallLegKey *leg);

int
SipNetworkInitCallHandle(SipEventHandle *evHandle, CallHandle *callHandle);

CallHandle * SipNetworkUpdateCallHandle(SipEventHandle *evb);

char * GetSipState(int state);

char * GetSipEvent(int type, int event);

CallHandle * SipInitAppCallHandleForEvent(SipEventHandle *evb);

int SipCheckFree(void *m);

int SipCheckFree2(void *m, int freefn);

int SipFreeUrl(header_url *url);

header_url * UrlAlloc(int mallocfn);
int UrlFree(header_url *url, int freefn);

header_url * UrlDup(header_url *url, int mallocfn);
SipAppMsgHandle *SipAllocAppMsgHandle(void);

SipEventHandle * SipEventHandleDup(SipEventHandle *evb);

char * SipXConnIdFromCallID(char *callID);

char * SipMediaTypeAsStr(int mediaType);

char * SipTransportAsStr(int transport);

int SipMediaFmtToCodec(char *fmt);

SipBool SetSipCustomHeader(SipMessage *s, SIP_S8bit *name, SIP_S8bit *body, SipError *err);

SipBool GetSipCustomHeader(SipMessage *s, SIP_S8bit *name, SIP_S8bit **body, SipError *err);

SipBool SipExtractContactList(SipMessage *m, header_url_list **contact_url_list, int realmId, SipError *err);

header_url * SipPopUrlFromContactList(header_url_list **contact_url_list, int freefn);

SipAppMsgHandle * SipCreateAppMsgHandleForCall(CallHandle *callHandle, header_url *newrequri);
int SipFreeSipCallHandle(SipCallHandle *sipCallHandle, int freefn);

// Privacy Related Utility Functions

SipBool SipExtractFromPAssertedID( SipMessage *m, header_url **pAssertedID, int hdrCount, SipError *err);
SipBool SipExtractFromPrivacy( SipMessage *m, char **priv_value, SipError *err);
SipBool SipIsHdrTypePresent(SipMessage* msg, int hdrType, int* count, SipError* err);
SipBool SipIsPAssertedIDPresent(SipMessage* msg,int* count, SipError* err);
SipBool SipIsPrivacyPresent(SipMessage* msg,int* count, SipError* err) ;
SipBool SipIsRemotePartyIdPresent(SipMessage* msg,int* count, SipError* err);
SipBool SipIsProxyRequirePresent(SipMessage* msg,int* count, SipError* err);
SipBool SipConvertRFC3325ToDraft01Hdrs(SipMessage *s, header_url* pAssertedID_Sip, SipPrivacyLevel privacy_level, SipError* err);
SipBool SipSetProxyRequire(SipMessage* s, SipError* err);
void SipPrivacyModifyMsgHandle(SipMessage* s,SipAppMsgHandle* appMsgHandle,header_url* from);
void PrivacyTranslateHdrs(SipMessage* m, SipAppMsgHandle* AppMsgHandle);
SipBool SipCopyRFC3325HdrsToDestination(SipMessage* m, SipAppMsgHandle* appMsgHandle,SipError* err);
SipBool SipCopyRFC3325PAIHdrToDestination (SipMessage* m, SipAppMsgHandle* appMsgHandle,SipError* err);
SipBool SipCopyRFC3325PrivacyHdrToDestination (SipMessage* m, char* priv_value);
SipBool SipCopyDraft01HdrToDestination(SipMessage* m, SipAppMsgHandle*appMsgHandle, SipError* err);
SipBool SipStoreRemotePartyHdr(SipMessage* m, char** rpid_hdr, SipError* err);
SipBool SipStoreProxyRequireHdr(SipMessage* m, char** rpid_hdr, SipError* err) ;
SipBool SipGetGenericHeader(SipMessage* m, char** strptr,int hdrtype,SipError* err);
SipBool SipExtractFromRemotePartyHdr(SipMessage* m, header_url** rpid_hdr, SipError* err);
SipBool SipStorePAssertedIDTel(SipMessage* m, char** pAssertedIDTel, int hdrCount, SipError* err);
void SipFormFromHdr(SipAppMsgHandle* appMsgHandle,header_url* from);
void SipGeneratePrivacyHdr(SipMessage*,SipAppMsgHandle*); 
SipBool CopyHdrToPAI(SipMessage*m, header_url*,int);
SipBool CopyHdrToRPID(SipMessage*m, header_url*, SipPrivacyLevel, SipError*);
SipBool SipExtractReqUri(SipMessage *m, header_url **req_uri, SipError *err);
SipBool SipExtractFromUri(SipMessage *m, header_url **from_uri, SipError *err);
SipBool SipExtractToUri(SipMessage *m, header_url **to, SipError *err);
SipBool SipGetCallID(SipMessage *m, char **callID, SipError *err);
SipBool SipIsContactWildCard(SipMessage *m, SipError *err);
SipBool SipExtractContact(SipMessage *m, header_url **contact_url, SipError *err);
SipBool SipGetCSeq(SipMessage *m, SIP_U32bit *seqnum, char **method, SipError *err);
SipBool SipExtractReferToUri(SipMessage *m, header_url **referto, SipError *err);
SipBool SipExtractReferByUri(SipMessage *m, header_url **referby, SipError *err);


void SipCreateEncryptAttrib(SDPAttr *attrib, char *value);
void SipCreateBandwidthAttrib(SDPAttr *attrib, char *value);

void SipCreateMediaAttrib(SDPAttr *attrib, int mLineNo, char *name, char *value);
void SipCreateMediaEncryptAttrib(SDPAttr *attrib, int mLineNo, char *value);
void SipCreateMediaBandwidthAttrib(SDPAttr *attrib, int mLineNo, char *value);

void SipDupAttrib(SDPAttr *attrib1, const SDPAttr *attrib2);
void SipCDupAttrib(cache_t cache, SDPAttr *attrib1, const SDPAttr *attrib2);
int SipFreeContext(SipEventContext *context);
int SipCopyHeaders(SipMessage *s, SipMessage *m, SipEventContext *context);
int SipSetStatusLine(SipMessage *s, int status_code, char *status_str);
int SipExtractUrlParamsFromUrl(SipUrl *sipurl, SipUrlParameter *url_parameters);
int SipMatchDomains(char *dom1, char *dom2);
int SipSetMaxForwards(SipMessage *s, int max_forwards);
int SipReplaceReqUriwithMirrorProxy(SipMessage *s, unsigned long mirrorproxy);
int SipReplaceToOrFromwithMirrorProxy(SipMessage *s, unsigned long mirrorproxy, int header_type);
int SipReplaceReqUri(SipMessage *s, unsigned long mirrorproxy, int port, char* type);
int SipReplaceToOrFrom(SipMessage *s, unsigned long mirrorproxy, int header_type);
int SipReplaceToOrFrom(SipMessage *s, unsigned long mirrorproxy, int header_type);
int SipReplaceContactwithRsa(SipMessage *s,header_url *req_uri, SipError *err);
int SipInsertContact(SipMessage *s,header_url *req_uri, SipError *err);
int SipSetReqUri(SipMessage *msg, header_url * req_uri, char * method, SipError *err);
int SipPhoneContextPlus(SIP_S8bit **username);
int SipCopyVia(SipMessage *req, SipMessage *resp);
int SipCopyRR(SipMessage *req, SipMessage *resp);
int SipSetSupported (SipMessage *s, char *options);
int SipSetMinSE (SipMessage *s, int minSE);
int SipSetSessionExpires (SipMessage *s, int sessionExpires, int refresher);
char * SipGetReason(int status);
int SipSetToFromHdr(SipMessage *m, header_url *pheaderurl, int headertype);
int SipSetReferToByHdr(SipMessage *m, header_url *referto, header_url *referby);
int SipSetRequire (SipMessage *s, char *option);
int SipExtractHdrAsString(SipMessage *m, en_HeaderType dType, char **str, int i);
int SipGetMaxForwards(SipMessage *s, int *max_forwards);
int SipGetSessionTimerSupport(SipMessage *s, int *timerSupport);
int SipGetMinSE (SipMessage *s, int *minSE);
void SipExtractPrivacyLevelFromRPID(SipMessage* m, char** hdr, SipError* err);
int SipGetSessionExpires(SipMessage *s, int *sessionExpires, int *refresher);
int SipGetSessionTimerRequire(SipMessage *s, int *timerSupport);
int SipDumpMessage(SipMessage *s);
int SipValidateFrom (header_url *url);
int SipValidateTo (header_url *url);
int SipFormatAck (SipMessage **ack, SipMessage *invite, SipMessage *resp);
char *SipMediaTypeAsStr (int mediaType);
MediaType SipMediaType (char *media);


extern void SipStripUriParams (char *uri);
extern int SipFindAppCallID (SipMsgHandle *msgHandle, char *callID, 
                             char *confID);
extern int SipBridgeInitSipCallHandle (SipEventHandle *evHandle, 
                                       SipCallHandle *sipCallHandle);
extern int AssignLocalSDPToCallHandle (SipEventHandle *evHandle, 
                                       CallHandle *callHandle);
extern int SipUASendToTSM (SipEventHandle *evb);
extern int SipFreeAppCallHandle (SipAppMsgHandle *appMsgHandle);
extern int SipFreeEventHandle (SipEventHandle *evb);
extern int SipFreeAppMsgHandle (SipAppMsgHandle *appMsgHandle);
extern int SipFreeSipCallKey (SipCallLegKey *callLeg, int freefn);
extern int SipFreeSipMsgHandle (SipMsgHandle *msgHandle);
extern int SipValidateSipEventHandle (SipEventHandle *evHandle);
extern int SipInitializeContact (CallHandle *callHandle, 
                                 SipMsgHandle *msgHandle);
extern int SipInitializeTags (CallHandle *callHandle, SipMsgHandle *msgHandle);
extern int SipCreateTags (CallHandle *callHandle);
extern int SipPrintState (CallHandle *callHandle);
extern int sipHuntError (int respCode);
extern int SipFindSrc (header_url *from_uri, char *srchost, 
                       long unsigned int ip, char *tg, char *cic, char *dnis, 
                       CallRealmInfo *realmInfo, CacheTableInfo *srcCacheInfo);
SipMsgHandle *
SipBridgeCreateMsgHandle2(SipCallHandle *sipCallHandle, 
			  char *method, int msgType, int respCode);


#endif /* _siputils_h_ */
