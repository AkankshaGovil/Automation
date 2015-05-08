/******************************************************************************
 ** FUNCTION:
 **	 	This file has all API definitions of SIP Caller & Callee 
 **             preferences Related Structures
 **
 ******************************************************************************
 **
 ** FILENAME:
 ** 		ccp.h
 **
 ** DESCRIPTION:
 **	 
 **
 ** DATE	NAME		REFERENCE	REASON
 ** ----	----		--------	------
 ** 8/2/2000	S.Luthra	Original
 **
 **
 **	Copyright 1999, Hughes Software Systems, Ltd. 
 ******************************************************************************/

#ifndef _CCP_H_
#define _CCP_H_

#include "sipstruct.h"
#include "ccpstruct.h"
#include "portlayer.h"
#include "sipinternal.h"
#include "ccpinternal.h"


/* Ensure Names are not mangled by C++ compilers */
#ifdef __cplusplus
extern "C" {
#endif 

#ifdef SIP_CCP_VERSION10
/**********************************************************************
**
** FUNCTION:  sip_ccp_getTypeFromAcceptContactParam
**
** DESCRIPTION: This function retrieves the typeof an accept-Conatct 
**		param i.e. whether its of dType ExtParam or Qvalue
**
**********************************************************************/
SipBool sip_ccp_getTypeFromAcceptContactParam 
	(SipAcceptContactParam *pAcceptContact, en_AcceptContactType *pType, SipError *pErr) ;

/**********************************************************************
**
** FUNCTION:  sip_ccp_getTokenParamFromAcceptContactParam
**
** DESCRIPTION: This function retrives the from a SIP
**		Accept-Contact pHeader structure
**
**********************************************************************/
SipBool sip_ccp_getTokenParamFromAcceptContactParam
	(SipAcceptContactParam *pAcceptContact, SIP_S8bit **ppParam, SipError *pErr) ;

/**********************************************************************
**
** FUNCTION:  sip_ccp_setTokenParamInAcceptContactHdr
**
** DESCRIPTION: This function sets the Display pName in filed in a SIP
**		Reject Contact pHeader
**
**********************************************************************/
SipBool sip_ccp_setTokenParamInAcceptContactParam
	(SipAcceptContactParam *pAcceptContact, SIP_S8bit *pName, SipError *pErr) ;


/**********************************************************************
**
** FUNCTION:  sip_ccp_getFeatureParamFromAcceptContactParam
**
** DESCRIPTION: This function retrives the feature-param field from
**		a SIP accept-contact param structure
**
**********************************************************************/
SipBool sip_ccp_getFeatureParamFromAcceptContactParam 
#ifdef SIP_BY_REFERENCE
	(SipAcceptContactParam *pAcceptContact, SipParam **ppParam, SipError *pErr) ;
#else
	(SipAcceptContactParam *pAcceptContact, SipParam *ppParam, SipError *pErr) ;
#endif


/**********************************************************************
**
** FUNCTION:  sip_ccp_setFeatureParamInAcceptContactParam
**
** DESCRIPTION: This function sets the feature-param field in a SIP
**		accept-contact param structure
**
**********************************************************************/
 SipBool sip_ccp_setFeatureParamInAcceptContactParam 
	(SipAcceptContactParam *pAcceptContact, SipParam *pParam, SipError *pErr) ;


/**********************************************************************
**
** FUNCTION:  sip_ccp_getGenericParamFromAcceptContactParam
**
** DESCRIPTION: This function retrives the extension-param field from
**		a SIP accept-contact param structure
**
**********************************************************************/
SipBool sip_ccp_getGenericParamFromAcceptContactParam 
#ifdef SIP_BY_REFERENCE
	(SipAcceptContactParam *pAcceptContact, SipParam **ppParam, SipError *pErr) ;
#else
	(SipAcceptContactParam *pAcceptContact, SipParam *ppParam, SipError *pErr) ;
#endif

/**********************************************************************
**
** FUNCTION:  sip_ccp_setGenericParamInAcceptContactParam
**
** DESCRIPTION: This function sets the extension-param field in a SIP
**		accept-contact param structure
**
**********************************************************************/
 SipBool sip_ccp_setGenericParamInAcceptContactParam 
	(SipAcceptContactParam *pAcceptContact, SipParam *pParam, SipError *pErr) ;

/**********************************************************************
**
** FUNCTION:  sip_ccp_getAcceptContactParamCountFromAcceptContactHdr
**
** DESCRIPTION: This function retrives the nember of accept-contact
**		pParam from a SIP Accept-Conatct pHeader
**
**********************************************************************/
SipBool sip_ccp_getAcceptContactParamCountFromAcceptContactHdr 
	(SipHeader *pHdr, SIP_U32bit *pCount, SipError *pErr) ;


/**********************************************************************
**
** FUNCTION:  sip_ccp_getAcceptContactParamAtIndexFromAcceptContactHdr
**
** DESCRIPTION: This function retrives an accept-contact param at a
**		specified index from a SIP accept Conatct pHeader
**
**********************************************************************/
SipBool sip_ccp_getAcceptContactParamAtIndexFromAcceptContactHdr 
#ifdef SIP_BY_REFERENCE
	(SipHeader *pHdr, SipAcceptContactParam **ppAcceptContactParam, SIP_U32bit index, SipError *pErr) ;
#else
	(SipHeader *pHdr, SipAcceptContactParam *pAcceptContactParam, SIP_U32bit index, SipError *pErr) ;
#endif


/**********************************************************************
**
** FUNCTION:  sip_ccp_getAcceptContactParamCountFromAcceptContactHdr
**
** DESCRIPTION: This function retrives the nember of accept-contact
**		pParam from a SIP Accept-Conatct pHeader
**
**********************************************************************/
SipBool sip_ccp_getAcceptContactParamCountFromAcceptContactHdr 
	(SipHeader *pHdr, SIP_U32bit *pCount, SipError *pErr) ;

/**********************************************************************
**
** FUNCTION:  sip_ccp_insertAcceptContactParamAtIndexInAcceptContactHdr
**
** DESCRIPTION: This function inserts an accept-contact param at a
**		specified index in the Accept-Contact pHeader
**
**********************************************************************/
SipBool sip_ccp_insertAcceptContactParamAtIndexInAcceptContactHdr 
	(SipHeader *pHdr, SipAcceptContactParam *pAcceptContactParam, SIP_U32bit index, SipError *pErr) ;



/**********************************************************************
**
** FUNCTION:  sip_ccp_getTypeFromRejectContactParam
**
** DESCRIPTION: This function retrieves the dType of a Reject-Contact
**		param structure i.e. Ext-param or Q-pValue
**
**********************************************************************/
SipBool sip_ccp_getTypeFromRejectContactParam 
	(SipRejectContactParam *pRejectContact, en_RejectContactType *pType, SipError *pErr) ;


/**********************************************************************
**
** FUNCTION:  sip_ccp_getTokenParamFromRejectContactParam
**
** DESCRIPTION: This function retrieves the pTokenParam-pValue field from a SIP
**		Reject-Contact param structure
**
**********************************************************************/
SipBool sip_ccp_getTokenParamFromRejectContactParam 
	(SipRejectContactParam *pRejectContact, SIP_S8bit **ppToken, SipError *pErr);

/**********************************************************************
**
** FUNCTION:  sip_ccp_getFeatureParamFromRejectContactParam
**
** DESCRIPTION: This function retrives the extension-parm field from a
**		SIP Reject contact param structrure
**
**********************************************************************/
SipBool sip_ccp_getFeatureParamFromRejectContactParam 
#ifdef SIP_BY_REFERENCE
	(SipRejectContactParam *pRejectContact, SipParam **ppParam, SipError *pErr) ;
#else
	(SipRejectContactParam *pRejectContact, SipParam *pParam, SipError *pErr) ;
#endif


/**********************************************************************
**
** FUNCTION:  sip_ccp_getGenericParamFromRejectContactParam
**
** DESCRIPTION: This function retrives the extension-parm field from a
**		SIP Reject contact param structrure
**
**********************************************************************/
SipBool sip_ccp_getGenericParamFromRejectContactParam 
#ifdef SIP_BY_REFERENCE
	(SipRejectContactParam *pRejectContact, SipParam **ppParam, SipError *pErr) ;
#else
	(SipRejectContactParam *pRejectContact, SipParam *pParam, SipError *pErr) ;
#endif

/**********************************************************************
**
** FUNCTION:  sip_ccp_setTokenParamInRejectContactPara
**
** DESCRIPTION: This function sets the Q-pValue field in a SIP Reject
**		contact param structure
**
**********************************************************************/
SipBool sip_ccp_setTokenParamInRejectContactParam 
	(SipRejectContactParam *pRejectContact, SIP_S8bit *pToken, SipError *pErr) ;

/**********************************************************************
**
** FUNCTION:  sip_ccp_setFeatureParamInRejectContactParam
**
** DESCRIPTION: This function sets the Feature-param field in a SIP
**		reject-contact param structure
**
**********************************************************************/
SipBool sip_ccp_setFeatureParamInRejectContactParam 
	(SipRejectContactParam *pRejectContact, SipParam *pFeatureParam, 
	SipError *pErr) ;


/**********************************************************************
**
** FUNCTION:  sip_ccp_setGenericParamInRejectContactParam
**
** DESCRIPTION: This function sets the Feature-param field in a SIP
**		reject-contact param structure
**
**********************************************************************/
SipBool sip_ccp_setGenericParamInRejectContactParam 
	(SipRejectContactParam *pRejectContact, SipParam *pFeatureParam, 
	SipError *pErr) ;
	
/**********************************************************************
**
** FUNCTION:  sip_ccp_getRejectContactParamCountFromRejectContactHdr
**
** DESCRIPTION: This function retrives the number of reject-contact
**		pParam inb a reject-contact pHeader structure
**
**********************************************************************/
SipBool sip_ccp_getRejectContactParamCountFromRejectContactHdr 
	(SipHeader *pHdr, SIP_U32bit *pCount, SipError *pErr) ;


/**********************************************************************
**
** FUNCTION:  sip_ccp_getRejectContactParamAtIndexFromRejectContactHdr
**
** DESCRIPTION: This function retrivea a reject-contact param at a 
**		specified index in a SIP Reject-contact pHeader.
**
**********************************************************************/
SipBool sip_ccp_getRejectContactParamAtIndexFromRejectContactHdr 
#ifdef SIP_BY_REFERENCE
	(SipHeader *pHdr, SipRejectContactParam **ppRejectContactParam, SIP_U32bit index, SipError *pErr) ;
#else
	(SipHeader *pHdr, SipRejectContactParam *pRejectContactParam, SIP_U32bit index, SipError *pErr) ;
#endif

/**********************************************************************
**
** FUNCTION:  sip_ccp_insertRejectContactParamAtIndexInRejectContactHd
**
** DESCRIPTION: This function inserts  a reject-contact param at a 
**		specified index in a SIP Reject contact pHeader structure
**
**********************************************************************/
SipBool sip_ccp_insertRejectContactParamAtIndexInRejectContactHdr 
	(SipHeader *pHdr, SipRejectContactParam *pRejectContactParam, 
	SIP_U32bit index, SipError *pErr) ;

/**********************************************************************
**
** FUNCTION:  sip_ccp_setRejectContactParamAtIndexInRejectContactHdr
**
** DESCRIPTION: This function sets a reject-contact param at a specified
** 		index in a SIP reject-contact pHeader structure
**
**********************************************************************/
SipBool sip_ccp_setRejectContactParamAtIndexInRejectContactHdr 
	(SipHeader *pHdr, SipRejectContactParam *pRejectContactParam, 
	SIP_U32bit index, SipError *pErr) ;


/**********************************************************************
**
** FUNCTION:  sip_ccp_deleteRejectContactParamAtIndexInRejectContactHdr
**
** DESCRIPTION: This function deletes a Reject-Contact param at a
**		specified index in the Reject-Contact Header structure
**
**********************************************************************/
SipBool sip_ccp_deleteRejectContactParamAtIndexInRejectContactHdr 
	(SipHeader *pHdr, SIP_U32bit index, SipError *pErr) ;



/**********************************************************************
**
** FUNCTION: sip_ccp_setAcceptContactParamAtIndexInAcceptContactHdr 
**
** DESCRIPTION: This fuunction sets an accept-contact param at a
**		specified index in the Accept-Contact pHeader
**
**********************************************************************/
SipBool sip_ccp_setAcceptContactParamAtIndexInAcceptContactHdr 
	(SipHeader *pHdr, SipAcceptContactParam *pAcceptContactParam, 
	SIP_U32bit index, SipError *pErr) ;

/**********************************************************************
**
** FUNCTION:  sip_ccp_deleteAcceptContactParamAtIndexInAcceptContactHdr
**
** DESCRIPTION: This function deletes an accept-contact param at a
**		specified index in the SIP Accept-Contact pHeader
**
**********************************************************************/
SipBool sip_ccp_deleteAcceptContactParamAtIndexInAcceptContactHdr 
	(SipHeader *pHdr, SIP_U32bit index, SipError *pErr) ;

#else
/******************************************************************
** Function:sip_ccp_getTypeFromAcceptContactParam 
** Description: get type from AcceptContact param
** Parameters:	
**				pAcceptContact(IN)	- Sip AcceptContact param 
**				pType(OUT) 			- type of acceptcontact param retrieved 
**				pErr(OUT)   		- Possible error value (See API ref doc)
********************************************************************/
extern SipBool sip_ccp_getTypeFromAcceptContactParam _ARGS_(( \
		SipAcceptContactParam *pAcceptContact, en_AcceptContactType *pType, SipError *pErr));

/*****************************************************************
** Function: sip_ccp_getQvalueFromAcceptContactParam 
** Description: get qvalue from AcceptContact param
** Parameters:	
**				pAcceptContact(IN)	- Sip AcceptContact param 
**				ppQvalue(OUT) 		- qvalue of acceptcontact param retrieved 
**				pErr(OUT)   		- Possible error value (See API ref doc)
*****************************************************************/
extern SipBool sip_ccp_getQvalueFromAcceptContactParam _ARGS_(( \
		SipAcceptContactParam *pAcceptContact, SIP_S8bit **ppQvalue, SipError *pErr));
#ifdef SIP_BY_REFERENCE
/*****************************************************************
** Function: sip_ccp_getExtParamFromAcceptContactParam 
** Description: get extension param from AcceptContact param
** Parameters:	
**				pAcceptContact(IN)	- Sip AcceptContact param 
**				ppExtParam(OUT)		- Extension param of acceptcontact param retrieved 
**				pErr(OUT)   		- Possible error value (See API ref doc)
*****************************************************************/
extern SipBool sip_ccp_getExtParamFromAcceptContactParam _ARGS_(( \
		SipAcceptContactParam *pAcceptContact, SipParam **ppExtParam, SipError *pErr));
#else
/*****************************************************************
** Function: sip_ccp_getExtParamFromAcceptContactParam 
** Description: get extension param from AcceptContact param
** Parameters:	
**				pAcceptContact(IN)	- Sip AcceptContact param 
**				ppExtParam(OUT)		- Extension param of acceptcontact param retrieved 
**				pErr(OUT)   		- Possible error value (See API ref doc)
*****************************************************************/
extern SipBool sip_ccp_getExtParamFromAcceptContactParam _ARGS_(( \
		SipAcceptContactParam *pAcceptContact, SipParam *pExtParam, SipError *pErr));
#endif

/*****************************************************************
** Function: sip_ccp_setQvalueInAcceptContactParam 
** Description: set qvalue in AcceptContact param
** Parameters:	
**				pAcceptContact(IN)	- Sip AcceptContact param 
**				pQvalue(OUT) 		- qvalue of acceptcontact param to  set 
**				pErr(OUT)   		- Possible error value (See API ref doc)
*****************************************************************/
extern SipBool sip_ccp_setQvalueInAcceptContactParam _ARGS_(( \
		SipAcceptContactParam *pAcceptContact, SIP_S8bit *pQvalue, SipError *pErr));

/*****************************************************************
** Function: sip_ccp_setExtParamInAcceptContactParam 
** Description: set Extension param in AcceptContact param
** Parameters:	
**				pAcceptContact(IN)	- Sip AcceptContact param 
**				pQvalue(OUT) 		- Extension param of acceptcontact param to  set 
**				pErr(OUT)   		- Possible error value (See API ref doc)
*****************************************************************/
extern SipBool sip_ccp_setExtParamInAcceptContactParam _ARGS_(( \
		SipAcceptContactParam *pAcceptContact, SipParam *pExtParam, SipError *pErr));

/*****************************************************************
** Function: sip_ccp_getTypeFromRejectContactParam 
** Description: get type from RejectContact param
** Parameters:	
**				pRejectContact(IN)	- Sip RejectContact param 
**				pType(OUT) 			- type of acceptcontact param  retrieved
**				pErr(OUT)   		- Possible error value (See API ref doc)
*****************************************************************/
extern SipBool sip_ccp_getTypeFromRejectContactParam _ARGS_ (( \
		SipRejectContactParam *pRejectContact, en_RejectContactType *pType, SipError *pErr));

/*****************************************************************
** Function: sip_ccp_getQvalueFromRejectContactParam 
** Description: get qvalue from RejectContact param
** Parameters:	
**				pRejectContact(IN)	- Sip RejectContact param 
**				ppQvalue(OUT) 		- qvalue of  reject contact param retrieved
**				pErr(OUT)   		- Possible error value (See API ref doc)
*****************************************************************/
extern SipBool sip_ccp_getQvalueFromRejectContactParam _ARGS_(( \
		SipRejectContactParam *pRejectContact, SIP_S8bit **ppQvalue, SipError *pErr));

#ifdef SIP_BY_REFERENCE
/*****************************************************************
** Function: sip_ccp_getExtParamFromRejectContactParam 
** Description: get extension param from RejectContact param
** Parameters:	
**				pRejectContact(IN)	- Sip RejectContact param 
**				ppQvalue(OUT) 		- qvalue of  reject contact param retrieved
**				pErr(OUT)   		- Possible error value (See API ref doc)
*****************************************************************/
extern SipBool sip_ccp_getExtParamFromRejectContactParam _ARGS_(( \
		SipRejectContactParam *pRejectContact, SipParam **ppExtParam, SipError *pErr));
#else
/*****************************************************************
** Function: sip_ccp_getExtParamFromRejectContactParam 
** Description: get extension param from RejectContact param
** Parameters:	
**				pRejectContact(IN)	- Sip RejectContact param 
**				pExtParam(OUT) 		- extension param of  reject contact param retrieved
**				pErr(OUT)   		- Possible error value (See API ref doc)
*****************************************************************/
extern SipBool sip_ccp_getExtParamFromRejectContactParam _ARGS_(( \
		SipRejectContactParam *pRejectContact, SipParam *pExtParam, SipError *pErr));
#endif

/*****************************************************************
** Function: sip_ccp_setQvalueInRejectContactParam 
** Description: set qvalue in RejectContact param
** Parameters:	
**				pRejectContact(IN/OUT)	- Sip RejectContact param 
**				ppQvalue(IN) 			- qvalue of  reject contact param to set
**				pErr(OUT)   			- Possible error value (See API ref doc)
*****************************************************************/
extern SipBool sip_ccp_setQvalueInRejectContactParam _ARGS_(( \
		SipRejectContactParam *pRejectContact, SIP_S8bit *pQvalue, SipError *pErr));

/*****************************************************************
** Function: sip_ccp_setExtParamInRejectContactParam 
** Description: set extension param in RejectContact param
** Parameters:	
**				pRejectContact(IN/OUT)	- Sip RejectContact param 
**				pExtParam(IN) 			- extension param of  reject contact param to set
**				pErr(OUT)   			- Possible error value (See API ref doc)
*****************************************************************/
extern SipBool sip_ccp_setExtParamInRejectContactParam _ARGS_(( \
		SipRejectContactParam *pRejectContact, SipParam *pExtParam, SipError *pErr));

/*****************************************************************
** Function: sip_ccp_getDispNameFromRejectContactHdr 
** Description: get display name from Reject Contact header
** Parameters:	
**				pHdr(IN)			- Sip reject contact Header 
**				ppDispName(OUT)		- display name of Reject Contact Header retrieved
**				pErr(OUT)   		- Possible error value (See API ref doc)
*****************************************************************/
extern SipBool sip_ccp_getDispNameFromRejectContactHdr _ARGS_ ((SipHeader *pHdr, \
		SIP_S8bit **ppDispName, SipError *pErr));

/*****************************************************************
** Function: sip_ccp_setDispNameInRejectContactHdr 
** Description: set display name in Reject Contact header
** Parameters:	
**				pHdr(IN/OUT)		- Sip Reject Contact Header 
**				pDispName(IN) 		- display name of Reject Contact Header to set
**				pErr(OUT)   		- Possible error value (See API ref doc)
*****************************************************************/
extern SipBool sip_ccp_setDispNameInRejectContactHdr _ARGS_ ((SipHeader *pHdr, \
		SIP_S8bit *pDispName, SipError *pErr));

#ifdef SIP_BY_REFERENCE
/*****************************************************************
** Function: sip_ccp_getAddrSpecFromRejectContactHdr 
** Description: get Addrspec from reject contact header
** Parameters:	
**				pHdr(IN)			- Sip reject contact Header 
**				ppAddrSpec(OUT)		- AddrSpec of reject contact Header retrieved
**				pErr(OUT)   		- Possible error value (See API ref doc)
*****************************************************************/
extern SipBool sip_ccp_getAddrSpecFromRejectContactHdr _ARGS_ ((SipHeader *pHdr, \
		SipAddrSpec **ppAddrSpec, SipError *pErr));
#else
/*****************************************************************
** Function: sip_ccp_getAddrSpecFromRejectContactHdr 
** Description: get Addrspec from reject contact header
** Parameters:	
**				pHdr(IN)			- Sip reject contact Header 
**				pAddrSpec(OUT)		- AddrSpec of reject contact Header retrieved
**				pErr(OUT)   		- Possible error value (See API ref doc)
*****************************************************************/
extern SipBool sip_ccp_getAddrSpecFromRejectContactHdr _ARGS_ ((SipHeader *pHdr, \
		SipAddrSpec *pAddrSpec, SipError *pErr));
#endif

/*****************************************************************
** Function: sip_ccp_setAddrSpecInRejectContactHdr 
** Description: set Addrspec in reject contact header
** Parameters:	
**				pHdr(IN/OUT)		- Sip reject contact Header 
**				pAddrSpec(IN)		- AddrSpec of reject contact Header to set
**				pErr(OUT)   		- Possible error value (See API ref doc)
*****************************************************************/
extern SipBool sip_ccp_setAddrSpecInRejectContactHdr _ARGS_ ((SipHeader *pHdr, \
		SipAddrSpec *pAddrSpec, SipError *pErr));

/*****************************************************************
** Function: sip_ccp_getRejectContactParamCountFromRejectContactHdr 
** Description: get count of reject contact params in reject contact header
** Parameters:	
**				pHdr(IN)			- Sip reject contact Header 
**				pCount(OUT)			- Count of reject contact params in reject contact Header 
**				pErr(OUT)   		- Possible error value (See API ref doc)
*****************************************************************/
extern SipBool sip_ccp_getRejectContactParamCountFromRejectContactHdr _ARGS_ ((SipHeader *pHdr, \
		SIP_U32bit *pCount, SipError *pErr));

#ifdef SIP_BY_REFERENCE
/*****************************************************************
** Function: sip_ccp_getRejectContactParamAtIndexFromRejectContactHdr 
** Description: get reject contact param at index from reject contact header
** Parameters:	
**				pHdr(IN)					- Sip reject contact Header 
**				ppRejectContactParam(OUT)	- reject contact param retrieved
**				index(IN)					- index at which reject contact param  is to retrieved
**				pErr(OUT)					- Possible error value (See API ref doc)
*****************************************************************/
extern SipBool sip_ccp_getRejectContactParamAtIndexFromRejectContactHdr _ARGS_ ((SipHeader *pHdr, \
		SipRejectContactParam **ppRejectContactParam, SIP_U32bit index, SipError *pErr));
#else
/*****************************************************************
** Function: sip_ccp_getRejectContactParamAtIndexFromRejectContactHdr 
** Description: get reject contact param at index from reject contact header
** Parameters:	
**				pHdr(IN)					- Sip reject contact Header 
**				ppRejectContactParam(OUT)	- reject contact param retrieved
**				index(IN)					- index at which reject contact param  is to retrieved
**				pErr(OUT)					- Possible error value (See API ref doc)
*****************************************************************/
extern SipBool sip_ccp_getRejectContactParamAtIndexFromRejectContactHdr _ARGS_ ((SipHeader *pHdr, \
		SipRejectContactParam *pRejectContactParam, SIP_U32bit index, SipError *pErr));
#endif

/*****************************************************************
** Function: sip_ccp_insertRejectContactParamAtIndexInRejectContactHdr 
** Description: insert reject contact param at index in reject contact header
** Parameters:	
**				pHdr(IN/OUT)				- Sip reject contact Header 
**				pParam(IN)					- reject contact param to be inserted
**				index(IN)					- index at which reject contact param  is to be inserted
**				pErr(OUT)					- Possible error value (See API ref doc)
*****************************************************************/
extern SipBool sip_ccp_insertRejectContactParamAtIndexInRejectContactHdr _ARGS_ ((SipHeader *pHdr, \
		SipRejectContactParam *pParam, SIP_U32bit index, SipError *pErr));

/*****************************************************************
** Function: sip_ccp_setRejectContactParamAtIndexInRejectContactHdr 
** Description: set reject contact param at index in reject contact header
** Parameters:	
**				pHdr(IN/OUT)				- Sip reject contact Header 
**				pParam(IN)					- reject contact param to be set
**				index(IN)					- index at which reject contact param  is to be set
**				pErr(OUT)					- Possible error value (See API ref doc)
*****************************************************************/
extern SipBool sip_ccp_setRejectContactParamAtIndexInRejectContactHdr _ARGS_ ((SipHeader *pHdr, \
		SipRejectContactParam *pParam, SIP_U32bit index, SipError *pErr));

/*****************************************************************
** Function: sip_ccp_deleteRejectContactParamAtIndexInRejectContactHdr 
** Description: delete reject contact param at index in reject contact header
** Parameters:	
**				pHdr(IN/OUT)				- Sip reject contact Header 
**				index(IN)					- index at which reject contact param  is to be deleted
**				pErr(OUT)					- Possible error value (See API ref doc)
*****************************************************************/
extern SipBool sip_ccp_deleteRejectContactParamAtIndexInRejectContactHdr _ARGS_ ((SipHeader *pHdr, \
		SIP_U32bit index, SipError *pErr));

#ifdef SIP_BY_REFERENCE
/*****************************************************************
** Function: sip_ccp_getAddrSpecFromAcceptContactHdr 
** Description: get Addrspec from accept contact header
** Parameters:	
**				pHdr(IN)			- Sip accept contact Header 
**				ppAddrSpec(OUT)		- AddrSpec of accept contact Header retrieved
**				pErr(OUT)   		- Possible error value (See API ref doc)
*****************************************************************/
extern SipBool sip_ccp_getAddrSpecFromAcceptContactHdr _ARGS_ ((SipHeader *pHdr, \
		SipAddrSpec **ppAddrSpec, SipError *pErr));
#else
/*****************************************************************
** Function: sip_ccp_getAddrSpecFromAcceptContactHdr 
** Description: get Addrspec from accept contact header
** Parameters:	
**				pHdr(IN)			- Sip accept contact Header 
**				ppAddrSpec(OUT)		- AddrSpec of accept contact Header retrieved
**				pErr(OUT)   		- Possible error value (See API ref doc)
*****************************************************************/
extern SipBool sip_ccp_getAddrSpecFromAcceptContactHdr _ARGS_ ((SipHeader *pHdr, \
		SipAddrSpec *pAddrSpec, SipError *pErr));
#endif

/*****************************************************************
** Function: sip_ccp_setAddrSpecInAcceptContactHdr 
** Description: set Addrspec in accept contact header
** Parameters:	
**				pHdr(IN/OUT)		- Sip accept contact Header 
**				pAddrSpec(IN)		- AddrSpec of accept contact Header to set
**				pErr(OUT)   		- Possible error value (See API ref doc)
*****************************************************************/
extern SipBool sip_ccp_setAddrSpecInAcceptContactHdr _ARGS_ ((SipHeader *pHdr, \
		SipAddrSpec *pAddrSpec, SipError *pErr));

/*****************************************************************
** Function: sip_ccp_getDispNameFromAcceptContactHdr 
** Description: get display name from accept Contact header
** Parameters:	
**				pHdr(IN)			- Sip accept contact Header 
**				ppDispName(OUT)		- display name of accept Contact Header retrieved
**				pErr(OUT)   		- Possible error value (See API ref doc)
*****************************************************************/
extern SipBool sip_ccp_getDispNameFromAcceptContactHdr _ARGS_ ((SipHeader *pHdr, \
		SIP_S8bit **ppDispName, SipError *pErr));

/*****************************************************************
** Function: sip_ccp_setDispNameInAcceptContactHdr 
** Description: set display name in accept Contact header
** Parameters:	
**				pHdr(IN/OUT)		- Sip accept Contact Header 
**				pDispName(IN) 		- display name of accept Contact Header to set
**				pErr(OUT)   		- Possible error value (See API ref doc)
*****************************************************************/
extern SipBool sip_ccp_setDispNameInAcceptContactHdr _ARGS_ ((SipHeader *pHdr, \
		SIP_S8bit *pDispName, SipError *pErr));

/*****************************************************************
** Function: sip_ccp_getAcceptContactParamCountFromAcceptContactHdr 
** Description: get accept contact param at index from accept contact header
** Parameters:	
**				pHdr(IN)					- Sip accept contact Header 
**				ppRejectContactParam(OUT)	- accept contact param retrieved
**				index(IN)					- index at which accept contact param  is to retrieved
**				pErr(OUT)					- Possible error value (See API ref doc)
*****************************************************************/
extern SipBool sip_ccp_getAcceptContactParamCountFromAcceptContactHdr _ARGS_ ((\
		SipHeader *pHdr, SIP_U32bit *pCount, SipError *pErr));

#ifdef SIP_BY_REFERENCE
/*****************************************************************
** Function: sip_ccp_getAcceptContactParamAtIndexFromAcceptContactHdr 
** Description: get accept contact param at index from accept contact header
** Parameters:	
**				pHdr(IN)					- Sip accept contact Header 
**				ppRejectContactParam(OUT)	- accept contact param retrieved
**				index(IN)					- index at which accept contact param  is to retrieved
**				pErr(OUT)					- Possible error value (See API ref doc)
*****************************************************************/
extern SipBool sip_ccp_getAcceptContactParamAtIndexFromAcceptContactHdr _ARGS_ ((\
		SipHeader *pHdr, SipAcceptContactParam **ppAcceptContactParam, \
				SIP_U32bit index, SipError *pErr));
#else
/*****************************************************************
** Function: sip_ccp_getAcceptContactParamAtIndexFromAcceptContactHdr 
** Description: get accept contact param at index from accept contact header
** Parameters:	
**				pHdr(IN)					- Sip accept contact Header 
**				ppRejectContactParam(OUT)	- accept contact param retrieved
**				index(IN)					- index at which accept contact param  is to retrieved
**				pErr(OUT)					- Possible error value (See API ref doc)
*****************************************************************/
extern SipBool sip_ccp_getAcceptContactParamAtIndexFromAcceptContactHdr _ARGS_ ((\
		SipHeader *pHdr, SipAcceptContactParam *pAcceptContactParam, \
				SIP_U32bit index, SipError *pErr));
#endif

/*****************************************************************
** Function: sip_ccp_setAcceptContactParamAtIndexInAcceptContactHdr 
** Description: set accept contact param at index in accept contact header
** Parameters:	
**				pHdr(IN/OUT)				- Sip accept contact Header 
**				pParam(IN)					- accept contact param to be set
**				index(IN)					- index at which accept contact param  is to be set
**				pErr(OUT)					- Possible error value (See API ref doc)
*****************************************************************/
extern SipBool sip_ccp_setAcceptContactParamAtIndexInAcceptContactHdr _ARGS_ ((\
		SipHeader *pHdr, SipAcceptContactParam *pAcceptContactParam, \
				SIP_U32bit index, SipError *pErr));

/*****************************************************************
** Function: sip_ccp_insertAcceptContactParamAtIndexInAcceptContactHdr 
** Description: insert accept contact param at index in accept contact header
** Parameters:	
**				pHdr(IN/OUT)				- Sip accept contact Header 
**				pParam(IN)					- accept contact param to be inserted
**				index(IN)					- index at which accept contact param  is to be inserted
**				pErr(OUT)					- Possible error value (See API ref doc)
*****************************************************************/
extern SipBool sip_ccp_insertAcceptContactParamAtIndexInAcceptContactHdr _ARGS_ ((\
		SipHeader *pHdr, SipAcceptContactParam *pAcceptContactParam, \
				SIP_U32bit index, SipError *pErr));

/*****************************************************************
** Function: sip_ccp_deleteAcceptContactParamAtIndexInAcceptContactHdr 
** Description: delete accept contact param at index in accept contact header
** Parameters:	
**				pHdr(IN/OUT)				- Sip accept contact Header 
**				index(IN)					- index at which accept contact param  is to be deleted
**				pErr(OUT)					- Possible error value (See API ref doc)
*****************************************************************/
extern SipBool sip_ccp_deleteAcceptContactParamAtIndexInAcceptContactHdr _ARGS_ ((\
		SipHeader *pHdr, SIP_U32bit index, SipError *pErr));

#endif

/**********************************************************************
**
** Function:  sip_ccp_getFeatureFromReqDispHdr
**
** Description: This function retrieves the feature field from a
**		SIP Request-Disposition pHeader structure
** Parameters:	
**				pHdr(IN)				- Sip Request Disposition Header 
**				ppFeature(OUT) 			- value of feature retrieved 
**				pErr(OUT)				- Possible error value (See API ref doc)
**********************************************************************/
extern SipBool sip_ccp_getFeatureFromReqDispHdr _ARGS_ ((SipHeader *pHdr,\
			 	SIP_S8bit **ppFeature, SipError *pErr ));

/**********************************************************************
**
** Function:  sip_ccp_setFeatureInReqDispHdr
**
** Description: This function sets the feature field in the SIP
**		Request-Disposition Header
** Parameters:	
**				pHdr(IN/OUT)			- Sip Request Disposition Header 
**				pFeature(IN) 			- value of feature to be set 
**				pErr(OUT)				- Possible error value (See API ref doc)
**********************************************************************/
extern SipBool sip_ccp_setFeatureInReqDispHdr _ARGS_ ((SipHeader *pHdr,\
				 SIP_S8bit *pFeature, SipError *pErr ));

/* Ensure Names are not mangled by C++ compilers */
#ifdef __cplusplus
}
#endif 

#endif
