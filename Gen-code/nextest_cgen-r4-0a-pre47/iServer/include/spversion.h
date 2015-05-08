
#ifndef _spversion_h_
#define _spversion_h_


/* Contains global information about releases */
#include "relinfo.h"


/*
 * This file contains version number information for
 * ls, vpns, cli, lsage, vpnsage.
 */

#undef VERSION
#define VERSION		"v3.2"
#define MINOR		"d24"
#define BUILDDATE	"11-02-2004"
#define LIC_VERS 	"2.1"

/*
 *********
 * GIS
 *********
 */
#define		GIS_NAME	"NexTone GIS Directory Server"

#define		GIS_VERSION	VERSION

#define		GIS_MINOR	MINOR

/* Import GENERIC_RELNAME from relinfo.h */
#define		GIS_RELNAME	CURRENT_RELNAME

#define		GIS_BUILDDATE	BUILDDATE

#define		GIS_COPYRIGHT	CURRENT_COPYRIGHT


/*
 *********
 * VPNS
 *********
 */
#define		VPNS_NAME	"NexTone VPN Directory Server"

#define		VPNS_VERSION	VERSION

#define		VPNS_MINOR	MINOR

/* Import GENERIC_RELNAME from relinfo.h */
#define		VPNS_RELNAME	CURRENT_RELNAME

#define		VPNS_BUILDDATE	BUILDDATE

#define		VPNS_COPYRIGHT	CURRENT_COPYRIGHT




/*
 *********
 * LS
 *********
 */
#define		LS_NAME	"NexTone GIS Directory Server"

#define		LS_VERSION	VERSION

#define		LS_MINOR	MINOR

/* Import GENERIC_RELNAME from relinfo.h */
#define		LS_RELNAME	CURRENT_RELNAME

#define		LS_BUILDDATE	BUILDDATE

#define		LS_COPYRIGHT	CURRENT_COPYRIGHT


/*
 *********
 *BCS 
 *********
 */
#define		BCS_NAME	"NexTone Billing Server"

#define		BCS_VERSION	VERSION

#define		BCS_MINOR	MINOR

/* Import GENERIC_RELNAME from relinfo.h */
#define		BCS_RELNAME	CURRENT_RELNAME

#define		BCS_BUILDDATE	BUILDDATE

#define		BCS_COPYRIGHT	CURRENT_COPYRIGHT

/*
 *********
 *FAXS 
 *********
 */
#define		FAXS_NAME	"NexTone Fax Server"

#define		FAXS_VERSION	VERSION

#define		FAXS_MINOR	MINOR

/* Import GENERIC_RELNAME from relinfo.h */
#define		FAXS_RELNAME	CURRENT_RELNAME

#define		FAXS_BUILDDATE	BUILDDATE

#define		FAXS_COPYRIGHT	CURRENT_COPYRIGHT


/*
 *********
 * CLI
 *********
 */
#define		CLI_NAME	"NexTone Command Line Provisioning Utility"

#define		CLI_VERSION	VERSION

#define		CLI_MINOR	MINOR

/* Import GENERIC_RELNAME from relinfo.h */
#define		CLI_RELNAME	CURRENT_RELNAME

#define		CLI_BUILDDATE	BUILDDATE

#define		CLI_COPYRIGHT	CURRENT_COPYRIGHT

/*
***********
 * jServer
 **********
 */
#define		JSERVER_NAME	"NexTone Configuration Server"

#define		JSERVER_VERSION	VERSION

#define		JSERVER_MINOR	MINOR

/* Import GENERIC_RELNAME from relinfo.h */
#define		JSERVER_RELNAME		CURRENT_RELNAME

#define		JSERVER_BUILDDATE	BUILDDATE

#define		JSERVER_COPYRIGHT	CURRENT_COPYRIGHT

/*
 *****************
 * Process Manager
 *****************
 */
#define		PM_NAME	"NexTone Process Manager"

#define		PM_VERSION	VERSION

#define		PM_MINOR	MINOR

/* Import GENERIC_RELNAME from relinfo.h */
#define		PM_RELNAME	CURRENT_RELNAME

#define		PM_BUILDDATE	BUILDDATE

#define		PM_COPYRIGHT	CURRENT_COPYRIGHT

/*
 *****************
 * Replication Server Daemon
 *****************
 */
#define		RS_NAME	"NexTone Replication Server"

#define		RS_VERSION	VERSION

#define		RS_MINOR	MINOR

/* Import GENERIC_RELNAME from relinfo.h */
#define		RS_RELNAME	CURRENT_RELNAME

#define		RS_BUILDDATE	BUILDDATE

#define		RS_COPYRIGHT	CURRENT_COPYRIGHT

/*
 *****************
 * Cmd Exection Server Daemon
 *****************
 */
#define		EXCD_NAME		"NexTone Cmd Execution Server"

#define		EXCD_VERSION	VERSION

#define		EXCD_MINOR		MINOR

/* Import GENERIC_RELNAME from relinfo.h */
#define		EXCD_RELNAME	CURRENT_RELNAME

#define		EXCD_BUILDDATE	BUILDDATE

#define		EXCD_COPYRIGHT	CURRENT_COPYRIGHT


#endif	/* _spversion_h_ */
