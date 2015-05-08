#ifndef _RADCLIENT_H_
#define _RADCLIENT_H_

#include "mem.h"
#include "list.h"
#include "gis.h"
#include "calldefs.h"
#include "serverp.h"
#include "ssip.h"
#include "libradius.h"

char nas_ipaddr[16];

typedef struct challenge_t
{
	char* nonce;
	char* realm;
	char* algorithm;
} Challenge;


typedef struct credentials_t
{
	char* method;
	char* authentication_name;
	char* realm;
	char* uri;
	char* digest_response;
	char* nonce;
	char* algorithm;
} Credentials;


int initRadiusClient(void);
int getChallenge(header_url* fromn, char* callid, Challenge* challenge);
int sendCredentials(header_url* fromn, char* callid, Credentials* credentials);
void setRadiusAccountingSessionId(CallHandle *callHandle);
int sendCiscoRadiusAccounting(CallHandle *callHandle, int flag);
int SipCiscoRadiusAuthenticate(SipEventHandle *evb);
int ciscoRadiusAuthourize(SCC_EventBlock *evtPtr);
int initRadiusAuth (void);
void stopRadiusAuth (void);
int SipCiscoRadiusAuthourizeWorker (SCC_EventBlock *evtPtr);
int SipCiscoRadiusAuthourize (SCC_EventBlock *evtPtr);
int rad_getport (const char *name);
int rad_send_packet (RADIUS_PACKET *req, RADIUS_PACKET **rep, char *secret);
void stopRadiusClient();
int startRadiusClient();


#endif /* _RADCLIENT_H_ */
