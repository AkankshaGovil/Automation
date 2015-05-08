#ifndef	 _EXECD_H_
#define	 _EXECD_H_

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ipc.h>
#include "srvrlog.h"
#include "ipcutils.h"
#include "serverp.h"

#define	MAX_LINE	256
#define MAX_MSGLEN	2048	
#define MAX_NUM_MSG	500
#define SRVR_MSG_TYP 1

typedef struct q_msg {
	unsigned int	self_id;
	unsigned int	peer_id;
	unsigned int	flag;
#define 	REQ_BIT	0
#define 	OUT_BIT	1	
	unsigned char		cmd[1];
} q_msg;

#define		ISSET_BIT(x,b) 	(((x)->flag) & (1<<(b)))
#define		SET_BIT(x,b) 	((x)->flag) |= (1<<(b))
#define		CLR_BIT(x,b) 	((x)->flag) &= ~(1<<(b))
#define 	Q_MSG_HDRLEN	( (int) &(((q_msg *)0)->cmd[0]))
#define		MAX_CMDLEN		(MAX_MSGLEN - Q_MSG_HDRLEN)

/* Functions exported */
extern int open_execd(void);

extern int sys_execd(int msgqid, unsigned int selfid, unsigned int peerid, unsigned int flag, 
	char *cmdstr, char *outbuf, int len); 

#endif
