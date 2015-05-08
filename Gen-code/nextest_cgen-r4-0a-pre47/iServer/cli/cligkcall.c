#include <unistd.h>
#include <limits.h>
#include "cli.h"
#include "serverp.h"
#include "gw.h"
#include "license.h"
#include "gis.h"
#include "qmsg.h"
#include "gisq.h"
#include "nxosd.h"
#include "log.h"

#undef malloc
#undef calloc
#undef free

#define MIN(x,y)	((x<y)?x:y)
#define MAX(x,y)	((x<y)?y:x)
#define NZMIN(x, y)	((x==0)?y:((y==0)?x:MIN(x,y)))
#define MAX_PATH_FILENAME_LEN  512

int
HandleCallCache(Command *comm, int argc, char **argv)
{
	char fn[] = "HandleCallCache():";
	char callID[CALL_ID_LEN] = { 0 };
	int rc = xleOk;
	QDesc sqdesc, dqdesc;
	time_t now;
	char qgismsg[sizeof(QMsgHdr)+MAX_PATH_FILENAME_LEN];
	CallHandle *callHandle;
	char *cwd = NULL;
	char *fpath = qgismsg + sizeof(QMsgHdr);

    if (argc < 1)
    {
		/* Here we prompt the user for the rest of the 
		 * information
		 */
		HandleCommandUsage(comm, argc, argv);
		return -xleInsuffArgs;
    }

	/* Write output file to cli's current working directory - TIcket 7012 */
	cwd = getcwd(NULL, MAX_PATH_FILENAME_LEN);
	if (cwd == NULL)
	{
		CLIPRINTF((stdout, "failed to get current working directory.\n"));
		rc = -xleOpNoPerm;  /* need to define a new rc */
		goto _return;
	}
	if ((sizeof(cwd) +  sizeof(argv[0])) >= MAX_PATH_FILENAME_LEN - 1)
	{
		CLIPRINTF((stdout, "%s - file name path too long.\n", argv[0]));
		rc = -xleOpNoPerm;  /* need to define a new rc */
		goto _return;
	}

	switch(cwd[0])
	{
	case '\\':
	case '~':
		strcpy(fpath, argv[0]);
		break;
	default:
		sprintf(fpath, "%s\\%s", cwd, argv[0]);
	}

	CLIPRINTF((stdout, "output dumped into file %s\n", fpath));
	if (QGet(&dqdesc, ISERVER_GIS_Q, GIS_SRVR_MSG_TYPE) < 0)
	{
		CLIPRINTF((stdout, "%s could not attach to gis queue\n", fn));
		rc = -xleOpNoPerm;
		goto _return;
	}

	time(&now);
	if (QGet(&sqdesc, ISERVER_GIS_Q, now) < 0)
	{
		CLIPRINTF((stdout, "%s could not attach to gis queue from cli\n", fn));
		rc = -xleOpNoPerm;
		goto _return;
	}

	if (QSendto(&dqdesc, &sqdesc, (QMsg *)qgismsg, 
			GISQMSG_CLICMD, sizeof(QMsgHdr)+512, 0) < 0)
	{
		CLIPRINTF((stdout, "%s could not send msg to gis queue\n", fn));
		rc = -xleOpNoPerm;
		goto _return;
	}

_return:

	return rc; 
}

int
HandleCallCacheDebug(Command *comm, int argc, char **argv)
{
	char fn[] = "HandleCallCache():";
	CallHandle *callHandle;
	ConfHandle *confHandle;
	SipCallHandle *sipCallHandle;
	SipTrans *sipTrans;
	char str1[CALL_ID_LEN], str2[CALL_ID_LEN];
	char zerocallID[CALL_ID_LEN] = { 0 };
	int incompConf = 0, idleStateCalls = 0, missingCalls = 0, invalidConf = 0,
		zeroCalls = 0;
	time_t badLow = 0, badHigh = 0, goodLow = 0, goodHigh = 0, tmptime1, tmptime2;
	char	ctimebuf[256];
	int n = 0, m = 0;
	int shmId;
	int i;
    char ttypname[2*_POSIX_PATH_MAX];

	if (argc != 0)
	{
		/* Here we prompt the user for the rest of the 
		 * information
		 */
		HandleCommandUsage(comm, argc, argv);
		return -xleInsuffArgs;
	}

	if ((shmId  = CacheAttach())== -1)
	{
		CLIPRINTF((stdout, "Unable to attach to GIS cache\n"));
	}
	else
	{
    	if (ttyname_r(1, ttypname, 2*_POSIX_PATH_MAX) != ENOTTY)
    	{
        	CLIPRINTF((stdout, "Please redirect output to a file\n"));
        	return 0;
    	}

		CLIPRINTF((stdout, "GK Call Cache...\n"));
     
		CacheGetLocks(callCache, LOCK_WRITE, LOCK_BLOCK);
		CacheGetLocks(confCache, LOCK_WRITE, LOCK_BLOCK);

		for (confHandle = CacheGetFirst(confCache);
			 confHandle;
			 confHandle = CacheGetNext(confCache, confHandle->confID))
		{	 
			n ++;
			CLIPRINTF((stdout, "+++++++++++++++++++++++++++++++++++++++++++++++++\n"));
			CLIPRINTF((stdout, "Conf %d:\n", n));
			PrintConfEntry(stdout,confHandle);
			CLIPRINTF((stdout, "\n+++++++++++++++++++++++++++++++++++++++++++++++++\n"));

	 		if (confHandle->ncalls < 2) incompConf ++;
	 		if (confHandle->ncalls > 2) invalidConf ++;

			for(i = 0; i<confHandle->ncalls;++i)
			{
				if (memcmp(confHandle->callID[i], zerocallID, CALL_ID_LEN))
				{
					callHandle = CacheGet(callCache, confHandle->callID[i]);
					if (callHandle)
					{
						if (callHandle->state == SCC_sIdle) idleStateCalls ++;

						CLIPRINTF((stdout, "*** Call %d Leg %d %s ***\n",
						i, callHandle->leg, 
						(callHandle->handleType == SCC_eH323CallHandle)?
						"h.323":
						(callHandle->handleType == SCC_eSipCallHandle)?
						"sip":"unknown"));

						PrintCallEntry(stdout, callHandle);
						CLIPRINTF((stdout, "******************\n"));
						m++;

						tmptime1 = NZMIN(timedef_sec(&callHandle->callStartTime), 
							NZMIN(timedef_sec(&callHandle->callConnectTime), 
							timedef_sec(&callHandle->callEndTime)));

						tmptime2 = MAX(timedef_sec(&callHandle->callStartTime), 
							MAX(timedef_sec(&callHandle->callConnectTime), 
							timedef_sec(&callHandle->callEndTime)));

						if (callHandle->state == SCC_sIdle)
						{
							// Find the times
							if (!badLow || 
								(tmptime1 && (tmptime1 < badLow)))
							{
								badLow = tmptime1;
							}

							if (tmptime2 > badHigh)
							{
								badHigh = tmptime2;
							}
						}
						else
						{
							// Find the times
							if (!goodLow ||
								(tmptime1 && (tmptime1 < goodLow)))
							{
								goodLow = tmptime1;
							}

							if (tmptime2 > badHigh)
							{
								goodHigh = tmptime2;
							}
						}
					}
					else
					{
						missingCalls++;
					}
				}
				else
				{
					zeroCalls ++;
				}
			}

		}
	
		/* Release the iterator */
		CacheFreeIterator(confCache);
	 
		CLIPRINTF((stdout, "\nGK %d Active Conferences %d Active Calls\n", n, m));

		CLIPRINTF((stdout, "GK Sip Call Cache...\n"));
		for (callHandle = CacheGetFirst(sipCallCache);
			 callHandle;
			 callHandle = CacheGetNext(sipCallCache, &sipCallHandle->callLeg))
		{
			CLIPRINTF((stdout, "call state %s\n", GetSipState(callHandle->state)));

			sipCallHandle = SipCallHandle(callHandle);
			CLIPRINTF((stdout, "sip call:\n"));
			CLIPRINTF((stdout, "cid %s cfid %s leg %d\n",
				CallID2String(callHandle->callID, str1),
				CallID2String(callHandle->confID, str2),
				callHandle->leg));
			if (callHandle->handleType != SCC_eSipCallHandle)
			{
				CLIPRINTF((stdout, "malformed call handle\n"));
			}
			if (sipCallHandle->callLeg.local)
				CLIPRINTF((stdout, "local: %s@%s:%d; tag=%s\n", 
					SVal(sipCallHandle->callLeg.local->name),
					SVal(sipCallHandle->callLeg.local->host), 
					sipCallHandle->callLeg.local->port,
					SVal(sipCallHandle->callLeg.local->tag)));
			if (sipCallHandle->callLeg.remote)
				CLIPRINTF((stdout, "remote: %s@%s:%d; tag=%s\n", 
					SVal(sipCallHandle->callLeg.remote->name),
					SVal(sipCallHandle->callLeg.remote->host), 
					sipCallHandle->callLeg.remote->port,
					SVal(sipCallHandle->callLeg.remote->tag)));
			CLIPRINTF((stdout, "callid: %s\n", sipCallHandle->callLeg.callid));
			if (sipCallHandle->requri)
				CLIPRINTF((stdout, "requri: %s@%s:%d\n", 
					SVal(sipCallHandle->requri->name),
					SVal(sipCallHandle->requri->host), 
					sipCallHandle->requri->port));
			if (sipCallHandle->localContact)
				CLIPRINTF((stdout, "local contact: %s@%s:%d\n", 
					SVal(sipCallHandle->localContact->name),
					SVal(sipCallHandle->localContact->host), 
					sipCallHandle->localContact->port));
			if (sipCallHandle->remoteContact)
				CLIPRINTF((stdout, "remote contact: %s@%s:%d\n", 
					SVal(sipCallHandle->remoteContact->name),
					SVal(sipCallHandle->remoteContact->host), 
					sipCallHandle->remoteContact->port));
			CLIPRINTF((stdout, "successful invites = %d\n", sipCallHandle->successfulInvites));
			CLIPRINTF((stdout, "\n"));
		}
		CacheFreeIterator(sipCallCache);

		CacheReleaseLocks(callCache);
		CacheReleaseLocks(confCache);

		CLIPRINTF((stdout, "incomplete confs = %d, idle call legs = %d, missing call legs = %d, invalid confs = %d, zero call ids = %d\n",
			incompConf, idleStateCalls, missingCalls, invalidConf, zeroCalls));

		if (badLow)
		{
			ctime_r( &badLow, ctimebuf );
			CLIPRINTF((stdout, "earliest active hung call was at %s", ctimebuf));
		}

		if (badHigh)
		{
			ctime_r( &badHigh, ctimebuf );
			CLIPRINTF((stdout, "current active hung call was at %s", ctimebuf));
		}

		if (goodLow)
		{
			ctime_r( &goodLow, ctimebuf );
			CLIPRINTF((stdout, "earliest active good call was at %s", ctimebuf));
		}

		if (goodHigh)
		{
			ctime_r( &goodHigh, ctimebuf );
			CLIPRINTF((stdout, "current active good call was at %s", ctimebuf));
		}

		CacheDetach();
	}
	 
	return xleOk; 
}
int
HandleCallLkup(Command *comm, int argc, char **argv)
{
	char fn[] = "HandleCallLkup():";
	char callID[CALL_ID_LEN] = { 0 };
	int rc = xleOk;
	CallHandle *callHandle;
	QDesc sqdesc, dqdesc;
	time_t 		now;
	char        qgismsg[sizeof(QMsgHdr)+CALL_ID_LEN + 512];

    if (argc < 2)
    {
		HandleCommandUsage(comm, argc, argv);
		return -xleInsuffArgs;
    }
	
	String2Guid(argv[0], callID);

	if (strlen(argv[1]) > 512)
	{
		CLIPRINTF((stdout, "%s File name too long\n", fn));
		goto _error;
	}

	// Send a message to the gis to delete this call
	if (QGet(&dqdesc, ISERVER_GIS_Q, GIS_SRVR_MSG_TYPE) < 0)
	{
		CLIPRINTF((stdout, "%s could not attach to gis queue\n", fn));
		rc = -xleOpNoPerm;
		goto _error;
	}

	time(&now);
	if (QGet(&sqdesc, ISERVER_GIS_Q, now) < 0)
	{
		CLIPRINTF((stdout, "%s could not attach to gis queue from cli\n", fn));
		rc = -xleOpNoPerm;
		goto _error;
	}

	memcpy(qgismsg+sizeof(QMsgHdr), callID, CALL_ID_LEN);
	strcpy(qgismsg+sizeof(QMsgHdr)+CALL_ID_LEN, argv[1]);

	if (QSendto(&dqdesc, &sqdesc, (QMsg*)qgismsg, 
		GISQMSG_CALLPRINT, (sizeof(QMsgHdr)+CALL_ID_LEN+512), 0) < 0)
	{
		CLIPRINTF((stdout, "%s could not send msg to gis queue\n", fn));
		rc = -xleOpNoPerm;
		goto _error;
	}

_error:
	return rc;
}

int
HandleCallDelete(Command *comm, int argc, char **argv)
{
	char fn[] = "HandleCallDelete():";
	char callID[CALL_ID_LEN] = { 0 };
	int rc = xleOk;
	QDesc sqdesc, dqdesc;
	time_t now;
	char qgismsg[sizeof(QMsgHdr)+CALL_ID_LEN];
	CallHandle *callHandle;

   if (argc < 1)
    {
		/* Here we prompt the user for the rest of the 
		 * information
		 */
		HandleCommandUsage(comm, argc, argv);
		return -xleInsuffArgs;
    }
	
	String2Guid(argv[0], callID);

	// Send a message to the gis to delete this call
	if (QGet(&dqdesc, ISERVER_GIS_Q, GIS_SRVR_MSG_TYPE) < 0)
	{
		CLIPRINTF((stdout, "%s could not attach to gis queue\n", fn));
		rc = -xleOpNoPerm;
		goto _return;
	}
	
	time(&now);
	if (QGet(&sqdesc, ISERVER_GIS_Q, now) < 0)
	{
		CLIPRINTF((stdout, "%s could not attach to gis queue from cli\n", fn));
		rc = -xleOpNoPerm;
		goto _return;
	}
	
	memcpy(qgismsg+sizeof(QMsgHdr), callID, CALL_ID_LEN);
	
	if (QSendto(&dqdesc, &sqdesc, (QMsg *)qgismsg, 
		    GISQMSG_CALLDROP, sizeof(QMsgHdr)+CALL_ID_LEN, 0) < 0)
	{
		CLIPRINTF((stdout, "%s could not send msg to gis queue\n", fn));
		rc = -xleOpNoPerm;
		goto _return;
	}
	
_return:

	return rc;
}

