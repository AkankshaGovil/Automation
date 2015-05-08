#ifndef _DBS_H_
#define _DBS_H_

#include "ipc.h"
#include "key.h"
#include "bits.h"

int GisPostCliIedgeRegCmd(char *regid, int uport, char *tags);
void DbScheduleIedgeUpdate(NetoidInfoEntry *netInfo);
int GisPostCliGkRegCmd(char *regid, int uport, char *tags);
int GisPostCliCRAddCmd(char *crname, char *tags);
void *GisPostCliCmdWorker(void *arg);
int GisPostCliCmd(int command, char *regid, int uport, char *tags);
int GisPostCliCRCmd(int command, char *crname, char *tags);
void DbScheduleIedgeDelete (NetoidInfoEntry *netInfo);
void DbScheduleRouteUpdate (RouteEntry *routeEntry);
void DbScheduleRouteDelete (RouteEntry *routeEntry);
int DbInit();
	

#endif /* _DBS_H_ */
