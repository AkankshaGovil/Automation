#ifndef __HELLO_H_
#define __HELLO_H_

#include "hello_common.h"

pthread_t   	timerThread = (pthread_t)-1;
NetFds      	Hellotimerfds;
HelloConfp  	hcp;
Lock			*hcp_lock;
cvtuple			hellocv = {PTHREAD_MUTEX_INITIALIZER, PTHREAD_COND_INITIALIZER, 0};

extern int		RunHelloProtocol(int argc, char **argv);
extern int		HelloRegCb(AppCB *appcb);

#endif	/*	__HELLO_H_ */
