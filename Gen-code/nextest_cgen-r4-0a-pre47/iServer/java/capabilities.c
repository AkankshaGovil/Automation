#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include "JServer.h"
#include "BridgeServer.h"
#include "licenseIf.h"
#include "log.h"
#include "utils.h"

static jclass iscClass = NULL;
static jmethodID iscConstId = NULL;

static jfieldID billingFid = NULL;

//static jfieldID billRadiusFid = NULL;
static jmethodID setRadiusMid = NULL;


static int
SetBillingFields (JNIEnv *env, jobject billCap) {
  jboolean result = JNI_FALSE;

  if (radiusEnabled() > 0)
    result = JNI_TRUE;

  LOCAL_DEBUG(JNI_TRUE, DEBUG_VERBOSE, ("radius Enabled returns %s", (result == JNI_TRUE
    )?"TRUE":"FALSE"));

  /* fast start */
  //(*env)->SetBooleanField(env, billCap, billRadiusFid, result?JNI_TRUE:JNI_FALSE);

  (*env)->CallVoidMethod(env, billCap, setRadiusMid, result?JNI_TRUE:JNI_FALSE);

  return 0;
}




/** returns a new capabilities object (Capabilities) */
static jobject
CreateNewCapObject (JNIEnv *env) {
  jobject cap,obj;

  if ((*env)->PushLocalFrame(env, 20))
    return NULL;

  LOCAL_DEBUG(JNI_TRUE, DEBUG_VERBOSE, ("creating new cap object"));
  cap = (*env)->NewObject(env, iscClass, iscConstId);

  /* now fill in the billing capabilities */
  LOCAL_DEBUG(JNI_TRUE, DEBUG_VERBOSE, ("creating billing capabilities"));
  obj = (*env)->GetObjectField(env, cap, billingFid);
  if (obj == NULL) {
    LOCAL_DEBUG(JNI_TRUE, DEBUG_ERROR, ("error getting billing capabilities"));
    (*env)->PopLocalFrame(env, NULL);
    return NULL;
  } 
  if (SetBillingFields(env, obj)) {
    (*env)->PopLocalFrame(env, NULL);
    return NULL;
  }
  (*env)->DeleteLocalRef(env, obj);
  
  return (*env)->PopLocalFrame(env, cap);
}




JNIEXPORT jobject JNICALL
Java_com_nextone_JServer_BridgeServer_getIServerCapabilities (JNIEnv *env, jobject obj) {
  jobject cap;
  
  cap = CreateNewCapObject(env);
  if (cap == NULL) {
    ThrowBridgeException(env, "Error accessing iServer capabilities");
    return NULL;
  }

  return cap;
}



int
iserverCapInit (JNIEnv *env, jobject cap) {
  jobject obj;
  jclass cls;

  

  iscClass = GetGlobalClassReference(env, "com/nextone/common/Capabilities");
  if (iscClass == NULL)
    return -1;
  

  iscConstId = (*env)->GetMethodID(env, iscClass, "<init>", "()V");
  if (iscConstId == NULL)
    return -1;



  /* billing cap*/
  billingFid = (*env)->GetFieldID(env, iscClass, "billing", "Lcom/nextone/common/Capabilities$Billing;");
  if (billingFid == NULL)
    return -1;
  
  obj = (*env)->GetObjectField(env, cap, billingFid);
  if (obj == NULL)
    return -1;
  
  cls = (*env)->GetObjectClass(env, obj);
  if (cls == NULL) {
    (*env)->DeleteLocalRef(env, obj);
    return -1;
  }
  
  /* billing cap*/
  setRadiusMid = (*env)->GetMethodID(env, cls, "setRadiusEnabled", "(Z)V");
  if (setRadiusMid == NULL) {
    (*env)->DeleteLocalRef(env, obj);
    (*env)->DeleteLocalRef(env, cls);
    return -1;
  }

  //billRadiusFid = (*env)->GetFieldID(env, cls, "radiusEnabled", "Z");
  //(*env)->CallIntMethod(env, billingConfig, billRadiusMid);
  //if (billRadiusFid == NULL) {
  //  (*env)->DeleteLocalRef(env, obj);
  //  (*env)->DeleteLocalRef(env, cls);
  //  return -1;
  //}
 
  (*env)->DeleteLocalRef(env, obj);
  (*env)->DeleteLocalRef(env, cls);


  return 0;
}


