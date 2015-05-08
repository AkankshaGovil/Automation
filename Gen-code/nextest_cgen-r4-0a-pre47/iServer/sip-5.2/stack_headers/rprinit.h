/******************************************************************************
** FUNCTION:
** 	
**
*******************************************************************************
**
** FILENAME:
** 	rprinit.h
**
** DESCRIPTION:
**  
**
** DATE      	NAME        	REFERENCE      	REASON
** ----      	----        	---------      	------
** 23Feb2000   	S.Luthra    	Original
**
** Copyrights 1999, Hughes Software Systems, Ltd.
**
******************************************************************************/
#ifndef _RPRINIT_H_
#define _RPRINIT_H_

#include "sipinit.h"

/* Ensure Names are not mangled by C++ compilers */
#ifdef __cplusplus
extern "C" {
#endif 


extern SipBool sip_rpr_initSipRAckHeader _ARGS_ ((SipRackHeader **ppHdr, SipError *pErr));

extern SipBool sip_rpr_initSipRSeqHeader _ARGS_ ((SipRseqHeader **ppHdr, SipError *pErr));


/* Ensure Names are not mangled by C++ compilers */
#ifdef __cplusplus
}
#endif 

#endif
