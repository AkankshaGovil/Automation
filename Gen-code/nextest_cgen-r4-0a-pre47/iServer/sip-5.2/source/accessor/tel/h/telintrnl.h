/**************************************************************
 ** FUNCTION:
 **	 	This file has the prototype definitions for the tel-url
 **		internal functions
 *************************************************************
 **
 ** FILENAME:
 ** telapi.h
 **
 ** DESCRIPTION:
 **	
 **	 
 ** DATE			NAME			REFERENCE		REASON
 ** ----			----			--------		------
 ** 4Jan01 		    Rajasri			--				Initial Creation	
****************************************************************/

#ifndef __TEL_INT_H_
#define __TEL_INT_H_

#include "telstruct.h"
#include "teldecodeintrnl.h"

/* Ensure Names are not mangled by C++ compilers */
#ifdef __cplusplus
extern "C" {
#endif 

/* global variables used by parsers */
extern SipError 		glbSipParserErrorValue;
extern SIP_S8bit*		glbSipParserInput;

/*****************************************************************
** FUNCTION: sip_formTelGlobalNum
** DESCRIPTION: Converts TelGlobalNum to text
** Parameters:
** 		pGlobal(IN)    - The TelGlobalNum to be used
**		out(OUT)       - The output buffer 
**		pErr (OUT)		- Possible Error value (see API ref doc)
*****************************************************************/

extern SipBool sip_formTelGlobalNum _ARGS_ ((TelGlobalNum *pGlobal,\
		SIP_S8bit* out,SipError *pErr));


/*****************************************************************
** FUNCTION: sip_formTelLocalNum
** DESCRIPTION: Converts a list of TelLocalNum to text
** Parameters:
** 		pLocal(IN)    - The TelLocalNum to be used
**		out(OUT)       - The output buffer 
**		pErr (OUT)		- Possible Error value (see API ref doc)
*****************************************************************/

extern SipBool sip_formTelLocalNum _ARGS_ ((TelLocalNum *pLocal,\
		SIP_S8bit* out,SipError *pErr));


/*****************************************************************
** FUNCTION: sip_formSipStringList
** DESCRIPTION: Converts a list of String to text
** Parameters:
** 		out(OUT)    - output buffer
**		list(IN)       -  the string list to be converted
** 		seperator(IN)	- each element to be sepearated by
**		leadingsep(IN)	- leading seperator
**		pErr (OUT)		- Possible Error value (see API ref doc)
*****************************************************************/
extern SipBool sip_formSipStringList _ARGS_ ((SIP_S8bit *out,\
 SipList *list, SIP_S8bit *separator, SIP_U8bit leadingsep, SipError *err));

/*****************************************************************
** FUNCTION: sip_formEscapedSipParamList
** DESCRIPTION: Converts a list of Param to text with escaping certain
**                  charaters
** Parameters:
** 		out(OUT)    - output buffer
**		list(IN)       -  the param list to be converted
** 		seperator(IN)	- each element to be sepearated by
**		leadingsep(IN)	- leading seperator
**		pErr (OUT)	- Possible Error value (see API ref doc)
**
*****************************************************************/

extern SipBool sip_formEscapedSipParamList _ARGS_ ((SIP_S8bit *out, \
	SipList	*list,SIP_S8bit	*separator, SIP_U8bit leadingsep, \
	SipError *err));
 

/*********************************************************
** FUNCTION:__sip_cloneTelGlobalNum
**
** DESCRIPTION:  This function makes a deep copy of the fileds from 
**	the  TelGlobalNum structures "from" to "to".
**
**********************************************************/
extern SipBool __sip_cloneTelGlobalNum _ARGS_ ((TelGlobalNum *to, \
	TelGlobalNum *from, SipError *pErr));

/*********************************************************
** FUNCTION:__sip_cloneTelLocalNum
**
** DESCRIPTION:  This function makes a deep copy of the fileds from 
**	the  TelLocalNum structures "from" to "to".
**
**********************************************************/
extern SipBool __sip_cloneTelLocalNum _ARGS_ ((TelLocalNum *to, \
	TelLocalNum *from, SipError *pErr));

/*********************************************************
** FUNCTION:__sip_cloneTelUrl
**
** DESCRIPTION:  This function makes a deep copy of the fileds from 
**	the  TelUrl structures "from" to "to".
** Parameters:
**	to (OUT)		- TelUrl								
**	from (IN)		- TelUrl which has to be cloned
**	pErr (OUT)		- Possible Error value (see API ref doc)
**
**
**********************************************************/
extern SipBool __sip_cloneTelUrl _ARGS_((TelUrl *to, TelUrl *from, \
	SipError *pErr));

/*****************************************************************
** FUNCTION: sip_formTelGlobalNum
** DESCRIPTION: Converts TelGlobalNum to text
** Parameters:
** 		pGlobal(IN)    - The TelGlobalNum to be used
**		out(OUT)       - The output buffer 
**		pErr (OUT)		- Possible Error value (see API ref doc)
*****************************************************************/
extern SipBool sip_formTelGlobalNum(TelGlobalNum *pGlobal,SIP_S8bit* out,\
	SipError *pErr);

/*****************************************************************
** FUNCTION: sip_formTelLocalNum
** DESCRIPTION: Converts a list of TelLocalNum to text
** Parameters:
** 		pLocal(IN)    - The TelLocalNum to be used
**		out(OUT)       - The output buffer 
**		pErr (OUT)		- Possible Error value (see API ref doc)
*****************************************************************/
extern SipBool sip_formTelLocalNum(TelLocalNum *pLocal,SIP_S8bit* out,\
	SipError *pErr);


/* Ensure Names are not mangled by C++ compilers */
#ifdef __cplusplus
}
#endif

#endif /* __TEL_INT_H_ */
