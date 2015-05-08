/***********************************************************************
 ** FUNCTION:
 **             Has All the porting layer functions

 *********************************************************************
 **
 ** FILENAME:
 ** sipfree.c
 **
 ** DESCRIPTION:
 **
 ** DATE        NAME                    REFERENCE               REASON
 ** ----        ----                    ---------              --------
 ** 6/12/99   Arjun RC       		                    Initial Creation
 **
 **
 **     Copyright 1999, Hughes Software Systems, Ltd.
 *********************************************************************/


#ifdef SIP_SOLARIS
#include <time.h>
#endif
#include <stdlib.h>
#include <stdarg.h>
#include <limits.h>
#include "sipcommon.h"
#include "portlayer.h"
#include "siptrace.h"

#ifdef SIP_THREAD_SAFE
#ifdef SIP_SOLARIS
#include <pthread.h>
#endif
#endif


#define MAX_LENGTH 4294967295u /* this is 2**32 -1 */
#include <errno.h>
/*
	These are retransmission values
	These values are initialised in sip_initStack() function in sipdecode.c
	You change change the values at runtime by directly modifying below variables

	These values are in milli seconds

*/

#ifdef DYNAMIC_MEMORY_CALC
   /* 
    * These two arrays are used for
    * calculating dynamic memory allocation for code size document.
    * glb_memory_alloc_dist contains memory allocation distribution.
    * glb_memory_alloc_dist[1] = memory_alloc between    0 to   10 bytes
    * glb_memory_alloc_dist[2] = memory_alloc between   10 to  100 bytes
    * glb_memory_alloc_dist[3] = memory_alloc between  100 to 1000 bytes
    * glb_memory_alloc_dist[4] = memory_alloc above   1000 bytes
    * 
    * glb_memory_alloc_parser contains the memory allocation for parser
    * modules.
    * glb_memory_alloc_parser[0] = allocation done for decode module.
    * glb_memory_alloc_parser[1] = allocation done for bison module.
    * glb_memory_alloc_parser[2] = allocation done for flex module.
    *
    */

    SIP_U32bit glb_memory_alloc_dist[5] ;
    SIP_U32bit glb_memory_alloc_parser[3] ;
#endif

SIP_U32bit	SIP_T1;				/* T1 retrans value */
SIP_U32bit	SIP_T2;				/* Cap off value */

SIP_U32bit	SIP_MAXRETRANS;		
/* No. of times non INVITE messages will be retransmitted */


SIP_U32bit  SIP_MAXINVRETRANS;  
/* No. of times INVITE messages will be retransmitted */


/* 
 * In VxWorks the stdout descriptor is not accessed in a thread safe manner
 * Hence if multiplethreads try to write using this variable, VxWorks
 * itself does not provide any safety, hence the application itself needs
 * to do writes in a thread safe mechanism. For this reason the stack uses
 * a mutex to ensure all prints/writes are done in a thread safe manner
 */

#ifdef SIP_THREAD_SAFE
#endif

/*****************************************************************
** FUNCTION:sip_releaseStackPortSpecific
**
**
** DESCRIPTION: This functions handles all the port specific handling 
**              while the sip_releaseStack API is called.
*******************************************************************/
void sip_releaseStackPortSpecific(void)
{
	int dummy;
	(void)dummy;
#ifdef SIP_THREAD_SAFE
#endif	
}

/*****************************************************************
** FUNCTION:sip_initStackPortSpecific
**
**
** DESCRIPTION: This functions handles all the port specific handling 
**              while the sip_initStack API is called.
*******************************************************************/
void sip_initStackPortSpecific(void)
{
	int dummy;
	(void)dummy;
#ifdef SIP_THREAD_SAFE
#endif	
}


#ifdef SIP_THREAD_SAFE
/*****************************************************************
** FUNCTION:fast_lock_synch
**
**
** DESCRIPTION:
*******************************************************************/
void fast_lock_synch
#ifdef ANSI_PROTO
	(thread_id_t tid, synch_id_t *mutex, SIP_U32bit flags)
#else
	(tid, mutex, flags)
	thread_id_t tid;
	synch_id_t *mutex;
	SIP_U32bit flags;
#endif
{
	thread_id_t dummy_tid;
	SIP_U32bit dummy_flags;
	dummy_tid = tid;
	dummy_flags =flags;

/* Port according to system and library selection */
#ifdef SIP_SOLARIS
	pthread_mutex_lock(mutex);
#endif
}
/*****************************************************************
** FUNCTION:fast_trylock_synch
**
**
** DESCRIPTION:
*******************************************************************/
SipBool fast_trylock_synch
#ifdef ANSI_PROTO
	(thread_id_t tid, synch_id_t *mutex, SIP_U32bit flags)
#else
	(tid, mutex, flags)
	thread_id_t tid;
	synch_id_t *mutex;
	SIP_U32bit flags;
#endif
{
	thread_id_t dummy_tid;
	SIP_U32bit dummy_flags;
	dummy_tid = tid;
	dummy_flags =flags;

/* Port according to system and library selection */
	sip_trace(SIP_Detailed,SIP_Init,
					(SIP_S8bit *)"STACK TRACE: Trying to Grab Mutex Lock");

#ifdef SIP_SOLARIS
	if ( pthread_mutex_trylock(mutex) != 0 )
		return SipFail;
#endif

sip_trace(SIP_Detailed,SIP_Init,
					(SIP_S8bit *)"STACK TRACE: Successfully Grabbed Mutex Lock");
	return SipSuccess;
}

/*****************************************************************
** FUNCTION:fast_unlock_synch
**
**
** DESCRIPTION:
*******************************************************************/
void fast_unlock_synch
#ifdef ANSI_PROTO
	(thread_id_t tid, synch_id_t *mutex)
#else
	(tid, mutex)
	thread_id_t tid;
	synch_id_t *mutex;
#endif
{
	thread_id_t dummy_tid;
	dummy_tid = tid;
/* Port according to system and library selection */
#ifdef SIP_SOLARIS
	pthread_mutex_unlock(mutex);
#endif
}

/*****************************************************************
** FUNCTION:fast_init_synch
**
**
** DESCRIPTION:
*******************************************************************/
void fast_init_synch
#ifdef ANSI_PROTO
	(synch_id_t *mutex)
#else
	( mutex)
	synch_id_t *mutex;
#endif
{
/* Port according to system and library selection */
	/* removed PTHREAD_PROCESS_PRIVATE from the next statement & made null*/
#ifdef SIP_SOLARIS
	pthread_mutex_init(mutex,NULL);
#endif
	sip_trace(SIP_Detailed,SIP_Init,
					(SIP_S8bit *)"STACK TRACE: Initialised Mutex Lock");
}
/*****************************************************************
** FUNCTION:fast_free_synch
**
**
** DESCRIPTION:
*******************************************************************/
void fast_free_synch
#ifdef ANSI_PROTO
	(synch_id_t *mutex)
#else
	( mutex)
	synch_id_t *mutex;
#endif
{
/* Port according to system and library selection */
	/* removed PTHREAD_PROCESS_PRIVATE from the next statement & made null*/
#ifdef SIP_SOLARIS
	pthread_mutex_destroy(mutex);
#endif
	sip_trace(SIP_Detailed,SIP_Init,
					(SIP_S8bit *)"STACK TRACE: Destroy Mutex Lock");
}


/* The following functions need to be ported for the message based build
   only. Normal stack builds need not implement these functions */

/*endif of #ifdef SIP_THREAD_SAFE*/
#endif


/*****************************************************************
* FUNCTION NAME :fast_memrealloc
*
*
* DESCRIPTION:
*******************************************************************/

SIP_Pvoid fast_memrealloc
#ifdef ANSI_PROTO
(SIP_U32bit module_id, SIP_Pvoid pBuffer, SIP_U32bit noctets, SipError *err)
#else
	(moduleId, pBuffer, noctets, err)
	SIP_U32bit module_id;
    SIP_Pvoid pBuffer;
	SIP_U32bit noctets;
	SipError *err;
#endif
{
    SIP_Pvoid pvoid;
    SIP_U32bit dummy;
    dummy = module_id;

#ifdef DYNAMIC_MEMORY_CALC_BUGGY
    /* This calculation might be wrong we need to test and 
      fix this.
      However this is for internal purposes and should not
      affect the working of the stack.
     */
    SIP_U32bit initSize = 0;
    initSize = strlen(pBuffer);
#endif

    pvoid = (SIP_Pvoid)realloc(pBuffer, noctets);

    if(pvoid == SIP_NULL)
    {
        if (err!=SIP_NULL) 
        {
            *err = E_NO_MEM;
        }
        return SIP_NULL;
    }

    if (err!=SIP_NULL) 
    {
        *err = E_NO_ERROR;
    }

#ifdef DYNAMIC_MEMORY_CAL_BUGGY
  {
      int indx=0 ;
      int flag=0 ;

      /*
       * If the allocation is done for parser modules
       * then add the memory allocated to the count
       */

      if ( module_id == DECODE_MEM_ID )
      {
          glb_memory_alloc_parser[0] -= initSize ;
          glb_memory_alloc_parser[0] += noctets ;
      }
      else if ( module_id == BISON_MEM_ID )
      {
          glb_memory_alloc_parser[1] -= initSize ;
          glb_memory_alloc_parser[1] += noctets ;
      }
      else if ( module_id == FLEX_MEM_ID )
      {
          glb_memory_alloc_parser[1] -= initSize ;
          glb_memory_alloc_parser[2] += noctets ;
      }
      else
      {
          flag = 1 ;
      }

      /*
       * If allocation is done for parser module
       * then count the memory allocation
       * pattern
       */
      if ( flag == 0 )
      {
          if ( (0 < noctets) && (noctets  <= 10) )
              indx = 1 ;
          else  if ( (10 < noctets  ) && ( noctets <= 100 ) )
              indx = 2 ;
          else  if ( (100 < noctets ) && (noctets <= 1000 ) )
              indx = 3 ;
          else
              indx = 4 ;
          glb_memory_alloc_dist[indx]++ ;
      }
  }
#endif
  return pvoid;
}

/*****************************************************************
* FUNCTION NAME :fast_memget
*
*
* DESCRIPTION:
*******************************************************************/

SIP_Pvoid fast_memget
#ifdef ANSI_PROTO
	( SIP_U32bit module_id, SIP_U32bit noctets, SipError *err)
#else
	(module_id, noctets, err)
	SIP_U32bit module_id;
	SIP_U32bit noctets;
	SipError *err;
#endif
{
        SIP_Pvoid pvoid;
		SIP_U32bit dummy;
		dummy = module_id;

        pvoid = (SIP_Pvoid)malloc(noctets);
        if(pvoid == SIP_NULL)
        {
		if (err!=SIP_NULL) *err = E_NO_MEM;
                return SIP_NULL;
        }

	/* NEXTONE - Added memset */
	memset(pvoid, 0, noctets);

	if (err!=SIP_NULL) *err = E_NO_ERROR;
#ifdef DYNAMIC_MEMORY_CALC
  {
      int indx=0 ;
      int flag=0 ;

      /*
       * If the allocation is done for parser modules
       * then add the memory allocated to the count
       */

      if ( module_id == DECODE_MEM_ID )
          glb_memory_alloc_parser[0] += noctets ;
      else if ( module_id == BISON_MEM_ID )
          glb_memory_alloc_parser[1] += noctets ;
      else if ( module_id == FLEX_MEM_ID )
          glb_memory_alloc_parser[2] += noctets ;
      else
          flag = 1 ;

      /*
       * If allocation is done for parser module
       * then count the memory allocation
       * pattern
       */
      if ( flag == 0 )
      {
          if ( (0 < noctets) && (noctets  <= 10) )
              indx = 1 ;
          else  if ( (10 < noctets  ) && ( noctets <= 100 ) )
              indx = 2 ;
          else  if ( (100 < noctets ) && (noctets <= 1000 ) )
              indx = 3 ;
          else
              indx = 4 ;
          glb_memory_alloc_dist[indx]++ ;
      }
  }
#endif
  return pvoid;
}

/*****************************************************************
** FUNCTION:fast_memfree
**
**
** DESCRIPTION:
*******************************************************************/

SipBool fast_memfree
#ifdef ANSI_PROTO
	( SIP_U32bit module_id, SIP_Pvoid p_buf , SipError *err)
#else
	(module_id, p_buf, err)
	SIP_U32bit module_id;
	SIP_Pvoid p_buf;
	SipError *err;
#endif
{
	SIP_U32bit dummy;
	dummy = module_id;
	if(p_buf != SIP_NULL)
		free(p_buf);

	if (err != SIP_NULL)
		*err = E_NO_ERROR;
        return SipSuccess;
}

/*****************************************************************
** FUNCTION:sip_memget
**
**
** DESCRIPTION: Same as fast_memget - defined for convenience
** Allows pUser to pass SIP_NULL in SipError* param
*******************************************************************/

SIP_Pvoid sip_memget
#ifdef ANSI_PROTO
	( SIP_U32bit module_id, SIP_U32bit noctets, SipError *err)
#else
	(module_id, noctets, err)
	SIP_U32bit module_id;
	SIP_U32bit noctets;
	SipError *err;
#endif
{
	return ( fast_memget(module_id, noctets, err));
}

/*****************************************************************
** FUNCTION:sip_memfree
**
**
** DESCRIPTION: frees memory and SIP_NULLifies pointer. Accepts pointer
** to pointer to area
** Allows pUser to pass SIP_NULL in SipError* param
*******************************************************************/

SipBool sip_memfree
#ifdef ANSI_PROTO
	( SIP_U32bit module_id, SIP_Pvoid* p_buf , SipError *err)
#else
	(module_id, p_buf, err)
	SIP_U32bit module_id;
	SIP_Pvoid* p_buf;
	SipError *err;
#endif
{
	SipBool x=SipSuccess;

	if (*p_buf!=SIP_NULL)
		x = fast_memfree(module_id, *p_buf, err);

	*p_buf=SIP_NULL;
	return x;

}

#ifdef SIP_OVERRIDE_SNPRINTF
int  snprintf(char *s , size_t n, const char * fmt, ...)
{

	va_list  ap;
	size_t dummy;
	dummy = n;
	va_start(ap,fmt);
	vsprintf(s,fmt,ap);
	va_end(ap);
	return 1; /* Should realy be returning the number of bytes formatted */
}

#endif

/*****************************************************************
** FUNCTION:STRTOU32LCAP
**
**
** DESCRIPTION: converts string to unsigned long.
** caps off pValue to 2^32-1 if larger pValue is given as input.
**
*******************************************************************/

SIP_U32bit STRTOU32CAP
#ifdef ANSI_PROTO
	( SIP_S8bit *str, SipError *pErr )
#else
	(str, pErr)
	SIP_S8bit *str;
	SipError *pErr;
#endif
{
#if defined(SIP_SOLARIS) || defined(SIP_LINUX) || defined(SIP_WINDOWS) || defined(SIP_VXWORKS) || defined(SIP_OSE)
	unsigned long result;
	errno=0;
	result= strtoul(str,NULL,SIP_BASE_10);
	if(errno == ERANGE)
	{
		if(pErr != SIP_NULL)
			*pErr = E_INV_RANGE;
		return MAX_LENGTH;
	}
	else
	{
		if(pErr != SIP_NULL)
			*pErr = E_NO_ERROR;
		return result;
	}
#endif
}


void sip_getTimeString
#ifdef ANSI_PROTO
	(SIP_S8bit **pString)
#else
	(pString)
	SIP_S8bit **pString;
#endif
{
#ifdef SIP_SOLARIS
	struct timeval tv;
	char *pTime;
	char buffer[SIP_MAX_CTIME_R_BUF_SIZE];
	gettimeofday(&tv,SIP_NULL);
#if (_POSIX_C_SOURCE - 0 >= 199506L) 
	pTime = ctime_r(&tv.tv_sec,buffer);
	strncpy(*pString,pTime,SIP_MAX_CTIME_R_BUF_SIZE);
#else
	pTime = ctime_r(&tv.tv_sec,buffer,SIP_MAX_CTIME_R_BUF_SIZE);
	strncpy(*pString,pTime,SIP_MAX_CTIME_R_BUF_SIZE);
#endif
#endif

}

/* These are required by Vxworks Port */



/*****************************************************************
** FUNCTION:sip_strdup
**
**
** DESCRIPTION:
**      strdup function that calls fast_memget followed by strcpy
**
*******************************************************************/
char*  sip_strdup
#ifdef ANSI_PROTO
        (const char *pOriginalString, SIP_U32bit dMemId)
#else
        (pOriginalString, dMemId)
        const char *pOriginalString;
		SIP_U32bit dMemId;
#endif
{
	char *pRetVal=SIP_NULL;
	SIP_U32bit dSize;

	if (pOriginalString == SIP_NULL)
		return SIP_NULL;

	dSize = strlen(pOriginalString);
	pRetVal =(char *) fast_memget(dMemId, (dSize+1), SIP_NULL);

	if (pRetVal == SIP_NULL)
		return SIP_NULL;
	strcpy(pRetVal, pOriginalString);
	return pRetVal;
}

/*****************************************************************
** FUNCTION:sip_strstr
**
**
** DESCRIPTION:
**      strstr wrapper function
**
*******************************************************************/
SIP_S8bit * sip_strstr
#ifdef ANSI_PROTO
	(SIP_S8bit *pOriginalString,SIP_S8bit *pStringToSearch)
#else
	(pOriginalString,pStringToSearch)
	SIP_S8bit	*pOriginalString;
	SIP_S8bit	*pStringToSearch;
#endif
{
	SIP_S8bit *pReturnValue;

	pReturnValue=strstr(pOriginalString,pStringToSearch);
	return(pReturnValue);
}


#ifdef SIP_NO_FILESYS_DEP
/* The following are the functions that will have to be implemented
	by the application. SIP_NON_INTERACTIVE lex does not use
	stdio.h. So all the releavant functions from stdio.h will have
	to be redined to the equivalent ones.
*/
/*****************************************************************
** FUNCTION:sip_printfunc
**
**
** DESCRIPTION:
**      printf equivalent function..to be implemented during porting
**      for SIP_NO_FILESYS_DEP
*******************************************************************/
void sip_printfunc(const char *format,...)
{
	(void)format;
}

/*****************************************************************
** FUNCTION:sip_sprintf
**
**
** DESCRIPTION:
**      sprintf equivalent function..to be implemented during porting
**      for SIP_NO_FILESYS_DEP
*******************************************************************/
void sip_sprintf(char *s,const char *format,...)
{
	(void)s;
	(void)format;
}


/*****************************************************************
** FUNCTION:sip_snprintf
**
**
** DESCRIPTION:
**      snprintf equivalent function..to be implemented during porting
**      for SIP_NO_FILESYS_DEP
*******************************************************************/
void sip_snprintf(char *s, unsigned int n,const char *format,...)
{
	(void)s;
	(void)n;
	(void)format;
}

/*****************************************************************
** FUNCTION:sip_fprintf
**
**
** DESCRIPTION:
**      fprintf equivalent function..to be implemented during porting
**      for SIP_NO_FILESYS_DEP
*******************************************************************/
void sip_fprintf(FILE *stream, const char *format,...)
{
	(void)stream;
	(void)format;
}

/*****************************************************************
** FUNCTION:sip_getc
**
**
** DESCRIPTION:
**      getc equivalent function..to be implemented during porting
**      for SIP_NO_FILESYS_DEP
*******************************************************************/
int sip_getc(FILE *stream)
{
	(void) stream;
	return 0;
}

/*****************************************************************
** FUNCTION:sip_ferror
**
**
** DESCRIPTION:
**      ferror equivalent function..to be implemented during porting
**      for SIP_NO_FILESYS_DEP
*******************************************************************/
int sip_ferror(FILE *stream)
{
	(void)stream;
	return 0;
}

/*****************************************************************
** FUNCTION:sip_fread
**
**
** DESCRIPTION:
**      fread equivalent function..to be implemented during porting
**      for SIP_NO_FILESYS_DEP
*******************************************************************/
size_t sip_fread(void *ptr,size_t size,size_t nitems,FILE *stream)
{
	(void)ptr;
	(void)size;
	(void)nitems;
	(void)stream;
	return 0;
}

#endif

