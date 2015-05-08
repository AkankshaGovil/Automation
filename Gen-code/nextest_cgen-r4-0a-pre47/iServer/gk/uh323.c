#include "cm.h"
#include "seli.h"
#include "gis.h"
#include "uh323.h"
#include "uh323cb.h"
#include "arq.h"
#include "codecs.h"
#include "lsprocess.h"
#include "h323realm.h"
#include "nxosd.h"
#include <malloc.h>
#include "ipcutils.h"
#include "ifs.h"
#include "log.h"
#include "callid.h"
#include "gk.h"
#include "uh323proto.h"
#include "msg.h"

/* static fns */
static void UH323InstanceInitRealm(int instance);

int UH323AllocatePorts(void);
int GkPoll(struct Timer *t);
int iServerUpdateH323Allocations(struct Timer* t);
struct  itimerval rastmr;
tid rastid;
HPST	hPstSetup = NULL; 
extern unsigned char * cmEmGetQ931Syntax();

int msAdd(char *);
int msDelete(char *);
int msDeleteAll();
int msSetDebugLevel();

// Allocated in main/temp instance
int PINodeId;

HPST hSynAlerting; 
HPST hSynBearerCap; 
HPST hSynVideo; 
HPST hSynData; 

// Useful information, when one instance needs to know
// about the other
UH323_tGlobals *uh323Globals;

pthread_key_t UH323GlobalsKey;

char h323valfiles[MAX_H323INSTANCES][256];
int sgkInstance = 0, arqInstance = 0;

char h323tmpvalfile[256] = "h323cfg-gktmp.val";
TimerPrivate *h323timerPrivate;
int h323maxCallsPerIngress = 0, h323maxCallsPerEgress = 0;
int h323dynamicPorts = 1721;
struct h323Ports_tag{
	int mPort;		/* RAS multicastAddress port */
	int rasPort;	/* RAS port */
	int q931Port;	/* Q.931 call signaling port */
}*h323PortArray = NULL;

int
UH323SetupValFiles()
{
	int i, j = 0, k = 0;

	// copy the main file into a template file
	// for backward compatibility, we will not introduce
	// a new template file, but use the distributed
	// val file instead.
	// system("cp h323cfg-gk.val h323cfg-gktemplate.val");

	if (nh323Instances == 1)
	{
		strcpy(h323valfiles[0], "h323cfg-gk.val");

		return 0;
	}

	for (i=0; i< SGK_MINSTANCE_ID; i++)
	{
		sprintf(h323valfiles[i], "h323cfg-gk%c%d-%d.val", 
				i%2?'o':'i', i/2+1, SGK_MINSTANCE_ID);
	}	

	// Setup val files for the ARQ and SGK instances
	strcpy(h323valfiles[SGK_MINSTANCE_ID], "h323cfg-gksgk.val");
	strcpy(h323valfiles[ARQ_MINSTANCE_ID], "h323cfg-gkarq.val");
	strcpy(h323valfiles[TMP_MINSTANCE_ID], "h323cfg-gktmp.val");

	return 0;
}

int
UH323ReadConfigs(char *config_file, int instance)
{
	HCFG	hCfg = NULL;

	hCfg = ciConstruct(config_file);

	if (hCfg == NULL)
	{
		NETERROR(MH323, ("Config file %s not found for instance %d\n",
			config_file, instance));	

		return -1;
	}

	// Get maximum concurrent calls value - maxCalls
	ciGetValue( hCfg, "system.maxCalls" , NULL, &uh323Globals[instance].maxCalls);

	ciDestruct(hCfg);

	return 0;
}

// config_file is the template file
int
UH323SetupConfigs(char *config_file, int instance)
{
	HCFG	hCfg = NULL;
	int 	mPort = 1918, rasPort = 1919, q931Port = 1720;

	hCfg = ciConstructEx(config_file, 256, 256);

	if (hCfg == NULL)
	{
		NETERROR(MH323, ("Config file %s not found for instance %d\n",
			config_file, instance));	

		return -1;
	}

	// SETUP values
	if (instance%2)
	{
		// Egress
		uh323Globals[instance].maxCalls = h323maxCallsPerEgress;
	}
	else
	{
		// Ingress
		uh323Globals[instance].maxCalls = h323maxCallsPerIngress;
	}

	// ARQ Instance
	if ((nh323Instances > 1) && (instance == ARQ_MINSTANCE_ID))
	{
		uh323Globals[instance].maxCalls = 150;

		// ARQ instance always uses 1723
		q931Port = 1723;

		// RasOut and RasIn should be set to 2.4*concurrent calls
		// That means our factor is 2.4(maxCalls/2) = 1.2 
		if (ciSetValue(hCfg, "system.maxRasOutTransactions", 0, 
			1.2*h323maxCalls, NULL) < 0)
		{
			NETERROR(MINIT, ("ciSetValue failed for system.maxRasOutTransactions"));
		}

		if (ciSetValue(hCfg, "system.maxRasInTransactions", 0, 
			1.2*h323maxCalls, NULL) < 0)
		{
			NETERROR(MINIT, ("ciSetValue failed for system.maxRasInTransactions"));
		}

		if (ciSetValue(hCfg, "system.allocations.maxBuffSize", 0, h323RasMaxBuffSize, NULL) < 0)
		{
			NETERROR(MINIT, ("ciSetValue failed for system.allocations.maxBuffSize"));
		}

#if 0
		if (ciSetValue(hCfg, "system.maxChannels", 0, 
			0, NULL) < 0)
		{
			NETERROR(MINIT, ("ciSetValue failed for system.maxRasInTransactions"));
		}
#endif
	}
	else if ((nh323Instances > 1) && (instance != SGK_MINSTANCE_ID))
	{
		// multi instance non ARQ instance
		if (ciSetValue(hCfg, "RAS.maxRetries", 0, 
			1, NULL) < 0)
		{
			NETERROR(MINIT, ("ciSetValue failed for system.maxRasInTransactions"));
		}
	}

	// SGK instance
	if ((nh323Instances > 1) && (instance == SGK_MINSTANCE_ID))
	{
		uh323Globals[instance].maxCalls = h323maxCallsSgk;
	}

	// Temp instance
	if ((nh323Instances > 1) && (instance == TMP_MINSTANCE_ID))
	{
		uh323Globals[instance].maxCalls = 100;

        	if (ciSetValue(hCfg, "system.allocations.vtNodeCount", 
			0,10000+1200*h323maxCallsPerEgress, NULL) < 0)
			// stack's formula = vtNodeCount = maxCalls * (600 + (maxChannels) * 75) + 900 
			// over here we are copying from 2 instances so just have extra buffer
        	{
            		NETERROR(MINIT, 
				("ciSetValue failed for system.maxCalls"));
        	}

		if (ciSetValue(hCfg, "RAS.responseTimeOut", 0, 
			0, NULL) < 0)
		{
			NETERROR(MINIT, ("ciSetValue failed for system.maxRasInTransactions"));
		}

#if 0
		if (ciSetValue(hCfg, "system.callPropertyMode.doNotUseProperty", 0, 
			0, NULL) < 0)
		{
			NETERROR(MINIT, ("ciSetValue failed for system.maxRasInTransactions"));
		}
#endif

#if 0
		if (ciSetValue(hCfg, "system.maxChannels", 0, 
			0, NULL) < 0)
		{
			NETERROR(MINIT, ("ciSetValue failed for system.maxRasInTransactions"));
		}
#endif
	}
#if 0
	if ((nh323Instances > 1) && (instance != ARQ_MINSTANCE_ID))
	{
		mPort = h323dynamicPorts++;
		rasPort = h323dynamicPorts++;
		q931Port = h323dynamicPorts++;

		// First Ingress instance always uses 1720
		if (instance == 0)
		{
			q931Port = 1720;
		}
	}
#else
	if ( h323PortArray != NULL )
	{
		// The stack ras listeners should be relegated to non standard ports
		mPort = h323PortArray[instance].mPort +200;
		rasPort = h323PortArray[instance].rasPort +200;
		q931Port = h323PortArray[instance].q931Port;
	}
#endif
	// SET in CONF
	if (ciSetValue(hCfg, "system.maxCalls", 0, uh323Globals[instance].maxCalls, NULL) < 0)
	{
		NETERROR(MINIT, ("ciSetValue failed for system.maxCalls"));
	}

	if (ciSetValue(hCfg, "Q931.maxCalls", 0, uh323Globals[instance].maxCalls, NULL) < 0)
	{
		NETERROR(MINIT, ("ciSetValue failed for Q931.maxCalls"));
	}

	// change ports	
	if (ciSetValue(hCfg, "RAS.rasMulticastAddress.ipAddress.port", 0, 
		mPort, NULL) < 0)
	{
		NETERROR(MINIT, ("ciSetValue failed for RAS.rasMulticastAddress.ipAddress.port"));
	}

	if (ciSetValue(hCfg, "RAS.rasPort", 0, rasPort, NULL) < 0)
	{
		NETERROR(MINIT, ("ciSetValue failed for RAS.rasPort"));
	}

	if (ciSetValue(hCfg, "Q931.callSignalingPort", 0, q931Port, NULL) < 0)
	{
		NETERROR(MINIT, ("ciSetValue failed for Q931.callSignalingPort"));
	}

	if(!localProceeding)
	{
		if (ciSetValue(hCfg, "Q931.manualCallProceeding", 0, 0, NULL) < 0)
		{
			NETERROR(MINIT, ("ciSetValue failed for Q931.manualCallProceeding"));
		}
	}

	if(h245Tunneling)
	{
		if (ciSetValue(hCfg, "Q931.h245Tunneling", 0, 0, NULL) < 0)
		{
			NETERROR(MINIT, ("ciSetValue failed for Q931.h245Tunneling"));
		}
	}

	if (ciSave(hCfg, h323valfiles[instance]) < 0)
	{
		NETERROR(MINIT, ("ciSave failed for instance %d, file %s\n",
			instance, h323valfiles[instance]));

		goto _error;
	}

	ciDestruct(hCfg);

	return 1;

_error:
	return -1;
}

// main instance
int
UH323Init(void)
{
	char  	ipStr[80];
	int 	rc;
	HAPP 	hApp;
	HCFG	hCfg;

	/* If H.323 is enabled, initialize it */
	if (!H323Enabled())
	{
		NETDEBUG(MH323, NETLOG_DEBUG3, ("H.323 is disabled\n"));
		return -1;
	}

	uh323Globals = (UH323_tGlobals *)SHM_Malloc(MAX_H323INSTANCES*
			sizeof(UH323_tGlobals));
	memset(uh323Globals, 0, sizeof(UH323_tGlobals));

	NETDEBUG(MH323, NETLOG_DEBUG3, ("H.323 is enabled\n"));

	if ( UH323AllocatePorts() < 0 )
	{
		return -1;
	}

	UH323SetupValFiles();

	h323maxCallsPerIngress = h323maxCalls;

	if (nh323Instances > 1)
	{
		sgkInstance = SGK_MINSTANCE_ID;
		arqInstance = ARQ_MINSTANCE_ID;

		seliSetMaxTasks(nh323Instances);
		h323maxCallsPerIngress = h323maxCalls/(ARQ_MINSTANCE_ID/2);
		h323maxCallsPerEgress = h323maxCalls/(SGK_MINSTANCE_ID/2);

		// To make it symmetric with single instance, we
		// will support only half of the calls on each side
		h323maxCallsPerIngress /= 2;
		h323maxCallsPerEgress /= 2;
		
		cmStartUp();
	}

	initMsdSeed();

	NETDEBUG(MH323, NETLOG_DEBUG3,
		("Initializing H.323 stack version:: %s \n", cmGetVersion()));

	NETDEBUG(MH323, NETLOG_DEBUG3,
		("Max Calls Per Ingress Instance =%d, Per Egress Instance = %d\n",
		h323maxCallsPerIngress, h323maxCallsPerEgress));

	UH323GlobalsInitKey();


	if (UH323SetupConfigs("h323cfg-gktemplate.val", 0) < 0)
	{
		NETERROR(MINIT, ("Config Initialization failed for instance 0\n"));
	}

	/* Initialize the stack instances */
	if (cmInitialize(h323valfiles[0], &hApp) < 0)
	{
		NETERROR(MH323, ("H.323 Initialization failed for %s\n", h323valfiles[0]));
		return -1;
	}
	
	UH323ReadConfigs(h323valfiles[0], 0);
	UH323GlobalsInit(hApp, 0);
 	UH323InstanceInitRealm(0);

	UH323AllocVarsKeys();
	UH323InitVarsKeys();
	uh323InitVars();

	uh235Init();

	if (nh323Instances == 1)
	{
		if(uh323InitPI()<0)
		{
			NETERROR(MH323, ("uh323InitPI() failed \n"));
		}
	}

	/* Specify where to send the H.323 stack logging output */
	/* Possible choices are "terminal", "logger", "file" or "file filename" */
	msSinkAdd("file");	

	return 1;
} 

void *
UH323InitMidInstance(void *arg)
{
	HAPP 	hApp;
	char  ipStr[80];
	int rc;

	sm_p(h323initsem, 0, 0);

	if (UH323SetupConfigs("h323cfg-gktemplate.val", TMP_MINSTANCE_ID) < 0)
	{
		NETERROR(MINIT, ("Config Initialization failed for instance %d\n",
			TMP_MINSTANCE_ID));
	}

	/* Initialize the stack instances */
	if (cmInitialize(h323tmpvalfile, &hApp) < 0)
	{
		NETERROR(MH323, ("H.323 Initialization failed for %s\n", h323tmpvalfile));
		return 0;
	}

	UH323GlobalsInit(hApp, nh323Instances-1);

	if(uh323InitPI()<0)
	{
		NETERROR(MH323, ("uh323InitPI() failed \n"));
	}
	
	// Inform main loop that we have initialized
	sm_v(h323waitsem);
	sm_v(h323initsem);

	// This instance does not have to exist and so
	// we dont have to record our thread id

	return 0;
}

int
UH323InitThread(int outid)
{
	if ((outid > 0) && (nh323Instances == 1))
	{
		UH323GlobalsAttach(0);
		cmThreadAttach(UH323Globals()->hApp, pthread_self());

		UH323InitVarsKeys();
		uh323InitVars();

	}
	else if (outid >= 1)
	{
		UH323InitInstance(h323valfiles[outid], outid);

		if (outid == ARQ_MINSTANCE_ID)
		{
			// Initialize the last instance ONLY in the end
			ThreadLaunch(UH323InitMidInstance, 0, 1);
		}
	}

	return 1;
}

int
UH323InitInstance(char *valfile, int instance)
{
	HAPP 	hApp;
	HCFG	hCfg;
	char  ipStr[80];
	int rc;

	sm_p(h323initsem, 0, 0);

	if (UH323SetupConfigs("h323cfg-gktemplate.val", instance) < 0)
	{
		NETERROR(MINIT, ("Config Initialization failed for instance %d\n",
			instance));
	}

	/* Initialize the stack instances */
	if (cmInitialize(valfile, &hApp) < 0)
	{
		NETERROR(MH323, ("H.323 Initialization failed for %s, instance %d\n", 
			valfile, instance));
		return -1;
	}

	UH323ReadConfigs(valfile, instance);
	UH323GlobalsInit(hApp, instance);
 	UH323InstanceInitRealm(instance);


	UH323InitVarsKeys();
	uh323InitVars();

	sm_v(h323waitsem);
	sm_v(h323initsem);

	return 1;
}

int
UH323GlobalsInit(HAPP hApp, int instance)
{
	char fn[] = "UH323GlobalsInit():";
	int status, i;
	UH323_tGlobals *glbs;

	glbs = &uh323Globals[instance];

	glbs->hApp = hApp;
	glbs->instance = instance;
	
	// setup stats pointers
	lsMem->allocStats[instance]->nCalls =
		&glbs->nCalls;
	lsMem->allocStats[instance]->arqRate =
		&glbs->narqs;
	lsMem->allocStats[instance]->setupRate =
		&glbs->nsetups;
	lsMem->allocStats[instance]->selims =
		&glbs->selims;
	lsMem->allocStats[instance]->bridgems =
		&glbs->bridgems;

	if ((instance == 0) || (instance != (nh323Instances-1)))
	{
		glbs->peerhApp = hApp;
	}
	else
	{
		for (i=0; i<nh323Instances-1; i++)
		{
			uh323Globals[i].peerhApp = hApp;
		}
	}

	if (status = pthread_setspecific(UH323GlobalsKey, glbs))
	{
		NETERROR(MH323, ("%s pthread_setspecific error %d\n", fn, status));
		return -1;
	}
	
   	if ((glbs->threadId = RvH323ThreadGetHandle()) == NULL)
	{
		NETERROR(MH323, ("%s RvH323ThreadGetHandle() returned error \n", fn));
	}

	return 0;
}

// config file must be read by this point
int
UH323InitCont(void)
{

	UH323LoggingInit();
	return 0;
} 

static void UH323InstanceInitRealm(int instance)
{
	static char fn[] = "UH323InstanceInitRealms";
	int i = 0;
	int listeners, sd;
  	RvH323ThreadHandle    threadId;
	UH323_tGlobals *glbs = NULL;
	Ras_type rastype;

	CacheRealmEntry *realmEntry;
	CacheGetLocks (realmCache, LOCK_READ, LOCK_BLOCK);
	glbs = &uh323Globals[instance];

	if ((nh323Instances > 1) && (instance == SGK_MINSTANCE_ID))
		rastype = Ras_eSgk;
	else
		rastype = Ras_eArq;
	for (realmEntry = (CacheRealmEntry *)CacheGetFirst (realmCache); 
			realmEntry;
		   	realmEntry = (CacheRealmEntry *)CacheGetNext(realmCache, &realmEntry->realm.realmId))
	{
		if(realmEntry->realm.adminStatus != 1)
		{
			continue;
		}
		if ((sd = createRasListener(realmEntry->realm.realmId, rastype,
			realmEntry->realm.rsa,h323PortArray[instance].rasPort,
			h323PortArray[instance].q931Port)) <0  )
		{
			NETERROR (MH323, ("%s: Failed to initialize realm %lu\n", 
				fn,realmEntry->realm.realmId));
			continue;
		}
		NETDEBUG(MH323,NETLOG_DEBUG4,("%s: Initialized realmId %lu sd = %d\n", 
			fn,realmEntry->realm.realmId,sd));
		threadId = 	glbs->threadId;
 		if (seliCallOnThread(sd, seliEvRead, (seliCallback) rasRecv, threadId) < 0)
		{
			NETERROR (MH323, ("%s: seliCallOnThread failure fd = %d, Failed to initialize realm %lu\n", 
				fn, sd, realmEntry->realm.realmId));
		}
	}

	CacheReleaseLocks (realmCache);
}


// If a Realm is admin disabled, but it exists in our listener
// set, take it out.
// If a realm does not exist in our listener set add it

int
UH323RealmReconfig(unsigned long rsa)
{
	static char fn[] = "UH323RealmReconfig";
	CacheRealmEntry *realmEntry;
	int i = 0;
	int	sd, ip, rc, listeners;
	unsigned short	port;
	unsigned long	realmid;
	RvH323ThreadHandle    threadId;
	UH323_tGlobals *glbs = NULL;

	CacheGetLocks (realmCache, LOCK_READ, LOCK_BLOCK);
	realmEntry = CacheGet(rsaCache, &rsa);
	if (realmEntry == NULL)
	{
		NETERROR (MH323, ("%s : Realm Entry for rsa 0x%lx not found \n",fn,rsa));
		goto _return;
	}

	realmid = realmEntry->realm.realmId;
	listeners = (nh323Instances == 1) ? nh323Instances: nh323Instances - 2;
	rc = getRasInfoFromRealmId(realmid, Ras_eArq, &sd, &ip, &port);

	if(rc == -1) 
	{
		/* Newly added realm with adminStatus Enabled */
		for (i = 0; i < listeners; ++i)
		{
			glbs = &uh323Globals[i];
			if(realmEntry->realm.adminStatus == 1)
			{
				if ((sd = createRasListener(realmid,Ras_eArq,rsa,h323PortArray[i].rasPort,
											h323PortArray[i].q931Port)) <0  )
				{
					NETERROR (MH323, ("%s: Failed to init H323 Ras_eArq on realm %s\n", 
						fn,realmEntry->realm.realmName));
					goto _return;
				}

				NETDEBUG(MH323,NETLOG_DEBUG4,("%s: Initialized H323 Ras_eArq on realm %s sd = %d\n", 
											fn,realmEntry->realm.realmName,sd));
				threadId = 	glbs->threadId;
 				if (seliCallOnThread(sd, seliEvRead, (seliCallback) rasRecv, threadId) < 0)
				{
					NETERROR (MH323, ("%s: seliCallOnThread failure fd = %d, Failed to initialize realm %lu\n", 
						fn, sd, realmEntry->realm.realmId));
				}
			}
		}

		if(iserverPrimary)
		{
			realmEntry->realm.operStatus = StatusChgIf(realmEntry->realm.vipName ,1);
		}

		if(nh323Instances > 1)
		{
			glbs = &uh323Globals[nh323Instances - 3];
			if ((sd = createRasListener(realmid,Ras_eSgk,rsa,
										h323PortArray[nh323Instances - 3].rasPort,
										h323PortArray[nh323Instances - 3].q931Port)) <0 )
			{
				NETERROR (MH323, ("%s: Failed to initialize H323 Ras_eSgk on realm %s\n",
									fn,realmEntry->realm.realmName));
				goto _return;
			}
				
			NETDEBUG(MH323,NETLOG_DEBUG4,("%s: Initialized H323 Ras_eSgk on realm %s sd = %d\n", 
											fn,realmEntry->realm.realmName,sd));
			threadId = 	glbs->threadId;
 			if (seliCallOnThread(sd, seliEvRead, (seliCallback) rasRecv, threadId) < 0)
			{
				NETERROR (MH323, ("%s: seliCallOnThread failure fd = %d, Failed to initialize realm %lu\n", 
					fn, sd, realmEntry->realm.realmId));
			}

			glbs = &uh323Globals[nh323Instances - 2];
			if ((sd = createRasListener(realmid,Ras_eArq, rsa,
										h323PortArray[nh323Instances - 2].rasPort,
										h323PortArray[nh323Instances - 2].q931Port)) < 0 )
			{
				 NETERROR (MH323, ("%s: Failed to initialize H323 Ras_eArq on realm %s\n",
									 fn,realmEntry->realm.realmName ));
				 goto _return;
			}

			NETDEBUG(MH323,NETLOG_DEBUG4,("%s: Initialized H323 Ras_eArq on realm %s sd = %d\n", 
											fn,realmEntry->realm.realmName,sd));

			threadId = 	glbs->threadId;
 			if (seliCallOnThread(sd, seliEvRead, (seliCallback) rasRecv, threadId) < 0)
			{
				NETERROR (MH323, ("%s: seliCallOnThread failure fd = %d, Failed to initialize realm %lu\n", 
					fn, sd, realmEntry->realm.realmId));
			}
		}
	}
	else
	{
		if (realmEntry->realm.adminStatus == 1)
		{
			goto _return;
		}

		for (i = 0; i < listeners; ++i)
		{
			/* the following assumes that ordering has been preserved which
			 * will be since its an array with ever increasing index. List
			 * implmentation might create troubles here
			 */
			glbs = &uh323Globals[i];
			if ((sd = destroyRasListener(realmid, Ras_eArq, h323PortArray[i].rasPort)) < 0)
			{
				 NETERROR (MH323, ("%s: Failed to stop H323 Ras_eArq on realm %s listener %d\n",
									 fn,realmEntry->realm.realmName, i));
				 continue;
			}

			threadId = 	glbs->threadId;
 			if (seliCallOnThread(sd, 0, (seliCallback) NULL, threadId) < 0)
			{
				NETERROR (MH323, ("%s: seliCallOnThread failure fd = %d, Failed to remove fd %lu\n", 
					fn, sd, realmEntry->realm.realmId));
			}
		}

		if (nh323Instances > 1)
		{
			glbs = &uh323Globals[nh323Instances - 3];
			if ((sd = destroyRasListener(realmid,Ras_eSgk,
										h323PortArray[nh323Instances - 3].rasPort)) <0 )
			{
				NETERROR (MH323, ("%s: Failed to stop H323 Ras_eSgk on realm %s\n",
									fn,realmEntry->realm.realmName));
			}
			else
			{
				threadId = 	glbs->threadId;
	 			if (seliCallOnThread(sd, 0, (seliCallback) NULL, threadId) < 0)
				{
					NETERROR (MH323, ("%s: seliCallOnThread failure fd = %d, Failed to remove fd %lu\n", 
						fn, sd, realmEntry->realm.realmId));
				}
				NETDEBUG(MH323,NETLOG_DEBUG4,("%s: stopped H323 Ras_eSgk on realm %s sd = %d\n", 
											fn,realmEntry->realm.realmName,sd));
			}
			
			glbs = &uh323Globals[nh323Instances - 2];
			if ((sd = destroyRasListener(realmid,Ras_eArq,
										h323PortArray[nh323Instances - 2].rasPort)) <0 )
			{
				NETERROR (MH323, ("%s: Failed to stop H323 Ras_eArq on realm %s\n",
									fn,realmEntry->realm.realmName));
			}
			else
			{
				threadId = 	glbs->threadId;
	 			if (seliCallOnThread(sd, 0, (seliCallback) NULL, threadId) < 0)
				{
					NETERROR (MH323, ("%s: seliCallOnThread failure fd = %d, Failed to remove fd %lu\n", 
						fn, sd, realmEntry->realm.realmId));
				}
				NETDEBUG (MH323,NETLOG_DEBUG4,("%s: stopped H323 Ras_eArq on realm %s sd = %d\n", 
											fn,realmEntry->realm.realmName, sd));
			}
		}
	}

_return:
	CacheReleaseLocks(realmCache);
	return 0;
}

// Called after all instances are initialized
int
UH323Ready(int id)
{
	int i;

	if (id != 0)
	{
		return 0;
	}

	sm_p(h323initsem, 0, 0);

	sm_v(h323initsem);

	for (i=1; i<nh323Instances; i++)
	{
		sm_p(h323waitsem, 0, 0);
	}

	for (i=1; i<nh323Instances; i++)
	{
		sm_v(h323waitsem);
	}

	// We are ready to setup the callbacks
	for (i=1; i<nh323Instances-1; i++)
	{
		UH323CBInit(i);
	}

	UH323CBInit(0);

	/* Add cachetimer for Gk Cache */
	memset(&rastmr, 0, sizeof(struct itimerval));
	rastmr.it_interval.tv_sec = 10; //use fixed 10 sec poll timer, it was using rrqtimer;
	
	rastid = timerAddToList(
			&h323timerPrivate[sgkInstance], 
			&rastmr,
			0,PSOS_TIMER_REL, "GkPollTimer", 
			GkPoll, NULL);

	/* We are ready to go into the main loop now */
	NETDEBUG(MH323, NETLOG_DEBUG3,	("H323 Initialization done for all instances\n"));

	/* Add polltimer for keep alive */
	memset(&rastmr, 0, sizeof(struct itimerval));
	rastmr.it_interval.tv_sec = 15;

	timerAddToList(&h323timerPrivate[0], &rastmr,
		0,PSOS_TIMER_REL, "ATimer", iServerUpdateH323Allocations, 
		NULL);

	return 0;
}

// Initialize the callbacks
int
UH323CBInit(int instance)
{
	char  ipStr[80];
	int rc;
	HAPP 	hApp = uh323Globals[instance].hApp;

	/* Set our RAS event handler */
	cmRASSetEventHandler(hApp,
		&cmRASEvent,
		sizeof(cmRASEvent));

	/* Setup our hook callbacks */
	cmSetProtocolEventHandler(hApp,
		&cmProtocolEvent,
		sizeof(cmProtocolEvent));

	/* Initialize the callbacks */
	cmSetGenEventHandler(hApp, 
		&cmEvent,
		sizeof(cmEvent));

	cmSetCallEventHandler(hApp,
		&cmCallEvent,
		sizeof(cmCallEvent));

	cmSetControlEventHandler(hApp,
		&cmControlEvent,
		sizeof(cmControlEvent));

	cmSetChannelEventHandler(hApp,
		&cmChannelEvent,
		sizeof(cmChannelEvent));

	/* Check our RAS address */
	rc = cmGetLocalRASAddress(
		hApp, 
		&uh323Globals[instance].rasAddr);

	if (!rc)
	{
		NETDEBUG(MH323, NETLOG_DEBUG3,
			("cmGetLocalRASAddress: %s:%d\n\n", 
			liIpToString(uh323Globals[instance].rasAddr.ip,ipStr), uh323Globals[instance].rasAddr.port));
	}
	else
	{
		NETERROR(MH323, 
			("H.323 initialization error for instance %d\n", instance));
	}


	/* Check our RAS address */
	rc = cmGetLocalCallSignalAddress(
			hApp, 
			&uh323Globals[instance].sigAddr);

	if (!rc)
	{
		NETDEBUG(MH323, NETLOG_DEBUG3,
			("cmGetLocalCallSignalAddress: %s:%d\n\n", 
			liIpToString(uh323Globals[instance].sigAddr.ip,ipStr), uh323Globals[instance].sigAddr.port));
	}
	else
	{
		NETERROR(MH323, 
			("H.323 initialization error for instance %d\n", instance));
	}

	/* We are ready to go into the main loop now */
	NETDEBUG(MH323, NETLOG_DEBUG3,	("H323 CB Initialization done for instance %d\n", 
		instance));

	NETDEBUG(MH323, NETLOG_DEBUG3, 
		("Stack Handle instance %d = %p, maxCalls = %d, sig=%x/%d, ras %x/%d\n", 
		instance, hApp, uh323Globals[instance].maxCalls, uh323Globals[instance].sigAddr.ip,
		uh323Globals[instance].sigAddr.port, uh323Globals[instance].rasAddr.ip, 
		uh323Globals[instance].rasAddr.port));

	return 0;
} 

int
UH323DetermineBestSigAddr(cmTransportAddress *dstSignalAddr)
{
	int i, bestInstance = -1;

	if ((nh323Instances == 1) && 
		((2*uh323Globals[0].nCalls + h323maxCallsPadFixed < uh323Globals[0].maxCalls) ||
		(2*uh323Globals[0].nCalls*100 < h323maxCallsPadVariable*uh323Globals[0].maxCalls)))
	{
		bestInstance = 0;
	}

	for (i=0; i< SGK_MINSTANCE_ID; i+=2)
	{
		if ((uh323Globals[i].nCalls + h323maxCallsPadFixed > uh323Globals[i].maxCalls) &&
			(uh323Globals[i].nCalls*100 > h323maxCallsPadVariable*uh323Globals[i].maxCalls))
		{
			continue;
		}

		if ((bestInstance < 0) ||
			(uh323Globals[i].nCalls < uh323Globals[bestInstance].nCalls))
		{
			bestInstance = i;
		}
	}

	if (bestInstance < 0)
	{
		return -1;
	}
	else
	{
		if (dstSignalAddr)
		{
			*dstSignalAddr = uh323Globals[bestInstance].sigAddr;
		}

		return 1;
	}
}

// An instance MUST be returned in this scenario. If unavailable,
// call can later be rejected by the instance.
pthread_t
UH323DetermineBestOutThread(int isDestSgk)
{
	int i, bestInstance = 1;

	if (nh323Instances == 1)
	{
		if (nH323Threads == 1)
		{
			return 0;
		}
		else
		{
			return 1;
		}
	}

	if (isDestSgk) 
	{
		if (uh323Globals[sgkInstance].nCalls <
				uh323Globals[sgkInstance].maxCalls)
		{
			return sgkInstance;
		}
		else
		{
			NETERROR(MH323, ("maxsgk calls must be increased %d, %d\n",
				uh323Globals[sgkInstance].nCalls, 
				uh323Globals[sgkInstance].maxCalls));
		}
	}

	for (i=1; i< SGK_MINSTANCE_ID; i+=2)
	{
		if (uh323Globals[i].nCalls >= uh323Globals[i].maxCalls)
		{
			continue;
		}

		if ((bestInstance < 0) ||
			(uh323Globals[i].nCalls < uh323Globals[bestInstance].nCalls))
		{
			bestInstance = i;
		}
	}

	return bestInstance;
}

UH323CallAppHandle *
uh323CallAllocAppHandle(void)
{
	char fn[] = "uh323AllocCallAppHandle():";
	UH323CallAppHandle *appHandle;

	appHandle = (UH323CallAppHandle *)malloc(sizeof(UH323CallAppHandle));
	memset(appHandle, 0, sizeof(UH323CallAppHandle));

	NETDEBUG(MH323, NETLOG_DEBUG4, ("%s allocated %p\n", fn, appHandle));

	return appHandle;
}

void
uh323CallFreeAppHandle(UH323CallAppHandle *appHandle)
{
	if (appHandle)
	{
		free (appHandle);
	}
}

int
uh323CallInitAppHandle(HCALL hsCall, UH323CallAppHandle *appHandle)
{
	char fn[] = "uh323CallInitAppHandle():";
	INT32 tlen;
	char	callID[CALL_ID_LEN] = {0},confID[CONF_ID_LEN] = {0};
	char 	callIDStr[CALL_ID_LEN+1], confIDStr[CONF_ID_LEN+1];


	if(appHandle == 0 )
	{
		NETERROR(MH323, ("%s appHandle NULL hsCall = %p \n", fn, hsCall));
		return -1;
	}	

	if(memcmp(appHandle->confID,confID,CONF_ID_LEN))
	{
		NETDEBUG(MH323, NETLOG_DEBUG3, 
			("appHandle being reinitialized.CallID = %s confID = %s\n",
			(char*) CallID2String(appHandle->callID,callIDStr),
			(char*) CallID2String(appHandle->confID,confIDStr)));
	}


	if(memcmp(appHandle->callID,callID,CALL_ID_LEN) )
	{
		NETDEBUG(MH323,NETLOG_DEBUG1,("%s : callId(%s) getting reinitialized\n",
			fn, (char*) CallID2String(appHandle->callID,callIDStr)));
	}

	generateConfId(appHandle->confID);
	generateCallId(appHandle->callID);

	return 0;
}

int
uh323InitVars()
{
	static char 	fn[] = 	"uh323InitVars()";
	INT32 			h245Conf;
	int 			nodeId;
	HPST 			hSyn = cmGetSynTreeByRootName(UH323Globals()->hApp,"capData");
	HPVT 			hVal = cmGetValTree(UH323Globals()->hApp);
	H323RTPSet		rtpSet = {0};

	
	h245Conf = cmGetH245ConfigurationHandle(UH323Globals()->hApp);
	hVal = cmGetValTree(UH323Globals()->hApp);

	cmMeiEnter(UH323Globals()->hApp);

	/* get local Capability */
	localTCSId = pvtGetNodeIdByPath(hVal, h245Conf, 
				".capabilities.terminalCapabilitySet");


	/* build empty TCS */
	emptyTCSId = pvtAddRoot(hVal,
				hSyn,
				0,
				NULL);

	pvtSetTree(hVal,emptyTCSId,hVal,localTCSId);

	if((nodeId = pvtGetNodeIdByPath(hVal,emptyTCSId,".multiplexCapability")) <0)
	{
		NETERROR(MH323,
			("%s Could not obtain capNodeId .multiplexCapability= %d \n", fn,nodeId));
	}
	else if(pvtDelete(hVal,nodeId) <0)
	{
		cmMeiExit(UH323Globals()->hApp);
		NETERROR(MH323,
		("%s Could not delete multiplexCapability subtree\n", fn));
		return -1;
	}


	cmMeiExit(UH323Globals()->hApp);

	/* Create some default data handles here */
	rtpSet.codecType = CodecGPCMU;
	rtpSet.param = 20;
	createDataHandle(&rtpSet);

	rtpSet.codecType = CodecGPCMA;
	rtpSet.param = 20;
	createDataHandle(&rtpSet);

	rtpSet.codecType = CodecG7231;
	rtpSet.param = 3;
	createDataHandle(&rtpSet);

	rtpSet.codecType = CodecG729;
	rtpSet.param = 2;
	createDataHandle(&rtpSet);
	rtpSet.param = 3;
	createDataHandle(&rtpSet);

	rtpSet.codecType = CodecG729A;
	rtpSet.param = 2;
	createDataHandle(&rtpSet);
	rtpSet.param = 3;
	createDataHandle(&rtpSet);

	rtpSet.codecType = CodecG729B;
	rtpSet.param = 2;
	createDataHandle(&rtpSet);

	rtpSet.codecType = CodecG729AwB;
	rtpSet.param = 2;
	createDataHandle(&rtpSet);

	rtpSet.codecType = CodecG728;
	rtpSet.param = 2;
	createDataHandle(&rtpSet);

	hPstSetup = pstConstruct(cmEmGetQ931Syntax(),"Setup-UUIE");
	if(hPstSetup == NULL)
	{
		NETERROR(MH323,
			("%s pstConstruct Setup-UUIE Error\n",fn));
	}

	if((hSynAlerting = pstConstruct(cmEmGetQ931Syntax(),"AlertingMessage"))<0)
	{
		NETERROR(MH323, ("pstConstruct for AlertingMessage failed\n"));
	}

	if((hSynVideo = pstConstruct(cmEmGetH245Syntax(), (char*)"VideoCapability")) < 0)
	{
		NETERROR(MH323, ("pstConstruct for video Cap failed\n"));
	}
	if((hSynData = pstConstruct(cmEmGetH245Syntax(), (char*)"DataApplicationCapability")) < 0)
	{
		NETERROR(MH323, ("pstConstruct for DataApplication failed\n"));
	}
	if((hSynBearerCap= pstConstruct(cmEmGetQ931Syntax(),"BearerCapability"))<0)
	{
		NETERROR(MH323, ("pstConstruct for BearerCapability failed\n"));
	}

	return(0);
}

int uh323GetLocalCaps()
{
#ifndef _slowdown
	return localTCSId;
#else
	char fn[] = "uh323GetLocalCaps()";
	INT32 h245Conf;
	HPVT hVal;
	int nodeId;

	h245Conf = cmGetH245ConfigurationHandle(UH323Globals()->hApp);
	hVal = cmGetValTree(UH323Globals()->hApp);

	cmMeiEnter(UH323Globals()->hApp);

	nodeId = pvtGetNodeIdByPath(hVal, h245Conf, 
				".capabilities.terminalCapabilitySet");

	cmMeiExit(UH323Globals()->hApp);

	return nodeId;
#endif
}
// Initialize the H.323 stack
int
uh323StackLock(void)
{
#if 0
	if (cmMeiEnter(UH323Globals()->hApp) < 0)
	{
		NETERROR(MH323, ("uh323Stack locks failed!!!\n"));
	}
#endif
	return(0);
}

int
uh323StackUnlock(void)
{
#if 0
	cmMeiExit(UH323Globals()->hApp);
#endif
	return(0);
}

static int initmsDebugLevel = 0;

int
_dummymsSetDebugLevel(int level)
{
	initmsDebugLevel = level;

	return(0);
}

static List initmsList = NULL;

int
_dummymsAdd(char *name)
{
	if (!initmsList)
	{
		initmsList = listInit();
	}

	listAddItem(initmsList, strdup(name));

	return(0);
}

void
initmsLogging()
{
	char *name;

	msSetDebugLevel(initmsDebugLevel);
	if (initmsList)
	{
		while (name = listGetFirstItem(initmsList))
		{
			msAdd(name);
			listDeleteItem(initmsList, name);
			free(name);
		}
	}

	listDestroy(initmsList);
	initmsList = NULL;
}

int
UH323LoggingInit(void)
{
	initmsLogging();

#ifndef NODEBUGLOG

	_msAdd = msAdd;
	_msDelete = msDelete;
	_msSetDebugLevel = msSetDebugLevel;
	_msDeleteAll = msDeleteAll;
#else
	_msAdd = 0;
	_msDelete = 0;
	_msSetDebugLevel = 0;
	_msDeleteAll = 0;
#endif
	return(0);
}

UH323_tGlobals *
UH323Globals(void)
{
	UH323_tGlobals *glbs;
	pthread_t myid;

	glbs = pthread_getspecific(UH323GlobalsKey);

	myid = pthread_self();

	if (glbs == NULL)
	{
		NETERROR(MH323, ("thread %lu does not have global key!\n", ULONG_FMT(myid)));
	}

	return glbs;
}

int
UH323GlobalsInitKey(void)
{
	char fn[] = "UH323GlobalsInitKey():";
	int status;

	if (status = pthread_key_create(&UH323GlobalsKey, NULL))
	{
		NETERROR(MINIT, ("%s pthread_key_create error %d\n", fn, status));
		return -1;
	}

	return 0;
}

pthread_key_t localTCSIdKey,
emptyTCSIdKey,
dtmfDataHandleKey,
dtmfStringDataHandleKey,
faxDataHandleKey,
faxLucentDataHandleKey,
rfc2833DataHandleKey,
gcpmaKey,
gcpmuKey,
g729aKey,
g729Key,
g723Key,
g723SSKey,
g728Key,
g729bKey,
g729awbKey,
mswNonStandardKey;

int
UH323InitVarsKeys(void)
{
	char fn[] = "UH323InitVarsKeys():";
	int status;
	int *glbs;

	glbs = (int *)malloc(sizeof(int));
	if (status = pthread_setspecific(localTCSIdKey, glbs))
	{
		NETERROR(MH323, ("%s pthread_setspecific error %d\n", fn, status));
		return -1;
	}

	glbs = (int *)malloc(sizeof(int));
	if (status = pthread_setspecific(emptyTCSIdKey, glbs))
	{
		NETERROR(MH323, ("%s pthread_setspecific error %d\n", fn, status));
		return -1;
	}

	glbs = (int *)malloc(sizeof(int));
	memset(glbs, 0, sizeof(int));
	if (status = pthread_setspecific(dtmfDataHandleKey, glbs))
	{
		NETERROR(MH323, ("%s pthread_setspecific error %d\n", fn, status));
		return -1;
	}

	glbs = (int *)malloc(sizeof(int));
	memset(glbs, 0, sizeof(int));
	if (status = pthread_setspecific(dtmfStringDataHandleKey, glbs))
	{
		NETERROR(MH323, ("%s pthread_setspecific error %d\n", fn, status));
		return -1;
	}

	glbs = (int *)malloc(sizeof(int));
	memset(glbs, 0, sizeof(int));
	if (status = pthread_setspecific(faxDataHandleKey, glbs))
	{
		NETERROR(MH323, ("%s pthread_setspecific error %d\n", fn, status));
		return -1;
	}

	glbs = (int *)malloc(sizeof(int));
	memset(glbs, 0, sizeof(int));
	if (status = pthread_setspecific(faxLucentDataHandleKey, glbs))
	{
		NETERROR(MH323, ("%s pthread_setspecific error %d\n", fn, status));
		return -1;
	}

	glbs = (int *)malloc(sizeof(int));
	memset(glbs, 0, sizeof(int));
	if (status = pthread_setspecific(rfc2833DataHandleKey, glbs))
	{
		NETERROR(MH323, ("%s pthread_setspecific error %d\n", fn, status));
		return -1;
	}

	glbs = (int *)malloc(MAXPARAM*sizeof(int));
	memset(glbs, 0, MAXPARAM*sizeof(int));
	if (status = pthread_setspecific(gcpmaKey, glbs))
	{
		NETERROR(MH323, ("%s pthread_setspecific error %d\n", fn, status));
		return -1;
	}

	glbs = (int *)malloc(MAXPARAM*sizeof(int));
	memset(glbs, 0, MAXPARAM*sizeof(int));
	if (status = pthread_setspecific(gcpmuKey, glbs))
	{
		NETERROR(MH323, ("%s pthread_setspecific error %d\n", fn, status));
		return -1;
	}

	glbs = (int *)malloc(MAXPARAM*sizeof(int));
	memset(glbs, 0, MAXPARAM*sizeof(int));
	if (status = pthread_setspecific(g729aKey, glbs))
	{
		NETERROR(MH323, ("%s pthread_setspecific error %d\n", fn, status));
		return -1;
	}

	glbs = (int *)malloc(MAXPARAM*sizeof(int));
	memset(glbs, 0, MAXPARAM*sizeof(int));
	if (status = pthread_setspecific(g729Key, glbs))
	{
		NETERROR(MH323, ("%s pthread_setspecific error %d\n", fn, status));
		return -1;
	}

	glbs = (int *)malloc(MAXPARAM*sizeof(int));
	memset(glbs, 0, MAXPARAM*sizeof(int));
	if (status = pthread_setspecific(g723Key, glbs))
	{
		NETERROR(MH323, ("%s pthread_setspecific error %d\n", fn, status));
		return -1;
	}

	glbs = (int *)malloc(MAXPARAM*sizeof(int));
	memset(glbs, 0, MAXPARAM*sizeof(int));
	if (status = pthread_setspecific(g723SSKey, glbs))
	{
		NETERROR(MH323, ("%s pthread_setspecific error %d\n", fn, status));
		return -1;
	}

	glbs = (int *)malloc(MAXPARAM*sizeof(int));
	memset(glbs, 0, MAXPARAM*sizeof(int));
	if (status = pthread_setspecific(g728Key, glbs))
	{
		NETERROR(MH323, ("%s pthread_setspecific error %d\n", fn, status));
		return -1;
	}

	glbs = (int *)malloc(MAXPARAM*sizeof(int));
	memset(glbs, 0, MAXPARAM*sizeof(int));
	if (status = pthread_setspecific(g729bKey, glbs))
	{
		NETERROR(MH323, ("%s pthread_setspecific error %d\n", fn, status));
		return -1;
	}

	glbs = (int *)malloc(MAXPARAM*sizeof(int));
	memset(glbs, 0, MAXPARAM*sizeof(int));
	if (status = pthread_setspecific(g729awbKey, glbs))
	{
		NETERROR(MH323, ("%s pthread_setspecific error %d\n", fn, status));
		return -1;
	}

	glbs = (int *)malloc(MAXPARAM*sizeof(int));
	memset(glbs, 0, MAXPARAM*sizeof(int));
	if (status = pthread_setspecific(mswNonStandardKey, glbs))
	{
		NETERROR(MH323, ("%s pthread_setspecific error %d\n", fn, status));
		return -1;
	}

	return 0;
}

int
UH323AllocVarsKeys(void)
{
	char fn[] = "UH323AllocVarsKeys():";
	int status;
	int *glbs;

	// Initialze keys for all global variables
	if (status = pthread_key_create(&localTCSIdKey, NULL))
	{
		NETERROR(MINIT, ("%s pthread_key_create error %d\n", fn, status));
		return -1;
	}

	if (status = pthread_key_create(&emptyTCSIdKey, NULL))
	{
		NETERROR(MINIT, ("%s pthread_key_create error %d\n", fn, status));
		return -1;
	}

	if (status = pthread_key_create(&dtmfDataHandleKey, NULL))
	{
		NETERROR(MINIT, ("%s pthread_key_create error %d\n", fn, status));
		return -1;
	}

	if (status = pthread_key_create(&dtmfStringDataHandleKey, NULL))
	{
		NETERROR(MINIT, ("%s pthread_key_create error %d\n", fn, status));
		return -1;
	}

	if (status = pthread_key_create(&faxDataHandleKey, NULL))
	{
		NETERROR(MINIT, ("%s pthread_key_create error %d\n", fn, status));
		return -1;
	}

	if (status = pthread_key_create(&faxLucentDataHandleKey, NULL))
	{
		NETERROR(MINIT, ("%s pthread_key_create error %d\n", fn, status));
		return -1;
	}

	if (status = pthread_key_create(&rfc2833DataHandleKey, NULL))
	{
		NETERROR(MINIT, ("%s pthread_key_create error %d\n", fn, status));
		return -1;
	}

	if (status = pthread_key_create(&gcpmaKey, NULL))
	{
		NETERROR(MINIT, ("%s pthread_key_create error %d\n", fn, status));
		return -1;
	}

	if (status = pthread_key_create(&gcpmuKey, NULL))
	{
		NETERROR(MINIT, ("%s pthread_key_create error %d\n", fn, status));
		return -1;
	}

	if (status = pthread_key_create(&g729aKey, NULL))
	{
		NETERROR(MINIT, ("%s pthread_key_create error %d\n", fn, status));
		return -1;
	}

	if (status = pthread_key_create(&g729Key, NULL))
	{
		NETERROR(MINIT, ("%s pthread_key_create error %d\n", fn, status));
		return -1;
	}

	if (status = pthread_key_create(&g723Key, NULL))
	{
		NETERROR(MINIT, ("%s pthread_key_create error %d\n", fn, status));
		return -1;
	}

	if (status = pthread_key_create(&g723SSKey, NULL))
	{
		NETERROR(MINIT, ("%s pthread_key_create error %d\n", fn, status));
		return -1;
	}

	if (status = pthread_key_create(&g728Key, NULL))
	{
		NETERROR(MINIT, ("%s pthread_key_create error %d\n", fn, status));
		return -1;
	}

	if (status = pthread_key_create(&g729bKey, NULL))
	{
		NETERROR(MINIT, ("%s pthread_key_create error %d\n", fn, status));
		return -1;
	}

	if (status = pthread_key_create(&g729awbKey, NULL))
	{
		NETERROR(MINIT, ("%s pthread_key_create error %d\n", fn, status));
		return -1;
	}

	if (status = pthread_key_create(&mswNonStandardKey, NULL))
	{
		NETERROR(MINIT, ("%s pthread_key_create error %d\n", fn, status));
		return -1;
	}

	return 0;
}

int
UH323GlobalsAttach(int instance)
{
	char fn[] = "UH323GlobalsAttach():";
	int status;
	UH323_tGlobals *glbs;

	glbs = &uh323Globals[instance];

	if (status = pthread_setspecific(UH323GlobalsKey, glbs))
	{
		NETERROR(MH323, ("%s pthread_setspecific error %d\n", fn, status));
		return -1;
	}

	return 0;
}

int *
_localTCSId()
{
	int *kv;
	pthread_t myid;

	kv = pthread_getspecific(localTCSIdKey);

	myid = pthread_self();

	if (kv == NULL)
	{
		NETERROR(MH323, ("thread %lu does not have global key!\n", ULONG_FMT(myid)));
	}

	return kv;
}

int *
_emptyTCSId()
{
	int *kv;
	pthread_t myid;

	kv = pthread_getspecific(emptyTCSIdKey);

	myid = pthread_self();

	if (kv == NULL)
	{
		NETERROR(MH323, ("thread %lu does not have global key!\n", ULONG_FMT(myid)));
	}

	return kv;
}

int *
_dtmfDataHandle()
{
	int *kv;
	pthread_t myid;

	kv = pthread_getspecific(dtmfDataHandleKey);

	myid = pthread_self();

	if (kv == NULL)
	{
		NETERROR(MH323, ("thread %lu does not have global key!\n", ULONG_FMT(myid)));
	}

	return kv;
}

int *
_dtmfStringDataHandle()
{
	int *kv;
	pthread_t myid;

	kv = pthread_getspecific(dtmfStringDataHandleKey);

	myid = pthread_self();

	if (kv == NULL)
	{
		NETERROR(MH323, ("thread %lu does not have global key!\n", ULONG_FMT(myid)));
	}

	return kv;
}

int *
_faxDataHandle()
{
	int *kv;
	pthread_t myid;

	kv = pthread_getspecific(faxDataHandleKey);

	myid = pthread_self();

	if (kv == NULL)
	{
		NETERROR(MH323, ("thread %lu does not have global key!\n", ULONG_FMT(myid)));
	}

	return kv;
}

int *
_rfc2833DataHandle()
{
	int *kv;
	pthread_t myid;

	kv = pthread_getspecific(rfc2833DataHandleKey);

	myid = pthread_self();

	if (kv == NULL)
	{
		NETERROR(MH323, ("thread %lu does not have global key!\n", ULONG_FMT(myid)));
	}

	return kv;
}

int *
_faxLucentDataHandle()
{
	int *kv;
	pthread_t myid;

	kv = pthread_getspecific(faxLucentDataHandleKey);

	myid = pthread_self();

	if (kv == NULL)
	{
		NETERROR(MH323, ("thread %lu does not have global key!\n", ULONG_FMT(myid)));
	}

	return kv;
}
int *
_gcpma()
{
	int *kv;
	pthread_t myid;

	kv = pthread_getspecific(gcpmaKey);

	myid = pthread_self();

	if (kv == NULL)
	{
		NETERROR(MH323, ("thread %lu does not have global key!\n", ULONG_FMT(myid)));
	}

	return kv;
}

int *
_gcpmu()
{
	int *kv;
	pthread_t myid;

	kv = pthread_getspecific(gcpmuKey);

	myid = pthread_self();

	if (kv == NULL)
	{
		NETERROR(MH323, ("thread %lu does not have global key!\n", ULONG_FMT(myid)));
	}

	return kv;
}

int *
_g729a()
{
	int *kv;
	pthread_t myid;

	kv = pthread_getspecific(g729aKey);

	myid = pthread_self();

	if (kv == NULL)
	{
		NETERROR(MH323, ("thread %lu does not have global key!\n", ULONG_FMT(myid)));
	}

	return kv;
}

int *
_g729()
{
	int *kv;
	pthread_t myid;

	kv = pthread_getspecific(g729Key);

	myid = pthread_self();

	if (kv == NULL)
	{
		NETERROR(MH323, ("thread %lu does not have global key!\n", ULONG_FMT(myid)));
	}

	return kv;
}

int *
_g723SS()
{
	int *kv;
	pthread_t myid;

	kv = pthread_getspecific(g723SSKey);

	myid = pthread_self();

	if (kv == NULL)
	{
		NETERROR(MH323, ("thread %lu does not have global key!\n", ULONG_FMT(myid)));
	}

	return kv;
}

int *
_g723()
{
	int *kv;
	pthread_t myid;

	kv = pthread_getspecific(g723Key);

	myid = pthread_self();

	if (kv == NULL)
	{
		NETERROR(MH323, ("thread %lu does not have global key!\n", ULONG_FMT(myid)));
	}

	return kv;
}

int *
_g728()
{
	int *kv;
	pthread_t myid;

	kv = pthread_getspecific(g728Key);

	myid = pthread_self();

	if (kv == NULL)
	{
		NETERROR(MH323, ("thread %lu does not have global key!\n", ULONG_FMT(myid)));
	}

	return kv;
}

int *
_g729b()
{
	int *kv;
	pthread_t myid;

	kv = pthread_getspecific(g729bKey);

	myid = pthread_self();

	if (kv == NULL)
	{
		NETERROR(MH323, ("thread %lu does not have global key!\n", ULONG_FMT(myid)));
	}

	return kv;
}

int *
_g729awb()
{
	int *kv;
	pthread_t myid;

	kv = pthread_getspecific(g729awbKey);

	myid = pthread_self();

	if (kv == NULL)
	{
		NETERROR(MH323, ("thread %lu does not have global key!\n", ULONG_FMT(myid)));
	}

	return kv;
}

int *
_mswNonStandard()
{
	int *kv;
	pthread_t myid;

	kv = pthread_getspecific(mswNonStandardKey);

	myid = pthread_self();

	if (kv == NULL)
	{
		NETERROR(MH323, ("thread %lu does not have global key!\n", ULONG_FMT(myid)));
	}

	return kv;
}


int uh323InitPI() 
{
	HPVT hVal = cmGetValTree(UH323Globals()->hApp);
	static char fn[] = "uh323InitPI()";
	int PIrtId;
	int		rv = -1;

	cmMeiEnter(UH323Globals()->hApp);

	if((PIrtId= pvtAddRoot(hVal,hSynAlerting,0, NULL))<0)
	{
			NETERROR(MH323, 
				("%s pvtAddRoot failed\n",fn));
		goto _return;
	}

	if(pvtBuildByPath( hVal, PIrtId, ".progressIndicator",0, NULL) <0)
	{
		NETERROR(MH323, 
			("%s pvtBuildByPath progressIndicator failed\n",fn));
		goto _return;
	}

	if((PINodeId = pvtGetNodeIdByPath(hVal,PIrtId,".progressIndicator"))>0)
	{
#ifdef H323v30
		if(pvtBuildByPath( hVal, PINodeId, ".octet3.ext",1, NULL) <0)
		{
			NETERROR(MH323, 
				("%s pvtBuildByPath failed\n",fn));
		}
#endif

		if(pvtBuildByPath( hVal, PINodeId, ".octet3.codingStandard",0, NULL) <0)
		{
			NETERROR(MH323, 
				("%s pvtBuildByPath octet3 codingStandard failed\n",fn));
		}

		if(pvtBuildByPath( hVal, PINodeId, ".octet3.spare",0, NULL) <0)
		{
			NETERROR(MH323, 
				("%s pvtBuildByPath octet3 spare failed\n",fn));
		}

		if(pvtBuildByPath( hVal, PINodeId, ".octet3.location",0, NULL) <0)
		{
			NETERROR(MH323, 
				("%s pvtBuildByPath octet3 locationfailed\n",fn));
		}

#ifdef H323v30
		if(pvtBuildByPath( hVal, PINodeId, ".octet4.ext",1, NULL) <0)
		{
			NETERROR(MH323, 
				("%s pvtBuildByPath octet4 ext \n",fn));
		}
#endif

		if(pvtBuildByPath( hVal, PINodeId, ".octet4.progressDescription",8, NULL) <0)
		{
			NETERROR(MH323, 
				("%s pvtBuildByPath octet4 progressDescription\n",fn));
		}
		rv = 0;
	}
	else {
		NETERROR(MH323, 
			("%s pvtAddRootByPath Failed failed.\n",fn));
	}

_return:
	cmMeiExit(UH323Globals()->hApp);
	return rv;
}

void 
generateOutgoingH323CallId(char callID[CALL_ID_LEN], int isDestSgk)
{
	// WARNING:  If you call this routine without locking callCache,
	// you may generate duplicate call IDs.
	hrtime_t hrtime;
	pthread_t	threadid;

	hrtime = nx_gethrtime();
	threadid = h323Threads[UH323DetermineBestOutThread(isDestSgk)];

	memcpy(callID, (char *)&iServerIP, 4);
	memcpy(callID+4, (char *)&threadid, 4);
	memcpy(callID+8, (char *)&hrtime, 8);
}

int
UH323UpdateStats(time_t *timeLast, time_t *now, int *ntotal)
{
	if (*now-*timeLast >=1)
	{
		// Window has changed
		*timeLast = *now;	
		*ntotal = 1;
		return 0;
	}

	(*ntotal)++;
	return 0;
}

int UH323Reconfig(void)
{
	static char fn[] = "UH323Reconfig()";
	int 				nodeId,procNodeId;
	HAPP 				hApp;
	HPVT 				hVal;
	int					i;

	for(i = 0;i < nh323Instances;++i)
	{
		UH323GlobalsAttach(i);
		hApp = UH323Globals()->hApp;
		nodeId = cmGetQ931ConfigurationHandle(hApp);
		cmThreadAttach(hApp, pthread_self());
		hVal = cmGetValTree(hApp);

		procNodeId = pvtGetNodeIdByPath(hVal,nodeId,"manualCallProceeding");
		if(localProceeding)
		{
			if( procNodeId >=0) 
			{
				pvtDelete(hVal,procNodeId);
			}
		}
		else  
		{
			// Disable local call proceeding if its not already disabled
			if((procNodeId < 0) && (pvtBuildByPath(hVal,nodeId, "manualCallProceeding",0,NULL) < 0)) 
			{
				NETERROR(MH323, 
				("%s failed to set local call proceeding\n",fn));	
			}
		}
		procNodeId = pvtGetNodeIdByPath(hVal,nodeId,"h245Tunneling");
		if(!h245Tunneling)
		{
			if( procNodeId >=0) 
			{
				pvtDelete(hVal,procNodeId);
			}
		}
		else  
		{
			// Disable local call proceeding if its not already disabled
			if((procNodeId < 0) && (pvtBuildByPath(hVal,nodeId, "h245Tunneling",0,NULL) < 0)) 
			{
				NETERROR(MH323, 
				("%s failed to set h245Tunneling \n",fn));	
			}
		}
		cmThreadDetach(UH323Globals()->hApp,pthread_self());
	}

	return 1;
}

int UH323AllocatePorts(void)
{
#define ASSIGN_PORT(port)  if ( (port) == 0 ) \
			{ 	if ( startingport == 1727 )	startingport = 1730; \
				(port) = startingport; \
				startingport++; }

	int startingport, i;

	h323PortArray = (struct h323Ports_tag *)malloc( nh323Instances * sizeof(struct h323Ports_tag));
	if ( h323PortArray == NULL )
	{
		NETERROR(MH323, ("H.323 failed to llocate ports\n"));
		return -1;
	}
	if ( nh323Instances == 1 )
	{
		h323PortArray[0].mPort = 1718;
		h323PortArray[0].rasPort = 1719;
		h323PortArray[0].q931Port = 1720;
	}
	else
	{
		/* clean to 0 first */
		memset( h323PortArray, 0, nh323Instances * sizeof(struct h323Ports_tag) );
		/* the I-0 instance q931 port must be 1720 */
		h323PortArray[0].q931Port = 1720;
		/* the ARQ instance mPort is 1718, rasPort must be 1719 */
		h323PortArray[ARQ_MINSTANCE_ID].mPort = 1718;
		h323PortArray[ARQ_MINSTANCE_ID].rasPort = 1719;
		/* the sgk instance ports are fixed for 1727, 1728, 1729 */
		h323PortArray[SGK_MINSTANCE_ID].mPort = 1727;
		h323PortArray[SGK_MINSTANCE_ID].rasPort = 1728;
		h323PortArray[SGK_MINSTANCE_ID].q931Port = 1729;
		
		/* assign whatever port number for reset of them */
		startingport = 1721;
		for( i = 0; i < nh323Instances; i++ )
		{
			ASSIGN_PORT(h323PortArray[i].mPort);
			ASSIGN_PORT(h323PortArray[i].rasPort); 
			ASSIGN_PORT(h323PortArray[i].q931Port);
		}
	}
	return 0;
}


int
GetVendorFromH221Info(h221VendorInfo *ns)
{
	if (ns->t35CountryCode == 181)
	{
		switch (ns->manufacturerCode)
		{
		case 18:
			return Vendor_eCisco;
		case 20:
			return Vendor_eLucentTnt;
		case 23:
			return Vendor_eVocalTec;
		}
	}

	return -1;
}
