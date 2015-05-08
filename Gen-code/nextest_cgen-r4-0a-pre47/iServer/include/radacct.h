#ifndef _RADACCT_H_
#define _RADACCT_H_

typedef struct accounting_info_t
{
	void* tid;
	int   packet_id;
	char* acctAuthentic;
	char* acctSessionId;
	char* acctSessionTime;
	char* acctStatusType;
	char* callConnectTime;
	char* callEndTime;
	char* callOrigin;
	char* callSessionProtocol;
	char* callStartTime;
	char* callType;
	char* calledStationId;
	char* callingStationId;
	char* cause;
	char* ciscoNasPort;
	char* confId;
	char* connectInfo;
	char* faxTxDuration;
	char* gkXlatedCdn;
	char* gwFinalXlatedCdn;
	char* gwId;
	char* gwRxdCdn;
	char* incomingConfId;
	char* nasIdentifer;
	char* nasIpaddr;
	char* nasPort;
	char* nasPortType;
	char* outgoingArea;
	char* proxyState;
	char* remoteAddr;
	char* remoteMediaIpaddr;
	char* serviceType;
	char* subscriber;
	char* tg;
} AccountingInfo;


#define CURRENT 0
#define BACKLOG 1

int initRadacct_db();
void closeRadacct_db();
int storeAccoutingInfo(int db, AccountingInfo *info);
AccountingInfo *getAccoutingInfo(int db);
void deleteAccoutingInfo(void *tid);

void *sendCiscoRadiusAccountingScheduler(void*);
void *sendCiscoRadiusAccountingBacklogScheduler(void*);
int initRadiusAcct (void);
void stopRadiusAcct (void);
void removeAccoutingInfo(void* tid);
void unMarshalAccountingInfo(char *buf, AccountingInfo *data);
void marshalAccountingInfo(AccountingInfo *data, char *buf, int size);

#endif /* _RADACCT_H_ */
