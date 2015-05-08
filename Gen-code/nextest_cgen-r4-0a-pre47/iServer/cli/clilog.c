#include <stdlib.h>
#include <time.h>
#include "cli.h"
#include "serverp.h"
#include <malloc.h>
#include "ipstring.h"


int
lsAlarmFn(time_t* valarms, time_t* mrvalarms)
{
  int index = 0;
  char printBuf[64] = {0};

  printf("VPort Alarms:\n");
  for(index=0; index < MAX_LS_ALARM; index++){
    if(valarms[index] >0){
      printf("%s\n",ctime_r(&valarms[index], printBuf));
    }
  }


  printf("MR VPort Alarms:\n");
  for(index=0; index < MAX_LS_ALARM; index++){
    if(mrvalarms[index] >0){
      printf("%s\n",ctime_r(&mrvalarms[index], printBuf));
    }
  }

  return 0;
}

int
CliRouteLogFn(RouteNode *routeNode)
{
	int type;

	switch (routeNode->branch)
	{
		case 1:
			printf("\t(b2): ");
			break;
		case 0:
			printf("\t(b1): ");
			break;
		default:
			break;
	}

	if (BIT_TEST(routeNode->xphonode.sflags, ISSET_REGID))
	{
		printf("%s/%lu ", 
			routeNode->xphonode.regid,
			routeNode->xphonode.uport);
	}

	if (BIT_TEST(routeNode->xphonode.sflags, ISSET_IPADDRESS))
	{
		printf("%s ", 
			ULIPtostring(routeNode->xphonode.ipaddress.l)); 
	} 

	if (BIT_TEST(routeNode->xphonode.sflags, ISSET_PHONE))
	{
		printf("%s ", 
			routeNode->xphonode.phone);
	}

	printf("\n");

	if (routeNode->crname)
	{
		printf("\troute: %s%s", routeNode->crname, routeNode->crflags&CRF_STICKY?" sticky":"");
		if (routeNode->cpname)
		{
			printf("plan: %s ", routeNode->cpname);
		}
		printf("\n");
	}

	if ((routeNode->forwardReason > 0) &&
			(routeNode->forwardReason < nextoneReasonMax))
	{
		printf("\tpreferred reason = %s ", 
			NextoneReasonNames[routeNode->forwardReason]);
	}

	if (!routeNode->rejectReason && !routeNode->forwardReason)
	{
		printf("\tfinal ");
	}

	if ((routeNode->rejectReason > 0) &&
			(routeNode->rejectReason < nextoneReasonMax))
	{
		switch (routeNode->rejectReason)
		{
		default:
			printf("\trejected reason = %s ", 
				NextoneReasonNames[routeNode->rejectReason]);
			break;
		}
	}

	printf("-> ");

	if (BIT_TEST(routeNode->yphonode.sflags, ISSET_REGID))
	{
		printf("%s/%lu ", 
			routeNode->yphonode.regid,
			routeNode->yphonode.uport);
	}

	if (BIT_TEST(routeNode->yphonode.sflags, ISSET_IPADDRESS))
	{
		printf("%s ", 
			ULIPtostring(routeNode->yphonode.ipaddress.l)); 
	} 

	if (!routeNode->rejectReason &&
		BIT_TEST(routeNode->yphonode.sflags, ISSET_PHONE))
	{
		printf("DIAL phone:%s ", 
			routeNode->yphonode.phone);
	}

	printf("\n");

	printf("\n");
	return(0);
}

