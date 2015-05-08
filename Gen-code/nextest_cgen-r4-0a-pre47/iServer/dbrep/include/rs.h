#ifndef 	__dbrep_rs_h
#define		__dbrep_rs_h

#include	<sys/types.h>
#include 	<time.h>
#include 	<ctype.h>
#include 	<netdb.h>
#include 	"hist.h"
#include 	"timer.h"
#include 	"lock.h"
#include	"thutils.h"
#include 	"srvrlog.h"
/* Header files for sconfig related configuration */
#include	"pids.h"
#include	"lsconfig.h"
#include 	"serverp.h"
#include	"rsd_common.h"
/* Header files for pm related configuration */
#include 	"pmpoll.h"
/* Header files for cli related config. */
#include	"execd.h"

#define 	CMD_EX(str)			( printf(str), printf("\n"), 0 )
#define 	MAX_CLIENTS			10
#define		CHECK_CLI_ORDER		1  /* whether the program should check the sequence */
#ifndef MAXHOSTNAMELEN
#define MAXHOSTNAMELEN 256
#endif

/* Some global variable to maintain the cli history */
extern int		rs_cur_ind;				/* Index of last command */
extern int		db_inited;
extern char		rs_cli_cmd_str[RS_LINELEN];		/* Cli command executed by RS */
extern char		rs_slave_cli_cmd_str[RS_LINELEN];	
extern char		rs_reg_cli_cmd_str[RS_LINELEN];	
extern char 	rs_hostname[MAXHOSTNAMELEN]; 	/* Host name of the machine on which rs is
										   running. The IP address of the interface
										   on which rs is communicating is preferable */
extern char 	host_ver[RS_LINELEN]; 	/* Architecure of host and version of iServer */

typedef struct	sharvar {
	pthread_mutex_t		mutex;
	int					value;
} sharvar;

typedef struct ReplServConf {
	FILE 		*locfp; 	/* Local streams open for reading */
	int 		sendfd;		/* Network udp socket fd bound to the unicast address. */
	int 		recvfd;		/* Network udp socket fd bound to the multicast address. */
	int 		rcmdfd;		/* Network tcp socket fd with which the server communicates. */
	int			listenfd;	/* Local socket to listen for cli clients */
	int			*loc_fds;
	int			*rem_fds;
	struct sockaddr	*sa; 	/* Multicast Address of the servers */
	socklen_t 	salen;  	/* Length of the sockaddr structure */
	int			role;
#define		REPL_SERV_PRI	(0x1)
#define		REPL_SERV_SEC	(0x2)

#define		THREAD_HELLO		0
#define		THREAD_COPY_SERVER	1
#define		THREAD_REPL_SERVER	2
#define		THREAD_SNO_SERVER	3
#define		THREAD_MIN			4
#define		THREAD_MAX			25
#if 0
#define		THREAD_HELLO		0
#define		THREAD_RSSD			1
#define		THREAD_COPY_SERVER	2
#define		THREAD_REPL_SERVER	3
#define		THREAD_MIN			4
#define		THREAD_MAX			25
#endif
	pthread_t	tids[THREAD_MAX];
	pthread_mutex_t	tid_mutex;
	cvtuple		replcv;
	cvtuple		copycv;
} RSConf, *RSConfp;

#define		NULL_TID		((pthread_t)-1)

extern RSConfp	the_rscp;
extern sharvar	host_dbrev;

typedef struct	CopyCli {
	RSConfp		rsp;
	pthread_t	tid;
	int			fd;
} CopyCli;

/*
 * The format of the message is the following
 * It contains a Msg header followed by a Pkt Header.
 * If the Pkt contains cli commands then they follow
 * sequentially. Each cli command string is prepended with
 * Cmd header.
 */

typedef struct CmdHdr {
	int		cmdtyp;
	int 	cmdlen;
	long	cmdseq;
	int		cmdrval;
	int		cmdact;
	int		cmdpid; 	
	int		cmdtim;		
} Cmd;

/*
 *  CMD_CLI conatins a cli command.
 *  CMD_FCP conatins a command to perform a file copy
 */

#define CMD_CLI	1
#define CMD_FCP	2
#define CMD_SNO	3

#define RET_STAT(rval)  ( (WEXITSTATUS(rval)) - ( ((WEXITSTATUS(rval)) & 0x80) ? (0x100) : 0 ) )

/*
 * This function returns the command string pointed to by
 * cmdp in the packet pointed to by pktp
 */
#define NULLCMD		  ((Cmd *) NULL)
#define CmdStr(cmdp)  ((char *)((Cmd *)(cmdp) +1))
#define CmdSeq(cmdp)  ((cmdp)->cmdseq)
#define CmdLen(cmdp)  ((cmdp)->cmdlen)
#define CmdTyp(cmdp)  ((cmdp)->cmdtyp)
#define CmdRval(cmdp) ((cmdp)->cmdrval)
#define CmdAct(cmdp)  ((cmdp)->cmdact)
#define CmdPid(cmdp)  ((cmdp)->cmdpid)
#define CmdTim(cmdp)  ((cmdp)->cmdtim)

typedef struct RSPktHdr {
	int 	type;    		/* See the types defined below */
	unsigned int datalen;
} RSPkt;

/* define packet types */
#define PKT_REG			(0x00)		/* Reg Pkt has pkttype hex clear */
#define PKT_FROM_PRI	(0x10)		/* Pkt from Primary */
#define PKT_FROM_SEC	(0x20)		/* Pkt from Secondary */
#define CLI_CMDS		(0x01)		/* Packet contains a list of cli commands */
#define CLI_CMD_SEND	(0x02)		/* The Packet contains a request for cli */
#define CLI_FILE_CP		(0x04)		/* Request for file copy */
#define CLI_SNO_BCAST	(0x08)		/* Sequence Number Broadcast */

/* Some Useful Macros for accessing Pkt fields */
#define NULLPkt 		((RSPkt *) NULL)
#define PktTyp(pktp)	((pktp)->type)
#define PktLen(pktp)	((pktp)->datalen + sizeof(RSPkt))
#define DataLen(pktp)	((pktp)->datalen)
/*** Skip Pkt Header to get to the data ***/
#define PktDatap(pktp)	((char *)((pktp) + 1))

/*  define pkt errors */
#define PKT_ERR_TYPE	(0x1)

#define MAX_PKT_LEN		(1460) /* Maximum Packet Length	*/
#define	MAXBUF			MAX_PKT_LEN
#define MAXLEN			(512)  /* Maximum Length of a command */

typedef struct NetMsg {
	struct sockaddr *pa;	/* Peer (or sender) address */
	socklen_t	palen; 		/* length of the sockaddr of peer */
	RSPkt 		*Pktp;		/* Data */
} Msg;

#define NULLMSG 		((Msg *) NULL)
#define TSTAMP_STR		(20)		/* Maximum space that the timestamp string may occupy */

/* Macros to determine if the server is primary or secondary */
#define Is_Pri_Serv(rscp)	(((rscp)->role) == REPL_SERV_PRI)
#define Is_Sec_Serv(rscp)	(((rscp)->role) == REPL_SERV_SEC)

/* define Valid Packet Types for master and secondary servers */
#define Valid_Pkt_Pri(rscp, pktp)	( (((rscp)->role) == REPL_SERV_PRI) && \
									(PktTyp(pktp) & PKT_FROM_SEC) )
#define Valid_Pkt_Sec(rscp, pktp)	( (((rscp)->role) == REPL_SERV_SEC) && \
									(PktTyp(pktp) & PKT_FROM_PRI) )
#define Valid_Reg_Pkt(pktp)			( (PktTyp(pktp) & 0xf0) == 0 )

#define RS_ERR_SEQ_NUM 		(-0x101)
#define RS_ERR_NEED_SYNC	(-0x102)
#define RS_ERR_CMD_FAIL		(-0x103)
#define RS_ERR_ABORT		(-0x104)
#define RS_ERR_INTRPT		(-0x105)
#define RS_ERR_UNKNOWN_CMD	(-0x106)
#define RS_ERR_FOPEN		(-0x107)
#define RS_ERR_FLOCK		(-0x108)

#define RS_NEED_DB_REQ		("NEEDDB")
#define RS_NEED_DB_SUCC		("SUCCESS")
#define RS_NEED_DB_SUMI		("SUCCESS_VER_MISMATCH")
#define RS_NEED_DB_FAIL		("FAIL")

#define CLI_CMD_FRM_SLAVE ("SLAVE_CMD")
#define CLI_CMD_SUCC	  ("SUCCESS")
#define CLI_STR			  ("cli")

void 	start_server(RSConfp rscp);
int		RS_Init(RSConfp rscp, HelloConfp hcp);
int		RS_Kill(RSConfp rscp, int role);
void 	start_copy_server(RSConfp rscp);
void 	SendSeqNum(RSConfp rscp);
int		ProcessDBSync(int sockfd);
int 	ProcessNetRcvd(RSConfp rscp, Msg *msgp);
int 	SendNetPkt(RSConfp rscp, Msg *msgp);
int		SendPktQry(RSConfp rscp, long seqno, struct sockaddr *sa, socklen_t salen);
Msg 	*ReadNetPkt(RSConfp rscp, int fd);
Msg		*CreatePkt(RSConfp rscp, int fd);
void	FreeMsg(Msg *msgp);
int		ProcessCmdWHist(Msg *msgp, Cmd *cmdp);
int		ProcessCmdWOHist(Msg *msgp, Cmd *cmdp);
int 	ProcessCliCmd(Cmd *cmdp);
int 	ProcessCPCmd(struct sockaddr *, Cmd *cmdp);
int 	CreateChildAndDo(char *cmdstr);
char 	*basename(char *path);
void	RPC_Init(void);
int		ProcessDBSync(int sockfd);
int 	GetDB(struct sockaddr *sap, socklen_t salen);
void 	CreateDir(char *dirname);
long 	SaveDBinDir(const char *dirname, int incompat);
void 	CreateDBCPCmd(const char *dirname, char * const buf, int buflen);
int 	ImportDB(const char *dirname, int incompat);
void	repl_serv_end(void *);
void	copy_serv_end(void *);
void	copy_cli_end(void *);
int 	ConfigureRSD(void);
void	parse_rs_opt(int argc, char *argv[]);
char	*ServTyp(int role);
char	*PacketTyp(int pkttype);
void	CliCmdThreadInit(void);
int 	ExecuteCliCommand(char *cmd);
void 	CliCmdWorker(char *args[]);


/* Imports from other libraries */
extern  int	ServerSetLogging(char *name, serplex_config *s);
extern	int DoConfig(int (*processConfig)(void));
extern 	int	FindServerConfig(void);
extern  int CacheAttach(void);
extern  int CacheDetach(void);

/* Pkt and Cmd Utilities */
int 	EntsToBuf(CliEntry ***clippp, char *buf, int buflen);
Cmd 	*SkipCmd(RSPkt *pktp, Cmd *cmdp);
int 	CmdToEnt(Cmd *cmdp, int cmdlen, CliEntry *clip);
int 	EntToCmd(CliEntry *clip,  char *buf, int *buflen);
int 	BufToEnts(char *bufp, int buflen, CliEntry **clipp);

char 	*skip(char *);

#endif 	/* __dbrep_rs_h */
