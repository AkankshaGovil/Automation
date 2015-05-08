#include <stdio.h>
#include <string.h>
#include <netinet/in.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>
#include "utils.h"
//#include "AutoDownload.h"
#include "BridgeServer.h"
#include "ProcessManagerClient.h"
#include "cli.h"
#include "systemlog.h"
#include "pids.h"
#include "pmpoll.h"
#include "lsconfig.h"
#include "protos.h"
#include "serverp.h"


extern int ProcessCommand(int, char **);
extern int CacheAttach (void);
extern int CacheDetach (void);
extern int SHM_Init (int);
extern int CacheInit (void);
extern void setConfigFile (void);
extern int DoConfig (int (*)(void));
extern int utilsIdInit (JNIEnv*);
extern int iserverConfigIdInit (JNIEnv*, jobject);
extern int iserverCapInit (JNIEnv*, jobject);
extern void setRestart (int);
extern int nlm_getUsedvportNolock (void);
extern int nlm_getUsedMRvportNolock (void);
extern int DbExtractCount(char*, int);

extern short cliLibFlags;

typedef struct {
  const char *vpnid;
  const char *vpnnam;
  const char *vpngrp;
  const char *vpnext;
  const char *firstName;
  const char *lastName;
  const char *location;
  const char *country;
  const char *comments;
  const char *customerId;
  const char *trunkGroup;
  const char *zone;
  const char *phone;
  const char *email;
  const char *forwardedPhone;
  jboolean callforward;
  jboolean isrollover;	
  int extLen;
  int devType;
  const char *vpnloc;
  const char *vpncon;
  jboolean	h323Enable;
  const	char *h323Id;
  jboolean	sipEnable;
  const	char *sipUri;
  const	char *sipContact;
  int	maxCalls;
  jboolean isGateway;
  jboolean grqEnable;
  jboolean raiEnable;
  jboolean techPrefixEnable;
  const char *techPrefix;
  const char *peerGkId;
  int priority;
  int rasPort;
  int q931Port;
  int callpartyType;
  int vendor;
  unsigned long subnetip;
  unsigned long subnetmask;
  const char *cpName;
  const char *h235Pass;

  /* FCE_REMOVED */
  jboolean mediaRouting;
  jboolean hideAddressChange;

  int maxHunts;
  int layer1Protocol;
} PData;

typedef struct {

  unsigned long port;
  unsigned long ipaddr;
  int extLen;
  int forwardedVpnExtLen;
  int devType;
  int maxCalls;
  int maxInCalls;
  int maxOutCalls;
  int currentCalls;
  int priority;
  int rasPort;
  int q931Port;
  int callpartyType;
  int vendor;
  int maxHunts;
  int extCaps;
  unsigned long subnetip;
  unsigned long subnetmask;
  unsigned long inceptionTime;
  unsigned long refreshTime;
  const char *sno;
  const char vpnName[VPNS_ATTR_LEN];
  const char *extNumber;
  const char *phone;
  const char *firstName;
  const char *lastName;
  const char *location;
  const char *country;
  const char *comments;
  const char *customerId;
  const char *trunkGroup;
  const char *ogp;
  const char *cpName;
  const char *zone;
  const char *email;
  const char *forwardedPhone;
  const	char *h323Id;
  const	char *sipUri;
  const	char *sipContact;
  const char *forwardedVpnPhone;
  const char *techPrefix;
  const char *peerGkId;
  const char *h235Pass;
  jboolean proxyValid;
  jboolean isProxied;
  jboolean isProxying;
  jboolean callForwarded;
  jboolean isCallRollover;

  short   caps;
  int sflags;
  int layer1Protocol;
  int infoTransCap;
  const char *srcIngressTg;
  const char *igrpName;
  const char *dtg;
  const char *newsrcdtg;
  const char *realmName;
  int natIp;
  int natPort
} IedgeListData;

typedef struct {
  const char *crname;
  const char *src;
  int srcLen;
  const char *dest;
  int destLen;
  const char *dstPrefix;
  const char *srcPrefix;
  int crflags;
  unsigned long refreshTime;
} CallPlanRouteData;


char xleString[xleMax+1][64] = {
  "Operation Successful",
  "Undefined Error",
  "No Such Entry",
  "Operation Not Permitted",
  "Access Denied",
  "System I/O failed",
  "Duplicate Entry Exists",
  "Invalid Arguments",
  "Insufficient Arguments",
  "Incompatible VPN Parameters",
  "License Limit Reached",

  "Unclassified error",   /* corresponds to error codes >= xleMax */
};

#define LAST_COMMAND_BUF_SIZE     1024
static char lastCommand[LAST_COMMAND_BUF_SIZE] = {0};

static char vpnId[PHONE_NUM_LEN];
static char vpnGroup[VPN_GROUP_LEN];
static char vpnName[VPNS_ATTR_LEN];
static char vpnLoc[VPNS_ATTR_LEN];
static char vpnContact[VPNS_ATTR_LEN];

static jobject	sendObject	=	NULL;
static JNIEnv*	jniEnv		=	NULL;	
static char presence[PHONE_NUM_LEN];
static char *isTraceRoute  =   "false";

static jclass cprClass = NULL;
static jmethodID cprConstId = NULL;

static jclass pdClass = NULL;
static jmethodID pdConstId = NULL;

static jclass ilClass = NULL;
static jmethodID ilConstId = NULL;
static jmethodID ilConstId1 = NULL;
static jfieldID  iEdgeMaxCallsFid  = NULL;
static jfieldID  iEdgeCurCallsFid  = NULL;


static jclass sliClass = NULL;
static jmethodID sliSendMid = NULL;

static jclass routeDataClass = NULL;
static jmethodID routeDataConstId = NULL;

static jmethodID cprResetMid = NULL;

time_t   licenseVportAlarms[MAX_LS_ALARM];
time_t   licenseMRVportAlarms[MAX_LS_ALARM];



/* return strings for error codes */
char* GetErrorString (int c) {
  int code = abs(c);

  if (code > xleMax)
    code = xleMax;
  return xleString[code];
}


/* get information from the database and store it in a local structure */
static int
CopyDbInfoEntry (NetoidInfoEntry *infoEntry, ClientAttribs *clAttribs, PData *pd) {
  VpnEntry vpnEntry, *tmp;

  /* phone number */
  if (BIT_TEST(infoEntry->sflags | infoEntry->dflags, ISSET_PHONE))
    pd->phone = infoEntry->phone;
  else
    pd->phone = "";

  /* vpn name */
  pd->vpnnam = infoEntry->vpnName?infoEntry->vpnName:"";
	  
  /* vpn group */
  memset(&vpnEntry, 0, sizeof(VpnEntry));
  strcpy(vpnEntry.vpnName, infoEntry->vpnName);

  tmp = DbFindVpnEntry(GDBMF(&defCommandData[0], DB_eVpns), (char *)&vpnEntry, sizeof(VpnKey));

  if (tmp != NULL) {
    memset(vpnId, 0, VPN_LEN);
    strcpy(vpnId, tmp->vpnId);
    pd->vpnid = vpnId;

    memset(vpnGroup, 0, VPN_GROUP_LEN);

    if(strlen(tmp->vpnGroup))
      strcpy(vpnGroup, tmp->vpnGroup);
    else
      strcpy(vpnGroup, "");
    pd->vpngrp = vpnGroup;

    memset(vpnLoc, 0, VPNS_ATTR_LEN);

    if(strlen(tmp->vpnLoc))
      strcpy(vpnLoc, tmp->vpnLoc);
    else
      strcpy(vpnLoc, "");
		 
    pd->vpnloc = vpnLoc;

    memset(vpnContact, 0, VPNS_ATTR_LEN);

    if(strlen(tmp->vpnContact))
      strcpy(vpnContact, tmp->vpnContact);
    else
      strcpy(vpnContact, "");

    pd->vpncon = vpnContact;

    free(tmp);
  } else {
    pd->vpngrp = "";
    pd->vpnid = "";
    pd->vpnloc = "";
    pd->vpncon = "";
  }

  /* vpn ext */
  pd->vpnext = infoEntry->vpnPhone?infoEntry->vpnPhone:"";

  /* first name */
  pd->firstName = (clAttribs && clAttribs->clFname)?clAttribs->clFname:"";

  /* last name */
  pd->lastName = (clAttribs && clAttribs->clLname)?clAttribs->clLname:"";

  /* location */
  pd->location = (clAttribs && clAttribs->clLoc)?clAttribs->clLoc:"";

  /* country */
  pd->country = (clAttribs && clAttribs->clCountry)?clAttribs->clCountry:"";

  /* comments */
  pd->comments = (clAttribs && clAttribs->clComments)?clAttribs->clComments:"";

  /* customer ID */
  pd->customerId = infoEntry->custID?infoEntry->custID:"";

  /* trunk group */
  pd->trunkGroup = infoEntry->tg?infoEntry->tg:"";

  /* zone */
  pd->zone = infoEntry->zone?infoEntry->zone:"";

  /* email */
  pd->email = infoEntry->email?infoEntry->email:"";

  /* vpn extension length */
  pd->extLen = (infoEntry->vpnExtLen);

  /* device type */
  pd->devType = (infoEntry->ispcorgw);

  /* call forwarding details */
  if (infoEntry->stateFlags & CL_FORWARD) {
    pd->callforward = JNI_TRUE;
    if ((infoEntry->protocol	==	NEXTONE_REDIRECT_ROLLOVER))
      pd->isrollover = JNI_TRUE;
    else
      pd->isrollover = JNI_FALSE;
    if (BIT_TEST(infoEntry->nsflags, ISSET_PHONE)){
      pd->forwardedPhone  = infoEntry->nphone;
    }
    else
      pd->forwardedPhone = "";
  } else{
    pd->callforward = JNI_FALSE;
    pd->forwardedPhone = "";
  }

  pd->h323Enable = (BIT_TEST(infoEntry->cap, CAP_H323))?JNI_TRUE:JNI_FALSE;

  pd->h323Id = infoEntry->h323id?infoEntry->h323id:"";

  pd->sipEnable = (BIT_TEST(infoEntry->cap, CAP_SIP))?JNI_TRUE:JNI_FALSE;

  pd->sipUri = infoEntry->uri?infoEntry->uri:"";

  pd->sipContact = infoEntry->contact?infoEntry->contact:"";

  pd->maxCalls = infoEntry->xcalls;

  pd->isGateway = IsGateway(infoEntry)?JNI_TRUE:JNI_FALSE;

  pd->grqEnable = (BIT_TEST(infoEntry->cap, CAP_GRQ))?JNI_TRUE:JNI_FALSE;

  pd->raiEnable = (BIT_TEST(infoEntry->cap, CAP_RAI))?JNI_TRUE:JNI_FALSE;

  pd->techPrefixEnable = (BIT_TEST(infoEntry->cap, CAP_TPG))?JNI_TRUE:JNI_FALSE;

  pd->techPrefix = infoEntry->techprefix?infoEntry->techprefix:"";

  pd->peerGkId = infoEntry->pgkid?infoEntry->pgkid:"";

  pd->priority = infoEntry->priority;

  pd->rasPort = infoEntry->rasport;

  pd->q931Port = infoEntry->callsigport;

  pd->callpartyType = infoEntry->q931IE[Q931IE_CDPN];

  pd->vendor = infoEntry->vendor;

  pd->subnetip = infoEntry->subnetip;

  pd->subnetmask = infoEntry->subnetmask;

  pd->cpName = infoEntry->cpname?infoEntry->cpname:"";

  pd->h235Pass = infoEntry->passwd?infoEntry->passwd:"";

#if FCE_REMOVED
  pd->mediaRouting = IsMediaRoutingEnabled(infoEntry)?JNI_TRUE:JNI_FALSE;

  pd->hideAddressChange = IsHideAddressChangeEnabled(infoEntry)?JNI_TRUE:JNI_FALSE;
#endif

  pd->maxHunts = infoEntry->maxHunts;
  pd->layer1Protocol  = infoEntry->bcap[0];

  return 0;
}


JNIEXPORT jobject JNICALL
Java_com_nextone_JServer_BridgeServer_findIedge (JNIEnv *env,
						 jobject obj,
						 jstring sno,
						 jint p) {
  jobject result = NULL;
  const char *serial;
  NetoidSNKey key;
  NetoidInfoEntry *netInfo = NULL;
  ClientAttribs *clAttribs = NULL;
  short port;
  PData pd = {0};
  jstring vpnname, vpnid, vpngrp, vpnext, firstName, lastName, location, country, comments, zone, phone, email, vloc, vcon, fwdphone, h323Id, sipUri, sipContact, techPrefix, peerGkId, cpName, h235Pass, customerId, trunkGroup;

  memset(&key, 0, sizeof(key));

  serial = (*env)->GetStringUTFChars(env, sno, NULL);
  if (serial == NULL)
    return NULL;    /* OutOfMemoryError will be thrown */

  if (strlen(serial) >= REG_ID_LEN) {
    (*env)->ReleaseStringUTFChars(env, sno, serial);
    ThrowBridgeException(env, "Registration ID length exceeds maximum allowed value");
    return NULL;
  }

  /* make sure we have enough memory to operate */
  if ((*env)->PushLocalFrame(env, 64)) {
    (*env)->ReleaseStringUTFChars(env, sno, serial);
    return NULL;  /* OutOfMemoryError will be thrown */
  }

  /* find the iedge */
  port = p;
  strcpy(key.regid, serial);
  key.uport = (port);

  if (OpenDatabases(&defCommandData[0]) < 0) {
    (*env)->ReleaseStringUTFChars(env, sno, serial);
    /* throw a BridgeException */
    ThrowBridgeException(env, "Error opening databases");
    return (*env)->PopLocalFrame(env, NULL);
  }

  netInfo = DbFindInfoEntry(GDBMF(&defCommandData[0], DB_eNetoids), 
			    (char *)&key, sizeof(key));

  if (netInfo != NULL) {
    LOCAL_DEBUG(JNI_FALSE, DEBUG_VERBOSE, ("lookup success for regid:%s port:%d\n", serial, (int)p));

    clAttribs = DbFindAttrEntry(GDBMF(&defCommandData[0], DB_eAttribs), 
				(char *)&key, sizeof(key));
    if (clAttribs == NULL) {
      (*env)->ReleaseStringUTFChars(env, sno, serial);
      free(netInfo);
      CloseDatabases(&defCommandData[0]);
      ThrowBridgeException(env, "Database format error (DB_eNetoids/clAttribs)");
      return (*env)->PopLocalFrame(env, NULL);
    }

    LOCAL_DEBUG(JNI_FALSE, DEBUG_VERBOSE, ("Forwarded flags = %d \t phone = %s\n",netInfo->nsflags,netInfo->nphone));

    /* get all the information from the database */
    if (CopyDbInfoEntry(netInfo, clAttribs, &pd)) {
      (*env)->ReleaseStringUTFChars(env, sno, serial);
      free(netInfo);
      free(clAttribs);
      CloseDatabases(&defCommandData[0]);
      ThrowBridgeException(env, "Database format error (DB_eNetoids)");
      return (*env)->PopLocalFrame(env, NULL);
    }

    LOCAL_DEBUG(JNI_FALSE, DEBUG_VERBOSE, ("Vpn Id: %s\n", pd.vpnid));
    LOCAL_DEBUG(JNI_FALSE, DEBUG_VERBOSE, ("Vpn Name: %s\n", pd.vpnnam));
    LOCAL_DEBUG(JNI_FALSE, DEBUG_VERBOSE, ("Vpn Group: %s\n", pd.vpngrp));
    LOCAL_DEBUG(JNI_FALSE, DEBUG_VERBOSE, ( "Ext: %s\n", pd.vpnext));
    LOCAL_DEBUG(JNI_FALSE, DEBUG_VERBOSE, ("First Name: %s\n", pd.firstName));
    LOCAL_DEBUG(JNI_FALSE, DEBUG_VERBOSE, ("Last Name: %s\n", pd.lastName));
    LOCAL_DEBUG(JNI_FALSE, DEBUG_VERBOSE, ("Location: %s\n", pd.location));
    LOCAL_DEBUG(JNI_FALSE, DEBUG_VERBOSE, ("Country: %s\n", pd.country));
    LOCAL_DEBUG(JNI_FALSE, DEBUG_VERBOSE, ("Comments: %s\n", pd.comments));
    LOCAL_DEBUG(JNI_FALSE, DEBUG_VERBOSE, ("Customer Id: %s\n", pd.customerId));
    LOCAL_DEBUG(JNI_FALSE, DEBUG_VERBOSE, ("Trunk Group: %s\n", pd.trunkGroup));
    LOCAL_DEBUG(JNI_FALSE, DEBUG_VERBOSE, ("Zone: %s\n", pd.zone));
    LOCAL_DEBUG(JNI_FALSE, DEBUG_VERBOSE, ("Phone: %s\n", pd.phone));
    LOCAL_DEBUG(JNI_FALSE, DEBUG_VERBOSE, ("Email: %s\n", pd.email));
    LOCAL_DEBUG(JNI_FALSE, DEBUG_VERBOSE, ("Forwarded phone: %s\n", pd.forwardedPhone));
    LOCAL_DEBUG(JNI_FALSE, DEBUG_VERBOSE, ("Extension length: %d\n", pd.extLen));
    LOCAL_DEBUG(JNI_FALSE, DEBUG_VERBOSE, ("Device type: %d\n", pd.devType));
    LOCAL_DEBUG(JNI_FALSE, DEBUG_VERBOSE, ("Vpn Location: %s\n", pd.vpnloc));

    /* extract info from the structure and set them in the local vars */
    vpnname = (*env)->NewStringUTF(env, pd.vpnnam);
    if (vpnname == NULL)
      goto cleanup;

    /* extract info from the structure and set them in the local vars */
    vpnid = (*env)->NewStringUTF(env, pd.vpnid);
    if (vpnid == NULL)
      goto cleanup;

    vpngrp = (*env)->NewStringUTF(env, pd.vpngrp);
    if (vpngrp == NULL)
      goto cleanup;

    vpnext = (*env)->NewStringUTF(env, pd.vpnext);
    if (vpnext == NULL)
      goto cleanup;

    firstName = (*env)->NewStringUTF(env, pd.firstName);
    if (firstName == NULL)
      goto cleanup;

    lastName = (*env)->NewStringUTF(env, pd.lastName);
    if (lastName == NULL)
      goto cleanup;

    location = (*env)->NewStringUTF(env, pd.location);
    if (location == NULL)
      goto cleanup;

    country = (*env)->NewStringUTF(env, pd.country);
    if (country == NULL)
      goto cleanup;

    comments = (*env)->NewStringUTF(env, pd.comments);
    if (comments == NULL)
      goto cleanup;

    customerId = (*env)->NewStringUTF(env, pd.customerId);
    if (customerId == NULL)
      goto cleanup;

    trunkGroup = (*env)->NewStringUTF(env, pd.trunkGroup);
    if (trunkGroup == NULL)
      goto cleanup;

    zone = (*env)->NewStringUTF(env, pd.zone);
    if (zone == NULL)
      goto cleanup;

    email = (*env)->NewStringUTF(env, pd.email);
    if (email == NULL)
      goto cleanup;

    vloc = (*env)->NewStringUTF(env, pd.vpnloc);
    if (vloc == NULL)
      goto cleanup;

    vcon = (*env)->NewStringUTF(env, pd.vpncon);
    if (vcon == NULL)
      goto cleanup;

    fwdphone = (*env)->NewStringUTF(env, pd.forwardedPhone);
    if (fwdphone == NULL)
      goto cleanup;

    phone = (*env)->NewStringUTF(env, pd.phone);
    if (phone == NULL)
      goto cleanup;

    h323Id= (*env)->NewStringUTF(env, pd.h323Id);
    if (h323Id == NULL)
      goto cleanup;

    sipUri = (*env)->NewStringUTF(env, pd.sipUri);
    if (sipUri == NULL)
      goto cleanup;

    sipContact = (*env)->NewStringUTF(env, pd.sipContact);
    if (sipContact == NULL)
      goto cleanup;

    techPrefix = (*env)->NewStringUTF(env, pd.techPrefix);
    if (techPrefix == NULL)
      goto cleanup;

    peerGkId = (*env)->NewStringUTF(env, pd.peerGkId);
    if (peerGkId == NULL)
      goto cleanup;

    cpName = (*env)->NewStringUTF(env, pd.cpName);
    if (cpName == NULL)
      goto cleanup;

    h235Pass = (*env)->NewStringUTF(env, pd.h235Pass);
    if (h235Pass == NULL)
      goto cleanup;

    /* construct the new ProvisionData object */
    result = (*env)->NewObject(env, pdClass, pdConstId, vpnname, vpnid, vpngrp, vpnext, firstName, lastName, location, country, comments, customerId, zone,  email, pd.extLen, pd.devType, vloc, vcon, fwdphone, phone, pd.callforward, pd.isrollover, pd.h323Enable, h323Id,pd.callpartyType, pd.sipEnable, sipUri, sipContact, (jint)pd.maxCalls, pd.isGateway, pd.grqEnable, pd.raiEnable, (jint)pd.priority, (jint)pd.rasPort, (jint)pd.q931Port, pd.techPrefixEnable, techPrefix, peerGkId, (jint)pd.vendor, (jint)pd.subnetip, (jint)pd.subnetmask, cpName, h235Pass, pd.mediaRouting, pd.hideAddressChange, pd.maxHunts, pd.layer1Protocol, trunkGroup);
  } else {
    LOCAL_DEBUG(JNI_FALSE, DEBUG_VERBOSE, ("lookup failed for regid-%s port-%d\n", serial, (int)p));
  }

 cleanup:
  (*env)->ReleaseStringUTFChars(env, sno, serial);
  checkFree(netInfo);
  checkFree(clAttribs);
  CloseDatabases(&defCommandData[0]);
  return (*env)->PopLocalFrame(env, result);
}


static int
ExtractDbInfoEntry (JNIEnv *env,
		    NetoidInfoEntry *infoEntry,
		    ClientAttribs *clAttribs,
		    IedgeListData *ld) {

  //   PrintDbAttrEntry(stdout, clAttribs);
  //   fflush(stdout);

  /* registration id */
  if (BIT_TEST(infoEntry->sflags, ISSET_REGID))
    ld->sno = infoEntry->regid;
  else
    ld->sno = "";

  /* port number */
  if (BIT_TEST(infoEntry->sflags, ISSET_UPORT))
    ld->port = (infoEntry->uport);
  else
    ld->port = -1;

  /* inception time */
  ld->inceptionTime = (infoEntry->iTime);
 /* refresh time */
  ld->refreshTime = (infoEntry->rTime);

  /* phone number */
  if (BIT_TEST(infoEntry->sflags | infoEntry->dflags, ISSET_PHONE))
    ld->phone = infoEntry->phone;
  else
    ld->phone = "";

  if (strlen(infoEntry->vpnPhone) < (infoEntry->vpnExtLen)) {
    LOCAL_DEBUG(JNI_FALSE, DEBUG_ERROR, ("vpn phone check error\n"));
    return -1;
  }

  /* vpn name */
  memset((void*)ld->vpnName, 0, sizeof(vpnName));
  if (strlen(infoEntry->vpnName) )
    strcpy((char*)ld->vpnName, (char*)infoEntry->vpnName);
  else
    strcpy((char*)ld->vpnName, "");


  /* vpn ext */
  if(infoEntry->vpnPhone	!=  NULL)
    ld->extNumber	=	infoEntry->vpnPhone;
  else
    ld->extNumber	=	"";

  /* ip address */
  if (BIT_TEST(infoEntry->sflags | infoEntry->dflags, ISSET_IPADDRESS))
    ld->ipaddr = (infoEntry->ipaddress.l);
  else
    ld->ipaddr = 0;

  if (clAttribs == NULL) {
    ld->firstName = "";
    ld->lastName = "";
    ld->location = "";
    ld->country = "";
    ld->comments = "";
  } else {
    /* first name */
    if (clAttribs->clFname != NULL )
      ld->firstName = clAttribs->clFname;
    else
      ld->firstName = "";

    /* last name */
    if (clAttribs->clLname != NULL)
      ld->lastName = clAttribs->clLname;
    else
      ld->lastName = "";

    /* location */
    if (clAttribs->clLoc != NULL)
      ld->location = clAttribs->clLoc;
    else
      ld->location = "";

    /* country */
    if (clAttribs->clCountry != NULL)
      ld->country = clAttribs->clCountry;
    else
      ld->country = "";

    /* comments */
    if (clAttribs->clComments != NULL)
      ld->comments = clAttribs->clComments;
    else
      ld->comments = "";
  }

  ld->customerId = infoEntry->custID?infoEntry->custID:"";

  ld->trunkGroup = infoEntry->tg?infoEntry->tg:"";

  ld->ogp = infoEntry->ogprefix[0]?infoEntry->ogprefix:"";

  /* calling plan name */
  if (infoEntry->cpname != NULL)
    ld->cpName = infoEntry->cpname;
  else
    ld->cpName = "";

  /* zone */
  if (infoEntry->zone != NULL)
    ld->zone = infoEntry->zone;
  else
    ld->zone = "";

  /* email */
  if (infoEntry->email != NULL)
    ld->email = infoEntry->email;
  else
    ld->email = "";

  /* srcIngressTg */
  if (infoEntry->srcIngressTG != NULL)
    ld->srcIngressTg = infoEntry->srcIngressTG;
  else
    ld->srcIngressTg = "";

  /* realmname */
  if (infoEntry->realmName != NULL)
    ld->realmName = infoEntry->realmName;
  else
    ld->realmName = "";

  /* igrpName */
  if (infoEntry->igrpName != NULL)
    ld->igrpName = infoEntry->igrpName;
  else
    ld->igrpName = "";

  /* dtg */
  if (infoEntry->dtg != NULL)
    ld->dtg = infoEntry->dtg;
  else
    ld->dtg = "";

  /* newsrcdtg */
  if (infoEntry->srcEgressTG != NULL)
    ld->newsrcdtg = infoEntry->srcEgressTG;
  else
    ld->newsrcdtg = "";

  /* vpn extension length */
  ld->extLen = (infoEntry->vpnExtLen);

  /* proxy information */
  if (infoEntry->stateFlags & CL_PROXY) {
    ld->proxyValid = JNI_TRUE;
    if ((infoEntry->stateFlags & (CL_PROXY|CL_PROXYING)) == 
	(CL_PROXY|CL_PROXYING))
      ld->isProxied = JNI_TRUE;
    else
      ld->isProxied = JNI_FALSE;
  } else
    ld->proxyValid = JNI_FALSE;
  if ((infoEntry->stateFlags & (CL_PROXY|CL_PROXYING)) == CL_PROXYING)
    ld->isProxying = JNI_TRUE;
  else
    ld->isProxying = JNI_FALSE;

  /* call forwarding details */
  if (infoEntry->stateFlags & CL_FORWARD) {
    ld->callForwarded = JNI_TRUE;
    if ((infoEntry->protocol	==	NEXTONE_REDIRECT_ROLLOVER))
      ld->isCallRollover = JNI_TRUE;
    else
      ld->isCallRollover = JNI_FALSE;
    if (BIT_TEST(infoEntry->nsflags, ISSET_PHONE))
      ld->forwardedPhone = infoEntry->nphone;
    else
      ld->forwardedPhone = "";
    if (BIT_TEST(infoEntry->nsflags, ISSET_VPNPHONE)) {
      ld->forwardedVpnPhone = infoEntry->nvpnPhone;
      ld->forwardedVpnExtLen = (infoEntry->nvpnExtLen);
    } else {
      ld->forwardedVpnPhone = "";
      ld->forwardedVpnExtLen = -1;
    }
  } else
    ld->callForwarded = JNI_FALSE;

  /* some more details */

  ld->caps  = infoEntry->cap;

  ld->devType = infoEntry->ispcorgw;

  if (infoEntry->h323id != NULL) {
    ld->h323Id = infoEntry->h323id;
  }

  if (infoEntry->uri != NULL) {
    ld->sipUri = infoEntry->uri;
  }

  if (infoEntry->contact != NULL) {
    ld->sipContact = infoEntry->contact;
  }

  ld->callpartyType = infoEntry->q931IE[Q931IE_CDPN];
  ld->layer1Protocol  = infoEntry->bcap[0];
  ld->maxCalls = infoEntry->xcalls;
  ld->maxInCalls = infoEntry->xcallsIn;
  ld->maxOutCalls = infoEntry->xcallsOut;

  ld->currentCalls = infoEntry->ncalls;

  ld->priority = infoEntry->priority;

  ld->rasPort = infoEntry->rasport;

  ld->q931Port = infoEntry->callsigport;


  if (infoEntry->techprefix != NULL)
    ld->techPrefix = infoEntry->techprefix;

  if (infoEntry->pgkid != NULL)
    ld->peerGkId = infoEntry->pgkid;

  ld->vendor = infoEntry->vendor;

  ld->subnetip = infoEntry->subnetip;

  ld->subnetmask = infoEntry->subnetmask;

  ld->h235Pass = infoEntry->passwd?infoEntry->passwd:"";

  ld->maxHunts  = infoEntry->maxHunts;
  ld->extCaps   = infoEntry->ecaps1;
  ld->sflags = infoEntry->stateFlags;
  ld->natIp = infoEntry->natIp;
  ld->natPort = infoEntry->natPort;
  return 0;
}


static void
DumpLData (IedgeListData *ld) {
  time_t tmptime;

  printf("RegistrationID: %s\n", ld->sno);
  fflush(stdout);
  printf("Port: %ld\n", ld->port);
  fflush(stdout);
  tmptime = ld->inceptionTime;
  printf("Inception Time: %s", ctime(&tmptime));
  fflush(stdout);
  tmptime = ld->refreshTime;
  printf("Refresh Time: %s", ctime(&tmptime));
  fflush(stdout);
  printf("IP Address: %lx\n", ld->ipaddr);
  fflush(stdout);
  printf("Phone: %s\n", ld->phone);
  fflush(stdout);
  printf("VPN Name: %s\n", ld->vpnName);
  fflush(stdout);
  printf("Name: %s %s\n", ld->firstName, ld->lastName);
  fflush(stdout);
  printf("Location: %s\n", ld->location);
  fflush(stdout);
  printf("Country: %s\n", ld->country);
  fflush(stdout);
  printf("Comments: %s\n", ld->comments);
  fflush(stdout);
  printf("Customer Id: %s\n", ld->customerId);
  fflush(stdout);
  printf("Trunk Group: %s\n", ld->trunkGroup);
  fflush(stdout);
  printf("Calling Plan Name: %s\n", ld->cpName);
  fflush(stdout);
  printf("Zone: %s\n", ld->zone);
  fflush(stdout);
  printf("Email: %s\n", ld->email);
  fflush(stdout);
  printf("Device Type: %d\n", ld->devType);
  fflush(stdout);
}

/*
static jobject
CreateNewIedgeListObject (JNIEnv *env, IedgeListData *ld) {
  jobject  obj;

  //  only maxcalls and cur calls are used in the iview. 
  //  The other fields are set to default value. If you to use other fields
  // update here

  if ((*env)->PushLocalFrame(env, 20))
    return NULL;
  obj = (*env)->NewObject(env, ilClass, ilConstId);
  (*env)->SetIntField(env, obj, iEdgeMaxCallsFid, (jint)ld->maxCalls);
  (*env)->SetIntField(env, obj, iEdgeCurCallsFid, (jint)ld->currentCalls);

  return (*env)->PopLocalFrame(env, obj);
}
*/
static jobject
CreateNewIedgeListObject (JNIEnv *env, IedgeListData *ld) {
  jobject result = NULL;
  jstring jserialNumber;
  jstring jvpnName;
  jstring jextNumber;
  jstring jphone;
  jstring jfirstName;
  jstring jlastName;
  jstring jlocation;
  jstring jcountry;
  jstring jcomments;
  jstring jcustomerId;
  jstring jcallingPlanName;
  jstring jzone;
  jstring jemail;
  jstring jforwardedPhone;
  jstring jh323Id;
  jstring jsipUri;
  jstring jsipContact;
  jstring jforwardedVpnPhone;
  jstring jtechPrefix;
  jstring jpeerGkId;
  jstring jh235Password;
  jstring jtrunkGroup;
  jstring jogp;
  jstring jsrcIngressTg;
  jstring jrealmName;
  jstring jigrpName;
  jstring jdtg;
  jstring jnewsrcdtg;

  // registration id 
  jserialNumber = (*env)->NewStringUTF(env, ld->sno);
  if(jserialNumber  ==  NULL)
    goto cleanup;

  // vpn name 
  jvpnName = (*env)->NewStringUTF(env, ld->vpnName);
  if(jvpnName  ==  NULL)
    goto cleanup;

  // vpn extension 
  jextNumber = (*env)->NewStringUTF(env, ld->extNumber);
  if(jextNumber ==  NULL)
    goto cleanup;

  // phone 
  jphone = (*env)->NewStringUTF(env, ld->phone);
  if(jphone ==  NULL)
    goto cleanup;

  // first name 
  jfirstName = (*env)->NewStringUTF(env, ld->firstName);
  if(jfirstName ==  NULL)
    goto cleanup;

  // last Name 
  jlastName = (*env)->NewStringUTF(env, ld->lastName);
  if(jlastName ==  NULL)
    goto cleanup;

  // location 
  jlocation = (*env)->NewStringUTF(env, ld->location);
  if(jlocation ==  NULL)
    goto cleanup;

  // country 
  jcountry = (*env)->NewStringUTF(env, ld->country);
  if(jcountry ==  NULL)
    goto cleanup;

  // comments 
  jcomments = (*env)->NewStringUTF(env, ld->comments);
  if(jcomments ==  NULL)
    goto cleanup;

  // customer id 
  jcustomerId = (*env)->NewStringUTF(env, ld->customerId);
  if (jcustomerId == NULL)
    goto cleanup;

  // calling plan 
  jcallingPlanName = (*env)->NewStringUTF(env, ld->cpName);
  if(jcallingPlanName ==  NULL)
    goto cleanup;

  // zone 
  jzone = (*env)->NewStringUTF(env, ld->zone);
  if(jzone ==  NULL)
    goto cleanup;

  // email 
  jemail = (*env)->NewStringUTF(env, ld->email);
  if(jemail ==  NULL)
    goto cleanup;


  // call forwarding stuff 

  if (ld->callForwarded == JNI_TRUE) {
    jforwardedPhone     = (*env)->NewStringUTF(env, ld->forwardedPhone);
    if(jforwardedPhone  ==  NULL)
      goto cleanup;
    jforwardedVpnPhone  = (*env)->NewStringUTF(env, ld->forwardedVpnPhone);
    if(jforwardedVpnPhone ==  NULL)
      goto cleanup;
  }else{
    jforwardedPhone     = NULL;
    jforwardedVpnPhone  = NULL;
  }

  jh323Id = (*env)->NewStringUTF(env, ld->h323Id);
  if(jh323Id ==  NULL)
    goto cleanup;

  jsipUri = (*env)->NewStringUTF(env, ld->sipUri);
  if(jsipUri ==  NULL)
    goto cleanup;

  jsipContact = (*env)->NewStringUTF(env, ld->sipContact);
  if(jsipContact ==  NULL)
    goto cleanup;

  jtechPrefix = (*env)->NewStringUTF(env, ld->techPrefix);
  if(jtechPrefix ==  NULL)
    goto cleanup;

  jpeerGkId = (*env)->NewStringUTF(env, ld->peerGkId);
  if(jpeerGkId ==  NULL)
    goto cleanup;

  jh235Password = (*env)->NewStringUTF(env, ld->h235Pass);
  if(jh235Password ==  NULL)
    goto cleanup;

  // trunk group
  jtrunkGroup = (*env)->NewStringUTF(env, ld->trunkGroup);
  if (jtrunkGroup == NULL)
    goto cleanup;

  jogp = (*env)->NewStringUTF(env, ld->ogp);
  if (jogp == NULL)
    goto cleanup;

  // srcIngressTg
  jsrcIngressTg = (*env)->NewStringUTF(env, ld->srcIngressTg);
  if(jsrcIngressTg ==  NULL)
    goto cleanup;

  //realmName 
  jrealmName= (*env)->NewStringUTF(env, ld->realmName);
  if(jrealmName ==  NULL)
    goto cleanup;

  // igrpName
  jigrpName = (*env)->NewStringUTF(env, ld->igrpName);
  if(jigrpName ==  NULL)
    goto cleanup;

  // dtg
  jdtg = (*env)->NewStringUTF(env, ld->dtg);
  if(jdtg ==  NULL)
    goto cleanup;

  // newsrcdtg
  jnewsrcdtg = (*env)->NewStringUTF(env, ld->newsrcdtg);
  if(jnewsrcdtg ==  NULL)
    goto cleanup;

  //DumpLData(ld);
  result = (*env)->NewObject(env, ilClass, ilConstId1,
    (jint)ld->port,
    (jint)ld->ipaddr,
    (jint)ld->extLen,
    (jint)ld->forwardedVpnExtLen,
    (jint)ld->devType,
    (jint)ld->maxCalls,
    (jint)ld->maxInCalls,
    (jint)ld->maxOutCalls,
    (jint)ld->currentCalls,
    (jint)ld->priority,
    (jint)ld->rasPort,
    (jint)ld->q931Port,
    (jint)ld->callpartyType,
    (jint)ld->vendor,
    (jint)ld->subnetip,
    (jint)ld->subnetmask,
    (jint)ld->maxHunts,
    (jint)ld->extCaps,
    (jint)ld->layer1Protocol,
    (jlong)ld->inceptionTime,
    (jlong)ld->refreshTime,
    jserialNumber,
    jvpnName,
    jextNumber,
    jphone,
    jfirstName,
    jlastName,
    jlocation,
    jcountry,
    jcomments,
    jcustomerId,
    jcallingPlanName,
    jzone,
    jemail,
    jforwardedPhone,
    jh323Id,
    jsipUri,
    jsipContact,
    jforwardedVpnPhone,
    jtechPrefix,
    jpeerGkId,
    jh235Password,
    (jboolean)ld->proxyValid,
    (jboolean)ld->isProxied,
    (jboolean)ld->isProxying,
    (jboolean)ld->callForwarded,
    (jboolean)ld->isCallRollover,
    (jshort)ld->caps,
    (jint)ld->sflags,
    jtrunkGroup,
    jogp,
    (jint)ld->infoTransCap,
    jsrcIngressTg,
    jrealmName,
    jigrpName,
    jdtg,
    jnewsrcdtg,
    (jint)ld->natIp,
    (jint)ld->natPort
);

cleanup:
  (*env)->DeleteLocalRef(env, jserialNumber);
  (*env)->DeleteLocalRef(env,  jvpnName);
  (*env)->DeleteLocalRef(env,  jextNumber);
  (*env)->DeleteLocalRef(env,  jphone);
  (*env)->DeleteLocalRef(env,  jfirstName);
  (*env)->DeleteLocalRef(env,  jlastName);
  (*env)->DeleteLocalRef(env,  jlocation);
  (*env)->DeleteLocalRef(env,  jcountry);
  (*env)->DeleteLocalRef(env,  jcomments);
  (*env)->DeleteLocalRef(env,  jcustomerId);
  (*env)->DeleteLocalRef(env,  jcallingPlanName);
  (*env)->DeleteLocalRef(env,  jzone);
  (*env)->DeleteLocalRef(env,  jemail);
  (*env)->DeleteLocalRef(env,  jforwardedPhone);
  (*env)->DeleteLocalRef(env,  jh323Id);
  (*env)->DeleteLocalRef(env,  jsipUri);
  (*env)->DeleteLocalRef(env,  jsipContact);
  (*env)->DeleteLocalRef(env,  jforwardedVpnPhone);
  (*env)->DeleteLocalRef(env,  jtechPrefix);
  (*env)->DeleteLocalRef(env,  jpeerGkId);
  (*env)->DeleteLocalRef(env,  jh235Password);
  (*env)->DeleteLocalRef(env,  jtrunkGroup);
  (*env)->DeleteLocalRef(env,  jogp);
  (*env)->DeleteLocalRef(env,  jsrcIngressTg);
  (*env)->DeleteLocalRef(env,  jrealmName);
  (*env)->DeleteLocalRef(env,  jigrpName);
  (*env)->DeleteLocalRef(env,  jdtg);
  (*env)->DeleteLocalRef(env,  jnewsrcdtg);

  if (result == NULL)
    return NULL;
  return (*env)->PopLocalFrame(env, result);
}
/*
static jobject
CreateNewIedgeListObject (JNIEnv *env, IedgeListData *ld) {
  jobject result = NULL;
  jstring jserialNumber;
  jstring jvpnName;
  jstring jextNumber;
  jstring jfirstName;
  jstring jlastName;
  jstring jlocation;
  jstring jcountry;
  jstring jcomments;
  jstring jcustomerId;
  jstring jtrunkGroup;
  jstring jogp;
  jstring jcallingPlanName;
  jstring jzone;
  jstring jphone;
  jstring jemail;
  jstring jforwardedPhone;
  jstring jforwardedVpnPhone;
  jstring jh323Id;
  jstring jsipUri;
  jstring jsipContact;
  jstring jtechPrefix;
  jstring jpeerGkId;
  jstring jh235Password;
  jstring jsrcIngressTg;
  jstring jigrpName;
  jstring jdtg;
  jstring jnewsrcdtg;

  // registration id 
  jserialNumber = (*env)->NewStringUTF(env, ld->sno);
  if(jserialNumber  ==  NULL)
    goto cleanup;

  // vpn name 
  jvpnName = (*env)->NewStringUTF(env, ld->vpnName);
  if(jvpnName  ==  NULL)
    goto cleanup;

  // vpn extension 
  jextNumber = (*env)->NewStringUTF(env, ld->extNumber);
  if(jextNumber ==  NULL)
    goto cleanup;

  // first name 
  jfirstName = (*env)->NewStringUTF(env, ld->firstName);
  if(jfirstName ==  NULL)
    goto cleanup;

  // last Name 
  jlastName = (*env)->NewStringUTF(env, ld->lastName);
  if(jlastName ==  NULL)
    goto cleanup;

  // location 
  jlocation = (*env)->NewStringUTF(env, ld->location);
  if(jlocation ==  NULL)
    goto cleanup;

  // country 
  jcountry = (*env)->NewStringUTF(env, ld->country);
  if(jcountry ==  NULL)
    goto cleanup;

  // comments 
  jcomments = (*env)->NewStringUTF(env, ld->comments);
  if(jcomments ==  NULL)
    goto cleanup;

  // customer id 
  jcustomerId = (*env)->NewStringUTF(env, ld->customerId);
  if (jcustomerId == NULL)
    goto cleanup;

  // trunk group 
  jtrunkGroup = (*env)->NewStringUTF(env, ld->trunkGroup);
  if (jtrunkGroup == NULL)
    goto cleanup;

  jogp = (*env)->NewStringUTF(env, ld->ogp);
  if (jogp == NULL)
    goto cleanup;

  // calling plan 
  jcallingPlanName = (*env)->NewStringUTF(env, ld->cpName);
  if(jcallingPlanName ==  NULL)
    goto cleanup;

  // zone 
  jzone = (*env)->NewStringUTF(env, ld->zone);
  if(jzone ==  NULL)
    goto cleanup;

  // phone 
  jphone = (*env)->NewStringUTF(env, ld->phone);
  if(jphone ==  NULL)
    goto cleanup;

  // email 
  jemail = (*env)->NewStringUTF(env, ld->email);
  if(jemail ==  NULL)
    goto cleanup;

  // srcIngressTg 
  jsrcIngressTg = (*env)->NewStringUTF(env, ld->srcIngressTg);
  if(jsrcIngressTg ==  NULL)
    goto cleanup;

  // igrpName 
  jigrpName = (*env)->NewStringUTF(env, ld->igrpName);
  if(jigrpName ==  NULL)
    goto cleanup;

  // dtg
  jdtg = (*env)->NewStringUTF(env, ld->dtg);
  if(jdtg ==  NULL)
    goto cleanup;

  // newsrcdtg
  jnewsrcdtg = (*env)->NewStringUTF(env, ld->newsrcdtg);
  if(jnewsrcdtg ==  NULL)
    goto cleanup;

  // call forwarding stuff 

  if (ld->callForwarded == JNI_TRUE) {
    jforwardedPhone     = (*env)->NewStringUTF(env, ld->forwardedPhone);
    if(jforwardedPhone  ==  NULL)
      goto cleanup;
    jforwardedVpnPhone  = (*env)->NewStringUTF(env, ld->forwardedVpnPhone);
    if(jforwardedVpnPhone ==  NULL)
      goto cleanup;
  }else{
    jforwardedPhone     = "";
    jforwardedVpnPhone  = "";
  }

  jh323Id = (*env)->NewStringUTF(env, ld->h323Id);
  if(jh323Id ==  NULL)
    goto cleanup;

  jsipUri = (*env)->NewStringUTF(env, ld->sipUri);
  if(jsipUri ==  NULL)
    goto cleanup;

  jsipContact = (*env)->NewStringUTF(env, ld->sipContact);
  if(jsipContact ==  NULL)
    goto cleanup;

  jtechPrefix = (*env)->NewStringUTF(env, ld->techPrefix);
  if(jtechPrefix ==  NULL)
    goto cleanup;

  jpeerGkId = (*env)->NewStringUTF(env, ld->peerGkId);
  if(jpeerGkId ==  NULL)
    goto cleanup;

  jh235Password = (*env)->NewStringUTF(env, ld->h235Pass);
  if(jh235Password ==  NULL)
    goto cleanup;

  //DumpLData(ld);

  result = (*env)->NewObject(env, ilClass, ilConstId,
    (jint)ld->port,
    (jint)ld->ipaddr,
    (jint)ld->extLen,
    (jint)ld->forwardedVpnExtLen,
    (jint)ld->devType,
    (jint)ld->maxCalls,
    (jint)ld->maxInCalls,
    (jint)ld->maxOutCalls,
    (jint)ld->currentCalls,
    (jint)ld->priority,
    (jint)ld->rasPort,
    (jint)ld->q931Port,
    (jint)ld->callpartyType,
    (jint)ld->vendor,
    (jint)ld->subnetip,
    (jint)ld->subnetmask,
    (jint)ld->maxHunts,
    (jint)ld->extCaps,
    (jint)ld->layer1Protocol,
    (jlong)ld->inceptionTime,
    (jlong)ld->refreshTime,
    jserialNumber,
    jvpnName,
    jextNumber,
    jphone,
    jfirstName,
    jlastName,
    jlocation,
    jcountry,
    jcomments,
    jcustomerId,
    jcallingPlanName,
    jzone,
    jemail,
    jforwardedPhone,
    jh323Id,
    jsipUri,
    jsipContact,
    jforwardedVpnPhone,
    jtechPrefix,
    jpeerGkId,
    jh235Password,
    (jboolean)ld->proxyValid,
    (jboolean)ld->isProxied,
    (jboolean)ld->isProxying,
    (jboolean)ld->callForwarded,
    (jboolean)ld->isCallRollover,
    (jshort)ld->caps,
    (jint)ld->sflags,
    jtrunkGroup,
    jogp,
    (jint)ld->infoTransCap,
    jsrcIngressTg,
    jigrpName,
    jdtg,
    jnewsrcdtg
    );

cleanup:
  (*env)->DeleteLocalRef(env, jserialNumber);
  (*env)->DeleteLocalRef(env,  jvpnName);
  (*env)->DeleteLocalRef(env,  jextNumber);
  (*env)->DeleteLocalRef(env,  jfirstName);
  (*env)->DeleteLocalRef(env,  jlastName);
  (*env)->DeleteLocalRef(env,  jlocation);
  (*env)->DeleteLocalRef(env,  jcountry);
  (*env)->DeleteLocalRef(env,  jcomments);
  (*env)->DeleteLocalRef(env,  jcustomerId);
  (*env)->DeleteLocalRef(env,  jtrunkGroup);
  (*env)->DeleteLocalRef(env,  jogp);
  (*env)->DeleteLocalRef(env,  jcallingPlanName);
  (*env)->DeleteLocalRef(env,  jzone);
  (*env)->DeleteLocalRef(env,  jphone);
  (*env)->DeleteLocalRef(env,  jemail);
  (*env)->DeleteLocalRef(env,  jforwardedPhone);
  (*env)->DeleteLocalRef(env,  jforwardedVpnPhone);
  (*env)->DeleteLocalRef(env,  jh323Id);
  (*env)->DeleteLocalRef(env,  jsipUri);
  (*env)->DeleteLocalRef(env,  jsipContact);
  (*env)->DeleteLocalRef(env,  jtechPrefix);
  (*env)->DeleteLocalRef(env,  jpeerGkId);
  (*env)->DeleteLocalRef(env,  jh235Password);
  (*env)->DeleteLocalRef(env,  jigrpName);
  (*env)->DeleteLocalRef(env,  jdtg);
  (*env)->DeleteLocalRef(env,  jnewsrcdtg);
  
  if (result == NULL)
    return NULL;
  return (*env)->PopLocalFrame(env, result);
}
*/


JNIEXPORT jboolean JNICALL
Java_com_nextone_JServer_MiscServer_processCommand (JNIEnv *env,
						    jobject obj,
						    jint argc,
						    jobjectArray cmds,
						    jobject sli) {
#define ARG_LIMIT   16
  char *argv[ARG_LIMIT] = {0}; /* limited to 8 arguments for now... */
  int i, retcode, devType, len;
  jstring str;
  char* sdevType;
  const char *string;
  jboolean result = JNI_FALSE;
  char error[1024] = {0};

  isTraceRoute   =   "true"; 
  sendObject	=	sli;
  jniEnv	=	env;
  
  if (argc > ARG_LIMIT) {
    LOCAL_DEBUG(JNI_FALSE, DEBUG_OFF, ("Only %d arguments allowed...\n", ARG_LIMIT));
    ThrowBridgeException(env, "Internal error");
    return JNI_FALSE;
  }

  /* check to make sure we have enough memory to operate */
  if ((*env)->EnsureLocalCapacity(env, 20) < 0)
    return JNI_FALSE;  /* OutOfMemoryError will be thrown */

  for (i = 0; i < argc; i++) {
    /* extract strings and assign to argv */
    str = (jstring)(*env)->GetObjectArrayElement(env, cmds, i);
    if (str == NULL) {
      goto cleanup;  /* ArrayIndexOutOfBoundsException thrown */
    }

    string = (*env)->GetStringUTFChars(env, str, NULL);
    if (string == NULL)
      goto cleanup;    /* OutOfMemoryError will be thrown */

    /* make a local array and copy arguments into it */
    argv[i] = (char *)calloc(strlen(string)+1, 1);
    strncpy(argv[i], string, strlen(string));

    (*env)->ReleaseStringUTFChars(env, str, string);
  }


  /* call Process Command */
  // if the command is edit device type, change the device type from integer into string
  if(argc	==	8){
    if( (strcmp(argv[0], "iedge")	==	0) && (strcmp(argv[1], "edit")	==	0) &&
	(strcmp(argv[5], "type")	==	0)
	){
      devType	=	atoi(argv[6]);
      sdevType = IedgeName(devType);
      free(argv[6]);
      argv[6] = (char *)calloc(strlen(sdevType)+1, 1);
      strncpy(argv[6], sdevType, strlen(sdevType));
    }
  }

  for (len = i = 0, memset(lastCommand, 0, LAST_COMMAND_BUF_SIZE); i < argc; i++, len = strlen(lastCommand))
    sprintf(&lastCommand[len], "%s:", argv[i]);
  lastCommand[len] = '\0';
  LOCAL_DEBUG(JNI_TRUE, DEBUG_NORMAL, ("Calling ProcessCommand() - %d:%s", (int)argc, lastCommand));

  //  LOCAL_DEBUG(JNI_FALSE, DEBUG_NORMAL, ("Calling ProcessCommand() - %d", (int)argc));
  //  for (i = 0; i < argc; i++) {
  //    LOCAL_DEBUG(JNI_FALSE, DEBUG_NORMAL, (":%s", argv[i]));
  //  }
  //  LOCAL_DEBUG(JNI_FALSE, DEBUG_NORMAL, ("\n"));

  switch (retcode = ProcessCommand(argc, argv)) {
  case xleOk:  {
    result = JNI_TRUE;
    break;
  }
  case -xleExists:  /* entry already exists */
    strcpy(error, "Error: ");
    strcat(error, GetErrorString(retcode));
    strcat(error, "\n");
    strcat(error, lastCommand);
    LOCAL_DEBUG(JNI_FALSE, DEBUG_VERBOSE, ("entry exists \t Error code = %d\n", retcode));
    ThrowExistException(env, error);
    break;
  case -xleNoEntry:  /* entry no exists */
    strcpy(error, "Error: ");
    strcat(error, GetErrorString(retcode));
    strcat(error, "\n");
    strcat(error, lastCommand);
    LOCAL_DEBUG(JNI_FALSE, DEBUG_VERBOSE, ("entry not exists \t Error code = %d\n", retcode));
    ThrowNoEntryException(env, error);
    break;
  default:   /* some kind of error happened */
    strcpy(error, "Error: ");
    strcat(error, GetErrorString(retcode));
    LOCAL_DEBUG(JNI_FALSE, DEBUG_ERROR, ("%s\t Error code = %d\n", error, retcode));
    ThrowBridgeException(env, error);
  }

 cleanup:
  for (i = 0; i < argc; i++) {
    if (argv[i] != NULL)
      free(argv[i]);
  }

  // last command executed successfully, reset the lastCommand buffer
  memset(lastCommand, 0, LAST_COMMAND_BUF_SIZE);

  return result;
}


JNIEXPORT jboolean JNICALL
Java_com_nextone_JServer_MiscServer_getCallerIdCallRoute (JNIEnv *env,
							  jobject obj,
							  jstring key,
							  jstring srcphone,
							  jstring destphone,
							  jobject sli) {
  const char *srcPhone = NULL;
  const char *destPhone = NULL;
  const char *command = NULL;
  char error[1024] = {0};
  const char *argv[4] = {0};
  int ret;

  isTraceRoute   =   "true"; 

  srcPhone = (*env)->GetStringUTFChars(env, srcphone, NULL);
  if (srcPhone == NULL)
    return JNI_FALSE;    /* OutOfMemoryError will be thrown */

  destPhone = (*env)->GetStringUTFChars(env, destphone, NULL);
  if (destPhone== NULL) {
    (*env)->ReleaseStringUTFChars(env, srcphone, srcPhone);
    return JNI_FALSE;    /* OutOfMemoryError will be thrown */
  }
  command = (*env)->GetStringUTFChars(env, key, NULL);
  if (command == NULL) {
    (*env)->ReleaseStringUTFChars(env, srcphone, srcPhone);
    (*env)->ReleaseStringUTFChars(env, destphone, destPhone);
    return JNI_FALSE;    /* OutOfMemoryError will be thrown */
  }
   
  sendObject	=	sli;
  jniEnv	=	env;
  argv[0]	=	"iedge";
  argv[1]	=	command;
  argv[2]	=	srcPhone;
  argv[3]	=	destPhone;
  LOCAL_DEBUG(JNI_FALSE, DEBUG_NORMAL, ("command iedge  %s %s %s \n",command, srcPhone,destPhone));
  LOCAL_DEBUG(JNI_FALSE, DEBUG_VERBOSE, ("srcPhone = %s  destphone = %s\n", srcPhone, destPhone));
  switch ((ret = ProcessCommand(4, (char **)argv))) {
  case xleOk:  /* new entry added fine */
    LOCAL_DEBUG(JNI_FALSE, DEBUG_VERBOSE, ("listed call routes\n"));
    break;

  default:   /* some kind of error happened */
    LOCAL_DEBUG(JNI_FALSE, DEBUG_ERROR, ("Error in listing call routes ret = %d\n",ret));
    (*env)->ReleaseStringUTFChars(env, srcphone, srcPhone);
    (*env)->ReleaseStringUTFChars(env, destphone, destPhone);
    (*env)->ReleaseStringUTFChars(env, key, command);
    strcpy(error, "Error getting route: ");	
    strcat(error, GetErrorString(ret));
    ThrowBridgeException(env, error);
    return JNI_FALSE;
  }

  (*env)->ReleaseStringUTFChars(env, srcphone, srcPhone);
  (*env)->ReleaseStringUTFChars(env, destphone, destPhone);
  (*env)->ReleaseStringUTFChars(env, key, command);
  return JNI_TRUE;  /* all is well that ends well */
}


JNIEXPORT jboolean JNICALL
Java_com_nextone_JServer_MiscServer_getRegIdCallRoute (JNIEnv *env,
						       jobject obj,
                   jstring key,
						       jstring reg,
						       jint p,
						       jstring destphone,
						       jobject sli) {
  const char *regId = NULL;
  const char *destPhone = NULL;
  const char *command= NULL;
  char portstr[10] = {0};
  char error[1024] = {0};
  const char *argv[5] = {0};
  int ret;

  
  isTraceRoute   =   "true";

  regId = (*env)->GetStringUTFChars(env, reg, NULL);
  if (regId == NULL)
    return JNI_FALSE;    /* OutOfMemoryError will be thrown */

  sprintf(portstr, "%d", (int)p);

  destPhone = (*env)->GetStringUTFChars(env, destphone, NULL);
  if (destPhone== NULL) {
    (*env)->ReleaseStringUTFChars(env, reg, regId);
    return JNI_FALSE;    /* OutOfMemoryError will be thrown */
  }
  command = (*env)->GetStringUTFChars(env, key, NULL);
  if (command == NULL) {
    (*env)->ReleaseStringUTFChars(env, reg, regId);
    (*env)->ReleaseStringUTFChars(env, destphone, destPhone);
    return JNI_FALSE;    /* OutOfMemoryError will be thrown */
  }



  sendObject	=	sli;
  jniEnv	=	env;
  argv[0]	=	"iedge";
  argv[1]	=	command ;
  argv[2]	=	regId;
  argv[3]	=	portstr;
  argv[4]	=	destPhone;
  LOCAL_DEBUG(JNI_FALSE, DEBUG_NORMAL, ("command iedge  %s %s %s %s\n",command, regId,portstr,destPhone));
  LOCAL_DEBUG(JNI_FALSE, DEBUG_VERBOSE, ("regid = %s port = %s destphone = %s\n",regId, portstr,destPhone));
  switch ((ret = ProcessCommand(5, (char **)argv))) {
  case xleOk:  /* new entry added fine */
    LOCAL_DEBUG(JNI_FALSE, DEBUG_VERBOSE, ("listed call routes\n"));
    break;

  default:   /* some kind of error happened */
    LOCAL_DEBUG(JNI_FALSE, DEBUG_ERROR, ("Error in listing call routes\n"));
    (*env)->ReleaseStringUTFChars(env, reg, regId);
    (*env)->ReleaseStringUTFChars(env, destphone, destPhone);
    (*env)->ReleaseStringUTFChars(env, key, command);
    strcpy(error, "Error getting route: ");	
    strcat(error, GetErrorString(ret));
    ThrowBridgeException(env, error);
    return JNI_FALSE;
  }

  (*env)->ReleaseStringUTFChars(env, reg, regId);
  (*env)->ReleaseStringUTFChars(env, destphone, destPhone);
  (*env)->ReleaseStringUTFChars(env, key, command);
  return JNI_TRUE;  /* all is well that ends well */

}


JNIEXPORT jstring JNICALL
Java_com_nextone_JServer_BridgeServer_getPresenceNumber (JNIEnv *env,
							 jobject obj,
							 jstring destphone) {
  const char *destPhone = NULL;
  char error[1024] = {0};
  const char *argv[4] = {0};
  jstring presenceNo   =   NULL;
  int ret;

  isTraceRoute     =   "false"; 

  memset(presence, 0, sizeof(presence));
   
  destPhone = (*env)->GetStringUTFChars(env, destphone, NULL);
  if (destPhone== NULL){
    strcpy(error, "Invalid destination phone number\n");  
    ThrowBridgeException(env, error);    /* OutOfMemoryError will be thrown */
    return NULL;
  }

  jniEnv	=	env;
  argv[0]	=	"iedge";
  argv[1]	=	"route";
  argv[2]	=	"\"*\"";
  argv[3]	=	destPhone;

  LOCAL_DEBUG(JNI_FALSE, DEBUG_VERBOSE, ("presence for the destphone [%s]\n",destPhone));

  switch ((ret = ProcessCommand(4, (char **)argv))) {
  case xleOk:  /* new entry added fine */
    presenceNo  =   (*env)->NewStringUTF(env, presence);	
    LOCAL_DEBUG(JNI_FALSE, DEBUG_VERBOSE, ("listed the current presence number ( %s ).\n",presence));
    break;

  default:   /* some kind of error happened */
    LOCAL_DEBUG(JNI_FALSE, DEBUG_ERROR, ("Error in listing presence number ret = %d\n",ret));
    (*env)->ReleaseStringUTFChars(env, destphone, destPhone);
    strcpy(error, "Error getting route: ");	
    strcat(error, GetErrorString(ret));
    ThrowBridgeException(env, error);
    return JNI_FALSE;
  }

  (*env)->ReleaseStringUTFChars(env, destphone, destPhone);
  return presenceNo;  /* all is well that ends well */
}


int
CliRouteLogFn (RouteNode *routeNode) {
#define BUF_SIZE 2056
  char		buff[BUF_SIZE]  = {0};
//  jshort	srcFlag;
  //jshort	destFlag;
  jstring	details;
//  jstring	forwardReason;
//  jstring	rejectReason;
//  jstring	srcNode;
//  jstring	destNode;
  jthrowable exc;
  jstring excString;
  const char *exception;
  char temp[1042] = {0};
  JNIEnv *env = jniEnv;
  jobject routeOb = NULL;

  if (env == NULL)
    return -1;

  sprintf(temp, "branch:%d   x:%s/%ld  y:%s/%ld  reject:%d  forward:%d", routeNode->branch, routeNode->xphonode.regid, (routeNode->xphonode.uport+1), routeNode->yphonode.regid, (routeNode->yphonode.uport+1), routeNode->rejectReason, routeNode->forwardReason);
  LOCAL_DEBUG(JNI_TRUE, DEBUG_VERBOSE, ("CliRouteLogFn: %s", temp));

  if (strcmp(isTraceRoute,"true")  ==  0) {
    if (sendObject == NULL ) {
      LOCAL_DEBUG(JNI_TRUE, DEBUG_ERROR, ("CliRouteLogFn: sendObject is null"));
      return -1;
    }

    LOCAL_DEBUG(JNI_TRUE, DEBUG_VERBOSE, ("CliRouteLogFn: creating trace route info"));



    strcpy(temp,"");
	  switch (routeNode->branch)
	  {
		  case 1:
			  sprintf(temp,"\t(b2): ");
			  break;
		  case 0:
			  sprintf(temp,"\t(b1): ");
			  break;
		  default:
			  break;
	  }

    strcpy(buff,temp);
	  if (BIT_TEST(routeNode->xphonode.sflags, ISSET_REGID))
	  {
		  sprintf(temp,"%s/%lu ", 
			  routeNode->xphonode.regid,
			  (routeNode->xphonode.uport+1));
      strcat(buff,temp);
	  }

	  if (BIT_TEST(routeNode->xphonode.sflags, ISSET_IPADDRESS))
	  {
		  sprintf(temp,"%s ", 
			  ULIPtostring(routeNode->xphonode.ipaddress.l)); 
      strcat(buff,temp);
	  } 

	  if (BIT_TEST(routeNode->xphonode.sflags, ISSET_PHONE))
	  {
		  sprintf(temp,"%s ", 
			  routeNode->xphonode.phone);
      strcat(buff,temp);
	  }
    strcat(buff,"\n");

	  if (routeNode->crname)
	  {
		  sprintf(temp,"\troute: %s%s", routeNode->crname, routeNode->crflags&CRF_STICKY?" sticky":"");
      strcat(buff,temp);
		  if (routeNode->cpname)
		  {
			  sprintf(temp,"plan: %s ", routeNode->cpname);
        strcat(buff,temp);
		  }
      strcat(buff,"\n");
	  }

	  if ((routeNode->forwardReason > 0) &&
			  (routeNode->forwardReason < nextoneReasonMax))
	  {
		  sprintf(temp,"\tpreferred reason = %s ", 
			  NextoneReasonNames[routeNode->forwardReason]);
      strcat(buff,temp);
	  }

	  if (!routeNode->rejectReason && !routeNode->forwardReason)
	  {
      strcat(buff,"\tfinal");
	  }

	  if ((routeNode->rejectReason > 0) &&
			  (routeNode->rejectReason < nextoneReasonMax))
	  {
		  switch (routeNode->rejectReason)
		  {
		  default:
			  sprintf(temp,"\trejected reason = %s ", 
				  NextoneReasonNames[routeNode->rejectReason]);
        strcat(buff,temp);
			  break;
		  }
	  }
    strcat(buff,"-> ");

	  if (BIT_TEST(routeNode->yphonode.sflags, ISSET_REGID))
	  {
		  sprintf(temp,"%s/%lu ", 
			  routeNode->yphonode.regid,
			  (routeNode->yphonode.uport+1));
      strcat(buff,temp);
	  }

	  if (BIT_TEST(routeNode->yphonode.sflags, ISSET_IPADDRESS))
	  {
		  sprintf(temp,"%s ", 
			  ULIPtostring(routeNode->yphonode.ipaddress.l)); 
      strcat(buff,temp);
	  } 

	  if (!routeNode->rejectReason &&
		  BIT_TEST(routeNode->yphonode.sflags, ISSET_PHONE))
	  {
		  sprintf(temp,"DIAL phone:%s ", 
			  routeNode->yphonode.phone);
      strcat(buff,temp);
	  }

	  strcat(buff,"\n");

	  strcat(buff,"\n");

    details	=	(*jniEnv)->NewStringUTF(jniEnv, buff);	

    routeOb = (*jniEnv)->NewObject(jniEnv, routeDataClass, routeDataConstId, details);
    (*jniEnv)->DeleteLocalRef(jniEnv, details);
    /* now send this object over to the other side */
    LOCAL_DEBUG(JNI_TRUE, DEBUG_VERBOSE, ("CliRouteLogFn: sending trace route info"));
    (*jniEnv)->CallVoidMethod(jniEnv, sendObject, sliSendMid, routeOb);

    /* any exception happened, log here and throw a new bridge exception */
    exc = (*jniEnv)->ExceptionOccurred(jniEnv);
    if (exc != NULL) {
      (*jniEnv)->ExceptionDescribe(jniEnv);
      (*jniEnv)->ExceptionClear(jniEnv);
      (*jniEnv)->DeleteLocalRef(jniEnv, routeOb);

      excString = ExtractExceptionString(jniEnv, exc);
      if (excString == NULL) {
	(*jniEnv)->DeleteLocalRef(jniEnv, exc); 
	return -1;   /* some exception was thrown */
      }

      exception = (*jniEnv)->GetStringUTFChars(jniEnv, excString, NULL);
      if (exception == NULL) {
	(*jniEnv)->DeleteLocalRef(jniEnv, exc);
	(*jniEnv)->DeleteLocalRef(jniEnv, excString);
	return -1;  /* out of memory exception is thrown */
      }

      ThrowBridgeException(jniEnv, exception);
      (*jniEnv)->ReleaseStringUTFChars(jniEnv, excString, exception);
      (*jniEnv)->DeleteLocalRef(jniEnv, excString);
      (*jniEnv)->DeleteLocalRef(jniEnv, exc);
      return -1;
    }

    (*jniEnv)->DeleteLocalRef(jniEnv, routeOb);
    LOCAL_DEBUG(JNI_TRUE, DEBUG_VERBOSE, ("CliRouteLogFn: sent trace route info"));
  } else {
    LOCAL_DEBUG(JNI_TRUE, DEBUG_VERBOSE, ("CliRouteLogFn: creating presence info"));

    if (BIT_TEST(routeNode->yphonode.sflags, ISSET_PHONE))
      strcpy(presence,routeNode->yphonode.phone);
    else
      memset(presence, 0, sizeof(presence));
  }

  return 0;
#undef BUF_SIZE
}



JNIEXPORT jboolean JNICALL
Java_com_nextone_JServer_BridgeServer_processCommand (JNIEnv *env,
						      jobject obj,
						      jint argc,
						      jobjectArray cmds) {
#define ARG_LIMIT   16
  char *argv[ARG_LIMIT] = {0}; /* limited to 8 arguments for now... */
  int i, retcode, devType, len;
  jstring str;
  char* sdevType;
  const char *string;
  jboolean result = JNI_FALSE;
  char error[512] = {0};

  if (argc > ARG_LIMIT) {
    LOCAL_DEBUG(JNI_FALSE, DEBUG_OFF, ("Only %d arguments allowed...\n", ARG_LIMIT));
    ThrowBridgeException(env, "Internal error");
    return JNI_FALSE;
  }

  /* check to make sure we have enough memory to operate */
  if ((*env)->EnsureLocalCapacity(env, 20) < 0)
    return JNI_FALSE;  /* OutOfMemoryError will be thrown */

  for (i = 0; i < argc; i++) {
    /* extract strings and assign to argv */
    str = (jstring)(*env)->GetObjectArrayElement(env, cmds, i);
    if (str == NULL) {
      goto cleanup;  /* ArrayIndexOutOfBoundsException thrown */
    }

    string = (*env)->GetStringUTFChars(env, str, NULL);
    if (string == NULL)
      goto cleanup;    /* OutOfMemoryError will be thrown */

    /* make a local array and copy arguments into it */
    argv[i] = (char *)calloc(strlen(string)+1, 1);
    strncpy(argv[i], string, strlen(string));

    (*env)->ReleaseStringUTFChars(env, str, string);
  }


  /* call Process Command */
  // if the command is edit device type, change the device type from integer into string
  if(argc	==	8){
    if( (strcmp(argv[0], "iedge")	==	0) && (strcmp(argv[1], "edit")	==	0) &&
	(strcmp(argv[5], "type")	==	0)
	){
      devType	=	atoi(argv[6]);
      sdevType = IedgeName(devType);
      free(argv[6]);
      argv[6] = (char *)calloc(strlen(sdevType)+1, 1);
      strncpy(argv[6], sdevType, strlen(sdevType));
    }
  }

  for (len = i = 0, memset(lastCommand, 0, LAST_COMMAND_BUF_SIZE); i < argc; i++, len = strlen(lastCommand))
    sprintf(&lastCommand[len], "%s:", argv[i]);
  lastCommand[len] = '\0';
  LOCAL_DEBUG(JNI_TRUE, DEBUG_NORMAL, ("Calling ProcessCommand() - %d:%s", (int)argc, lastCommand));

  //  LOCAL_DEBUG(JNI_FALSE, DEBUG_NORMAL, ("Calling ProcessCommand() - %d", (int)argc));
  //  for (i = 0; i < argc; i++) {
  //    LOCAL_DEBUG(JNI_FALSE, DEBUG_NORMAL, (":%s", argv[i]));
  //  }
  //  LOCAL_DEBUG(JNI_FALSE, DEBUG_NORMAL, ("\n"));

  switch (retcode = ProcessCommand(argc, argv)) {
  case xleOk:  {
    result = JNI_TRUE;
    break;
  }
  case -xleExists:  /* entry already exists */
    strcpy(error, "Error: ");
    strcat(error, GetErrorString(retcode));
    strcat(error, "\n");
    strcat(error, lastCommand);
    LOCAL_DEBUG(JNI_FALSE, DEBUG_VERBOSE, ("entry exists \t Error code = %d\n", retcode));
    ThrowExistException(env, error);
    break;
  case -xleNoEntry:  /* entry no exists */
    strcpy(error, "Error: ");
    strcat(error, GetErrorString(retcode));
    strcat(error, "\n");
    strcat(error, lastCommand);
    LOCAL_DEBUG(JNI_FALSE, DEBUG_VERBOSE, ("entry not exists \t Error code = %d\n", retcode));
    ThrowNoEntryException(env, error);
    break;
  default:   /* some kind of error happened */
    strcpy(error, "Error: ");
    strcat(error, GetErrorString(retcode));
    LOCAL_DEBUG(JNI_FALSE, DEBUG_ERROR, ("%s\t Error code = %d\n", error, retcode));
    ThrowBridgeException(env, error);
  }

 cleanup:
  for (i = 0; i < argc; i++) {
    if (argv[i] != NULL)
      free(argv[i]);
  }

  // last command executed successfully, reset the lastCommand buffer
  memset(lastCommand, 0, LAST_COMMAND_BUF_SIZE);

  return result;
}

JNIEXPORT jint JNICALL
Java_com_nextone_JServer_BridgeServer_getPhoneNumLen (JNIEnv *env, jobject obj) {
  return PHONE_NUM_LEN;
}


JNIEXPORT jint JNICALL
Java_com_nextone_JServer_BridgeServer_getVpnGroupLen (JNIEnv *env, jobject obj) {
  return VPN_GROUP_LEN;
}


JNIEXPORT jint JNICALL
Java_com_nextone_JServer_BridgeServer_getVpnNumLen (JNIEnv *env, jobject obj) {
  return VPN_LEN;   /* same as PHONE_NUM_LEN */
}


JNIEXPORT jint JNICALL
Java_com_nextone_JServer_BridgeServer_getProcessManagerPollInterval (JNIEnv *env,
								     jobject obj) {
  return POLL_TIME_OUT;
}


JNIEXPORT jboolean JNICALL
Java_com_nextone_JServer_BridgeServer_restartRSD (JNIEnv *env, jobject obj)
{
   int status;

   setRestart(RSD_RESTART);
   status = Sendpoll(NULL);

   LOCAL_DEBUG(JNI_TRUE, DEBUG_WARNING, ("Sent RSD restart request to PM"));
   if (status == -1)
     return JNI_FALSE;
   return JNI_TRUE;
}


JNIEXPORT jboolean JNICALL
Java_com_nextone_JServer_BridgeServer_restartIServer (JNIEnv *env, jobject obj)
{
   int status;

   setRestart(ISERVER_RESTART);
   status = Sendpoll(NULL);

   LOCAL_DEBUG(JNI_TRUE, DEBUG_WARNING, ("Sent iserver restart request to PM"));
   if (status == -1)
     return JNI_FALSE;
   return JNI_TRUE;
}


JNIEXPORT jint JNICALL
Java_com_nextone_JServer_BridgeServer_getVpnNameLen (JNIEnv *env, jobject obj) {
  return VPNS_ATTR_LEN;   
}

JNIEXPORT jint JNICALL
Java_com_nextone_JServer_BridgeServer_getCallingPlanRouteLen (JNIEnv *env, jobject obj) {
  return CALLPLAN_ATTR_LEN;
}

JNIEXPORT jint JNICALL
Java_com_nextone_JServer_BridgeServer_getCallingPlanCount(JNIEnv *env, jobject obj) {

  int count=0;
  LOCAL_DEBUG(JNI_FALSE, DEBUG_VERBOSE, ("Retrieving total number of calling plan records\n"));
//  CacheGetLocks(cpCache, LOCK_READ, LOCK_BLOCK);
//		count = cpCache->nitems;
//  LOCAL_DEBUG(JNI_FALSE, DEBUG_VERBOSE, ("Retrieved calling plan records count = %d\n",count));
//  CacheReleaseLocks(cpCache);
//  return count;

  count = DbExtractCount(CALLPLAN_DB_FILE,DB_eCallPlan);
  LOCAL_DEBUG(JNI_FALSE, DEBUG_VERBOSE, ("Retrieved calling plan records count = %d\n",count));
  return count;
}

JNIEXPORT jint JNICALL
Java_com_nextone_JServer_BridgeServer_getCallingPlanBindingCount(JNIEnv *env, jobject obj) {
  int count=0;
  LOCAL_DEBUG(JNI_FALSE, DEBUG_VERBOSE, ("Retrieving total number of calling plan binding records\n"));
  CacheGetLocks(cpbCache, LOCK_READ, LOCK_BLOCK);
		count = cpbCache->nitems;
  LOCAL_DEBUG(JNI_FALSE, DEBUG_VERBOSE, ("Retrieved calling plan binding records count = %d\n",count));
  CacheReleaseLocks(cpbCache);
  return count;

//    return DbExtractCount(CALLPLANBIND_DB_FILE,DB_eCallPlanBind);
}

JNIEXPORT jint JNICALL
Java_com_nextone_JServer_BridgeServer_getCallRouteCount(JNIEnv *env, jobject obj) {
  int count =0;
  LOCAL_DEBUG(JNI_FALSE, DEBUG_VERBOSE, ("Retrieving calling plan route records \n"));
  CacheGetLocks(cpCache, LOCK_READ, LOCK_BLOCK);
	count = cpCache->nitems;
  LOCAL_DEBUG(JNI_FALSE, DEBUG_VERBOSE, ("Retrieved calling plan route records count = %d\n",count));
  CacheReleaseLocks(cpCache);
 //  count  = DbExtractCount(CALLROUTE_DB_FILE,DB_eCallRoute);
  return count;

}

JNIEXPORT jint JNICALL
Java_com_nextone_JServer_BridgeServer_getVpnGroupCount(JNIEnv *env, jobject obj) {

  int count=0;
  LOCAL_DEBUG(JNI_FALSE, DEBUG_VERBOSE, ("Retrieving total number of vpn group records\n"));
//  CacheGetLocks(cpCache, LOCK_READ, LOCK_BLOCK);
//		count = cpCache->nitems;
//  LOCAL_DEBUG(JNI_FALSE, DEBUG_VERBOSE, ("Retrieved calling plan records count = %d\n",count));
//  CacheReleaseLocks(cpCache);
//  return count;

  count = DbExtractCount(VPNG_DB_FILE,DB_eVpnG);
  LOCAL_DEBUG(JNI_FALSE, DEBUG_VERBOSE, ("Retrieved group records count = %d\n",count));
  return count;
}

JNIEXPORT jint JNICALL
Java_com_nextone_JServer_BridgeServer_getVpnCount(JNIEnv *env, jobject obj) {
  int count=0;
  LOCAL_DEBUG(JNI_FALSE, DEBUG_VERBOSE, ("Retrieving total number of vpn records\n"));
  CacheGetLocks(vpnCache, LOCK_READ, LOCK_BLOCK);
		count = vpnCache->nitems;
  LOCAL_DEBUG(JNI_FALSE, DEBUG_VERBOSE, ("Retrieved vpn records count = %d\n",count));
  CacheReleaseLocks(vpnCache);
  return count;

//    return DbExtractCount(CALLPLANBIND_DB_FILE,DB_eCallPlanBind);
}

JNIEXPORT jint JNICALL
Java_com_nextone_JServer_BridgeServer_getIEdgeCount(JNIEnv *env, jobject obj) {
  int count =0;
  LOCAL_DEBUG(JNI_FALSE, DEBUG_VERBOSE, ("Retrieving IEdge records \n"));
  CacheGetLocks(regCache, LOCK_READ, LOCK_BLOCK);
	count = regCache->nitems;
  LOCAL_DEBUG(JNI_FALSE, DEBUG_VERBOSE, ("Retrieved IEdge records count = %d\n",count));
  CacheReleaseLocks(regCache);
  return count;

}

JNIEXPORT jint JNICALL
Java_com_nextone_JServer_BridgeServer_getTriggerCount(JNIEnv *env, jobject obj) {
  int count =0;
  LOCAL_DEBUG(JNI_FALSE, DEBUG_VERBOSE, ("Retrieving trigger records \n"));
  CacheGetLocks(triggerCache, LOCK_READ, LOCK_BLOCK);
  count  = DbExtractCount(TRIGGER_DB_FILE,DB_eTrigger);
//	count = triggerCache->nitems;
  LOCAL_DEBUG(JNI_FALSE, DEBUG_NORMAL, ("Retrieved trigger records count = %d\n",count));
  CacheReleaseLocks(triggerCache);
  return count;

}

JNIEXPORT jint JNICALL
Java_com_nextone_JServer_BridgeServer_getRealmCount(JNIEnv *env, jobject obj) {
  int count =0;
  LOCAL_DEBUG(JNI_FALSE, DEBUG_VERBOSE, ("Retrieving realm records \n"));
  CacheGetLocks(realmCache, LOCK_READ, LOCK_BLOCK);
	count = realmCache->nitems;
  LOCAL_DEBUG(JNI_FALSE, DEBUG_NORMAL, ("Retrieved realm records count = %d\n",count));
  CacheReleaseLocks(realmCache);
  return count;

}

JNIEXPORT jint JNICALL
Java_com_nextone_JServer_BridgeServer_getVnetCount(JNIEnv *env, jobject obj) {
  int count =0;
  LOCAL_DEBUG(JNI_FALSE, DEBUG_VERBOSE, ("Retrieving vnet records \n"));
  CacheGetLocks(vnetCache, LOCK_READ, LOCK_BLOCK);
	count = vnetCache->nitems;
  LOCAL_DEBUG(JNI_FALSE, DEBUG_NORMAL, ("Retrieved vnet records count = %d\n",count));
  CacheReleaseLocks(vnetCache);
  return count;

}

JNIEXPORT jint JNICALL
Java_com_nextone_JServer_BridgeServer_getIGRPCount(JNIEnv *env, jobject obj) {
  int count =0;
  LOCAL_DEBUG(JNI_FALSE, DEBUG_VERBOSE, ("Retrieving igrp records \n"));
  count  = DbExtractCount(IGRP_DB_FILE, DB_eIgrp);
  LOCAL_DEBUG(JNI_FALSE, DEBUG_NORMAL, ("Retrieved igrp records count = %d\n",count));
  return count;
}

JNIEXPORT jboolean JNICALL
Java_com_nextone_JServer_ProcessManagerClient_initPoll (JNIEnv *env, jobject obj) {
  int status = Initpoll(JSERVER_ID, JSERVER_GID);

  if (status == -1)
    return JNI_FALSE;

  return JNI_TRUE;
}


JNIEXPORT jboolean JNICALL
Java_com_nextone_JServer_ProcessManagerClient_sendPoll (JNIEnv *env, jobject obj) {
  int status = Sendpoll(NULL);

  if (status == -1)
    return JNI_FALSE;

  return JNI_TRUE;
}

JNIEXPORT jint JNICALL
Java_com_nextone_JServer_JServer_getDatabasePrimary (JNIEnv *env, jclass cls) {
  jint result = 0;
  int i;

  if (CacheAttach() > 0)
  {
    if (LockGetLock(&(lsMem->rsdmutex), LOCK_READ, LOCK_BLOCK) == AL_OK)
    {
      for (i = 0; i < lsMem->rsdInfo->count; i++)
      {
	if (lsMem->rsdInfo->records[i].status == RS_MASTER)
	{
	  result = lsMem->rsdInfo->records[i].ipaddr;
	  break;
	}
      }
      LockReleaseLock(&(lsMem->rsdmutex));
    }
    else
    {
      LOCAL_DEBUG(JNI_FALSE, DEBUG_ERROR, ("Error getting lock to read rsd info\n"));
    }
    CacheDetach();
  }
  else
  {
    LOCAL_DEBUG(JNI_FALSE, DEBUG_ERROR, ("unable to attach to cache to read rsd info\n"));
  }

  return result;
}

JNIEXPORT jstring JNICALL
Java_com_nextone_JServer_JServer_getLastNativeCommand (JNIEnv *env, jclass cls) {
  return (*env)->NewStringUTF(env, lastCommand);
}

JNIEXPORT jint JNICALL
Java_com_nextone_JServer_JServer_getPid (JNIEnv *env, jclass cls) {
  return (jint)getpid();
}

JNIEXPORT jint JNICALL
Java_com_nextone_JServer_JServer_getPgid (JNIEnv *env, jclass cls) {
  return (jint)getpgrp();
}

JNIEXPORT jint JNICALL
Java_com_nextone_JServer_JServer_getPpid (JNIEnv *env, jclass cls) {
  return (jint)getppid();
}

JNIEXPORT jboolean JNICALL
Java_com_nextone_JServer_JServer_isNativeInitDone (JNIEnv *env, jclass cls) {
  return CHECK_STATUS(lsMem, STATUS_ALL_INIT)?JNI_TRUE:JNI_FALSE;
}

JNIEXPORT jint JNICALL
Java_com_nextone_JServer_JServer_getUsedLicense(JNIEnv *env, jobject obj) {
    return lsMem->usedlic;
}

JNIEXPORT jint JNICALL
Java_com_nextone_JServer_JServer_getLicenseLimit(JNIEnv *env, jobject obj) {
   return lsMem->nlic;
}

JNIEXPORT jint JNICALL
Java_com_nextone_JServer_BridgeServer_getMaximumCalls(JNIEnv *env, jobject obj) {
  return lsMem->maxCalls;
}

JNIEXPORT jint JNICALL
Java_com_nextone_JServer_BridgeServer_getMaximumMRCalls(JNIEnv *env, jobject obj) {
  return lsMem->maxMRCalls;
}

JNIEXPORT jint JNICALL
Java_com_nextone_JServer_JServer_getCalls(JNIEnv *env, jobject obj) {
    int calls =0;
    int shmId;

    shmId = CacheAttach();
    if (shmId == -1) {
      ThrowBridgeException(env, "Unable to attach to the cache");
      return -1;
    }
    calls = nlm_getUsedvportNolock();
    /* detach from the cache */
    CacheDetach();
    return calls;
}

JNIEXPORT jint JNICALL
Java_com_nextone_JServer_JServer_getMRCalls(JNIEnv *env, jobject obj) {
    int calls =0;
    int shmId;

    shmId = CacheAttach();
    if (shmId == -1) {
      ThrowBridgeException(env, "Unable to attach to the cache");
      return -1;
    }
    calls = nlm_getUsedMRvportNolock();
    /* detach from the cache */
    CacheDetach();
    return calls;
}

JNIEXPORT jlong JNICALL
Java_com_nextone_JServer_JServer_getLicenseExpiryTime(JNIEnv *env, jobject obj) {
  return lsMem->expiry_time;
}

JNIEXPORT jintArray JNICALL
Java_com_nextone_JServer_BridgeServer_getIServerVportLicenseAlarms(JNIEnv *env, jobject obj){
    const char *argv = NULL;
    jintArray alarmArr;
    jint valarms[MAX_LS_ALARM];
    int count =0;
    int ret;

    argv        =       "lsalarm";

    memset(licenseVportAlarms, 0, MAX_LS_ALARM*sizeof(time_t));

   switch ((ret = ProcessCommand(1, (char **)&argv))) {
      case xleOk:
        LOCAL_DEBUG(JNI_FALSE, DEBUG_VERBOSE, ("listed vport license alarms\n"));
        break;
      default:   /* some kind of error happened */
       LOCAL_DEBUG(JNI_FALSE, DEBUG_ERROR, ("Error in listing vport license alarms ret= %d\n",ret));
       return NULL;
   }
    // no alarms
    if(licenseVportAlarms[0] <=0)
       return NULL;
    for(count=0; count <   MAX_LS_ALARM; count++){
      valarms[count]   = (jint)licenseVportAlarms[count];
      LOCAL_DEBUG(JNI_FALSE, DEBUG_VERBOSE, ("/-------- valarm[%d] = %d\n", count, licenseVportAlarms[count]));
    }
    alarmArr = (*env)->NewIntArray(env,MAX_LS_ALARM);
    if(alarmArr == NULL)
      return NULL;
    (*env)->SetIntArrayRegion(env,alarmArr,0,MAX_LS_ALARM,valarms);
    (*env)->ReleaseIntArrayElements(env,alarmArr,valarms,0);
    return alarmArr;
}


JNIEXPORT jintArray JNICALL
Java_com_nextone_JServer_BridgeServer_getIServerMRVportLicenseAlarms(JNIEnv *env, jobject obj){
    const char *argv = NULL;
    jintArray alarmArr;
    jint mrvalarms[MAX_LS_ALARM];
    int count =0;
    int ret;

    argv        =       "lsalarm";

    memset(licenseMRVportAlarms, 0, MAX_LS_ALARM*sizeof(time_t));

   switch ((ret = ProcessCommand(1, (char **)&argv))) {
      case xleOk:
        LOCAL_DEBUG(JNI_FALSE, DEBUG_VERBOSE, ("listed mrvport license alarms\n"));
        break;
      default:   /* some kind of error happened */
       LOCAL_DEBUG(JNI_FALSE, DEBUG_ERROR, ("Error in listing mrvport license alarms ret= %d\n",ret));
       return NULL;
   }
    // no alarms
    if(licenseMRVportAlarms[0] <= 0)
       return NULL;
    for(count=0; count <   MAX_LS_ALARM; count++){
      mrvalarms[count]   = (jint)licenseMRVportAlarms[count];
      LOCAL_DEBUG(JNI_FALSE, DEBUG_VERBOSE, ("/-------- mrvalarm[%d] = %d\n", count, licenseMRVportAlarms[count]));
    }
    alarmArr = (*env)->NewIntArray(env,MAX_LS_ALARM);
    if(alarmArr == NULL)
      return NULL;
    (*env)->SetIntArrayRegion(env,alarmArr,0,MAX_LS_ALARM,mrvalarms);
    (*env)->ReleaseIntArrayElements(env,alarmArr,mrvalarms,0);
    return alarmArr;
}


int lsAlarmFn(time_t* valarms, time_t* mrvalarms){
    memcpy(licenseVportAlarms,valarms,MAX_LS_ALARM*sizeof(time_t));
    memcpy(licenseMRVportAlarms,mrvalarms,MAX_LS_ALARM*sizeof(time_t));
}



static int
ExtractCallPlanRouteEntry (VpnRouteEntry *entry, CallPlanRouteData *cprd) {
  cprd->crname = entry->crname;
  cprd->src = entry->src;
  cprd->srcLen = entry->srclen;
  cprd->dest = entry->dest;
  cprd->destLen = entry->destlen;
  cprd->dstPrefix = entry->prefix;
  cprd->srcPrefix = entry->srcprefix;
  cprd->crflags = entry->crflags;
  cprd->refreshTime = entry->mTime;
  return 0;
}





static jboolean
CreateNewCallPlanRouteObject (JNIEnv *env, CallPlanRouteData *cprd, jobject cprObject) {
  jboolean result = JNI_TRUE;
  jstring crname, src, dest, dstPrefix, srcPrefix;
  jthrowable exc;
  jstring excString;
  const char *exception;

  crname = (*env)->NewStringUTF(env, cprd->crname);
  if (crname == NULL)
    return JNI_FALSE;

  src = (*env)->NewStringUTF(env, cprd->src);
  if (src == NULL) {
    (*env)->DeleteLocalRef(env, crname);
    return JNI_FALSE;
  }

  dest = (*env)->NewStringUTF(env, cprd->dest);
  if (dest == NULL) {
    (*env)->DeleteLocalRef(env, crname);
    (*env)->DeleteLocalRef(env, src);
    return JNI_FALSE;
  }

  dstPrefix = (*env)->NewStringUTF(env, cprd->dstPrefix);
  if (dstPrefix == NULL) {
    (*env)->DeleteLocalRef(env, crname);
    (*env)->DeleteLocalRef(env, src);
    (*env)->DeleteLocalRef(env, dest);
    return JNI_FALSE;
  }

  srcPrefix = (*env)->NewStringUTF(env, cprd->srcPrefix);
  if (srcPrefix == NULL) {
    (*env)->DeleteLocalRef(env, crname);
    (*env)->DeleteLocalRef(env, src);
    (*env)->DeleteLocalRef(env, dest);
    (*env)->DeleteLocalRef(env, dstPrefix);
    return JNI_FALSE;
  }

  (*env)->CallVoidMethod(env, cprObject, cprResetMid, dest, dstPrefix, (jlong)cprd->refreshTime, JNI_TRUE, src, srcPrefix, (jint)cprd->srcLen, (jint)cprd->destLen, crname, (jint)cprd->crflags);

  exc = (*env)->ExceptionOccurred(env);
  if (exc != NULL) {
    /* any exception happens, log here and throw a new bridge exception */
    (*env)->ExceptionDescribe(env);
    (*env)->ExceptionClear(env);

    LOCAL_DEBUG(JNI_TRUE, DEBUG_VERBOSE, ("CreateNewCallPlanRouteObject: error happened"));

    excString = ExtractExceptionString(env, exc);
    if (excString == NULL) {
      (*env)->DeleteLocalRef(env, exc);
      (*env)->DeleteLocalRef(env, crname);
      (*env)->DeleteLocalRef(env, src);
      (*env)->DeleteLocalRef(env, dest);
      (*env)->DeleteLocalRef(env, dstPrefix);
      (*env)->DeleteLocalRef(env, srcPrefix);
      return JNI_FALSE;   /* some exception was thrown */
    }

    exception = (*env)->GetStringUTFChars(env, excString, NULL);
    if (exception == NULL) {
      (*env)->DeleteLocalRef(env, excString);
      (*env)->DeleteLocalRef(env, exc);
      (*env)->DeleteLocalRef(env, crname);
      (*env)->DeleteLocalRef(env, src);
      (*env)->DeleteLocalRef(env, dest);
      (*env)->DeleteLocalRef(env, dstPrefix);
      (*env)->DeleteLocalRef(env, srcPrefix);
      return JNI_FALSE;  /* out of memory exception is thrown */
    }

    ThrowBridgeException(env, exception);
    (*env)->ReleaseStringUTFChars(env, excString, exception);
    (*env)->DeleteLocalRef(env, excString);
    (*env)->DeleteLocalRef(env, exc);
    result = JNI_FALSE;
  }

  (*env)->DeleteLocalRef(env, crname);
  (*env)->DeleteLocalRef(env, src);
  (*env)->DeleteLocalRef(env, dest);
  (*env)->DeleteLocalRef(env, dstPrefix);
  (*env)->DeleteLocalRef(env, srcPrefix);
  return result;
}


JNIEXPORT jboolean JNICALL
Java_com_nextone_JServer_BridgeServer_getCallingPlanRouteImpl (JNIEnv *env,
							       jobject obj,
							       jstring rname,
							       jobject cprObject) {
  CallPlanRouteEntry *routeEntry = NULL;
  CallPlanRouteData cprd = {0};
  const char *croutename;
  jboolean result = JNI_TRUE;
  VpnRouteKey  key;

  memset(&key, 0, sizeof(VpnRouteKey));

  croutename = (*env)->GetStringUTFChars(env, rname, NULL);
  strcpy(key.crname,croutename);

  if (croutename == NULL)
    return JNI_FALSE;  /* OutOfMemoryError will be thrown */

  routeEntry = (CallPlanRouteEntry *)DbExtractEntry(DBNAME(&defCommandData[0], DB_eCallRoute),
						    DB_eCallRoute, (char *)&key, sizeof(CallPlanRouteKey));

  if (routeEntry !=  NULL) {
    if (ExtractCallPlanRouteEntry(routeEntry, &cprd)) {
      checkFree(routeEntry);
      ThrowBridgeException(env, "Database format error (DB_eCallRoute)");
      return JNI_FALSE;
    }

    result = CreateNewCallPlanRouteObject(env, &cprd, cprObject);
    checkFree(routeEntry);
  }

  (*env)->ReleaseStringUTFChars(env, rname, croutename);

  if (result == JNI_TRUE) {
    LOCAL_DEBUG(JNI_TRUE, DEBUG_VERBOSE, ("getCallingPlanRouteImpl: returning a valid Calling plan route"));
  }

  return result;
}



JNIEXPORT jint JNICALL
Java_com_nextone_JServer_BridgeServer_getCallPlanAttrLen (JNIEnv *env, jobject obj) {
  return CALLPLAN_ATTR_LEN;
}


/**
 * fills the IedgeListData structure for the given regid from the database. If the port
 * given is < zero, searches up to 120 ports to find the first matching entry with a valid
 * IP address.
 *
 * @return 0 on success, -1 on failure
 */
static int
getIedgeListData (JNIEnv *env, const char *serial, int port, IedgeListData *ld)
{
  NetoidInfoEntry *netInfo = NULL;
  ClientAttribs *clAttribs = NULL;
  NetoidSNKey key;
  int i, count;

  /* open databases */
  if (OpenDatabases(&defCommandData[0]) < 0) {
    ThrowBridgeException(env, "Error opening databases");
    return -1;
  }

  memset(&key, 0, sizeof(key));
  strcpy(key.regid, serial);
  if (port < 0) {
    key.uport = 0;
    count = 120;
  } else {
    key.uport = port;
    count = 1;
  }

  for (i = 0; i < count; key.uport++, i++) {
    netInfo = DbFindInfoEntry(GDBMF(&defCommandData[0], DB_eNetoids), 
			      (char *)&key, sizeof(key));
    if (netInfo == NULL)
      continue;  /* try the next port */

    clAttribs = DbFindAttrEntry(GDBMF(&defCommandData[0], DB_eAttribs), 
				(char *)&key,
				sizeof(NetoidSNKey));

    if (ExtractDbInfoEntry(env, netInfo, clAttribs, ld)) {
      checkFree(netInfo);
      checkFree(clAttribs);
      CloseDatabases(&defCommandData[0]);
      ThrowBridgeException(env, "Database format error (DB_eNetoids)");
      return -1;
    }

    checkFree(netInfo);
    checkFree(clAttribs);

    /* if the ipaddress is not found try the next port */
    if ((count ==  120) && (ld->ipaddr == 0))
      continue;

    CloseDatabases(&defCommandData[0]);
    return 0;
  }

  CloseDatabases(&defCommandData[0]);

  LOCAL_DEBUG(JNI_TRUE, DEBUG_NORMAL, ("getIEdgeListData: no such port (%d) exist", port+1));
  return -1;
}


static jobject
getIEdgeList (JNIEnv *env, jstring regid, jint port) {
  const char *serial;
  jobject newObject;
  IedgeListData ld;

  memset(&ld, 0, sizeof(ld));
  /* check to make sure we have enough memory to operate */
  if ((*env)->EnsureLocalCapacity(env, 20) < 0)
    return NULL;  /* OutOfMemoryError will be thrown */

  serial = (*env)->GetStringUTFChars(env, regid, NULL);
  if (serial == NULL)
    return NULL;    /* OutOfMemoryError will be thrown */

  if (strlen(serial) >= REG_ID_LEN) {
    (*env)->ReleaseStringUTFChars(env, regid, serial);
    ThrowBridgeException(env, "Registration ID length exceeds maximum allowed value");
    return NULL;
  }

  if (getIedgeListData(env, serial, port, &ld)) {
    (*env)->ReleaseStringUTFChars(env, regid, serial);
    return NULL;
  }

  (*env)->ReleaseStringUTFChars(env, regid, serial);

  newObject = CreateNewIedgeListObject(env, &ld);
  if (newObject != NULL) {
    LOCAL_DEBUG(JNI_TRUE, DEBUG_VERBOSE, ("getIedgeList: returning IEdgeList for %s:%d", serial, port+1));
  }

  return newObject;
}


JNIEXPORT jobject JNICALL
Java_com_nextone_JServer_AutoDownload_getIEdgeList (JNIEnv *env,
						    jclass cls,
						    jstring regid) {
  return getIEdgeList(env, regid, -2);
}


/* returns the IEdgeList object for the InfoEntry passed. Lookup could be 
 * based on phone number, vpn number, regid, etc...
 */
static jobject
getIedgeListForInfoEntry (JNIEnv *env, InfoEntry *pentry) {
  IedgeListData ld = {0};
  CacheTableInfo tmpCacheInfo = {0};
  CacheTableInfo *cacheInfo = &tmpCacheInfo;
  ClientAttribs *clAttribs = NULL;
  jobject newObject;
  int shmId;

  memset(&ld, 0, sizeof(IedgeListData));
  /* check to make sure we have enough memory to operate */
  if ((*env)->EnsureLocalCapacity(env, 20) < 0)
    return NULL;  /* OutOfMemoryError will be thrown */

  /* attach to the cache */
  shmId = CacheAttach();
  if (shmId == -1) {
    ThrowBridgeException(env, "Unable to attach to the cache");
    return NULL;
  }

  /* find the iedge in the cache */
  if (FindIedge(pentry, cacheInfo, sizeof(CacheTableInfo)) < 0) {
    if (BIT_TEST(pentry->sflags, ISSET_PHONE)) {
      LOCAL_DEBUG(JNI_TRUE, DEBUG_NORMAL, ("getIedgeListForInfoEntry: No entry for phone - %s", pentry->phone));
    } else if (BIT_TEST(pentry->sflags, ISSET_VPNPHONE)) {
      LOCAL_DEBUG(JNI_TRUE, DEBUG_NORMAL, ("getIedgeListForInfoEntry: No entry for vpn phone - %s:%d", pentry->vpnPhone, pentry->vpnExtLen));
    } else if (BIT_TEST(pentry->sflags, ISSET_REGID)) {
      LOCAL_DEBUG(JNI_TRUE, DEBUG_NORMAL, ("getIedgeListForInfoEntry: No entry for regid/uport - %s/%d", pentry->regid, pentry->uport));
    }
    CacheDetach();
    return NULL;
  }

  /* detach from the cache */
  CacheDetach();

  /* open databases */
  if (OpenDatabases(&defCommandData[0]) < 0) {
    ThrowBridgeException(env, "Error opening databases");
    return NULL;
  }

  clAttribs = DbFindAttrEntry(GDBMF(&defCommandData[0], DB_eAttribs), 
			      (char *)&cacheInfo->data,
			      sizeof(NetoidSNKey));

  if (ExtractDbInfoEntry(env, &cacheInfo->data, clAttribs, &ld)) {
    ThrowBridgeException(env, "Database format error");
    checkFree(clAttribs);
    return NULL;
  }

  //DumpLData(&ld);

  newObject = CreateNewIedgeListObject(env, &ld);
  if (newObject != NULL) {
    LOCAL_DEBUG(JNI_TRUE, DEBUG_VERBOSE, ("getIedgeListForInfoEntry: returning a valid IEdgeList"));
  }

  checkFree(clAttribs);

  /* close the database */
  CloseDatabases(&defCommandData[0]);

  return newObject;
}


/**
 * gets the information from the cache
 */
JNIEXPORT jobject JNICALL
Java_com_nextone_JServer_BridgeServer_getIedgeList (JNIEnv *env,
						    jobject obj,
						    jstring regid,
						    jint port) {
  InfoEntry infoEntry;
  const char *serial;

  /* check to make sure we have enough memory to operate */
  if ((*env)->EnsureLocalCapacity(env, 20) < 0)
    return NULL;  /* OutOfMemoryError will be thrown */

  serial = (*env)->GetStringUTFChars(env, regid, NULL);
  if (serial == NULL)
    return NULL;    /* OutOfMemoryError will be thrown */

  if (strlen(serial) >= REG_ID_LEN) {
    (*env)->ReleaseStringUTFChars(env, regid, serial);
    ThrowBridgeException(env, "Registration ID length exceeds maximum allowed value");
    return NULL;
  }

  memset(&infoEntry, 0, sizeof(InfoEntry));

  strncpy(infoEntry.regid, serial, REG_ID_LEN);
  BIT_SET(infoEntry.sflags, ISSET_REGID);
  infoEntry.uport = port;
  BIT_SET(infoEntry.sflags, ISSET_UPORT);

  (*env)->ReleaseStringUTFChars(env, regid, serial);

  return getIedgeListForInfoEntry(env, &infoEntry);
}


/**
 * gets the information from the cache
 */
JNIEXPORT jobject JNICALL
Java_com_nextone_JServer_BridgeServer_getIedgeListForPhone (JNIEnv *env,
							    jobject obj,
							    jstring ph) {
  InfoEntry infoEntry;
  const char *phone;

  memset(&infoEntry, 0, sizeof(InfoEntry));
  phone = (*env)->GetStringUTFChars(env, ph, NULL);
  if (phone == NULL)
    return NULL;  /* OutOfMemoryError will be thrown */

  strncpy(infoEntry.phone, phone, PHONE_NUM_LEN);
  BIT_SET(infoEntry.sflags, ISSET_PHONE);

  (*env)->ReleaseStringUTFChars(env, ph, phone);

  return getIedgeListForInfoEntry(env, &infoEntry);
}


/**
 * gets the information from the cache
 */
JNIEXPORT jobject JNICALL
Java_com_nextone_JServer_BridgeServer_getIedgeListForVpnPhone (JNIEnv *env,
							       jobject obj,
							       jstring ph,
							       jint extLen) {
  InfoEntry infoEntry;
  const char *phone;

  memset(&infoEntry, 0, sizeof(InfoEntry));
  phone = (*env)->GetStringUTFChars(env, ph, NULL);
  if (phone == NULL)
    return NULL;  /* OutOfMemoryError will be thrown */

  strncpy(infoEntry.vpnPhone, phone, VPN_LEN);
  infoEntry.vpnExtLen = extLen;
  BIT_SET(infoEntry.sflags, ISSET_VPNPHONE);

  (*env)->ReleaseStringUTFChars(env, ph, phone);

  return getIedgeListForInfoEntry(env, &infoEntry);
}


JNIEXPORT void JNICALL
Java_com_nextone_JServer_JServer_declareDbStale (JNIEnv *env, jobject obj) {
  const char *argv[2] = {0};

  argv[0] = "db";
  argv[1] = "stale";
  ProcessCommand(2, (char **)argv);
}


JNIEXPORT jint JNICALL
Java_com_nextone_JServer_JServerMain_mswInit (JNIEnv *env, jclass cls) {

  /* read the config file */
  setConfigFile();
  DoConfig(JserverProcessConfig);

  if (SHM_Init(ISERVER_CACHE_INDEX) < 0) {
    printf("mswInit: SHM_Init - cannot initialize shared memory");
    return -1;
  }

  if (segowner != CONFIG_SERPLEX_JSERVER) {
    while (CacheAttach() < 0) {
//      printf("waiting for CacheAttach to be successfull...\n");
      sleep(1);
    }
  } else if (CacheInit() < 0) {
    printf("mswInit: CacheInit - cannot attach to shared memory, segowner not jserver");
    return -1;
  }

  //  cliLibFlags = COMMANDF_ALL;

  return 0;
}


JNIEXPORT void JNICALL
Java_com_nextone_JServer_JServerMain_nativeInit (JNIEnv *env, jclass cls, jobject config, jobject cap) {

  if (utilsIdInit(env))
    return;

  /* initialize all the static class, field and method Ids used */

  /* SendListItem */
  sliClass = GetGlobalClassReference(env, "com/nextone/JServer/SendListItem");
  if (sliClass == NULL) {
    return;
  }

  sliSendMid = (*env)->GetMethodID(env, sliClass, "send", "(Ljava/lang/Object;)V");
  if (sliSendMid == NULL) {
    return;
  }

  /* RouteData */
  routeDataClass = GetGlobalClassReference(env, "com/nextone/common/RouteData");
  if (routeDataClass == NULL) {
    return;
  }

  routeDataConstId = (*env)->GetMethodID(env, routeDataClass, "<init>", "(Ljava/lang/String;)V");
  if (routeDataConstId == NULL) {
    return;
  }


  /* CallPlanRoute */
  cprClass = GetGlobalClassReference(env, "com/nextone/common/CallPlanRoute");
  if (cprClass == NULL) {
    return;
  }

  cprConstId = (*env)->GetMethodID(env, cprClass, "<init>", "(Ljava/lang/String;Ljava/lang/String;ILjava/lang/String;ILjava/lang/String;Ljava/lang/String;IJ)V");
  if (cprConstId == NULL) {
    return;
  }

  cprResetMid = (*env)->GetMethodID(env, cprClass, "reset", "(Ljava/lang/String;Ljava/lang/String;JZLjava/lang/String;Ljava/lang/String;IILjava/lang/String;I)V");
  if (cprResetMid == NULL) {
    return;
  }
  /* ProvisionData */
  pdClass = GetGlobalClassReference(env, "com/nextone/common/ProvisionData");
  if (pdClass == NULL) {
    return;
  }

  pdConstId = (*env)->GetMethodID(env, pdClass, "<init>", "(Ljava/lang/String;Ljava/lang/String;Ljava/lang/String;Ljava/lang/String;Ljava/lang/String;Ljava/lang/String;Ljava/lang/String;Ljava/lang/String;Ljava/lang/String;Ljava/lang/String;Ljava/lang/String;Ljava/lang/String;IILjava/lang/String;Ljava/lang/String;Ljava/lang/String;Ljava/lang/String;ZZZLjava/lang/String;IZLjava/lang/String;Ljava/lang/String;IZZZIIIZLjava/lang/String;Ljava/lang/String;IIILjava/lang/String;Ljava/lang/String;ZZIILjava/lang/String;)V");
  if (pdConstId == NULL) {
    return;
  }
  /* IEdgeList */
  ilClass = GetGlobalClassReference(env, "com/nextone/common/IEdgeList");
  if (ilClass == NULL) {
    return;
  }

  ilConstId = (*env)->GetMethodID(env, ilClass, "<init>", "(IIIIIIIIIIIIIIIIIIIJJLjava/lang/String;Ljava/lang/String;Ljava/lang/String;Ljava/lang/String;Ljava/lang/String;Ljava/lang/String;Ljava/lang/String;Ljava/lang/String;Ljava/lang/String;Ljava/lang/String;Ljava/lang/String;Ljava/lang/String;Ljava/lang/String;Ljava/lang/String;Ljava/lang/String;Ljava/lang/String;Ljava/lang/String;Ljava/lang/String;Ljava/lang/String;Ljava/lang/String;Ljava/lang/String;ZZZZZSILjava/lang/String;Ljava/lang/String;ILjava/lang/String;Ljava/lang/String;Ljava/lang/String;Ljava/lang/String;)V");

  if (ilConstId== NULL)
    return;

 ilConstId1 = (*env)->GetMethodID(env,ilClass,"<init>","(IIIIIIIIIIIIIIIIIIIJJLjava/lang/String;Ljava/lang/String;Ljava/lang/String;Ljava/lang/String;Ljava/lang/String;Ljava/lang/String;Ljava/lang/String;Ljava/lang/String;Ljava/lang/String;Ljava/lang/String;Ljava/lang/String;Ljava/lang/String;Ljava/lang/String;Ljava/lang/String;Ljava/lang/String;Ljava/lang/String;Ljava/lang/String;Ljava/lang/String;Ljava/lang/String;Ljava/lang/String;Ljava/lang/String;ZZZZZSILjava/lang/String;Ljava/lang/String;ILjava/lang/String;Ljava/lang/String;Ljava/lang/String;Ljava/lang/String;Ljava/lang/String;II)V");
  if(ilConstId1 == NULL)
    return;

  iEdgeMaxCallsFid = (*env)->GetFieldID(env, ilClass, "maxCalls", "I");
  if (iEdgeMaxCallsFid == NULL) {
    (*env)->DeleteLocalRef(env, ilClass);
    return ;
  }

  iEdgeCurCallsFid = (*env)->GetFieldID(env, ilClass, "currentCalls", "I");
  if (iEdgeCurCallsFid == NULL) {
    (*env)->DeleteLocalRef(env, ilClass);
    return ;
  }

  if (iserverConfigIdInit(env, config))
    return;

  if(iserverCapInit(env,cap))
    return;

 CliSetupExtFns(); 
}
