#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include "utils.h"
#include "JServer.h"
#include "BridgeServer.h"
#include "cli.h"
#include "lsconfig.h"
#include "cdr.h"
#include "firewallcontrol.h"
#include "log.h"
#include "ifs.h"
#include "systemlog.h"
#include "ispd_common.h"

extern int FindServerConfig (void);
extern void setConfigFile (void);
extern int DoConfig (int (*)(void));
//extern char* FCECreateXMLFromConfig ();

extern char PrivateInterface[256];
extern char PublicInterface[256];

// this caches the config object read from server.cfg
static jobject configObject = NULL;
// if network redundancy is enabled, this caches the IP address of the interface
// on which the vip is configured
static jobject fromIpAddr = NULL;

static jclass iscClass = NULL;
static jmethodID iscConstId = NULL;

static jfieldID sipConfigFid = NULL;
static jfieldID h323ConfigFid = NULL;
static jfieldID fceConfigFid = NULL;
static jfieldID fceConfigNewFid = NULL;
static jfieldID billingConfigFid = NULL;
static jfieldID advancedConfigFid = NULL;
static jfieldID systemConfigFid = NULL;
static jfieldID loggingConfigFid = NULL;
static jfieldID redConfigFid = NULL;
static jfieldID redNetConfigFid = NULL;
static jfieldID redDbConfigFid = NULL;
static jfieldID radiusConfigFid = NULL;

static jfieldID scServerNameFid = NULL;
static jfieldID scSipDomainFid = NULL;
//static jfieldID scAuthEnabledFid = NULL;
static jfieldID scAuthFid = NULL;
static jfieldID scAuthPasswordFid = NULL;
static jfieldID scRecordRouteEnabledFid = NULL;
static jfieldID scServerTypeFid = NULL;
static jfieldID scMaxForwardsFid = NULL;
static jfieldID scSipTimerCFid = NULL;
static jfieldID scSipPortFid = NULL;
static jfieldID scOBPFid = NULL;
static jfieldID scDynamicEPFid = NULL;
static jfieldID scInternalCallFid = NULL;


static jfieldID h323SgkRegPrefixFid = NULL;
static jfieldID h323GkIdFid = NULL;
static jfieldID h323RrqTimerFid = NULL;
static jfieldID h323RrqTtlFid = NULL;
static jfieldID h323H245RoutingEnabledFid = NULL;
static jfieldID h323ForceH245Fid = NULL;
static jfieldID h323CallRoutingEnabledFid = NULL;
static jfieldID h323FastStartEnabledFid = NULL;
static jfieldID h323CpsFid = NULL;
static jfieldID h323InstancesFid = NULL;
static jfieldID h323SgkMaxCallsFid = NULL;
static jfieldID h323MaxCallsFid = NULL;
static jfieldID h323LocalProceedingFid = NULL;
static jfieldID h323InfoTransCapFid = NULL;
static jfieldID h323AllowDestArqEnabledFid  = NULL;
static jfieldID h323AllowAuthArqEnabledFid  = NULL;
static jfieldID h323H245TunnelEnabledFid  = NULL;

#if FCE_REMOVED
static jfieldID fceFwNameFid = NULL;
static jfieldID fceH245Fid = NULL;
static jfieldID fceDefaultPublicFid = NULL;
static jmethodID fceSetFwConnectAddrMid = NULL;
static jmethodID fceGetFwConnectAddrMid = NULL;
static jmethodID fceAddNetworkListMid = NULL;
static jmethodID fceGetNetworkListMid = NULL;
static jfieldID fceNetworkListIPMaskFid = NULL;
static jfieldID fceNetworkListIsPublicFid = NULL;
static jmethodID fceIPMaskGetIpMid = NULL;
static jmethodID fceIPMaskGetMaskMid = NULL;
#endif

static jfieldID fceNewFwNameFid = NULL;
static jmethodID fceNewSetFwAddressMid = NULL;
static jmethodID fceNewGetFwAddressMid = NULL;

static jfieldID billCdrTypeFid = NULL;
static jfieldID billCdrFormatFid = NULL;
static jfieldID billDirFid = NULL;
static jfieldID billCdrTimerFid = NULL;
static jfieldID billBillingTypeFid = NULL;
static jfieldID billCdrLogStart1Fid = NULL;
static jfieldID billCdrLogStart2Fid = NULL;
static jfieldID billCdrLogEnd2Fid = NULL;
static jfieldID billCdrLogHuntFid = NULL;

static jfieldID adNumSegmentsFid = NULL;
static jfieldID adSegSizeFid = NULL;
static jfieldID adPriorityFid = NULL;
static jfieldID adNumThreadsFid = NULL;
static jfieldID adThreadStackSizeFid = NULL;

static jfieldID sysG711Ulaw64DurationFid = NULL;
static jfieldID sysG711Alaw64DurationFid = NULL;
static jfieldID sysG729FramesFid = NULL;
static jfieldID sysG7231FramesFid = NULL;
static jfieldID sysDefaultCodecFid = NULL;
static jfieldID sysEnumDomainFid = NULL;
static jfieldID sysRolloverTimeFid = NULL;
static jfieldID sysCacheTimeoutFid = NULL;
//static jfieldID sysRadiusServerFid = NULL;
//static jfieldID sysRadiusSecretFid = NULL;
static jfieldID sysAllowAllSrcFid = NULL;
static jfieldID sysAllowAllDstFid = NULL;
static jfieldID sysAllowHairpinCallsFid = NULL;
static jfieldID sysRemoveRFC2833Fid = NULL;
static jfieldID sysRemoveT38Fid = NULL;
static jfieldID sysDefaultMediaRoutingFid = NULL;
static jfieldID sysDefaultMidCallMediaChangeFid = NULL;
static jfieldID sysMaxHuntsFid = NULL;
static jfieldID sysMaxHuntsLimitFid = NULL;
static jfieldID sysMaxHuntsAllowableDurationFid = NULL;
static jfieldID sysForwardSrcAddrFid = NULL;
static jfieldID sysAllowAllRtpFid = NULL;
static jfieldID sysMaxCallDurationFid = NULL;
static jfieldID sysMswNameFid = NULL;
static jfieldID sysMgmtIpFid = NULL;
static jfieldID sysMapIsdnccFid = NULL;
static jfieldID sysMapLrjReasonFid = NULL;
static jfieldID sysUseCodeMapFid = NULL;

static jmethodID logSetModMid = NULL;
static jmethodID logGetModMid = NULL;
static jfieldID logDebugLevelFid = NULL;
static jfieldID logUpdateAllocationFid = NULL;
static jfieldID logTpktchanFid = NULL;
static jfieldID logUdpchanFid = NULL;
static jfieldID logPererrFid = NULL;
static jfieldID logCmFid = NULL;
static jfieldID logCmapicbFid  = NULL;
static jfieldID logCmapiFid  = NULL;
static jfieldID logCmerrFid  = NULL;
static jfieldID logLiFid  = NULL;
static jfieldID logLiinfoFid  = NULL;
static jfieldID logSDebugLevelFid = NULL;

static jfieldID redNetServerTypeFid = NULL;
// vip is not used from 3.1 msw
//static jfieldID redNetPrimIfNameFid = NULL;
//static jfieldID redNetSecondIfNameFid = NULL;
//static jfieldID redNetPrimIfVipFid = NULL;
//static jfieldID  redNetPrimaryInterfaceVipsFid   = NULL;
//static jmethodID redNetPrimaryInterfaceVipsClearMid = NULL;
//static jmethodID redNetPrimaryInterfaceVipAddMid = NULL;
//static jfieldID  redNetSecondaryInterfaceVipsFid   = NULL;
//static jmethodID redNetSecondaryInterfaceVipsClearMid = NULL;
//static jmethodID redNetSecondaryInterfaceVipAddMid = NULL;


//static jfieldID redNetPrimIfRouterFid = NULL;
//static jfieldID redNetSecondIfRouterFid = NULL;
static jfieldID redNetControlIfNameFid = NULL;
static jfieldID redNetPeersFid = NULL;
static jfieldID redNetScmFid = NULL;
static jmethodID redNetPeersClearMid = NULL;
static jmethodID redNetPeersAddMid = NULL;
static jfieldID redDbServerStatusFid = NULL;
static jfieldID redDbIfNameFid = NULL;
static jfieldID redDbMcastAddrFid = NULL;
static jfieldID redDbPortFid = NULL;
static jfieldID redDbPriorityFid = NULL;

// radius
static jfieldID radiusServersFid  = NULL;
static jfieldID radiusSecretsFid  = NULL;
static jfieldID radiusDirNameFid  = NULL;
static jfieldID radiusTimeoutFid  = NULL;
static jfieldID radiusRetryFid    = NULL;
static jfieldID radiusDeadTimeFid = NULL;
static jfieldID radiusSendMsgFid  = NULL;
static jfieldID radiusUseOSIFFid  = NULL;

//static jmethodID radiusGetServersMid = NULL;
static jmethodID radiusSetServersMid = NULL;
//static jmethodID radiusGetSecretsMid = NULL;
static jmethodID radiusSetSecretsMid = NULL;


static jclass sysutilClass = NULL;
static jmethodID suCreateStringArrayMid = NULL;
static jmethodID suCreateObjectArrayMid = NULL;

static jclass inetAddressClass = NULL;
static jmethodID inetAddressGetByNameMid = NULL;

static int logServers [] = {
  CONFIG_SERPLEX_LUS,	
  CONFIG_SERPLEX_VPNS,	
  CONFIG_SERPLEX_BCS,
  CONFIG_SERPLEX_JSERVER,
  CONFIG_SERPLEX_FAXS,
  CONFIG_SERPLEX_GIS,
  CONFIG_SERPLEX_PM
};
#define MAX_LOG_SERVERS  7

int* mswNMState;

static int
SetSipFields (JNIEnv *env, jobject sipConfig) {
  jstring jstr;

  /* sip server name */
  if (strlen(sipservername) > 0) {
    jstr = (*env)->NewStringUTF(env, sipservername);
    (*env)->SetObjectField(env, sipConfig, scServerNameFid, jstr);
    (*env)->DeleteLocalRef(env, jstr);
  }

  /* sip domain */
  if (strlen(sipdomain) > 0) {
    jstr = (*env)->NewStringUTF(env, sipdomain);
    (*env)->SetObjectField(env, sipConfig, scSipDomainFid, jstr);
    (*env)->DeleteLocalRef(env, jstr);
  }

  /* sip auth */
  /*if (sipauth)
    (*env)->SetBooleanField(env, sipConfig, scAuthEnabledFid, JNI_TRUE);
  else
    (*env)->SetBooleanField(env, sipConfig, scAuthEnabledFid, JNI_FALSE);
  */
  (*env)->SetIntField(env,sipConfig,scAuthFid,(jint)sipauth);

  /* sip auth password */
  if (strlen(sipauthpassword) > 0) {
    jstr = (*env)->NewStringUTF(env, sipauthpassword);
    (*env)->SetObjectField(env, sipConfig, scAuthPasswordFid, jstr);
    (*env)->DeleteLocalRef(env, jstr);
  }

  /* record route */
  if (recordroute)
    (*env)->SetBooleanField(env, sipConfig, scRecordRouteEnabledFid, JNI_TRUE);
  else
    (*env)->SetBooleanField(env, sipConfig, scRecordRouteEnabledFid, JNI_FALSE);
  /* sip obp */
 if (obpEnabled)
    (*env)->SetBooleanField(env, sipConfig, scOBPFid, JNI_TRUE);
  else
    (*env)->SetBooleanField(env, sipConfig, scOBPFid, JNI_FALSE);

 if (allowDynamicEndpoints)
    (*env)->SetBooleanField(env, sipConfig, scDynamicEPFid, JNI_TRUE);
  else
    (*env)->SetBooleanField(env, sipConfig, scDynamicEPFid, JNI_FALSE);

 if (allowInternalCalling)
    (*env)->SetBooleanField(env, sipConfig, scInternalCallFid, JNI_TRUE);
  else
    (*env)->SetBooleanField(env, sipConfig, scInternalCallFid, JNI_FALSE);

  /* server type */
  (*env)->SetIntField(env, sipConfig, scServerTypeFid, (jint)sipservertype);

  /* sip max forwards */
  (*env)->SetIntField(env, sipConfig, scMaxForwardsFid, (jint)sipmaxforwards);

  /* sip timerC */
  (*env)->SetIntField(env, sipConfig, scSipTimerCFid, (jint)siptimerC);

 /* sip port */
  (*env)->SetIntField(env, sipConfig, scSipPortFid, (jint)sipport);
  return 0;
}


static int
SetH323Fields (JNIEnv *env, jobject h323Config) {
  jstring jstr;

  /* SGK reg prefix */
  if (strlen(prefix) > 0) {
    jstr = (*env)->NewStringUTF(env, prefix);
    (*env)->SetObjectField(env, h323Config, h323SgkRegPrefixFid, jstr);
    (*env)->DeleteLocalRef(env, jstr);
  }

  /* GK ID */
  if (strlen(gkid) > 0) {
    jstr = (*env)->NewStringUTF(env, gkid);
    (*env)->SetObjectField(env, h323Config, h323GkIdFid, jstr);
    (*env)->DeleteLocalRef(env, jstr);
  }

  /* RRQ timer */
  if (rrqtimer != 0)
    (*env)->SetIntField(env, h323Config, h323RrqTimerFid, (jint)rrqtimer);

  /* RRQ TTL */
  if (rrqttl != 0)
    (*env)->SetIntField(env, h323Config, h323RrqTtlFid, (jint)rrqttl);

  /* H.245 routing */
  (*env)->SetBooleanField(env, h323Config, h323H245RoutingEnabledFid, routeH245?JNI_TRUE:JNI_FALSE);

  /* Force H.245 */
  (*env)->SetBooleanField(env, h323Config, h323ForceH245Fid, forceh245?JNI_TRUE:JNI_FALSE);

  /* Call Routing */
  (*env)->SetBooleanField(env, h323Config, h323CallRoutingEnabledFid, routecall?JNI_TRUE:JNI_FALSE);

  /* fast start */
  (*env)->SetBooleanField(env, h323Config, h323FastStartEnabledFid, doFastStart?JNI_TRUE:JNI_FALSE);

  /* calls per second */
  (*env)->SetIntField(env, h323Config, h323CpsFid, (jint)h323Cps);

  /* instances */
  (*env)->SetIntField(env, h323Config, h323InstancesFid, (jint)nh323CfgInstances);

  /* sgatekeeper max calls*/
  (*env)->SetIntField(env, h323Config, h323SgkMaxCallsFid, (jint)h323maxCallsSgk);

  /* max calls*/
  (*env)->SetIntField(env, h323Config, h323MaxCallsFid, (jint)h323maxCalls);

  /* local proceeding flag */
  (*env)->SetBooleanField(env, h323Config, h323LocalProceedingFid, localProceeding?JNI_TRUE:JNI_FALSE);

  /* info transfer cap*/
  (*env)->SetIntField(env, h323Config, h323InfoTransCapFid, (jint)h323infoTransCap);

  /* allow dest arq*/
  (*env)->SetBooleanField(env, h323Config, h323AllowDestArqEnabledFid, allowDestArq?JNI_TRUE:JNI_FALSE);

  /* allow auth arq*/
  (*env)->SetBooleanField(env, h323Config, h323AllowAuthArqEnabledFid, allowAuthArq?JNI_TRUE:JNI_FALSE);

  /* allow h245 tunnel arq*/
  (*env)->SetBooleanField(env, h323Config, h323H245TunnelEnabledFid, h245Tunneling?JNI_TRUE:JNI_FALSE);

  return 0;
}

#if FCE_REMOVED

/* call back method passed into FCEListNetworkList to list the private networks 
   configured */
static int
AddNetworkList (int addr, int mask, int isPublic, void *passAlong1, void *passAlong2) {
  jobject fceConfig = (jobject)passAlong1;
  JNIEnv *env = (JNIEnv *)passAlong2;
  char addr1[32], addr2[32];

  /* add a private net */
  (*env)->CallVoidMethod(env, fceConfig, fceAddNetworkListMid, (jint)addr, (jint)mask, (isPublic == TRUE)?JNI_TRUE:JNI_FALSE);
  if (CheckAndThrowBridgeException(env))
    return -1;

  LOCAL_DEBUG(JNI_TRUE, DEBUG_VERBOSE, ("Adding network: %s %s/%s", (isPublic == TRUE)?"public":"private", FormatIpAddress(addr, addr1), FormatIpAddress(mask, addr2)));

  return 0;
}
#endif


static int
SetFceFields (JNIEnv *env, jobject fceConfig) {
#if FCE_REMOVED
  jstring jstr;
  /* firewall name */
  if (strlen(fceConfigFwName) > 0) 
    jstr = (*env)->NewStringUTF(env, fceConfigFwName);
  else
    jstr = (*env)->NewStringUTF(env, "none");
  (*env)->SetObjectField(env, fceConfig, fceFwNameFid, jstr);
  (*env)->DeleteLocalRef(env, jstr);

  /* H.245 pinhole enabled? */
  (*env)->SetBooleanField(env, fceConfig, fceH245Fid, (fceH245PinholeEnabled == TRUE)?JNI_TRUE:JNI_FALSE);

  /* firewall connect addr */
  (*env)->CallVoidMethod(env, fceConfig, fceSetFwConnectAddrMid, (jint)fceConfigOurIpAddr);
  if (CheckAndThrowBridgeException(env))
    return -1;

  /* default network space */
  (*env)->SetBooleanField(env, fceConfig, fceDefaultPublicFid, (fceDefaultPublic == TRUE)?JNI_TRUE:JNI_FALSE);

  /* list of private nets */
  return FCEListNetworkList(AddNetworkList, fceConfig, env);
#endif
  return 0;
}

static int
SetFceNewFields (JNIEnv *env, jobject fceConfig) {
  jstring jstr;

  /* firewall name */
  if (strlen(fceConfigFwName) > 0) 
    jstr = (*env)->NewStringUTF(env, fceConfigFwName);
  else
    jstr = (*env)->NewStringUTF(env, "none");
  (*env)->SetObjectField(env, fceConfig, fceNewFwNameFid, jstr);
  (*env)->DeleteLocalRef(env, jstr);

  /* firewall ip address */
  (*env)->CallVoidMethod(env, fceConfig, fceNewSetFwAddressMid, (jint)fceFirewallAddresses[0]); 
  if (CheckAndThrowBridgeException(env))
    return -1;

  return 0;
}
static int
SetBillingFields (JNIEnv *env, jobject billingConfig) {
  jstring jstr;

  /* CDR type */
  (*env)->SetIntField(env, billingConfig, billCdrTypeFid, (jint)cdrtype);

  /* CDR format */
  (*env)->SetIntField(env, billingConfig, billCdrFormatFid, (jint)cdrformat);

  /* CDR directory */
  if (strlen(cdrdirname) > 0) {
    jstr = (*env)->NewStringUTF(env, cdrdirname);
    (*env)->SetObjectField(env, billingConfig, billDirFid, jstr);
    (*env)->DeleteLocalRef(env, jstr);
  }

  /* CDR timer */
  (*env)->SetIntField(env, billingConfig, billCdrTimerFid, (jint)cdrtimer);

  /* billing type */
  (*env)->SetIntField(env, billingConfig, billBillingTypeFid, (jint)billingType);

  /* cdr logs */
  (*env)->SetBooleanField(env, billingConfig, billCdrLogStart1Fid, (cdrevents & CDRSTART1)?JNI_TRUE:JNI_FALSE);
  (*env)->SetBooleanField(env, billingConfig, billCdrLogStart2Fid, (cdrevents & CDRSTART2)?JNI_TRUE:JNI_FALSE);
  (*env)->SetBooleanField(env, billingConfig, billCdrLogEnd2Fid, (cdrevents & CDREND2)?JNI_TRUE:JNI_FALSE);
  (*env)->SetBooleanField(env, billingConfig, billCdrLogHuntFid, (cdrevents & CDRHUNT)?JNI_TRUE:JNI_FALSE);

  return 0;
}


static int
SetRadiusFields (JNIEnv *env, jobject radiusConfig) {
  jstring jstr;
  jobjectArray  serverArr;
  jobjectArray  secretArr;

  int i;


  /* if radius is configured */

  if (strlen(rad_server_addr[0]) > 0) {
    serverArr = (jobjectArray)(*env)->NewObjectArray(env,MAX_NUM_RAD_ENTRIES,
                  (*env)->FindClass(env,"java/lang/String"),
                  (*env)->NewStringUTF(env,""));

    secretArr = (jobjectArray)(*env)->NewObjectArray(env,MAX_NUM_RAD_ENTRIES,
                  (*env)->FindClass(env,"java/lang/String"),
                  (*env)->NewStringUTF(env,""));

    if( serverArr  !=  NULL  &&
      secretArr  != NULL){
      for(i=0;i<MAX_NUM_RAD_ENTRIES;i++) {
        // set the server
        jstr = (*env)->NewStringUTF(env, rad_server_addr[i]);
        (*env)->SetObjectArrayElement(env,serverArr,i,jstr);
        (*env)->DeleteLocalRef(env, jstr);
        // set the secret
        jstr = (*env)->NewStringUTF(env, secret[i]);
        (*env)->SetObjectArrayElement(env,secretArr,i,jstr);
        (*env)->DeleteLocalRef(env, jstr);
      }
    }
    (*env)->CallVoidMethod(env, radiusConfig, radiusSetServersMid, (jobjectArray)serverArr);
    (*env)->CallVoidMethod(env, radiusConfig, radiusSetSecretsMid, (jobjectArray)secretArr);

        /* dir name*/
   /* if (strlen(rad_dirname) > 0) {
      jstr = (*env)->NewStringUTF(env, rad_dirname);
      (*env)->SetObjectField(env, radiusConfig, radiusDirNameFid, jstr);
      (*env)->DeleteLocalRef(env, jstr);
    }
  */

    // Timeout
    (*env)->SetIntField(env, radiusConfig, radiusTimeoutFid, (jint)rad_timeout);

    // Retry
    (*env)->SetIntField(env, radiusConfig, radiusRetryFid, (jint)rad_retries);
    // Dead Time
    (*env)->SetIntField(env, radiusConfig, radiusDeadTimeFid, (jint)rad_deadtime);
    // Send Accounting Messages 
    (*env)->SetBooleanField(env, radiusConfig, radiusSendMsgFid, rad_acct?JNI_TRUE:JNI_FALSE);
    // Use Overload Session Id Format 
    (*env)->SetBooleanField(env, radiusConfig, radiusUseOSIFFid, \
                            rad_acct_session_id_overloaded?JNI_TRUE:JNI_FALSE);
    LOCAL_DEBUG(JNI_TRUE, DEBUG_NORMAL, ("radius configured "));
  }else{
    LOCAL_DEBUG(JNI_TRUE, DEBUG_NORMAL, ("radius not configured "));
  }



  return 0;
}

static int
SetAdvancedFields (JNIEnv *env, jobject advancedConfig) {
  int match = -1;
  int prevConfigServerType = myConfigServerType;

  /* maximum number of segments */
  if (max_segs != 0)
    (*env)->SetIntField(env, advancedConfig, adNumSegmentsFid, (jint)max_segs);

  /* segment size */
  if (max_segsize != 0)
    (*env)->SetIntField(env, advancedConfig, adSegSizeFid, (jint)max_segsize);

  myConfigServerType = CONFIG_SERPLEX_GIS;
  match = FindServerConfig();
  if (match == -1) {
    LOCAL_DEBUG(JNI_TRUE, DEBUG_ERROR, ("Not Configured to run lus (servertype=%d)...", myConfigServerType));
    myConfigServerType = prevConfigServerType;
    return 0;
  }

  /* priority */
  (*env)->SetIntField(env, advancedConfig, adPriorityFid, (jint)serplexes[match].prio);

  /* max number of threads */
  (*env)->SetIntField(env, advancedConfig, adNumThreadsFid, (jint)serplexes[match].threads);

  /* thread stack size */
  (*env)->SetIntField(env, advancedConfig, adThreadStackSizeFid, (jint)serplexes[match].threadstack);

  myConfigServerType = prevConfigServerType;

  return 0;
}


static int
SetSystemFields (JNIEnv *env, jobject systemConfig) {
  jstring jstr;
  int match;
  //char msg[256];
  int prevServerType  = myConfigServerType;

  /* g711Ulaw64Duration */
  if (g711Ulaw64Duration != 0)
    (*env)->SetIntField(env, systemConfig, sysG711Ulaw64DurationFid, (jint)g711Ulaw64Duration);

  /* g711Alaw64Duration */
  if (g711Alaw64Duration != 0)
    (*env)->SetIntField(env, systemConfig, sysG711Alaw64DurationFid, (jint)g711Alaw64Duration);

  /* g729Frames */
  if (g729Frames != 0)
    (*env)->SetIntField(env, systemConfig, sysG729FramesFid, (jint)g729Frames);

  /* g7231Frames */
  if (g7231Frames != 0)
    (*env)->SetIntField(env, systemConfig, sysG7231FramesFid, (jint)g7231Frames);

  /* Default Codec */
  (*env)->SetIntField(env, systemConfig, sysDefaultCodecFid, (jint)defaultCodec);
    

  /* rollover time */
  if (rolloverTime != 0)
    (*env)->SetIntField(env, systemConfig, sysRolloverTimeFid, (jint)rolloverTime);

  /* cache timeout */
  myConfigServerType = CONFIG_SERPLEX_GIS;
  match = FindServerConfig();
  if (match  !=  -1)
     (*env)->SetIntField(env, systemConfig, sysCacheTimeoutFid, (jint)serplexes[match].age.cache_timeout);
/*
  sprintf(msg, "SetIntField:match=%d, serplexes[match].age.cache_timeout=%d, GetIntField=%d", \
                          match, serplexes[match].age.cache_timeout, \
                          (int)(*env)->GetIntField(env, systemConfig, sysCacheTimeoutFid));
  LOCAL_DEBUG(JNI_TRUE, DEBUG_VERBOSE, (msg));
*/

  myConfigServerType = prevServerType;

  /* enum domain */
  if (strlen(enumdomain) > 0) {
    jstr = (*env)->NewStringUTF(env, enumdomain);
    (*env)->SetObjectField(env, systemConfig, sysEnumDomainFid, jstr);
    (*env)->DeleteLocalRef(env, jstr);
  }

  /* RADIUS server */
/*  if (strlen(rad_server_addr[0]) > 0) {
    jstr = (*env)->NewStringUTF(env, rad_server_addr[0]);
    (*env)->SetObjectField(env, systemConfig, sysRadiusServerFid, jstr);
    (*env)->DeleteLocalRef(env, jstr);
  }
*/
  /* RADIUS secret */
/*  if (strlen(secret[0]) > 0) {
    jstr = (*env)->NewStringUTF(env, secret[0]);
    (*env)->SetObjectField(env, systemConfig, sysRadiusSecretFid, jstr);
    (*env)->DeleteLocalRef(env, jstr);
  }
*/
  /* access control */
  (*env)->SetBooleanField(env, systemConfig, sysAllowAllSrcFid, allowSrcAll?JNI_TRUE:JNI_FALSE);
  (*env)->SetBooleanField(env, systemConfig, sysAllowAllDstFid, allowDestAll?JNI_TRUE:JNI_FALSE);
  (*env)->SetBooleanField(env, systemConfig, sysAllowHairpinCallsFid, allowHairPin?JNI_TRUE:JNI_FALSE);
  (*env)->SetBooleanField(env, systemConfig, sysRemoveRFC2833Fid, h323RemoveTcs2833?JNI_TRUE:JNI_FALSE);
  (*env)->SetBooleanField(env, systemConfig, sysRemoveT38Fid, h323RemoveTcsT38?JNI_TRUE:JNI_FALSE);
#if FCE_REMOVED
  (*env)->SetBooleanField(env, systemConfig, sysDefaultMediaRoutingFid, defaultMediaRouting?JNI_TRUE:JNI_FALSE);
  (*env)->SetBooleanField(env, systemConfig, sysDefaultMidCallMediaChangeFid, defaultHideAddressChange?JNI_TRUE:JNI_FALSE);
#endif
  (*env)->SetIntField(env, systemConfig, sysMaxHuntsFid, (jint)maxHunts);
  (*env)->SetIntField(env, systemConfig, sysMaxHuntsLimitFid, SYSTEM_MAX_HUNTS);
  (*env)->SetIntField(env, systemConfig, sysMaxHuntsAllowableDurationFid, (jint)max_hunt_allowable_duration);
  (*env)->SetBooleanField(env, systemConfig, sysAllowAllRtpFid, allowRtpAll?JNI_TRUE:JNI_FALSE);

  (*env)->SetBooleanField(env, systemConfig, sysForwardSrcAddrFid, forwardSrcAddr?JNI_TRUE:JNI_FALSE);
  (*env)->SetIntField(env, systemConfig, sysMaxCallDurationFid, (jint)max_call_duration);

  // msw name
  if (strlen(mswname) > 0) {
    jstr = (*env)->NewStringUTF(env, mswname);
    (*env)->SetObjectField(env, systemConfig, sysMswNameFid, jstr);
    (*env)->DeleteLocalRef(env, jstr);
  }
  
  //management interface 
  if (strlen(mgmtInterfaceIp) > 0) {
    jstr = (*env)->NewStringUTF(env, mgmtInterfaceIp);
    (*env)->SetObjectField(env, systemConfig, sysMgmtIpFid, jstr);
    (*env)->DeleteLocalRef(env, jstr);
  }

  //CodeMap fields
  (*env)->SetBooleanField(env, systemConfig, sysMapIsdnccFid, mapisdncc?JNI_TRUE:JNI_FALSE);
  (*env)->SetBooleanField(env, systemConfig, sysMapLrjReasonFid, maplrjreason?JNI_TRUE:JNI_FALSE);
  if (strlen(codemaptemplate) > 0) {
    jstr = (*env)->NewStringUTF(env, codemaptemplate);
    (*env)->SetObjectField(env, systemConfig, sysUseCodeMapFid, jstr);
    (*env)->DeleteLocalRef(env, jstr);
  }
  
  return 0;
}


/**
 * returns the IP address (InetAddress object) for the interface name specified
 */
static jobject
getIpForInterface (JNIEnv *env, char *ifName) {
  struct ifi_info *ifihead = NULL, *ptr;
  jobject addr = NULL;
  const char *cp;
  char buf[INET_ADDRSTRLEN];
  jstring jstr;

  for (ifihead = ptr = initIfs(); ptr != NULL; ptr = ptr->ifi_next) {
    if (strcmp(ifName, ptr->ifi_name))
      continue; // no match

    // found a match
    cp = inet_ntop(AF_INET, &ptr->ifi_addr->sin_addr, buf, INET_ADDRSTRLEN);
    if (cp == NULL)
      break;

    jstr = (*env)->NewStringUTF(env, cp);
    if (jstr == NULL)
      break;

    addr = (*env)->CallStaticObjectMethod(env, inetAddressClass, inetAddressGetByNameMid, jstr);
    (*env)->DeleteLocalRef(env, jstr);
  }

  free_ifi_info(ifihead);

  return addr;
}


static int
SetNetRedundsFields (JNIEnv *env, jobject redNetConfig) {
  jstring jstr;
  int i;
  jobject addr;

  /* cache the ip address of the interface when vip will be enabled */
  addr = getIpForInterface(env, ispd_primary.name);
  if (addr != NULL)
    fromIpAddr = (*env)->NewGlobalRef(env, addr);

  /* server type */
  if (ispd_type == ISPD_TYPE_STANDBY)
    jstr = (*env)->NewStringUTF(env, "standby");
  else if (ispd_type == ISPD_TYPE_ACTIVE)
    jstr = (*env)->NewStringUTF(env, "active");
  else
    jstr = (*env)->NewStringUTF(env, "disabled");
  (*env)->SetObjectField(env, redNetConfig, redNetServerTypeFid, jstr);
  (*env)->DeleteLocalRef(env, jstr);

  // vip is not used from 3.1 msw 
  // primary interface name 
/*
  jstr = (*env)->NewStringUTF(env, ispd_primary.name);
  (*env)->SetObjectField(env, redNetConfig, redNetPrimIfNameFid, jstr);
  (*env)->DeleteLocalRef(env, jstr);

  // secondary interface name 
  jstr = (*env)->NewStringUTF(env, ispd_secondary.name);
  (*env)->SetObjectField(env, redNetConfig, redNetSecondIfNameFid, jstr);
  (*env)->DeleteLocalRef(env, jstr);


  // primary interface vips
  // clear the list
  (*env)->CallVoidMethod(env, redNetConfig, redNetPrimaryInterfaceVipsClearMid);
  if (CheckAndThrowBridgeException(env))
    return -1;
  // add items to the list
  (*env)->CallVoidMethod(env, redNetConfig, redNetPrimaryInterfaceVipAddMid, (*env)->NewStringUTF(env, (const char*)&ispd_primary.vip));
  if (CheckAndThrowBridgeException(env))
    return -1;

  // secondary interface vips
  // clear the list
  (*env)->CallVoidMethod(env, redNetConfig, redNetSecondaryInterfaceVipsClearMid);
  if (CheckAndThrowBridgeException(env))
    return -1;
  // add items to the list
  (*env)->CallVoidMethod(env, redNetConfig, redNetSecondaryInterfaceVipAddMid, (*env)->NewStringUTF(env, (const char*)&ispd_secondary.vip));
  if (CheckAndThrowBridgeException(env))
    return -1;


  // primary interface router 
  jstr = (*env)->NewStringUTF(env, ispd_primary.router);
  (*env)->SetObjectField(env, redNetConfig, redNetPrimIfRouterFid, jstr);
  (*env)->DeleteLocalRef(env, jstr);

  // secondary interface router 
  jstr = (*env)->NewStringUTF(env, ispd_secondary.router);
  (*env)->SetObjectField(env, redNetConfig, redNetSecondIfRouterFid, jstr);
  (*env)->DeleteLocalRef(env, jstr);

*/
  /* control interface name */
  jstr = (*env)->NewStringUTF(env, ispd_ctl.name);
  (*env)->SetObjectField(env, redNetConfig, redNetControlIfNameFid, jstr);
  (*env)->DeleteLocalRef(env, jstr);

  (*env)->SetBooleanField(env, redNetConfig, redNetScmFid, doScm?JNI_TRUE:JNI_FALSE);


  /* peer iservers */
  // clear the list
  (*env)->CallVoidMethod(env, redNetConfig, redNetPeersClearMid);
  if (CheckAndThrowBridgeException(env))
    return -1;
  // add items to the list
  for (i = 0; i < ispd_ctl.peer_count; i++) {
    (*env)->CallVoidMethod(env, redNetConfig, redNetPeersAddMid, (*env)->NewStringUTF(env, &ispd_ctl.peer_iservers[i][0]));
    if (CheckAndThrowBridgeException(env))
      return -1;
  }

  return 0;
}


static int
SetDbRedundsFields (JNIEnv *env, jobject redDbConfig) {
  jstring jstr;

  /* server status */

  (*env)->SetBooleanField(env, redDbConfig, redDbServerStatusFid, RSDConfig?JNI_TRUE:JNI_FALSE);

  /* interface name */
  jstr = (*env)->NewStringUTF(env, rs_ifname);
  (*env)->SetObjectField(env, redDbConfig, redDbIfNameFid, jstr);
  (*env)->DeleteLocalRef(env, jstr);

  /* mcast address */
  jstr = (*env)->NewStringUTF(env, rs_mcast_addr);
  (*env)->SetObjectField(env, redDbConfig, redDbMcastAddrFid, jstr);
  (*env)->DeleteLocalRef(env, jstr);

  /* port */
  (*env)->SetIntField(env, redDbConfig, redDbPortFid, (jint)atoi(rs_port));

  /* priority */
  (*env)->SetIntField(env, redDbConfig, redDbPriorityFid, (jint)rs_host_prio);

  return 0;
}


static int
SetRedundsFields (JNIEnv *env, jobject redConfig) {
  jobject redNetConfig, redDbConfig;
  int status;

  redNetConfig = (*env)->GetObjectField(env, redConfig, redNetConfigFid);
  if (redNetConfig == NULL) {
    LOCAL_DEBUG(JNI_TRUE, DEBUG_ERROR, ("error getting net redundancy config object"));
    (*env)->PopLocalFrame(env, NULL);
    return -1;
  }
  redDbConfig = (*env)->GetObjectField(env, redConfig, redDbConfigFid);
  if (redDbConfig == NULL) {
    LOCAL_DEBUG(JNI_TRUE, DEBUG_ERROR, ("error getting db redundancy config object"));
    (*env)->PopLocalFrame(env, NULL);
    return -1;
  }

  status = SetNetRedundsFields(env, redNetConfig);
  if (status == 0)
    status = SetDbRedundsFields(env, redDbConfig);

  (*env)->DeleteLocalRef(env, redNetConfig);
  (*env)->DeleteLocalRef(env, redDbConfig);

  return status;
}


static int
SetLoggingFields (JNIEnv *env, jobject loggingConfig) {
  int i, j, k;
  char temp[1024];
  int prevServerType  = myConfigServerType;
  int match, moduleVal;
  jstring module;
  int retval = 0;
  
  for (i = 0; i < MAX_LOG_SERVERS; i++) {
    myConfigServerType = logServers[i];
    match = FindServerConfig();
    if (match != -1) {
      memset(temp, 0, sizeof(temp));

      for (k = 0, j = 0; j < MNETLOGMAX; j++) {
	      moduleVal = serplexes[match].debconfigs.netLogStatus[j];
	      temp[k++] = (moduleVal)>0?'1':'0';
	      temp[k++] = ';';
      }
      temp[k] = '\0';

      module = (*env)->NewStringUTF(env, temp);
      if (module == NULL) {
	myConfigServerType = prevServerType;
	return -1;
      } else {
	if ((*env)->CallBooleanMethod(env, loggingConfig, logSetModMid, (jint)myConfigServerType, module) == JNI_FALSE) {
	  LOCAL_DEBUG(JNI_TRUE, DEBUG_ERROR, ("Error setting logging config into the config object"));
	  retval = -1;
	}
	(*env)->DeleteLocalRef(env, module);
      }
    }
  }
  myConfigServerType = CONFIG_SERPLEX_GIS;
  match = FindServerConfig();
  if(match  !=  -1){

    (*env)->SetBooleanField(env, loggingConfig, logUpdateAllocationFid, updateAllocations?JNI_TRUE:JNI_FALSE);
    if(serplexes[match].cusconfigs.level >= 3){
      if(serplexes[match].cusconfigs.customLogStatus[TPKTCHAN] > 0)
        (*env)->SetBooleanField(env, loggingConfig, logTpktchanFid, JNI_TRUE);
      if(serplexes[match].cusconfigs.customLogStatus[UDPCHAN] > 0)
        (*env)->SetBooleanField(env, loggingConfig, logUdpchanFid, JNI_TRUE);
      if(serplexes[match].cusconfigs.customLogStatus[PERERR] > 0)
        (*env)->SetBooleanField(env, loggingConfig, logPererrFid, JNI_TRUE);
      if(serplexes[match].cusconfigs.customLogStatus[CM] > 0)
        (*env)->SetBooleanField(env, loggingConfig, logCmFid, JNI_TRUE);
      if(serplexes[match].cusconfigs.customLogStatus[CMAPICB] > 0)
        (*env)->SetBooleanField(env, loggingConfig, logCmapicbFid, JNI_TRUE);
      if(serplexes[match].cusconfigs.customLogStatus[CMAPI] > 0)
        (*env)->SetBooleanField(env, loggingConfig, logCmapiFid, JNI_TRUE);
      if(serplexes[match].cusconfigs.customLogStatus[CMERR] > 0)
        (*env)->SetBooleanField(env, loggingConfig, logCmerrFid, JNI_TRUE);
      if(serplexes[match].cusconfigs.customLogStatus[LI] > 0)
        (*env)->SetBooleanField(env, loggingConfig, logLiFid, JNI_TRUE);
      if(serplexes[match].cusconfigs.customLogStatus[LIINFO] > 0)
        (*env)->SetBooleanField(env, loggingConfig, logLiinfoFid, JNI_TRUE);
      (*env)->SetIntField(env, loggingConfig, logDebugLevelFid, serplexes[match].cusconfigs.level);
    }
    //if(serplexes[match].cusconfigs.slevel >= 0) {
      (*env)->SetIntField(env, loggingConfig, logSDebugLevelFid, serplexes[match].cusconfigs.slevel);
    //}
  }

  myConfigServerType  = prevServerType;

  return retval;
}



/** returns a new config object (iServerConfig) */
static jobject
CreateNewConfigObject (JNIEnv *env) {
  jobject config, obj;

  if ((*env)->PushLocalFrame(env, 20))
    return NULL;

  LOCAL_DEBUG(JNI_TRUE, DEBUG_VERBOSE, ("creating new iserver config object"));
  config = (*env)->NewObject(env, iscClass, iscConstId);

  /* now fill in the sip configuration */
  LOCAL_DEBUG(JNI_TRUE, DEBUG_VERBOSE, ("creating new sip config object"));
  obj = (*env)->GetObjectField(env, config, sipConfigFid);
  if (obj == NULL) {
    LOCAL_DEBUG(JNI_TRUE, DEBUG_ERROR, ("error getting sipConfig object"));
    (*env)->PopLocalFrame(env, NULL);
    return NULL;
  } 
  if (SetSipFields(env, obj)) {
    (*env)->PopLocalFrame(env, NULL);
    return NULL;
  }
  (*env)->DeleteLocalRef(env, obj);

  /* now fill in the H.323 configuration */
  LOCAL_DEBUG(JNI_TRUE, DEBUG_VERBOSE, ("creating new H.323 config object"));
  obj = (*env)->GetObjectField(env, config, h323ConfigFid);
  if (obj == NULL) {
    LOCAL_DEBUG(JNI_TRUE, DEBUG_ERROR, ("error getting h323Config object"));
    (*env)->PopLocalFrame(env, NULL);
    return NULL;
  }
  if (SetH323Fields(env, obj)) {
    (*env)->PopLocalFrame(env, NULL);
    return NULL;
  }
  (*env)->DeleteLocalRef(env, obj);

  /* now fill in the FCE configuration */
  LOCAL_DEBUG(JNI_TRUE, DEBUG_VERBOSE, ("creating new fce config object"));
  obj = (*env)->GetObjectField(env, config, fceConfigFid);
  if (obj == NULL) {
    LOCAL_DEBUG(JNI_TRUE, DEBUG_ERROR, ("error getting fceConfig object"));
    (*env)->PopLocalFrame(env, NULL);
    return NULL;
  }
  if (SetFceFields(env, obj)) {
    (*env)->PopLocalFrame(env, NULL);
    return NULL;
  }
  (*env)->DeleteLocalRef(env, obj);


  /* now fill in the FCE New configuration */
  LOCAL_DEBUG(JNI_TRUE, DEBUG_VERBOSE, ("creating new fce config new object"));
  obj = (*env)->GetObjectField(env, config, fceConfigNewFid);
  if (obj == NULL) {
    LOCAL_DEBUG(JNI_TRUE, DEBUG_ERROR, ("error getting fceConfigNew object"));
    (*env)->PopLocalFrame(env, NULL);
    return NULL;
  }
  if (SetFceNewFields(env, obj)) {
    (*env)->PopLocalFrame(env, NULL);
    return NULL;
  }
  (*env)->DeleteLocalRef(env, obj);

  /* now fill in the billing configuration */
  LOCAL_DEBUG(JNI_TRUE, DEBUG_VERBOSE, ("creating new billing config object"));
  obj = (*env)->GetObjectField(env, config, billingConfigFid);
  if (obj == NULL) {
    LOCAL_DEBUG(JNI_TRUE, DEBUG_ERROR, ("error getting billingConfig object"));
    (*env)->PopLocalFrame(env, NULL);
    return NULL;
  }
  if (SetBillingFields(env, obj)) {
    (*env)->PopLocalFrame(env, NULL);
    return NULL;
  }
  (*env)->DeleteLocalRef(env, obj);

  /* now fill in the advanced configuration */
  LOCAL_DEBUG(JNI_TRUE, DEBUG_VERBOSE, ("creating new advanced config object"));
  obj = (*env)->GetObjectField(env, config, advancedConfigFid);
  if (obj == NULL) {
    LOCAL_DEBUG(JNI_TRUE, DEBUG_ERROR, ("error getting advancedConfig object"));
    (*env)->PopLocalFrame(env, NULL);
    return NULL;
  }
  if (SetAdvancedFields(env, obj)) {
    (*env)->PopLocalFrame(env, NULL);
    return NULL;
  }
  (*env)->DeleteLocalRef(env, obj);

  /* now fill in the system configuration */
  LOCAL_DEBUG(JNI_TRUE, DEBUG_VERBOSE, ("creating new system config object"));
  obj = (*env)->GetObjectField(env, config, systemConfigFid);
  if (obj == NULL) {
    LOCAL_DEBUG(JNI_TRUE, DEBUG_ERROR, ("error getting systemConfig object"));
    (*env)->PopLocalFrame(env, NULL);
    return NULL;
  }
  if (SetSystemFields(env, obj)) {
    (*env)->PopLocalFrame(env, NULL);
    return NULL;
  }
  (*env)->DeleteLocalRef(env, obj);

  /* now fill in the redundancy configuration */
  LOCAL_DEBUG(JNI_TRUE, DEBUG_VERBOSE, ("creating new redundancy config object"));
  obj = (*env)->GetObjectField(env, config, redConfigFid);
  if (obj == NULL) {
    LOCAL_DEBUG(JNI_TRUE, DEBUG_ERROR, ("error getting redundancyConfig object"));
    (*env)->PopLocalFrame(env, NULL);
    return NULL;
  }
  if (SetRedundsFields(env, obj)) {
    (*env)->PopLocalFrame(env, NULL);
    return NULL;
  }
  (*env)->DeleteLocalRef(env, obj);

  /* now fill in the logging configuration */
  LOCAL_DEBUG(JNI_TRUE, DEBUG_VERBOSE, ("creating new logging config object"));
  obj = (*env)->GetObjectField(env, config, loggingConfigFid);
  if (obj == NULL) {
    LOCAL_DEBUG(JNI_TRUE, DEBUG_ERROR, ("error getting loggingConfig object"));
    (*env)->PopLocalFrame(env, NULL);
    return NULL;
  }
  if (SetLoggingFields(env, obj)) {
    (*env)->PopLocalFrame(env, NULL);
    return NULL;
  }
  (*env)->DeleteLocalRef(env, obj);



  /* now fill in the radius configuration */
  LOCAL_DEBUG(JNI_TRUE, DEBUG_VERBOSE, ("creating new radius config object"));
  obj = (*env)->GetObjectField(env, config, radiusConfigFid);
  if (obj == NULL) {
    LOCAL_DEBUG(JNI_TRUE, DEBUG_ERROR, ("error getting radiusConfig object"));
    (*env)->PopLocalFrame(env, NULL);
    return NULL;
  }
  if (SetRadiusFields(env, obj)) {
    (*env)->PopLocalFrame(env, NULL);
    return NULL;
  }
  (*env)->DeleteLocalRef(env, obj);


  LOCAL_DEBUG(JNI_TRUE, DEBUG_VERBOSE, ("iserverconfig: created new iserver config object:"));
 // printObject(env, DEBUG_VERBOSE, config);

  return (*env)->PopLocalFrame(env, config);
}


static void
CreateConfig (JNIEnv *env) {
  jobject config;
  config = CreateNewConfigObject(env);
  if (config == NULL) {
    LOCAL_DEBUG(JNI_TRUE, DEBUG_VERBOSE, ("Error creating config object"));
  } else {
    configObject = (*env)->NewGlobalRef(env, config);
    (*env)->DeleteLocalRef(env, config);
  }
}


JNIEXPORT void JNICALL
Java_com_nextone_JServer_JServer_serverCfgInit (JNIEnv *env, jobject obj) {
  setConfigFile();
  DoConfig(JserverProcessConfig);

  if (configObject != NULL) {
    (*env)->DeleteGlobalRef(env, configObject);
    configObject = NULL;
  }
  if (fromIpAddr != NULL) {
    (*env)->DeleteGlobalRef(env, fromIpAddr);
    fromIpAddr = NULL;
  }

  CreateConfig(env);
}


JNIEXPORT jobject JNICALL
Java_com_nextone_JServer_BridgeServer_getNativeIserverConfig (JNIEnv *env, jobject obj) {
  if (configObject == NULL){
    CreateConfig(env);
  }
  if (configObject == NULL) {
    ThrowBridgeException(env, "Error accessing iServer configuration");
    return NULL;
  }

  LOCAL_DEBUG(JNI_TRUE, DEBUG_VERBOSE, ("iserverconfig: returning iserver config object"));
 // printObject(env, DEBUG_VERBOSE, configObject);
  return configObject;
}


JNIEXPORT jobject JNICALL
Java_com_nextone_JServer_JServer_getRedundPrimaryIfAddress (JNIEnv *env, jobject obj) {
  return fromIpAddr;
}


int
iserverConfigIdInit (JNIEnv *env, jobject config) {
  jobject obj, redObj;
  jclass cls, fceIPMaskClass, fceNetworkListClass, redCls;


  iscClass = GetGlobalClassReference(env, "com/nextone/common/iServerConfig");
  if (iscClass == NULL)
    return -1;

  iscConstId = (*env)->GetMethodID(env, iscClass, "<init>", "()V");
  if (iscConstId == NULL)
    return -1;

  /* sip configuration */
  sipConfigFid = (*env)->GetFieldID(env, iscClass, "sipConfig", "Lcom/nextone/common/iServerConfig$SipConfig;");
  if (sipConfigFid == NULL)
    return -1;
  obj = (*env)->GetObjectField(env, config, sipConfigFid);
  if (obj == NULL)
    return -1;
  cls = (*env)->GetObjectClass(env, obj);
  if (cls == NULL) {
    (*env)->DeleteLocalRef(env, obj);
    return -1;
  }

  scServerNameFid = (*env)->GetFieldID(env, cls, "serverName", "Ljava/lang/String;");
  if (scServerNameFid == NULL) {
    (*env)->DeleteLocalRef(env, obj);
    (*env)->DeleteLocalRef(env, cls);
    return -1;
  }
  scSipDomainFid = (*env)->GetFieldID(env, cls, "sipDomain", "Ljava/lang/String;");
  if (scSipDomainFid == NULL) {
    (*env)->DeleteLocalRef(env, obj);
    (*env)->DeleteLocalRef(env, cls);
    return -1;
  }
  /*scAuthEnabledFid = (*env)->GetFieldID(env, cls, "authEnabled", "Z");
  if (scAuthEnabledFid == NULL) {
    (*env)->DeleteLocalRef(env, obj);
    (*env)->DeleteLocalRef(env, cls);
    return -1;
  }
*/

  scAuthFid = (*env)->GetFieldID(env, cls, "auth", "I");
  if (scAuthFid == NULL) {
    (*env)->DeleteLocalRef(env, obj);
    (*env)->DeleteLocalRef(env, cls);
    return -1;
  }
  scAuthPasswordFid = (*env)->GetFieldID(env, cls, "authPassword", "Ljava/lang/String;");
  if (scAuthPasswordFid == NULL) {
    (*env)->DeleteLocalRef(env, obj);
    (*env)->DeleteLocalRef(env, cls);
    return -1;
  }
  scRecordRouteEnabledFid = (*env)->GetFieldID(env, cls, "recordRouteEnabled", "Z");
  if (scRecordRouteEnabledFid == NULL) {
    (*env)->DeleteLocalRef(env, obj);
    (*env)->DeleteLocalRef(env, cls);
    return -1;
  }

  scOBPFid = (*env)->GetFieldID(env, cls, "obpEnabled", "Z");
  if (scOBPFid == NULL) {
    (*env)->DeleteLocalRef(env, obj);
    (*env)->DeleteLocalRef(env, cls);
    return -1;
  }
  scDynamicEPFid = (*env)->GetFieldID(env, cls, "dynamicEPAllowed", "Z");
  if (scDynamicEPFid == NULL) {
    (*env)->DeleteLocalRef(env, obj);
    (*env)->DeleteLocalRef(env, cls);
    return -1;
  }
  scInternalCallFid = (*env)->GetFieldID(env, cls, "internalCallAllowed", "Z");
  if (scInternalCallFid == NULL) {
    (*env)->DeleteLocalRef(env, obj);
    (*env)->DeleteLocalRef(env, cls);
    return -1;
  }

  scServerTypeFid = (*env)->GetFieldID(env, cls, "serverType", "I");
  if (scServerTypeFid == NULL) {
    (*env)->DeleteLocalRef(env, obj);
    (*env)->DeleteLocalRef(env, cls);
    return -1;
  }
  scMaxForwardsFid = (*env)->GetFieldID(env, cls, "maxForwards", "I");
  if (scMaxForwardsFid == NULL) {
    (*env)->DeleteLocalRef(env, obj);
    (*env)->DeleteLocalRef(env, cls);
    return -1;
  }
  scSipTimerCFid = (*env)->GetFieldID(env, cls, "sipTimerC", "I");
  if (scSipTimerCFid == NULL) {
    (*env)->DeleteLocalRef(env, obj);
    (*env)->DeleteLocalRef(env, cls);
    return -1;
  }
  scSipPortFid = (*env)->GetFieldID(env, cls, "sipPort", "I");
  if (scSipPortFid == NULL) {
    (*env)->DeleteLocalRef(env, obj);
    (*env)->DeleteLocalRef(env, cls);
    return -1;
  }
  (*env)->DeleteLocalRef(env, obj);
  (*env)->DeleteLocalRef(env, cls);

  /* H.232 configuration */
  h323ConfigFid = (*env)->GetFieldID(env, iscClass, "h323Config", "Lcom/nextone/common/iServerConfig$H323Config;");
  if (h323ConfigFid == NULL)
    return -1;
  obj = (*env)->GetObjectField(env, config, h323ConfigFid);
  if (obj == NULL)
    return -1;
  cls = (*env)->GetObjectClass(env, obj);
  if (cls == NULL) {
    (*env)->DeleteLocalRef(env, obj);
    return -1;
  }

  h323SgkRegPrefixFid = (*env)->GetFieldID(env, cls, "sgkRegPrefix", "Ljava/lang/String;");
  if (h323SgkRegPrefixFid == NULL) {
    (*env)->DeleteLocalRef(env, obj);
    (*env)->DeleteLocalRef(env, cls);
    return -1;
  }
  h323GkIdFid = (*env)->GetFieldID(env, cls, "gkId", "Ljava/lang/String;");
  if (h323GkIdFid == NULL) {
    (*env)->DeleteLocalRef(env, obj);
    (*env)->DeleteLocalRef(env, cls);
    return -1;
  }
  h323RrqTimerFid = (*env)->GetFieldID(env, cls, "rrqTimer", "I");
  if (h323RrqTimerFid == NULL) {
    (*env)->DeleteLocalRef(env, obj);
    (*env)->DeleteLocalRef(env, cls);
    return -1;
  }
  h323RrqTtlFid = (*env)->GetFieldID(env, cls, "rrqTtl", "I");
  if (h323RrqTtlFid == NULL) {
    (*env)->DeleteLocalRef(env, obj);
    (*env)->DeleteLocalRef(env, cls);
    return -1;
  }
  h323H245RoutingEnabledFid = (*env)->GetFieldID(env, cls, "h245RoutingEnabled", "Z");
  if (h323H245RoutingEnabledFid == NULL) {
    (*env)->DeleteLocalRef(env, obj);
    (*env)->DeleteLocalRef(env, cls);
    return -1;
  }
  h323ForceH245Fid = (*env)->GetFieldID(env, cls, "forceH245", "Z");
  if (h323ForceH245Fid == NULL) {
    (*env)->DeleteLocalRef(env, obj);
    (*env)->DeleteLocalRef(env, cls);
    return -1;
  }
  h323CallRoutingEnabledFid = (*env)->GetFieldID(env, cls, "callRoutingEnabled", "Z");
  if (h323CallRoutingEnabledFid == NULL) {
    (*env)->DeleteLocalRef(env, obj);
    (*env)->DeleteLocalRef(env, cls);
    return -1;
  }
  h323FastStartEnabledFid = (*env)->GetFieldID(env, cls, "fastStartEnabled", "Z");
  if (h323FastStartEnabledFid == NULL) {
    (*env)->DeleteLocalRef(env, obj);
    (*env)->DeleteLocalRef(env, cls);
    return -1;
  }

  h323CpsFid = (*env)->GetFieldID(env, cls, "cps", "I");
  if (h323CpsFid == NULL) {
    (*env)->DeleteLocalRef(env, obj);
    (*env)->DeleteLocalRef(env, cls);
    return -1;
  }

  h323InstancesFid = (*env)->GetFieldID(env, cls, "instances", "I");
  if (h323InstancesFid == NULL) {
    (*env)->DeleteLocalRef(env, obj);
    (*env)->DeleteLocalRef(env, cls);
    return -1;
  }

  h323SgkMaxCallsFid = (*env)->GetFieldID(env, cls, "sgkMaxCalls", "I");
  if (h323SgkMaxCallsFid == NULL) {
    (*env)->DeleteLocalRef(env, obj);
    (*env)->DeleteLocalRef(env, cls);
    return -1;
  }

  h323MaxCallsFid = (*env)->GetFieldID(env, cls, "maxCalls", "I");
  if (h323MaxCallsFid == NULL) {
    (*env)->DeleteLocalRef(env, obj);
    (*env)->DeleteLocalRef(env, cls);
    return -1;
  }

  h323LocalProceedingFid = (*env)->GetFieldID(env, cls, "localProceeding", "Z");
  if (h323LocalProceedingFid == NULL) {
    (*env)->DeleteLocalRef(env, obj);
    (*env)->DeleteLocalRef(env, cls);
    return -1;
  }

  h323InfoTransCapFid = (*env)->GetFieldID(env, cls, "infoTransCap", "I");
  if (h323InfoTransCapFid == NULL) {
    (*env)->DeleteLocalRef(env, obj);
    (*env)->DeleteLocalRef(env, cls);
    return -1;
  }

  h323AllowDestArqEnabledFid= (*env)->GetFieldID(env, cls, "allowDestArqEnabled", "Z");
  if (h323AllowDestArqEnabledFid == NULL) {
    (*env)->DeleteLocalRef(env, obj);
    (*env)->DeleteLocalRef(env, cls);
    return -1;
  }

  h323AllowAuthArqEnabledFid= (*env)->GetFieldID(env, cls, "allowAuthArqEnabled", "Z");
  if (h323AllowAuthArqEnabledFid == NULL) {
    (*env)->DeleteLocalRef(env, obj);
    (*env)->DeleteLocalRef(env, cls);
    return -1;
  }

  h323H245TunnelEnabledFid= (*env)->GetFieldID(env, cls, "h245TunnelEnabled", "Z");
  if (h323H245TunnelEnabledFid == NULL) {
    (*env)->DeleteLocalRef(env, obj);
    (*env)->DeleteLocalRef(env, cls);
    return -1;
  }

  (*env)->DeleteLocalRef(env, obj);
  (*env)->DeleteLocalRef(env, cls);

#if FCE_REMOVED

  /* FCE configuration */
  fceConfigFid = (*env)->GetFieldID(env, iscClass, "fceConfig", "Lcom/nextone/common/iServerConfig$FceConfig;");
  if (fceConfigFid == NULL)
    return -1;
  obj = (*env)->GetObjectField(env, config, fceConfigFid);
  if (obj == NULL)
    return -1;
  cls = (*env)->GetObjectClass(env, obj);
  if (cls == NULL) {
    (*env)->DeleteLocalRef(env, obj);
    return -1;
  }

  fceFwNameFid = (*env)->GetFieldID(env, cls, "fwName", "Ljava/lang/String;");
  if (fceFwNameFid == NULL) {
    (*env)->DeleteLocalRef(env, obj);
    (*env)->DeleteLocalRef(env, cls);
    return -1;
  }
  fceH245Fid = (*env)->GetFieldID(env, cls, "h245Enabled", "Z");
  if (fceH245Fid == NULL) {
    (*env)->DeleteLocalRef(env, obj);
    (*env)->DeleteLocalRef(env, cls);
    return -1;
  }
  fceDefaultPublicFid = (*env)->GetFieldID(env, cls, "isPublic", "Z");
  if (fceDefaultPublicFid == NULL) {
    (*env)->DeleteLocalRef(env, obj);
    (*env)->DeleteLocalRef(env, cls);
    return -1;
  }
  fceSetFwConnectAddrMid = (*env)->GetMethodID(env, cls, "setFirewallConnectAddr", "(I)V");
  if (fceSetFwConnectAddrMid == NULL) {
    (*env)->DeleteLocalRef(env, obj);
    (*env)->DeleteLocalRef(env, cls);
    return -1;
  }
  fceGetFwConnectAddrMid = (*env)->GetMethodID(env, cls, "getFirewallConnectAddrAsInt", "()I");
  if (fceGetFwConnectAddrMid == NULL) {
    (*env)->DeleteLocalRef(env, obj);
    (*env)->DeleteLocalRef(env, cls);
    return -1;
  }
  fceAddNetworkListMid = (*env)->GetMethodID(env, cls, "addNetworkList", "(IIZ)V");
  if (fceAddNetworkListMid == NULL) {
    (*env)->DeleteLocalRef(env, obj);
    (*env)->DeleteLocalRef(env, cls);
    return -1;
  }
  fceGetNetworkListMid = (*env)->GetMethodID(env, cls, "getNetworkListAsArray", "()[Lcom/nextone/common/NetworkList;");
  if (fceGetNetworkListMid == NULL) {
    (*env)->DeleteLocalRef(env, obj);
    (*env)->DeleteLocalRef(env, cls);
    return -1;
  }
  fceNetworkListClass = (*env)->FindClass(env, "com/nextone/common/NetworkList");
  if (fceNetworkListClass == NULL) {
    (*env)->DeleteLocalRef(env, obj);
    (*env)->DeleteLocalRef(env, cls);
    return -1;
  }
  fceNetworkListIPMaskFid = (*env)->GetFieldID(env, fceNetworkListClass, "ipmask", "Lcom/nextone/util/IPMask;");
  if (fceNetworkListIPMaskFid == NULL) {
    (*env)->DeleteLocalRef(env, obj);
    (*env)->DeleteLocalRef(env, cls);
    return -1;
  }
  fceNetworkListIsPublicFid = (*env)->GetFieldID(env, fceNetworkListClass, "isPublic", "Z");
  if (fceNetworkListIsPublicFid == NULL) {
    (*env)->DeleteLocalRef(env, obj);
    (*env)->DeleteLocalRef(env, cls);
    return -1;
  }
  fceIPMaskClass = (*env)->FindClass(env, "com/nextone/util/IPMask");
  if (fceIPMaskClass == NULL) {
    (*env)->DeleteLocalRef(env, obj);
    (*env)->DeleteLocalRef(env, cls);
    return -1;
  }
  fceIPMaskGetIpMid = (*env)->GetMethodID(env, fceIPMaskClass, "getIPAsString", "()Ljava/lang/String;");
  if (fceIPMaskGetIpMid == NULL) {
    (*env)->DeleteLocalRef(env, obj);
    (*env)->DeleteLocalRef(env, cls);
    return -1;
  }
  fceIPMaskGetMaskMid = (*env)->GetMethodID(env, fceIPMaskClass, "getMaskAsString", "()Ljava/lang/String;");
  if (fceIPMaskGetMaskMid == NULL) {
    (*env)->DeleteLocalRef(env, obj);
    (*env)->DeleteLocalRef(env, cls);
    return -1;
  }
  (*env)->DeleteLocalRef(env, obj);
  (*env)->DeleteLocalRef(env, cls);
  (*env)->DeleteLocalRef(env, fceIPMaskClass);
#endif

  /* FCE New configuration */
  fceConfigNewFid = (*env)->GetFieldID(env, iscClass, "fceConfigNew", "Lcom/nextone/common/iServerConfig$FceConfigNew;");
  if (fceConfigNewFid == NULL)
    return -1;
  obj = (*env)->GetObjectField(env, config, fceConfigNewFid);
  if (obj == NULL)
    return -1;
  cls = (*env)->GetObjectClass(env, obj);
  if (cls == NULL) {
    (*env)->DeleteLocalRef(env, obj);
    return -1;
  }

  fceNewFwNameFid = (*env)->GetFieldID(env, cls, "fwName", "Ljava/lang/String;");
  if (fceNewFwNameFid == NULL) {
    (*env)->DeleteLocalRef(env, obj);
    (*env)->DeleteLocalRef(env, cls);
    return -1;
  }

  fceNewSetFwAddressMid = (*env)->GetMethodID(env, cls, "setFirewallAddress", "(I)V");
  if (fceNewSetFwAddressMid == NULL) {
    (*env)->DeleteLocalRef(env, obj);
    (*env)->DeleteLocalRef(env, cls);
    return -1;
  }

  fceNewGetFwAddressMid = (*env)->GetMethodID(env, cls, "getFirewallAddressAsInt", "()I");
  if (fceNewGetFwAddressMid == NULL) {
    (*env)->DeleteLocalRef(env, obj);
    (*env)->DeleteLocalRef(env, cls);
    return -1;
  }

  (*env)->DeleteLocalRef(env, obj);
  (*env)->DeleteLocalRef(env, cls);


  /* billing configuration */
  billingConfigFid = (*env)->GetFieldID(env, iscClass, "billingConfig", "Lcom/nextone/common/iServerConfig$BillingConfig;");
  if (billingConfigFid == NULL)
    return -1;
  obj = (*env)->GetObjectField(env, config, billingConfigFid);
  if (obj == NULL)
    return -1;
  cls = (*env)->GetObjectClass(env, obj);
  if (cls == NULL) {
    (*env)->DeleteLocalRef(env, obj);
    return -1;
  }

  billCdrTypeFid = (*env)->GetFieldID(env, cls, "cdrType", "I");
  if (billCdrTypeFid == NULL) {
    (*env)->DeleteLocalRef(env, obj);
    (*env)->DeleteLocalRef(env, cls);
    return -1;
  }
  billCdrFormatFid = (*env)->GetFieldID(env, cls, "cdrFormat", "I");
  if (billCdrFormatFid == NULL) {
    (*env)->DeleteLocalRef(env, obj);
    (*env)->DeleteLocalRef(env, cls);
    return -1;
  }
  billDirFid = (*env)->GetFieldID(env, cls, "dir", "Ljava/lang/String;");
  if (billDirFid == NULL) {
    (*env)->DeleteLocalRef(env, obj);
    (*env)->DeleteLocalRef(env, cls);
    return -1;
  }
  billCdrTimerFid = (*env)->GetFieldID(env, cls, "cdrTimer", "I");
  if (billCdrTimerFid == NULL) {
    (*env)->DeleteLocalRef(env, obj);
    (*env)->DeleteLocalRef(env, cls);
    return -1;
  }
  billBillingTypeFid = (*env)->GetFieldID(env, cls, "billingType", "I");
  if (billBillingTypeFid == NULL) {
    (*env)->DeleteLocalRef(env, obj);
    (*env)->DeleteLocalRef(env, cls);
    return -1;
  }
  billCdrLogStart1Fid = (*env)->GetFieldID(env, cls, "cdrLogStart1", "Z");
  if (billCdrLogStart1Fid == NULL) {
    (*env)->DeleteLocalRef(env, obj);
    (*env)->DeleteLocalRef(env, cls);
    return -1;
  }
  billCdrLogStart2Fid = (*env)->GetFieldID(env, cls, "cdrLogStart2", "Z");
  if (billCdrLogStart2Fid == NULL) {
    (*env)->DeleteLocalRef(env, obj);
    (*env)->DeleteLocalRef(env, cls);
    return -1;
  }
  billCdrLogEnd2Fid = (*env)->GetFieldID(env, cls, "cdrLogEnd2", "Z");
  if (billCdrLogEnd2Fid == NULL) {
    (*env)->DeleteLocalRef(env, obj);
    (*env)->DeleteLocalRef(env, cls);
    return -1;
  }
  billCdrLogHuntFid = (*env)->GetFieldID(env, cls, "cdrLogHunt", "Z");
  if (billCdrLogHuntFid == NULL) {
    (*env)->DeleteLocalRef(env, obj);
    (*env)->DeleteLocalRef(env, cls);
    return -1;
  }
  (*env)->DeleteLocalRef(env, obj);
  (*env)->DeleteLocalRef(env, cls);

  /* advanced configuration */
  advancedConfigFid = (*env)->GetFieldID(env, iscClass, "advancedConfig", "Lcom/nextone/common/iServerConfig$AdvancedConfig;");
  if (advancedConfigFid == NULL)
    return -1;
  obj = (*env)->GetObjectField(env, config, advancedConfigFid);
  if (obj == NULL)
    return -1;
  cls = (*env)->GetObjectClass(env, obj);
  if (cls == NULL) {
    (*env)->DeleteLocalRef(env, obj);
    return -1;
  }

  adNumSegmentsFid = (*env)->GetFieldID(env, cls, "numSegments", "I");
  if (adNumSegmentsFid == NULL) {
    (*env)->DeleteLocalRef(env, obj);
    (*env)->DeleteLocalRef(env, cls);
    return -1;
  }
  adSegSizeFid = (*env)->GetFieldID(env, cls, "segSize", "I");
  if (adSegSizeFid == NULL) {
    (*env)->DeleteLocalRef(env, obj);
    (*env)->DeleteLocalRef(env, cls);
    return -1;
  }
  adPriorityFid = (*env)->GetFieldID(env, cls, "priority", "I");
  if (adPriorityFid == NULL) {
    (*env)->DeleteLocalRef(env, obj);
    (*env)->DeleteLocalRef(env, cls);
    return -1;
  }
  adNumThreadsFid = (*env)->GetFieldID(env, cls, "numThreads", "I");
  if (adNumThreadsFid == NULL) {
    (*env)->DeleteLocalRef(env, obj);
    (*env)->DeleteLocalRef(env, cls);
    return -1;
  }
  adThreadStackSizeFid = (*env)->GetFieldID(env, cls, "threadStackSize", "I");
  if (adThreadStackSizeFid == NULL) {
    (*env)->DeleteLocalRef(env, obj);
    (*env)->DeleteLocalRef(env, cls);
    return -1;
  }
  (*env)->DeleteLocalRef(env, obj);
  (*env)->DeleteLocalRef(env, cls);

  /* system configuration */
  systemConfigFid = (*env)->GetFieldID(env, iscClass, "systemConfig", "Lcom/nextone/common/iServerConfig$SystemConfig;");
  if (systemConfigFid == NULL)
    return -1;
  obj = (*env)->GetObjectField(env, config, systemConfigFid);
  if (obj == NULL)
    return -1;
  cls = (*env)->GetObjectClass(env, obj);
  if (cls == NULL) {
    (*env)->DeleteLocalRef(env, obj);
    return -1;
  }

  sysG711Ulaw64DurationFid = (*env)->GetFieldID(env, cls, "g711Ulaw64Duration", "I");
  if (sysG711Ulaw64DurationFid == NULL) {
    (*env)->DeleteLocalRef(env, obj);
    (*env)->DeleteLocalRef(env, cls);
    return -1;
  }
  sysG711Alaw64DurationFid = (*env)->GetFieldID(env, cls, "g711Alaw64Duration", "I");
  if (sysG711Alaw64DurationFid == NULL) {
    (*env)->DeleteLocalRef(env, obj);
    (*env)->DeleteLocalRef(env, cls);
    return -1;
  }
  sysG729FramesFid = (*env)->GetFieldID(env, cls, "g729Frames", "I");
  if (sysG729FramesFid == NULL) {
    (*env)->DeleteLocalRef(env, obj);
    (*env)->DeleteLocalRef(env, cls);
    return -1;
  }
  sysG7231FramesFid = (*env)->GetFieldID(env, cls, "g7231Frames", "I");
  if (sysG7231FramesFid == NULL) {
    (*env)->DeleteLocalRef(env, obj);
    (*env)->DeleteLocalRef(env, cls);
    return -1;
  }

  sysDefaultCodecFid= (*env)->GetFieldID(env, cls, "defaultCodec", "I");
  if (sysDefaultCodecFid== NULL) {
    (*env)->DeleteLocalRef(env, obj);
    (*env)->DeleteLocalRef(env, cls);
    return -1;
  }

  sysRolloverTimeFid = (*env)->GetFieldID(env, cls, "rolloverTime", "I");
  if (sysRolloverTimeFid == NULL) {
    (*env)->DeleteLocalRef(env, obj);
    (*env)->DeleteLocalRef(env, cls);
    return -1;
  }
  sysCacheTimeoutFid = (*env)->GetFieldID(env, cls, "cacheTimeout", "I");
  if (sysCacheTimeoutFid == NULL) {
    (*env)->DeleteLocalRef(env, obj);
    (*env)->DeleteLocalRef(env, cls);
    return -1;
  }
  sysEnumDomainFid = (*env)->GetFieldID(env, cls, "enumDomain", "Ljava/lang/String;");
  if (sysEnumDomainFid == NULL) {
    (*env)->DeleteLocalRef(env, obj);
    (*env)->DeleteLocalRef(env, cls);
    return -1;
  }
/*  sysRadiusServerFid = (*env)->GetFieldID(env, cls, "radiusServer", "Ljava/lang/String;");
  if (sysRadiusServerFid == NULL) {
    (*env)->DeleteLocalRef(env, obj);
    (*env)->DeleteLocalRef(env, cls);
    return -1;
  }
  sysRadiusSecretFid = (*env)->GetFieldID(env, cls, "radiusSecret", "Ljava/lang/String;");
  if (sysRadiusSecretFid == NULL) {
    (*env)->DeleteLocalRef(env, obj);
    (*env)->DeleteLocalRef(env, cls);
    return -1;
  }
*/
  sysAllowAllSrcFid = (*env)->GetFieldID(env, cls, "allowAllSrc", "Z");
  if (sysAllowAllSrcFid == NULL) {
    (*env)->DeleteLocalRef(env, obj);
    (*env)->DeleteLocalRef(env, cls);
    return -1;
  }
  sysAllowAllDstFid = (*env)->GetFieldID(env, cls, "allowAllDst", "Z");
  if (sysAllowAllDstFid == NULL) {
    (*env)->DeleteLocalRef(env, obj);
    (*env)->DeleteLocalRef(env, cls);
    return -1;
  }
  sysAllowHairpinCallsFid = (*env)->GetFieldID(env, cls, "allowHairpinCalls", "Z");
  if (sysAllowHairpinCallsFid == NULL) {
    (*env)->DeleteLocalRef(env, obj);
    (*env)->DeleteLocalRef(env, cls);
    return -1;
  }
  sysRemoveRFC2833Fid = (*env)->GetFieldID(env, cls, "removeRFC2833", "Z");
  if (sysRemoveRFC2833Fid == NULL) {
    (*env)->DeleteLocalRef(env, obj);
    (*env)->DeleteLocalRef(env, cls);
    return -1;
  }
  sysRemoveT38Fid = (*env)->GetFieldID(env, cls, "removeT38", "Z");
  if (sysRemoveT38Fid == NULL) {
    (*env)->DeleteLocalRef(env, obj);
    (*env)->DeleteLocalRef(env, cls);
    return -1;
  }
  sysDefaultMediaRoutingFid = (*env)->GetFieldID(env, cls, "defaultMediaRouting", "Z");
  if (sysDefaultMediaRoutingFid == NULL) {
    (*env)->DeleteLocalRef(env, obj);
    (*env)->DeleteLocalRef(env, cls);
    return -1;
  }
  sysDefaultMidCallMediaChangeFid = (*env)->GetFieldID(env, cls, "defaultMidCallMediaChange", "Z");
  if (sysDefaultMidCallMediaChangeFid == NULL) {
    (*env)->DeleteLocalRef(env, obj);
    (*env)->DeleteLocalRef(env, cls);
    return -1;
  }
  sysMaxHuntsFid = (*env)->GetFieldID(env, cls, "maxHunts", "I");
  if (sysMaxHuntsFid == NULL) {
    (*env)->DeleteLocalRef(env, obj);
    (*env)->DeleteLocalRef(env, cls);
    return -1;
  }
  sysMaxHuntsLimitFid = (*env)->GetFieldID(env, cls, "maxHuntsLimit", "I");
  if (sysMaxHuntsLimitFid == NULL) {
    (*env)->DeleteLocalRef(env, obj);
    (*env)->DeleteLocalRef(env, cls);
    return -1;
  }
  sysMaxHuntsAllowableDurationFid = (*env)->GetFieldID(env, cls, "maxHuntsAllowableDuration", "I");
  if (sysMaxHuntsAllowableDurationFid == NULL) {
    (*env)->DeleteLocalRef(env, obj);
    (*env)->DeleteLocalRef(env, cls);
    return -1;
  }
  sysAllowAllRtpFid = (*env)->GetFieldID(env, cls, "allowAllRtp", "Z");
  if (sysAllowAllRtpFid == NULL) {
    (*env)->DeleteLocalRef(env, obj);
    (*env)->DeleteLocalRef(env, cls);
    return -1;
  }
  sysForwardSrcAddrFid = (*env)->GetFieldID(env, cls, "forwardSrcAddr", "Z");
  if (sysForwardSrcAddrFid == NULL) {
    (*env)->DeleteLocalRef(env, obj);
    (*env)->DeleteLocalRef(env, cls);
    return -1;
  }
  sysMaxCallDurationFid = (*env)->GetFieldID(env, cls, "maxCallDuration", "I");
  if (sysMaxCallDurationFid == NULL) {
    (*env)->DeleteLocalRef(env, obj);
    (*env)->DeleteLocalRef(env, cls);
    return -1;
  }
  sysMswNameFid = (*env)->GetFieldID(env, cls, "mswName", "Ljava/lang/String;");
  if (sysMswNameFid== NULL) {
    (*env)->DeleteLocalRef(env, obj);
    (*env)->DeleteLocalRef(env, cls);
    return -1;
  }

  sysMgmtIpFid = (*env)->GetFieldID(env, cls, "mgmtIp", "Ljava/lang/String;");
  if (sysMgmtIpFid== NULL) {
    (*env)->DeleteLocalRef(env, obj);
    (*env)->DeleteLocalRef(env, cls);
    return -1;
  }
  sysMapIsdnccFid = (*env)->GetFieldID(env, cls, "mapIsdncc", "Z");
  if (sysMapIsdnccFid == NULL) {
    (*env)->DeleteLocalRef(env, obj);
    (*env)->DeleteLocalRef(env, cls);
    return -1;
  }
  sysMapLrjReasonFid = (*env)->GetFieldID(env, cls, "mapLrjReason", "Z");
  if (sysMapLrjReasonFid == NULL) {
    (*env)->DeleteLocalRef(env, obj);
    (*env)->DeleteLocalRef(env, cls);
    return -1;
  }
  sysUseCodeMapFid = (*env)->GetFieldID(env, cls, "useCodeMap", "Ljava/lang/String;");
  if (sysUseCodeMapFid== NULL) {
    (*env)->DeleteLocalRef(env, obj);
    (*env)->DeleteLocalRef(env, cls);
    return -1;
  }

  (*env)->DeleteLocalRef(env, obj);
  (*env)->DeleteLocalRef(env, cls);

  /* logging configuration */
  loggingConfigFid = (*env)->GetFieldID(env, iscClass, "loggingConfig", "Lcom/nextone/common/iServerConfig$LoggingConfig;");
  if (loggingConfigFid == NULL)
    return -1;
  obj = (*env)->GetObjectField(env, config, loggingConfigFid);
  if (obj == NULL)
    return -1;
  cls = (*env)->GetObjectClass(env, obj);
  if (cls == NULL) {
    (*env)->DeleteLocalRef(env, obj);
    return -1;
  }
  logSetModMid = (*env)->GetMethodID(env, cls, "setModuleLogs", "(ILjava/lang/String;)Z");
  if (logSetModMid == NULL) {
    (*env)->DeleteLocalRef(env, obj);
    (*env)->DeleteLocalRef(env, cls);  
    return -1;
  }
  logGetModMid = (*env)->GetMethodID(env, cls, "getModuleLogs", "(I)Ljava/lang/String;");
  if (logGetModMid == NULL) {
    (*env)->DeleteLocalRef(env, obj);
    (*env)->DeleteLocalRef(env, cls);
    return -1;
  }
  logUpdateAllocationFid  = (*env)->GetFieldID(env,cls,"updateAllocation","Z");
  if (logUpdateAllocationFid == NULL) {
    (*env)->DeleteLocalRef(env, obj);
    (*env)->DeleteLocalRef(env, cls);
    return -1;
  }

  logDebugLevelFid  = (*env)->GetFieldID(env,cls,"debugLevel","I");
  if(logDebugLevelFid  ==  NULL){
    (*env)->DeleteLocalRef(env,obj);
    (*env)->DeleteLocalRef(env,cls);
    return -1;
  }

  logTpktchanFid  = (*env)->GetFieldID(env,cls,"tpktchan","Z");
  if(logTpktchanFid  ==  NULL){
    (*env)->DeleteLocalRef(env,obj);
    (*env)->DeleteLocalRef(env,cls);
    return -1;
  }

  logUdpchanFid  = (*env)->GetFieldID(env,cls,"udpchan","Z");
  if(logUdpchanFid  ==  NULL){
    (*env)->DeleteLocalRef(env,obj);
    (*env)->DeleteLocalRef(env,cls);
    return -1;
  }
  logPererrFid  = (*env)->GetFieldID(env,cls,"pererr","Z");
  if(logPererrFid  ==  NULL){
    (*env)->DeleteLocalRef(env,obj);
    (*env)->DeleteLocalRef(env,cls);
    return -1;
  }
  logCmFid  = (*env)->GetFieldID(env,cls,"cm","Z");
  if(logCmFid  ==  NULL){
    (*env)->DeleteLocalRef(env,obj);
    (*env)->DeleteLocalRef(env,cls);
    return -1;
  }
  logCmapicbFid = (*env)->GetFieldID(env,cls,"cmapicb","Z");
  if(logCmapicbFid ==  NULL){
    (*env)->DeleteLocalRef(env,obj);
    (*env)->DeleteLocalRef(env,cls);
    return -1;
  }
  logCmapiFid = (*env)->GetFieldID(env,cls,"cmapi","Z");
  if(logCmapiFid ==  NULL){
    (*env)->DeleteLocalRef(env,obj);
    (*env)->DeleteLocalRef(env,cls);
    return -1;
  }
  logCmerrFid = (*env)->GetFieldID(env,cls,"cmerr","Z");
  if(logCmerrFid ==  NULL){
    (*env)->DeleteLocalRef(env,obj);
    (*env)->DeleteLocalRef(env,cls);
    return -1;
  }
  logLiFid = (*env)->GetFieldID(env,cls,"li","Z");
  if(logLiFid ==  NULL){
    (*env)->DeleteLocalRef(env,obj);
    (*env)->DeleteLocalRef(env,cls);
    return -1;
  }

  logLiinfoFid = (*env)->GetFieldID(env,cls,"liinfo","Z");
  if(logLiinfoFid ==  NULL){
    (*env)->DeleteLocalRef(env,obj);
    (*env)->DeleteLocalRef(env,cls);
    return -1;
  }

  logSDebugLevelFid  = (*env)->GetFieldID(env,cls,"sdebugLevel","I");
  if(logSDebugLevelFid  ==  NULL) {
    (*env)->DeleteLocalRef(env,obj);
    (*env)->DeleteLocalRef(env,cls);
    return -1;
  }


  (*env)->DeleteLocalRef(env, obj);
  (*env)->DeleteLocalRef(env, cls);  

  /* Redundancy configuration */
  redConfigFid = (*env)->GetFieldID(env, iscClass, "redundsConfig", "Lcom/nextone/common/iServerConfig$RedundsConfig;");
  if (redConfigFid == NULL)
    return -1;
  redObj = (*env)->GetObjectField(env, config, redConfigFid);
  if (redObj == NULL)
    return -1;
  redCls = (*env)->GetObjectClass(env, redObj);
  if (redCls == NULL) {
    (*env)->DeleteLocalRef(env, redObj);
    return -1;
  }

  redNetConfigFid = (*env)->GetFieldID(env, redCls, "networkConfig", "Lcom/nextone/common/iServerConfig$RedundsConfig$NetworkConfig;");
  if (redNetConfigFid == NULL) {
    (*env)->DeleteLocalRef(env, redObj);
    (*env)->DeleteLocalRef(env, redCls);
    return -1;
  }
  obj = (*env)->GetObjectField(env, redObj, redNetConfigFid);
  if (obj == NULL) {
    (*env)->DeleteLocalRef(env, redObj);
    (*env)->DeleteLocalRef(env, redCls);
    return -1;
  }
  cls = (*env)->GetObjectClass(env, obj);
  if (cls == NULL) {
    (*env)->DeleteLocalRef(env, redObj);
    (*env)->DeleteLocalRef(env, redCls);
    (*env)->DeleteLocalRef(env, obj);
    return -1;
  }

  redNetServerTypeFid = (*env)->GetFieldID(env, cls, "serverType", "Ljava/lang/String;");
  if (redNetServerTypeFid == NULL) {
    (*env)->DeleteLocalRef(env, obj);
    (*env)->DeleteLocalRef(env, cls);
    (*env)->DeleteLocalRef(env, redObj);
    (*env)->DeleteLocalRef(env, redCls);
    return -1;
  }
/*
  redNetPrimIfNameFid = (*env)->GetFieldID(env, cls, "primaryInterface", "Ljava/lang/String;");
  if (redNetPrimIfNameFid == NULL) {
    (*env)->DeleteLocalRef(env, obj);
    (*env)->DeleteLocalRef(env, cls);
    (*env)->DeleteLocalRef(env, redObj);
    (*env)->DeleteLocalRef(env, redCls);
    return -1;
  }

  redNetSecondIfNameFid = (*env)->GetFieldID(env, cls, "secondaryInterface", "Ljava/lang/String;");
  if (redNetSecondIfNameFid == NULL) {
    (*env)->DeleteLocalRef(env, obj);
    (*env)->DeleteLocalRef(env, cls);
    (*env)->DeleteLocalRef(env, redObj);
    (*env)->DeleteLocalRef(env, redCls);
    return -1;
  }

/*
  redNetPrimIfVipFid = (*env)->GetFieldID(env, cls, "primaryInterfaceVip", "Ljava/lang/String;");
  if (redNetPrimIfVipFid == NULL) {
    (*env)->DeleteLocalRef(env, obj);
    (*env)->DeleteLocalRef(env, cls);
    (*env)->DeleteLocalRef(env, redObj);
    (*env)->DeleteLocalRef(env, redCls);
    return -1;
  }

  redNetPrimaryInterfaceVipsFid = (*env)->GetFieldID(env, cls, "primaryInterfaceVips", "[Ljava/lang/String;");
  if (redNetPrimaryInterfaceVipsFid == NULL) {
    (*env)->DeleteLocalRef(env, obj);
    (*env)->DeleteLocalRef(env, cls);
    (*env)->DeleteLocalRef(env, redObj);
    (*env)->DeleteLocalRef(env, redCls);
    return -1;
  }

  redNetSecondaryInterfaceVipsFid = (*env)->GetFieldID(env, cls, "secondaryInterfaceVips", "[Ljava/lang/String;");
  if (redNetSecondaryInterfaceVipsFid == NULL) {
    (*env)->DeleteLocalRef(env, obj);
    (*env)->DeleteLocalRef(env, cls);
    (*env)->DeleteLocalRef(env, redObj);
    (*env)->DeleteLocalRef(env, redCls);
    return -1;
  }

  redNetPrimaryInterfaceVipsClearMid = (*env)->GetMethodID(env, cls, "clearPrimaryInterfaceVips", "()V");
  if (redNetPrimaryInterfaceVipsClearMid == NULL) {
    (*env)->DeleteLocalRef(env, obj);
    (*env)->DeleteLocalRef(env, cls);
    (*env)->DeleteLocalRef(env, redObj);
    (*env)->DeleteLocalRef(env, redCls);
    return -1;
  }
  redNetSecondaryInterfaceVipsClearMid = (*env)->GetMethodID(env, cls, "clearSecondaryInterfaceVips", "()V");
  if (redNetSecondaryInterfaceVipsClearMid == NULL) {
    (*env)->DeleteLocalRef(env, obj);
    (*env)->DeleteLocalRef(env, cls);
    (*env)->DeleteLocalRef(env, redObj);
    (*env)->DeleteLocalRef(env, redCls);
    return -1;
  }

  redNetPrimaryInterfaceVipAddMid = (*env)->GetMethodID(env, cls, "addPrimaryInterfaceVip", "(Ljava/lang/String;)V");
  if (redNetPrimaryInterfaceVipAddMid == NULL) {
    (*env)->DeleteLocalRef(env, obj);
    (*env)->DeleteLocalRef(env, cls);
    (*env)->DeleteLocalRef(env, redObj);
    (*env)->DeleteLocalRef(env, redCls);
    return -1;
  }

  redNetSecondaryInterfaceVipAddMid = (*env)->GetMethodID(env, cls, "addSecondaryInterfaceVip", "(Ljava/lang/String;)V");
  if (redNetSecondaryInterfaceVipAddMid == NULL) {
    (*env)->DeleteLocalRef(env, obj);
    (*env)->DeleteLocalRef(env, cls);
    (*env)->DeleteLocalRef(env, redObj);
    (*env)->DeleteLocalRef(env, redCls);
    return -1;
  }

  redNetPrimIfRouterFid = (*env)->GetFieldID(env, cls, "primaryInterfaceRouter", "Ljava/lang/String;");
  if (redNetPrimIfRouterFid == NULL) {
    (*env)->DeleteLocalRef(env, obj);
    (*env)->DeleteLocalRef(env, cls);
    (*env)->DeleteLocalRef(env, redObj);
    (*env)->DeleteLocalRef(env, redCls);
    return -1;
  }
  redNetSecondIfRouterFid = (*env)->GetFieldID(env, cls, "secondaryInterfaceRouter", "Ljava/lang/String;");
  if (redNetSecondIfRouterFid == NULL) {
    (*env)->DeleteLocalRef(env, obj);
    (*env)->DeleteLocalRef(env, cls);
    (*env)->DeleteLocalRef(env, redObj);
    (*env)->DeleteLocalRef(env, redCls);
    return -1;
  }
*/
  redNetControlIfNameFid = (*env)->GetFieldID(env, cls, "controlInterface", "Ljava/lang/String;");
  if (redNetControlIfNameFid == NULL) {
    (*env)->DeleteLocalRef(env, obj);
    (*env)->DeleteLocalRef(env, cls);
    (*env)->DeleteLocalRef(env, redObj);
    (*env)->DeleteLocalRef(env, redCls);
    return -1;
  }
  redNetPeersFid = (*env)->GetFieldID(env, cls, "peers", "[Ljava/lang/String;");
  if (redNetPeersFid == NULL) {
    (*env)->DeleteLocalRef(env, obj);
    (*env)->DeleteLocalRef(env, cls);
    (*env)->DeleteLocalRef(env, redObj);
    (*env)->DeleteLocalRef(env, redCls);
    return -1;
  }
  redNetScmFid = (*env)->GetFieldID(env, cls, "scmEnabled", "Z");
  if (redNetScmFid == NULL) {
    (*env)->DeleteLocalRef(env, obj);
    (*env)->DeleteLocalRef(env, cls);
    (*env)->DeleteLocalRef(env, redObj);
    (*env)->DeleteLocalRef(env, redCls);
    return -1;
  }
  redNetPeersClearMid = (*env)->GetMethodID(env, cls, "clearPeers", "()V");
  if (redNetPeersClearMid == NULL) {
    (*env)->DeleteLocalRef(env, obj);
    (*env)->DeleteLocalRef(env, cls);
    (*env)->DeleteLocalRef(env, redObj);
    (*env)->DeleteLocalRef(env, redCls);
    return -1;
  }
  redNetPeersAddMid = (*env)->GetMethodID(env, cls, "addPeer", "(Ljava/lang/String;)V");
  if (redNetPeersAddMid == NULL) {
    (*env)->DeleteLocalRef(env, obj);
    (*env)->DeleteLocalRef(env, cls);
    (*env)->DeleteLocalRef(env, redObj);
    (*env)->DeleteLocalRef(env, redCls);
    return -1;
  }




  (*env)->DeleteLocalRef(env, obj);
  (*env)->DeleteLocalRef(env, cls);

  redDbConfigFid = (*env)->GetFieldID(env, redCls, "databaseConfig", "Lcom/nextone/common/iServerConfig$RedundsConfig$DatabaseConfig;");
  if (redDbConfigFid == NULL) {
    (*env)->DeleteLocalRef(env, redObj);
    (*env)->DeleteLocalRef(env, redCls);
    return -1;
  }
  obj = (*env)->GetObjectField(env, redObj, redDbConfigFid);
  if (obj == NULL) {
    (*env)->DeleteLocalRef(env, redObj);
    (*env)->DeleteLocalRef(env, redCls);
    return -1;
  }
  cls = (*env)->GetObjectClass(env, obj);
  if (cls == NULL) {
    (*env)->DeleteLocalRef(env, redObj);
    (*env)->DeleteLocalRef(env, redCls);
    (*env)->DeleteLocalRef(env, obj);
    return -1;
  }

  redDbServerStatusFid = (*env)->GetFieldID(env, cls, "serverStatus", "Z");
  if (redDbServerStatusFid == NULL) {
    (*env)->DeleteLocalRef(env, obj);
    (*env)->DeleteLocalRef(env, cls);
    (*env)->DeleteLocalRef(env, redObj);
    (*env)->DeleteLocalRef(env, redCls);
    return -1;
  }
  redDbIfNameFid = (*env)->GetFieldID(env, cls, "ifName", "Ljava/lang/String;");
  if (redDbIfNameFid == NULL) {
    (*env)->DeleteLocalRef(env, obj);
    (*env)->DeleteLocalRef(env, cls);
    (*env)->DeleteLocalRef(env, redObj);
    (*env)->DeleteLocalRef(env, redCls);
    return -1;
  }
  redDbMcastAddrFid = (*env)->GetFieldID(env, cls, "mcastAddr", "Ljava/lang/String;");
  if (redDbMcastAddrFid == NULL) {
    (*env)->DeleteLocalRef(env, obj);
    (*env)->DeleteLocalRef(env, cls);
    (*env)->DeleteLocalRef(env, redObj);
    (*env)->DeleteLocalRef(env, redCls);
    return -1;
  }
  redDbPortFid = (*env)->GetFieldID(env, cls, "port", "I");
  if (redDbPortFid == NULL) {
    (*env)->DeleteLocalRef(env, obj);
    (*env)->DeleteLocalRef(env, cls);
    (*env)->DeleteLocalRef(env, redObj);
    (*env)->DeleteLocalRef(env, redCls);
    return -1;
  }
  redDbPriorityFid = (*env)->GetFieldID(env, cls, "priority", "I");
  if (redDbPriorityFid == NULL) {
    (*env)->DeleteLocalRef(env, obj);
    (*env)->DeleteLocalRef(env, cls);
    (*env)->DeleteLocalRef(env, redObj);
    (*env)->DeleteLocalRef(env, redCls);
    return -1;
  }
  (*env)->DeleteLocalRef(env, obj);
  (*env)->DeleteLocalRef(env, cls);
  (*env)->DeleteLocalRef(env, redObj);
  (*env)->DeleteLocalRef(env, redCls);

  sysutilClass = GetGlobalClassReference(env, "com/nextone/util/SysUtil");
  if (sysutilClass == NULL)
    return -1;

  suCreateStringArrayMid = (*env)->GetStaticMethodID(env, sysutilClass, "createStringArray", "([Ljava/lang/String;Ljava/lang/String;)[Ljava/lang/String;");
  if (suCreateStringArrayMid == NULL)
    return -1;

  suCreateObjectArrayMid = (*env)->GetStaticMethodID(env, sysutilClass, "createObjectArray", "([Ljava/lang/Object;Ljava/lang/Object;)[Ljava/lang/Object;");
  if (suCreateObjectArrayMid == NULL)
    return -1;

  inetAddressClass = GetGlobalClassReference(env, "java/net/InetAddress");
  if (inetAddressClass == NULL)
    return -1;

  inetAddressGetByNameMid = (*env)->GetStaticMethodID(env, inetAddressClass, "getByName", "(Ljava/lang/String;)Ljava/net/InetAddress;");
  if (inetAddressGetByNameMid == NULL)
    return -1;


// radius details
  radiusConfigFid = (*env)->GetFieldID(env, iscClass, "radiusConfig", "Lcom/nextone/common/iServerConfig$RadiusConfig;");
  if (radiusConfigFid == NULL)
    return -1;
  obj = (*env)->GetObjectField(env, config, radiusConfigFid);
  if (obj == NULL)
    return -1;
  cls = (*env)->GetObjectClass(env, obj);
  if (cls == NULL) {
    (*env)->DeleteLocalRef(env, obj);
    return -1;
  }

  
  radiusSetServersMid= (*env)->GetMethodID(env, cls, "setServers", "([Ljava/lang/String;)V");
  if (radiusSetServersMid == NULL) {
    (*env)->DeleteLocalRef(env, obj);
    (*env)->DeleteLocalRef(env, cls);  
    return -1;
  }

  radiusSetSecretsMid= (*env)->GetMethodID(env, cls, "setSecrets", "([Ljava/lang/String;)V");
  if (radiusSetSecretsMid == NULL) {
    (*env)->DeleteLocalRef(env, obj);
    (*env)->DeleteLocalRef(env, cls);  
    return -1;
  }

  /*
  radiusGetServersMid= (*env)->GetMethodID(env, cls, "getServers", "([Ljava/lang/String;)V");
  if (radiusGetServersMid == NULL) {
    (*env)->DeleteLocalRef(env, obj);
    (*env)->DeleteLocalRef(env, cls);  
    return -1;
  }
  LOCAL_DEBUG(JNI_TRUE, DEBUG_VERBOSE, ("iserverconfig: radiusGetServersMid"));

  LOCAL_DEBUG(JNI_TRUE, DEBUG_VERBOSE, ("iserverconfig: radiusSetServersMid"));
  radiusGetSecretsMid= (*env)->GetMethodID(env, cls, "getSecrets", "([Ljava/lang/String;)V");
  if (radiusGetSecretsMid == NULL) {
    (*env)->DeleteLocalRef(env, obj);
    (*env)->DeleteLocalRef(env, cls);  
    return -1;
  }

  LOCAL_DEBUG(JNI_TRUE, DEBUG_VERBOSE, ("iserverconfig: radiusGetSecretsMid"));
  radiusSetSecretsMid= (*env)->GetMethodID(env, cls, "setSecrets", "(V)[Ljava/lang/String;");
  if (radiusSetSecretsMid == NULL) {
    (*env)->DeleteLocalRef(env, obj);
    (*env)->DeleteLocalRef(env, cls);  
    return -1;
  }
*/

  radiusServersFid  = (*env)->GetFieldID(env,cls,"servers","[Ljava/lang/String;");
  if (radiusServersFid == NULL) {
    (*env)->DeleteLocalRef(env, obj);
    (*env)->DeleteLocalRef(env, cls);
      return -1;
  }
  
  radiusSecretsFid  = (*env)->GetFieldID(env,cls,"secrets","[Ljava/lang/String;");
  if (radiusSecretsFid == NULL) {
    (*env)->DeleteLocalRef(env, obj);
    (*env)->DeleteLocalRef(env, cls);
    return -1;
  }

  radiusDirNameFid  = (*env)->GetFieldID(env,cls,"dirName","Ljava/lang/String;");
  if (radiusDirNameFid == NULL) {
    (*env)->DeleteLocalRef(env, obj);
    (*env)->DeleteLocalRef(env, cls);
    return -1;
  }

  radiusTimeoutFid  = (*env)->GetFieldID(env,cls,"timeout","I");
  if (radiusTimeoutFid == NULL) {
    (*env)->DeleteLocalRef(env, obj);
    (*env)->DeleteLocalRef(env, cls);
    return -1;
  }

  radiusRetryFid  = (*env)->GetFieldID(env,cls,"retry","I");
  if (radiusRetryFid  == NULL) {
    (*env)->DeleteLocalRef(env, obj);
    (*env)->DeleteLocalRef(env, cls);
    return -1;
  }

  radiusDeadTimeFid  = (*env)->GetFieldID(env,cls,"deadTime","I");
  if (radiusDeadTimeFid  == NULL) {
    (*env)->DeleteLocalRef(env, obj);
    (*env)->DeleteLocalRef(env, cls);
    return -1;
  }

  radiusSendMsgFid = (*env)->GetFieldID(env,cls,"sendMsg","Z");
  if (radiusSendMsgFid == NULL) {
    (*env)->DeleteLocalRef(env, obj);
    (*env)->DeleteLocalRef(env, cls);
    return -1;
  }

  radiusUseOSIFFid = (*env)->GetFieldID(env,cls,"useOSIF","Z");
  if (radiusUseOSIFFid == NULL) {
    (*env)->DeleteLocalRef(env, obj);
    (*env)->DeleteLocalRef(env, cls);
    return -1;
  }

  initMswNMState();


  return 0;
}

 
static void initMswNMState(){
  int id;
  mswNMState  =  NULL; 
  id = shmget(ISERVER_STATE_SHM_KEY,sizeof(int),SHM_R);
  if(id < 0){
    printf("[iserverconfig.c/initMswNmState]  shmget returning false\n");
  }else{
    mswNMState = shmat(id,(void*)0,SHM_R);
    if((int)mswNMState == -1){
      mswNMState = NULL;
      printf("[iserverconfig.c/initMswNmState]  shmat returning false, error = %s\n",strerror(errno));
    }
  }
}

static int getMswNMState(){
  if(mswNMState == NULL){
    initMswNMState();
  }
  if(mswNMState == NULL){
    printf("[iserverconfig.c/getMswState]  shmat state is NULL\n");
     return -1;
  }
  return *mswNMState;
}
  


static int
ExtractSipFields (JNIEnv *env, jobject sipConfig) {
  jstring jstr;
  jboolean bool;
  const char *str;

  /* sip server name */
  jstr = (jstring)(*env)->GetObjectField(env, sipConfig, scServerNameFid);
  if (jstr == NULL) {
    memset(sipservername, 0, SIPURL_LEN);
  } else {
    str = (*env)->GetStringUTFChars(env, jstr, NULL);
    if (str != NULL) {
      strncpy(sipservername, str, SIPURL_LEN);
      sipservername[SIPURL_LEN-1] = '\0';
      (*env)->ReleaseStringUTFChars(env, jstr, str);
    }
    (*env)->DeleteLocalRef(env, jstr);
  }

  /* sip domain */
  jstr = (jstring)(*env)->GetObjectField(env, sipConfig, scSipDomainFid);
  if (jstr == NULL) {
    memset(sipdomain, 0, SIPDOMAIN_LEN);
  } else {
    str = (*env)->GetStringUTFChars(env, jstr, NULL);
    if (str != NULL) {
      strncpy(sipdomain, str, SIPDOMAIN_LEN);
      sipdomain[SIPDOMAIN_LEN-1] = '\0';
      (*env)->ReleaseStringUTFChars(env, jstr, str);
    }
    (*env)->DeleteLocalRef(env, jstr);
  }

  /* sip auth */
/*
  bool = (*env)->GetBooleanField(env, sipConfig, scAuthEnabledFid);
  sipauth = (bool == JNI_FALSE)?0:1;
*/
  sipauth = (int)(*env)->GetIntField(env,sipConfig,scAuthFid);

  /* sip auth password */
  jstr = (jstring)(*env)->GetObjectField(env, sipConfig, scAuthPasswordFid);
  if (jstr == NULL) {
    memset(sipauthpassword, 0, SIPAUTHPASSWORD_LEN);
  } else {
    str = (*env)->GetStringUTFChars(env, jstr, NULL);
    if (str != NULL) {
      strncpy(sipauthpassword, str, SIPAUTHPASSWORD_LEN);
      sipauthpassword[SIPAUTHPASSWORD_LEN-1] = '\0';
      (*env)->ReleaseStringUTFChars(env, jstr, str);
    }
    (*env)->DeleteLocalRef(env, jstr);
  }

  /* record route */
  bool = (*env)->GetBooleanField(env, sipConfig, scRecordRouteEnabledFid);
  recordroute = (bool == JNI_FALSE)?0:1;

  /* obp */
  bool = (*env)->GetBooleanField(env, sipConfig, scOBPFid);
  obpEnabled = (bool == JNI_FALSE)?0:1;
  bool = (*env)->GetBooleanField(env, sipConfig, scDynamicEPFid);
  allowDynamicEndpoints = (bool == JNI_FALSE)?0:1;
  bool = (*env)->GetBooleanField(env, sipConfig, scInternalCallFid);
  allowInternalCalling = (bool == JNI_FALSE)?0:1;

  /* server type */
  sipservertype = (int)(*env)->GetIntField(env, sipConfig, scServerTypeFid);

  /* max forwards */
  sipmaxforwards = (int)(*env)->GetIntField(env, sipConfig, scMaxForwardsFid);

  /* sip timerC */
  siptimerC = (int)(*env)->GetIntField(env, sipConfig, scSipTimerCFid);

  /* sip port */
  sipport = (int)(*env)->GetIntField(env, sipConfig, scSipPortFid);

  return 0;
}


static int
ExtractH323Fields (JNIEnv *env, jobject h323Config) {
  jstring jstr;
  const char *str;
  jboolean bool;

  /* SGK reg prefix */
  jstr = (jstring)(*env)->GetObjectField(env, h323Config, h323SgkRegPrefixFid);
  if (jstr == NULL) {
    memset(prefix, 0, PHONE_NUM_LEN);
  } else {
    str = (*env)->GetStringUTFChars(env, jstr, NULL);
    if (str != NULL) {
      strncpy(prefix, str, PHONE_NUM_LEN);
      prefix[PHONE_NUM_LEN-1] = '\0';
      (*env)->ReleaseStringUTFChars(env, jstr, str);
    }
    (*env)->DeleteLocalRef(env, jstr);
  }

  /* GK ID */
  jstr = (jstring)(*env)->GetObjectField(env, h323Config, h323GkIdFid);
  if (jstr == NULL) {
    memset(gkid, 0, GKID_LEN);
  } else {
    str = (*env)->GetStringUTFChars(env, jstr, NULL);
    if (str != NULL) {
      strncpy(gkid, str, GKID_LEN);
      gkid[GKID_LEN-1] = '\0';
      (*env)->ReleaseStringUTFChars(env, jstr, str);
    }
    (*env)->DeleteLocalRef(env, jstr);
  }

  /* RRQ timer */
  rrqtimer = (int)(*env)->GetIntField(env, h323Config, h323RrqTimerFid);

  /* RRQ TTL */
  rrqttl = (int)(*env)->GetIntField(env, h323Config, h323RrqTtlFid);

  /* H.245 routing */
  bool = (*env)->GetBooleanField(env, h323Config, h323H245RoutingEnabledFid);
  routeH245 = (bool == JNI_FALSE)?0:1;

  /* Force H.245 */
  bool = (*env)->GetBooleanField(env, h323Config, h323ForceH245Fid);
  forceh245 = (bool == JNI_FALSE)?0:1;

  /* Call Routing */
  bool = (*env)->GetBooleanField(env, h323Config, h323CallRoutingEnabledFid);
  routecall = (bool == JNI_FALSE)?0:1;

  /* fast start */
  bool = (*env)->GetBooleanField(env, h323Config, h323FastStartEnabledFid);
  doFastStart = (bool == JNI_FALSE)?0:1;

  /* calls per second */
  h323Cps         = (int)(*env)->GetIntField(env, h323Config, h323CpsFid);

  /* instances */
  nh323CfgInstances  = (int)(*env)->GetIntField(env, h323Config, h323InstancesFid);

  /* Sgatekeeper max calls */
  h323maxCallsSgk  = (int)(*env)->GetIntField(env, h323Config, h323SgkMaxCallsFid);

  /*  max calls */
  h323maxCalls  = (int)(*env)->GetIntField(env, h323Config, h323MaxCallsFid);

  /* local proceeding */
  bool = (*env)->GetBooleanField(env, h323Config, h323LocalProceedingFid);
  localProceeding = (bool == JNI_FALSE)?0:1;

  /* info trans cap */
  h323infoTransCap  = (int)(*env)->GetIntField(env, h323Config, h323InfoTransCapFid);

  /* allow destination arq */
  bool = (*env)->GetBooleanField(env, h323Config, h323AllowDestArqEnabledFid);
  allowDestArq = (bool == JNI_FALSE)?0:1;

  /* allow auth arq */
  bool = (*env)->GetBooleanField(env, h323Config, h323AllowAuthArqEnabledFid);
  allowAuthArq = (bool == JNI_FALSE)?0:1;

  /* h245 tunnel */
  bool = (*env)->GetBooleanField(env, h323Config, h323H245TunnelEnabledFid);
  h245Tunneling = (bool == JNI_FALSE)?0:1;

  return 0;
}


static int
ExtractFceFields (JNIEnv *env, jobject fceConfig) {
  jstring jstr1, jstr2;
  const char *str;
  jint addr;
  jobjectArray nla;
  const char *ip, *mask;
  char ipmaskbuffer[64] = {0};
  int len, i;
  jobject obj, ipmask;
  char buf[32];
  jboolean bool;

  if ((*env)->PushLocalFrame(env, 20))
    return -1;

#if FCE_REMOVED
  /* firewall name */
  jstr1 = (jstring)(*env)->GetObjectField(env, fceConfig, fceFwNameFid);
  if (jstr1 == NULL) {
    memset(fceConfigFwName, 0, 128);
  } else {
    str = (*env)->GetStringUTFChars(env, jstr1, NULL);
    if (str != NULL) {
      strncpy(fceConfigFwName, str, 128);
      fceConfigFwName[127] = '\0';
      (*env)->ReleaseStringUTFChars(env, jstr1, str);
    }
    (*env)->DeleteLocalRef(env, jstr1);
  }

  /* H.245 pinholes enabled? */
  bool = (*env)->GetBooleanField(env, fceConfig, fceH245Fid);
  fceH245PinholeEnabled = (bool == JNI_TRUE)?TRUE:FALSE;

  /* firewall connect ip address */
  addr = (*env)->CallIntMethod(env, fceConfig, fceGetFwConnectAddrMid);
  if (CheckAndThrowBridgeException(env))
    return -1;
  if (addr != -1)
    fceConfigOurIpAddr = addr;
  LOCAL_DEBUG(JNI_TRUE, DEBUG_VERBOSE, ("firewall connect address is being set to %s\n", FormatIpAddress(addr, buf)));

  /* default network space */
  bool = (*env)->GetBooleanField(env, fceConfig, fceDefaultPublicFid);
  fceDefaultPublic = (bool == JNI_TRUE)?TRUE:FALSE;

  /* private/public network list */
  nla = (jobjectArray)(*env)->CallObjectMethod(env, fceConfig, fceGetNetworkListMid);
  if (CheckAndThrowBridgeException(env))
    return -1;
  FCENetworkListConfig(NULL, TRUE);
  if (nla != NULL) {  /* network list is not empty */	
    len = (*env)->GetArrayLength(env, nla);
    for (i = 0; i < len; i++) {
      if ((*env)->PushLocalFrame(env, 10))
	return -1;

      obj = (*env)->GetObjectArrayElement(env, nla, i);
      if (CheckAndThrowBridgeException(env)) {
	(*env)->PopLocalFrame(env, NULL);
	return -1;
      }
      /* IPMask field */
      ipmask = (*env)->GetObjectField(env, obj, fceNetworkListIPMaskFid);
      if (ipmask == NULL) {
	(*env)->PopLocalFrame(env, NULL);
	return -1;
      }
      /* isPublic field */
      bool = (*env)->GetBooleanField(env, obj, fceNetworkListIsPublicFid);
      /* ip address */
      jstr1 = (jstring)(*env)->CallObjectMethod(env, ipmask, fceIPMaskGetIpMid);
      if (CheckAndThrowBridgeException(env)) {
	(*env)->PopLocalFrame(env, NULL);
	return -1;
      }
      /* mask */
      jstr2 = (jstring)(*env)->CallObjectMethod(env, ipmask, fceIPMaskGetMaskMid);
      if (CheckAndThrowBridgeException(env)) {
	(*env)->PopLocalFrame(env, NULL);
	return -1;
      }
      ip = (*env)->GetStringUTFChars(env, jstr1, NULL);
      if (ip == NULL) {
	(*env)->PopLocalFrame(env, NULL);
	return -1;
      }
      mask = (*env)->GetStringUTFChars(env, jstr2, NULL);
      if (mask == NULL) {
	(*env)->ReleaseStringUTFChars(env, jstr1, ip);
	(*env)->PopLocalFrame(env, NULL);
	return -1;
      }
      if (strlen(ip) == 0 || strlen(mask) == 0) {
	LOCAL_DEBUG(JNI_TRUE, DEBUG_WARNING, ("ip/mask is zero length - %s:%s", ip, mask));
	(*env)->ReleaseStringUTFChars(env, jstr1, ip);
	(*env)->ReleaseStringUTFChars(env, jstr2, mask);
	(*env)->PopLocalFrame(env, NULL);
	continue;
      }
      sprintf(ipmaskbuffer, "%s/%s", ip, mask);
      LOCAL_DEBUG(JNI_TRUE, DEBUG_VERBOSE, ("adding \"%s %s\"", ipmaskbuffer, (bool == JNI_TRUE)?"public":"private"));
      FCENetworkListConfig(ipmaskbuffer, (bool == JNI_TRUE)?TRUE:FALSE);
      (*env)->ReleaseStringUTFChars(env, jstr1, ip);
      (*env)->ReleaseStringUTFChars(env, jstr2, mask);
      (*env)->PopLocalFrame(env, NULL);
    }
  }

  (*env)->PopLocalFrame(env, NULL);
#endif
  return 0;
}


static int
ExtractFceNewFields (JNIEnv *env, jobject fceConfig) {
  jstring jstr1;
  const char *str;
  int addr;
  char buf[32];

  if ((*env)->PushLocalFrame(env, 20))
    return -1;

  /* firewall name */
  jstr1 = (jstring)(*env)->GetObjectField(env, fceConfig, fceNewFwNameFid);
  if (jstr1 == NULL) {
    memset(fceConfigFwName, 0, 128);
  } else {
    str = (*env)->GetStringUTFChars(env, jstr1, NULL);
    if (str != NULL) {
      strncpy(fceConfigFwName, str, 128);
      fceConfigFwName[127] = '\0';
      (*env)->ReleaseStringUTFChars(env, jstr1, str);
    }
    (*env)->DeleteLocalRef(env, jstr1);
  }

  /* firewall ip address */
  addr = (*env)->CallIntMethod(env, fceConfig, fceNewGetFwAddressMid);

  if (CheckAndThrowBridgeException(env))
    return -1;

  if (addr != -1)
    fceFirewallAddresses[0] = addr;

  LOCAL_DEBUG(JNI_TRUE, DEBUG_VERBOSE, ("firewall address is being set to %s\n", FormatIpAddress(addr, buf)));

  return 0;
}



static int
ExtractBillingFields (JNIEnv *env, jobject billingConfig) {
  jstring jstr;
  const char *str;
  jboolean bool;

  /* CDR type */
  cdrtype = (int)(*env)->GetIntField(env, billingConfig, billCdrTypeFid);

  /* CDR format */
  cdrformat = (int)(*env)->GetIntField(env, billingConfig, billCdrFormatFid);

  /* CDR directory */
  jstr = (jstring)(*env)->GetObjectField(env, billingConfig, billDirFid);
  if (jstr == NULL) {
    memset(cdrdirname, 0, CDRDIRNAMELEN);
  } else {
    str = (*env)->GetStringUTFChars(env, jstr, NULL);
    if (str != NULL) {
      strncpy(cdrdirname, str, CDRDIRNAMELEN);
      cdrdirname[CDRDIRNAMELEN-1] = '\0';
      (*env)->ReleaseStringUTFChars(env, jstr, str);
    }
    (*env)->DeleteLocalRef(env, jstr);
  }

  /* CDR timer */
  cdrtimer = (int)(*env)->GetIntField(env, billingConfig, billCdrTimerFid);

  /* billing type */
  billingType = (int)(*env)->GetIntField(env, billingConfig, billBillingTypeFid);

  /* cdr logs */
  bool = (*env)->GetBooleanField(env, billingConfig, billCdrLogStart1Fid);
  if (bool)
    cdrevents |= CDRSTART1;
  else
    cdrevents &= ~CDRSTART1;
  bool = (*env)->GetBooleanField(env, billingConfig, billCdrLogStart2Fid);
  if (bool)
    cdrevents |= CDRSTART2;
  else
    cdrevents &= ~CDRSTART2;
  bool = (*env)->GetBooleanField(env, billingConfig, billCdrLogEnd2Fid);
  if (bool)
    cdrevents |= CDREND2;
  else
    cdrevents &= ~CDREND2;
  bool = (*env)->GetBooleanField(env, billingConfig, billCdrLogHuntFid);
  if (bool)
    cdrevents |= CDRHUNT;
  else
    cdrevents &= ~CDRHUNT;

  return 0;
}


static int
ExtractAdvancedFields (JNIEnv *env, jobject advancedConfig) {
  int match = -1;
  int prevConfigServerType = myConfigServerType;

  /* maximum number of segments */
  max_segs = (int)(*env)->GetIntField(env, advancedConfig, adNumSegmentsFid);

  /* segment size */
  max_segsize = (int)(*env)->GetIntField(env, advancedConfig, adSegSizeFid);

  myConfigServerType = CONFIG_SERPLEX_GIS;
  match = FindServerConfig();
  if (match == -1) {
    LOCAL_DEBUG(JNI_TRUE, DEBUG_ERROR, ("Not Configured to run lus (servertype=%d)...", myConfigServerType));
    myConfigServerType = prevConfigServerType;
    return 0;
  }

  /* priority */
  serplexes[match].prio = (int)(*env)->GetIntField(env, advancedConfig, adPriorityFid);

  /* max number of threads */
  serplexes[match].threads = (int)(*env)->GetIntField(env, advancedConfig, adNumThreadsFid);

  /* thread stack size */
  serplexes[match].threadstack = (int)(*env)->GetIntField(env, advancedConfig, adThreadStackSizeFid);

  //  fprintf(stdout, "GIS serplex after setting advancedconfig:\n");
  //  PrintSerplex(&serplexes[match]);

  myConfigServerType = prevConfigServerType;

  return 0;
}


static int
ExtractSystemFields (JNIEnv *env, jobject systemConfig) {
  jstring jstr;
  const char *str;
  jboolean bool;
  int match;
  int prevServerType  = myConfigServerType;

  /* g711Ulaw64Duration */
  g711Ulaw64Duration = (int)(*env)->GetIntField(env, systemConfig, sysG711Ulaw64DurationFid);

  /* g711Alaw64Duration */
  g711Alaw64Duration = (int)(*env)->GetIntField(env, systemConfig, sysG711Alaw64DurationFid);

  /* g729Frames */
  g729Frames = (int)(*env)->GetIntField(env, systemConfig, sysG729FramesFid);

  /* g7231Frames */
  g7231Frames = (int)(*env)->GetIntField(env, systemConfig, sysG7231FramesFid);

  //  default codec
  defaultCodec = (int)(*env)->GetIntField(env, systemConfig, sysDefaultCodecFid);
  
  /* rollover time */
  rolloverTime = (int)(*env)->GetIntField(env, systemConfig, sysRolloverTimeFid);

  /* cache timeout */
  myConfigServerType = CONFIG_SERPLEX_GIS;
  match = FindServerConfig();
  if (match  !=  -1)
    serplexes[match].age.cache_timeout = (int)(*env)->GetIntField(env, systemConfig, sysCacheTimeoutFid);
  //sprintf(msg, "match=%d, serplexes[match].age.cache_timeout=%d", match, serplexes[match].age.cache_timeout);
  //LOCAL_DEBUG(JNI_TRUE, DEBUG_VERBOSE, (msg));
  myConfigServerType = prevServerType;

  /* enum domain */
  jstr = (jstring)(*env)->GetObjectField(env, systemConfig, sysEnumDomainFid);
  if (jstr == NULL) {
    memset(enumdomain, 0, ENUMDOMAIN_LEN);
  } else {
    str = (*env)->GetStringUTFChars(env, jstr, NULL);
    if (str != NULL) {
      strncpy(enumdomain, str, ENUMDOMAIN_LEN);
      enumdomain[ENUMDOMAIN_LEN-1] = '\0';
      (*env)->ReleaseStringUTFChars(env, jstr, str);
    }
    (*env)->DeleteLocalRef(env, jstr);
  }

  #if 0
  /* RADIUS server */
/*  jstr = (jstring)(*env)->GetObjectField(env, systemConfig, sysRadiusServerFid);
  if (jstr == NULL) {
    memset(rad_server_addr[0], 0, RADSERVERADDR_LEN);
  } else {
    str = (*env)->GetStringUTFChars(env, jstr, NULL);
    if (str != NULL) {
      strncpy(rad_server_addr[0], str, RADSERVERADDR_LEN);
      rad_server_addr[0][RADSERVERADDR_LEN-1] = '\0';
      (*env)->ReleaseStringUTFChars(env, jstr, str);
    }
    (*env)->DeleteLocalRef(env, jstr);
  }
*/
  /* RADIUS secret */
/* jstr = (jstring)(*env)->GetObjectField(env, systemConfig, sysRadiusSecretFid);
  if (jstr == NULL) {
    memset(secret[0], 0, SECRET_LEN);
  } else {
    str = (*env)->GetStringUTFChars(env, jstr, NULL);
    if (str != NULL) {
      strncpy(secret[0], str, SECRET_LEN);
      secret[0][SECRET_LEN-1] = '\0';
      (*env)->ReleaseStringUTFChars(env, jstr, str);
    }
    (*env)->DeleteLocalRef(env, jstr);
  }
*/
  #endif

  /* access control */
  bool = (*env)->GetBooleanField(env, systemConfig, sysAllowAllSrcFid);
  allowSrcAll = (bool == JNI_TRUE)?1:0;
  bool = (*env)->GetBooleanField(env, systemConfig, sysAllowAllDstFid);
  allowDestAll = (bool == JNI_TRUE)?1:0;
  bool = (*env)->GetBooleanField(env, systemConfig, sysAllowHairpinCallsFid);
  allowHairPin = (bool == JNI_TRUE)?1:0;
  bool = (*env)->GetBooleanField(env, systemConfig, sysRemoveRFC2833Fid);
  h323RemoveTcs2833 = (bool == JNI_TRUE)?TRUE:FALSE;
  bool = (*env)->GetBooleanField(env, systemConfig, sysRemoveT38Fid);
  h323RemoveTcsT38 = (bool == JNI_TRUE)?TRUE:FALSE;
#if FCE_REMOVED
  bool = (*env)->GetBooleanField(env, systemConfig, sysDefaultMediaRoutingFid);
  defaultMediaRouting = (bool == JNI_TRUE)?1:0;
  bool = (*env)->GetBooleanField(env, systemConfig, sysDefaultMidCallMediaChangeFid);
  defaultHideAddressChange = (bool == JNI_TRUE)?1:0;
#endif

  maxHunts = (int)(*env)->GetIntField(env, systemConfig, sysMaxHuntsFid);
  max_hunt_allowable_duration = (int)(*env)->GetIntField(env, systemConfig, sysMaxHuntsAllowableDurationFid);
  bool = (*env)->GetBooleanField(env, systemConfig, sysAllowAllRtpFid);
  allowRtpAll = (bool == JNI_TRUE)?1:0;

  bool = (*env)->GetBooleanField(env, systemConfig, sysForwardSrcAddrFid);
  forwardSrcAddr = (bool == JNI_TRUE)?1:0;
  max_call_duration = (int)(*env)->GetIntField(env, systemConfig, sysMaxCallDurationFid);


  // MSW Name
  jstr = (jstring)(*env)->GetObjectField(env, systemConfig, sysMswNameFid);
  if (jstr != NULL) {
    str = (*env)->GetStringUTFChars(env, jstr, NULL);
    if (str != NULL) {
      strncpy(mswname, str, MSWNAME_LEN);
      mswname[MSWNAME_LEN-1] = '\0';
      (*env)->ReleaseStringUTFChars(env, jstr, str);
    }
    (*env)->DeleteLocalRef(env, jstr);
  }


  // Management interface ip 
  jstr = (jstring)(*env)->GetObjectField(env, systemConfig, sysMgmtIpFid);
  if (jstr != NULL) {
    str = (*env)->GetStringUTFChars(env, jstr, NULL);
    if (str != NULL) {
      strncpy(mgmtInterfaceIp, str, 127);
      mgmtInterfaceIp[128-1] = '\0';
      (*env)->ReleaseStringUTFChars(env, jstr, str);
    }
    (*env)->DeleteLocalRef(env, jstr);
  }


  //CodeMap fields
  //LOCAL_DEBUG(JNI_TRUE, DEBUG_VERBOSE, ("Setting system codemap fields"));
  bool = (*env)->GetBooleanField(env, systemConfig, sysMapIsdnccFid);
  mapisdncc = (bool == JNI_TRUE)?1:0;
  //LOCAL_DEBUG(JNI_TRUE, DEBUG_VERBOSE, ("Setted mapisdncc"));
  bool = (*env)->GetBooleanField(env, systemConfig, sysMapLrjReasonFid);
  maplrjreason = (bool == JNI_TRUE)?1:0;
  //LOCAL_DEBUG(JNI_TRUE, DEBUG_VERBOSE, ("Setted maplrjreason"));
  jstr = (jstring)(*env)->GetObjectField(env, systemConfig, sysUseCodeMapFid);
  if (jstr != NULL) {
    str = (*env)->GetStringUTFChars(env, jstr, NULL);
    LOCAL_DEBUG(JNI_TRUE, DEBUG_VERBOSE, ("Setting codemap=%s",str));
    
    if (str != NULL) {
      strncpy(codemaptemplate, str, 128);
      codemaptemplate[128-1] = '\0';
      strcpy(codemapfilename, "codemap_");
      strcat(codemapfilename, codemaptemplate);
      strcat(codemapfilename, ".dat");
      (*env)->ReleaseStringUTFChars(env, jstr, str);
    }
    (*env)->DeleteLocalRef(env, jstr);
  }
  //LOCAL_DEBUG(JNI_TRUE, DEBUG_VERBOSE, ("Setted codemaptemplate"));


  return 0;
}


static int
ExtractNetRedundsFields (JNIEnv *env, jobject redNetConfig) {
  jstring jstr;
  const char *str;
  jobjectArray strArray;
  int i, len, size;
  jboolean bool;

  /* server type */
  jstr = (jstring)(*env)->GetObjectField(env, redNetConfig, redNetServerTypeFid);
  ispd_type = ISPD_TYPE_DISABLED;
  if (jstr != NULL) {
    str = (*env)->GetStringUTFChars(env, jstr, NULL);
    if (str != NULL) {
      if (!strcmp(str, "active"))
	ispd_type = ISPD_TYPE_ACTIVE;
      else if (!strcmp(str, "standby"))
	ispd_type = ISPD_TYPE_STANDBY;
      (*env)->ReleaseStringUTFChars(env, jstr, str);
    }
    (*env)->DeleteLocalRef(env, jstr);
  }

  // we support secondary interfaces.Till we add this support in the iView, do not update the secondary interfaces     
  //ispd_secondary.defined = 0;

  // vip is not used from 3.1 msw
  // primary interface name 
/*
  jstr = (jstring)(*env)->GetObjectField(env, redNetConfig, redNetPrimIfNameFid);
  if (jstr == NULL) {
    memset(ispd_primary.name, 0, MAX_ISPD_IFNAMELEN);
    ispd_primary.defined = 0;
  } else {
    str = (*env)->GetStringUTFChars(env, jstr, NULL);
    if (str != NULL) {
      nx_strlcpy(ispd_primary.name, str, MAX_ISPD_IFNAMELEN);
      (*env)->ReleaseStringUTFChars(env, jstr, str);
      ispd_primary.defined = 1;
    }
    (*env)->DeleteLocalRef(env, jstr);
  }

  // secondary interface name 
  ispd_secondary.defined = 0;
  jstr = (jstring)(*env)->GetObjectField(env, redNetConfig, redNetSecondIfNameFid);
  if (jstr == NULL) {
    memset(ispd_secondary.name, 0, MAX_ISPD_IFNAMELEN);
  } else {
    str = (*env)->GetStringUTFChars(env, jstr, NULL);
    if (str != NULL && strlen(str) != 0) {
      nx_strlcpy(ispd_secondary.name, str, MAX_ISPD_IFNAMELEN);
      (*env)->ReleaseStringUTFChars(env, jstr, str);
      ispd_secondary.defined = 1;
    }
    (*env)->DeleteLocalRef(env, jstr);
  }

  // primary interface vips/
  strArray = (jobjectArray)(*env)->GetObjectField(env, redNetConfig, redNetPrimaryInterfaceVipsFid);
  if(strArray ==  NULL){
    memset(ispd_primary.vip, 0, MAX_ISPD_IPLEN);
  }else{
    len = (*env)->GetArrayLength(env, strArray);
    for (i = 0, size = 0; i < len; i++) {
      jstr = (jstring)(*env)->GetObjectArrayElement(env, strArray, i);
      if (jstr == NULL)
	      continue;
      str = (*env)->GetStringUTFChars(env, jstr, NULL);
      if (str != NULL) {
	      nx_strlcpy((char*)&ispd_primary.vip, str, MAX_ISPD_IPLEN);
	      (*env)->ReleaseStringUTFChars(env, jstr, str);
      }
      (*env)->DeleteLocalRef(env, jstr);
    }

  }
  // secondary interface vips
  strArray = (jobjectArray)(*env)->GetObjectField(env, redNetConfig, redNetSecondaryInterfaceVipsFid);
  if(strArray ==  NULL){
    memset(ispd_secondary.vip, 0, MAX_ISPD_IPLEN);
  }else{
    len = (*env)->GetArrayLength(env, strArray);
    for (i = 0, size = 0; i < len; i++) {
      jstr = (jstring)(*env)->GetObjectArrayElement(env, strArray, i);
      if (jstr == NULL)
	      continue;
      str = (*env)->GetStringUTFChars(env, jstr, NULL);
      if (str != NULL) {
	      nx_strlcpy((char*)&ispd_secondary.vip, str, MAX_ISPD_IPLEN);
	      (*env)->ReleaseStringUTFChars(env, jstr, str);
      }
      (*env)->DeleteLocalRef(env, jstr);
    }

  }


  // primary interface router /
  jstr = (jstring)(*env)->GetObjectField(env, redNetConfig, redNetPrimIfRouterFid);
  if (jstr == NULL) {
    memset(ispd_primary.router, 0, MAX_ISPD_ROUTERLEN);
  } else {
    str = (*env)->GetStringUTFChars(env, jstr, NULL);
    if (str != NULL) {
      nx_strlcpy(ispd_primary.router, str, MAX_ISPD_ROUTERLEN);
      (*env)->ReleaseStringUTFChars(env, jstr, str);
    }
    (*env)->DeleteLocalRef(env, jstr);
  }

  // secondary interface router /
  jstr = (jstring)(*env)->GetObjectField(env, redNetConfig, redNetSecondIfRouterFid);
  if (jstr == NULL) {
    memset(ispd_secondary.router, 0, MAX_ISPD_ROUTERLEN);
  } else {
    str = (*env)->GetStringUTFChars(env, jstr, NULL);
    if (str != NULL) {
      nx_strlcpy(ispd_secondary.router, str, MAX_ISPD_ROUTERLEN);
      (*env)->ReleaseStringUTFChars(env, jstr, str);
    }
    (*env)->DeleteLocalRef(env, jstr);
  }
*/

  /* control interface name */
  jstr = (jstring)(*env)->GetObjectField(env, redNetConfig, redNetControlIfNameFid);
  if (jstr == NULL) {
    memset(ispd_ctl.name, 0, MAX_ISPD_IFNAMELEN);
    ispd_ctl.defined = 0;
  } else {
    str = (*env)->GetStringUTFChars(env, jstr, NULL);
    if (str != NULL) {
      nx_strlcpy(ispd_ctl.name, str, MAX_ISPD_IFNAMELEN);
      (*env)->ReleaseStringUTFChars(env, jstr, str);
      ispd_ctl.defined = 1;
    }
    (*env)->DeleteLocalRef(env, jstr);
  }

  /* peer iservers */
  strArray = (jobjectArray)(*env)->GetObjectField(env, redNetConfig, redNetPeersFid);
  /* erase all the current peers */
  for (i = 0; i < ispd_ctl.peer_count; i++)
    memset(&ispd_ctl.peer_iservers[i][0], 0, MAX_ISPD_IPLEN);
  ispd_ctl.peer_count = 0;
  if (strArray != NULL) {
    /* copy all the new config */
    len = (*env)->GetArrayLength(env, strArray);
    for (i = 0, size = 0; i < len; i++) {
      jstr = (jstring)(*env)->GetObjectArrayElement(env, strArray, i);
      if (jstr == NULL)
	continue;
      str = (*env)->GetStringUTFChars(env, jstr, NULL);
      if (str != NULL) {
	nx_strlcpy(&ispd_ctl.peer_iservers[size++][0], str, MAX_ISPD_IPLEN);
	(*env)->ReleaseStringUTFChars(env, jstr, str);
      }
      (*env)->DeleteLocalRef(env, jstr);
    }
    ispd_ctl.peer_count = size;
  }

  // state full call migration

  bool = (*env)->GetBooleanField(env, redNetConfig, redNetScmFid);
  doScm = (bool == JNI_FALSE)?0:1;


  return 0;
}


static int
ExtractDbRedundsFields (JNIEnv *env, jobject redDbConfig) {
  jstring jstr;
  jboolean bool;
  const char *str;
  int port, prio;
  jboolean restartNeeded = JNI_FALSE;

  /* server status */
  bool = (*env)->GetBooleanField(env, redDbConfig, redDbServerStatusFid);


  if ((bool == JNI_TRUE && RSDConfig != 1) ||
      (bool == JNI_FALSE && RSDConfig != 0)) {
    RSDConfig = (bool == JNI_FALSE)?0:1;
    restartNeeded = JNI_TRUE;
  }

  /* interface name */
  jstr = (jstring)(*env)->GetObjectField(env, redDbConfig, redDbIfNameFid);
  if (jstr == NULL) {
    memset(rs_ifname, 0, RS_LINELEN);
    restartNeeded = JNI_TRUE;
  } else {
    str = (*env)->GetStringUTFChars(env, jstr, NULL);
    if (str != NULL) {
      if (strcmp(str, rs_ifname)) {
	nx_strlcpy(rs_ifname, str, RS_LINELEN);
	restartNeeded = JNI_TRUE;
      }
      (*env)->ReleaseStringUTFChars(env, jstr, str);
    }
    (*env)->DeleteLocalRef(env, jstr);
  }

  /* mcast address */
  jstr = (jstring)(*env)->GetObjectField(env, redDbConfig, redDbMcastAddrFid);
  if (jstr == NULL) {
    nx_strlcpy(rs_mcast_addr, RS_DEF_MCAST_ADDR, RS_LINELEN);
    restartNeeded = JNI_TRUE;
  } else {
    str = (*env)->GetStringUTFChars(env, jstr, NULL);
    if (str != NULL) {
      if (strcmp(str, rs_mcast_addr)) {
	nx_strlcpy(rs_mcast_addr, str, RS_LINELEN);
	restartNeeded = JNI_TRUE;
      }
      (*env)->ReleaseStringUTFChars(env, jstr, str);
    }
    (*env)->DeleteLocalRef(env, jstr);
  }

  /* port */
  port = (int)(*env)->GetIntField(env, redDbConfig, redDbPortFid);
  if (port == 0) {
    nx_strlcpy(rs_port, RS_DEF_PORT, RS_LINELEN);
    restartNeeded = JNI_TRUE;
  } else {
    if (atoi(rs_port) != port) {
      memset(rs_port, 0, RS_LINELEN);
      sprintf(rs_port, "%d", port);
      restartNeeded = JNI_TRUE;
    }
  }

  /* priority */
  prio = (int)(*env)->GetIntField(env, redDbConfig, redDbPriorityFid);
  if (rs_host_prio != prio) {
    rs_host_prio = prio;
    restartNeeded = JNI_TRUE;
  }

  /* if any of the configuration changed, restart RSD */
  if (restartNeeded == JNI_TRUE) {
    if (Java_com_nextone_JServer_BridgeServer_restartRSD(env, NULL) == JNI_TRUE) {
      LOCAL_DEBUG(JNI_TRUE, DEBUG_VERBOSE, ("Configuration changed, restarted RSD"));
    } else {
      LOCAL_DEBUG(JNI_TRUE, DEBUG_WARNING, ("Configuration changed, restarting RSD failed"));
    }
  }

  return 0;
}


static int
ExtractRedundsFields (JNIEnv *env, jobject redConfig) {
  jobject redNetConfig, redDbConfig;
  int status;

  redNetConfig = (*env)->GetObjectField(env, redConfig, redNetConfigFid);
  if (redNetConfig == NULL) {
    LOCAL_DEBUG(JNI_TRUE, DEBUG_ERROR, ("error getting net redundancy config object"));
    (*env)->PopLocalFrame(env, NULL);
    return -1;
  }
  redDbConfig = (*env)->GetObjectField(env, redConfig, redDbConfigFid);
  if (redDbConfig == NULL) {
    LOCAL_DEBUG(JNI_TRUE, DEBUG_ERROR, ("error getting db redundancy config object"));
    (*env)->PopLocalFrame(env, NULL);
    return -1;
  }

  status = ExtractNetRedundsFields(env, redNetConfig);
  if (status == 0)
    status = ExtractDbRedundsFields(env, redDbConfig);

  (*env)->DeleteLocalRef(env, redNetConfig);
  (*env)->DeleteLocalRef(env, redDbConfig);

  return status;
}


static int
ExtractLoggingFields (JNIEnv *env, jobject loggingConfig) {
  int i, j;
  int prevServerType = myConfigServerType;
  int match, len;
  jstring module;
  char ch;
  const char* moduleLog;
  jboolean bool;
  int level = 0;
  int newLevel = 0;
  int levelValue  = 0;
  int newSLevel = 0;
        
  for (i = 0; i < MAX_LOG_SERVERS; i++) {
    if ((*env)->PushLocalFrame(env, 20))
      return -1;

    myConfigServerType = logServers[i];
    match = FindServerConfig();
    if (match != -1) {
      module = (jstring)(*env)->CallObjectMethod(env, loggingConfig, logGetModMid, (jint)myConfigServerType);
      if (module == NULL) { 
	      (*env)->PopLocalFrame(env, NULL);
	      return -1;
      }

      moduleLog = (*env)->GetStringUTFChars(env, module, NULL);
      if (moduleLog == NULL) {
	      (*env)->PopLocalFrame(env, NULL);
	      return -1;
      }

      len = strlen(moduleLog);

      for (j = 0; j < len; j++) {
	      ch = moduleLog[j];
	      serplexes[match].debconfigs.netLogStatus[j] = (ch == '1')?NETLOG_DEBUG4:0;
      }
      (*env)->ReleaseStringUTFChars(env, module, moduleLog);
      (*env)->PopLocalFrame(env, NULL);
    }
  }
  myConfigServerType = CONFIG_SERPLEX_GIS;

  match = FindServerConfig();
  if(match  !=  -1){
    newLevel  =  (*env)->GetIntField(env, loggingConfig, logDebugLevelFid);

    //  if the packet is received from old iView, set the hdebug level to 3
    if(newLevel ==  -1)
      levelValue  = 3;
    else
      levelValue  = newLevel;

    bool = (*env)->GetBooleanField(env, loggingConfig, logUpdateAllocationFid);
    updateAllocations = (bool == JNI_TRUE)?1:0;

    bool = (*env)->GetBooleanField(env, loggingConfig, logTpktchanFid);
    level |=  serplexes[match].cusconfigs.customLogStatus[TPKTCHAN]   = (bool == JNI_TRUE)?levelValue:0;
    bool = (*env)->GetBooleanField(env, loggingConfig, logPererrFid);
    level |=  serplexes[match].cusconfigs.customLogStatus[PERERR]   = (bool == JNI_TRUE)?levelValue:0;
    bool = (*env)->GetBooleanField(env, loggingConfig, logCmFid);
    level |=  serplexes[match].cusconfigs.customLogStatus[CM]   = (bool == JNI_TRUE)?levelValue:0;
    bool = (*env)->GetBooleanField(env, loggingConfig, logCmapicbFid);
    level |=  serplexes[match].cusconfigs.customLogStatus[CMAPICB]  = (bool == JNI_TRUE)?levelValue:0;
    bool = (*env)->GetBooleanField(env, loggingConfig, logCmapiFid);
    level |=  serplexes[match].cusconfigs.customLogStatus[CMAPI]= (bool == JNI_TRUE)?levelValue:0;

    //  if the packet is received from old iView, do not update the following logs
    if(newLevel ==  -1){
      level |=  serplexes[match].cusconfigs.customLogStatus[UDPCHAN];
      level |=  serplexes[match].cusconfigs.customLogStatus[CMERR];
      level |=  serplexes[match].cusconfigs.customLogStatus[LI];
      level |=  serplexes[match].cusconfigs.customLogStatus[LIINFO];
      serplexes[match].cusconfigs.level = (level > 0) ? 3: 0;
    }
    else{
      bool = (*env)->GetBooleanField(env, loggingConfig, logUdpchanFid);
      level |=  serplexes[match].cusconfigs.customLogStatus[UDPCHAN]   = (bool == JNI_TRUE)?levelValue:0;
      bool = (*env)->GetBooleanField(env, loggingConfig, logCmerrFid);
      level |=  serplexes[match].cusconfigs.customLogStatus[CMERR]= (bool == JNI_TRUE)?levelValue:0;
      bool = (*env)->GetBooleanField(env, loggingConfig, logLiFid);
      level |=  serplexes[match].cusconfigs.customLogStatus[LI]= (bool == JNI_TRUE)?levelValue:0;
      bool = (*env)->GetBooleanField(env, loggingConfig, logLiinfoFid);
      level |=  serplexes[match].cusconfigs.customLogStatus[LIINFO]= (bool == JNI_TRUE)?levelValue:0;
      serplexes[match].cusconfigs.level = newLevel;
    }
     
    newSLevel  =  (*env)->GetIntField(env, loggingConfig, logSDebugLevelFid);
    serplexes[match].cusconfigs.slevel = (newSLevel >= 0) ? newSLevel : 3;
  }
  myConfigServerType = prevServerType;

  return 0;
}


static int
ExtractRadiusFields (JNIEnv *env, jobject radiusConfig) {
  jobjectArray strArray;
  jobjectArray secretArray;
  int i;
  int j = 0;
  int len;
  jstring jserver, jsecret;
  const char *server, *radsecret;
  jboolean bool;

  strArray    = (jobjectArray)(*env)->GetObjectField(env, radiusConfig, radiusServersFid);
  secretArray = (jobjectArray)(*env)->GetObjectField(env, radiusConfig, radiusSecretsFid);

  if(strArray ==  NULL){
    strcpy(rad_server_addr[0],"");
  }else{

    len = (*env)->GetArrayLength(env, strArray);

    if( len > MAX_NUM_RAD_ENTRIES)
      len = MAX_NUM_RAD_ENTRIES;

    for (i = 0;i < len; i++) {
      server  = NULL;
      radsecret = NULL;

      jserver = (jstring)(*env)->GetObjectArrayElement(env, strArray, i);
      if (jserver == NULL)
	      continue;
      server    = (*env)->GetStringUTFChars(env, jserver, NULL);

      if(secretArray  !=  NULL)
        jsecret = (jstring)(*env)->GetObjectArrayElement(env, secretArray, i);

      if(jsecret != NULL)
        radsecret = (*env)->GetStringUTFChars(env, jsecret, NULL);
      nx_strlcpy(rad_server_addr[j], server,RADSERVERADDR_LEN);
	    (*env)->ReleaseStringUTFChars(env, jserver, server);
      if(radsecret  !=  NULL){
	      nx_strlcpy(secret[j++], radsecret,SECRET_LEN);
	      (*env)->ReleaseStringUTFChars(env, jsecret, radsecret);
      }else{
        nx_strlcpy(secret[j++], "",SECRET_LEN);
      }

      if(jserver  !=  NULL)
        (*env)->DeleteLocalRef(env, jserver);
      if(jsecret  !=  NULL)
        (*env)->DeleteLocalRef(env, jsecret);
    }


    // dir name
/*    jstr = (jstring)(*env)->GetObjectField(env, radiusConfig, radiusDirNameFid);
    if (jstr == NULL) {
      memset(rad_dirname, 0, CDRDIRNAMELEN);
    } else {
      str = (*env)->GetStringUTFChars(env, jstr, NULL);
      if (str != NULL) {
        strncpy(rad_dirname, str, CDRDIRNAMELEN);
        rad_dirname[CDRDIRNAMELEN-1] = '\0';
        (*env)->ReleaseStringUTFChars(env, jstr, str);
      }
      (*env)->DeleteLocalRef(env, jstr);
    }
*/

    // timeout
    rad_timeout = (int)(*env)->GetIntField(env, radiusConfig, radiusTimeoutFid);
    rad_retries = (int)(*env)->GetIntField(env, radiusConfig, radiusRetryFid);
    // dead time
    rad_deadtime = (int)(*env)->GetIntField(env, radiusConfig, radiusDeadTimeFid);
    // send accounting messages 
    bool = (*env)->GetBooleanField(env, radiusConfig, radiusSendMsgFid);
    rad_acct = (bool == JNI_FALSE)?FALSE:TRUE;

    // use overloaded session id format 
    bool = (*env)->GetBooleanField(env, radiusConfig, radiusUseOSIFFid);
    rad_acct_session_id_overloaded = (bool == JNI_FALSE)?FALSE:TRUE;
  }

 
  return 0;

}



static jboolean generateCfgFile () {
  time_t clock;
  char buf1[256] = {0}, buf2[128] = {0};

  time(&clock);
  sprintf(buf1, "# Generated by jserver - %s\n", ctime_r(&clock, buf2));
  if (GenerateCfgFile("jserver", "/usr/local/nextone/bin/server.cfg", buf1))
    return JNI_FALSE;

  return JNI_TRUE;
}


/* extract the config variables and create the server.cfg file */
JNIEXPORT jboolean JNICALL
Java_com_nextone_JServer_BridgeServer_setNativeIserverConfig (JNIEnv *env, jobject obj, jobject config) {
  jobject tmpObj;

  if ((*env)->PushLocalFrame(env, 20))
    return JNI_FALSE;

  LOCAL_DEBUG(JNI_TRUE, DEBUG_VERBOSE, ("iserverconfig: received config:"));
//  printObject(env, DEBUG_VERBOSE, config);

  /* now extract the sip configuration */
  tmpObj = (*env)->GetObjectField(env, config, sipConfigFid);
  if (tmpObj == NULL) {
    LOCAL_DEBUG(JNI_TRUE, DEBUG_ERROR, ("error getting sipConfig object"));
    (*env)->PopLocalFrame(env, NULL);
    return JNI_FALSE;
  } 
  if (ExtractSipFields(env, tmpObj)) {
    (*env)->PopLocalFrame(env, NULL);
    return JNI_FALSE;
  }
  (*env)->DeleteLocalRef(env, tmpObj);

  /* now extract the H.323 configuration */
  tmpObj = (*env)->GetObjectField(env, config, h323ConfigFid);
  if (tmpObj == NULL) {
    LOCAL_DEBUG(JNI_TRUE, DEBUG_ERROR, ("error getting h323Config object"));
    (*env)->PopLocalFrame(env, NULL);
    return JNI_FALSE;
  }
  if (ExtractH323Fields(env, tmpObj)) {
    (*env)->PopLocalFrame(env, NULL);
    return JNI_FALSE;
  }
  (*env)->DeleteLocalRef(env, tmpObj);

  /* now extract the FCE configuration */
  tmpObj = (*env)->GetObjectField(env, config, fceConfigFid);
  if (tmpObj == NULL) {
    LOCAL_DEBUG(JNI_TRUE, DEBUG_ERROR, ("error getting fceConfig object"));
    (*env)->PopLocalFrame(env, NULL);
    return JNI_FALSE;
  }
  if (ExtractFceFields(env, tmpObj)) {
    (*env)->PopLocalFrame(env, NULL);
    return JNI_FALSE;
  }
  (*env)->DeleteLocalRef(env, tmpObj);


  /* now extract the FCE New configuration */
  tmpObj = (*env)->GetObjectField(env, config, fceConfigNewFid);
  if (tmpObj == NULL) {
    LOCAL_DEBUG(JNI_TRUE, DEBUG_ERROR, ("error getting fceConfig new object"));
    (*env)->PopLocalFrame(env, NULL);
    return JNI_FALSE;
  }
  if (ExtractFceNewFields(env, tmpObj)) {
    (*env)->PopLocalFrame(env, NULL);
    return JNI_FALSE;
  }
  (*env)->DeleteLocalRef(env, tmpObj);


  /* now extract the billing configuration */
  tmpObj = (*env)->GetObjectField(env, config, billingConfigFid);
  if (tmpObj == NULL) {
    LOCAL_DEBUG(JNI_TRUE, DEBUG_ERROR, ("error getting billingConfig object"));
    (*env)->PopLocalFrame(env, NULL);
    return JNI_FALSE;
  }
  if (ExtractBillingFields(env, tmpObj)) {
    (*env)->PopLocalFrame(env, NULL);
    return JNI_FALSE;
  }
  (*env)->DeleteLocalRef(env, tmpObj);

  /* now extract the advanced configuration */
  tmpObj = (*env)->GetObjectField(env, config, advancedConfigFid);
  if (tmpObj == NULL) {
    LOCAL_DEBUG(JNI_TRUE, DEBUG_ERROR, ("error getting advancedConfig object"));
    (*env)->PopLocalFrame(env, NULL);
    return JNI_FALSE;
  }
  if (ExtractAdvancedFields(env, tmpObj)) {
    (*env)->PopLocalFrame(env, NULL);
    return JNI_FALSE;
  }
  (*env)->DeleteLocalRef(env, tmpObj);

  /* now extract the system configuration */
  tmpObj = (*env)->GetObjectField(env, config, systemConfigFid);
  if (tmpObj == NULL) {
    LOCAL_DEBUG(JNI_TRUE, DEBUG_ERROR, ("error getting systemConfig object"));
    (*env)->PopLocalFrame(env, NULL);
    return JNI_FALSE;
  }
  if (ExtractSystemFields(env, tmpObj)) {
    (*env)->PopLocalFrame(env, NULL);
    return JNI_FALSE;
  }
  (*env)->DeleteLocalRef(env, tmpObj);

  /* now extract the redundancy configuration */
  tmpObj = (*env)->GetObjectField(env, config, redConfigFid);
  if (tmpObj == NULL) {
    LOCAL_DEBUG(JNI_TRUE, DEBUG_ERROR, ("error getting redundancyConfig object"));
    (*env)->PopLocalFrame(env, NULL);
    return JNI_FALSE;
  }
  if (ExtractRedundsFields(env, tmpObj)) {
    (*env)->PopLocalFrame(env, NULL);
    return JNI_FALSE;
  }
  (*env)->DeleteLocalRef(env, tmpObj);

  /* now extract the logging configuration */
  tmpObj = (*env)->GetObjectField(env, config, loggingConfigFid);
  if (tmpObj == NULL) {
    LOCAL_DEBUG(JNI_TRUE, DEBUG_ERROR, ("error getting loggingConfig object"));
    (*env)->PopLocalFrame(env, NULL);
    return JNI_FALSE;
  }
  if (ExtractLoggingFields(env, tmpObj)) {
    (*env)->PopLocalFrame(env, NULL);
    return JNI_FALSE;
  }
  (*env)->DeleteLocalRef(env, tmpObj);


  /* now extract the radius configuration */
  tmpObj = (*env)->GetObjectField(env, config, radiusConfigFid);
  if (tmpObj == NULL) {
    LOCAL_DEBUG(JNI_TRUE, DEBUG_ERROR, ("error getting raiusConfig object"));
    (*env)->PopLocalFrame(env, NULL);
    return JNI_FALSE;
  }
  if (ExtractRadiusFields(env, tmpObj)) {
    (*env)->PopLocalFrame(env, NULL);
    return JNI_FALSE;
  }
  (*env)->DeleteLocalRef(env, tmpObj);


  LOCAL_DEBUG(JNI_TRUE, DEBUG_VERBOSE, ("creating new server.cfg file"));

  (*env)->PopLocalFrame(env, NULL);

  return generateCfgFile();
}


JNIEXPORT jboolean JNICALL
Java_com_nextone_JServer_JServer_generateCfgFile (JNIEnv *env, jobject obj) {
  return generateCfgFile();
}


JNIEXPORT jboolean JNICALL
Java_com_nextone_JServer_JServer_setJServerConfig (JNIEnv *env, jobject obj, jstring r, jstring w) {
  const char *read = NULL, *write = NULL;

  read = (*env)->GetStringUTFChars(env, r, NULL);
  if (read == NULL)
    return JNI_FALSE;

  write = (*env)->GetStringUTFChars(env, w, NULL);
  if (write == NULL) {
    (*env)->ReleaseStringUTFChars(env, r, read);
    return JNI_FALSE;
  }

  nx_strlcpy(readPass, read, 20);
  nx_strlcpy(writePass, write, 20);

  (*env)->ReleaseStringUTFChars(env, r, read);
  (*env)->ReleaseStringUTFChars(env, w, write);

  return JNI_TRUE;
}


JNIEXPORT jstring JNICALL
Java_com_nextone_JServer_JServer_getJServerReadPassword (JNIEnv *env, jobject obj) {
  return (*env)->NewStringUTF(env, readPass);
}


JNIEXPORT jstring JNICALL
Java_com_nextone_JServer_JServer_getJServerWritePassword (JNIEnv *env, jobject obj) {
  return (*env)->NewStringUTF(env, writePass);
}


JNIEXPORT jstring JNICALL
Java_com_nextone_JServer_JServer_getJServerLogLevel (JNIEnv *env, jobject obj) {
  return (*env)->NewStringUTF(env, jsLogLevel);
}


JNIEXPORT jint JNICALL
Java_com_nextone_JServer_JServer_getJServerCompression (JNIEnv *env, jobject obj) {
  return (jint)jsCompression;
}


JNIEXPORT jstring JNICALL
Java_com_nextone_JServer_JServer_getJServerLogFileName (JNIEnv *env, jobject obj) {
  return (*env)->NewStringUTF(env, jsLogFile);
}


JNIEXPORT jstring JNICALL
Java_com_nextone_JServer_BridgeServer_getAravoxConfig (JNIEnv *env, jobject obj) {
  jstring jstr = NULL;
  const char *xmlMessage = "";//FCECreateXMLFromConfig();

  if (xmlMessage == NULL) {
    LOCAL_DEBUG(JNI_TRUE, DEBUG_ERROR, ("error creating aravox XML message"));
    return NULL;
  }

  jstr = (*env)->NewStringUTF(env, xmlMessage);
  free((void *)xmlMessage);

  return jstr;
}


JNIEXPORT jobjectArray JNICALL
Java_com_nextone_JServer_BridgeServer_getInterfaceNames (JNIEnv *env, jobject obj) {
  struct ifi_info *ifihead = NULL;
  char *curName = NULL;
  jobjectArray result = NULL;
  jstring jstr;

  ifihead = initIfs();

  while ((curName = GetNextIfname(ifihead, curName)) != NULL) {
    if (!strncmp(curName, "lo", 2))
      continue;

	if (!strncmp(curName, "hhn", 3))
      continue;
	
    jstr = (*env)->NewStringUTF(env, curName);
    if (jstr == NULL) {
      free_ifi_info(ifihead);
      return NULL;
    }

    result = (*env)->CallStaticObjectMethod(env, sysutilClass, suCreateStringArrayMid, result, jstr);
    (*env)->DeleteLocalRef(env, jstr);
    if (result == NULL) {
      free_ifi_info(ifihead);
      return NULL;
    }
  }

  free_ifi_info(ifihead);

  return result;
}


/**
 * returns InetAddress [] of all the addresses of interfaces in the system, does
 * not include any aliased interfaces
 */
JNIEXPORT jobjectArray JNICALL
Java_com_nextone_JServer_JServer_getAllLocalAddress (JNIEnv *env, jclass cls) {
  struct ifi_info *ifihead = NULL, *ptr;
  jobjectArray result = NULL;
  jstring jstr;
  char buf[INET_ADDRSTRLEN];
  const char *cp;
  jobject addr;

  for (ifihead = ptr = initIfs(); ptr != NULL; ptr = ptr->ifi_next) {

    cp = inet_ntop(AF_INET, &ptr->ifi_addr->sin_addr, buf, INET_ADDRSTRLEN);
    if (cp == NULL) {
      free_ifi_info(ifihead);
      return NULL;
    }

    jstr = (*env)->NewStringUTF(env, cp);
    if (jstr == NULL) {
      free_ifi_info(ifihead);
      return NULL;
    }

    addr = (*env)->CallStaticObjectMethod(env, inetAddressClass, inetAddressGetByNameMid, jstr);
    if (addr == NULL) {
      free_ifi_info(ifihead);
      (*env)->DeleteLocalRef(env, jstr);
      return NULL;
    }

    result = (*env)->CallStaticObjectMethod(env, sysutilClass, suCreateObjectArrayMid, result, addr);

    (*env)->DeleteLocalRef(env, addr);
    (*env)->DeleteLocalRef(env, jstr);

    if (result == NULL) {
      free_ifi_info(ifihead);
      return NULL;
    }
  }

  free_ifi_info(ifihead);

  return result;
}


/**
 * looks if the IP address given is one any of the interfaces (including any aliased interface)
 * and returns it that interface status is up. The ip should be in network byte order.
 */
JNIEXPORT jboolean JNICALL
Java_com_nextone_JServer_JServer_isInterfaceUp (JNIEnv *env, jclass cls, jlong iplong) {
  struct ifi_info *ifihead = NULL, *ptr;
  jboolean result = JNI_FALSE;
  char ipaddr[INET_ADDRSTRLEN] = {0};
  unsigned int ip = (unsigned int)iplong;

  FormatIpAddress(ip, ipaddr);

  for (ifihead = ptr = initIfs2(1); ptr != NULL; ptr = ptr->ifi_next) {
    LOCAL_DEBUG(JNI_TRUE, DEBUG_VERBOSE, ("checking interface %s for ip %s", ptr->ifi_name, ipaddr));
    LOCAL_DEBUG(JNI_TRUE, DEBUG_VERBOSE, ("interface ip = 0x%x, given = 0x%x, upflag = %d", ptr->ifi_addr->sin_addr.s_addr, ip, (ptr->ifi_flags & IFF_UP)));
    if (htonl(ptr->ifi_addr->sin_addr.s_addr) == ip &&
	(ptr->ifi_flags & IFF_UP)) {
      result = JNI_TRUE;
      break;
    }
  }

  free_ifi_info(ifihead);

  LOCAL_DEBUG(JNI_TRUE, DEBUG_VERBOSE, ("isInterfaceUp returns %s\n", (result == JNI_TRUE)?"TRUE":"FALSE"));

  return result;
}


/**
 * returns true if database redundancy is disabled or is enabled and local machine
 * is master
 */
JNIEXPORT jboolean JNICALL
Java_com_nextone_JServer_JServer_isDatabaseOpAllowed (JNIEnv *env, jclass cls) {
  jboolean result = JNI_TRUE;

  if (RSDConfig)
    result = Java_com_nextone_JServer_JServer_isInterfaceUp(env, cls, Java_com_nextone_JServer_JServer_getDatabasePrimary(env, cls));

  LOCAL_DEBUG(JNI_TRUE, DEBUG_VERBOSE, ("isDatabaseOpAllowed returns %s", (result == JNI_TRUE)?"TRUE":"FALSE"));

  return result;
}




/**
 * returns true if local machine is network master
 */
JNIEXPORT jboolean JNICALL
Java_com_nextone_JServer_JServer_isNetworkMaster (JNIEnv *env, jclass cls) {
  jboolean result = JNI_TRUE;

  if(getMswNMState() != 2)
    result = JNI_FALSE;

   LOCAL_DEBUG(JNI_TRUE, DEBUG_VERBOSE, ("isNetworkMaster returns %s", (result == JNI_TRUE)?"TRUE":"FALSE"));

  return result;
}
