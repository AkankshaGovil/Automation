#include <stdio.h>
#include <stdlib.h>
#include <sys/syslog.h>
#include <limits.h>
#include <fcntl.h>
#include <unistd.h>
#include <ctype.h>

#include "xmlparse.h"
#include "spversion.h"
#include "serverp.h"
#include "mem.h"
#include "srvrlog.h"
#include "license.h"
#include "licenseIf.h"
#include "cacheinit.h"
#include "md5.h"
#include <malloc.h>

// FROM xlicense.c
//#include <stdlib.h>

#define TRUE    1
#define FALSE   0
#define OK      0



/* Prototypes for forward referenced internal routines */
static int readXmlLicense(void);

#ifndef LIC_BUFSIZE
#define LIC_BUFSIZE        2048
#endif
#define SIGSIZE		128
#define XML_LICENSE_FILE "/usr/local/nextone/bin/iserverlc.xml"
#define DEFAULT_COMPANY "NexTone Customer"
#define DEFAULT_PRODUCT "iServer"
#define DEFAULT_VERSION (VERSION + 1)
#define EXPIRES_TIME_FORMAT "%a %b %d %T %Y"

/* Data storage for license values */
static int   vport_alarm_index = 0;
static int   mrvport_alarm_index = 0;
#if 0
static char  *companyName;
static char  *productName;
static char  *productVersion;
#endif

char	*xmlLicenseFile = NULL;

typedef struct _server {
	int   serverValid;
	char *serverIP;
	char *serverHostid;
	char  serverMac[MAC_ADDR_LEN+1];
} ServerData;

/* Structure used temporarily while license is being read */
typedef struct _licdata {
	char  *companyName;
	char  *productName;
	char  *productVersion;
	char  *licenseVersion;
	time_t expiry_time;		/* 0 = never expires */
	time_t initTime;
	time_t termTime;
	int    maxEndpoints;
	int    maxCalls;
	int    maxMRCalls;
	int    maxRealms;
	int    maxRoutes;
	int    maxDynamicEP;
	int    features;
	char   sigComputed[SIGSIZE];		/* Store computed signature here */
	char   sigRead[SIGSIZE];			/* Signature read from file goes here */
	int    sigLength;		/* Length of both signature buffers (SIGSIZE) */
	char   buffer[LIC_BUFSIZE];			/* File buffer, compute signature on this buffer */
	int    bufferLen;		/* Length of buffer (LIC_BUFSIZE) */
	int    bufOffset;		/* Where to store next char into buffer */
	List   serverList;		/* List of <SERVER ... /> from license file */
	int    serverValid;		/* True if at least one server in the above list is valid */
	int    sigValid;		/* License signature is valid */
	int    sigReadLen;		/* Actual length of signature read from the file */
	char	macstr[MAC_ADDR_LEN+1];
} LicenseData;

static LicenseData license = {0};

static unsigned char sig_xx[] = {104,107,71,135,118,205,144,120,155,127,134,224,21,79,47};

char featureList[MAX_FEATURES][FEATURE_NAME_LEN] = 
	{
		"H323","SIP","FCE","RADIUS","GEN", "NAT", "SCM", "SIPT", "REGISTRAR", "CAC"
	};

#define	H323_ENABLED		0x001
#define SIP_ENABLED			0x002
#define FCE_ENABLED 		0x004
#define RADIUS_ENABLED 		0x008
#define GEN_ENABLED 		0x010
#define NAT_ENABLED 		0x020
#define SCM_ENABLED 		0x040
#define SIPT_ENABLED 		0x080
#define REGISTRAR_ENABLED 	0x100
#define CAC_ENABLED 		0x200


void nlm_setconfigport(int n)
{
	lsMem->usedlic = n;
	DEBUG(MLMGR,NETLOG_DEBUG4,("numConfigPort = %d\n",n));
}

int nlm_getconfigport(void)
{
	return lsMem->usedlic;
}


/* returns 0 on success -1 on failure */
int license_init(void)
{
	int ret;

	ret = readXmlLicense();

	if (ret == 0)
	{
		NETDEBUG(MLMGR,NETLOG_DEBUG4,
			("license_init Max Calls = %d MR Calls = %d Expiry  = %s \n", lsMem->maxCalls, lsMem->maxMRCalls, ctime(&lsMem->expiry_time)));
	}
	else
	{
		ERROR (MLMGR, ("license_init: invalid license!"));
		return(-1);
	}


	return 0;
}	

int nlm_initConfigPort()
{
	DEBUG(MLMGR,NETLOG_DEBUG4,
		("license_init Max Calls = %d Expiry  = %s \n",lsMem->maxCalls, ctime(&lsMem->expiry_time)));

	return 0;
}

#if 0
/* returns 0 on success -1 on failure */
int license_allocate(int n)
{
   int status = OK;
   char *addr;
   int  shmId;

	shmId = CacheAttach();
	if (shmId <0)
	{
		ERROR(MLMGR,("CacheAttach Failed \n"));
		return -1;
	}

	DEBUG(MLMGR,NETLOG_DEBUG4,("license_allocate Requested license = %d"
		" Total licence count = %d \nAlready used licences = %d\n",
		n,lsMem->nlic,lsMem->usedlic));


	if (n <= lsMem->nlic - lsMem->usedlic)
	{
		lsMem->usedlic += n;
		status = OK;
		goto _return;
	}

	/* Error case */
	status = -1;

                ERROR(MLMGR,("Failed to obtain license Total licence count = %d"
		" used licences count = %d\n",lsMem->nlic,lsMem->usedlic));


_return :

	CacheDetach();

	return status;

}
#endif

#if 0
void license_release(int n)
{
   char *addr;
   int  shmId;

	shmId = CacheAttach();

	if (shmId < 0)
	{
		ERROR(MLMGR,("CacheAttach Failed \n"));
		return;
	}

	lsMem->usedlic -= n;

	DEBUG(MLMGR,NETLOG_DEBUG4,("license_release Released  license = %d"
		" Total licence count = %d used licences count = %d\n",
		n,lsMem->nlic,lsMem->usedlic));

	CacheDetach();

	return ;

}
#endif

void license_display(void)
{
   char *addr;
   int  shmId;

	shmId = CacheAttach();

	if (shmId < 0)
	{
		ERROR(MLMGR,("CacheAttach Failed \n"));
		return;
	}

	DEBUG(MLMGR,NETLOG_DEBUG4,	
	("license_display: Licensed Endpoints = %d Configured Endpoints = %d\n",
	lsMem->nlic,lsMem->usedlic));

	DEBUG(MLMGR,NETLOG_DEBUG4,	
	("license_display: Maximum Simultaneous Calls = %d\n",
	lsMem->maxCalls));

	CacheDetach();

	return;
}

/* returns 0 if a vport is available  - else -1 */
/* expects to be called from inside confCache Locks */
int nlm_getvport()
{
	static char 	fn[] = "nlm_getvport";
	time_t cur_time;
	int prevIndex, oldValue;

	if (confCache && (confCache->nitems >= lsMem->maxCalls))
	{
		NETDEBUG(MLMGR,NETLOG_DEBUG4, ("%s: Licensed Calls = %d Active Calls = %d\n",
					fn,lsMem->maxCalls,confCache->nitems));
		time(&cur_time);
		LockGetLock(&(lsMem->alarmmutex), LOCK_WRITE, LOCK_BLOCK);
		oldValue = vport_alarm_index;
		if(vport_alarm_index >= MAX_LS_ALARM)
			vport_alarm_index = 0;

		prevIndex = (vport_alarm_index == 0)?MAX_LS_ALARM-1:vport_alarm_index-1;
		if (cur_time > (lsMem->lsVportAlarm[prevIndex] + LS_ALARM_INTERVAL))
		{
			lsMem->lsVportAlarm[vport_alarm_index] = cur_time;
			vport_alarm_index++;
		}
		else
			vport_alarm_index = oldValue;

		LockReleaseLock(&(lsMem->alarmmutex));
		return -1;
	}
	return 0;
}

/* returns 0 if a vport is available  - else -1 */
int nlm_getMRvport(void)
{
	static char 	fn[] = "nlm_getMRvport";
	time_t cur_time;
	int prevIndex, oldValue;

	if(lsMem->nMRCalls >= lsMem->maxMRCalls)
	{
		NETDEBUG(MLMGR,NETLOG_DEBUG4, ("%s: Licensed MR vports = %d Active MR vports = %d\n",
					fn,lsMem->nMRCalls,lsMem->maxMRCalls));
		time(&cur_time);
		LockGetLock(&(lsMem->alarmmutex), LOCK_WRITE, LOCK_BLOCK);
		oldValue = mrvport_alarm_index;
		if(mrvport_alarm_index >= MAX_LS_ALARM)
			mrvport_alarm_index = 0;

		prevIndex = (mrvport_alarm_index == 0)?MAX_LS_ALARM-1:mrvport_alarm_index-1;
		if (cur_time > (lsMem->lsMRVportAlarm[prevIndex] + LS_ALARM_INTERVAL))
		{
			lsMem->lsMRVportAlarm[mrvport_alarm_index] = cur_time;
			mrvport_alarm_index++;
		}
		else
			mrvport_alarm_index = oldValue;

		LockReleaseLock(&(lsMem->alarmmutex));
		return -1;
	}
	lsMem->nMRCalls++;
	return 0;
}

/* returns 0 if a vport is available  - else -1 */
int nlm_freeMRvport(void)
{
	static char 	fn[] = "nlm_freeMRvport";
	time_t cur_time;
	if(lsMem->nMRCalls <= 0)
	{
		NETERROR(MLMGR,("%s: Trying to free more than allocated\n", fn));
		return -1;
	}
	lsMem->nMRCalls--;
	return 0;
}

/* returns number of activecalls/vports */
int nlm_getUsedvport()
{
	static char 	fn[] = "nlm_getUsedvport";
	int				activeCalls;

	CacheGetLocks( confCache,LOCK_READ,LOCK_BLOCK);
	activeCalls = confCache->nitems;
	CacheReleaseLocks(confCache);
	return activeCalls;
}

/* return number of activecalls/vports without locking the call cache */
int nlm_getUsedvportNolock ()
{
	return confCache->nitems;
}

/* return number of active MR vports without locking the call cache */
int nlm_getUsedMRvportNolock ()
{
	return lsMem->nMRCalls;
}

int nlm_isExpired(void)
{
	return (time(NULL) > lsMem->expiry_time);
}

int sipEnabled()
{
	return (lsMem->features&SIP_ENABLED);
}

int fceEnabled()
{
	return (lsMem->features&FCE_ENABLED);
}

int radiusEnabled()
{
	return (lsMem->features&RADIUS_ENABLED);
}

int h323Enabled()
{
	return (lsMem->features&H323_ENABLED);
}

int genEnabled()
{
	return (lsMem->features&GEN_ENABLED);
}

int natEnabled() { return (lsMem->features&NAT_ENABLED); } 
int scmEnabled() { return (lsMem->features&SCM_ENABLED); } 
int siptEnabled() { return (lsMem->features&SIPT_ENABLED); } 
int registrarEnabled() { return (lsMem->features&REGISTRAR_ENABLED); } 
int cacEnabled() { return (lsMem->features&CAC_ENABLED); }

char * nlm_getFeatureList(char flist[MAX_FEATURES*FEATURE_NAME_LEN])
{
	int 	i;
	char 	*p= flist;
	for(i=0;i<MAX_FEATURES;++i)
	{
		if((lsMem->features & (1<<i)))
		{
			sprintf(p,"%s\t",featureList[i]);
			p += strlen(p);
		}
	}
	return flist;
}



/* Routines to handle XML formatted license */

void writeXmlLicense (void)
{
	char *tmp;
	char *tmp1;
	char buffer[LIC_BUFSIZE];
	int offset = 0;
	unsigned char sig[SIGSIZE];
	int siglen=0;
	int err=0;
	int i;
	int count = 0;
	int fh;
	int saveOffset;
	struct tm tm;
	FILE *f;
	ServerData *sd;

	offset += sprintf (&buffer[offset], "<LICENSE ");

	tmp = license.companyName ? license.companyName : DEFAULT_COMPANY;
	offset += sprintf (&buffer[offset], "CUSTOMER = '%s' ", tmp);

	tmp1 = license.licenseVersion ? license.licenseVersion : DEFAULT_VERSION;
	tmp = license.productName ? license.productName : DEFAULT_PRODUCT;
	offset += sprintf (&buffer[offset], "VERSION = '%s' PRODUCT = '%s' ", tmp1, tmp);
	tmp1 = license.productVersion ? license.productVersion : DEFAULT_VERSION;
	offset += sprintf (&buffer[offset], "PRODUCTVERSION = '%s' EXPIRES = '", tmp1);
	if (lsMem->expiry_time)
	{
		localtime_r(&lsMem->expiry_time, &tm);
		asctime_r (&tm, &buffer[offset]);
		offset += 24;		// We want to overwrite the newline that asctime_r adds
		offset += sprintf (&buffer[offset], "'>\n");
	}
	else
	{
		offset += sprintf (&buffer[offset], "timeless'>\n");
	}

	if(license.serverList)
	{
		count = listItems(license.serverList);
	}

	if (count>0)
	{
		sd = listGetFirstItem(license.serverList);
		do {
			offset += sprintf (&buffer[offset], "    <SERVER ");
			if (sd->serverIP)
			{
				offset += sprintf (&buffer[offset], "ip='%s' ", sd->serverIP);
			}
			if ( strlen(sd->serverMac) )
			{
				if(strcasecmp(sd->serverMac, "unbound") == 0)
				{
					offset += sprintf (&buffer[offset], "mac='unbound' ");
				}
				else
				{
					offset += sprintf (&buffer[offset], "mac='");
					for (i=0; i<MAC_ADDR_LEN-2; i+=2)
					{
						offset += sprintf (&buffer[offset], "%c%c:", sd->serverMac[i], sd->serverMac[i+1]);
					}
					offset += sprintf (&buffer[offset], "%c%c' ", sd->serverMac[i], sd->serverMac[i+1]);
				}
			}
			if (sd->serverHostid)
			{
				offset += sprintf (&buffer[offset], "hostid='%s' ", sd->serverHostid);
			}
			offset += sprintf (&buffer[offset], "/>\n");
		} while (sd=listGetNextItem(license.serverList, sd));
	}

	offset += sprintf (&buffer[offset], "    <FEATURES ");
	i = (lsMem->features & NAT_ENABLED)?1:0;
	offset += sprintf (&buffer[offset], "NAT='%d' ", i);
	i = (lsMem->features & SCM_ENABLED)?1:0;
	offset += sprintf (&buffer[offset], "SCM='%d' ", i);
	i = (lsMem->features & SIPT_ENABLED)?1:0;
	offset += sprintf (&buffer[offset], "SIPT='%d' ", i);
	i = (lsMem->features & REGISTRAR_ENABLED)?1:0;
	offset += sprintf (&buffer[offset], "REGISTRAR='%d' ", i);
	i = (lsMem->features & SIP_ENABLED)?1:0;
	offset += sprintf (&buffer[offset], "SIP='%d' ", i);
	i = (lsMem->features & H323_ENABLED)?1:0;
	offset += sprintf (&buffer[offset], "H323='%d' ", i);
	i = (lsMem->features & FCE_ENABLED)?1:0;
	offset += sprintf (&buffer[offset], "FCE='%d' ", i);
	i = (lsMem->features & RADIUS_ENABLED)?1:0;
	offset += sprintf (&buffer[offset], "RADIUS='%d' ", i);
	i = (lsMem->features & CAC_ENABLED)?1:0;
	offset += sprintf (&buffer[offset], "CAC='%d' ", i);
	i = (lsMem->features & GEN_ENABLED)?1:0;
	offset += sprintf (&buffer[offset], "GEN='%d' ", i);

	offset += sprintf (&buffer[offset], "MAXEndpoints='%d' ", lsMem->nlic);
	offset += sprintf (&buffer[offset], "MAXCalls='%d' ", lsMem->maxCalls);
	offset += sprintf (&buffer[offset], "MRVport='%d' ", lsMem->maxMRCalls);
	offset += sprintf (&buffer[offset], "Realms='%d' ", lsMem->maxRealms);
	offset += sprintf (&buffer[offset], "DynamicEndpoints='%d' ", lsMem->maxDynamicEP);
	offset += sprintf (&buffer[offset], "Routes='%d' ", lsMem->maxRoutes);
	offset += sprintf (&buffer[offset], "/>\n");

	if(count == 0)
	{
		offset += sprintf (&buffer[offset], "    <SERVER hostid='%s'/>\n", lsMem->macstr);
	}

// ENF section
	offset += sprintf (&buffer[offset], "    <ENF TERM='%lu' INIT='%lu' />\n", lsMem->termTime, lsMem->initTime);

//	Compute signature
	saveOffset = offset;
	offset += sprintf (&buffer[offset], "</LICENSE>\n");
	for (i=0; i<sizeof(sig_xx); i++)
	{
		offset += sprintf(&buffer[offset], "%02x", sig_xx[i]&0xFF);
	}
	MD5(buffer, offset, sig);
	offset = saveOffset;

// SIGNATURE
	offset += sprintf (&buffer[offset], "    <SIGNATURE id='");
	for (i=0; i<MD5_DIGEST_LENGTH; i++)
	{
		offset += sprintf (&buffer[offset], "%02x",(sig[i]&0xFF));
	}
	offset += sprintf (&buffer[offset], "'/>\n");

	offset += sprintf (&buffer[offset], "</LICENSE>\n");

	if(xmlLicenseFile == NULL)
	{
		xmlLicenseFile = XML_LICENSE_FILE;
	}

	// Write the file
	fh = open (xmlLicenseFile, O_WRONLY|O_CREAT|O_TRUNC, 0644);
	if (fh < 0)
	{
		NETDEBUG (MLMGR, NETLOG_DEBUG4, ("Cannot open %s for writing, %s (%d)\n", xmlLicenseFile,
					strerror(errno), errno));
		exit(3);
	}
	err = write (fh, buffer, offset);
	if (err < 0)
	{
		NETDEBUG (MLMGR, NETLOG_DEBUG4, ("Cannot write %s, %s (%d)\n", xmlLicenseFile, strerror(errno), errno));
		exit(4);
	}
	close (fh);
}

int nlm_getInitTime(void)
{
	return (lsMem->initTime);
}

void nlm_setInitTime(time_t t)
{
	lsMem->initTime = t;
	writeXmlLicense();
}

extern time_t altzone, timezone;

static void handleLicenseParams (LicenseData *ld, const char **atts)
{
	int i;
	char *ret;
	struct tm tm;

	for (i=0;  atts[i]!=NULL; i+=2)
	{
		if (strcasecmp(atts[i], "customer") == 0)
		{
			if (ld->companyName)
				free(ld->companyName);

			ld->companyName = strdup(atts[i+1]);
		}
		else if (strcasecmp(atts[i], "product") == 0)
		{
			if (ld->productName)
				free(ld->productName);

			ld->productName = strdup(atts[i+1]);
		}
		else if (strcasecmp(atts[i], "productversion") == 0)
		{
			if (ld->productVersion)
				free (ld->productVersion);

			ld->productVersion = strdup(atts[i+1]);
		}
		else if (strcasecmp(atts[i], "version") == 0)
		{
			if (ld->licenseVersion)
				free (ld->licenseVersion);

			ld->licenseVersion = strdup(atts[i+1]);
		}
		else if (strcasecmp(atts[i], "expires") == 0)
		{
			if (strcasecmp(atts[i+1], "timeless")==0)
			{
				ld->expiry_time = 0;
			}
			else
			{
				ret = strptime(atts[i+1], EXPIRES_TIME_FORMAT, &tm);
				if (ret == NULL)
				{
					ERROR (MLMGR, ("Invalid expires time '%s'\n", atts[i+1]));
				}
				else
				{
					ld->expiry_time = mktime(&tm);
				}
			}
   			NETDEBUG (MLMGR, NETLOG_DEBUG4, ("expires time set to %lu\n", ld->expiry_time));
		}
		else
		{
			ERROR (MLMGR, ("Unknown License parameter %s ignored.\n", atts[i]));
		}
	}
}

static void handleServerParams (LicenseData *ld, const char **atts)
{
	int i;
	int j;
	char *s;
	ServerData *srv;

	srv = malloc(sizeof(ServerData));
	if (srv == NULL)
	{
		ERROR (MLMGR, ("cannot malloc serverdata structure\n"));
		return;
	}

	memset (srv, 0, sizeof(ServerData));

	for (i=0;  atts[i]!=0; i+=2)
	{
		if (strcasecmp(atts[i], "ip") == 0)
		{
			NETDEBUG (MLMGR, NETLOG_DEBUG4, ("Found ip='%s', ignored\n", atts[i+1]));
			srv->serverIP = strdup(atts[i+1]);
		}
		else if (strcasecmp(atts[i], "hostid") == 0)
		{
			s = (char *)atts[i+1];
			srv->serverHostid = strdup(s);

			if (strcasecmp (s, "unbound") == 0)
			{
				// don't allow timeless and unbound
				if(ld->expiry_time != 0)
				{
					srv->serverValid = 1;
				}
			}
			else
			{
				if (validateHostid(s) == 0)
				{
					srv->serverValid = 1;
					strcpy(ld->macstr, s);
				}
			}
		}
		else if (strcasecmp(atts[i], "mac") == 0)
		{
			s = (char *)atts[i+1];
			if (strcasecmp (s, "unbound") == 0)
			{
				strcpy(srv->serverMac, s);

				// don't allow timeless and unbound
				if(ld->expiry_time != 0)
				{
					srv->serverValid = 1;
				}
			}
			else
			{
				for (j=0; j<MAC_ADDR_LEN, *s; s++)
				{
					if (isxdigit(*s))
						srv->serverMac[j++] = *s;
				}
				if (validateHwAddress(j,srv->serverMac) == 0)
				{
					srv->serverValid = 1;
					strcpy(ld->macstr, srv->serverMac);
				}
			}
		}
		else
			ERROR (MLMGR, ("Unknown Server parameter %s ignored.\n", atts[i]));
	}
	listAddItem (ld->serverList, srv);
}

static void handleFeatureParams (LicenseData *ld, const char **atts)
{
	int i;

	for (i=0;  atts[i]!=0; i+=2)
	{
		if (strcasecmp(atts[i], "H323") == 0)
		{
			if (atoi(atts[i+1]) > 0)
				ld->features |= H323_ENABLED;
		}
		else if (strcasecmp(atts[i], "SIP") == 0)
		{
			if (atoi(atts[i+1]) > 0)
				ld->features |= SIP_ENABLED;
		}
		else if (strcasecmp(atts[i], "FCE") == 0)
		{
			if (atoi(atts[i+1]) > 0)
				ld->features |= FCE_ENABLED;
		}
		else if (strcasecmp(atts[i], "GEN") == 0)
		{
			if (atoi(atts[i+1]) > 0)
				ld->features |= GEN_ENABLED;
		}
		else if (strcasecmp(atts[i], "NAT") == 0)
		{
			if (atoi(atts[i+1]) > 0)
				ld->features |= NAT_ENABLED;
		}
		else if (strcasecmp(atts[i], "SCM") == 0)
		{
			if (atoi(atts[i+1]) > 0)
				ld->features |= SCM_ENABLED;
		}
		else if (strcasecmp(atts[i], "SIPT") == 0)
		{
			if (atoi(atts[i+1]) > 0)
				ld->features |= SIPT_ENABLED;
		}
		else if (strcasecmp(atts[i], "RADIUS") == 0)
		{
			if (atoi(atts[i+1]) > 0)
				ld->features |= RADIUS_ENABLED;
		}
		else if (strcasecmp(atts[i], "REGISTRAR") == 0)
		{
			if (atoi(atts[i+1]) > 0)
				ld->features |= REGISTRAR_ENABLED;
		}
		else if (strcasecmp(atts[i], "CAC") == 0)
		{
			if (atoi(atts[i+1]) > 0)
				ld->features |= CAC_ENABLED;
		}
		else if (strcasecmp(atts[i], "maxendpoints") == 0)
			ld->maxEndpoints = atoi(atts[i+1]);
		else if (strcasecmp(atts[i], "maxcalls") == 0)
			ld->maxCalls = atoi(atts[i+1]);
		else if (strcasecmp(atts[i], "mrvport") == 0)
			ld->maxMRCalls = atoi(atts[i+1]);
		else if (strcasecmp(atts[i], "realms") == 0)
			ld->maxRealms = atoi(atts[i+1]);
		else if (strcasecmp(atts[i], "routes") == 0)
			ld->maxRoutes = atoi(atts[i+1]);
		else if (strcasecmp(atts[i], "dynamicendpoints") == 0)
			ld->maxDynamicEP = atoi(atts[i+1]);
		else
			ERROR (MLMGR, ("Unknown Feature parameter %s ignored.\n", atts[i]));
	}
}

static void handleEnfParams (LicenseData *ld, const char **atts)
{
	int i;

	for (i=0;  atts[i]!=0; i+=2)
	{
		if (strcasecmp(atts[i], "init") == 0)
			ld->initTime = atoi(atts[i+1]);
		else if (strcasecmp(atts[i], "term") == 0)
			ld->termTime = atoi(atts[i+1]);
		else
			ERROR (MLMGR, ("Unknown enf parameter %s ignored.\n", atts[i]));
	}
}

static unsigned int asc2hex (const unsigned char c)
{
	if (c >= '0' && c <= '9')
		return (c - '0');
	else if (c >= 'a' && c <= 'f')
		return (10 + c - 'a');
	else if (c >= 'A' && c <= 'F')
		return (10 + c - 'A');

	return(0);
}

static int atSigLine = 0;	// Make it easier to construct string for sig verification

static void handleSignatureParams (LicenseData *ld, const char **atts)
{
	int i;
	int j;
	unsigned int hi, lo;
	int fh;
	const char *s;

   	NETDEBUG (MLMGR, NETLOG_DEBUG4, ("handleSignatureParams start\n"));

	atSigLine = 1;

	for (i=0;  atts[i]!=0; i+=2)
	{
		if (strcasecmp(atts[i], "id") == 0)
		{
			s = atts[i+1];
			for (j=0; j<SIGSIZE, *s != '\0'; j++)
			{
				hi = asc2hex(*s++);
				lo = asc2hex(*s++);
				ld->sigRead[j] = (hi<<4)+lo;
			}
			ld->sigReadLen = j;
		}
		else
		{
			ERROR (MLMGR, ("Unknown signature parameter %s ignored.\n", atts[i]));
		}
	}
}

static void startElement (void *ud, const char *name, const char **atts)
{
	int i;
	int siglen;
	unsigned int nameLen = strlen(name)+1;
	LicenseData *ld = (LicenseData *)ud;
	
   	NETDEBUG (MLMGR, NETLOG_DEBUG4, ("startElement %s\n", name));

	for (i=0;  atts[i]!=0; i+=2)
	{
		NETDEBUG (MLMGR, NETLOG_DEBUG4, ("param='%s', value='%s'\n", atts[i], atts[i+1]));
	}
	// See if we are expecting this element
	if (strcasecmp(name, "license") == 0)
	{
		handleLicenseParams (ld, atts);
	}
	else if (strcasecmp(name, "server") == 0)
	{
		handleServerParams (ld, atts);
	}
	else if (strcasecmp(name, "agent") == 0)
	{
		NETDEBUG (MLMGR, NETLOG_DEBUG4, ("Ignoring AGENT specification in license file\n"));
	}
	else if (strcasecmp(name, "features") == 0)
	{
		handleFeatureParams (ld, atts);
	}
	else if (strcasecmp(name, "enf") == 0)
	{
		handleEnfParams (ld, atts);
	}
	else if (strcasecmp(name, "signature") == 0)
	{
		handleSignatureParams (ld, atts);
	}
	else
	{
		ERROR (MLMGR, ("Unknown element %s ignored.\n", name));
	}
}


static void endElement (void *ud, const char *name)
{
	char str[32];

   	NETDEBUG (MLMGR, NETLOG_DEBUG4, ("endElement %s\n", name));
	
	// See if we are expecting this element
	return;
}

static int readXmlLicense(void)
{
	char *buf;
	char *ret;
	char line[256];
	XML_Parser parser = XML_ParserCreate(NULL);
	int done;
	int i;
	int res=-1;
	FILE *file = NULL;
	size_t len;
	int sigValid = 0;
	LicenseData *ld = &license;
	unsigned char compsig[MD5_DIGEST_LENGTH];
	int	serverCount = 0;
	ServerData	*si;

	memset (ld, 0, sizeof(LicenseData));
	buf = ld->buffer;
	ld->sigLength = SIGSIZE;
	ld->bufferLen = LIC_BUFSIZE;
	ld->sigValid = 0;		// Assume invalid signature
	ld->serverValid = 0;		// Assume invalid server
	ld->expiry_time=1;		// Since 0=never expires, init to expired license
	ld->serverList = listInit();

	XML_SetUserData (parser, (void *)ld);

	if(xmlLicenseFile == NULL)
	{
		xmlLicenseFile = XML_LICENSE_FILE;
	}

	// open the license file
	if ((file = fopen(xmlLicenseFile, "r")) == NULL)
	{
		ERROR (MLMGR, ("Cannot open License file: %s %s\n", xmlLicenseFile, strerror(errno)));
		return -1;
	}
	
	XML_SetElementHandler(parser, startElement, endElement);
	
	do {
		if ((ret = fgets (line, sizeof(line), file)) != NULL)
		{
			len = strlen(line);
			if (XML_Parse(parser, line, len, 0) == 0)
			{
				NETDEBUG (MLMGR, NETLOG_DEBUG4,("readXML: error parsing %s: %s at %d\n", xmlLicenseFile,
						XML_ErrorString(XML_GetErrorCode(parser)), XML_GetCurrentLineNumber(parser)));
				return -1;
			}
			if (!atSigLine)
			{
				strcpy (&buf[ld->bufOffset], line);
				ld->bufOffset += len;
			}
			atSigLine=0;
		}
	} while (ret);

	if (feof(file))
	{
		if (XML_Parse(parser, line, 0, 1) == 0)
		{
			NETDEBUG (MLMGR, NETLOG_DEBUG4,("readXML: error parsing %s: %s at %d (EOF)\n", xmlLicenseFile,
					XML_ErrorString(XML_GetErrorCode(parser)), XML_GetCurrentLineNumber(parser)));
			return -1;
		}
	}
	else
	{
		ERROR (MLMGR, ("Error reading license file\n"));
		return -1;
	}

	len = ld->bufOffset;
	for (i=0; i<sizeof(sig_xx); i++)
	{
		len += sprintf(&buf[len], "%02x", sig_xx[i]&0xFF);
	}

	MD5 (buf, len, compsig);
	if (ld->sigReadLen == MD5_DIGEST_LENGTH)
	{
		ld->sigValid = (memcmp (ld->sigRead, compsig, MD5_DIGEST_LENGTH) == 0)?1:0;
	}

	XML_ParserFree(parser);
	fclose(file);

	if(ld->serverList)
	{
		serverCount = listItems(ld->serverList);
	}

	if (serverCount > 0)
	{
		si = listGetFirstItem(ld->serverList);
		do {
			if(si->serverValid)
			{
				ld->serverValid = 1;
				break;
			}
		} while (si = listGetNextItem(ld->serverList, si));
	}

	if (ld->sigValid && ld->serverValid)
	{
		res = 0;
	}
	else
	{
		res = -1;
		// Set TERM date
		ld->termTime = time(NULL);
	}

	lsMem->nlic = ld->maxEndpoints;
	lsMem->expiry_time = ld->expiry_time;
	lsMem->initTime = ld->initTime;
	lsMem->termTime = ld->termTime;
	if( strlen(ld->macstr) )
	{
		memcpy (lsMem->macstr, ld->macstr, sizeof(lsMem->macstr));
	}
	else
	{
		strcpy (lsMem->macstr, "unbound");
	}
	lsMem->maxCalls = ld->maxCalls;
	lsMem->features = ld->features;
	lsMem->maxMRCalls = ld->maxMRCalls;
	lsMem->maxRealms = ld->maxRealms;
	lsMem->maxRoutes = ld->maxRoutes;
	lsMem->maxDynamicEP = ld->maxDynamicEP;

	if( (ld->expiry_time == 0) && (strcmp(lsMem->macstr, "unbound") == 0) )
	{
		res = -1;
		lsMem->termTime = ld->termTime = time(NULL);
	}
	if( ld->productVersion && (strncmp(ld->productVersion, VERSION + 1, strlen(ld->productVersion)) != 0) )
	{
		res = -1;
		lsMem->termTime = ld->termTime = time(NULL);
	}

	return (res);
}
