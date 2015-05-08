#ifndef __FCEUTILS_H
#define __FCEUTILS_H


/**
 * initializes mutex and stores in the given pointer
 *
 * @return 0 if init is successfull, -1 otherwise
 */ 
extern int initMutex (pthread_mutex_t *mutex, pthread_mutexattr_t *mutexAttr);

/**
 * destroys the previously initialized mutex and mutex attribute
 *
 * @return 0 if destroy is successfull, -1 otherwise
 */ 
extern int destroyMutex (pthread_mutex_t *mutex, pthread_mutexattr_t *mutexAttr);

/**
 * locks the mutex, prints any error if happens
 *
 * @return 0 if locking was successfull, -1 otherwise
 */
extern int lockMutex (pthread_mutex_t *mutex);

/**
 * unlocks the mutex, prints any error if happens
 *
 * @return 0 if unlocking was successfull, -1 otherwise
 */
extern int unlockMutex (pthread_mutex_t *mutex);


#endif

