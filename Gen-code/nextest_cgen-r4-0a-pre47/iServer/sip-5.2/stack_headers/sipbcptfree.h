
/***********************************************************************
 ** FUNCTION:
 **             Has Free Functions For all bcpt Structures

 *********************************************************************
 **
 ** FILENAME:
 ** sipbcptfree.h
 **
 ** DESCRIPTION:
 ** This file contains code to free all bcpt structures
 **
 ** DATE        NAME                    REFERENCE               REASON
 ** ----        ----                    ---------              --------
 ** 10/02/00   B. Borthakur       		                    Initial Creation
 **
 **
 **     Copyright 1999, Hughes Software Systems, Ltd.
 *********************************************************************/

#ifndef __SIP_BCPTFREE_H_
#define __SIP_BCPTFREE_H_


#include <stdlib.h>
#include "sipstruct.h"
#include "sipcommon.h"


/* Ensure Names are not mangled by C++ compilers */
#ifdef __cplusplus
extern "C" {
#endif 


extern void sip_bcpt_freeIsupMessage _ARGS_((IsupMessage * m));
extern void sip_bcpt_freeQsigMessage _ARGS_((QsigMessage * m));
extern void sip_bcpt_freeSipMimeVersionHeader _ARGS_((SipMimeVersionHeader * h));
extern void sip_bcpt_freeMimeMessage _ARGS_((MimeMessage * m));
extern void sip_bcpt_freeSipMimeHeader _ARGS_((SipMimeHeader * h));
extern void __sip_bcpt_freeMimeMessage _ARGS_((SIP_Pvoid  m));


/* Ensure Names are not mangled by C++ compilers */
#ifdef __cplusplus
}
#endif 

#endif
