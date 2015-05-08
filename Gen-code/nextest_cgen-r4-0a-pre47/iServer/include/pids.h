#ifndef _pids_h_
#define _pids_h_

#define PIDS_DIRECTORY "/databases"

#define LUS_PID_FILE 		"lus.pid"
#define VPNS_PID_FILE 		"vpns.pid"
#define GIS_PID_FILE 		"gis.pid"
#define LUSAGE_PID_FILE 	"lusage.pid"
#define GISAGE_PID_FILE 	"gisage.pid"
#define VPNSAGE_PID_FILE 	"vpnsage.pid"
#define BCS_PID_FILE 		"bcs.pid"
#define FAXS_PID_FILE 		"faxs.pid"
#define FAXD_PID_FILE 		"faxd.pid"
#define PM_PID_FILE 		"pm.pid"
#define ISPD_PID_FILE 		"ispd.pid"
#define RSD_PID_FILE 		"rsd.pid"
#define EXECD_PID_FILE 		"execd.pid"

#define SERPLEX_GID	0
#define JSERVER_GID	0
#define MAX_PS_GRP 	1 /* Total no. of process grps */

/* serplex process grp 
 * no gaps allowed
 */
#define JSERVER_ID	0
#define EXECD_ID	1
#define GIS_ID		2
#define RSD_ID		3

#define PROCESS_COUNT 4

int ReadPid(char *pathname);
int StorePid(char *pathname);
int UnlinkPid(char *pathname);


#endif /* _pids_h_ */
