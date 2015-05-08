#include "qmsg.h"
#include "gisq.h"
#include "gis.h"
#include "nxosd.h"
#include "bridge.h"
#include "licenseIf.h"
#include "log.h"

#include "bridge.h"
#include "ssip.h"

int
GisHandleCliCmd(void *data, int mtype, char *msg, int len)
{
	char fn[] = "GisHandleCliCmd():";
	char fname[512];
	FILE *out;

	NETERROR(MINIT, ("%s received message type %d len = %d\n",
		fn, mtype, len));

	nx_strlcpy(fname, msg+sizeof(QMsgHdr), 512);

	// A cli command has arrived, we will have to find
	// out what command it is
	
	// Now pass this command to the cli library	
	// open an output file for the cli library to use
	// and return that file back to the command

   	if ((out = fopen(fname, "w")) != NULL)
	{
		PrintCallCache(out);
		fclose(out);
	}
	else
	{
		NETERROR(MINIT, ("%s file open failed!\n", fn));
	}

	return 0;
}

int
GisHandleCallDropMsg(void *data, int mtype, char *msg, int len)
{
	char fn[] = "GisHandleCallDropMsg():";
	char callID[CALL_ID_LEN];
	char callIDStr[CALL_ID_LEN];
	
	NETERROR(MINIT, ("%s received message type %d len = %d\n",
		fn, mtype, len));

	memcpy(callID, msg+sizeof(QMsgHdr), CALL_ID_LEN);

	NETERROR(MSCC,("%s Administratively bringing down call %s\n",
		fn, (char*) CallID2String(callID,callIDStr)));

	disconnectCall(callID, SCC_errorAdmin);

	return 0;
}

int
GisHandleCallPrintMsg(void *data, int mtype, char *msg, int len)
{
	char fn[] = "GisHandlePrintCall():";
	char	callID[CALL_ID_LEN] = {0};
	char callIDStr[CALL_ID_LEN];
	CallHandle *callHandle=NULL;
	QMsgHdr *qmsghdr;
	char fname[512];
	FILE *out;

	qmsghdr = (QMsgHdr *)msg;
	memcpy(callID, msg+sizeof(QMsgHdr), CALL_ID_LEN);
	nx_strlcpy(fname, msg+sizeof(QMsgHdr)+CALL_ID_LEN, 512);

	callHandle = CacheGet(callCache, callID);
	if (callHandle == NULL)
	{
		NETERROR(MINIT, ("%s no CallHandle for %s\n", fn, CallID2String(callID,callIDStr)));
		return 0;
	}

	if ((out = fopen(fname, "w")) != NULL)
	{
		PrintCallEntry(out, callHandle);
		fclose(out);
	}
	else
	{
		NETERROR(MINIT, ("%s file open failed!\n", fn));
	}

	return 0;
}

int
GisHandleRealmMsg(void *data, int mtype, char *msg, int len)
{
	char fn[] = "GisHandleRealmMsg():";
	unsigned long rsa;
	QMsgHdr *qmsghdr;

	NETERROR(MINIT, ("%s received message type %d len = %d\n",
		fn, mtype, len));

	qmsghdr = (QMsgHdr *)msg;
	memcpy(&rsa, msg+sizeof(QMsgHdr), sizeof(unsigned long));
	rsa = ntohl(rsa);

	disconnectCallsOnRealm(rsa, SCC_errorAdmin);

	if(h323Enabled())
	{
		UH323RealmReconfig(rsa);
	}

	if(sipEnabled())
	{
		SipRealmReconfig(rsa);
	}

	// Send ACK back
	QSendto(&qmsghdr->destdesc, &qmsghdr->srcdesc, (QMsg*) msg, 
		GISQMSG_OK, sizeof(QMsgHdr), 0);
	
	return 0;
}

int
GisQInit()
{
	if (QCreateDispatchTable(&gisQDispatchTable, GISQMSG_MAX) < 0)
	{
		return -1;
	}

	if (QCreate(&gisQ, ISERVER_GIS_Q, GIS_SRVR_MSG_TYPE, 0, 256, 
		sizeof(QGisSrvrMsg)) < 0)
	{
		return -1;
	}

	// Register callbacks for message types
	QRegisterMsgType(&gisQDispatchTable, GISQMSG_RSA, 
		GisHandleRealmMsg, NULL);
	QRegisterMsgType(&gisQDispatchTable, GISQMSG_CALLDROP, 
		GisHandleCallDropMsg, NULL);
	QRegisterMsgType(&gisQDispatchTable, GISQMSG_CLICMD, 
		GisHandleCliCmd, NULL);
	QRegisterMsgType(&gisQDispatchTable, GISQMSG_CALLPRINT, 
		GisHandleCallPrintMsg, NULL);

    if (ThreadLaunch(GisQReader, NULL, 1) != 0)
	{
		NETERROR(MINIT, ("Could not launch thread\n"));
		return -1;
	}
    
    return 0;
}

/* 
 * The following functions are example code of how to use the above 
 * mentioned apis
 */

/*
 * The QSrvReader function can be called as a function or
 * Dispatched to a thread and then thread will become the 
 * reader thread.
 * execd will call it as a function where as gis will dispatch
 * it to a reader thread
 */
static void*
GisQReader(void *arq)
{
	char 	gisqmsg[1024];

    while(1)
    {
        if (QRcvfrom(&gisQ, (QMsg *)&gisqmsg, 1024, 0, 
			&gisQDispatchTable) < 0)
		{
			/* fatal error bail out */
			return 0;	
		}
    }

    return 0;
}
