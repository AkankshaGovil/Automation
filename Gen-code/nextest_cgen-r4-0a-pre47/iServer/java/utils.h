#ifndef __UTILS_H
#define __UTILS_H

#include <jni.h>


#define DEBUG_OFF     0
#define DEBUG_ERROR   1
#define DEBUG_NORMAL  2
#define DEBUG_WARNING 3
#define DEBUG_VERBOSE 4

#define LOCAL_DEBUG(newln, lvl, msg)  fillDebugBuffer msg; \
                                      printDebug(env, newln, lvl)



/* prototypes */
extern void fillDebugBuffer (char*, ...);
extern void printDebug (JNIEnv*, jboolean, int);
extern void printJserverDebug (JNIEnv*, jboolean, int, jstring);
extern void printObject (JNIEnv*, int, jobject);
extern void checkFree (void*);
extern int ThrowBridgeException (JNIEnv*, const char*);
extern int ThrowExistException (JNIEnv*, const char*);
extern int ThrowNoEntryException (JNIEnv*, const char*);
extern int ThrowException (JNIEnv*, const char*);
extern jstring ExtractExceptionString (JNIEnv*, jthrowable);
extern int CheckAndThrowBridgeException (JNIEnv*);
extern int JserverProcessConfig (void);
extern jclass GetGlobalClassReference (JNIEnv*, const char*);

#endif
