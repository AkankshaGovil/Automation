
/*
** THIS FILE IS INTERNALLY INCLUDED BY THE STACK
** THE USER SHOULD NOT INCLUDE THIS FILE DIRECTLY
*/

/******************************************************************************
** FUNCTION:
** 	
**
*******************************************************************************
**
** FILENAME:
** 	rprinternal.h
**
** DESCRIPTION:
**  	
**
** DATE      	NAME        	REFERENCE      	REASON
** ----      	----        	---------      	------
** 23Feb2000   	S.Luthra    	Original
** 28Feb2000	B.Borthakur	Included Supported heeader
** 25Jul2000	S.Luthra	Moved Protypes of Supported header
** 
** Copyrights 1999, Hughes Software Systems, Ltd.
**
******************************************************************************/

#ifndef _RPRINTERNAL_H_
#define _RPRINTERNAL_H_

#include "portlayer.h"
#include "rprinit.h"
#include "rprfree.h"

/* Ensure Names are not mangled by C++ compilers */
#ifdef __cplusplus
extern "C" {
#endif 

SipBool __sip_rpr_cloneSipRAckHeader _ARGS_ ((SipRackHeader *pDest, SipRackHeader *pSource, SipError *pErr));
SipBool __sip_rpr_cloneSipRSeqHeader _ARGS_ ((SipRseqHeader *pDest, SipRseqHeader *pSource, SipError *pErr));

/* Ensure Names are not mangled by C++ compilers */
#ifdef __cplusplus
}
#endif 

#endif
