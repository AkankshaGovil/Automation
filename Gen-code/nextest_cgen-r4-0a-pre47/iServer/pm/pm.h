#ifndef _pm_h_
#define _pm_h_

static int ProcessConfig(void);
int startProcesses(int sd);
static void sig_handler(int signo);
static void SignalInit(void);
static void* AsyncSigHandlerThread( void* args );
static void SyncSigHandler(int signo);
static void DummySigHandler(int signo);
void ChildSetNonRT(void);
extern int ReadPid(char *pathname);
static int wait_for_coredump (char *proc);
int start(PsEntry*);
void resetPoll(MonitoredPsGrp *psgrp);
int RSDConfigured(void);
extern int FillUptimeInfo (char *buf);
extern void initPsGrp (void);
extern int checkPoll (int grpid, int sd);
extern void handleKA (void);
extern int initserver (void);
extern int handlepkt (KeepAlive *pmsg);

#endif /* _pm_h_ */
