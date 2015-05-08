#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <netdb.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <poll.h>
#include <netinet/in.h>
#include <time.h>
#include <libradius.h>
#include <conf.h>
#include <radpaths.h>
#include "cdr.h"
#include "radclient.h"
#include "radacct.h"
#include "srvrlog.h"
#include "licenseIf.h"
#include "lsconfig.h"
#include <malloc.h>
#include "iwfutils.h"

#define Val(_x_)       ((_x_)?(_x_):"")

#ifndef INADDR_NONE
#define INADDR_NONE     ((in_addr_t) 0xffffffff)
#endif

typedef struct
{
	enum {NEW_REQUEST=1, BACKLOG_REQUEST, RESPONSE, TIMEOUT} event;
	RADIUS_PACKET *packet;
} RadacctEvent;


typedef struct _RadacctListEntry
{
	struct _RadacctListEntry *prev;
	struct _RadacctListEntry *next;
	AccountingInfo *acct_info;
	RADIUS_PACKET *packet;
	int server;
	int retries;
	time_t timestamp;
} RadacctListEntry;


typedef struct _RadacctServer
{
	int id;
	int ipaddr;
	int port;
	int dead;
	time_t fail_time;
} RadacctServer;


int radius_initialised;

static int radacctinPool;
static int radacctinClass;

static int sockfd;

static RadacctServer radacct_server[MAX_NUM_RAD_ENTRIES];
static int num_radacct_servers;

static int acct_session_id;

int max_outstanding_requests = 1000;

static int current_pending_requests = 0;
static int backlog_pending_requests = 0;

void *sendCiscoRadiusAcctWorker(void *arg);
void *recvCiscoRadiusAcctResponses(void *arg);
int radAcctTimer(struct Timer *t);

static RADIUS_PACKET *constructAccountingRequest(AccountingInfo *data, int id, int sockfd, int ipaddr, int port);
static AccountingInfo *createAccountingInfo(CallHandle *callHandle, int flag);
static void freeAccountingInfo(AccountingInfo *data);


List radacctList[256];


int initRadiusAcct()
{
	char fn[] = "initRadiusAcct";
	struct  itimerval tmr;
	int i, j;
	RadacctEvent *evtPtr;
	struct sockaddr_in cliaddr;

	acct_session_id = ((int)getpid() & 0xffff);

	for(i = 0, j = 0; i < MAX_NUM_RAD_ENTRIES; ++i)
	{
		if(*rad_server_addr[i])
		{
			if((radacct_server[j].ipaddr = ip_getaddr(rad_server_addr[i])) != INADDR_NONE)
			{
				radacct_server[j].id = i;

				if((radacct_server[j].port = rad_getport("radacct")) == 0)
				{
					radacct_server[j].port = PW_ACCT_UDP_PORT;
				}

				radacct_server[j].dead = 0;

				++j;
			}
			else
			{
				NETERROR(MRADC, ("%s Unable to determine ip address for Radius Server (%s)\n",
											 fn, rad_server_addr[i]));
			}
		}
	}

	if((num_radacct_servers = j) == 0)
	{
		NETERROR(MRADC, ("%s Unable to determine ip address for any Radius Server\n", fn));

		radacct_server[0].ipaddr = INADDR_NONE;
	}

	initRadacct_db(&current_pending_requests, &backlog_pending_requests);

	for(i = 0; i < 256; ++i)
	{
		radacctList[i] = listInit();
	}

	if((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0)
	{
		NETERROR(MRADC, ("%s Unable to create socket \n", fn));
		return 1;
	}

	// if management interface is configured, bind to it
	if( strlen(mgmtInterfaceIp) )
	{
		bzero(&cliaddr,sizeof(cliaddr));
		cliaddr.sin_family = AF_INET;
		cliaddr.sin_addr.s_addr = inet_addr(mgmtInterfaceIp);
		cliaddr.sin_port = htons(0);

		if(bind(sockfd,(struct sockaddr *)&cliaddr,sizeof(cliaddr)) < 0)
		{
			NETERROR(MRADC, ("%s Unable to bind socket \n", fn));
			close(sockfd);
			return 1;
		}
	}

	radacctinPool = ThreadPoolInit("radacct", 1, PTHREAD_SCOPE_PROCESS, 0, 1);

	radacctinClass = ThreadAddPoolClass("radacct-class", radacctinPool, 0, 100000000);

	ThreadPoolStart(radacctinPool);

	ThreadLaunch(recvCiscoRadiusAcctResponses, NULL, 1);

	memset(&tmr, 0, sizeof(struct itimerval));
	tmr.it_value.tv_sec = 1;
	tmr.it_interval.tv_sec = 1;

	timerAddToList(&localConfig.timerPrivate, &tmr, 0, PSOS_TIMER_REL, "RadAcctTimer", radAcctTimer, NULL);

	if(current_pending_requests > 0 || backlog_pending_requests > 0)
	{
		if((evtPtr = malloc(sizeof(RadacctEvent))))
		{
			memset(evtPtr, 0, sizeof(RadacctEvent));
			evtPtr->event = NEW_REQUEST;

			ThreadDispatch(radacctinPool, 0, (void*(*)(void*)) sendCiscoRadiusAcctWorker, evtPtr,
								1, PTHREAD_SCOPE_PROCESS, SCHED_FIFO, 59);
		}
		else
		{
			NETERROR(MRADC, ("%s Unable to malloc evtPtr\n", fn));
		}
	}

	return 0;
}


void stopRadiusAcct()
{
	ThreadPoolEnd(radacctinPool);

	closeRadacct_db();
}


void setRadiusAccountingSessionId(CallHandle *callHandle)
{
	static pthread_mutex_t acct_session_id_mutex = PTHREAD_MUTEX_INITIALIZER;

	if(!radius_initialised) return;

	pthread_mutex_lock(&acct_session_id_mutex);
	sprintf(callHandle->acct_session_id, "%x", acct_session_id++);
	pthread_mutex_unlock(&acct_session_id_mutex);
}


static RADIUS_PACKET *constructAccountingRequest(AccountingInfo *data, int id, int sockfd, int ipaddr, int port)
{
	char fn[] = "constructCiscoStartAccounting";
	RADIUS_PACKET *req;
	DICT_ATTR  *da;
	DICT_VALUE *v;
	VALUE_PAIR *vp;
	char buf[512];

	if(ipaddr == INADDR_NONE)
	{
		NETDEBUG(MRADC, NETLOG_DEBUG2, ("%s ipaddr is INADDR_NONE\n", fn));
		return NULL;
	}

	if((req = rad_alloc(1)) == NULL)
	{
		NETERROR(MRADC, ("%s rad_alloc failed\n", fn));
		return NULL;
	}

	req->code = PW_ACCOUNTING_REQUEST;
	req->id = id;
	req->sockfd = sockfd;
	req->dst_ipaddr = ipaddr;
	req->dst_port = port;

	if((vp = pairmake("Called-Station-Id", Val(data->calledStationId), T_OP_EQ)))
	{
		pairadd(&req->vps, vp);
	}
	else
	{
		NETERROR(MRADC, ("%s Failed to find attribute \"Called-Station-Id\" \n", fn));
	}

	if((vp = pairmake("Calling-Station-Id", Val(data->callingStationId), T_OP_EQ)))
	{
		pairadd(&req->vps, vp);
	}
	else
	{
		NETERROR(MRADC, ("%s Failed to find attribute \"Calling-Station-Id\" \n", fn));
	}

	if((vp = pairmake("Acct-Status-Type", Val(data->acctStatusType), T_OP_EQ)))
	{
		pairadd(&req->vps, vp);
	}
	else
	{
		NETERROR(MRADC, ("%s Failed to find attribute \"Acct-Status-Type\" \n", fn));
	}

	if((vp = pairmake("Acct-Delay-Time", Val("0"), T_OP_EQ)))
	{
		pairadd(&req->vps, vp);
	}
	else
	{
		NETERROR(MRADC, ("%s Failed to find attribute \"Acct-Delay-Time\" \n", fn));
	}

	if((vp = pairmake("Acct-Authentic", Val(data->acctAuthentic), T_OP_EQ)))
	{
		pairadd(&req->vps, vp);
	}
	else
	{
		NETERROR(MRADC, ("%s Failed to find attribute \"Acct-Authentic\" \n", fn));
	}

	if(rad_acct_session_id_overloaded)
	{
		snprintf(buf, sizeof(buf), "%s/%s/%s/%s/%s/%s/%s/%s/%s/%s/%s",
					Val(data->acctSessionId),
					Val(data->callStartTime),
					Val(data->gwId),
					Val(data->incomingConfId),
					Val(data->callOrigin),
					Val(data->callType),
					Val(data->callConnectTime),
					Val(data->callEndTime),
					Val(data->cause),
					Val(data->remoteAddr),
					Val(data->confId));

		if((vp = pairmake("Acct-Session-Id", buf, T_OP_EQ)))
		{
			pairadd(&req->vps, vp);
		}
		else
		{
			NETERROR(MRADC, ("%s Failed to find attribute \"Acct-Session-Id\" \n", fn));
		}
	}
	else
	{
		if((vp = pairmake("Acct-Session-Id", Val(data->acctSessionId), T_OP_EQ)))
		{
			pairadd(&req->vps, vp);
		}
		else
		{
			NETERROR(MRADC, ("%s Failed to find attribute \"Acct-Session-Id\" \n", fn));
		}
	}

	if(strlen(Val(data->acctSessionTime)) > 0)
	{
		if((vp = pairmake("Acct-Session-Time", Val(data->acctSessionTime), T_OP_EQ)))
		{
			pairadd(&req->vps, vp);
		}
		else
		{
			NETERROR(MRADC, ("%s Failed to find attribute \"Acct-Session-Time\" \n", fn));
		}
	}

	if((vp = pairmake("Service-Type", Val(data->serviceType), T_OP_EQ)))
	{
		pairadd(&req->vps, vp);
	}
	else
	{
		NETERROR(MRADC, ("%s Failed to find attribute \"Service-Type\" \n", fn));
	}

	if((vp = pairmake("NAS-IP-Address", Val(data->nasIpaddr), T_OP_EQ)))
	{
		pairadd(&req->vps, vp);
	}
	else
	{
		NETERROR(MRADC, ("%s Failed to find attribute \"NAS-IP-Address\" \n", fn));
	}

	if((vp = pairmake("NAS-Port", Val(data->nasPort), T_OP_EQ)))
	{
		pairadd(&req->vps, vp);
	}
	else
	{
		NETERROR(MRADC, ("%s Failed to find attribute \"NAS-Port\" \n", fn));
	}

	if((vp = pairmake("NAS-Port-Type", Val(data->nasPortType), T_OP_EQ)))
	{
		pairadd(&req->vps, vp);
	}
	else
	{
		NETERROR(MRADC, ("%s Failed to find attribute \"NAS-Port-Type\" \n", fn));
	}

	if((vp = pairmake("NAS-Identifier", Val(data->nasIdentifer), T_OP_EQ)))
	{
		pairadd(&req->vps, vp);
	}
	else
	{
		NETERROR(MRADC, ("%s Failed to find attribute \"NAS-Identifier\" \n", fn));
	}

	if((vp = pairmake("Proxy-State", Val(data->proxyState), T_OP_EQ)))
	{
		pairadd(&req->vps, vp);
	}
	else
	{
		NETERROR(MRADC, ("%s Failed to find attribute \"Proxy-State\" \n", fn));
	}

	if((vp = pairmake("Connect-Info", Val(data->connectInfo), T_OP_EQ)))
	{
		pairadd(&req->vps, vp);
	}
	else
	{
		NETERROR(MRADC, ("%s Failed to find attribute \"Connect-Info\" \n", fn));
	}

	if((vp = pairmake("Cisco-NAS-Port", Val(data->ciscoNasPort), T_OP_EQ)))
	{
		pairadd(&req->vps, vp);
	}
	else
	{
		NETERROR(MRADC, ("%s Failed to find attribute \"Cisco-NAS-Port\" \n", fn));
	}

	if(!rad_acct_session_id_overloaded)
	{
		snprintf(buf, sizeof(buf), "h323-incoming-conf-id=%s", Val(data->incomingConfId));

		if((vp = pairmake("Cisco-AVPair", buf, T_OP_EQ)))
		{
			pairadd(&req->vps, vp);
		}
		else
		{
			NETERROR(MRADC, ("%s Failed to find attribute \"Cisco-AVPair (h323-incoming-conf-id)\" \n", fn));
		}

		snprintf(buf, sizeof(buf), "subscriber=%s", Val(data->subscriber));

		if((vp = pairmake("Cisco-AVPair", buf, T_OP_EQ)))
		{
			pairadd(&req->vps, vp);
		}
		else
		{
			NETERROR(MRADC, ("%s Failed to find attribute \"Cisco-AVPair (subscriber)\" \n", fn));
		}

		if(strlen(Val(data->faxTxDuration)) > 0)
		{
			snprintf(buf, sizeof(buf), "fax-tx-duration=%s", Val(data->faxTxDuration));

			if((vp = pairmake("Cisco-AVPair", buf, T_OP_EQ)))
			{
				pairadd(&req->vps, vp);
			}
			else
			{
				NETERROR(MRADC, ("%s Failed to find attribute \"Cisco-AVPair (fax-tx-duration)\" \n", fn));
			}
		}

		snprintf(buf, sizeof(buf), "session-protocol=%s", Val(data->callSessionProtocol));

		if((vp = pairmake("Cisco-AVPair", buf, T_OP_EQ)))
		{
			pairadd(&req->vps, vp);
		}
		else
		{
			NETERROR(MRADC, ("%s Failed to find attribute \"Cisco-AVPair (session-protocol)\" \n", fn));
		}

		snprintf(buf, sizeof(buf), "remote-media-address=%s", Val(data->remoteMediaIpaddr));

		if((vp = pairmake("Cisco-AVPair", buf, T_OP_EQ)))
		{
			pairadd(&req->vps, vp);
		}
		else
		{
			NETERROR(MRADC, ("%s Failed to find attribute \"Cisco-AVPair (remote-media-address)\" \n", fn));
		}

		snprintf(buf, sizeof(buf), "in-trunkgroup-label=%s", Val(data->tg));

		if((vp = pairmake("Cisco-AVPair", buf, T_OP_EQ)))
		{
			pairadd(&req->vps, vp);
		}
		else
		{
			NETERROR(MRADC, ("%s Failed to find attribute \"Cisco-AVPair (in-trunkgroup-label)\" \n", fn));
		}

		snprintf(buf, sizeof(buf), "in-carrier-id=%s", Val(data->tg));

		if((vp = pairmake("Cisco-AVPair", buf, T_OP_EQ)))
		{
			pairadd(&req->vps, vp);
		}
		else
		{
			NETERROR(MRADC, ("%s Failed to find attribute \"Cisco-AVPair (in-carrier-id)\" \n", fn));
		}

		snprintf(buf, sizeof(buf), "gw-rxd-cdn=%s", Val(data->gwRxdCdn));

		if((vp = pairmake("Cisco-AVPair", buf, T_OP_EQ)))
		{
			pairadd(&req->vps, vp);
		}
		else
		{
			NETERROR(MRADC, ("%s Failed to find attribute \"Cisco-AVPair (gw-rxd-cdn)\" \n", fn));
		}

		snprintf(buf, sizeof(buf), "gk-xlated-cdn=%s", Val(data->gkXlatedCdn));

		if((vp = pairmake("Cisco-AVPair", buf, T_OP_EQ)))
		{
			pairadd(&req->vps, vp);
		}
		else
		{
			NETERROR(MRADC, ("%s Failed to find attribute \"Cisco-AVPair (gk-xlated-cdn)\" \n", fn));
		}

		snprintf(buf, sizeof(buf), "gw-final-xlated-cdn=%s" , Val(data->gwFinalXlatedCdn));

		if((vp = pairmake("Cisco-AVPair", buf, T_OP_EQ)))
		{
			pairadd(&req->vps, vp);
		}
		else
		{
			NETERROR(MRADC, ("%s Failed to find attribute \"Cisco-AVPair (gw-final-xlated-cdn)\" \n", fn));
		}

		snprintf(buf, sizeof(buf), "outgoing-area=%s", Val(data->outgoingArea));

		if((vp = pairmake("Cisco-AVPair", buf, T_OP_EQ)))
		{
			pairadd(&req->vps, vp);
		}
		else
		{
			NETERROR(MRADC, ("%s Failed to find attribute \"Cisco-AVPair (outgoing-area)\" \n", fn));
		}

		snprintf(buf, sizeof(buf), "h323-conf-id=%s", Val(data->confId));

		if((vp = pairmake("h323-conf-id", buf, T_OP_EQ)))
		{
			pairadd(&req->vps, vp);
		}
		else
		{
			NETERROR(MRADC, ("%s Failed to find attribute \"h323-conf-id\" \n", fn));
		}

		snprintf(buf, sizeof(buf), "h323-call-origin=%s", Val(data->callOrigin));

		if((vp = pairmake("h323-call-origin", buf, T_OP_EQ)))
		{
			pairadd(&req->vps, vp);
		}
		else
		{
			NETERROR(MRADC, ("%s Failed to find attribute \"h323-call-origin\" \n", fn));
		}

		snprintf(buf, sizeof(buf), "h323-call-type=%s", Val(data->callType));

		if((vp = pairmake("h323-call-type", buf, T_OP_EQ)))
		{
			pairadd(&req->vps, vp);
		}
		else
		{
			NETERROR(MRADC, ("%s Failed to find attribute \"h323-call-type\" \n", fn));
		}

		snprintf(buf, sizeof(buf), "h323-remote-address=%s", Val(data->remoteAddr));

		if((vp = pairmake("h323-remote-address", buf, T_OP_EQ)))
		{
			pairadd(&req->vps, vp);
		}
		else
		{
			NETERROR(MRADC, ("%s Failed to find attribute \"h323-remote-address\" \n", fn));
		}

		snprintf(buf, sizeof(buf), "h323-setup-time=%s", Val(data->callStartTime));

		if((vp = pairmake("h323-setup-time", buf, T_OP_EQ)))
		{
			pairadd(&req->vps, vp);
		}
		else
		{
			NETERROR(MRADC, ("%s Failed to find attribute \"h323-setup-time\" \n", fn));
		}

		if(strcasecmp(Val(data->acctStatusType), "Stop") == 0)
		{
			snprintf(buf, sizeof(buf), "h323-connect-time=%s", Val(data->callConnectTime));

			if((vp = pairmake("h323-connect-time", buf, T_OP_EQ)))
			{
				pairadd(&req->vps, vp);
			}
			else
			{
				NETERROR(MRADC, ("%s Failed to find attribute \"h323-connect-time\" \n", fn));
			}

			snprintf(buf, sizeof(buf), "h323-disconnect-time=%s", Val(data->callEndTime));

			if((vp = pairmake("h323-disconnect-time", buf, T_OP_EQ)))
			{
				pairadd(&req->vps, vp);
			}
			else
			{
				NETERROR(MRADC, ("%s Failed to find attribute \"h323-disconnect-time\" \n", fn));
			}

			snprintf(buf, sizeof(buf), "h323-disconnect-cause=%s", Val(data->cause));
			if((vp = pairmake("h323-disconnect-cause", buf, T_OP_EQ)))
			{
				pairadd(&req->vps, vp);
			}
			else
			{
				NETERROR(MRADC, ("%s Failed to find attribute \"h323-disconnect-cause\" \n", fn));
			}
		}

		snprintf(buf, sizeof(buf), "h323-gw-id=%s", Val(data->gwId));

		if((vp = pairmake("h323-gw-id", buf, T_OP_EQ)))
		{
			pairadd(&req->vps, vp);
		}
		else
		{
			NETERROR(MRADC, ("%s Failed to find attribute \"h323-gw-id\" \n", fn));
		}
	}

	librad_md5_calc(req->vector, req->vector, sizeof(req->vector));

	return req;
}


void *sendCiscoRadiusAcctWorker(void *arg)
{
	char fn[] = "sendCiscoRadiusAcctWorker";
	static int outstanding_requests = 0;
	static unsigned char packet_id = 0;
	RadacctEvent *evtPtr = arg;
	RADIUS_PACKET *req, *rep;
	AccountingInfo *acct_info;
	RadacctListEntry *list_entry;
	time_t now;
	int server;
	int i, j, check_pending = 0;

	if(evtPtr == NULL)
	{
		return NULL;
	}

	switch(evtPtr->event)
	{
		case NEW_REQUEST:
			if(outstanding_requests < max_outstanding_requests)
			{
				if((acct_info = getAccoutingInfo(CURRENT)))
				{
					for(j = 0; j < num_radacct_servers; ++j)
					{
						if(!radacct_server[j].dead)
						{
							break;
						}
					}

					server = j;

					if((req = constructAccountingRequest(acct_info, ++packet_id, sockfd,
									radacct_server[server].ipaddr, radacct_server[server].port)))
					{
						rad_send(req, NULL, secret[radacct_server[server].id]);

						++outstanding_requests;

						list_entry = malloc(sizeof(RadacctListEntry));
						list_entry->acct_info = acct_info;
						list_entry->packet = req;
						list_entry->server = server;
						list_entry->retries = 0;
						list_entry->timestamp = time(0) + rad_timeout;
						listAddItem(radacctList[req->id], list_entry);
					}
					else
					{
						NETERROR(MRADC, ("%s failed to construct account request\n", fn));

						storeAccoutingInfo(BACKLOG, acct_info);
						++backlog_pending_requests;

						removeAccoutingInfo(acct_info->tid);

						freeAccountingInfo(acct_info);
					}
				}
				else
				{
					NETERROR(MRADC, ("%s got null record\n", fn));
				}

				check_pending = 1;
			}
			else
			{
				NETDEBUG(MRADC, NETLOG_DEBUG2, ("%s too many outstanding requests\n", fn));
				++current_pending_requests;
			}
			break;

		case BACKLOG_REQUEST:
			if(outstanding_requests < max_outstanding_requests)
			{
				if((acct_info = getAccoutingInfo(BACKLOG)))
				{
					for(j = 0; j < num_radacct_servers; ++j)
					{
						if(!radacct_server[j].dead)
						{
							break;
						}
					}

					server = j;

					if((req = constructAccountingRequest(acct_info, ++packet_id, sockfd,
									radacct_server[server].ipaddr, radacct_server[server].port)))
					{
						rad_send(req, NULL, secret[radacct_server[server].id]);

						++outstanding_requests;

						list_entry = malloc(sizeof(RadacctListEntry));
						list_entry->acct_info = acct_info;
						list_entry->packet = req;
						list_entry->server = server;
						list_entry->retries = 0;
						list_entry->timestamp = time(0) + rad_timeout;
						listAddItem(radacctList[req->id], list_entry);
					}
					else
					{
						NETERROR(MRADC, ("%s failed to construct account request\n", fn));

						storeAccoutingInfo(BACKLOG, acct_info);
						++backlog_pending_requests;

						removeAccoutingInfo(acct_info->tid);

						freeAccountingInfo(acct_info);
					}
				}
				else
				{
					NETERROR(MRADC, ("%s got null record\n", fn));
				}

				check_pending = 1;
			}
			else
			{
				NETDEBUG(MRADC, NETLOG_DEBUG2, ("%s too many outstanding requests\n", fn));
				++backlog_pending_requests;
			}
			break;

		case RESPONSE:
			if((rep = evtPtr->packet))
			{
				if((list_entry = (RadacctListEntry*)listGetFirstItem(radacctList[rep->id])))
				{
					do
					{
						req = list_entry->packet;
						server = list_entry->server;

						if(rad_decode(rep, req, secret[radacct_server[server].id]) == 0)
						{
							NETDEBUG(MRADC, NETLOG_DEBUG4, ("%s found request\n", fn));

							radacct_server[server].dead = 0;

							radacct_server[server].fail_time = 0;

							--outstanding_requests;

							listDeleteItem(radacctList[rep->id], list_entry);

							rad_free(&req);

							removeAccoutingInfo(list_entry->acct_info->tid);

							freeAccountingInfo(list_entry->acct_info);

							free(list_entry);

							check_pending = 1;

							break;
						}
					}
					while((list_entry = (RadacctListEntry*)listGetNextItem(radacctList[rep->id], list_entry)));
				}

				rad_free(&rep);
			}
			else
			{
				NETERROR(MRADC, ("%s got null packet\n", fn));
			}
			break;

		case TIMEOUT:
			now = time(0);

			for(i = 0; i < num_radacct_servers; ++i)
			{
				if(radacct_server[i].dead)
				{
					--radacct_server[i].dead;
				}
			}

			for(i = 0; i < 256; ++i)
			{
				while((list_entry = (RadacctListEntry*)listGetFirstItem(radacctList[i]))
										&& difftime(list_entry->timestamp, now) <= 0)
				{
					listDeleteItem(radacctList[i], list_entry);

					req = list_entry->packet;
					server = list_entry->server;

					if(++list_entry->retries < rad_retries)
					{
						rad_send(req, NULL, secret[radacct_server[server].id]);

						list_entry->timestamp = now + rad_timeout;
						listAddItem(radacctList[i], list_entry);
					}
					else if((server + 1) < num_radacct_servers)
					{
						NETDEBUG(MRADC, NETLOG_DEBUG2, ("%s No response from server: %s\n",
											fn, rad_server_addr[radacct_server[server].id]));

						if(rad_deadtime)
						{
							if(radacct_server[server].fail_time == 0)
							{
								radacct_server[server].fail_time = now + rad_deadtime;
							}
							else if(difftime(radacct_server[server].fail_time, now) <= 0)
							{
								radacct_server[server].dead = rad_deadtime;
							}
						}

						for(j = server + 1; j < num_radacct_servers; ++j)
						{
							if(!radacct_server[j].dead)
							{
								break;
							}
						}

						server = j;

						rad_free(&req);

						if((req = constructAccountingRequest(list_entry->acct_info, i, sockfd,
										radacct_server[server].ipaddr, radacct_server[server].port)))
						{

							rad_send(req, NULL, secret[server]);

							list_entry->packet = req;
							list_entry->server = server;
							list_entry->retries = 0;
							list_entry->timestamp = now + rad_timeout;
							listAddItem(radacctList[i], list_entry);
						}
						else
						{
							NETERROR(MRADC, ("%s failed to construct account request\n", fn));

							--outstanding_requests;

							storeAccoutingInfo(BACKLOG, list_entry->acct_info);
							++backlog_pending_requests;

							removeAccoutingInfo(list_entry->acct_info->tid);

							freeAccountingInfo(list_entry->acct_info);

							free(list_entry);

							check_pending = 1;
						}
					}
					else
					{
						NETDEBUG(MRADC, NETLOG_DEBUG2, ("%s No response from server: %s\n",
											fn, rad_server_addr[radacct_server[server].id]));

						--outstanding_requests;

						storeAccoutingInfo(BACKLOG, list_entry->acct_info);
						++backlog_pending_requests;

						removeAccoutingInfo(list_entry->acct_info->tid);

						freeAccountingInfo(list_entry->acct_info);

						rad_free(&list_entry->packet);

						free(list_entry);

						check_pending = 1;
					}
				}
			}
			break;

		default:
			break;
	}

	if(check_pending)
	{
		if(current_pending_requests > 0)
		{
			--current_pending_requests;
			evtPtr->event = NEW_REQUEST;

			ThreadDispatch(radacctinPool, 0, (void*(*)(void*)) sendCiscoRadiusAcctWorker, evtPtr,
								1, PTHREAD_SCOPE_PROCESS, SCHED_FIFO, 59);
		}
		else if(backlog_pending_requests > 0 && num_radacct_servers > 0)
		{
			--backlog_pending_requests;
			evtPtr->event = BACKLOG_REQUEST;

			ThreadDispatch(radacctinPool, 0, (void*(*)(void*)) sendCiscoRadiusAcctWorker, evtPtr,
								1, PTHREAD_SCOPE_PROCESS, SCHED_FIFO, 59);
		}
		else
		{
			free(evtPtr);
		}
	}
	else
	{
		free(evtPtr);
	}
	return(NULL);
}


void *recvCiscoRadiusAcctResponses(void *arg)
{
	char fn[] = "recvCiscoRadiusAcctResponses";
	RADIUS_PACKET *rep;
	RadacctEvent *evtPtr;

	for(;;)
	{
		rep = rad_recv(sockfd);

		if((evtPtr = malloc(sizeof(RadacctEvent))))
		{
			memset(evtPtr, 0, sizeof(RadacctEvent));
			evtPtr->event = RESPONSE;
			evtPtr->packet = rep;

			ThreadDispatch(radacctinPool, 0, (void*(*)(void*)) sendCiscoRadiusAcctWorker, evtPtr,
								1, PTHREAD_SCOPE_PROCESS, SCHED_FIFO, 59);
		}
		else
		{
			NETERROR(MRADC, ("%s Unable to malloc evtPtr\n", fn));
			rad_free(&rep);
		}
	}
}


int radAcctTimer(struct Timer *t)
{
	char fn[] = "radiusAcctTimer";
	RadacctEvent *evtPtr;
	struct  itimerval tmr;

	if((evtPtr = malloc(sizeof(RadacctEvent))))
	{
		memset(evtPtr, 0, sizeof(RadacctEvent));
		evtPtr->event = TIMEOUT;

		ThreadDispatch(radacctinPool, 0, (void*(*)(void*)) sendCiscoRadiusAcctWorker, evtPtr,
								1, PTHREAD_SCOPE_PROCESS, SCHED_FIFO, 59);
	}
	else
	{
		NETERROR(MRADC, ("%s Unable to malloc evtPtr\n", fn));
	}

	return 1;
}


int sendCiscoRadiusAccounting(CallHandle *callHandle, int flag)
{
	char fn[] = "sendCiscoRadiusAccounting";
	AccountingInfo *acct_info;
	RadacctEvent *evtPtr;

	if(!radius_initialised) return -1;

	if((acct_info = createAccountingInfo(callHandle, flag)))
	{
		storeAccoutingInfo(CURRENT, acct_info);

		if((evtPtr = malloc(sizeof(RadacctEvent))))
		{
			memset(evtPtr, 0, sizeof(RadacctEvent));

			evtPtr->event = NEW_REQUEST;

			ThreadDispatch(radacctinPool, 0, (void*(*)(void*)) sendCiscoRadiusAcctWorker, evtPtr,
								1, PTHREAD_SCOPE_PROCESS, SCHED_FIFO, 59);
		}
		else
		{
			NETERROR(MRADC, ("%s Unable to malloc evtPtr\n", fn));
		}

		freeAccountingInfo(acct_info);

		return 0;
	}
	else
	{
		NETERROR(MSIP, ("%s sendCiscoRadiusAccounting failed to create accounting info\n", fn));
		return -1;
	}
}


static AccountingInfo *createAccountingInfo(CallHandle *callHandle, int flag)
{
	AccountingInfo *data;
	PhoNode *local, *remote;
	timedef callStartTime, callConnectTime, callEndTime, DurTime;
	int duration;
	char buf[256], *rptr, mstr[5];
	struct tm res;

	if(callHandle->callSource == 0)
	{
		local = &callHandle->phonode;
		remote = &callHandle->rfphonode;
	}
	else
	{
		local = &callHandle->rfphonode;
		remote = &callHandle->phonode;
	}

	if(!timedef_iszero(&callHandle->callStartTime))
	{
		callStartTime = callHandle->callStartTime;
	}
	else
	{
		// Start time can never be zero
		timedef_cur(&callStartTime);
	}

	if(!timedef_iszero(&callHandle->callEndTime))
	{
		callEndTime = callHandle->callEndTime;
	}
	else
	{
		// End time can never be zero
		timedef_cur(&callEndTime);
	}

	if(!timedef_iszero(&callHandle->callConnectTime))
	{
		callConnectTime = callHandle->callConnectTime;

		if (timedef_cmp(&callEndTime, &callConnectTime) < 0)
		{
			callConnectTime = callEndTime;
		}
	}
	else
	{
		// Connect time can never be zero
		callConnectTime = callEndTime;
	}

	timedef_sub(&callEndTime, &callConnectTime, &DurTime);
	duration = timedef_rndsec(&DurTime);

	if((data = malloc(sizeof(AccountingInfo))))
	{
		data->acctAuthentic = strdup("Local");

		data->acctSessionId = strdup(Val(callHandle->acct_session_id));

		data->acctStatusType = strdup(flag == CDR_CALLSETUP ? "Start" : "Stop");

		strftime(buf, sizeof(buf), "%T.000 %Z %a %b %d %Y", localtime_r(&timedef_sec(&callStartTime), &res));
		if (rptr = strstr(buf, ".000")) {
			if (snprintf(mstr, 5, ".%3.3ld", timedef_msec(&callStartTime)) >= 0) {
				bcopy(mstr, rptr, 4);
			}
		}
		data->callStartTime = strdup(buf);

		if(flag != CDR_CALLSETUP)
		{
			snprintf(buf, sizeof(buf), "%d", duration);
			data->acctSessionTime = strdup(buf);

			if(callHandle->flags & FL_CALL_FAX)
			{
				snprintf(buf, sizeof(buf), "%ld", (timedef_sec(&DurTime)*1000 + timedef_msec(&DurTime)));
				data->faxTxDuration = strdup(buf);
			}
			else
			{
				data->faxTxDuration = strdup("");
			}

			strftime(buf, sizeof(buf), "%T.000 %Z %a %b %d %Y", localtime_r(&timedef_sec(&callConnectTime), &res));
			if (rptr = strstr(buf, ".000")) {
				if (snprintf(mstr, 5, ".%3.3ld", timedef_msec(&callConnectTime)) >= 0) {
					bcopy(mstr, rptr, 4);
				}
			}
			data->callConnectTime = strdup(buf);

			strftime(buf, sizeof(buf), "%T.000 %Z %a %b %d %Y", localtime_r(&timedef_sec(&callEndTime), &res));
			if (rptr = strstr(buf, ".000")) {
				if (snprintf(mstr, 5, ".%3.3ld", timedef_msec(&callEndTime)) >= 0) {
					bcopy(mstr, rptr, 4);
				}
			}
			data->callEndTime = strdup(buf);

			callHandle->cause = (callHandle->handleType == SCC_eH323CallHandle) ? callHandle->cause : iwfConvertSipCodeToCause(callHandle->responseCode);
			sprintf(buf, "%x", callHandle->cause ? callHandle->cause - 1 : 16);
			data->cause = strdup(buf);

		}
		else
		{
			data->acctSessionTime = strdup("");
			data->callConnectTime = strdup("");
			data->callEndTime = strdup("");
			data->cause = strdup("");
			data->faxTxDuration = strdup("");
		}

		data->callOrigin = strdup(callHandle->callSource ? "originate" : "answer");

		data->callSessionProtocol = strdup(callHandle->handleType == SCC_eH323CallHandle ? "H.323" : "SIP");

		data->callType = strdup("VoIP");

		data->calledStationId = strdup(Val(callHandle->inputNumber));

		data->callingStationId = strdup(Val(local->phone));

		data->ciscoNasPort = strdup("");

		data->confId = strdup(Val(callHandle->conf_id));

		data->connectInfo = strdup("");

		data->gwRxdCdn = strdup(Val(callHandle->inputNumber));

		data->gkXlatedCdn = strdup(Val(callHandle->dialledNumber));

		data->gwFinalXlatedCdn = strdup(Val(remote->phone));

		data->gwId = strdup(callHandle->callSource == 0 ? Val(local->regid) : Val(remote->regid));

		data->incomingConfId = strdup(Val(callHandle->incoming_conf_id));

		data->nasIdentifer = strdup(Val(sipservername));

		data->nasIpaddr = strdup(nas_ipaddr);

		data->nasPort = strdup("0");

		data->nasPortType = strdup("Async");

		data->outgoingArea = strdup(callHandle->callSource == 0 ? "" : Val(remote->regid));

		data->proxyState = strdup("");

		FormatIpAddress(callHandle->callSource == 0 ? local->ipaddress.l : callHandle->peerIp, buf);
		data->remoteAddr = strdup(buf);

		FormatIpAddress(callHandle->lastMediaIp, buf);
		data->remoteMediaIpaddr = strdup(buf);

		data->serviceType = strdup("Login");

		data->subscriber = strdup("Subscriber");

		data->tg = strdup(Val(callHandle->tg));
	}

	return data;
}


static void freeAccountingInfo(AccountingInfo *data)
{
	#define CheckFree(p) if(p) free(p);

	if(data)
	{
		CheckFree(data->acctAuthentic);
		CheckFree(data->acctSessionId);
		CheckFree(data->acctSessionTime);
		CheckFree(data->acctStatusType);
		CheckFree(data->callConnectTime);
		CheckFree(data->callEndTime);
		CheckFree(data->callOrigin);
		CheckFree(data->callSessionProtocol);
		CheckFree(data->callStartTime);
		CheckFree(data->callType);
		CheckFree(data->calledStationId);
		CheckFree(data->callingStationId);
		CheckFree(data->cause);
		CheckFree(data->ciscoNasPort);
		CheckFree(data->confId);
		CheckFree(data->connectInfo);
		CheckFree(data->faxTxDuration);
		CheckFree(data->gkXlatedCdn);
		CheckFree(data->gwFinalXlatedCdn);
		CheckFree(data->gwId);
		CheckFree(data->gwRxdCdn);
		CheckFree(data->incomingConfId);
		CheckFree(data->nasIdentifer);
		CheckFree(data->nasIpaddr);
		CheckFree(data->nasPort);
		CheckFree(data->nasPortType);
		CheckFree(data->outgoingArea);
		CheckFree(data->proxyState);
		CheckFree(data->remoteAddr);
		CheckFree(data->remoteMediaIpaddr);
		CheckFree(data->serviceType);
		CheckFree(data->subscriber);
		CheckFree(data->tg);

		free(data);
	}
}

