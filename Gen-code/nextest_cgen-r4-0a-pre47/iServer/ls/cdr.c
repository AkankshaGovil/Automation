#include <unistd.h>
#include <limits.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>
#include <dirent.h>
#include <fnmatch.h>
#include "gis.h"
#include "lock.h"
#include "timer.h"
#include "radclient.h"
#include "nxioctl.h"
#include "nxosd.h"
#include "cdr.h"
#include "log.h"
#include "ifs.h"

#define MAXFILESIZE	(1<<20)			/* 1 Mega bytes */
#define CDR (cdr+len)
#define MAX (max-len)

#define MAXCALLIDPRINTLEN	cdrcallidlen
#define MAXCDRPNAME		256		// Max CDR file pathname

static char cdrfilename[MAXCDRPNAME];
static char ctrfilename[MAXCDRPNAME];
static char cdrpname[MAXCDRPNAME];
static char ctrpname[MAXCDRPNAME];

static int filelen = 0;
int callno = 0;
static unsigned long localGwIp = 0x7f000001;
tid cdrFileTimer = 0;
Lock cdr_mutex;

int cdrinPool, cdrinClass;

int cdrseqfd = -1;	// The next value will be stored
			// ie., the variable is incremented
			// each time a cdr is printed
unsigned long *cdrseqno = NULL;
unsigned long cdrseqno0 = 1;	// start value

int
CdrClose(void)
{
	char fn[] = "CdrClose():";
	void *timer_data;
	
	if (localConfig.cdrfile)
	{
		fclose(localConfig.cdrfile);
		localConfig.cdrfile = NULL;

        	if (cdrFileTimer != 0)
        	{
         		if(timerDeleteFromList (&localConfig.timerPrivate, 
				cdrFileTimer, &timer_data))
				{
					free(timer_data);
				}

				cdrFileTimer = 0;
        	}
	}

	if (localConfig.ctrfile)
	{
		fclose(localConfig.ctrfile);
		localConfig.ctrfile = NULL;
	}

	return(0);
}

int
CdrFileTimer(struct Timer *t)
{
	LockGetLock(&cdr_mutex, 0, 0);

    CdrRotateFile();

	LockReleaseLock(&cdr_mutex);

	timerFreeHandle(t);

	return 0;
}

int
CdrOpen(void)
{
	char fn[] = "CdrOpen():";
	struct stat statbuf = { 0 };
    struct 	itimerval cdrtmr;
	struct tm res;
        
	strcpy(localConfig.cdrdirname, cdrdirname);
        localConfig.cdrtimer = cdrtimer;
        
	localConfig.cdrfiletype = cdrtype;
	localConfig.cdrevents = cdrevents;
	localConfig.billingType = billingType;
	localConfig.cdrformat = cdrformat;

	NETDEBUG(MCDR, NETLOG_DEBUG1,
			 ("cdrtype is %d\n", cdrtype));

	if (strlen(localConfig.cdrdirname))
	{
		 /* Depending on the type, we will open a file */
		 if (localConfig.cdrfiletype == CDRMINDCTIFIXED)
		 {
			  nx_strlcpy(cdrfilename, "DATA.CDR", MAXCDRPNAME);
		 }
		 else if (localConfig.cdrfiletype == CDRMINDCTIDAILY)
		 {
			  char daily[MAXCDRPNAME] = { 0 };
			  time_t now, tomorrow;
			  
			  time(&now);

			  strftime(daily, MAXCDRPNAME, "%Y%m%d", localtime_r(&now, &res));
			  snprintf(cdrfilename, MAXCDRPNAME, "D%s.CDT", daily);


			  // We must start a timer upto the next day
			  // figure out the no of secs needed
			  tomorrow = (23 - res.tm_hour)*3600 + 
							(60-res.tm_min)*60 + (60-res.tm_sec);

			  if (tomorrow <= 0)
			  {
				tomorrow += 24*60*60;
			  }

			  /* Start the timer... */
			  memset(&cdrtmr, 0, sizeof(struct itimerval));
			  cdrtmr.it_value.tv_sec = tomorrow;

			  cdrFileTimer = timerAddToList(&localConfig.timerPrivate, &cdrtmr,
         						0,PSOS_TIMER_REL, "CDRTimer", CdrFileTimer, NULL);    
		 }
		 else if (localConfig.cdrfiletype == CDRMINDCTISEQ)
		 {
			int n;

			/* We have to figure out how many files are already
			* there in the directory, and then use one more than
			* that to create the new file
			*/
			n = CdrCountFiles(localConfig.cdrdirname, "S*.CDR");

			if (n >= 0)
			{	
				/* Not sure if we should just use n+1 or determine the size
				* of the last one, and if its less that a limit, continue
				* to write into it. The danger is how the server knows the
				* old file contains new data.
				*/
			  	snprintf(cdrfilename, MAXCDRPNAME, "S%4.4d.CDT", n+1);
			}
		 }
         else if (localConfig.cdrfiletype == CDRNEXTONETIME)
         {
			/* Name of the file would be THHMM.CDR
			 * where HH= current hour
				MM = current minute
			 */

			char daily[MAXCDRPNAME] = { 0 };
			time_t now;
                     
			time(&now);

			strftime(daily, MAXCDRPNAME, "%Y%m%d%H%M", gmtime_r(&now, &res));
                     
			snprintf(cdrfilename, MAXCDRPNAME, "T%s.CDT", daily);

			/* Start the timer... */
			memset(&cdrtmr, 0, sizeof(struct itimerval));
			cdrtmr.it_value.tv_sec = localConfig.cdrtimer*60;

			cdrFileTimer = timerAddToList(&localConfig.timerPrivate, &cdrtmr,
         						0,PSOS_TIMER_REL, "CDRTimer", CdrFileTimer, NULL);    
		}

		strcpy(ctrfilename, cdrfilename);
		ctrfilename[strlen(ctrfilename)-2] = 'T';
	}
	else
	{
		 memset(cdrfilename, 0, MAXCDRPNAME);
		 memset(ctrfilename, 0, MAXCDRPNAME);
	}

	// Move temp files
	CdrMoveTempFiles(localConfig.cdrdirname, "CDT", "CDR", cdrfilename);
	CdrMoveTempFiles(localConfig.cdrdirname, "CTT", "CTR", ctrfilename);

	if (strlen(cdrfilename))
	{
		int fd;
		int nbio = 1;

		snprintf(cdrpname, MAXCDRPNAME, "%s/%s",
			localConfig.cdrdirname, cdrfilename);

		NETDEBUG(MCDR, NETLOG_DEBUG1,
			("%s New CDR File is %s\n", fn, cdrpname));

		localConfig.cdrfile = fopen(cdrpname, "a+");
		if (localConfig.cdrfile == NULL)
		{
			NETERROR(MCDR,
			("%s Could not open file %s\n", fn, cdrpname));

			return 0;
		}

		fd = fileno(localConfig.cdrfile);

		/* evaluate the filelen at this time */
		fstat( fd , &statbuf );
		filelen = statbuf.st_size;

		// Also setup some options on the file
		if (ioctl(fd, FIONBIO, &nbio) < 0)
		{
			perror("Failed to make cdr file non-blocking\n");
		}

		 /* evaluate the filelen at this time */
		 if (fd >= 0)
		 {
		 	fstat( fd , &statbuf );
		 	filelen = statbuf.st_size;
		 }
	}

	if ((cdrevents &~(CDREND1|CDRHUNT)) && strlen(ctrfilename))
	{
		int fd;
		int nbio = 1;

		snprintf(ctrpname, MAXCDRPNAME, "%s/%s",
			localConfig.cdrdirname, ctrfilename);

		NETDEBUG(MCDR, NETLOG_DEBUG1,
			("%s New CTR File is %s\n", fn, ctrpname));

		localConfig.ctrfile = fopen(ctrpname, "a+");
		if (localConfig.ctrfile == NULL)
		{
			NETERROR(MCDR,
			("%s Could not open file %s\n", fn, ctrpname));

			return 0;
		}

		fd = fileno(localConfig.ctrfile);

		// Also setup some options on the file
		if (ioctl(fd, FIONBIO, &nbio) < 0)
		{
			perror("Failed to make ctr file non-blocking\n");
		}
	}

	return 0;
}

int
CdrRotateFile(void)
{
	/* We will rotate the file, only if its sequential file
	 */
	CdrClose();
	CdrOpen();
	return(0);
}

int
CdrCfg(void)
{
	char fn[] = "CdrCfg():";

	NETDEBUG(MCDR, NETLOG_DEBUG4,
			 ("%s Entering \n", fn));

	LockGetLock(&cdr_mutex, 0, 0);

	if (strcmp(cdrdirname, localConfig.cdrdirname) ||
		(cdrtype != localConfig.cdrfiletype) ||
		(cdrevents != localConfig.cdrevents) ||
		(billingType != localConfig.billingType) ||
		(cdrformat != localConfig.cdrformat) )
	{
		 CdrClose();
	}

	if (localConfig.cdrfile == NULL)
	{
		 CdrOpen();
	}

	localGwIp = getLocalIf(ifihead);

	LockReleaseLock(&cdr_mutex);
	return(0);
}

int
CdrInit(void)
{
	void *mapaddr;

	LockInit(&cdr_mutex, 0);

	cdrinPool = ThreadPoolInit("cdr", 1, PTHREAD_SCOPE_PROCESS, 0, 0);
	cdrinClass = ThreadAddPoolClass("cdrclass", cdrinPool, 0, 100000000);
	ThreadPoolStart(cdrinPool);

	// Open the memory map file
	cdrseqfd = open(CDRSEQFNAME, O_RDWR|O_CREAT, 
			S_IRWXU|S_IRWXO|S_IRWXG);
	
	if (cdrseqfd < 0)
	{
		NETERROR(MINIT, ("Could not open file %s\n",
			CDRSEQFNAME));
	}
	else
	{
		if (read(cdrseqfd, (char *)cdrseqno, sizeof(*cdrseqno)) < 
			sizeof(*cdrseqno))
		{
			// File needs to be initialized
			if (lseek(cdrseqfd, 0, SEEK_SET) < 0)
			{
				NETERROR(MINIT, ("lseek failed on %s\n",
					CDRSEQFNAME));
			}

			if (write(cdrseqfd, (char *)&cdrseqno0, 
				sizeof(cdrseqno0)) < sizeof(cdrseqno0))
			{
				NETERROR(MINIT, ("write failed for %s\n",
					CDRSEQFNAME));
			}
		}

		mapaddr = mmap(NULL, sizeof(*cdrseqno), PROT_READ|PROT_WRITE,
			MAP_SHARED, cdrseqfd, 0);

		if (mapaddr == NULL)
		{
			cdrseqno = NULL;
			NETERROR(MINIT, ("Could not map file %s into memory\n",
				CDRSEQFNAME));
			close(cdrseqfd);
			cdrseqfd = -1;
		}
		else
		{
			cdrseqno = (unsigned long *)mapaddr;
		}
	}

	return(0);
}

void
CdrEnd(void)
{
#if 0
	if (cdrseqno != NULL)
	{
		if (munmap(cdrseqno, sizeof(*cdrseqno)) < 0)
		{
		}
	}

	if (cdrseqfd >= 0)
	{
		close(cdrseqfd);
	}
#endif
}

int
CdrLogSyslog(CallHandle *callHandle, int flag)
{
	timedef tmptime;

	if (billingType == BILLING_POSTPAID)
	{
 		struct in_addr in;
                char buf[INET_ADDRSTRLEN];
     	int h, m, secs, msecs, dur;
		PhoNode *local, *remote;
		char timestr[26];

		local=&callHandle->phonode;
		remote=&callHandle->rfphonode;

     	NETCDR(MCDR, NETLOG_DEBUG1,
		("\n***************** CDR START %s  ******************\n", 
			(flag==CDR_CALLSETUP) ? "Call Setup": "Call Termination"));
#if 0
		NETCDR(MCDR, NETLOG_DEBUG1,("%s\n",
			BIT_TEST(data_pkt->data.profile.flags, CDR_ORIGINATOR) ? 
		"Call Origin": "Call Destination"));
#endif
		NETCDR(MCDR, NETLOG_DEBUG1,("Call ID %lu-%lu-%llu\n", 
			*(unsigned long *)callHandle->callID,
			*(unsigned long *)(callHandle->callID+4),
			*(hrtime_t *)(callHandle->callID+8)));

		NETCDR(MCDR, NETLOG_DEBUG1,("%s\n", "Call Origin"));

		if (strlen(local->regid))
		{
     		NETCDR(MCDR, NETLOG_DEBUG1,
			(" o Registration ID %s\n", local->regid));
     		NETCDR(MCDR, NETLOG_DEBUG1,
			(" o Port %lu\n", (local->uport)));
		}

		in.s_addr = htonl(local->ipaddress.l);
		if (local->ipaddress.l)
		{
			NETCDR(MCDR, NETLOG_DEBUG1,(" o Ip %s\n", inet_ntop( AF_INET, &in, buf, INET_ADDRSTRLEN)));
		}
		
		if (strlen(callHandle->inputANI))
		{
     		NETCDR(MCDR, NETLOG_DEBUG1,
			(" o ANI %s\n", callHandle->inputANI));
		}

		if (strlen(local->phone))
		{
     		NETCDR(MCDR, NETLOG_DEBUG1,
			(" o New ANI %s\n", local->phone));
		}
#if 0
     	NETCDR(MCDR, NETLOG_DEBUG1,
		(" o VpnPhone %s\n", local.vpnPhone));
#endif

		NETCDR(MCDR, NETLOG_DEBUG1, (" o Details\n"));

		if (callHandle->handleType == SCC_eSipCallHandle)
		{
			SipCallHandle *sipCallHandle;
			sipCallHandle = SipCallHandle(callHandle);

     		NETCDR(MCDR, NETLOG_DEBUG1,
				(" o From %s@%s\n", 
					SVal(sipCallHandle->callLeg.remote->name),
					SVal(sipCallHandle->callLeg.remote->host)));
		}
		else
		{
			if (strlen(H323callingPartyNumber(callHandle)))
			{
     			NETCDR(MCDR, NETLOG_DEBUG1,
					(" o Phone %s\n", H323callingPartyNumber(callHandle)));
			}
		}

#if 0
     	NETCDR(MCDR, NETLOG_DEBUG1,("%s\n",
		!BIT_TEST(data_pkt->data.profile.flags, CDR_ORIGINATOR) ? 
		"Call Origin" : "Call Destination"));
#endif
     	NETCDR(MCDR, NETLOG_DEBUG1,("%s\n", "Call Destination"));

		if (strlen(remote->regid))
		{
     		NETCDR(MCDR, NETLOG_DEBUG1,
				(" o Registration ID %s\n", remote->regid));
     		NETCDR(MCDR, NETLOG_DEBUG1,
				(" o Port %lu\n", (remote->uport)));
		}

		if (remote->ipaddress.l)
		{
     		in.s_addr = htonl(remote->ipaddress.l);
     		NETCDR(MCDR, NETLOG_DEBUG1,(" o Ip %s\n", inet_ntop( AF_INET, &in, buf, INET_ADDRSTRLEN)));
		}

		if (strlen(remote->phone))
		{	
     		NETCDR(MCDR, NETLOG_DEBUG1,
				(" o Phone %s\n", remote->phone));
		}

		if (callHandle->handleType == SCC_eSipCallHandle)
		{
			SipCallHandle *sipCallHandle;
			sipCallHandle = SipCallHandle(callHandle);

			NETCDR(MCDR, NETLOG_DEBUG1, (" o Details\n"));

     		NETCDR(MCDR, NETLOG_DEBUG1,
				(" o To %s@%s\n", 
					SVal(sipCallHandle->callLeg.local->name),
					SVal(sipCallHandle->callLeg.local->host)));

     		NETCDR(MCDR, NETLOG_DEBUG1,
				(" o Dest Phone %s@%s\n", 
					SVal(sipCallHandle->inrequri->name),
					SVal(sipCallHandle->inrequri->host)));
		}
#if 0
     	NETCDR(MCDR, NETLOG_DEBUG1,
		(" o VpnPhone %s\n", data_pkt->data.profile.remote.vpnPhone));
#endif

     	NETCDR(MCDR, NETLOG_DEBUG1,
			(" o Call Start Time %s\n", 
			ctime_r(&timedef_sec(&callHandle->callStartTime), timestr)));

     	if (flag != CDR_CALLSETUP)
		{
     		NETCDR(MCDR, NETLOG_DEBUG1,("Duration\n"));
			if (!timedef_iszero(&callHandle->callConnectTime))
			{
				timedef_sub(&callHandle->callEndTime, &callHandle->callConnectTime, &tmptime);
     				dur = timedef_sec(&tmptime);
     				msecs = timedef_msec(&tmptime);
			}
			else
			{
				dur = 0;
				msecs = 0;
			}


   			h = dur/(60*60); m = dur/60 - h*60; secs = dur - 60*(h*60 + m);
   			NETCDR(MCDR, NETLOG_DEBUG1,
			(" o %d Hrs, %d Mts and %d Secs %d mSecs\n", h, m, secs, msecs));


     		NETCDR(MCDR, NETLOG_DEBUG1,
			(" ISDN cause %d \n", callHandle->cause));
		}

     	NETCDR(MCDR, NETLOG_DEBUG1,
		("\n***************** CDR END ******************\n"));
	}

	return 0;
}

// Put the CDR in memory, and it will get logged
// by another thread running at lower priority
int
CdrLogXml(FILE *file, CallHandle *callHandle, int flag)
{
 	struct in_addr in;
        char buf[INET_ADDRSTRLEN];
    int h, m, secs, dur, msecs;
	PhoNode *local, *remote;
	char timestr[26];
	char *cdr = NULL;
	int len = 0, max=1024;
	int fd;
	timedef tmptime;

	local=&callHandle->phonode;
	remote=&callHandle->rfphonode;

	cdr=(char *)malloc(1024);

	len+=snprintf(CDR, MAX, "<CDR>\n");

	// 15
	if (flag==CDR_CALLSETUP) 
	{
		len+=snprintf(CDR, MAX, "<CDR-SETUP>\n");
	}
	else
	{
		len+=snprintf(CDR, MAX, "<CDR-TERM>\n");
	}

	// 30
	len+=snprintf(CDR, MAX, "<CID> %lu-%lu-%llu </CID>\n",
		*(unsigned long *)callHandle->callID,
		*(unsigned long *)(callHandle->callID+4),
		*(hrtime_t *)(callHandle->callID+8));

	// 6
	len+=snprintf(CDR, MAX, "<ORG>\n");

	// 85
	if (strlen(local->regid))
	{
		len+=snprintf(CDR, MAX, "<REGID> %.64s/%1.3ld </REGID>\n",
				local->regid, local->uport);
	}

	// 30
	in.s_addr = htonl(local->ipaddress.l);
	if (local->ipaddress.l)
	{
		len+=snprintf(CDR, MAX, "<IP> %.16s </IP>\n",
				inet_ntop( AF_INET, &in, buf, INET_ADDRSTRLEN));
	}
		
	// 44
	if (strlen(callHandle->inputANI))
	{
		len+=snprintf(CDR, MAX, "<CGPN> %.24s </CGPN>\n",
				callHandle->inputANI);
	}

	// 44
	if (strlen(local->phone))
	{
		len+=snprintf(CDR, MAX, "<NEW-CGPN> %.24s </NEW-CGPN>\n",
				local->phone);
	}

	// 126
	if (callHandle->handleType == SCC_eSipCallHandle)
	{
		SipCallHandle *sipCallHandle;
		sipCallHandle = SipCallHandle(callHandle);

		len+=snprintf(CDR, MAX, "<DETAILS>\n");
		len+=snprintf(CDR, MAX, "<FROM> %.24s@%.64s </FROM>\n", 
					SVal(sipCallHandle->callLeg.remote->name),
					SVal(sipCallHandle->callLeg.remote->host));
		len+=snprintf(CDR, MAX, "</DETAILS>\n");
	}
	else
	{
		if (strlen(H323callingPartyNumber(callHandle)))
		{
			len+=snprintf(CDR, MAX, "<DETAILS>\n");
			len+=snprintf(CDR, MAX, "<CGPN> %.24s </CGPN>\n",
						H323callingPartyNumber(callHandle));
			len+=snprintf(CDR, MAX, "</DETAILS>\n");
		}
	}

	// 14
	len+=snprintf(CDR, MAX, "</ORG>\n");
	len+=snprintf(CDR, MAX, "<DST>\n");

	if (strlen(remote->regid))
	{
		len+=snprintf(CDR, MAX, "<REGID> %.64s/%1.3ld </REGID>\n",
				remote->regid, remote->uport);
	}

	// 30
	if (remote->ipaddress.l)
	{
   		in.s_addr = htonl(remote->ipaddress.l);
		len+=snprintf(CDR, MAX, "<IP> %.16s </IP>\n", inet_ntop( AF_INET, &in, buf, INET_ADDRSTRLEN));
	}

	// 46
	if (strlen(remote->phone))
	{	
		len+=snprintf(CDR, MAX, "<CDPN> %.24s </CDPN>\n",
				remote->phone);
	}

	// 46
	if (strlen(callHandle->inputNumber))
	{
		len+=snprintf(CDR, MAX, "<USER-DIALLED> %.24s </USER-DIALLED>\n",
				callHandle->inputNumber);
	}

	// 46
	if (strlen(callHandle->dialledNumber))
	{
		len+=snprintf(CDR, MAX, "<DIALLED> %.24s </DIALLED>\n",
				callHandle->dialledNumber);
	}

	// 250
	if (callHandle->handleType == SCC_eSipCallHandle)
	{
		SipCallHandle *sipCallHandle;
		sipCallHandle = SipCallHandle(callHandle);

		len+=snprintf(CDR, MAX, "<DETAILS>\n");
		len+=snprintf(CDR, MAX, "<TO> %.24s@%.64s </TO>\n", 
				SVal(sipCallHandle->callLeg.local->name),
				SVal(sipCallHandle->callLeg.local->host));

		len+=snprintf(CDR, MAX, "<CDPN> %.24s@%.64s </CDPN>\n",
				SVal(sipCallHandle->inrequri->name),
				SVal(sipCallHandle->inrequri->host));

		len+=snprintf(CDR, MAX, "</DETAILS>\n");
	}

	// 8
	len+=snprintf(CDR, MAX, "</DST>\n");

	len+=snprintf(CDR, MAX, "<CTIME> %.30s </CTIME>\n",
			ctime_r(&timedef_sec(&callHandle->callStartTime), timestr));

	//30 
   	if (flag != CDR_CALLSETUP)
	{
		if (!timedef_iszero(&callHandle->callConnectTime))
		{
			timedef_sub(&callHandle->callEndTime, &callHandle->callConnectTime, &tmptime);
   				dur = timedef_sec(&tmptime);
   				msecs = timedef_msec(&tmptime);
		}
		else
		{
			dur = 0;
			msecs = 0;
		}

		h = dur/(60*60); m = dur/60 - h*60; secs = dur - 60*(h*60 + m);
		len+=snprintf(CDR, MAX, "<DUR> %1.3d/%1.3d/%1.3d.%3d </DUR>\n",
					h, m, secs, msecs);

		//32
		len+=snprintf(CDR, MAX, "<ISDN-CAUSE> %d </ISDN-CAUSE>\n",
			callHandle->cause);
	}


	// 15
	if (flag==CDR_CALLSETUP) 
	{
		len+=snprintf(CDR, MAX, "</CDR-SETUP>\n");
	}
	else
	{
		len+=snprintf(CDR, MAX, "</CDR-TERM>\n");
	}

	len+=snprintf(CDR, MAX, "</CDR>\n\n");

	if (len>max)
	{
		// not possible, but still check
		NETERROR(MINIT, ("Shared memory may be corrupted!\n"));
	}

	fd = fileno(file);
	if (write(fd, cdr, len+1) < 0)
	{
		int thiserr = errno;

		NETINFOMSG(MINIT, ("CDR write failed. Errno = %d\n", thiserr));
	}

	//fprintf(file, cdr);
	free(cdr);
	return len;
}

/* Pass the Terminating CDR from originator of the call */
int
CdrLogMindCti(FILE *file, CallHandle *callHandle, int flag)
{
	char fn[] = "CdrLogMindCti():";
	char tmptime[256] = { 0 };
	int h, m, secs, msecs, dur;
	struct in_addr in;
        char buf[INET_ADDRSTRLEN];
	PhoNode *local, *remote;
	char callidstr[128] = { 0 };
	struct tm res = { 0 };
	char cdrbuffer[4096];
	char *cdr = &cdrbuffer[0];
	int len = 0, max=4096;
	int holdtime = 0, pddtime = 0;
	int fd;
	int newError = callHandle->callError;
	int newError2 = callHandle->callDetails2.callError;
	int lastEvent = callHandle->lastEvent;
	int lastEvent2 = callHandle->callDetails2.lastEvent;
	timedef durtime, hldtime;
	CacheRealmEntry	*realmEntryPtr =NULL;		

	local = remote = NULL;
	
	if (callHandle->callSource)
	{
		remote=&callHandle->phonode;
		local=&callHandle->rfphonode;
	}
	else
	{
		local=&callHandle->phonode;
		remote=&callHandle->rfphonode;
	}

	/* Log in Mind Cti format */
	/* 1. Starting Date and time of call on local gw
	 * YYYY-MM-DD HH:MM:SS
	*/
	if (timedef_iszero(&callHandle->callStartTime))
	{
		// THIS must never BE ZERO
		timedef_cur(&callHandle->callStartTime);
	}

	if (timedef_iszero(&callHandle->callEndTime))
	{
		// THIS must never BE ZERO, as what we will put here
		// includes the cdr queued up time
		timedef_cur(&callHandle->callEndTime);
	}

	strftime(tmptime, 256, "%Y-%m-%d %T", localtime_r(&timedef_sec(&callHandle->callStartTime), &res));
	len+=snprintf(CDR, MAX, tmptime);

	/* 2. (1.) in int format */
	len+=snprintf(CDR, MAX, ";%ld", timedef_sec(&callHandle->callStartTime));

	/* 3. Duration of the call 
	 * HHH:MM:SS
	 */
	if (timedef_iszero(&callHandle->callConnectTime))
	{
		// this has not been initialized yet.
		dur = 0;
		msecs = 0;
	}
	else
	{
		timedef_sub(&callHandle->callEndTime, &callHandle->callConnectTime, &durtime);
   		dur = timedef_rndsec(&durtime);
     	msecs = timedef_msec(&durtime);
		if (dur < 0)
		{
			NETERROR(MINIT, ("%s invalid dur %d %ld.%6.6ld-%ld.%6.6ld\n", 
				fn, dur, timedef_sec(&callHandle->callEndTime), timedef_usec(&callHandle->callEndTime), 
				timedef_sec(&callHandle->callConnectTime), timedef_usec(&callHandle->callConnectTime)));
			dur = 0;
			msecs = 0;
		}
	}
	
   	h = dur/(60*60); m = dur/60 - h*60; secs = dur - 60*(h*60 + m);
	len+=snprintf(CDR, MAX, ";%3.3d:%2.2d:%2.2d", h, m, secs);

	/* 4. Originator IP */

	if (callHandle->callSource)
	{
		len+=snprintf(CDR, MAX, ";");
	}
	else
	{
		in.s_addr = htonl(local->ipaddress.l);
		len+=snprintf(CDR, MAX, ";%s", inet_ntop( AF_INET, &in, buf, INET_ADDRSTRLEN));
	}


#if 0
	/* 5. Originator Line */
	len+=snprintf(CDR, MAX, ";0");
#endif
	/* 5. Originator Line  - meanwhile is the src signaling port*/
	len+=snprintf(CDR, MAX, ";%d", callHandle->peerPort);

	/* 6. Terminator IP */
	in.s_addr = htonl(remote->ipaddress.l);
	len+=snprintf(CDR, MAX, ";%s", inet_ntop( AF_INET, &in, buf, INET_ADDRSTRLEN));

	/* 7. Terminator Line */
	len+=snprintf(CDR, MAX, ";");

	/* 8. User Id of originator */
	len+=snprintf(CDR, MAX, ";%s", 
		callHandle->custID?callHandle->custID:"");

	/* 9. Called number in E.164 format */
	len+=snprintf(CDR, MAX, ";%s", remote->phone);

	/* 10. Called number as dialled by the user */
	len+=snprintf(CDR, MAX, ";%s", 
		callHandle->callSource?"":callHandle->inputNumber);

	/* 11. Call Type */
	if (callHandle->flags & FL_CALL_FAX)
	{
		len+=snprintf(CDR, MAX, ";IF");
	}
	else
	{
		len+=snprintf(CDR, MAX, ";IV");
	}

	/* 12. Call Parties */
	len+=snprintf(CDR, MAX, ";01");

	/* 13. Call Disconnect Reason */
	// Shutdown has to be treated as a special scenario
	// for now. It is a mid call error, and will be moved
	// into a different cdr field for mid call errors
	// reporting.
	if ((flag != CDR_CALLSETUP) &&
		(timedef_iszero(&callHandle->callConnectTime)) &&
		(newError == SCC_errorNone))
	{
		newError = SCC_errorUndefinedReason;
	}
	else if ((flag != CDR_CALLSETUP) &&
				(newError != SCC_errorShutdown &&
					newError != SCC_errorMaxCallDuration) &&
				(!timedef_iszero(&callHandle->callConnectTime)))
	{
		newError = SCC_errorNone;
	}

	switch (newError)
	{
	case SCC_errorAbandoned:
		len+=snprintf(CDR, MAX, ";A");
		break;
	case SCC_errorBusy:
		len+=snprintf(CDR, MAX, ";B");
		break;
	case SCC_errorNone:
		len+=snprintf(CDR, MAX, ";N");
		break;
	default:
		len+=snprintf(CDR, MAX, ";E");
		break;
	}

	/* Optional fields */
	// 14,15
	if (newError == SCC_errorNone)
	{
		len+=snprintf(CDR, MAX, ";;");
	}
	else
	{
		len+=snprintf(CDR, MAX, ";%d;%s",
			newError, CallGetErrorStr(newError));
	}

	//16,17
	/* error descr, fax pages, priority */
	len+=snprintf(CDR, MAX, ";;");

	//18
	/* orig ANI */
	len+=snprintf(CDR, MAX, ";%s", callHandle->inputANI);

	//19,20,21
	/* DNIS, no of bytes sent, number of bytes recved, seq no, local gw stop date+time */
	len+=snprintf(CDR, MAX, ";;;");

	//22
	if ((flag != CDR_CALLSETUP) &&
		!callHandle->callSource)
	{
		/* seqno in seconds */
		len+=snprintf(CDR, MAX, ";%lu", (*cdrseqno)++);
	}
	else
	{
		len+=snprintf(CDR, MAX, ";");
	}

	//23
	len+=snprintf(CDR, MAX, ";");

	//24
	/* Call id  - at most MAXCALLIDPRINTLEN bytes will be printed */
	if (callHandle->conf_id)
	{
		len+=snprintf(CDR, MAX<MAXCALLIDPRINTLEN?MAX:MAXCALLIDPRINTLEN, ";%s", 
			callHandle->conf_id);
	}
	else
	{
		CallID2String(callHandle->callID, callidstr);
		len+=snprintf(CDR, MAX, ";%s", callidstr);
	}

	if (timedef_iszero(&callHandle->callConnectTime))
	{
		// the call did not get connected
		timedef_sub(&callHandle->callEndTime, &callHandle->callStartTime, &hldtime);

		holdtime = timedef_rndsec(&hldtime);

		if (holdtime < 0)
		{
			NETERROR(MINIT, ("%s invalid holdtime %d %ld.%ld-%ld.%ld\n", 
				fn, holdtime, timedef_sec(&callHandle->callEndTime), 
				timedef_usec(&callHandle->callEndTime), timedef_sec(&callHandle->callStartTime), 
				timedef_usec(&callHandle->callStartTime)));
			holdtime = 0;
		}
	}
	else
	{
		// Call got connected
		timedef_sub(&callHandle->callConnectTime, &callHandle->callStartTime, &hldtime);

		holdtime = timedef_rndsec(&hldtime);

		if (holdtime < 0)
		{
			NETERROR(MINIT, ("%s invalid holdtime %d %ld.%ld-%ld.%ld\n", 
				fn, holdtime, timedef_sec(&callHandle->callConnectTime),
				timedef_usec(&callHandle->callConnectTime), timedef_sec(&callHandle->callStartTime), 
				timedef_usec(&callHandle->callStartTime)));
			holdtime = 0;
		}
	}
	

	h = holdtime/3600; m = holdtime/60 - h*60; secs = holdtime - h*3600 - m*60;

	//25
	len+=snprintf(CDR, MAX, ";%3.3d:%2.2d:%2.2d", h, m, secs);

	//26,27
	if (!callHandle->callSource)
	{
		len+=snprintf(CDR, MAX, ";%s;%lu", 
			strlen(local->regid)?local->regid:"", local->uport);
	}
	else
	{
		len+=snprintf(CDR, MAX, ";;");
	}

	//28,29
	len+=snprintf(CDR, MAX, ";%s;%lu",
		strlen(remote->regid)?remote->regid:"", remote->uport);	

	//30
	/* ISDN Cause Code */
	if (callHandle->cause > 0)
	{
		// Real cause is off by 1
		len+=snprintf(CDR, MAX, ";%d",callHandle->cause-1);
	}
	else
	{
		// We don't have anything
		len+=snprintf(CDR, MAX, ";");
	}

	//31
	/* Called number prior to destination selection */
	len+=snprintf(CDR, MAX, ";%s", callHandle->dialledNumber);

	//32,33
	// Leg 2 error
	if (newError2 == SCC_errorNone)
	{
		len+=snprintf(CDR, MAX, ";;");
	}
	else
	{
		len+=snprintf(CDR, MAX, ";%d;%s",
			newError2, CallGetErrorStr(newError2));
	}

	//34
	if (callHandle->callSource)
	{
		len+=snprintf(CDR, MAX, ";na#%s", CallGetEvtStr(lastEvent));
	}
	else
	{
		len+=snprintf(CDR, MAX, ";%s#%s", CallGetEvtStr(lastEvent),
				lastEvent2?CallGetEvtStr(lastEvent2):"na");
	}

	//35
	/* new ANI */
	len+=snprintf(CDR, MAX, ";%s", local->phone);

	//36
	/* duration in seconds (or milliseconds) */
	len+=snprintf(CDR, MAX, ";%d", dur);

	//37
	/* Call id  - at most MAXCALLIDPRINTLEN bytes will be printed */
	if (callHandle->callSource)
	{
		if (callHandle->incoming_conf_id)
		{
			len+=snprintf(CDR, MAX<MAXCALLIDPRINTLEN?MAX:MAXCALLIDPRINTLEN, ";%s", 
				callHandle->incoming_conf_id);
		}
		else
		{
			CallID2String(callHandle->callID, callidstr);
			len+=snprintf(CDR, MAX, ";%s", callidstr);
		}
	}
	else
	{
		len+=snprintf(CDR, MAX, ";");
	}

	//38
	switch (callHandle->handleType)
	{
	case SCC_eSipCallHandle:
		len+=snprintf(CDR, MAX, ";sip");
		break;
	case SCC_eH323CallHandle:
		len+=snprintf(CDR, MAX, ";h323");
		break;
	default:
		len+=snprintf(CDR, MAX, ";unknown");
		break;
	}

	//39
	if (callHandle->callSource)
	{
		len+=snprintf(CDR, MAX, ";%s",
			(flag==CDR_CALLSETUP)?"start2":"end2");
	}
	else
	{
		len+=snprintf(CDR, MAX, ";%s",
			(flag==CDR_CALLSETUP)?"start1":(flag==CDR_CALLHUNT)?"hunt":"end1");
	}

	//40
	if (callHandle->callSource)
	{
		len+=snprintf(CDR, MAX, ";");
	}
	else
	{
		// If nhunts exceeds maxhunts, correct it
		// to reflect failed resolution
		len+=snprintf(CDR, MAX, ";%d", 
			(callHandle->nhunts > SYSTEM_MAX_HUNTS)?
			callHandle->nhunts-SYSTEM_MAX_HUNTS:callHandle->nhunts);
	}

	//41
	len+=snprintf(CDR, MAX, ";%s", 
		callHandle->tg?callHandle->tg:"");

	//42
	if (!timedef_iszero(&callHandle->callRingbackTime))
	{
		// Call got a ringback
		timedef_sub(&callHandle->callRingbackTime, &callHandle->callStartTime, &hldtime);


		pddtime =  timedef_sec(&hldtime) * 1000 + timedef_msec(&hldtime);

		if (pddtime < 0)
		{
			NETERROR(MINIT, ("%s invalid pddtime %d %ld.%ld-%ld.%ld\n", 
				fn, pddtime, timedef_sec(&callHandle->callRingbackTime), 
				timedef_usec(&callHandle->callRingbackTime), timedef_sec(&callHandle->callStartTime), 
				timedef_usec(&callHandle->callStartTime)));
			pddtime = 0;
		}
	}
	else
	{
		// There was no Ringback
		// In this case PDD = hold time
		pddtime = holdtime;

		if (pddtime < 0)
		{
			NETERROR(MINIT, ("%s invalid pddtime %d\n", fn, pddtime));
			pddtime = 0;
		}
	}

	len+=snprintf(CDR, MAX, ";%d", pddtime);

	//43 Actual H.323 RAS Error received
	if (callHandle->rasReason)
	{
		len+=snprintf(CDR, MAX, ";%d", callHandle->rasReason);
	}
	else
	{
		len+=snprintf(CDR, MAX, ";");
	}

	//44 Actual H.323 H.225 Error received
	if (callHandle->h225Reason)
	{
		len+=snprintf(CDR, MAX, ";%d", callHandle->h225Reason);
	}
	else
	{
		len+=snprintf(CDR, MAX, ";");
	}

	//45 Actual Sip Final response code received
	if (callHandle->responseCode)
	{
		len+=snprintf(CDR, MAX, ";%d", callHandle->responseCode);
	}
	else
	{
		len+=snprintf(CDR, MAX, ";");
	}

	//46 Destination trunk group ID
	if(callHandle->callSource)
	{
		len+=snprintf(CDR, MAX, ";%s", callHandle->destTg ? callHandle->destTg : "");
	}
	else if(callHandle->destTg == NULL)
	{
		len+=snprintf(CDR, MAX, ";%s", callHandle->dtgInfo ? callHandle->dtgInfo : "");
	}
	else
	{
		len+=snprintf(CDR, MAX, ";%s", callHandle->destTg);
	}

	//47 Duration in sec.msec part of duration
	len+=snprintf(CDR, MAX, ";%d.%3.3d", (msecs < 500 ? dur : dur -1), msecs);

	//48 Local timezone
	strftime(tmptime, 256, "%Z", localtime_r(&timedef_sec(&callHandle->callStartTime), &res));
	len+=snprintf(CDR, MAX, ";%s", tmptime);

	//49 MSW name
	len+=snprintf(CDR, MAX, ";%s", mswname);

	//50 called party number after applying transit route
	if(callHandle->transRouteNumber)
	{
		len+=snprintf(CDR, MAX, ";%s", callHandle->transRouteNumber);
	}
	else
	{
		len+=snprintf(CDR, MAX, ";");
	}

	//51 destination number type
	len+=snprintf(CDR, MAX, ";%d", callHandle->destCalledPartyNumType);

	//52 called number type
	if (callHandle->handleType == SCC_eH323CallHandle)
	{
		len+=snprintf(CDR, MAX, ";%d", callHandle->handle.h323CallHandle.origCalledPartyNumType);
	}
	else
	{
		len+=snprintf(CDR, MAX, ";");
	}
	
	//53 and 54
	if(local)
	{
		realmEntryPtr = NULL;
		CacheGetLocks(realmCache, LOCK_READ, LOCK_BLOCK);		
		realmEntryPtr = CacheGet(realmCache, &(local->realmId));		
		if (realmEntryPtr && strlen(realmEntryPtr->realm.realmName) )
		{
			len += snprintf( CDR, MAX, ";%s", realmEntryPtr->realm.realmName);
		}
		else
		{
			len += snprintf( CDR, MAX, ";");
		}
		CacheReleaseLocks(realmCache);
	}
	else
	{
		len += snprintf( CDR, MAX, ";");

	}

	if(remote)
	{
		realmEntryPtr = NULL;
		CacheGetLocks(realmCache, LOCK_READ, LOCK_BLOCK);		
		realmEntryPtr = CacheGet(realmCache, &(remote->realmId));		
		if (realmEntryPtr && strlen(realmEntryPtr->realm.realmName))
		{
			len += snprintf( CDR, MAX, ";%s", realmEntryPtr->realm.realmName);
		}
		else
		{
			len += snprintf( CDR, MAX, ";");
		}
		CacheReleaseLocks(realmCache);
	}	
	else 
	{
		len += snprintf( CDR, MAX, ";");
	}
    
	// NOTHING AFTER THIS:
	len+=snprintf(CDR, MAX, "\n");

	fd = fileno(file);
	if (write(fd, cdr, len) < 0)
	{
		int thiserr = errno;

		NETINFOMSG(MINIT, ("CDR write failed. Errno = %d\n", thiserr));
	}

	//fprintf(file, cdr);

	return len;
}

int
CdrLogCiscoRadius(CallHandle *callHandle, int flag)
{
	return sendCiscoRadiusAccounting(callHandle, flag);
}

static int billingEnabled = 1;

CdrArgs *
CdrArgsDup(CallHandle *callHandle, int flag)
{
	CallHandle *cdrcallHandle = NULL;
	CdrArgs *args;

	// At this point we must delegate
	cdrcallHandle = (CallHandle *)MMalloc(MEM_LOCAL, sizeof(CallHandle));

	memcpy(cdrcallHandle, callHandle, sizeof(CallHandle));

	// duplicate necessary fields
	cdrcallHandle->custID = CStrdup(callCache, callHandle->custID);
	cdrcallHandle->conf_id = CStrdup(callCache, callHandle->conf_id);
	cdrcallHandle->incoming_conf_id = 
	CStrdup(callCache, callHandle->incoming_conf_id);
	cdrcallHandle->tg = CStrdup(callCache, callHandle->tg);
	cdrcallHandle->destTg = CStrdup(callCache, callHandle->destTg);
	cdrcallHandle->dtgInfo = CStrdup(callCache, callHandle->dtgInfo);
	cdrcallHandle->srccrname = CStrdup(callCache, callHandle->srccrname);
	cdrcallHandle->transitcrname = CStrdup(callCache, callHandle->transitcrname);
	cdrcallHandle->destcrname = CStrdup(callCache, callHandle->destcrname);
	cdrcallHandle->transRouteNumber = CStrdup(callCache, callHandle->transRouteNumber);
	nx_strlcpy(cdrcallHandle->confID, callHandle->confID,CALL_ID_LEN);
	nx_strlcpy(cdrcallHandle->callID, callHandle->callID,CALL_ID_LEN);

	cdrcallHandle->realmInfo = RealmInfoDup(callHandle->realmInfo, MEM_LOCAL);

	args = (CdrArgs *)MMalloc(MEM_LOCAL, sizeof(CdrArgs));
	args->flag = flag;
	args->callHandle = cdrcallHandle;

	return args;
}
 
void
CdrArgsFree(CdrArgs *args)
{
	CallHandle *callHandle = NULL;

	if (!args)
	{
		return;
	}

	if ((callHandle = args->callHandle))
 	{
		if(callHandle->custID) CFree(callCache)(callHandle->custID);
		if(callHandle->conf_id) CFree(callCache)(callHandle->conf_id);
		if(callHandle->incoming_conf_id) CFree(callCache)(callHandle->incoming_conf_id);
		if(callHandle->tg) CFree(callCache)(callHandle->tg);
		if(callHandle->destTg) CFree(callCache)(callHandle->destTg);
		if(callHandle->dtgInfo) CFree(callCache)(callHandle->dtgInfo);
		if(callHandle->srccrname) CFree(callCache)(callHandle->srccrname);
		if(callHandle->transitcrname) CFree(callCache)(callHandle->transitcrname);
		if(callHandle->destcrname) CFree(callCache)(callHandle->destcrname);
		if(callHandle->transRouteNumber) CFree(callCache)(callHandle->transRouteNumber);

		if(callHandle->realmInfo)
		{
			RealmInfoFree(callHandle->realmInfo,MEM_LOCAL);
		}
			

		MFree(MEM_LOCAL, callHandle);
	}

	MFree(MEM_LOCAL, args);
}

int
CheckCdrEvents(CallHandle *callHandle, int flag)
{
	return (((flag == CDR_CALLSETUP) && 
			((callHandle->callSource && (cdrevents & CDRSTART2)) ||
			(!callHandle->callSource && (cdrevents & CDRSTART1)))) ||

			// For hunt CDRs, we should always let it proceed as later 
			// on we may determine that this cdr is our final cdr
			((flag == CDR_CALLHUNT) && 
			!callHandle->callSource) ||

			((flag == CDR_CALLDROPPED) &&
			((callHandle->callSource && (cdrevents & CDREND2)) ||
			(!callHandle->callSource && (cdrevents & CDREND1)))));
}


CdrArgs *
BillCallCreateCdr(CallHandle *callHandle, int flag, int *proceed)
{
	char fn[] = "BillCallCreateCdr():";
	FILE *file;
	char tmptime[256] = { 0 };
	int h, m, secs, dur;
	struct in_addr in;
	int len = 0;
	struct stat statbuf = { 0 };
	CdrArgs *args;

	if (billingEnabled == 0)
	{
		// Billing has been disabled, probably due to
		// shutdown or some other reason. Throw away this
		// cdr.
		return 0;
	}

	if (billingType == BILLING_NONE && !rad_acct)
	{
		return 0;
	}

	*proceed = CheckCdrEvents(callHandle, flag);
			
	if (!*proceed)
	{
		return 0;
	}

	if ((callHandle->nhunts > SYSTEM_MAX_HUNTS) &&
		callHandle->prevcdr)
	{
		// We don't need to do any extra work on logging this
		// cdr, as later on we will discard this logging of
		// this call

		return 0;
	}

	args = CdrArgsDup(callHandle, flag);

	return args;
}

// Print the hunting (prev) cdr, if hunting cdr's are enabled
int
BillCallPrevCdr(CallHandle *callHandle)
{
	if (callHandle->prevcdr == NULL)
	{
		return 0;
	}

	// If this is a hunt cdr, then check to see if hunting is
	// enabled. If not a hunt cdr, then just log it.
	if (((cdrevents & CDRHUNT) &&
			(callHandle->prevcdr->flag == CDR_CALLHUNT)) ||
		(callHandle->prevcdr->flag != CDR_CALLHUNT))
	{
		// Hunt cdr's are turned on
		cdrQueueCdr(callHandle->prevcdr);
	}
	else
	{
		CdrArgsFree(callHandle->prevcdr);
	}

	callHandle->prevcdr = NULL;
	
	return 0;
}

// Hold a new hunt cdr
int
BillCallHoldPrevCdr(CallHandle *callHandle, CdrArgs *args)
{
	CdrArgsFree(callHandle->prevcdr);
	callHandle->prevcdr = args;

	return 0;
}

// Set up the call handle with error/response code
// from the prev cdr
void
BillCallSetFromPrevCdr(CallHandle *callHandle)
{
	callHandle->callError = 
		callHandle->prevcdr->callHandle->callError;

	callHandle->cause = callHandle->prevcdr->callHandle->cause;
	callHandle->h225Reason = callHandle->prevcdr->callHandle->h225Reason;
	callHandle->rasReason = callHandle->prevcdr->callHandle->rasReason;

	callHandle->responseCode = 
		callHandle->prevcdr->callHandle->responseCode;
}

int
BillCall(CallHandle *callHandle, int flag)
{
	char fn[] = "BillCall():";
	FILE *file;
	char tmptime[256] = { 0 };
	int h, m, secs, dur;
	struct in_addr in;
	int len = 0, proceed = 0;
	struct stat statbuf = { 0 };
	CdrArgs *args;
	
	if (billingEnabled == 0)
	{
		// Billing has been disabled, probably due to
		// shutdown or some other reason. Throw away this
		// cdr.
		return 0;
	}

	if (billingType == BILLING_NONE)
	{
		return 0;
	}

	args = BillCallCreateCdr(callHandle, flag, &proceed);

	if (proceed == 0)
	{
		if (args)
		{
			NETERROR(MCDR, ("%s args should be NULL when proceed=0\n", 
				fn));
		}

		return 0;
	}

	// If this is a hunt CDR, then save the CDR inside the
	// call handle.
	// If hunting CDR logging is enabled, then log it also.

	if ((flag == CDR_CALLHUNT) && !callHandle->callSource)
	{
		// args MUST be there in this case
		if (args)
		{
			// Log the previous hunt cdr, if one exists
			BillCallPrevCdr(callHandle);

			// Store this one
			BillCallHoldPrevCdr(callHandle, args);
		}
		else
		{
			NETERROR(MCDR, ("%s Invalid scenario, args is NULL for hunt\n", fn));
		}

		return 0;
	}
	else if (!callHandle->callSource)
	{
		// Non hunt cdr.
		// Check to see if we need to resort to the hunting cdr
		// which may be saved off in the call handle
		// If CDR is invalid, we can resort to the prev one

		if (!BillCallIsCdrValid(callHandle))
		{
			// Change the context to present context
			callHandle->prevcdr->flag = flag;

			// Since this is the final cdr for the call,
			// set up the callError and cause to reflect that.
			// This way the protocol error will be consistent with the cdr

			BillCallSetFromPrevCdr(callHandle);

			BillCallPrevCdr(callHandle);

			// Skip the logging of the actual call
			// Make sure we free the args. Code has been
			// optimized not to allocate args in this case.
			CdrArgsFree(args);

			return 0;
		}
	}

	if (args)
	{
		cdrQueueCdr(args);
	}
	else
	{
		NETERROR(MCDR, ("%s Invalid scenario, args is NULL\n", fn));
	}

	return 0;
}

int
ConfEnd(void)
{
	char fn[] = "ConfEnd():";
	int n = 0, i, flag = CDR_CALLDROPPED;
	CallHandle *callHandle;
	ConfHandle *confHandle;

	// Destroy the conf cache, and print all
	// the cdr's in it.
	
	CacheGetLocks(callCache, LOCK_WRITE, LOCK_BLOCK);
	CacheGetLocks(confCache, LOCK_WRITE, LOCK_BLOCK);

	if (confCache && (confCache->nitems != 0))
	{
		NETINFOMSG(MDEF, 
			("shutdown: %d calls still active\n", confCache->nitems));
	}

	for (confHandle = CacheGetFirst(confCache);
		confHandle;
		confHandle = CacheGetNext(confCache, confHandle->confID))
	{
		// CDRs will be printed for leg1 and leg2
		// for unbilled calls 
		for(i = 0; i<confHandle->ncalls;++i)
		{
			callHandle = CacheGet(callCache, confHandle->callID[i]);			
			if (callHandle)
			{
				if ((callHandle->callError == 0) &&
					(CheckCdrEvents(callHandle, flag)) > 0) 
				{
					// This call hasnt been billed as of now
					callHandle->callError = SCC_errorShutdown;
					BillCallWorker(callHandle, flag);
				}
			}
		}
	}

	// Also mark the billing thread not to take any new calls,
	// just in case shutdown is delayed due to other activity
	billingEnabled = 0;
		
	CacheReleaseLocks(confCache);
	CacheReleaseLocks(callCache);

	return 0;
}

int
BillCallScheduler(CdrArgs *args)
{
	BillCallWorker(args->callHandle, args->flag);

	CdrArgsFree(args);

	return(0);
}

// Logging entry point
// Logging can either be directed to syslog
// OR a local file, similar to MIND files, in MIND format
// OR a local file, similar to MIND files, in XML format
// OR to network TCP/UDP in XML format
int
BillCallWorker(CallHandle *callHandle, int flag)
{
	char fn[] = "BillCall():";
	FILE *file;
	char tmptime[256] = { 0 };
	int h, m, secs, dur;
	struct in_addr in;
	int len = 0;
	struct stat statbuf = { 0 };

	if (billingType == BILLING_NONE)
	{
		return 0;
	}

	if (cdrformat == CDRFORMAT_SYSLOG)
	{
		return CdrLogSyslog(callHandle, flag);
	}

	if (localConfig.cdrfile == NULL)
	{
		 return 0;
	}

	if (rad_acct)
	{
		CdrLogCiscoRadius(callHandle, flag);
	}
	
	LockGetLock(&cdr_mutex, 0, 0);

	if ((filelen > MAXFILESIZE) &&
		(localConfig.cdrfiletype == CDRMINDCTISEQ))
	{
		 CdrRotateFile();
	}

	if ( stat( (const char*) cdrpname, &statbuf) < 0)
	{
		 CdrRotateFile();
	}

	file = ((flag == CDR_CALLSETUP)||callHandle->callSource)?localConfig.ctrfile:localConfig.cdrfile;

	if (file == NULL)
	{
		NETDEBUG(MDEF, NETLOG_DEBUG4, ("Skipping CDR logging\n"));
		goto _return;
	}

	if (cdrformat == CDRFORMAT_MIND)
	{
		filelen += CdrLogMindCti(file, callHandle, flag);
	}
	else if (cdrformat == CDRFORMAT_XML)
	{
		filelen += CdrLogXml(file, callHandle, flag);
	}

_return:
_error:
	LockReleaseLock(&cdr_mutex);

	return 0;
}

int
CdrCountFiles(char *dir, char *pattern)
{
	char fn[] = "CdrCountFiles():";
	int myerrno, pattlen, fnamelen, count = 0;
	DIR *dirp;
	struct dirent *dp;
	char dentry[sizeof(struct dirent)+ PATH_MAX + 1];

	/* count files which match pattern in directory dir */
	dirp = opendir(dir);

	if (!dirp)
	{
		myerrno = errno;
		NETERROR(MINIT, ("opendir failed errno = %d\n", myerrno));
		return -1;
	}

	while (!readdir_r(dirp, (struct dirent *)dentry, &dp) && dp)
	{
		if (!fnmatch(pattern, dp->d_name, FNM_PATHNAME|FNM_PERIOD))
		{
			count ++;
		}
	}

	if (closedir(dirp))
	{
		myerrno = errno;
		NETERROR(MINIT, ("opendir failed errno = %d\n", myerrno));
	}

	return count;
}

// Move files identified by temporary extension
// to a new extenstion. Exclude the current working file
// if it is specified.
int
CdrMoveTempFiles(char *dir, char *textn, char *extn, char *exclude)
{
	char fn[] = "CdrMoveTempFiles():";
	int extlen, textlen, i = 0, n = 0;
	char newname[MAXCDRPNAME], pname[MAXCDRPNAME], newpname[MAXCDRPNAME];
	int myerrno, fnamelen, count = 0;
	DIR *dirp;
	struct dirent *dp;
	struct stat statbuf;
	char dentry[sizeof(struct dirent)+ PATH_MAX + 1];

	/* count files which match pattern in directory dir */
	dirp = opendir(dir);

	if (!dirp)
	{
		myerrno = errno;
		NETERROR(MINIT, ("opendir failed errno = %d\n", myerrno));
		return -1;
	}

	textlen = strlen(textn);

	while (!readdir_r(dirp, (struct dirent *)dentry, &dp) && dp)
	{
		if (exclude && !strcmp(exclude, dp->d_name))
		{
			// skip this file
			continue;
		}

		fnamelen = strlen(dp->d_name);

		if (!strncmp(dp->d_name+fnamelen-textlen, textn, textlen))
		{
			// file matches extension
			nx_strlcpy(newname, dp->d_name, MAXCDRPNAME);
			nx_strlcpy(newname+fnamelen-textlen, extn, MAXCDRPNAME-(fnamelen-textlen));
		}
		else
		{
			continue;
		}

		// append dirname
		snprintf(newpname, MAXCDRPNAME, "%s/%s", dir, newname);
		snprintf(pname, MAXCDRPNAME, "%s/%s", dir, dp->d_name);

		// Before we move, we MUST check if the
		// file already exists
		if (stat(newpname, &statbuf)) 
		{
			// file does not exist
			if (rename(pname, newpname) < 0)
			{
				NETERROR(MINIT,
					("%s Could not rename %s -> %s, errno %d\n",
					fn, pname, newpname, errno));
			}
		}
		else
		{
			// file exists, do not do any rename
			// as this may lead to loss of cdr files.
			// We know that this is not the current cdr file
			// as this was not the one excluded, so complain
			NETERROR(MINIT,
				("%s File %s exists, cannot rename %s -> %s\n",
					fn, newpname, pname, newpname));
		}

		i++;
	}

	if (closedir(dirp))
	{
		myerrno = errno;
		NETERROR(MINIT, ("opendir failed errno = %d\n", myerrno));
	}

	return i;
}

#ifndef SUNOS
void 
cftime(char *s, char *format, const time_t *clock)
{
	struct tm *tmptr;

	/* use strftime */
	tmptr = gmtime(clock);
	strftime(s, 256, format, tmptr);
}

#endif

int
cdrQueueCdr(void *arg)
{
	char fn[] = "cdrQueueCdr():";

	if (ThreadDispatch(cdrinPool, 0, (void*(*)(void*)) BillCallScheduler, arg,
			1, PTHREAD_SCOPE_PROCESS, SCHED_FIFO, 59))
	{
		NETERROR(MDEF, ("%s Error in dispatching\n", fn));
		return(-1);
	}
	return(0);
}
