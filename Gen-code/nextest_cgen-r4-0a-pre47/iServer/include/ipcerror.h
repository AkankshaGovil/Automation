#ifndef _ipcerror_h_
#define _ipcerror_h_

#include "ipc.h"

/* Serplex Library Error (xle) codes */
/* when adding error codes here, corresponding error string need to be
 * added in BridgeServerImpl.c */
typedef enum
{
	xleOk = 0,			/* No Error */

	xleUndefined = 1,		/* Undefined Error */

	/* general errors */
	xleNoEntry = 2,			/* No Such Entry */
	xleOpNoPerm = 3,		/* Operation not permitted  */
	xleNoAccess = 4,		/* Access Denied */
	xleIOError = 5,			/* System I/O failed */
	xleExists = 6,			/* Entry exists */
	xleInvalArgs = 7,		/* Invalid Arguments */
	xleInsuffArgs = 8,		/* Insufficient Arguments */

	/* specific ones */
	xleBadVpn = 9,	
	xleNoLicense = 10,		/* No more licenses for add ports */

	xleMax,
} SerplexLibError;

#endif /* _ipcerror_h_ */
