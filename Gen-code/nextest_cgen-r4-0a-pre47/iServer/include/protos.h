#ifndef _proto_s_h_
#define _proto_s_h_

/*
 * Prototypes.
 */
int IpcParse (int argc, char * argv[] );
int IpcTerminate (void);
int IpcMainLoop (void);
int PktSend (int sockfd, Pkt * pkt);
int ControlPacketSend (int sockfd, Pkt * pkt, char * ipaddress);
int daemonize (void);
char * IPtostring(IPaddr ipaddress);
char * RouteFlagsString(unsigned long flags);
char * CallID2String(char *callID, char *str);
char * ConfID2String(char *confID, char *str);
char * GetSipRegState(int state);
char * GetSipRegEvent(int type, int event);
char * GetSipBridgeEvent(int event);
char * h323InstanceName(int instance);
char * DbOperToStr(int op);

char* createAuth(NetoidInfoEntry *infoEntry, char *method, char *authenticate);
	
#endif	/* _proto_s_h */
