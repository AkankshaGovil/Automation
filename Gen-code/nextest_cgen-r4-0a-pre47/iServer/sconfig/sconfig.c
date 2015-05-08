/*
 * sconfig.c
 * Provide a command line interface to edit the 
 * server.cfg file
 */

#include "generic.h"
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/syslog.h>
#include <netdb.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/time.h>
#ifdef SUNOS
#include <sys/sockio.h>
#include <sys/filio.h>
#else
#include <linux/sockios.h>
#include <pwd.h>
#endif
#include <string.h>
#ifdef _QNX
#include <sys/select.h>
#endif
#include <sys/uio.h>

#include <signal.h>
#include <sys/wait.h>

#include "spversion.h"

#include "generic.h"
#include "srvrlog.h"
#include "bits.h"
#include "ipc.h"
#include "serverdb.h"
#include "key.h"
#include "mem.h"
#include "dh.h"
#include "bn.h"
#include "isakmp.h"
#include "isadb.h"
#include "protocol.h"
#include "lsprocess.h"
#include "entry.h"
#include "pef.h"
#include "lsconfig.h"
#include "phone.h"
#include "serverp.h"
#include "pids.h"
#include "ifs.h"
#include "gw.h"
#include "timer.h"
#include "fdsets.h"
#include "db.h"
#include "connapi.h"
#include "shm.h"
#include "shmapp.h"
#include "xmltags.h"
#include "rs.h"
#include "sconfig.h"
#include <malloc.h>
#include "nxosd.h"
#include "nxosdtypes.h"

#include "log.h"

char 	pidfile[256];
char config_file[25] = "./server.cfg";
struct ifi_info *ifihead;

char addr[256];
char cfgfname[256] = "server.cfg";
char sdirname[256] = "./";
char fname[256];
char storeb[1024];
char outf[256];
static char *progname;

MemoryMap *map;
LsMemStruct *lsMem = 0;

extern  char    fax_dir_pathname[];
extern  char    fax_user[];
extern  char    fax_password[];

static char usage[] = "usage: %s [-n| [[-d <server.cfg dir path>] [-f <server.cfg file path>]] [-o outfile] [-help]\n";
static char help[] = "\
-n -> do not use any input server.cfg file\n\
-d -> directory path of input file, if different from \".\"\n\
-f -> name of input file, if different from \"server.cfg\"\n\
-o -> output file name, if different from server.cfg\n\
-h -> this menu\n";

/* -n option means don't prompt for config file */

static serplex_config *gis = 0, *lus = 0, *gisr = 0, *vpns = 0, *vpnsr = 0, *rsd = 0, *execd = 0;
static int debugmodules = 0;

#define ISPD_CONTROL	3

static void	GetInterfaceInfo( int iface_type );

static int	validate_host(	char *				hostname,
							struct ifi_info	*	ifi_ptr,
							char *				output );

static void	generate_rsyncd_conf(	void );
static void GetRSDInfo(void);
static void GetExecDInfo(void);
extern int ExecDConfig;

int
main(int argc, char **argv)
{
	int	c;
	int	isout = 0;
	int nocfgfile = 0;
	int	inputfile = 0;

	extern	char *	optarg;
	extern	int		optind;

	progname = argv[0];

	ifihead = initIfs();

	while ( ( c = getopt( argc, argv, "d:f:hmno:" ) ) != EOF )
	{
		switch ( c )
		{
		case 'd':
			strcpy( sdirname, optarg );
			strcat( sdirname, "/" );
			inputfile = 1;
			break;
		case 'f':
			strcpy(cfgfname, optarg );
			inputfile = 1;
			break;
		case 'h':
			fprintf(stdout, "%s", help);
			exit(0);
			break;
		case 'm':
			debugmodules = 1;
			break;
		case 'n':
			nocfgfile = 1;
			break;
		case 'o':
			isout = 1;
			strcpy( outf, optarg );
			break;
		case 'y':
			break;
		default:
			fprintf(stdout, usage, argv[0]);
			exit(0);
			break;
		}
	}

	strcpy(fname, sdirname);
	strcat(fname, cfgfname);

	if (nocfgfile && inputfile)
	{
		fprintf(stdout, usage, argv[0]);
		fprintf(stdout, "Can't specify -n and -[f[d]] simultaneously");
		return -1;
	}

	if ((nocfgfile == 0) && (inputfile == 0))
	{
		/* Arguments are parsed, now read the config file */
		fprintf(stdout, "cfg file [%s]: ", fname);
		GetInput(stdin, storeb, 256);
		if (strlen(storeb) > 0) strcpy(fname, storeb);
	}

	if (isout == 0)
	{
		strcpy( outf, fname );
	}

	InitVars();
	InitConfig();
	
	if (!nocfgfile)
	{	
		if (sconfig_parse_config(fname) != 0)
		{
			fprintf(stdout, "Could Not Parse config file %s\n", fname);
			return -1;
		}
	}

	ProcessConfigFile(fname);

	return(0);

}

int 
GetPasswd(char *prompt, char *passwd)
{
	char str[256];
	int i;
	char *p	;
	if(!prompt || !passwd) 
		return -1;
	for(i=0; i<3 ; ++i)
	{
		p = getpass(prompt);
		if (!p) return -1;
		strcpy(passwd, p); 
		if(strlen(passwd) == 0) 
			return -1;
		p = getpass("\nRenter New Password :");
		if (!p) return -1;
		strcpy(str,p);
		if (!strcmp(passwd,str))
			return 0;
		fprintf(stdout,"\nSorry, passwords do not match ");
		fflush(stdout);
	}
	fprintf(stdout,"Using default passwd\n");
	fflush(stdout);
	return -1;
}
  
int
ProcessConfigFile(char *cfg)
{
	int i, match = -1, tmpRolloverTime, prevFirewall;
	int radius_config_required = 0;
	char str[256], s1[256];
        char buf[INET_ADDRSTRLEN];
	time_t clock;
	int	 oldCST;

	if (strlen(mswname) > 0)
		fprintf(stdout, "MSW Name? <[\"%s\"]|none>: ", mswname);
	else
		fprintf(stdout, "MSW Name? [none]: ");

	GetInput(stdin, storeb, MSWNAME_LEN);
	if (!strcmp(storeb, "none"))
	{
		memset(mswname, 0, MSWNAME_LEN);
	}
	else if (strlen(storeb))
	{
		strncpy(mswname, storeb, MSWNAME_LEN);
	}

#ifdef OLD_STYLE_FAX_

	fprintf(stdout, "\nfax ftp dir [%s]: ", fax_dir_pathname);
	GetInput(stdin, storeb, 256);
	if (strlen(storeb) > 0) strcpy(fax_dir_pathname, storeb);

	fprintf(stdout, "\nfax ftp user [%s]: ", fax_user);
	GetInput(stdin, storeb, 256);
	if (strlen(storeb) > 0) strcpy(fax_user, storeb);

	sprintf(str, "\nfax ftp password [%s]: ", fax_password);
	if (! (GetPasswd(str,storeb)) )
		strcpy(fax_password, storeb);
#endif

	_billingq:
	radius_config_required = 0;
	fprintf(stdout, "\nBilling:\n--------\nBilling Type? <%s>: ",
			(billingType==BILLING_POSTPAID)?"[postpaid]|ciscoprepaid|none":
				(billingType==BILLING_CISCOPREPAID)?"postpaid|[ciscoprepaid]|none":
					"postpaid|ciscoprepaid|[none]");
	GetInput(stdin, storeb, 20);
	if (strlen(storeb))
	{
		if (!strcmp(storeb, "postpaid"))
		{
			billingType=BILLING_POSTPAID;
		}
		else if (!strcmp(storeb, "ciscoprepaid"))
		{
			billingType=BILLING_CISCOPREPAID;
			radius_config_required = 1;
		}
		else if (!strcmp(storeb, "none"))
		{
			billingType=BILLING_NONE;
		}
		else
			goto _billingq;
	}

	if (billingType!=BILLING_NONE)
	{
		_formatq:
		fprintf(stdout, "Billing Format? <%s>: ",
			"[mind]");

		GetInput(stdin, storeb, 20);
		if (strlen(storeb))
		{
			if (!strcmp(storeb, "mind"))
			{
				cdrformat=CDRFORMAT_MIND;
			}
			else
				goto _formatq;
		}

		_cdrformatq:
		fprintf(stdout, "CDR File Format? <%s>: ",
					(cdrtype==CDRMINDCTIFIXED)?"[fixed]|daily|seq|time":
						((cdrtype==CDRMINDCTIDAILY)?"fixed|[daily]|seq|time":
							((cdrtype==CDRNEXTONETIME)?"fixed|daily|seq|[time]":
								"fixed|daily|[seq]|time")));

		GetInput(stdin, storeb, 256);
		if (strlen(storeb))
		{
			if (!strcmp(storeb, "fixed"))
			{
				 cdrtype = CDRMINDCTIFIXED;
			}
			else if (!strcmp(storeb, "daily"))
			{
				 cdrtype = CDRMINDCTIDAILY;
			}
			else if (!strcmp(storeb, "seq"))
			{
				 cdrtype = CDRMINDCTISEQ;
			}
			else if (!strcmp(storeb, "time"))
			{
				cdrtype = CDRNEXTONETIME;
			}
			else
				goto _cdrformatq;
		}

		if (cdrtype == CDRNEXTONETIME)
		{
			_cdrtimerq:
			fprintf(stdout, "Time (in min) for the CDR File? [%d]:", cdrtimer);
			GetInput(stdin,storeb,5);
			if (strlen(storeb) > 0)
				cdrtimer = atoi(storeb);
			if (cdrtimer > MAX_CDR_TIMER_VALUE)
			{
				fprintf(stderr, "Timer value cannot exceed %d minutes\n", MAX_CDR_TIMER_VALUE);
				goto _cdrtimerq;
			}
		}

		if (!strlen(cdrdirname))
		{
			 strcpy(cdrdirname, ".");
		}

		fprintf(stdout, "CDR Log Directory? [%s]: ", cdrdirname);
		GetInput(stdin, storeb, 256);
		if (strlen(storeb) > 0)
		{
			 strcpy(cdrdirname, storeb);
		}

		_radacctq:
		fprintf(stdout, "Do you want to send Radius accounting messages? [%s]: ",
							rad_acct ? "[yes]|no" : "yes|[no]");
		GetInput(stdin, storeb, 256);
		if (strlen(storeb) > 0)
		{
			if(!strcmp(storeb, "yes"))
			{
				rad_acct = TRUE;
			}
			else if(!strcmp(storeb, "no"))
			{
				rad_acct = FALSE;
			}
			else
			{
				goto _radacctq;
			}
		}
		if(rad_acct)
		{
			radius_config_required = 1;

			if (!strlen(rad_dirname))
			{
				 strcpy(rad_dirname, ".");
			}

			fprintf(stdout, "Radius Log Directory? [%s]: ", rad_dirname);
			GetInput(stdin, storeb, 256);
			if (strlen(storeb) > 0)
			{
				 strcpy(rad_dirname, storeb);
			}

			_radacctsessidq:
			fprintf(stdout, "Do you want to use the overloaded session id format? [%s]: ",
								rad_acct_session_id_overloaded ? "[yes]|no" : "yes|[no]");
			GetInput(stdin, storeb, 256);
			if (strlen(storeb) > 0)
			{
				if(!strcmp(storeb, "yes"))
				{
					rad_acct_session_id_overloaded = TRUE;
				}
				else if(!strcmp(storeb, "no"))
				{
					rad_acct_session_id_overloaded = FALSE;
				}
				else
				{
					goto _radacctsessidq;
				}
			}
		}
		else
		{
			radius_config_required = 0;
		}
	}

	if (billingType==BILLING_CISCOPREPAID)
	{
		if (strlen(first_auth_username) > 0)
			fprintf(stdout, "Authentication Username? <\"%s\">\n: ", first_auth_username);
		else
			fprintf(stdout, "Authentication Username? <>\n: ");
		GetInput(stdin, storeb, 128);
		if (strlen(storeb) > 0)
		{
			strcpy(first_auth_username, storeb);
		}

		if (strlen(first_auth_password) > 0)
			fprintf(stdout, "Authentication Password? <\"%s\">\n: ", first_auth_password);
		else
			fprintf(stdout, "Authentication Password? <>\n: ");
		GetInput(stdin, storeb, 128);
		if (strlen(storeb) > 0)
		{
			strcpy(first_auth_password, storeb);
		}

		if (strlen(second_auth_username) > 0)
			fprintf(stdout, "Authorization Username? <\"%s\">\n: ", second_auth_username);
		else
			fprintf(stdout, "Authorization Username? <>\n: ");
		GetInput(stdin, storeb, 128);
		if (strlen(storeb) > 0)
		{
			strcpy(second_auth_username, storeb);
		}

		if (strlen(second_auth_password) > 0)
			fprintf(stdout, "Authorization Password? <\"%s\">\n: ", second_auth_password);
		else
			fprintf(stdout, "Authorization Password? <>\n: ");
		GetInput(stdin, storeb, 128);
		if (strlen(storeb) > 0)
		{
			strcpy(second_auth_password, storeb);
		}
	}

	/* jserver stuff */
	fprintf(stdout, "\nConfiguration Server:\n---------------------\nConfiguration Server Log File? [%s]: ", jsLogFile);
	GetInput(stdin, storeb, 512);
	if (strlen(storeb) > 0) strcpy(jsLogFile, storeb);

	if (strlen(readPass) > 0)
		fprintf(stdout, "Configuration Read Permission String? <[\"%s\"]|none>\n\t(Enter up to 16 characters): ", readPass);
	else
		fprintf(stdout, "Configuration ReadPermission String? [none]\n\t(Enter up to 16 characters): ");
	GetInput(stdin, storeb, 16);
	if (strlen(storeb) > 0)
	{
	   if (!strcmp(storeb, "none"))
	   {
		  memset(readPass, 0, 16);
	   }
	   else
	   {
		  strcpy(readPass, storeb);
	   }
	}

	if (strlen(writePass) > 0)
		fprintf(stdout, "Configuration Write Permission String? <[\"%s\"]|none>\n\t(Enter up to 16 characters): ", writePass);
	else
		fprintf(stdout, "Configuration Write Permission String? [none]\n\t(Enter up to 16 characters): ");

	GetInput(stdin, storeb, 16);
	if (strlen(storeb) > 0)
	{
	   if (!strcmp(storeb, "none"))
	   {
		  memset(writePass, 0, 16);
	   }
	   else
	   {
		  strcpy(writePass, storeb);
	   }
	}

	_jscomp:
	fprintf(stdout, "Use compression? <%s>: ", jsCompression?"[enable]|disable":"enable|[disable]");
	GetInput(stdin, storeb, 10);
	if (strlen(storeb) > 0)
	{
		if (!strcmp(storeb, "enable"))
		{
			jsCompression = 1;
		}
		else if (!strcmp(storeb, "disable"))
		{
			jsCompression = 0;
		}
		else
			goto _jscomp;
	}


	fprintf(stdout, "\nSIP:\n----\n");
#ifdef _sipdomain
	/* get the sipdomain */
	if (strlen(sipdomain) > 0)
		fprintf(stdout, "SIP Domain? <[\"%s\"]|none>: ", sipdomain);
	else
		fprintf(stdout, "SIP Domain? [none]: ");

	GetInput(stdin, storeb, SIPDOMAIN_LEN);
	if (!strcmp(storeb, "none"))
	{
		memset(sipdomain, 0, SIPDOMAIN_LEN);
	}
	else if (strlen(storeb))
	{
		strncpy(sipdomain, storeb, SIPDOMAIN_LEN);
	}
#endif

	_sipauthq:
	fprintf(stdout, "SIP Authentication? <%s>: ", 
		sipauth==0?"local|radius|[disable]":sipauth==1?"[local]|radius|disable":"local|[radius]|disable");

	GetInput(stdin, storeb, 10);
	if (strlen(storeb) > 0)
	{
		if (!strcmp(storeb, "local"))
		{
			sipauth = 1;
		}
		else if (!strcmp(storeb, "radius"))
		{
			sipauth = 2;
		}
		else if (!strcmp(storeb, "disable"))
		{
			sipauth = 0;
		}
		else
			goto _sipauthq;
	}

	if(sipauth == 2)
	{
		radius_config_required = 1;
	}

	if (sipauth == 1)
	{
		if (strlen(sipauthpassword) > 0)
			fprintf(stdout, "SIP Authentication Password? <[\"%s\"]|none>: ", sipauthpassword);
		else
			fprintf(stdout, "SIP Authentication Password? [none]: ");

		GetInput(stdin, storeb, SIPAUTHPASSWORD_LEN);
		if (!strcmp(storeb, "none"))
		{
			memset(sipauthpassword, 0, SIPAUTHPASSWORD_LEN);
		}
		else if (strlen(storeb))
		{
			strncpy(sipauthpassword, storeb, SIPAUTHPASSWORD_LEN);
		}
	}

	if (strlen(sipservername) > 0)
		fprintf(stdout, "SIP Server Name? <[\"%s\"]|none>: ", sipservername);
	else
		fprintf(stdout, "SIP Server Name? [none]: ");

	GetInput(stdin, storeb, SIPURL_LEN);
	if (!strcmp(storeb, "none"))
	{
		memset(sipservername, 0, SIPURL_LEN);
	}
	else if (strlen(storeb))
	{
		strncpy(sipservername, storeb, SIPURL_LEN);
	}

	_sipserverq:
	fprintf(stdout, "SIP Server Type? <%s>: ",
			(sipservertype==SERVER_REDIRECT)? "[redirect]|proxy|stateful|obp":
			(sipservertype==SERVER_PROXY)? "redirect|[proxy]|stateful|obp":
			(sipservertype==SERVER_PROXYSTATEFULL && !obpEnabled)? "redirect|proxy|[stateful]|obp":
			"redirect|proxy|stateful|[obp]");

	GetInput(stdin, storeb, 25);
	if (strlen(storeb) > 0)
	{
		if (!strcmp(storeb, "redirect"))
		{
			sipservertype=SERVER_REDIRECT;
			obpEnabled = 0;
		}
		else if (!strcmp(storeb, "proxy"))
		{
			sipservertype=SERVER_PROXY;
			obpEnabled = 0;
		}
		else if (!strcmp(storeb, "stateful"))
		{
			sipservertype=SERVER_PROXYSTATEFULL;
			obpEnabled = 0;
		}
		else if (!strcmp(storeb, "obp"))
		{
			sipservertype=SERVER_PROXYSTATEFULL;
			obpEnabled = 1;
		}
		else
			goto _sipserverq;
	}

	if(obpEnabled)
	{
		allowInternalCalling = 1;

		_obpdynamicepq:
		fprintf(stdout, "OBP dynamic endpoint registration? <%s>: ",
			allowDynamicEndpoints ? "[enable]|disable":"enable|[disable]");

		GetInput(stdin, storeb, 25);
		if (strlen(storeb) > 0)
		{
			if (!strcmp(storeb, "enable"))
			{
				allowDynamicEndpoints = 1;
			}
			else if (!strcmp(storeb, "disable"))
			{
				allowDynamicEndpoints = 0;
			}
			else
				goto _obpdynamicepq;
		}
	}

	if(radius_config_required)
	{
		fprintf(stdout, "\nRADIUS:\n-------\n");
		for(i = 0; i < MAX_NUM_RAD_ENTRIES; ++i)
		{
			/* Get Radius server address */
			if (strlen(rad_server_addr[i]) > 0)
				fprintf(stdout, "Radius Server [%d]? <\"%s\">: ", i, rad_server_addr[i]);
			else
				fprintf(stdout, "Radius Server [%d] ? <>: ", i);

			GetInput(stdin, storeb, RADSERVERADDR_LEN);
			if (strlen(storeb))
			{
				strncpy(rad_server_addr[i], storeb, RADSERVERADDR_LEN);
			}

			/* Get Radius server shared secret */
			if (strlen(secret[i]) > 0)
				fprintf(stdout, "Radius Server Shared Secret [%d]? <\"%s\">: ", i, secret[i]);
			else
				fprintf(stdout, "Radius Server Shared Secret [%d]? <>: ", i);

			GetInput(stdin, storeb, SECRET_LEN);
			if (strlen(storeb))
			{
				strncpy(secret[i], storeb, SECRET_LEN);
			}
		}
	}
	else
	{
		for(i = 0; i < MAX_NUM_RAD_ENTRIES; ++i)
		{
			strcpy(rad_server_addr[i], "");
			strcpy(secret[i], "");
		}
		rad_acct = FALSE;
	}

	fprintf(stdout, "\nH.323:\n------\n");

	fprintf(stdout, "H.323 Instances? <%d>: ", nh323CfgInstances);
	GetInput(stdin, storeb, 10);
	if (strlen(storeb))
	{
		nh323CfgInstances = atoi(storeb);
	}

	fprintf(stdout, "H.323 Calls/s (ARQs)? <%d>: ", h323Cps);
	GetInput(stdin, storeb, 10);
	if (strlen(storeb))
	{
		h323Cps = atoi(storeb);
	}

	if (nh323CfgInstances > 1)
	{
		fprintf(stdout, "H.323 Max Calls for Sgatekeepers? <%d>: ", 
			h323maxCallsSgk);
		GetInput(stdin, storeb, 10);
		if (strlen(storeb))
		{
			h323maxCallsSgk = atoi(storeb);
		}
	}

	fprintf(stdout, "H.323 Max Calls? <%d>: ", h323maxCalls);
	GetInput(stdin, storeb, 10);
	if (strlen(storeb))
	{
		h323maxCalls = atoi(storeb);
	}
#if 0	
	fprintf(stdout, "H.323 RAS Max Buffer Size? <%d>: ", h323RasMaxBuffSize);
	GetInput(stdin, storeb, 10);
	if (strlen(storeb))
	{
		int temp;
		temp = atoi(storeb);
		if (temp >= 2048 && temp <= 4096 )
		{
			h323RasMaxBuffSize = temp;
		}
	}
#endif
	fprintf(stdout, "H.323 Information Transfer Capablity? <%s>: ",
		( h323infoTransCap == INFO_TRANSCAP_SPEECH ? "[speech]|unrestricted|restricted|audio|unrestrictedtones|video|pass" :
		( h323infoTransCap == INFO_TRANSCAP_UNRESTRICTED ? "speech|[unrestricted]|restricted|audio|unrestrictedtones|video|pass" :
		( h323infoTransCap == INFO_TRANSCAP_RESTRICTED ? "speech|unrestricted|[restricted]|audio|unrestrictedtones|video|pass" :
		( h323infoTransCap == INFO_TRANSCAP_AUDIO ? "speech|unrestricted|restricted|[audio]|unrestrictedtones|video|pass" :
		( h323infoTransCap == INFO_TRANSCAP_UNRESTRICTEDTONES ? "speech|unrestricted|restricted|audio|[unrestrictedtones]|video|pass" :
		( h323infoTransCap == INFO_TRANSCAP_VIDEO ? "speech|unrestricted|restricted|audio|unrestrictedtones|[video]|pass" :
		 "speech|unrestricted|restricted|audio|unrestrictedtones|video|[pass]" )))))) );

	GetInput(stdin, storeb, 20);
	
	if (strlen(storeb))
	{
		int tmpITC;
		
		tmpITC = str2enum(infoTransCapOptions,storeb);
		if ( tmpITC >=0 && tmpITC != INFO_TRANSCAP_DEFAULT)
		{
			h323infoTransCap = tmpITC;
		}
	}

	_callrouteq:
	fprintf(stdout, "H.323 Routed Calls? <%s>: ", routecall?"[enable]|disable":"enable|[disable]");
	GetInput(stdin, storeb, 10);
	if (strlen(storeb))
	{
		if (!strcmp(storeb, "enable"))
		{
			routecall = 1;
		}
		else if (!strcmp(storeb, "disable"))
		{
			routecall = 0;
		}
		else
			goto _callrouteq;
	}

	_faststartq:
	fprintf(stdout, "H.323 Fast Start? <%s>: ", doFastStart?"[enable]|disable":"enable|[disable]");
	GetInput(stdin, storeb, 10);
	if (strlen(storeb))
	{
		if (!strcmp(storeb, "enable"))
		{
			doFastStart = 1;
		}
		else if (!strcmp(storeb, "disable"))
		{
			doFastStart = 0;
		}
		else
			goto _faststartq;
	}

#if 0
	_forceh245q:
	fprintf(stdout, "Force H.245? <%s>: ", forceh245?"[enable]|disable":"enable|[disable]");
	GetInput(stdin, storeb, 10);
	if (strlen(storeb))
	{
		if (!strcmp(storeb, "enable"))
		{
			forceh245 = 1; routeH245 = 1;
		}
		else if (!strcmp(storeb, "disable"))
		{
			forceh245 = 0;
		}
		else
			goto _forceh245q;
	}
#endif

	if (forceh245 == 0)
	{
		_h245routingq:
		fprintf(stdout, "H.245 Routing? <%s>: ", routeH245?"[enable]|disable":"enable|[disable]");
		GetInput(stdin, storeb, 10);
		if (strlen(storeb))
		{
			if (!strcmp(storeb, "enable"))
			{
				routeH245 = 1;
			}
			else if (!strcmp(storeb, "disable"))
			{
				routeH245 = 0;
			}
			else
				goto _h245routingq;
		}
	}

	_hairpinq:
	fprintf(stdout, "HairPin Calls? <%s>: ", allowHairPin?"[enable]|disable":"enable|[disable]");
	GetInput(stdin, storeb, 10);
	if (strlen(storeb))
	{
		if (!strcmp(storeb, "enable"))
		{
			allowHairPin = 1;
		}
		else if (!strcmp(storeb, "disable"))
		{
			allowHairPin = 0;
		}
		else
			goto _hairpinq;
	}

	_localproceedingq:
	fprintf(stdout, "Local Proceeding? <%s>: ", localProceeding?"[enable]|disable":"enable|[disable]");
	GetInput(stdin, storeb, 10);
	if (strlen(storeb))
	{
		if (!strcmp(storeb, "enable"))
		{
			localProceeding = 1;
		}
		else if (!strcmp(storeb, "disable"))
		{
			localProceeding = 0;
		}
		else
			goto _localproceedingq;
	}

	_h245Tunnelingq:
	fprintf(stdout, "H245 Tunneling ? <%s>: ", h245Tunneling?"[enable]|disable":"enable|[disable]");
	GetInput(stdin, storeb, 10);
	if (strlen(storeb))
	{
		if (!strcmp(storeb, "enable"))
		{
			h245Tunneling = 1;
		}
		else if (!strcmp(storeb, "disable"))
		{
			h245Tunneling = 0;
		}
		else
			goto _h245Tunnelingq;
	}

	_removetcs2833:
	fprintf(stdout, "Remove TCS 2833 CAP? <%s>: ", h323RemoveTcs2833?"[enable]|disable":"enable|[disable]");
	GetInput(stdin, storeb, 10);
	if (strlen(storeb))
	{
		if (!strcmp(storeb, "enable"))
		{
			h323RemoveTcs2833 = 1;
		}
		else if (!strcmp(storeb, "disable"))
		{
			h323RemoveTcs2833 = 0;
		}
		else
			goto _removetcs2833;
	}

	_removetcst38:
	fprintf(stdout, "Remove TCS T38FAX CAP? <%s>: ", h323RemoveTcsT38?"[enable]|disable":"enable|[disable]");
	GetInput(stdin, storeb, 10);
	if (strlen(storeb))
	{
		if (!strcmp(storeb, "enable"))
		{
			h323RemoveTcsT38 = 1;
		}
		else if (!strcmp(storeb, "disable"))
		{
			h323RemoveTcsT38 = 0;
		}
		else
			goto _removetcst38;
	}

	fprintf(stdout, "\nOther:\n------\n");

	/* Make guess now for ip addresses */
	myConfigServerType = CONFIG_SERPLEX_GIS;

	match = FindServerConfig();

	if (match != -1)
	{
		gis = &serplexes[match];
#if 0
		sprintf(addr, "%s", inet_ntop( AF_INET, &gis->location.address.sin_addr, buf, INET_ADDRSTRLEN));
		fprintf(stdout, "Local Server Address? [%s]: ", addr);
		GetInput(stdin, storeb, 256);
		if (strlen(storeb) > 0) 
		{
			if (strcmp(storeb, "local") == 0)
			{
				gis->location.address.sin_addr.s_addr = inet_addr("127.0.0.1");
			}
			else
			{
				gis->location.address.sin_addr.s_addr = inet_addr(storeb);
			}
		}		
#endif
	}

#if 0
	if (redunds)
	{
		gisr = redunds;
		sprintf(addr, "%s", inet_ntop( AF_INET, &redunds->location.address.sin_addr, buf, INET_ADDRSTRLEN));
		fprintf(stdout, "Redundant Server Address? <[\"%s\"]|none>: ", addr);
		GetInput(stdin, storeb, 256);
		if (strlen(storeb) > 0) 
		{
			if (!strcmp(storeb, "none"))
			{
				gisrip = redunds->location.address.sin_addr.s_addr = 0;
			}
			else
			{
				gisrip = redunds->location.address.sin_addr.s_addr = 
					inet_addr(storeb);
			}
		}
		else
		{
			gisrip = redunds->location.address.sin_addr.s_addr;
		}
	}
	else if (gis)
	{
		fprintf(stdout, "Redundant Server Address? []: ");
		GetInput(stdin, storeb, 256);
		if (strlen(storeb) > 0) 
		{
			gisrip = inet_addr(storeb);
		}
	}

	redunds = 0;
	
	if (gis == NULL)
	{
		/* If we didnt find any GIS entries, it may mean that this is
		 * an older release. Try to get information in the old way now.
		 */

		/* Make guess now for ip addresses */
		myConfigServerType = CONFIG_SERPLEX_LUS;

		match = FindServerConfig();

		if (match != -1)
		{
			gis = lus = &serplexes[match];
			sprintf(addr, "%s", inet_ntop( AF_INET, &lus->location.address.sin_addr, buf, INET_ADDRSTRLEN));
			fprintf(stdout, "Local Server Address? [%s]: ", addr);
			GetInput(stdin, storeb, 256);
			if (strlen(storeb) > 0) 
			{
				if (strcmp(storeb, "local") == 0)
				{
					gis->location.address.sin_addr.s_addr = 
					lus->location.address.sin_addr.s_addr = inet_addr("127.0.0.1");
				}
				else
				{
					gis->location.address.sin_addr.s_addr = 
					lus->location.address.sin_addr.s_addr = inet_addr(storeb);
				}
			}		
		}

		if (redunds)
		{
			gisr = redunds;
			sprintf(addr, "%s", inet_ntop( AF_INET, &redunds->location.address.sin_addr, buf, INET_ADDRSTRLEN));
			fprintf(stdout, "Redundant Server Address? <[\"%s\"]|none>: ", addr);
			GetInput(stdin, storeb, 256);
			if (strlen(storeb) > 0) 
			{
				if (!strcmp(storeb, "none"))
				{
					gisrip = redunds->location.address.sin_addr.s_addr = 0;
				}
				else
				{
					gisrip = redunds->location.address.sin_addr.s_addr = 
						inet_addr(storeb);
				}
			}
			else
			{
				gisrip = redunds->location.address.sin_addr.s_addr;
			}
		}
		else
		{
			fprintf(stdout, "Redundant Server Address? []: ");
			GetInput(stdin, storeb, 256);
			if (strlen(storeb) > 0) 
			{
				gisrip = inet_addr(storeb);
			}
		}
	}
#endif

	/* get the enumdomain */
	if (strlen(enumdomain) > 0)
		fprintf(stdout, "ENUM Domain? <[\"%s\"]|none>: ", enumdomain);
	else
		fprintf(stdout, "ENUM Domain? [none]: ");
	GetInput(stdin, storeb, ENUMDOMAIN_LEN);
	if (!strcmp(storeb, "none"))
	{
		memset(enumdomain, 0, ENUMDOMAIN_LEN);
	}
	else if (strlen(storeb))
	{
		strncpy(enumdomain, storeb, ENUMDOMAIN_LEN);
	}

	_allowsrcq:
	fprintf(stdout, "Allow All Source Numbers? <%s>: ", allowSrcAll?"[enable]|disable":"enable|[disable]");
	GetInput(stdin, storeb, 10);
	if (strlen(storeb))
	{
		if (!strcmp(storeb, "enable"))
		{
			allowSrcAll = 1;
		}
		else if (!strcmp(storeb, "disable"))
		{
			allowSrcAll = 0;
		}
		else
			goto _allowsrcq;
	}

	prevFirewall = strcmp(fceConfigFwName, "none");
	fprintf(stdout, "\nFirewall:\n---------\nFirewall Control Proxy? <%s>: ", prevFirewall?"[enable]|disable":"enable|[disable]");
	GetInput(stdin, storeb, 10);
	if ((strlen(storeb) == 0 && prevFirewall) ||
	    (strlen(storeb) > 0 &&
	     (!strcmp(storeb, "enable")))) {

	   /* firewall name */
	   fprintf(stdout, "Firewall name [%s]: ", fceConfigFwName);
	   GetInput(stdin, storeb, 128);
	   if (strlen(storeb) > 0)
		  strcpy(fceConfigFwName, storeb);
#if FCE_REMOVED
	   /* signalling address */
	  keepaskingforip:
	   fprintf(stdout, "IP Address to receive the inbound signalling traffic? [%s]: ", FormatIpAddress(fceConfigOurIpAddr, str));
	   GetInput(stdin, storeb, 16);
	   if (strlen(storeb) > 0)
	   {
		  if ((fceConfigOurIpAddr = ntohl(inet_addr(storeb))) == -1)
		  {
			 fprintf(stdout, "Invalid IP address (%s)\n", storeb);
			 fceConfigOurIpAddr = 0;
			 goto keepaskingforip;
		  }
	   }

	   /* dynamic holes for H.245 signaling */
	   _pinholeq:
	   fprintf(stdout, "Dynamic Pinholes for H.245 Signaling? <%s>: ", (fceH245PinholeEnabled == TRUE)?"[enable]|disable":"enable|[disable]");
	   GetInput(stdin, storeb, 10);
	   if (strlen(storeb) > 0)
	   {
		if (!strcmp(storeb, "enable"))
		{
			fceH245PinholeEnabled = TRUE;
		}
		else if (!strcmp(storeb, "disable"))
		{
			fceH245PinholeEnabled = FALSE;
		}
		else
			goto _pinholeq;
	   }

	   /* default public/private status for addresses */
	   _defaultpublicq:
	   fprintf(stdout, "Default Address Space is Public? <%s>: ", (fceDefaultPublic == TRUE)?"[enable]|disable":"enable|[disable]");
	   GetInput(stdin, storeb, 10);
	   if (strlen(storeb) > 0)
	   {
		if (!strcmp(storeb, "enable"))
		{
			fceDefaultPublic = TRUE;
		}
		else if (!strcmp(storeb, "disable"))
		{
			fceDefaultPublic = FALSE;
		}
		else
			goto _defaultpublicq;
	   }

	   /* default media routing flag */
	   _defaultmediarouting:
	   fprintf(stdout, "Default Media Routing? <%s>: ", defaultMediaRouting?"[enable]|disable":"enable|[disable]");
	   GetInput(stdin, storeb, 10);
	   if (strlen(storeb) > 0)
	   {
		if (!strcmp(storeb, "enable"))
		{
			defaultMediaRouting = 1;
		}
		else if (!strcmp(storeb, "disable"))
		{
			defaultMediaRouting = 0;
		}
		else
			goto _defaultmediarouting;
	   }

	   /* default hide address change flag */
           /*
	   _defaultmidcallmediachange:
	   fprintf(stdout, "Default Mid-call Media Address Change? <%s>: ", defaultHideAddressChange?"[enable]|disable":"enable|[disable]");
	   GetInput(stdin, storeb, 10);
	   if (strlen(storeb) > 0)
	   {
		if (!strcmp(storeb, "enable"))
		{
			defaultHideAddressChange = 1;
		}
		else if (!strcmp(storeb, "disable"))
		{
			defaultHideAddressChange = 0;
		}
		else
			goto _defaultmidcallmediachange;
	   }
           */
#endif
	} else {
	   strcpy(fceConfigFwName, "none");
	}

	// Peering configuration Questions ?

	for (;;)
	{
		fprintf(stdout,	"\n"
						"iServer Peering Configuration:\n"
						"------------------------------\n"
						"Redundant Peer Configuration? <%s>: ",
						(ispd_type != ISPD_TYPE_DISABLED)?"[enable]|disable":"enable|[disable]");

		GetInput(stdin, storeb, 10);

		if (strlen(storeb) > 0)
		{
			if ( !strcmp( storeb, "enable") )
			{
				ispd_type = ISPD_TYPE_ACTIVE;
			}
			else if ( !strcmp(storeb, "disable") )
			{
				ispd_type = ISPD_TYPE_DISABLED;
				break;
			}
			else
				continue;
		}
		else
		{
			// Use the default

			if ( ispd_type == ISPD_TYPE_DISABLED )
				break;	// No peering specified
		}

		// get control interface information

		GetInterfaceInfo( ISPD_CONTROL );

//		generate_rsyncd_conf();

		_scmq:
		fprintf(stdout, "stateful call migration? <%s>: ", doScm?"[enable]|disable":"enable|[disable]");
		GetInput(stdin, storeb, 10);
		if (strlen(storeb))
		{
			if (!strcmp(storeb, "enable"))
			{
				doScm = 1;
			}
			else if (!strcmp(storeb, "disable"))
			{
				doScm = 0;
			}
			else
				goto _scmq;
		}

		break;
	}

	for (;;)
	{
		/* Get the previous type of rsd */
		oldCST = myConfigServerType;
		myConfigServerType = CONFIG_SERPLEX_RSD;
		match = FindServerConfig();
		myConfigServerType = oldCST;

		if (match != -1) {
			rsd = &serplexes[match];
		}

		fprintf(stdout,	"\n"
						"iServer Database Replication Server Configuration:\n"
						"------------------------------\n"
						"Replication Server Configuration? <%s>: ",
						RSDConfig?"[enable]|disable":"enable|[disable]");

		GetInput(stdin, storeb, 10);

		if (strlen(storeb) > 0)
		{
			if ( !strcmp( storeb, "enable") )
				RSDConfig = CONFIG_LOCATION_LOCAL;
			else if ( !strcmp(storeb, "disable") )
			{
				RSDConfig = CONFIG_LOCATION_NONE;
				break;
			}
			else
				continue;
		}
		else
		{
			// Use the default

			if ( RSDConfig == CONFIG_LOCATION_NONE )
				break;	// No database replication specified
		}

		GetRSDInfo();
		break;
	}

	for (;;)
	{
		/* Get the previous type of execd */
		oldCST = myConfigServerType;
		myConfigServerType = CONFIG_SERPLEX_EXECD;
		match = FindServerConfig();
		myConfigServerType = oldCST;

		if (match != -1) {
			execd = &serplexes[match];
		}

		fprintf(stdout,	"\n"
						"iServer Cmd Execution Server Configuration:\n"
						"------------------------------\n"
						"Cmd Execution Server Configured? <%s>: ",
						ExecDConfig?"[enable]|disable":"enable|[disable]");

		GetInput(stdin, storeb, 10);

		if (strlen(storeb) > 0)
		{
			if ( !strcmp( storeb, "enable") )
				ExecDConfig = CONFIG_LOCATION_LOCAL;
			else if ( !strcmp(storeb, "disable") )
			{
				ExecDConfig = CONFIG_LOCATION_NONE;
				break;
			}
			else
				continue;
		}
		else
		{
			// Use the default

			if ( ExecDConfig == CONFIG_LOCATION_NONE )
				break;	// No database replication specified
		}

		GetExecDInfo();
		break;
	}

	for(;;)
	{
		fprintf(stdout,	"\n"
						"iServer Management Interface Configuration:\n"
						"------------------------------\n");
		fprintf(stdout,	"Management Interface Address  ? <[%s]>: ", mgmtInterfaceIp);
		GetInput(stdin, storeb, 128 );

		if (strlen(storeb) > 0)
		{
			unsigned long tmp;
		 	if ((tmp = ntohl(inet_addr(storeb))) == -1)
			{
				fprintf(stdout, "Invalid IP address (%s)\n", storeb);
			}
			else
			{
				strcpy(mgmtInterfaceIp, storeb);
				break;
			}
		}
		else
		{
			break;
		}
	}

	/* Generate the file */
	time(&clock);
	sprintf(str, "# Generated by sconfig - %s\n", ctime_r(&clock, s1));
	GenerateCfgFile(progname, outf, str);

	if (CacheAttach() != -1)
	{
		fprintf(stdout, "\nPlease run <./iserver all reconfig> to activate the changes.\n");
		CacheDetach();
	}

	fprintf(stdout, "\n");
	return(0);
}

int
ConfigIserver(serplex_config *s)
{
	return(0);
}

int
ConfigBcs(serplex_config *s)
{
	return(0);
}

int
ConfigJserver(serplex_config *s)
{
	return(0);
}

int
ConfigFaxs(serplex_config *s)
{
	return(0);
}

int
msSetDebugLevel(int i)
{
	return(0);
}

int
msAdd(char *n)
{
	return(0);
}

int
msDelete(char *n)
{
	return(0);
}

//
//	Function :
//		GetInterfaceInfo()
//
//  Arguments       :
//		None.
//
//	Description :
//		Called from ProcessConfig() subroutine in sconfig
//		to ask the user about ispd Peering Configuration
//		and fill in the related data structures appropriately.
//
//	Return Values:
//		None.
//
static void
GetInterfaceInfo( int iface_type )
{
	char *					iface_type_str = "";
	ispd_ctl_interface_t *	ctl_ptr = NULL;
	ispd_interface_t *		data_ptr = NULL;
	int						interface_count = 0, i, mandatory = 1;
	char *					curname = NULL;
	char					interface_list[256];
	char					ip_addr_str[256];
	int						interface_defined = 0;
	struct ifi_info *		ifi_ptr;

	#define					MAX_IFS	10

	char					interface_names[ MAX_IFS ][ 20 ];

	// Get list of interfaces to choose from

	for	( i = 0; i < MAX_IFS; i++ )
	{
		if ( ( curname = GetNextIfname( ifihead, curname ) ) == NULL )
			break;

		if ( !strncmp( curname, "lo", 2 ) ) 
			continue;

		strcpy( &interface_names[ interface_count ][ 0 ], curname );
		interface_count++;
	}

	// Setup for questions based on specified interface

	switch ( iface_type )
	{
	case ISPD_CONTROL:
		iface_type_str = "Control  ";
		ctl_ptr = &ispd_ctl;
		interface_defined = ispd_ctl.defined;
		mandatory = 1;		// User must specify a control interface
		break;
	}

	// Ask questions about the specified interface.

	switch ( iface_type )
	{
	case ISPD_CONTROL:

		// Manditory - must be defined

		ctl_ptr->defined = 1;

		// Choose control interface name

		for (;;)
		{
			memset( interface_list, (int) 0, 256 );

			for ( i = 0; i < interface_count; i++ )
			{
				if ( !strcmp( &interface_names[ i ][0], ctl_ptr->name ) )
					strcat( interface_list, "[" );

				strcat( interface_list, interface_names[ i ] );

				if ( !strcmp( &interface_names[ i ][0], ctl_ptr->name ) )
					strcat( interface_list, "]" );

				if ( i != (interface_count - 1) )
					strcat( interface_list, "|" );
			}

			fprintf(stdout,	"%s interface name   ? <%s>: ",
							iface_type_str,
							interface_list );

			GetInput(stdin, storeb, 10);

			if (strlen(storeb) > 0)
			{
				if ( !strncmp( storeb, "lo", 2 ) ||
					 ( (ifi_ptr = findIfByIfname( ifihead, storeb )) ==
							(void*) NULL ) )
				{
					fprintf(stdout,	"Invalid interface name, %s \n"
									"please select one from the presented list\n",
									storeb );
				}
				else
				{
					strcpy( ctl_ptr->name, storeb );
					break;
				}
			}
			else
			{
				// Use the default as long as something is specified

				if ( strlen( ctl_ptr->name ) > 0 )
				{
					if ( !strncmp( ctl_ptr->name, "lo", 2 ) ||
						 ( (ifi_ptr = findIfByIfname( ifihead, ctl_ptr->name )) ==
								(void*) NULL ) )
					{
						fprintf(stdout,	"Invalid interface name, %s \n"
										"please select one from the presented list\n",
										ctl_ptr->name );
					}
					else
						break;
				}
			}
		}

		// Choose peer iservers

		ctl_ptr->peer_count = 0;

		for ( i = 0; i < MAX_ISPD_PEERS; i++ )
		{
			for (;;)
			{

				fprintf(stdout,	"%s iServer peer[%1d]  ? <[%s]>: ",
								iface_type_str,
								(ctl_ptr->peer_count + 1),
								&ctl_ptr->peer_iservers[ i ][0] );

				GetInput(stdin, storeb, 128 );

				if (strlen(storeb) > 0)
				{
					// Validate iserver ip address or hostname

					memset( ip_addr_str, (int) 0, 256 );

					if ( validate_host( storeb, ifi_ptr, ip_addr_str ) == 0 )
					{
						strcpy( &ctl_ptr->peer_iservers[ i ][0], storeb );
						ctl_ptr->peer_count++;
						break;
					}
					else
					{

						if ( strlen( ip_addr_str ) == 0 )
						{
							fprintf(stdout,
									"hostname, %s, does not evaluate to "
										"host on %s lan\n"
									"Please enter the hostname or ip of %s "
										"lan iserver\n",
									storeb,
									ctl_ptr->name,
									iface_type_str );
						}
						else
						{
							fprintf(stdout,
									"hostname, %s (%s), does not evaluate "
										"to host on %s lan\n"
									"Please enter the hostname or ip "
										"of %s lan iserver\n",
									storeb,
									ip_addr_str,
									ctl_ptr->name,
									iface_type_str );
						}
					}
				}
				else
				{
					if ( strlen( &ctl_ptr->peer_iservers[ i ][0] ) > 0 )
					{
						// Validate iserver ip address or hostname

						memset( ip_addr_str, (int) 0, 256 );

						if ( validate_host( &ctl_ptr->peer_iservers[ i ][0],
											ifi_ptr,
											ip_addr_str ) == 0 )
						{
							ctl_ptr->peer_count++;
							break;
						}
						else
						{

							if ( strlen( ip_addr_str ) == 0 )
							{
								fprintf(stdout,
										"hostname, %s, does not evaluate to "
											"host on %s lan\n"
										"Please enter the hostname or ip of %s "
											"lan iserver\n",
										&ctl_ptr->peer_iservers[ i ][0],
										ctl_ptr->name,
										iface_type_str );
							}
							else
							{
								fprintf(stdout,
										"hostname, %s (%s), does not evaluate "
											"to host on %s lan\n"
										"Please enter the hostname or ip "
											"of %s lan iserver\n",
										&ctl_ptr->peer_iservers[ i ][0],
										ip_addr_str,
										ctl_ptr->name,
										iface_type_str );
							}
						}
					}

					if ( ctl_ptr->peer_count )
						goto iserversEntered;

					fprintf(stdout,	"At least one peer iServer must be defined\n" );
				}
			}
		}

      iserversEntered:

		break;
	}

	return;
}

//
//	Function :
//		validate_host()
//
//  Arguments       :
//		hostname		name or ip address of host to be evaluated
//
//		ifi_ptr			pointer to ifi_info_t for interface to check
//						against
//
//	Description :
//		This routine is given a character string containing the
//		hostname or ip address of a host. It makes sure that the
//		ip address that the host equates to is on the lan represented
//		by the ip address.
//
//	Return Values:
//		 0		indicates success
//		-1		indicates failure
//
static int
validate_host(	char *				hostname,
				struct ifi_info	*	ifi_ptr,
				char *				output )
{
	ulong_t				addr;
	struct hostent		he;
	struct hostent *	hp = &he;
	char				buffer[256];
	char **				p;
	int					error = 0;
	struct in_addr 		in;
        char                            buf[INET_ADDRSTRLEN];
	uint32_t			ipaddr, netmask;

	// Is it an inet address ?

	if ( (int)( addr = inet_addr( hostname ) ) == -1 )
	{
		// No, Let gethostbyname_r() get the address for us
		// if there is one.

		nx_gethostbyname_r(	hostname,
							hp,
							buffer,
							256,
							&error );

		if ( error != 0 )
		{
			// host information not found for input - return error
			goto errorExit;
		}

		if (hp == NULL)
		{
			// host information not found for input - return error

			goto errorExit;
		}

		p = hp->h_addr_list;

		(void) memcpy( &in.s_addr, *p, sizeof( in.s_addr ) );
		(void) memcpy( &ipaddr, &in.s_addr, sizeof( uint32_t ) );

		// copy ip address to output

		(void) sprintf( output, "%s", inet_ntop( AF_INET, & in , buf, INET_ADDRSTRLEN) );

	}
	else
	{
		// Yes, let FormatIpAddress() convert the address to ascii for us

		FormatIpAddress( htonl( addr), output );

		ipaddr = addr;
	}

	if ( ifi_ptr->ifi_addr == NULL || ifi_ptr->ifi_netmask == NULL )
	{
		goto errorExit;
	}

	(void) memcpy(	&netmask,
					&ifi_ptr->ifi_netmask->sin_addr.s_addr,
					sizeof( uint32_t ) );

	// Is specified hostname on appropriate interface ?

	if ( ( ipaddr & netmask ) ==
		 ( ifi_ptr->ifi_addr->sin_addr.s_addr & netmask ) )
	{
		return(0);
	}

errorExit:

	return(-1);
}

//
//	Function :
//		generate_rsyncd_conf()
//
//  Arguments       :
//		None
//
//	Description :
//		This routine generates an rsyncd.conf file in the /etc
//		directory based on the ispd configuration input.
//
//	Return Values:
//		None
//
static void
generate_rsyncd_conf(	void )
{
	FILE *				cmdfile;
	struct ifi_info *	ifi_ptr;
	struct in_addr		netaddr, netmask;
        char                    buf[INET_ADDRSTRLEN];

	char				allow_netaddr[32];
	char				allow_netmask[32];

	char				command[2048];

	// The following command builds the rsyncd.conf file in the
	// /etc directory

	char *	command_string = 
		"/bin/sh  \'"
			"PATH=/usr/bin:/sbin:/bin;"
			"echo \"# rsyncd.conf\" >/etc/rsyncd.conf;"
			"echo \"#\" >>/etc/rsyncd.conf;"
			"echo \"#motd file = /etc/motd\" >>/etc/rsyncd.conf;"
			"echo \"#max connections = 5\" >>/etc/rsyncd.conf;"
			"echo \"log file = /tmp/rsync.log\" >>/etc/rsyncd.conf;"
			"echo \"#syslog facility = local5\" >>/etc/rsyncd.conf;"
			"echo \"dont compress = * *.*\" >>/etc/rsyncd.conf;"
			"echo \"\" >>/etc/rsyncd.conf;"
			"echo \"[nextone]\" >>/etc/rsyncd.conf;"
			"echo \"\tcomment = nextone area\" >>/etc/rsyncd.conf;"
			"echo \"\tpath = /\" >>/etc/rsyncd.conf;"
			"echo \"\tread only = no\" >>/etc/rsyncd.conf;"
			"echo \"\tauth users = root\" >>/etc/rsyncd.conf;"
			"echo \"#\tsecrets file = /etc/rsyncd.secrets\" >>/etc/rsyncd.conf;"
			"echo \"\tuid = root\" >>/etc/rsyncd.conf;"
			"echo \"\tgid = root\" >>/etc/rsyncd.conf;"
			"echo \"\ttimeout = 300\" >>/etc/rsyncd.conf;"
			"echo \"\thosts allow = %s/%s\" >>/etc/rsyncd.conf;\'";


	ifi_ptr = findIfByIfname( ifihead, ispd_ctl.name );

	netaddr.s_addr = (in_addr_t)
			(	(ulong) ifi_ptr->ifi_addr->sin_addr.s_addr &
				(ulong) ifi_ptr->ifi_netmask->sin_addr.s_addr );

	netmask = ifi_ptr->ifi_netmask->sin_addr;

	sprintf( allow_netaddr, "%s", inet_ntop( AF_INET, & netaddr , buf, INET_ADDRSTRLEN) );
	sprintf( allow_netmask, "%s", inet_ntop( AF_INET, & netmask , buf, INET_ADDRSTRLEN) );

	sprintf( command, command_string, allow_netaddr, allow_netmask );

	//
	// Issue command to build rsyncd.conf file
	//

	if ( ( cmdfile = popen( command, "r" ) ) >= 0 )
	{
		pclose( cmdfile );
	}

	return;
}

static void
GetRSDInfo(void)
{
	char 					*curname = NULL;
	int						interface_count = 0, i;
	char					interface_list[256];
	struct ifi_info 		*ifi_ptr;

	#define					MAX_IFS	10

	char					interface_names[ MAX_IFS ][ 20 ];

	// Get list of interfaces to choose from

	for	( i = 0; i < MAX_IFS; i++ )
	{
		if ( ( curname = GetNextIfname( ifihead, curname ) ) == NULL )
			break;

		if ( !strncmp( curname, "lo", 2 ) ) 
			continue;

		strcpy( &interface_names[ interface_count ][ 0 ], curname );
		interface_count++;
	}

	// Choose interface name

	for (;;)
	{
		memset( interface_list, (int) 0, 256 );

		for ( i = 0; i < interface_count; i++ )
		{
			if ( !strcmp( &interface_names[ i ][0], rs_ifname ) )
				strcat( interface_list, "[" );

			strcat( interface_list, interface_names[ i ] );

			if ( !strcmp( &interface_names[ i ][0], rs_ifname ) )
				strcat( interface_list, "]" );

			if ( i != (interface_count - 1) )
				strcat( interface_list, "|" );
		}

		fprintf(stdout,	"%s interface name   ? <%s>: ",
						"replication server",
						interface_list );

		GetInput(stdin, storeb, 10);

		if (strlen(storeb) > 0)
		{
			// Validate interface
			if ( !strncmp( storeb, "lo", 2 ) ||
				 ( (ifi_ptr = findIfByIfname( ifihead, storeb )) ==
						(void*) NULL ) )
			{
				fprintf(stdout,	"Invalid interface name, %s \n"
								"please select one from the presented list\n",
								storeb );
			}
			else
			{
				strcpy( rs_ifname, storeb );
				break;
			}
		}
		else
		{
			// Use the default as long as something is specified

			if ( strlen( rs_ifname ) > 0 )
			{
					if ( !strncmp( rs_ifname, "lo", 2 ) ||
						 ( (ifi_ptr = findIfByIfname( ifihead, rs_ifname )) ==
								(void*) NULL ) )
					{
						fprintf(stdout,	"Invalid interface name, %s \n"
										"please select one from the presented list\n",
										rs_ifname );
					}
					else
						break;
			}
		}
	}

	// Choose multicast address 

	for (;;)
	{
		fprintf(stdout,	"multicast address for replication server? <[%s]>: ",
					rs_mcast_addr );

		GetInput(stdin, storeb, 128 );

		if (strlen(storeb) > 0)
		{
			// Can validate mcast ip address or hostname

			strcpy( rs_mcast_addr, storeb );
			break;

		}
		else
		{
			// Use whatever is there as long as something is specified

			if ( strlen( rs_mcast_addr ) )
				break;

			fprintf(stdout, "use default multicast address - %s\n", RS_DEF_MCAST_ADDR);
		}
	}

	// Choose multicast port 

	for (;;)
	{
		fprintf(stdout,	"multicast port for replication server? <[%s]>: ",
					rs_port );

		GetInput(stdin, storeb, 128 );

		if (strlen(storeb) > 0)
		{
			// Can validate mcast ip address or hostname

			strcpy( rs_port, storeb );
			break;

		}
		else
		{
			// Use whatever is there as long as something is specified

			if ( strlen( rs_port ) )
				break;

			fprintf(stdout, "use default multicast port - %s\n", RS_DEF_PORT);
		}
	}

	for (;;)
	{
		fprintf(stdout,	"This host's priority for database controller? <[%d]>: ",
					rs_host_prio );

		GetInput(stdin, storeb, 128 );

		if (strlen(storeb) > 0)
		{
			// Can validate host priority here

			rs_host_prio = atoi(storeb);
			break;

		}
		else
		{
			// Use whatever is there as long as something is specified

			if ( rs_host_prio >= 0 )
				break;
		}
	}
}

static void
GetExecDInfo(void)
{
}
