/******************************************************************************
** FUNCTION:
** 	
**
*******************************************************************************
**
** FILENAME:
** 	rprfree.h
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
#ifndef _RPRFREE_H_
#define _RPRFREE_H_

#include "sipfree.h"

/* Ensure Names are not mangled by C++ compilers */
#ifdef __cplusplus
extern "C" {
#endif 

extern void sip_rpr_freeSipRAckHeader _ARGS_ ((SipRackHeader *pHdr));

extern void sip_rpr_freeSipRSeqHeader _ARGS_ ((SipRseqHeader *pHdr));


/* Ensure Names are not mangled by C++ compilers */
#ifdef __cplusplus
}
#endif 

#endif
