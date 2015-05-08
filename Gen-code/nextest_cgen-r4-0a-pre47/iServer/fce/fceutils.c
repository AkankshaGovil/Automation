/**
 * this file contains some utility methods
 */

#include <stdio.h>
#include <pthread.h>
#include <errno.h>
#include <string.h>

#include "srvrlog.h"


/**
 * locks the mutex, prints any error if happens
 *
 * @return 0 if locking was successfull, -1 otherwise
 */
int lockMutex (pthread_mutex_t *mutex)
{
   int errno;
   char *errmsg;

   if ((errno = pthread_mutex_lock(mutex)) != 0 )
   {
      errmsg = strerror(errno);
      if ( errmsg )
      {
	  	NETERROR(MFCE, ("Error locking the mutex [%d]: %s\n", errno,
                errmsg ));
      }
      else
      {
	  	NETERROR(MFCE, ("Error locking the mutex [%d]\n", errno));
      }

	  return -1;
   }

   return 0;
}

/**
 * unlocks the mutex, prints any error if happens
 *
 * @return 0 if unlocking was successfull, -1 otherwise
 */
int unlockMutex (pthread_mutex_t *mutex)
{
   int errno;
   char *errmsg;

   if ((errno = pthread_mutex_unlock(mutex)) != 0)
   {
      errmsg = strerror(errno);

      if ( errmsg )
      {
	  	NETERROR(MFCE, ("Error unlocking the mutex [%d]: %s\n", errno,
                errmsg ));
      }
      else
      {
	  	NETERROR(MFCE, ("Error unlocking the mutex [%d]\n", errno));
      }

	  return -1;
   }

   return 0;
}

/**
 * initializes mutex and stores in the given pointer
 *
 * @return 0 if init is successfull, -1 otherwise
 */ 
int initMutex (pthread_mutex_t *mutex, pthread_mutexattr_t *mutexAttr)
{
   char* errmsg;
   int   errno;

   /* initialize the mutex attribute */
   if (mutexAttr != NULL)
   {
	  if ((errno = pthread_mutexattr_init(mutexAttr)) != 0)
	  {
         errmsg = strerror( errno );

         if ( errmsg )
         {
		 	NETERROR(MFCE, ("Error initializing mutex attr [%d]: %s\n", errno,
                    errmsg ));
         }
         else
         {
		 	NETERROR(MFCE, ("Error initializing mutex attr [%d]\n", errno ));
         }

		 return -1;
	  }

	  /* set the mutex attribute values */

	  if ((errno = 
            pthread_mutexattr_setpshared(mutexAttr, PTHREAD_PROCESS_PRIVATE) != 0 ) /* ||
 cannot use RECURSIVE or ERRORCHECK, pthread_mutex_init fails
		  (errno = pthread_mutexattr_settype(mutexAttr, PTHREAD_MUTEX_RECURSIVE)) || 
 cannot user PTHREAD_PRIO_INHERIT in sol 2.7 machines
		  (errno = pthread_mutexattr_setprotocol(mutexAttr, PTHREAD_PRIO_INHERIT))*/)
	  {
         errmsg = strerror( errno );

         if ( errmsg )
         {
		 	NETERROR(MFCE, ("Error setting mutex attr values [%d]: %s\n", errno,
                    errmsg ));
         }
         else
         {
		 	NETERROR(MFCE, ("Error setting mutex attr values [%d]\n", errno));
         }

		 if ( (errno = pthread_mutexattr_destroy(mutexAttr) ) != 0 )
		 {
            errmsg = strerror( errno );
 
            if ( errmsg )
            {
				NETERROR(MFCE, ("Error destroying mutex attr [%d]: %s\n", errno,
                        errmsg ));
            }
            else
            {
				NETERROR(MFCE, ("Error destroying mutex attr [%d]\n", errno ));
            }
		 }
		 return -1;
	  }
   }

   /* initialize the mutex */
   if ((errno = pthread_mutex_init(mutex, mutexAttr)) != 0)
   {
      errmsg = strerror( errno );

      if ( errmsg )
      {
	  	NETERROR(MFCE, ("Error initializing mutex [%d]: %s\n", errno,
                errmsg ));
      }
      else
      {
	  	NETERROR(MFCE, ("Error initializing mutex [%d]\n", errno));
      }


	  if (mutexAttr != NULL)
	  {

		 if ( (errno = pthread_mutexattr_destroy(mutexAttr) ) != 0 )
		 {
            errmsg = strerror( errno );

            if ( errmsg )
		    {
			    NETERROR(MFCE, ("Error destroying mutex attr [%d]: %s\n", errno,
		                errmsg ));
            }
            else
		    {
			    NETERROR(MFCE, ("Error destroying mutex attr [%d]\n", errno ));
            }
		 }
	  }
	  return -1;
   }

   return 0;
}


/**
 * destroys the previously initialized mutex and mutex attribute
 *
 * @return 0 if destroy is successfull, -1 otherwise
 */ 
int destroyMutex (pthread_mutex_t *mutex, pthread_mutexattr_t *mutexAttr)
{
   int retcode = 0;
   char* errmsg;
   int errno;

   if ((errno = pthread_mutex_destroy(mutex)) != 0 )
   {
	  errmsg = strerror( errno );

	  if ( errmsg )
	  {
	  	NETERROR(MFCE, ("Error destroying mutex [%d]: %s\n", errno,
		      	errmsg ));
      }
      else
	  {
	  	NETERROR(MFCE, ("Error destroying mutex [%d]\n", errno ));
      }

	  retcode = -1;
   }

   if (mutexAttr != NULL)
   {
	  if ((errno = pthread_mutexattr_destroy(mutexAttr)) != 0)
	  {
		 errmsg = strerror( errno );

		 if ( errmsg )
		 {
		 	NETERROR(MFCE, ("Error destroying mutex attr [%d]: %s\n", errno,
		         	errmsg ));
		 }
		 else
		 {
		 	NETERROR(MFCE, ("Error destroying mutex attr [%d]\n", errno ));
		 }
	
		 retcode = -1;
	  }
   }

   return retcode;
}

