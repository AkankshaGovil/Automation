#ifndef _H323REALM_H_
#define _H323REALM_H_
#include <stdio.h>
#include "seli.h"
#include "rvcommon.h"

typedef enum {
	Ras_eSgk,
	Ras_eArq,
} Ras_type;

typedef struct {
	unsigned long destIP; 		// Local IP on which message was received
	unsigned short destPort; 	// Port on which message was received
	unsigned short q931Port;	// local q931 port which will receive q931 signalling message for this realm. Needed when MSW acts as a gateway
	int	sd; // Socket on which the message was received can be used
	int realmId; // logical id used for indexing
	Ras_type type; //sgk or arq instance
} RasListener;


#define MAX_RAS_LISTENER  1024
extern RasListener rasListenerTbl[MAX_RAS_LISTENER]; 
int getQ931InfoFromRealmId(int realmId, Ras_type type, int *sdptr,int *ipptr, unsigned short *portptr);
int getRasInfoFromRealmId(int realmId, Ras_type type, int *sdptr,int *ipptr, unsigned short *portptr);
int getQ931InfoFromRealmId(int realmId, Ras_type type, int *sdptr,int *ipptr, unsigned short *portptr);
int startServer(uint32_t ip, short port);
int createRasListener(int realmId, Ras_type type, uint32_t ip, unsigned short port, unsigned short q931port);
void rasRecv(int sd, seliEvents sEvent, BOOL error);
int getRasRealmInfo(int sd, int *ip, unsigned short *port, int *realmIdptr);
int getRasInstances(void);
int destroyRasListener(int realmId, Ras_type type,unsigned short port);
int getQ931RealmInfo(int sd, int *ipptr, unsigned short *portptr, int *realmIdptr);

#endif
