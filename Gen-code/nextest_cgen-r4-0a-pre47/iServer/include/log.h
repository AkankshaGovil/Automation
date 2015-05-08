#ifndef _log_h_
#define _log_h_

#include "ipc.h"
#include "key.h"
#include "serverdb.h"
#include "mem.h"

/*
 * log.h
 */

/* Initialize logging into a certain file stream */
void
logInit (FILE * stream);

/* Actual API */
extern char stateString[][25];

char *
FormatIpAddress(unsigned int ipAddress, char s[]);

int
PrintInfoEntry(FILE *stream, InfoEntry *infoEntry);

int
PrintDbInfoEntry(FILE *stream, InfoEntry *infoEntry);

char *
CallID2String(char *callID, char *str);


char * ConfID2String(char *confID, char *str);
char * ConfID2String2(char *confID, char *str);
char * GetVendorDescription(Vendor vendor);
char * getfilename(char *path, char *dirdelim);
char* GetBcapLayer1Str(char proto);
char* GetQ931CdpnStr(char cdpn);
char* GetQ931CgpnStr(char cgpn);
char GetBcapLayer1(char *str);

int PrintCallID(FILE *stream, char *callID);

void log (int severity, int syserr, char *format, ...);
int format (FILE *stream, int level);
char *GetVendorDescription (Vendor vendor);
int GetVendor (char *v);
void String2Guid (char *str, char *callID);


struct CallHandleStruct;
//struct ConfHandle;
#include "list.h"
#include "confdefs.h"
#include "callsm.h"
#include "callconf.h"

int PrintCallEntry (FILE *stream, struct CallHandleStruct *callHandle);
int PrintH323CallEntry (FILE *stream, struct CallHandleStruct *callHandle);
int PrintSipCallEntry (FILE *stream, struct CallHandleStruct *callHandle);
void PrintConfEntry (FILE *stream, ConfHandle *confHandle);
int PrintGkInfoEntry (FILE *stream, GkInfo *gkInfo);
int PrintDbAttrEntry (FILE *stream, ClientAttribs *clAttribs);

int DEBUG_PrintInfoEntry (int module, int level, InfoEntry *infoEntry);
int ERROR_PrintInfoEntry (int module, InfoEntry *infoEntry);
int PrintVpnEntry (FILE *stream, VpnEntry *vpnEntry);
int PrintCPEntry (FILE *stream, CallPlanEntry *entry);
int PrintCPBEntry (FILE *stream, CallPlanBindEntry *entry);
int PrintCREntry (FILE *stream, VpnRouteEntry *entry);
void PrintTriggerEntry (FILE *stream, TriggerEntry *tgEntry);
int PrintRealmEntry (FILE *stream, RealmEntry *rmEntry);
int PrintIgrpEntry (FILE *stream, IgrpInfo *igrp);
int PrintVnetEntry (FILE *stream, VnetEntry *vnetEntry);
int DEBUG_PrintPhoNode (int module, int level, PhoNode *node);
char *GetInput (FILE *stream, char *buf, int xinlen);
int FindIedgeType (char *type);
int SetIedgeType (NetoidInfoEntry *netInfo, int type);
int SetIedgeTypeOptional (NetoidInfoEntry *netInfo, int type);
int SetIedgeTypeMandatory (NetoidInfoEntry *netInfo, int type);
int SetIedgeTypeOptional (NetoidInfoEntry *netInfo, int type);
int SetIedgeTypeMandatory (NetoidInfoEntry *netInfo, int type);
int RouteFlagsValue (unsigned int crflags, char *string);
int chr2hex (char *str, unsigned char *hexStr);
int hex2chr (char *str, int xlen, unsigned char *hexStr, int hexLen);
int DEBUG_PrintBuffer(int module, int level, char *buf, int len);
int PrintCallCache(FILE *out);
char * RouteFlagsString(unsigned long flags);
#endif
