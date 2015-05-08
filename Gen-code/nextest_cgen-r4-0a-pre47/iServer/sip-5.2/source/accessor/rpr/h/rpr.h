/******************************************************************************
** FUNCTION:
** 	
**
*******************************************************************************
**
** FILENAME:
** 	rpr.h
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

#ifndef _RPR_H_
#define _RPR_H_

#include "rprinternal.h"
#include "sipstruct.h"
#include "sipcommon.h"
#include "rprfree.h"
#include "rprinit.h"


/* Ensure Names are not mangled by C++ compilers */
#ifdef __cplusplus
extern "C" {
#endif 

/****************************************************************************
** Function: sip_rpr_getRespNumFromRSeqHdr 
** Description:  get response number from RSeq header
** Parameters:
**		pHdr (IN)		- RSeq Header
**		pRespNum (OUT)	- retrieved response number
**		pErr (OUT)		- possible error value (see api ref. document)
****************************************************************************/
extern SipBool sip_rpr_getRespNumFromRSeqHdr _ARGS_ ((SipHeader *pHdr, \
	SIP_U32bit *pRespNum, SipError *pErr));

/****************************************************************************
** Function: sip_rpr_setRespNumInRSeqHdr 
** Description:  set response number in RSeq header
** Parameters:
**		pHdr (IN/OUT)	- RSeq Header
**		respNum (IN)	- response number to set
**		pErr (OUT)		- possible error value (see api ref. document)
****************************************************************************/
extern SipBool sip_rpr_setRespNumInRSeqHdr _ARGS_ ((SipHeader *pHdr, \
	SIP_U32bit respNum, SipError *pErr));

/****************************************************************************
** Function: sip_rpr_getRespNumFromRAckHdr 
** Description:  get Response number from RAck header
** Parameters:
**		pHdr (IN)		- RAck header
**		pRespNum (OUT)	- retrieved response number
**		pErr (OUT)		- possible error value (see api ref. document)
****************************************************************************/
extern SipBool sip_rpr_getRespNumFromRAckHdr _ARGS_ ((SipHeader *pHdr, \
	SIP_U32bit *pRespNum, SipError *pErr));

/****************************************************************************
** Function: sip_rpr_setRespNumInRAckHdr 
** Description:  set Response number in RAck header
** Parameters:
**		pHdr (IN/OUT) 	- RAck header
**		respNum (IN)	- response number to set
**		pErr (OUT)		- possible error value (see api ref. document)
****************************************************************************/
extern SipBool sip_rpr_setRespNumInRAckHdr _ARGS_ ((SipHeader *pHdr, \
	SIP_U32bit respNum, SipError *pErr));

/****************************************************************************
** Function: sip_rpr_getCseqNumFromRAckHdr 
** Description:  get CSeq number from RAck header
** Parameters:
**		pHdr (IN)	- Rack header
**		pCseq (OUT)	- retrieved seq. number
**		pErr (OUT)	- possible error value (see api ref. document)
****************************************************************************/
extern SipBool sip_rpr_getCseqNumFromRAckHdr _ARGS_ ((SipHeader *pHdr, \
	SIP_U32bit *pCseq, SipError *pErr));

/****************************************************************************
** Function: sip_rpr_setCseqNumInRAckHdr 
** Description:  set CSeq number in RAck header
** Parameters:
**		pHdr (IN/OUT)	- Rack header
**		cSeq (IN)		- seq. number to set
**		pErr (OUT)		- possible error value (see api ref. document)
**
****************************************************************************/
extern SipBool sip_rpr_setCseqNumInRAckHdr _ARGS_ ((SipHeader *pHdr, \
	SIP_U32bit cSeq, SipError *pErr));

/****************************************************************************
** Function: sip_rpr_getMethodFromRAckHdr 
** Description:  get Method value from RAck header
** Parameters:
**		pHdr (IN)		- Rack Header
**		ppMethod (OUT)	-  retrieved header method
**		pErr (OUT)		- possible error value (see api ref. document)
****************************************************************************/
extern SipBool sip_rpr_getMethodFromRAckHdr _ARGS_ ((SipHeader *pHdr, \
	SIP_S8bit **ppMethod, SipError *pErr));

/****************************************************************************
** Function: sip_rpr_setMethodInRAckHdr 
** Description:  set Method in RAck header
** Parameters:
**		pHdr (IN/OUT)		- Rack Header
**		pMethod (IN)		- method value to set
**		pErr (OUT)			- possible error value (see api ref. document)
****************************************************************************/
extern SipBool sip_rpr_setMethodInRAckHdr _ARGS_ ((SipHeader *pHdr, \
	SIP_S8bit *pMethod, SipError *pErr));


/* Ensure Names are not mangled by C++ compilers */
#ifdef __cplusplus
}
#endif 

#endif
