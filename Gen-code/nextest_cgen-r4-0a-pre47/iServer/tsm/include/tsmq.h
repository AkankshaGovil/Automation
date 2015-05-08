#ifndef _tsmq_h_
#define _tsm_q_h_

typedef struct
{
	SipMessage 	*s;
	struct Timer *timer;
	int			from;
	char 		*method;	
	SipEventContext *context;

} TsmQEntry;

TsmQEntry * TsmNewQEntry(void);

#endif /* _tsm_q_h_ */
