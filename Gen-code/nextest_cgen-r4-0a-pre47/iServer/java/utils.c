#include <jni.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include "utils.h"
#include "cli.h"
#include "lsconfig.h"


extern int FindServerConfig (void);
extern int ServerSetLogging (char*, serplex_config*);


static char dbgMsg[1024] = {0};

static jclass jserverClass = NULL;
static jmethodID jsPrintDebugMid = NULL;
static jclass bridgeExceptionClass = NULL;
static jclass existExceptionClass = NULL;
static jclass noEntryExceptionClass = NULL;
static jclass exceptionClass = NULL;

jclass GetGlobalClassReference (JNIEnv *env, const char *className) {
  jclass localRef;

  localRef = (*env)->FindClass(env, className);
  return (localRef == NULL)?NULL:(*env)->NewGlobalRef(env, localRef);
}


void fillDebugBuffer (char *fmt, ...) {
  va_list ap;

  memset(dbgMsg, 0, sizeof(dbgMsg));

  /* extract the information into the buffer */
  va_start(ap, fmt);
  vsprintf(dbgMsg, fmt, ap);
  va_end(ap);
}


/* calls the corresponding static method in JServer */
void printDebug (JNIEnv *env, jboolean newln, int level) {
  jstring str;

  str = (*env)->NewStringUTF(env, dbgMsg);
  printJserverDebug(env, newln, level, str);
  (*env)->DeleteLocalRef(env, str);
}


void printJserverDebug (JNIEnv *env, jboolean newln, int level, jstring msg) {
  (*env)->CallStaticVoidMethod(env, jserverClass, jsPrintDebugMid, msg, level, newln);
}


void printObject (JNIEnv *env, int level, jobject config) {
  jclass cls;
  jmethodID mid;
  jstring str;

  if (config == NULL) {
    LOCAL_DEBUG(JNI_TRUE, level, ("cannot print: object is NULL"));
    return;
  }

  cls = (*env)->GetObjectClass(env, config);
  if (cls == NULL)
    return;

  mid = (*env)->GetMethodID(env, cls, "toString", "()Ljava/lang/String;");
  if (mid == NULL)
    return;

  str = (jstring)(*env)->CallObjectMethod(env, config, mid);
  printJserverDebug(env, JNI_TRUE, level, str);
  (*env)->DeleteLocalRef(env, str);
}


/* check for NULL before attempting to free */
void checkFree (void *ptr) {
  if (ptr != NULL)
    free(ptr);
  ptr = NULL;
}


/* throw a new java exception
   0 when thrown successfully, -1 if not
*/
int ThrowBridgeException (JNIEnv *env, const char *msg) {

  LOCAL_DEBUG(JNI_FALSE, DEBUG_WARNING, ("Throwing new BridgeException:\n\t%s\n", msg));

  return (*env)->ThrowNew(env, bridgeExceptionClass, msg);
}


/* throw a new Exist exception
   0 when thrown successfully, -1 if not
*/
int ThrowExistException (JNIEnv *env, const char *msg) {

  LOCAL_DEBUG(JNI_FALSE, DEBUG_WARNING, ("Throwing new ExistException:\n\t%s\n", msg));

  return (*env)->ThrowNew(env, existExceptionClass, msg);
}

/* throw a new No Entry exception
   0 when thrown successfully, -1 if not
*/
int ThrowNoEntryException (JNIEnv *env, const char *msg) {

  LOCAL_DEBUG(JNI_FALSE, DEBUG_WARNING, ("Throwing new NoEntryException:\n\t%s\n", msg));

  return (*env)->ThrowNew(env, noEntryExceptionClass, msg);
}
/* throw a new java Exception
   0 when thrown successfully, -1 if not
*/
int ThrowException (JNIEnv *env, const char *msg) {

  LOCAL_DEBUG(JNI_FALSE, DEBUG_WARNING, ("Throwing new Exception:\n\t%s\n", msg));

  return (*env)->ThrowNew(env, exceptionClass, msg);
}


jstring ExtractExceptionString (JNIEnv *env, jthrowable exc) {
  jclass cls = NULL;
  jmethodID mid = NULL;

  cls = (*env)->GetObjectClass(env, exc);
  if (cls == NULL)
    return NULL;   /* class not found exception will be thrown */

  mid = (*env)->GetMethodID(env, cls, "toString", "()Ljava/lang/String;");
  if (mid == NULL)
    return NULL;  /* no such method error will be thrown */

  return (jstring)((*env)->CallObjectMethod(env, exc, mid));
}


int CheckAndThrowBridgeException (JNIEnv *env) {
  jthrowable exc;
  jstring excString;
  const char *exception;

  exc = (*env)->ExceptionOccurred(env);
  if (exc != NULL) {
    (*env)->ExceptionDescribe(env);
    (*env)->ExceptionClear(env);

    excString = ExtractExceptionString(env, exc);
    if (excString == NULL) {
      (*env)->DeleteLocalRef(env, exc);
      return -1;
    }

    exception = (*env)->GetStringUTFChars(env, excString, NULL);
    if (exception == NULL) {
      (*env)->DeleteLocalRef(env, exc);
      (*env)->DeleteLocalRef(env, excString);
      return -1;
    }

    ThrowBridgeException(env, exception);
    (*env)->ReleaseStringUTFChars(env, excString, exception);
    (*env)->DeleteLocalRef(env, excString);
    (*env)->DeleteLocalRef(env, exc);
    return -1;
  }

  return 0;
}


int JserverProcessConfig (void) {
  int match = -1;

  myConfigServerType = CONFIG_SERPLEX_JSERVER;

  /* Process Configuration, read from the config file */
  match = FindServerConfig();

  if (match == -1) {
    fprintf(stderr, "Not Configured to run jserver (servertype=%d)...\n", myConfigServerType);
    exit(0);
  }

  ServerSetLogging("jServer", &serplexes[match]);

  return 0;
}


int utilsIdInit (JNIEnv *env) {

  jserverClass = GetGlobalClassReference(env, "com/nextone/JServer/JServer");
  if (jserverClass == NULL)
    return -1;  // out of memory exception is thrown

  jsPrintDebugMid = (*env)->GetStaticMethodID(env, jserverClass, "printDebug", "(Ljava/lang/Object;IZ)V");
  if (jsPrintDebugMid == NULL)
    return -1;  // exception thrown

  bridgeExceptionClass = GetGlobalClassReference(env, "com/nextone/common/BridgeException");
  if (bridgeExceptionClass == NULL)
    return -1;

  existExceptionClass = GetGlobalClassReference(env, "com/nextone/common/ExistException");
  if (existExceptionClass == NULL)
    return -1;

  noEntryExceptionClass = GetGlobalClassReference(env, "com/nextone/common/NoEntryException");
  if (noEntryExceptionClass == NULL)
    return -1;
  exceptionClass = GetGlobalClassReference(env, "java/lang/Exception");
  if (exceptionClass == NULL)
    return -1;

  return 0;
}

