#ifndef _sconfig_h_
#define _sconfig_h_

#ifdef ALLOW_ISERVER_H323
#include "uh323.h"
#endif

#include "cdr.h"

/* Peer states */
#define PEERSTATE_DISC	0	/* Attained */
#define PEERSTATE_CONN	1	/* Skipped */
#define PEERSTATE_HELLO	2	/* Skipped */
#define PEERSTATE_SYN	3	/* Attained */
#define PEERSTATE_UPDT	4	/* Attained */

/* Local Configuration */
typedef struct
{
	ConnEntry		connEntry;
	ConnHandle 		*handle;
	int 			fd;

	serplex_config 	redunds;
	int				isparent;

	int				timereq;
	int				havetime;
	int				hastime;
	long 			offsetSecs;

	int				state;
} RedundConfig;

typedef struct
{
	serplex_config 	myconfig;
	RedundConfig	redconfig;
	TimerPrivate	timerPrivate;
	tid				timer;
	int				netfd;
	int				vpnsfd;
	int				redundsfd;
#ifdef ALLOW_ISERVER_H323
	//UH323Globals 	uh323Globals;
#endif
	char			cdrfiletype;
	char			cdrdirname[CDRDIRNAMELEN];
	FILE			*cdrfile;
    	int                     cdrtimer;
	FILE			*ctrfile;
	int			cdrevents;
 	int			cdrformat;
	int  			billingType;
    
} Config;

extern NetFds lsnetfds;
extern Config localConfig;

extern TimerPrivate *h323timerPrivate;
int
ProcessConfigFile(char *cfg);
int sconfig_parse_config(char *infile);
int InitVars();
int InitConfig();
void sconfigerror(char *str);
int SetDebugLevel(int moduleid, int set, int all, int level);
void SetCustomDebug(char* debug);
#endif /* _sconfig_h_ */ 
