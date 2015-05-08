#ifndef _rsd_common_h_
#define _rsd_common_h_

#define		RS_DEF_MCAST_ADDR		"230.1.1.1" /* Rendezvous point with remote clients */
#define		RS_DEF_PORT				"5000"
#define		RS_DEF_COMM_DEV			"znb1"		/* RS Communication Device Name */
#define 	RS_DEF_CP_CMD_STR		"/usr/local/bin/rsync -a "
#define		RS_DEF_TMP_DIR			"/tmp/rs/"
#define		RS_DEF_HOST_PRIO		0

#define		RS_BASE_DIR			"/usr/local/nextone/bin"
#define 	RS_CLI_CMD_STR		"/usr/local/nextone/bin/cli "
#define 	RS_STR_FNAME 		"/.rscp"    /* Rendezvous point with local clients */
#define 	RS_DB_SAVE_STR		"db save "
#define 	RS_DB_COPY_STR		"db copy "
#define 	RS_DB_CREATE_STR	"db create "
#define 	RS_DB_EXPORT_STR	"db export "
#define		RS_CLI_SLAVE_SUFF	"-r" 		/* Don't send suffix */
#define		RS_CLI_REG_SUFF		"-s"		/* Don't store suffix */
#define 	RS_CLI_DIR_SUFF		"-d"		/* Directory suffix */
#define 	RS_CLI_NO_OUT_SUFF	"-o"		/* Disable output suffix */

#define 	RS_TMP_XML_FNAME	"db.xml"

#define		RS_LINELEN			80			/* Length of a line */
#define		RS_MSGLEN			2048	

#define		RS_SEND_SEQNUM_INT	(15*60) 	/* 15 Minutes */

#define		CLI_MAX_HIST		2000		/* WARNING: keep it same as hist.c */

/* RSD Stuff */
extern char		rs_ifname[RS_LINELEN];
extern char		rs_mcast_addr[RS_LINELEN];
extern char		rs_port[RS_LINELEN];
extern char		rs_tmp_dir[RS_LINELEN];
extern char		rs_cp_cmd_str[RS_LINELEN];
extern int		rs_host_prio;
extern int		rs_ssn_int;
extern int		histdb_size;
extern int		RSDConfig;

#endif /* _rsd_common_h_ */
