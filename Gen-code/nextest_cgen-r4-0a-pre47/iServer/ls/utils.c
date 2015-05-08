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
#endif
#include <string.h>
#ifdef _QNX
#include <sys/select.h>
#endif
#include <sys/uio.h>
#include <signal.h>
#include <sys/wait.h>
#include <license.h>
#include "poll.h"

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
#include "sconfig.h"

#include "gis.h"
#include "lrq.h"

#include "ls.h"

int
GisExtractRegIdInfo(char *endptIdStr, PhoNode *phonodep)
{
	char *token,*ptrptr;
	int caps;
    phonodep->realmId = REALM_ID_UNASSIGNED;

#ifdef SUNOS
	token = strtok_r(endptIdStr, "!", (char **)&ptrptr);
#else
	token = strsep(&endptIdStr, "!");
#endif

	if (token && strlen(token))
	{
		  strncpy(phonodep->regid, token, REG_ID_LEN);
		  BIT_SET(phonodep->sflags, ISSET_REGID);
	}

#ifdef SUNOS
	token = strtok_r(endptIdStr, "!", (char **)&ptrptr);
#else
	token = strsep(&endptIdStr, "!");
#endif

	if (token && strlen(token))
	{
		  phonodep->ipaddress.l = ntohl(atoi(token));
		  BIT_SET(phonodep->sflags, ISSET_IPADDRESS);
	}

#ifdef SUNOS
	token = strtok_r(endptIdStr, "!", (char **)&ptrptr);
#else
	token = strsep(&endptIdStr, "!");
#endif

	if (token && strlen(token))
	{
		  phonodep->uport = ntohl(atoi(token));
		  BIT_SET(phonodep->sflags, ISSET_UPORT);
	}

#ifdef SUNOS
	token = strtok_r(endptIdStr, "!", (char **)&ptrptr);
#else
	token = strsep(&endptIdStr, "!");
#endif

	if (token && strlen(token))
	{
		  phonodep->realmId = atoi(token);
	}

	return 0;
}

int
GisVerifyRegIdInfo(char *endptIdStr, PhoNode *phonodep)
{
	 char *token, *ptrptr;
#ifdef SUNOS
	 token = strtok_r(endptIdStr, "!", (char **)&ptrptr);
#else
	 token = strsep(&endptIdStr, "!");
#endif

	 if (token && strlen(token) && 
		BIT_TEST(phonodep->sflags, ISSET_REGID))
	 {
		if (strncmp(phonodep->regid, token, REG_ID_LEN))
		{
			return 1;
		}
	 }

#ifdef SUNOS
	 token = strtok_r(endptIdStr, "!", (char **)&ptrptr);
#else
	 token = strsep(&endptIdStr, "!");
#endif

	 if (token && strlen(token) &&
		BIT_TEST(phonodep->sflags, ISSET_IPADDRESS))
	 {
		if (phonodep->ipaddress.l != ntohl(atoi(token)))
		{
			return 1;
		}
	 }

	 return 0;
}

int
uh323AddRASAlias(HRAS hsRas, cmAlias *alias, int *index, 
				 cmRASTrStage stage, cmRASParam param)
{
	 char fn[] = "uh323AddRASAlias():";

	 if (alias->length <= 0)
	 {
		  return 0;
	 }

	 NETDEBUG(MH323, NETLOG_DEBUG1,
		   ("Alias - %s\n", alias->string));

	 if (cmRASSetParam(hsRas,
					   stage,
					   param, *index, 
					   sizeof(cmAlias), (char *)alias) < 0)
	{
		 NETERROR(MH323, 
				  ("%s cmRASSetParam failed stage = %d, param = %d, index = %d\n",
				   fn, stage, param, *index));
		 return 0;
	}	

	 *index = -1;
	 return 0;
}

