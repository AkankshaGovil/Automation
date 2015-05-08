#ifndef _LS_H_
#define _LS_H_

#include "bits.h"
#include "ipc.h"

int GisExtractRegIdInfo(char *endptIdStr, PhoNode *phonodep);
int uh323AddRASAlias(HRAS hsRas, cmAlias *alias, int *index,
		cmRASTrStage stage, cmRASParam param);
int GkUnregisterAllIedgePortsByIP(DB db, NetoidInfoEntry *infoEntry, 
		int gws, int sgks);
int GisVerifyRegIdInfo(char *endptIdStr, PhoNode *phonodep);

extern int read_command (FILE *in, char *buffer, int buflen);

extern int FindRemote (ResolveHandle *rhandle);
extern int GkUnregisterAllIedgePortsByIP (DB db, NetoidInfoEntry *infoEntry, int gws, int sgks);

extern int uh323AddRASAlias (HRAS hsRas, cmAlias *alias, int *index, cmRASTrStage stage, cmRASParam param);

int IServerNotify(void *);
int HandleNotify(int fd, FD_MODE rw, void *data);
int IServerH323Notify(void *);
int HandleH323Notify(int fd, FD_MODE rw, void *data);

int ProcessCommandLine(FILE *in);
int PrintCommandPrompt(FILE *out);
int lm_ChkExpiry(struct Timer *lmtimer);
int FindRemoteByEnum(ResolveHandle *rhandle);
int ProcessEnumUri(char *inuri, InfoEntry *info);
int iServerCheckHealth(void);
int NetH323FdsInit(void);
int InitPipe(int notifyPipe[]);
int GisQInit();
int CliDInit(uint16_t port);
int LsAgeInit();
void IpcH323LaunchLoops(void);
int LusSetupTimeoutInMsec(TimerPrivate *timerPrivate, int *msec);
int iServerPoll(tid t);
int SendXML(int fd, char *xmlEnc, int len);
int XMLEncodeVpnGEntry(VpnGroupEntry *vpnGroupEntry, char *buf, int buflen, unsigned char *tags);
int XMLEncodeInfoEntry(InfoEntry *infoEntry, char *buf, int buflen, unsigned char *tags);
int XMLEncodeCREntry(VpnRouteEntry *entry, char *buf, int buflen, unsigned char *tags);
int XMLEncodeVpnEntry(VpnEntry *vpnEntry, char *buf, int buflen, unsigned char *tags);
int XMLEncodeCPEntry(CallPlanEntry *entry, char *buf, int buflen, unsigned char *tags);
int AddEntry(int type, char *buf, int buflen, void *entry);
int CliDMain();
int CFG_ReserveFds(void);
int handle_command(char *buffer);
int type_command(char *cmd);
int handle_get_command(char *cmd);
int handle_set_command(char *cmd);
int handle_reset_command(char *cmd);
int handle_bye_command(char *cmd);
int handle_cli_command(char *buffer);
int LusAdjustTimeout(struct timeval *tout, int *msec);
int HandleAgedIedgeIpAddr(NetoidInfoEntry *infoEntry);
int SrvrReportErrorToPhoNode(int errtype, int errcode, unsigned long myip, PhoNode *phonodep);
void IServerSecondary(void);
void IServerPrimary(void);
int IpcMainLoop(void);	
int IpcParse (int argc, char * argv[]);	
int IpcTerminate (void);

#endif /* _LS_H_ */
