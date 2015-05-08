/***********************************************************************
 ** FUNCTION:
 **             Has Free Function Declarations For all tel-url Structures

 *********************************************************************
 **
 ** FILENAME:
 ** telfree.h
 **
 ** DESCRIPTION:
 ** This file contains code to free all structures
 **
 ** DATE        NAME                    REFERENCE               REASON
 ** ----        ----                    ---------              --------
 ** 4Jan01  	Rajasri       		                    Initial Creation
 **
 **
 **     Copyright 1999, Hughes Software Systems, Ltd.
 *********************************************************************/


#ifndef __TEL_FREE_H_
#define __TEL_FREE_H_

#include "telstruct.h"

/* Ensure Names are not mangled by C++ compilers */
#ifdef __cplusplus
extern "C" {
#endif 


extern void sip_freeTelUrl _ARGS_((TelUrl *pUrl));
extern void sip_freeTelLocalNum _ARGS_((TelLocalNum *pLocal));
extern void sip_freeTelGlobalNum _ARGS_((TelGlobalNum *pGlobal));


/* Ensure Names are not mangled by C++ compilers */
#ifdef __cplusplus
}
#endif 

#endif /*__TEL_FREE_H */
