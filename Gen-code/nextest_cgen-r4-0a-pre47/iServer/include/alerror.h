#ifndef _al_error_h_
#define _al_error_h_

typedef enum
{
	AL_OK = 0,
	AL_OKAY = 0,
	AL_DBOK = 0,
	AL_GENERROR = -1,               /* Some low level error */
	AL_FILENOTFOUND = -2,		/* File not found */
	AL_CANTOPENDBLOCKFILE = -3,	/* Can't open DB lock file */
	AL_NOEXLOCK = -4,		/* Cannot obtain exclusive lock */
	AL_DBMOPENERROR = -5,		/* Cannot open database */
	AL_DBUNLOCK = -6,		/* Cannot unlock database */
	AL_DBERRSTORE = -7,		/* Error in storing data */
	AL_DBERRDELETE = -8,		/* Error in deleting data */
	AL_ALIGNERROR = -9,             /* Badly aligned segment */
	AL_LOCKBUSY = -10,              /* Lock is already locked */
} AlStatus;



#endif 	/* _al_error_h_ */
