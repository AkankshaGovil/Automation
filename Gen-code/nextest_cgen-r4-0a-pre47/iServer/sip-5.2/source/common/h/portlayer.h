/***********************************************************************
 ** FUNCTION:
 **             Port Layer code - has all memory related and other
 **		common utils
 *********************************************************************
 **
 ** FILENAME:
 ** sipfree.c
 **
 ** DESCRIPTION:
 ** This file contains code to free all structures
 **
 ** DATE       NAME                  REFERENCE               REASON
 ** ----       ----                  ---------              --------
 ** 8/12/99    Arjun RC       		                    Initial Creation
 **
 **     Copyright 1999, Hughes Software Systems, Ltd.
 *********************************************************************/

/* NEXTONE -- added dbmalloc */

#ifndef __PORTLAYER_H_
#define __PORTLAYER_H_


#include "sipcommon.h"
#include <time.h>


#ifndef SIP_NO_FILESYS_DEP
#include <stdio.h>
#endif
#include <stdarg.h>
#include <stdlib.h>
#ifdef SIP_SOLARIS
#include <strings.h>
#else
#include <string.h>
#endif


#undef _POSIX_PTHREAD_SEMANTICS

#ifdef SIP_THREAD_SAFE
#ifdef SIP_SOLARIS
#include "pthread.h"
#endif
#endif


#ifdef SIP_SOLARIS
#include <netinet/in.h>
#endif




/* Ensure Names are not mangled by C++ compilers */
#ifdef __cplusplus
extern "C" {
#endif



/* 
 * In VxWorks the stdout descriptor is not accessed in a thread safe manner
 * Hence if multiplethreads try to write using this variable, VxWorks
 * itself does not provide any safety, hence the application itself needs
 * to do writes in a thread safe mechanism. For this reason the stack uses
 * a mutex to ensure all prints/writes are done in a thread safe manner
 */


/*
 * The 2 below prototypes are functions that are called from within the
 * sip_initStack and sip_releaseStack APIs. These functions handle all port
  specific handling involved in the release and initing of the Stack
 */
extern void sip_initStackPortSpecific(void);
extern void sip_releaseStackPortSpecific(void);

#if defined(SIP_VXWORKS) || defined(SIP_OSE)
#define NUM_BUCKET  100
#else
#define NUM_BUCKET	1000
#endif

/* ===============================================================
   These are  memory allocation related identifiers.
   The stack uses these IDs when allocating or deallocating
   memory. Each ID is used by each component. This means that
   if the developer wants to allocate different memory pools
   for each component, he may do so by overriding the fast_memget
   and fast_memfree APIs and recognizing stack components
   by the module id

   To override, you also need to override the STRDUP macros
   defines below for each module.
============================================================== */

/* These are memory IDS that will be used for fast_memget */
#define DECODE_MEM_ID		0
#define FLEX_MEM_ID			1
#define BISON_MEM_ID		2
#define FORM_MEM_ID			3
#define TIMER_MEM_ID		4
#define	NETWORK_MEM_ID		5
#define ACCESSOR_MEM_ID		6
 /*
  * new mem id used in place of '0' in fast_memget 
  *and fast_memfree functions
  */
#define NON_SPECIFIC_MEM_ID 7

/* INIT and FREE IDs are used by sipinit.c & sipfree.c
  since there is no way of guessing in which context
  these free/inits were called, they have a different
  memory id */

#define	INIT_MEM_ID			7
#define FREE_MEM_ID			8
/* user apps may want to use this or any other value > 9 */
#define APP_MEM_ID			9

/*
 * Buffer size to be used in ctime function
 */
#define SIP_MAX_CTIME_R_BUF_SIZE 26

/* NEXTONE -- dbmalloc changes */
#ifdef _DBMALLOC_
#define fast_memget(_x_, _y_, _z_)	(calloc(_y_, 1))
#define fast_memfree(_x_, _y_, _z_) (free(_y_), SipSuccess)
#endif

/* Easy macros which will be used in generated flex and Yacc files to replace mallocs and alloca s
   The user should not change these macros
*/
#define FLEXMALLOC(x)		fast_memget(FLEX_MEM_ID, x, SIP_NULL)
#define FLEXREALLOC(x,n)	fast_memrealloc(FLEX_MEM_ID, x, n, SIP_NULL)
#define FLEXFREE(x)			fast_memfree(FLEX_MEM_ID, x, SIP_NULL)
#define FLEXSTRDUP(x)       sip_strdup(x, FLEX_MEM_ID)
#define BISONMALLOC(x)		fast_memget(BISON_MEM_ID, x, SIP_NULL)
#define BISONREALLOC(x,n)	fast_memrealloc(BISON_MEM_ID, x, n, SIP_NULL)
#define BISONFREE(x)		fast_memfree(BISON_MEM_ID, x, SIP_NULL)
#define BISONSTRDUP(x)      sip_strdup(x, BISON_MEM_ID)


/*****************************************************************
** FUNCTION:sip_strdup
**
**
** DESCRIPTION:
**      strdup function that calls fast_memget followed by strcpy
**
*******************************************************************/
extern char*  sip_strdup _ARGS_((const char *pOriginalString, \
	SIP_U32bit dMemId));


/*****************************************************************
** FUNCTION:sip_strstr
**
**
** DESCRIPTION:
**      strstr wrapper function
**
*******************************************************************/
extern SIP_S8bit*  sip_strstr _ARGS_((SIP_S8bit *pOriginalString, \
	SIP_S8bit *pStringToSearch));

/* These macros define STRDUP for all components. If you want them to be allocated
 	to different pools, redefine these macros */

#define STRDUP(x) sip_strdup(x, 0)
#define STRDUPTIMER(x) sip_strdup(x, TIMER_MEM_ID)
#define STRDUPDECODE(x) sip_strdup(x, DECODE_MEM_ID)
#define STRDUPLEX(x) sip_strdup(x, FLEX_MEM_ID)
#define STRDUPBISON(x) sip_strdup(x, BISON_MEM_ID)
#define STRDUPNETWORK(x) sip_strdup(x, NETWORK_MEM_ID)
#define STRDUPACCESSOR(x) sip_strdup(x, ACCESSOR_MEM_ID)

#define SIP_RETURN_IMMEDIATELY	0


#define STRCAT(a,b) \
do \
{ \
	if ((b!=SIP_NULL)) strcat (((char *)a),((char *)b));\
}\
while(0)

#define STRCPY(a,b) \
do \
{ \
	if ((b!=SIP_NULL)) strcpy (((char *)a),((char *)b));\
}\
while(0)

#define INIT(x) \
do \
{ \
	x=SIP_NULL;\
} \
while(0)

#define HSS_FREE(x) \
do \
{ \
	if ((x!=SIP_NULL)) fast_memfree(FREE_MEM_ID,x,SIP_NULL);\
} \
while(0)

#ifdef  SIP_DEBUG

#ifdef SIP_SOLARIS
#define SIPDEBUG(a)  printf ("SIP_DEBUG:%s\n", a)
#endif



#else
#define SIPDEBUG(a)
#endif

#ifdef  SIP_FNDEBUG
#ifdef SIP_SOLARIS
#include <stdio.h>
#define SIPDEBUGFN(a) printf ("FUNCTION SIP_DEBUG:%s\n", a)
#endif
#else
#define SIPDEBUGFN(a)
#endif


#ifdef SIP_THREAD_SAFE
#endif


#ifdef SIP_THREAD_SAFE



#ifdef SIP_SOLARIS
typedef pthread_t thread_id_t;
typedef pthread_mutex_t synch_id_t; /* port this to mutex library variable */
typedef pthread_cond_t	thread_cond_t;
typedef pthread_attr_t	thread_attr_t;
typedef void * (*fpThreadStartFunc)(void *args);
#endif



#endif

#define FAST_MXLOCK_SHARED (0)
#define FAST_MXLOCK_EXCLUSIVE (1)

extern SIP_Pvoid fast_memget _ARGS_((SIP_U32bit module_id, SIP_U32bit noctets, SipError *err));
extern SIP_Pvoid fast_memrealloc _ARGS_((SIP_U32bit module_id, SIP_Pvoid pBuffer, SIP_U32bit noctets, SipError *err));
extern SIP_Pvoid sip_memget _ARGS_((SIP_U32bit module_id, SIP_U32bit noctets, SipError *err));

extern SipBool fast_memfree _ARGS_((SIP_U32bit module_id, SIP_Pvoid p_buf , SipError *err));
extern SipBool sip_memfree _ARGS_((SIP_U32bit module_id, SIP_Pvoid* p_buf , SipError *err));


#ifdef SIP_THREAD_SAFE
extern SipBool fast_trylock_synch _ARGS_((thread_id_t tid,\
							synch_id_t *mutex, SIP_U32bit flags));
extern void fast_lock_synch _ARGS_((thread_id_t tid,\
							synch_id_t *mutex, SIP_U32bit flags));
extern void fast_unlock_synch _ARGS_((thread_id_t tid,\
							synch_id_t *mutex));
extern void fast_init_synch _ARGS_((synch_id_t *mutex));
extern void fast_free_synch _ARGS_((synch_id_t *mutex));
#endif

#ifdef SIP_OVERRIDE_SNPRINTF
extern int  snprintf(char *, size_t, const char *, ... );
#endif
/* Changed STRTOLCAP to STRTOU32CAP. */
extern SIP_U32bit STRTOU32CAP _ARGS_(( SIP_S8bit *str , SipError* pErr));


void sip_getTimeString _ARGS_((SIP_S8bit **pString));

/* SIP retransmission values variables */
extern SIP_U32bit SIP_T1;
extern SIP_U32bit SIP_T2;
extern SIP_U32bit SIP_MAXRETRANS;
extern SIP_U32bit SIP_MAXINVRETRANS;


#ifdef SIP_NO_FILESYS_DEP

/*================================================================
In this section some data-types, macros and functions have been
redefined. These are required mainly for the flex generated code.
If your system defines any of these types, you may remove these
re-definitions.

Setting the FILE type and the file descriptor macros to dummy
values does not affect flex code since the stack does not use
portions that read input from files.
==================================================================*/
struct sip_dummy_file {
	char  dummy;
};
#define FILE struct sip_dummy_file

#define YYDEBUG 0
#define ECHO
#define YY_NEVER_INTERACTIVE (1)
#define YY_INPUT(a,b,c)


#ifdef stdin
#undef stdin
#endif
#define stdin  (( FILE *)1)

#ifdef stdout
#undef stdout
#endif
#define stdout  ((FILE*) 1)

#ifdef stderr
#undef stderr
#endif
#define stderr  ((FILE*) 1)

#ifdef EOF
#undef EOF
#endif
#define EOF  -1

/*================================================================

REDEFINE THESE MACROS OR IMPLEMENT THE ABOVE FUNCS
The following macros should be made to point to an implementation:
HSS_SPRINTF should be defined to an equivalent implementation of
sprintf
HSS_SNPRINTF should be defined to an equivalent implementation of
snprintf

HSS_FPRINTF, HSS_FERROR, HSS_FREAD and HSS_GETC are optional. These may be
defined to dummy functions. Though these functions are present in the flex
generated code, they are used only to report errors or to read
buffers from files (the lexers however will not be used in the mode
where they have to read data from files.)

==================================================================*/
#define HSS_SPRINTF sip_sprintf
#define HSS_SNPRINTF sip_snprintf
#define HSS_FPRINTF sip_fprintf
#define HSS_FERROR sip_ferror
#define HSS_FREAD sip_fread
#define HSS_GETC sip_getc

/*================================================================
The following have been defined to check compilation.
They should be removed after the macros above have been defined
to the right functions.
==================================================================*/
extern void sip_printfunc _ARGS_((const char *format,...));
extern void sip_sprintf _ARGS_((char *s,const char *format,...));
extern void sip_snprintf _ARGS_((char *s, unsigned int n,const \
	char *format,...));
extern void sip_fprintf _ARGS_((FILE *stream, const char *format,...));
extern int sip_getc _ARGS_((FILE *stream));
extern int sip_ferror _ARGS_((FILE *stream));
extern size_t sip_fread _ARGS_((void *ptr,size_t size,size_t nitems,FILE *stream));

#else

#define HSS_SPRINTF sprintf
#define HSS_SNPRINTF snprintf
#define HSS_FPRINTF fprintf
#define HSS_FERROR ferror
#define HSS_FREAD fread
#define HSS_GETC getc

#endif



#ifdef SIP_LOCKEDREFCOUNT

#define HSS_INITREF(x) do { fast_init_synch(&(x.lock)); x.ref=1;} while (0)
#define HSS_LOCKREF(x) do { fast_lock_synch(0,&(x.lock),0);} while (0)
#define HSS_UNLOCKREF(x) do { fast_unlock_synch(0,&(x.lock));} while (0)
#define HSS_INCREF(x) do { x.ref++;} while (0)
#define HSS_DECREF(x) do { x.ref--;} while (0)
#define HSS_CHECKREF(x) (x.ref==0)
#define HSS_LOCKEDINCREF(x) do { \
	fast_lock_synch(0,&(x.lock),0); \
	x.ref++;\
	fast_unlock_synch(0,&(x.lock)); } while (0)
#define HSS_LOCKEDDECREF(x) do { \
	fast_lock_synch(0,&(x.lock),0); \
	x.ref--;\
	fast_unlock_synch(0,&(x.lock)); } while (0)
#define HSS_DELETEREF(x) do { fast_free_synch(&(x.lock));} while (0)

#else

#define HSS_INITREF(x) x=1
#define HSS_LOCKREF(x)
#define HSS_UNLOCKREF(x)
#define HSS_INCREF(x) x++
#define HSS_DECREF(x) x--
#define HSS_CHECKREF(x) x==0
#define HSS_LOCKEDINCREF(x) x++
#define HSS_LOCKEDDECREF(x) x--
#define HSS_DELETEREF(x)

#endif




extern void sip_initStackPortSpecific(void);
extern void sip_releaseStackPortSpecific(void);

/* NEXTONE -- dbmalloc changes */
#ifdef _DBMALLOC_
#include <malloc.h>
#endif

/* Ensure Names are not mangled by C++ compilers */
#ifdef __cplusplus
}
#endif

#endif
