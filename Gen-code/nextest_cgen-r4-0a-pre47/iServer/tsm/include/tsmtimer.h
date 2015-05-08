#ifndef _tsmtimer_h_
#define _tsmtimer_h_
//#define TIMER_T1  500000	/* micro seconds */
//#define TIMER_T2 4000000

#define TIMER_T1  	siptimerT1	/* micro seconds */
#define TIMER_T2 	siptimerT2

/* transactioin is terminated (removed) when timer expires */
#define SIPTRAN_GLB_TERM_TIMER 32	/* seconds */
#define INVITE_SERVER_TERM_TIMER 32	/* seconds */
#define INVITE_CLIENT_TERM_TIMER 32

#define INFO_SERVER_TERM_TIMER 10 /* seconds */
#define INFO_CLIENT_TERM_TIMER 10 /* seconds */

#define OTHER_SERVER_TERM_TIMER 40	/* 10*T2 */
#define OTHER_CLIENT_TERM_TIMER 0	/* not used */

//#define MAX_INV_RQT_RETRAN 7
#define MAX_INV_RQT_RETRAN 	sipMaxInvRqtRetran
#define MAX_INV_RSP_RETRAN 7
#define MAX_INFO_RETRAN    5
#define MAX_OTHER_RQT_RETRAN 11

int SipTimerProcessor(struct Timer*);
int SipOperateTimer(SipTrans *siptranptr, struct Timer *timer);
int SipGlbTimer(SipTrans *siptran);

#endif /* _tsmtimer_h_ */
