#include <unistd.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include "ipc.h"
#include "bits.h"
#include "serverdb.h"
#include "phonode.h"
#include "ipstring.h"
#include <malloc.h>

#define PRINTF(X)	printf X

PhoNode *
PhoNodeDup(PhoNode *phonode)
{
	PhoNode *dup;
	
	if (phonode == 0)
	{
		/* A dup of null is a null */
		return 0;
	}

	dup = (PhoNode *)malloc(sizeof(PhoNode));
	memcpy(dup, phonode, sizeof(PhoNode));
	return (dup);
}

PhoNode *
NewPhoNode(void)
{
	PhoNode *dup;
	
	dup = (PhoNode *)malloc(sizeof(PhoNode));
	memset(dup, 0, sizeof(PhoNode));

	return (dup);
}

int
PhoNodeCmp(PhoNode *n1, PhoNode *n2)
{
	if (BIT_TEST(n1->sflags, ISSET_REGID) &&
		BIT_TEST(n2->sflags, ISSET_REGID))
	{
		if (memcmp(n1->regid, n2->regid, 
			REG_ID_LEN) != 0)
		{
			return 1;
		}
	}

	if (BIT_TEST(n1->sflags, ISSET_UPORT) &&
		BIT_TEST(n2->sflags, ISSET_UPORT))
	{
		if (n1->uport != n2->uport)
		{
			return 1;
		}
	}
	
	if (BIT_TEST(n1->sflags, ISSET_IPADDRESS) &&
		BIT_TEST(n2->sflags, ISSET_IPADDRESS))
	{
		if ((n1->ipaddress.l != n2->ipaddress.l) ||
			(n1->realmId != n2->realmId) )
		{
			return 1;
		}
	}
	
	if (BIT_TEST(n1->sflags, ISSET_PHONE) &&
		BIT_TEST(n2->sflags, ISSET_PHONE))
	{
		if (strcmp(n1->phone, n2->phone) != 0)
		{
			return 1;
		}
	}

	if (BIT_TEST(n1->sflags, ISSET_VPNPHONE) &&
		BIT_TEST(n2->sflags, ISSET_VPNPHONE))
	{
		if (strcmp(n1->vpnPhone, n2->vpnPhone) != 0)
		{
			return 1;
		}
	}

	return 0;
}

int
Phonode2NetInfo(PhoNode *phonodep, InfoEntry *info)
{
	memcpy(info->regid, phonodep->regid, REG_ID_LEN);
	info->uport = phonodep->uport;
	info->ipaddress.l = phonodep->ipaddress.l;
	info->realmId = phonodep->realmId;
	memcpy(info->phone, phonodep->phone, PHONE_NUM_LEN);
	memcpy(info->vpnPhone, phonodep->vpnPhone, VPN_LEN);
	info->vpnExtLen = phonodep->vpnExtLen;
	info->sflags = phonodep->sflags;
	info->cap = phonodep->cap;

	return 0;
}

int
htonPhonode(PhoNode *phonodep)
{
	phonodep->uport = htonl(phonodep->uport);
	phonodep->ipaddress.l = htonl(phonodep->ipaddress.l);
	phonodep->vpnExtLen = htonl(phonodep->vpnExtLen);
	phonodep->clientState = htonl(phonodep->clientState);
	return(0);
}

int
ntohPhonode(PhoNode *phonodep)
{
	phonodep->uport = ntohl(phonodep->uport);
	phonodep->ipaddress.l = ntohl(phonodep->ipaddress.l);
	phonodep->vpnExtLen = ntohl(phonodep->vpnExtLen);
	phonodep->clientState = ntohl(phonodep->clientState);
	return(0);
}

int
Phonode2PortStatus(PhoNode *phonodep, PortStatusData *portStatus)
{
	portStatus->uport = phonodep->uport;
    strncpy(portStatus->phone, phonodep->phone, PHONE_NUM_LEN);
    strncpy(portStatus->vpnPhone, phonodep->vpnPhone, PHONE_NUM_LEN);
    portStatus->vpnExtLen = phonodep->vpnExtLen;
    portStatus->cap = phonodep->cap;
    portStatus->sflags = phonodep->sflags;
    portStatus->clientState = phonodep->clientState;
	return(0);
}

int
PortStatus2Phonode(PortStatusData *portStatus, PhoNode *phonodep)
{
	phonodep->uport = portStatus->uport;
    strncpy(phonodep->phone, portStatus->phone, PHONE_NUM_LEN);
    strncpy(phonodep->vpnPhone, portStatus->vpnPhone, PHONE_NUM_LEN);
    phonodep->vpnExtLen = portStatus->vpnExtLen;
    phonodep->cap = portStatus->cap;
    phonodep->sflags = portStatus->sflags;
    phonodep->clientState = portStatus->clientState;
	return(0);
}

void
htonPortStatus(PortStatusData *portStatus)
{
	portStatus->uport = htonl(portStatus->uport);
	portStatus->vpnExtLen = htonl(portStatus->vpnExtLen);
	portStatus->clientState = htonl(portStatus->clientState);
	portStatus->reason = htonl(portStatus->reason);
}

void
ntohPortStatus(PortStatusData *portStatus)
{
	portStatus->uport = ntohl(portStatus->uport);
	portStatus->vpnExtLen = ntohl(portStatus->vpnExtLen);
	portStatus->clientState = ntohl(portStatus->clientState);
	portStatus->reason = ntohl(portStatus->reason);
}

int
InitPhonodeFromInfoEntry(InfoEntry *info, PhoNode *phonode)
{
	memset(phonode, 0, sizeof(PhoNode));
	memcpy(phonode->regid, info->regid, REG_ID_LEN);
	phonode->uport = info->uport;
	if (strlen(phonode->regid))
	{
		BIT_SET(phonode->sflags, ISSET_REGID);
		BIT_SET(phonode->sflags, ISSET_UPORT);
	}

	phonode->ipaddress.l = info->ipaddress.l;
	phonode->realmId = info->realmId;
	memcpy(phonode->phone, info->phone, PHONE_NUM_LEN);
	memcpy(phonode->vpnPhone, info->vpnPhone, VPN_LEN);
	phonode->vpnExtLen = info->vpnExtLen;

	BIT_COPY(phonode->sflags, ISSET_IPADDRESS, info->sflags, ISSET_IPADDRESS);
	BIT_COPY(phonode->sflags, ISSET_PHONE, info->sflags, ISSET_PHONE);
	BIT_COPY(phonode->sflags, ISSET_VPNPHONE, info->sflags, ISSET_VPNPHONE);

	phonode->cap = info->cap;
	return(0);
}

int
PrintPhoNode(PhoNode *node)
{
	PRINTF(("PrintPhoNode begin\n"));
	if (BIT_TEST(node->sflags, ISSET_REGID))
	{
		PRINTF(("Registration ID %s\n", node->regid));
	}

	if (BIT_TEST(node->sflags, ISSET_UPORT))
	{
		PRINTF (("Uport %lu\n", node->uport));
	}
	
	if (BIT_TEST(node->sflags, ISSET_IPADDRESS))
	{
		PRINTF (("IPADDRESS %s\n", (char*) ULIPtostring((node->ipaddress.l))));
	}
	
	if (BIT_TEST(node->sflags, ISSET_PHONE))
	{
		PRINTF( ("Phone %s\n", node->phone));
	}

	if (BIT_TEST(node->sflags, ISSET_VPNPHONE))
	{
		PRINTF( ("Vpn Phone %s %lu\n", node->vpnPhone, node->vpnExtLen));
	}
	PRINTF( ("PrintPhoNode end\n"));	
	return(0);
}

