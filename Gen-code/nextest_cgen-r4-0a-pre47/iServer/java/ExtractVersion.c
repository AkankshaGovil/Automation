/*
 * extracts the version number information from the C header file
 * and gives it to the java code
 */
#include <jni.h>
#include <stdio.h>
#include "ExtractVersion.h"
#include "spversion.h"
#include "pmpoll.h"

/* return build date string */
JNIEXPORT jstring JNICALL Java_com_nextone_JServer_JServerMain_getBuildDate (JNIEnv *env, jclass cls) {
  return (*env)->NewStringUTF(env, JSERVER_BUILDDATE);
}

/* return copyright string */
JNIEXPORT jstring JNICALL Java_com_nextone_JServer_JServerMain_getCopyright (JNIEnv *env, jclass cls) {
  return (*env)->NewStringUTF(env, JSERVER_COPYRIGHT);
}

/* return major version string */
JNIEXPORT jstring JNICALL Java_com_nextone_JServer_JServerMain_getMajorVersion (JNIEnv *env, jclass cls) {
  return (*env)->NewStringUTF(env, JSERVER_VERSION);
}

/* return minor version string */
JNIEXPORT jstring JNICALL Java_com_nextone_JServer_JServerMain_getMinorVersion (JNIEnv *env, jclass cls) {
  return (*env)->NewStringUTF(env, JSERVER_MINOR);
}

/* return server name string */
JNIEXPORT jstring JNICALL Java_com_nextone_JServer_JServerMain_getName (JNIEnv *env, jclass cls) {
  return (*env)->NewStringUTF(env, JSERVER_NAME);
}

/* return release name string */
JNIEXPORT jstring JNICALL Java_com_nextone_JServer_JServerMain_getReleaseName (JNIEnv *env, jclass cls) {
  return (*env)->NewStringUTF(env, JSERVER_RELNAME);
}

JNIEXPORT jint JNICALL Java_com_nextone_JServer_JServerMain_getUptimeServerPort (JNIEnv *env, jclass cls) {
  return SERV_PORT;
}

JNIEXPORT jint JNICALL Java_com_nextone_JServer_JServerMain_getUptimePacketSize (JNIEnv *env, jclass cls) {
  return sizeof(KeepAlive);
}

