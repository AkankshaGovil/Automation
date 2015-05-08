#ifndef COMMON_H
#define COMMON_H
#include <stddef.h>
#include <strings.h>
#include <string.h>
#include <stdlib.h>
#include <signal.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/in_systm.h>
#include <netinet/ip.h>
#include <netinet/in.h>
#include <netinet/ip_icmp.h>
#include "timer.h"
#include "nxosdtypes.h"

extern int ExtractArgs (char *argsin, int *argc, char **argv, int xargv);

extern int IcmpdProcessPacket (char *buf, int n, struct sockaddr_in *from, 
                               int fromlen);
extern int IcmpdCheckCallback (struct icmp *icmp, int icmplen);
extern int IcmpdRegisterCallback (uchar_t ip_p, struct sockaddr *src, 
                                  struct sockaddr *dest, 
                                  int (*fn) (char *, int, void *), void *arg);

extern int FillAllSignals (sigset_t *all);
extern int BlockAllSignals (sigset_t *new, sigset_t *old);
extern int UnblockAllSignals (sigset_t *new, sigset_t *pending);
int ExtractArg(char *argsin, char **arg, char **argsout);
	
int timersPrint(TimerPrivate *pvt);
int TimerStats(TimerPrivate *pvt);
	
#define NX_INT_TO_POINTER(i)      ((void *)(intptr_t)(i))
#define NX_POINTER_TO_INT(p)      ((int)(intptr_t)(p))
	

#endif
