
/* Ensure Names are not mangled by C++ compilers */
#ifdef __cplusplus
 extern "C" {
#endif
/**************************************************************
 ** FUNCTION:
 **	 	This file has the definitions for the tel-url
 **		APIs
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
 ** 5Jan01			Maria							Added GlobalNum APIs
 ** 7Jan01			Seshashayi					    Added LocalNum APIs
****************************************************************/

#ifndef __TEL_API_C_
#define __TEL_API_C_

#include "sipstruct.h"
#include "ctype.h"
#include "sipcommon.h"
#include "sipstatistics.h"
#include "telstruct.h"
#include "telapi.h"
#include "telintrnl.h"
#include "teldecodeintrnl.h"
#include "sipparserclone.h"
#include "portlayer.h"
#include "sipinit.h"
#include "telinit.h"
#include "telfree.h"

/******************************************************************************
** FUNCTION: sip_compareTelUri
**
** DESCRIPTION: This function is used to compare two Tel Address Specs 
*******************************************************************************/
SipBool  sip_compareTelUri
#ifdef ANSI_PROTO	
(TelUrl		 *pTel1, 
 TelUrl		 *pTel2,
 SipError *pError)
#else
(pTel1, pTel2, pError)
 TelUrl *pTel1;
 TelUrl *pTel2;
 SipError *pError;
#endif
{
		
	(void)pError; 

	/* Compare the Global field of the Tel-URL
	 */

	/* If the Global filed is present one of the tel-url structure and the
	 * other tel-url structure do not have Global filed 
	 * then return SipFail(failure)
	 * Otherwies if the Global field is presents in both the tel-url structure
	 * then Compare each filead of the Global Tel Url Structure
	 */
	if( pTel1->pGlobal != SIP_NULL )
	{
		SIP_U32bit dGlobalAreaSpecifier1=0,dGlobalAreaSpecifier2=0,i=0,j=0;
		
		if(pTel2->pGlobal == SIP_NULL )
			return SipFail;

		/* Compare the Base numerber of the Global phone number
		 */

		if( pTel1->pGlobal->pBaseNo == SIP_NULL )
		{
			if( pTel2->pGlobal->pBaseNo != SIP_NULL )
				return SipFail;
		}
		else
		{
			if( pTel2->pGlobal->pBaseNo == SIP_NULL )
				return SipFail;
		}
		if( ( pTel1->pGlobal->pBaseNo != SIP_NULL ) &&
			( pTel2->pGlobal->pBaseNo != SIP_NULL ))
		{

			if( strcmp( pTel1->pGlobal->pBaseNo, \
								pTel2->pGlobal->pBaseNo) != 0)
				return SipFail;
		}

		/*	Compare the Isdn part of the Global Phone number
		 */

		if( pTel1->pGlobal->pIsdnSubAddr == SIP_NULL )
		{
			if( pTel2->pGlobal->pIsdnSubAddr != SIP_NULL )
				return SipFail;
		}
		else
		{
			if( pTel2->pGlobal->pIsdnSubAddr == SIP_NULL )
				return SipFail;
		}

		if( ( pTel1->pGlobal->pIsdnSubAddr != SIP_NULL ) &&
			( pTel2->pGlobal->pIsdnSubAddr != SIP_NULL ))
		{
	
			if( strcmp( pTel1->pGlobal->pIsdnSubAddr, \
								pTel2->pGlobal->pIsdnSubAddr) != 0)
				return SipFail;
		}

		/* Compare the Post dial number of the Global tel-url structure
		 */

		if( pTel1->pGlobal->pPostDial == SIP_NULL )
		{
			if( pTel2->pGlobal->pPostDial != SIP_NULL )
				return SipFail;
		}
		else
		{
			if( pTel2->pGlobal->pPostDial == SIP_NULL )
				return SipFail;
		}

		if( ( pTel1->pGlobal->pPostDial != SIP_NULL ) &&
			( pTel2->pGlobal->pPostDial != SIP_NULL ))
		{
	
			if( strcmp( pTel1->pGlobal->pPostDial, \
								pTel2->pGlobal->pPostDial) != 0)
				return SipFail;
		}
		/* Compare the Area Specifier
		 */
		if(sip_getAreaSpecifierCountFromTelGlobalNum(pTel1->pGlobal,\
				&dGlobalAreaSpecifier1,pError) == SipFail)
		{
			return SipFail;
		}
		if(sip_getAreaSpecifierCountFromTelGlobalNum(pTel2->pGlobal,\
				&dGlobalAreaSpecifier2,pError) == SipFail)
		{
			return SipFail;
		}
		if(dGlobalAreaSpecifier2 != dGlobalAreaSpecifier1)
			return SipFail;
		
		/*Compare each of the area specifer
		 */

		for(i=0; i<dGlobalAreaSpecifier1;i++)
		{
			SIP_S8bit *pGloblaSpecifier1=SIP_NULL;
			SipBool	dAreaSpecifierMatch=SipFail;
			if(sip_getAreaSpecifierAtIndexFromTelGlobalNum(pTel1->pGlobal,\
				&pGloblaSpecifier1,i,pError)==SipFail)
			{
				return SipFail;
			}

			for(j=0; j<dGlobalAreaSpecifier2;j++)
			{
				SIP_S8bit *pGloblaSpecifier2=SIP_NULL;
				if(sip_getAreaSpecifierAtIndexFromTelGlobalNum(pTel2->pGlobal,\
					&pGloblaSpecifier2,j,pError)==SipFail)
				{
					return SipFail;
				}
				if(strcasecmp(pGloblaSpecifier2,pGloblaSpecifier1) ==0)
				{
					dAreaSpecifierMatch=SipSuccess;
					break;	
				}
			}
			if(dAreaSpecifierMatch !=SipSuccess)
				return SipFail;
		}

	}
	else 
	{	
		if( pTel2->pGlobal != SIP_NULL )
			return SipFail;
	}

	/* Compare the Local phone number part of the Tel-Url Structure
	 */

	/* If the Local filed is present one of the tel-url structure and the
	 * other tel-url structure do not have Local filed 
	 * then return SipFail(failure)
	 * Otherwies if the Local field is presents in both the tel-url structure
	 * then Compare each filead of the Global Tel Url Structure
	 */


	if( pTel1->pLocal != SIP_NULL )
	{
		SIP_U32bit dLocalAreaSpecifier1=0,dLocalAreaSpecifier2=0,i=0,j=0;

		if(pTel2->pLocal == SIP_NULL )
			return SipFail;

		/* Compare the Local Phone digit
		 */

		if( pTel1->pLocal->pLocalPhoneDigit == SIP_NULL )
		{
			if( pTel2->pLocal->pLocalPhoneDigit != SIP_NULL )
				return SipFail;
		}
		if( ( pTel1->pLocal->pLocalPhoneDigit != SIP_NULL ) &&
			( pTel2->pLocal->pLocalPhoneDigit != SIP_NULL ))
		{

			if( strcmp( pTel1->pLocal->pLocalPhoneDigit, \
								pTel2->pLocal->pLocalPhoneDigit) != 0)
				return SipFail;
		}

		/*	Compare the Isdn part of the Global Phone number
		 */

		if( pTel1->pLocal->pIsdnSubAddr == SIP_NULL )
		{
			if( pTel2->pLocal->pIsdnSubAddr != SIP_NULL )
				return SipFail;
		}

		if( ( pTel1->pLocal->pIsdnSubAddr != SIP_NULL ) &&
			( pTel2->pLocal->pIsdnSubAddr != SIP_NULL ))
		{
	
			if( strcmp( pTel1->pLocal->pIsdnSubAddr, \
								pTel2->pLocal->pIsdnSubAddr) != 0)
				return SipFail;
		}
		
		/* Compare the Post dial number of the Global tel-url structure
		 */

		if( pTel1->pLocal->pPostDial == SIP_NULL )
		{
			if( pTel2->pLocal->pPostDial != SIP_NULL )
				return SipFail;
		}

		if( ( pTel1->pLocal->pPostDial != SIP_NULL ) &&
			( pTel2->pLocal->pPostDial != SIP_NULL ))
		{
	
			if( strcmp( pTel1->pLocal->pPostDial, \
								pTel2->pLocal->pPostDial) != 0)
				return SipFail;
		}


		/* Compare the Area Specifier
		 */
		if(sip_getAreaSpecifierCountFromTelLocalNum(pTel1->pLocal,\
				&dLocalAreaSpecifier1,pError) == SipFail)
		{
			return SipFail;
		}
		if(sip_getAreaSpecifierCountFromTelLocalNum(pTel2->pLocal,\
				&dLocalAreaSpecifier2,pError) == SipFail)
		{
			return SipFail;
		}
		if(dLocalAreaSpecifier2 != dLocalAreaSpecifier1)
			return SipFail;
		
		/*Compare each of the area specifer
		 */

		for(i=0; i<dLocalAreaSpecifier1;i++)
		{
			SIP_S8bit *pGloblaSpecifier1=SIP_NULL;
			SipBool	dAreaSpecifierMatch=SipFail;
			if(sip_getAreaSpecifierAtIndexFromTelLocalNum(pTel1->pLocal,\
				&pGloblaSpecifier1,i,pError)==SipFail)
			{
				return SipFail;
			}

			for(j=0; j<dLocalAreaSpecifier2;j++)
			{
				SIP_S8bit *pGloblaSpecifier2=SIP_NULL;
				if(sip_getAreaSpecifierAtIndexFromTelLocalNum(pTel2->pLocal,\
					&pGloblaSpecifier2,j,pError)==SipFail)
				{
					return SipFail;
				}
				if(strcasecmp(pGloblaSpecifier2,pGloblaSpecifier1) ==0)
				{
					dAreaSpecifierMatch=SipSuccess;
					break;	
				}
			}
			if(dAreaSpecifierMatch !=SipSuccess)
				return SipFail;
		}

	}
	else 
	{	
		if( pTel2->pLocal != SIP_NULL )
			return SipFail;
	}
	return SipSuccess;
}


/***********************************************************************
** Function: sip_isTelUrl
** Description: Checks if the Addrspec has a tel-url
** Parameters:
**		pAddrSpec (IN)	- SipAddrSpec
**		pErr (OUT)		- Possible Error value (see API ref doc)
************************************************************************/
SipBool sip_isTelUrl
#ifdef ANSI_PROTO
	(SipAddrSpec *pAddrSpec, SipError *pErr)
#else
	(pAddrSpec,pErr)
	SipAddrSpec *pAddrSpec;
	SipError *pErr;
#endif
{
	SIP_S8bit *pTempUri;

	SipUrl *pTempUrl = SIP_NULL ;
	SIP_U32bit count =0;
	SIP_U32bit icount =0;
	SipParam* pTempParam = SIP_NULL ;
	SIP_S8bit* pTempValue = SIP_NULL ;
	en_AddrType pAddrtype;

	SIP_U32bit i;
	SIPDEBUGFN("Entering sip_isTelUrl\n");
#ifndef SIP_NO_CHECK
	if (pErr == SIP_NULL)
		return SipFail;
	if (pAddrSpec == SIP_NULL)
	{
		*pErr = E_INV_PARAM;
		return SipFail;
	}
#endif

	pAddrtype = pAddrSpec->dType;
	if ( pAddrtype == SipAddrReqUri )
	{
		pTempUri = pAddrSpec->u.pUri;

		i=0;
		if (( pTempUri[i] == 'T' || pTempUri[i] == 't') &&
			( pTempUri[i+1] == 'E' || pTempUri[i+1] == 'e')&&
			( pTempUri[i+2] == 'L' || pTempUri[i+2] == 'l'))
		{
			for (i=3;((pTempUri[i] == ' ') || (pTempUri[i] == '\t'))&& \
					(pTempUri[i] != '\0');i++);

			if (pTempUri[i] == ':')
			{
				*pErr = E_NO_ERROR;
				SIPDEBUGFN("Exiting sip_isTelUrl\n");
				return SipSuccess;
			}
		}
		SIPDEBUGFN("Exiting sip_isTelUrl\n");
		*pErr = E_NO_EXIST;
		return SipFail;
	}

	if((pAddrtype != SipAddrSipUri) && (pAddrtype != SipAddrSipSUri))
	{
		SIPDEBUGFN("Exiting sip_isTelUrl\n");
		*pErr = E_INV_PARAM;
		return SipFail;
	}

	pTempUrl = pAddrSpec->u.pSipUrl;

	/* get the count of URL param */
	if (sip_listSizeOf(&(pTempUrl->slParam), &count , pErr) == SipFail )
	{
		return SipFail;
	}

	while ( count > 0 )
	{
		/* get Url Param from the Url at index */
		if( sip_listGetAt(&(pTempUrl->slParam),count-1,\
				(SIP_Pvoid*)&pTempParam, pErr) == SipFail)
        {
			return SipFail;
        }

		if ( strcasecmp(pTempParam->pName,"user") == 0 )
		{
			sip_listSizeOf(&(pTempParam->slValue),&icount,pErr);
			if ( icount == 1 )
			{
				if ( sip_listGetAt(&(pTempParam->slValue),0,\
					(SIP_Pvoid*)&pTempValue,pErr) != SipSuccess )
				{
			/*		sip_freeSipParam(pTempParam);
			Fmr due to this  */
					*pErr = E_NO_EXIST;
					SIPDEBUGFN("Exiting sip_isTelUrl\n");
					return SipFail;
				}
				if ( strcasecmp(pTempValue,"phone") == 0 )
				{
			/*		sip_freeSipParam(pTempParam);
			Fmr due to this free. pTempParam is not allocated memory here */
					*pErr = E_NO_ERROR;
					SIPDEBUGFN("Exiting sip_isTelUrl\n");
					return SipSuccess;
				}
			}
		}
		count--;
	}

	SIPDEBUGFN("Exiting sip_isTelUrl\n");
	*pErr = E_NO_EXIST;
	return SipFail;
}

/*********************************************************
** FUNCTION:__sip_cloneTelLocalNum
**
** DESCRIPTION:  This function makes a deep copy of the fileds from
**	the  TelLocalNum structures "from" to "to".
**
**********************************************************/
SipBool __sip_cloneTelLocalNum
# ifdef ANSI_PROTO
	(TelLocalNum *to,
	TelLocalNum *from,
	SipError *pErr)
# else
	( to,from, pErr )
	TelLocalNum *to;
	TelLocalNum *from;
	SipError *pErr;
#endif
{
	SIP_U32bit dLength;
	SIPDEBUGFN( "Entering __sip_cloneTelLocalNum");

	if ((from==SIP_NULL)||(to==SIP_NULL))
	{
		*pErr = E_INV_PARAM;
		return SipFail;
	}

	/* clean up of to */

	if(to->pLocalPhoneDigit  != SIP_NULL )
	{
		if(sip_memfree(ACCESSOR_MEM_ID, (SIP_Pvoid*)(&(to->pLocalPhoneDigit)),\
			pErr) == SipFail)
			return SipFail;
	}

	if(to->pIsdnSubAddr  != SIP_NULL )
	{
		if(sip_memfree(ACCESSOR_MEM_ID, (SIP_Pvoid*)(&(to->pIsdnSubAddr)), \
			pErr) == SipFail)
			return SipFail;
	}

	if(to-> pPostDial != SIP_NULL )
	{
		if(sip_memfree(ACCESSOR_MEM_ID, (SIP_Pvoid*)(&(to->pPostDial)), pErr) \
			== SipFail)
			return SipFail;
	}

	if (sip_listDeleteAll(&(to->slParams), pErr) == SipFail)
		return SipFail;

	if (sip_listDeleteAll(&(to->slAreaSpecifier), pErr) == SipFail)
		return SipFail;


	/* clean up over */

	if(from->pLocalPhoneDigit== SIP_NULL)
	{
		to->pLocalPhoneDigit=SIP_NULL;
	}
	else
	{
		dLength = strlen(from->pLocalPhoneDigit );

		to->pLocalPhoneDigit=( SIP_S8bit * )fast_memget(ACCESSOR_MEM_ID,\
			dLength+1,pErr);
		if ( to->pLocalPhoneDigit == SIP_NULL )
		{
			*pErr = E_NO_MEM;
			return SipFail;
		}
		strcpy( to->pLocalPhoneDigit, from->pLocalPhoneDigit );
	}

	if(from->pIsdnSubAddr== SIP_NULL)
	{
		to->pIsdnSubAddr=SIP_NULL;
	}
	else
	{
		dLength = strlen(from->pIsdnSubAddr );

		to->pIsdnSubAddr=( SIP_S8bit * )fast_memget(\
			ACCESSOR_MEM_ID,dLength+1,pErr);
		if ( to->pIsdnSubAddr == SIP_NULL )
		{
			*pErr = E_NO_MEM;
			return SipFail;
		}
		strcpy( to->pIsdnSubAddr, from->pIsdnSubAddr );
	}

	if(from->pPostDial== SIP_NULL)
	{
		to->pPostDial=SIP_NULL;
	}
	else
	{
		dLength = strlen(from->pPostDial );

		to->pPostDial=( SIP_S8bit * )fast_memget(\
			ACCESSOR_MEM_ID,dLength+1,pErr);
		if ( to->pPostDial == SIP_NULL )
		{
			*pErr = E_NO_MEM;
			return SipFail;
		}
		strcpy( to->pPostDial, from->pPostDial );
	}

	if (__sipParser_cloneSipStringList(&(to->slAreaSpecifier), &(\
		from->slAreaSpecifier), pErr) == SipFail)
		return SipFail;

	if (__sipParser_cloneSipParamList (&(to->slParams), &(from->slParams), \
		pErr) == SipFail)
		return SipFail;

	SIPDEBUGFN( "Exitting __sip_cloneTelLocalNum");
	*pErr = E_NO_ERROR;
	return SipSuccess;
}


/*********************************************************
** FUNCTION:__sip_cloneTelGlobalNum
**
** DESCRIPTION:  This function makes a deep copy of the fileds from
**	the  TelGlobalNum structures "from" to "to".
**
**********************************************************/
SipBool __sip_cloneTelGlobalNum
# ifdef ANSI_PROTO
	(TelGlobalNum *to,
	TelGlobalNum *from,
	SipError *pErr)
# else
	( to,from, pErr )
	TelGlobalNum *to;
	TelGlobalNum *from;
	SipError *pErr;
#endif
{
	SIP_U32bit dLength;
	SIPDEBUGFN( "Entering __sip_cloneTelGlobalNum");

	if ((from==SIP_NULL)||(to==SIP_NULL))
	{
		*pErr = E_INV_PARAM;
		return SipFail;
	}

	/* clean up of to */

	if(to->pBaseNo  != SIP_NULL )
	{
		if(sip_memfree(ACCESSOR_MEM_ID, (SIP_Pvoid*)(&(to->pBaseNo)), pErr) \
			== SipFail)
			return SipFail;
	}

	if(to->pIsdnSubAddr  != SIP_NULL )
	{
		if(sip_memfree(ACCESSOR_MEM_ID, (SIP_Pvoid*)(&(to->pIsdnSubAddr)), \
			pErr) == SipFail)
			return SipFail;
	}

	if(to-> pPostDial != SIP_NULL )
	{
		if(sip_memfree(ACCESSOR_MEM_ID, (SIP_Pvoid*)(&(to->pPostDial)), pErr) \
			== SipFail)
			return SipFail;
	}

	if (sip_listDeleteAll(&(to->slParams), pErr) == SipFail)
		return SipFail;

	if (sip_listDeleteAll(&(to->slAreaSpecifier), pErr) == SipFail)
		return SipFail;


	/* clean up over */

	if(from->pBaseNo== SIP_NULL)
	{
		to->pBaseNo=SIP_NULL;
	}
	else
	{
		dLength = strlen(from->pBaseNo );

		to->pBaseNo=( SIP_S8bit * )fast_memget(ACCESSOR_MEM_ID,dLength+1,pErr);
		if ( to->pBaseNo == SIP_NULL )
		{
			*pErr = E_NO_MEM;
			return SipFail;
		}
		strcpy( to->pBaseNo, from->pBaseNo );
	}

	if(from->pIsdnSubAddr== SIP_NULL)
	{
		to->pIsdnSubAddr=SIP_NULL;
	}
	else
	{
		dLength = strlen(from->pIsdnSubAddr );

		to->pIsdnSubAddr=( SIP_S8bit * )fast_memget(ACCESSOR_MEM_ID,dLength+1,\
			pErr);
		if ( to->pIsdnSubAddr == SIP_NULL )
		{
			*pErr = E_NO_MEM;
			return SipFail;
		}
		strcpy( to->pIsdnSubAddr, from->pIsdnSubAddr );
	}

	if(from->pPostDial== SIP_NULL)
	{
		to->pPostDial=SIP_NULL;
	}
	else
	{
		dLength = strlen(from->pPostDial );

		to->pPostDial=( SIP_S8bit * )fast_memget(ACCESSOR_MEM_ID,dLength+1,\
			pErr);
		if ( to->pPostDial == SIP_NULL )
		{
			*pErr = E_NO_MEM;
			return SipFail;
		}
		strcpy( to->pPostDial, from->pPostDial );
	}

	if (__sipParser_cloneSipStringList(&(to->slAreaSpecifier), \
		&(from->slAreaSpecifier), pErr) == SipFail)
		return SipFail;

	if (__sipParser_cloneSipParamList (&(to->slParams), &(from->slParams), \
		pErr) == SipFail)
		return SipFail;

	SIPDEBUGFN( "Exitting __sip_cloneTelGlobalNum");
	*pErr = E_NO_ERROR;
	return SipSuccess;
}


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
SipBool __sip_cloneTelUrl
#ifdef ANSI_PROTO
	(TelUrl *to,
	TelUrl *from,
	SipError *pErr)
#else
	( to,from, pErr )
	TelUrl *to;
	TelUrl *from;
	SipError *pErr;
#endif
{

	SIPDEBUGFN("Entering function __sip_cloneTelUrl");
	if (from->pGlobal == SIP_NULL)
		to->pGlobal = SIP_NULL;
	else
	{
		if (sip_initTelGlobalNum(&(to->pGlobal),pErr) == SipFail)
			return SipFail;
		if (__sip_cloneTelGlobalNum(to->pGlobal,from->pGlobal,pErr) == SipFail)
			return SipFail;
	}
	if (from->pLocal == SIP_NULL)
		to->pLocal = SIP_NULL;
	else
	{
		if (sip_initTelLocalNum(&(to->pLocal),pErr) == SipFail)
			return SipFail;
		if (__sip_cloneTelLocalNum(to->pLocal,from->pLocal,pErr) == SipFail)
			return SipFail;
	}
	*pErr = E_NO_ERROR;
	SIPDEBUGFN("Exitting function __sip_cloneTelUrl");
	return SipSuccess;
}

/*********************************************************
** FUNCTION:sip_cloneTelUrl
**
** DESCRIPTION:  This function makes a deep copy of the fileds from
**	the  TelUrl structures "from" to "to".
** Parameters:
**	to (OUT)		- TelUrl
**	from (IN)		- TelUrl which has to be cloned
**	pErr (OUT)		- Possible Error value (see API ref doc)
**
**********************************************************/

SipBool sip_cloneTelUrl
#ifdef ANSI_PROTO
	(TelUrl *to,
	TelUrl *from,
	SipError *pErr)
#else
	( to,from, pErr )
	TelUrl *to;
	TelUrl *from;
	SipError *pErr;
#endif
{
	SIPDEBUGFN("Entering function sip_cloneTelUrl");
#ifndef SIP_NO_CHECK
	if (pErr == SIP_NULL)
		return SipFail;
	if ((to == SIP_NULL)||(from  == SIP_NULL))
	{
		*pErr = E_INV_PARAM;
		return SipFail;
	}
#endif
	if  (__sip_cloneTelUrl(to,from, pErr) == SipFail)
		return SipFail;

	*pErr = E_NO_ERROR;
	SIPDEBUGFN("Exitting function sip_cloneTelUrl");
	return SipSuccess;
}





/***********************************************************************
** Function: sip_getTelUrlFromAddrSpec
** Description: gets the TelUrl from the SipAddrSpec structure
** Parameters:
**		pAddrSpec (IN)	- SipAddrSpec
**		pTelUrl (OUT)	- retrieved TelUrl
**		pErr (OUT)		- Possible Error value (see API ref doc)
************************************************************************/
#ifdef ANSI_PROTO
#ifdef SIP_BY_REFERENCE
SipBool sip_getTelUrlFromAddrSpec (
	SipAddrSpec *pAddrSpec, TelUrl **ppTelUrl, SipError *pErr)
#else
SipBool sip_getTelUrlFromAddrSpec (
	SipAddrSpec *pAddrSpec, TelUrl *pTelUrl, SipError *pErr)
#endif
#else  /* ANSI_PROTO */
#ifdef SIP_BY_REFERENCE
SipBool sip_getTelUrlFromAddrSpec (
	pAddrSpec,ppTelUrl, pErr)
	SipAddrSpec *pAddrSpec;
	TelUrl **ppTelUrl;
	SipError *pErr;
#else
SipBool sip_getTelUrlFromAddrSpec (
	pAddrSpec, pTelUrl, pErr)
	SipAddrSpec *pAddrSpec;
	TelUrl *pTelUrl;
	SipError *pErr;
#endif
#endif /*ANSI_PROTO */
{
	SIP_S8bit *pParserBuffer;
	SIP_S32bit len;
	SipTelParserParam *pTelParserParam;
	SIP_S8bit *pSipParserInput;

	en_AddrType pAddrtype;

	INC_API_COUNT

	SIPDEBUGFN ("Entering sip_getTelUrlFromAddrSpec\n");
#ifndef SIP_NO_CHECK
	if (pErr == SIP_NULL)
		return SipFail;
	if (pAddrSpec == SIP_NULL)
	{
		*pErr = E_INV_PARAM;
		return SipFail;
	}
#ifndef SIP_BY_REFERENCE
	if ( pTelUrl == SIP_NULL)
	{
		*pErr = E_INV_PARAM;
		return SipFail;
	}
#else
	if ( ppTelUrl == SIP_NULL)
	{
		*pErr = E_INV_PARAM;
		return SipFail;
	}
#endif
	if (pAddrSpec->u.pUri == SIP_NULL)
	{
		*pErr = E_NO_EXIST;
		return SipFail;
	}
#endif
	if ( sip_isTelUrl(pAddrSpec, pErr) == SipFail)
	{
		*pErr = E_NO_EXIST;
		return SipFail;
	}

	pAddrtype = pAddrSpec->dType;
	if ( (pAddrtype == SipAddrSipUri) || (pAddrtype == SipAddrSipSUri) )
	{
		if ( pAddrSpec->u.pSipUrl->pUser == SIP_NULL )
		{
			*pErr = E_INV_TYPE;
			return SipFail;
		}
		len = strlen(pAddrSpec->u.pSipUrl->pUser);
		len = len +4;
	}
	else
	{
		len = strlen(pAddrSpec->u.pUri);
	}


	pTelParserParam = (SipTelParserParam*)fast_memget(ACCESSOR_MEM_ID,\
		sizeof(SipTelParserParam), pErr);
	if(pTelParserParam==SIP_NULL)
	{
		INC_ERROR_MEM
		*pErr = E_NO_MEM;
		return SipFail;
	}
	pTelParserParam->pError = (SipError*)fast_memget( \
		ACCESSOR_MEM_ID,sizeof(SipError),pErr);
	if(pTelParserParam->pError==SIP_NULL)
	{
		INC_ERROR_MEM
		*pErr = E_NO_MEM;
		return SipFail;
	}
	pParserBuffer = (SIP_S8bit *) fast_memget(ACCESSOR_MEM_ID, len+2, pErr);
	if(pParserBuffer == SIP_NULL)
	{
		INC_ERROR_MEM
		*pErr = E_NO_MEM;
		return SipFail;
	}
	if ( (pAddrtype == SipAddrSipUri) || (pAddrtype == SipAddrSipSUri) )
	{
		strcpy(pParserBuffer,"tel:" );
		strcat(pParserBuffer,pAddrSpec->u.pSipUrl->pUser);
		pParserBuffer[len+1]='\0';
	}
	else
	{
		strcpy(pParserBuffer,pAddrSpec->u.pUri );
		pParserBuffer[len+1]='\0';
	}
	*(pTelParserParam->pError) = E_NO_ERROR;
	if (sip_lex_Tel_scan_buffer(pParserBuffer, len+2) != 0)
	{
		INC_ERROR_MEM
		fast_memfree(ACCESSOR_MEM_ID,(pTelParserParam->pError),SIP_NULL);
		fast_memfree(ACCESSOR_MEM_ID,(pTelParserParam),SIP_NULL);
		fast_memfree(ACCESSOR_MEM_ID,pParserBuffer,SIP_NULL);
		*pErr = E_NO_MEM;
		return SipFail;
	}
	sip_lex_Tel_reset_state();
	pSipParserInput = pParserBuffer;
	pTelParserParam->pTelUrl = SIP_NULL;
	glbSipParserTelparse((void *)pTelParserParam);
	sip_lex_Tel_release_buffer();
	fast_memfree(ACCESSOR_MEM_ID,pParserBuffer,SIP_NULL);

	if (*(pTelParserParam->pError) != E_NO_ERROR)
	{
#ifdef SIP_STATISTICS
		INC_SECOND_ERROR_PROTOCOL
#endif

		*pErr = E_PARSER_ERROR;
		fast_memfree(ACCESSOR_MEM_ID,(pTelParserParam->pError),SIP_NULL);
		fast_memfree(ACCESSOR_MEM_ID,(pTelParserParam),SIP_NULL);
		return SipFail;
	}
#ifdef SIP_STATISTICS
	INC_SECOND_API_REQ_PARSED
#endif

#ifdef SIP_BY_REFERENCE
	*ppTelUrl = pTelParserParam->pTelUrl;
#else
	if (sip_cloneTelUrl(pTelUrl, pTelParserParam->pTelUrl, pErr) == SipFail)
	{
		sip_freeTelUrl(pTelParserParam->pTelUrl);
		fast_memfree(ACCESSOR_MEM_ID,(pTelParserParam->pError),SIP_NULL);
		fast_memfree(ACCESSOR_MEM_ID,(pTelParserParam),SIP_NULL);
		return SipFail;
	}
	sip_freeTelUrl(pTelParserParam->pTelUrl);
#endif

	fast_memfree(ACCESSOR_MEM_ID,(pTelParserParam->pError),SIP_NULL);
	fast_memfree(ACCESSOR_MEM_ID,(pTelParserParam),SIP_NULL);
	SIPDEBUGFN ("Exiting sip_getTelUrlFromAddrSpec\n");
	*pErr = E_NO_ERROR;
	return SipSuccess;
}

/***********************************************************************
** Function: sip_setTelUrlInAddrSpec
** Description: sets the TelUrl in the SipAddrSpec structure
** Parameters:
**		pAddrSpec (IN)	- SipAddrSpec
**		pTelUrl (OUT)	- TelUrl to be set
**		pErr (OUT)		- Possible Error value (see API ref doc)
************************************************************************/
SipBool sip_setTelUrlInAddrSpec
#ifdef ANSI_PROTO
	(SipAddrSpec *pAddrSpec, TelUrl *pTelUrl, SipError *pErr)
#else
	(pAddrSpec, pTelUrl, pErr)
	SipAddrSpec *pAddrSpec;
	TelUrl *pTelUrl;
	SipError *pErr;
#endif
{
	SIP_S8bit *out;
	en_AddrType pAddrtype;

	SIPDEBUGFN("Entering sip_setTelUrlInAddrSpec\n");
#ifndef SIP_NO_CHECK
	if (pErr == SIP_NULL)
		return SipFail;
	if (pAddrSpec == SIP_NULL)
	{
		*pErr = E_INV_PARAM;
		return SipFail;
	}
#endif
	
	pAddrtype = pAddrSpec->dType;
	if ( (pAddrtype == SipAddrSipUri) || (pAddrtype == SipAddrSipSUri) )
	{
		if ( pAddrSpec->u.pSipUrl != SIP_NULL )
		{
			sip_freeSipUrl((pAddrSpec->u).pSipUrl);
		}
	}

	if ( pAddrtype == SipAddrReqUri )
	{
		if ( pAddrSpec->u.pUri != SIP_NULL)
		{
			if(sip_memfree(ACCESSOR_MEM_ID, (SIP_Pvoid*)&(pAddrSpec->u.pUri),\
				pErr) ==SipFail)
				return SipFail;
		}
		if (pTelUrl == SIP_NULL)
		{
			pAddrSpec->u.pUri = SIP_NULL;
			*pErr = E_NO_ERROR;
			return SipSuccess;
		}
	}

	pAddrSpec->dType = SipAddrReqUri;
	out=(SIP_S8bit *)fast_memget(ACCESSOR_MEM_ID,MAX_TEL_URL_SIZE, pErr);
	if (out == SIP_NULL)
	{
		*pErr = E_NO_MEM;
		return SipFail;
	}
	strcpy(out,"Tel:");
	if (pTelUrl->pGlobal != SIP_NULL)
		sip_formTelGlobalNum(pTelUrl->pGlobal,(char *)out,pErr);
	else if (pTelUrl->pLocal != SIP_NULL)
		sip_formTelLocalNum(pTelUrl->pLocal,(char *)out,pErr);
	else {
		fast_memfree(ACCESSOR_MEM_ID, out, pErr);
		*pErr = E_INV_PARAM;
		return SipFail;
	}
	pAddrSpec->u.pUri=out;

	SIPDEBUGFN("Exitting sip_setTelUrlInAddrSpec\n");
	return SipSuccess;
}

/*****************************************************************
** FUNCTION: sip_formTelGlobalNum
** DESCRIPTION: Converts TelGlobalNum to text
** Parameters:
** 		pGlobal(IN)    - The TelGlobalNum to be used
**		out(OUT)       - The output buffer
**		pErr (OUT)		- Possible Error value (see API ref doc)
*****************************************************************/

SipBool sip_formTelGlobalNum(TelGlobalNum *pGlobal,SIP_S8bit* out,SipError *pErr)
{
	SIP_S8bit* tempStr;
	STRCAT(out,"+");
	STRCAT(out,pGlobal->pBaseNo);
	if(pGlobal->pIsdnSubAddr != SIP_NULL)
		STRCAT(out,";isub=");
	STRCAT(out,pGlobal->pIsdnSubAddr);

	if(pGlobal->pPostDial !=SIP_NULL)
	{
		STRCAT(out,";postd=");

		/* escape here for post dial */
		tempStr=escapeCharacters(pGlobal->pPostDial,pErr);
		STRCAT(out,tempStr);
		fast_memfree(0,tempStr,pErr);
	}

	/* extract each element of pGlobal->slAreaSpecifier and escape */
	sip_formSipStringList(out, &(pGlobal->slAreaSpecifier), (SIP_S8bit *) \
		";",1, pErr);
	/* extract each element of pGlobal->slParams and escape */
	sip_formEscapedSipParamList(out, &(pGlobal->slParams), (SIP_S8bit *) ";",1, pErr);
	return SipSuccess;

}

/*****************************************************************
** FUNCTION: sip_formTelLocalNum
** DESCRIPTION: Converts a list of TelLocalNum to text
** Parameters:
** 		pLocal(IN)    - The TelLocalNum to be used
**		out(OUT)       - The output buffer
**		pErr (OUT)		- Possible Error value (see API ref doc)
*****************************************************************/

SipBool sip_formTelLocalNum(TelLocalNum *pLocal,SIP_S8bit* out,SipError *pErr)
{
	SIP_S8bit* tempStr;
	STRCAT(out,pLocal->pLocalPhoneDigit);
	if(pLocal->pIsdnSubAddr != SIP_NULL)
		STRCAT(out,";isub=");
	STRCAT(out,pLocal->pIsdnSubAddr);
	/* escape here for post dial */
	if(pLocal->pPostDial !=SIP_NULL)
	{
		STRCAT(out,";postd=");
		tempStr=escapeCharacters(pLocal->pPostDial,pErr);
		STRCAT(out,tempStr);
		fast_memfree(0,tempStr,pErr);

	}
	/* extract each element of pGlobal->slAreaSpecifier and escape */
	sip_formSipStringList(out, &(pLocal->slAreaSpecifier), (SIP_S8bit *) \
		";",1, pErr);
	/* extract each element of pGlobal->slParams and escape */
	sip_formEscapedSipParamList(out, &(pLocal->slParams), (SIP_S8bit *) ";",1, pErr);
	return SipSuccess;
}

/*****************************************************************
** FUNCTION: sip_formEscapedSipParamList
** DESCRIPTION: Converts a list of Param to text with escaping certain
**                  charaters
** Parameters:
** 		out(OUT)    - output buffer
**		list(IN)       -  the param list to be converted
** 		seperator(IN)	- each element to be sepearated by
**		leadingsep(IN)	- leading seperator
**		pErr (OUT)		- Possible Error value (see API ref doc)
**
** DESCRIPTION: Converts a list of SipParams to text
*****************************************************************/

SipBool sip_formEscapedSipParamList
#ifdef ANSI_PROTO
	(SIP_S8bit 	*out,
	SipList 	*list,
	SIP_S8bit 	*separator,
	SIP_U8bit	leadingsep,
	SipError 	*err)
#else
	(out, list, separator, leadingsep, err)
	SIP_S8bit *out;
	SipList *list;
	SIP_S8bit *separator;
	SIP_U8bit leadingsep;
	SipError *err;
#endif
{
		SIP_U32bit listSize,listIter;
		SIP_S8bit* tempStr;
		sip_listSizeOf( list, &listSize, err);
		listIter = 0;
		while (listIter < listSize)
		{
			SipParam *pParam;
			SIP_U32bit valueSize;

			sip_listGetAt (list, listIter, (SIP_Pvoid *) &pParam, err);
			if((listIter!=0)||(leadingsep!=0))
				STRCAT ( (char *)out, separator);
			tempStr=escapeCharacters(pParam->pName,err);
			STRCAT(out,tempStr);
			fast_memfree(0,tempStr,err);
			sip_listSizeOf( &(pParam->slValue), &valueSize, err);
			if ( valueSize>=1)
			{
				SIP_S8bit *value;
				SIP_U32bit valueIter=0;
				STRCAT((char *) out,"=");
				while(valueIter < valueSize)
				{
					if(valueIter>0)
						STRCAT((char *) out,",");
					sip_listGetAt (&(pParam->slValue), valueIter, \
						(SIP_Pvoid *) &value, err);
					if(!((strcasecmp(pParam->pName,"tsp")==0)&&\
						(value[0]=='[')&&(value[strlen(value)-1]==']')))
					{
						tempStr=escapeCharacters(value,err);
						STRCAT(out,tempStr);
						fast_memfree(0,tempStr,err);
					}
					else
					{
						STRCAT(out,value);
					}


					valueIter++;
				}
			}
			listIter++;
		} /* while */
		return SipSuccess;
}


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

SipBool sip_formSipStringList
#ifdef ANSI_PROTO
	(SIP_S8bit 	*out,
	SipList 	*list,
	SIP_S8bit 	*separator,
	SIP_U8bit	leadingsep,
	SipError 	*pErr)
#else
	(out, list, separator, leadingsep, pErr)
	SIP_S8bit *out;
	SipList *list;
	SIP_S8bit *separator;
	SIP_U8bit leadingsep;
	SipError *pErr;
#endif
{
		SIP_U32bit listSize,listIter;
		SIP_S8bit* tempStr;
		sip_listSizeOf( list, &listSize, pErr);
		listIter = 0;
		while (listIter < listSize)
		{
			SIP_S8bit *pString;
			sip_listGetAt (list, listIter, (SIP_Pvoid *) &pString, pErr);
			if((listIter!=0)||(leadingsep!=0))
				STRCAT ((char *)out, separator);
			STRCAT((char *) out,"phone-context=");
			tempStr=escapeCharacters(pString,pErr);
			STRCAT(out,tempStr);
			fast_memfree(0,tempStr,pErr);
			listIter++;
		} /* while */
		return SipSuccess;
}

/***********************************************************************
** Function: sip_getGlobalNumFromTelUrl
** Description: gets the global number from the TelUrl structure
** Parameters:
**		pUrl (IN)		- TelUrl
**		ppLocal (OUT)	- retrieved global number
**		pErr (OUT)		- Possible Error value (see API ref doc)
************************************************************************/
SipBool sip_getGlobalNumFromTelUrl
#ifdef ANSI_PROTO
#ifdef SIP_BY_REFERENCE
	(TelUrl *pUrl,
	TelGlobalNum **ppGlobal,
	SipError *pErr)
#else
	(TelUrl *pUrl,
	TelGlobalNum *pGlobal,
	SipError *pErr)
#endif
#else
#ifdef SIP_BY_REFERENCE
	(pUrl,ppGlobal,pErr)
	TelUrl *pUrl;
	TelGlobalNum **ppGlobal;
	SipError *pErr;
#else
	(pUrl,pGlobal,pErr)
	TelUrl *pUrl;
	TelGlobalNum *pGlobal;
	SipError *pErr;
#endif
#endif
{
	SIPDEBUGFN("Entering sip_getGlobalNumFromTelUrl\n");
#ifndef SIP_NO_CHECK
	if (pErr == SIP_NULL)
		return SipFail;
	if ( pUrl == SIP_NULL)
	{
		*pErr = E_INV_PARAM;
		return SipFail;
	}
#ifndef SIP_BY_REFERENCE
	if ( pGlobal == SIP_NULL)
	{
		*pErr = E_INV_PARAM;
		return SipFail;
	}
#else
	if ( ppGlobal == SIP_NULL)
	{
		*pErr = E_INV_PARAM;
		return SipFail;
	}
#endif

#endif

	if (pUrl->pGlobal == SIP_NULL)
	{
		*pErr=E_NO_EXIST;
		return SipFail;
	}
#ifdef SIP_BY_REFERENCE
	HSS_LOCKEDINCREF(pUrl->pGlobal->dRefCount);
	*ppGlobal = pUrl->pGlobal;
#else
	if (__sip_cloneTelGlobalNum(pGlobal,pUrl->pGlobal,pErr) == SipFail)
	{
		sip_freeTelGlobalNum(pGlobal);
		return SipFail;
	}
#endif
	SIPDEBUGFN("Exiting sip_getGlobalNumFromTelUrl\n");
	*pErr = E_NO_ERROR;
	return SipSuccess;
}

/***********************************************************************
** Function: sip_setGlobalNumInTelUrl
** Description: sets the global number in the TelUrl structure
** Parameters:
**		pUrl (IN/OUT)	- TelUrl
**		pGlobal (OUT)	- Global number to be set
**		pErr (OUT)		- Possible Error value (see API ref doc)
************************************************************************/
SipBool sip_setGlobalNumInTelUrl
#ifdef ANSI_PROTO
	(TelUrl *pUrl,
	TelGlobalNum *pGlobal,
	SipError *pErr)
#else
	(pUrl,pGlobal,pErr)
	TelUrl *pUrl;
	TelGlobalNum *pGlobal;
	SipError *pErr;
#endif
{
#ifndef SIP_BY_REFERENCE
	TelGlobalNum *pTempGlobal;
#endif
	SIPDEBUGFN("Entering sip_setGlobalNumInTelUrl\n");
#ifndef SIP_NO_CHECK
	if (pErr == SIP_NULL)
		return SipFail;
	if ( pUrl == SIP_NULL)
	{
		*pErr = E_INV_PARAM;
		return SipFail;
	}
#endif
	if (pUrl->pGlobal != SIP_NULL)
		sip_freeTelGlobalNum(pUrl->pGlobal);
	if (pGlobal==SIP_NULL)
	{
		pUrl->pGlobal=SIP_NULL;
	}
	else
	{	
#ifdef SIP_BY_REFERENCE
		HSS_LOCKEDINCREF(pGlobal->dRefCount);
		pUrl->pGlobal=pGlobal;
#else
		if (sip_initTelGlobalNum(&pTempGlobal,pErr) == SipFail)
			return SipFail;
		if (__sip_cloneTelGlobalNum(pTempGlobal,pGlobal,pErr) == SipFail)
			return SipFail;
		pUrl->pGlobal=pTempGlobal;
#endif
	}	
	SIPDEBUGFN("Exitting sip_setGlobalNumInTelUrl\n");
	*pErr = E_NO_ERROR;
	return SipSuccess;
}

/***********************************************************************
** Function: sip_getLocalNumFromTelUrl
** Description: gets the local url from the TelUrl structure
** Parameters:
**		pUrl (IN)		- TelUrl
**		ppLocal (OUT)	- retrieved local number
**		pErr (OUT)		- Possible Error value (see API ref doc)
************************************************************************/
SipBool sip_getLocalNumFromTelUrl
#ifdef ANSI_PROTO
#ifdef SIP_BY_REFERENCE
	(TelUrl *pUrl,
	TelLocalNum **ppLocal,
	SipError *pErr)
#else
	(TelUrl *pUrl,
	TelLocalNum *pLocal,
	SipError *pErr)
#endif
#else
#ifdef SIP_BY_REFERENCE
	(pUrl,ppLocal,pErr)
	TelUrl *pUrl;
	TelLocalNum **ppLocal;
	SipError *pErr;
#else
	(pUrl,pLocal,pErr)
	TelUrl *pUrl;
	TelLocalNum *pLocal;
	SipError *pErr;
#endif
#endif
{
	SIPDEBUGFN("Entering sip_getLocalNumFromTelUrl\n");
#ifndef SIP_NO_CHECK
	if (pErr == SIP_NULL)
		return SipFail;
	if ( pUrl == SIP_NULL)
	{
		*pErr = E_INV_PARAM;
		return SipFail;
	}
#ifndef SIP_BY_REFERENCE
	if ( pLocal == SIP_NULL)
	{
		*pErr = E_INV_PARAM;
		return SipFail;
	}
#else
	if ( ppLocal == SIP_NULL)
	{
		*pErr = E_INV_PARAM;
		return SipFail;
	}
#endif
#endif
	if (pUrl->pLocal == SIP_NULL)
	{
		*pErr=E_NO_EXIST;
		return SipFail;
	}

#ifdef SIP_BY_REFERENCE
	HSS_LOCKEDINCREF(pUrl->pLocal->dRefCount);
	*ppLocal = pUrl->pLocal;
#else
	if (__sip_cloneTelLocalNum(pLocal,pUrl->pLocal,pErr) == SipFail)
	{
		sip_freeTelLocalNum(pLocal);
		return SipFail;
	}
#endif
	SIPDEBUGFN("Exiting sip_getLocalNumFromTelUrl\n");
	*pErr = E_NO_ERROR;
	return SipSuccess;
}


/***********************************************************************
** Function: sip_setLocalNumInTelUrl
** Description: sets the local url from the TelUrl structure
** Parameters:
**		pUrl (IN/OUT)	- TelUrl
**		pLocal (OUT)	- local number to be set
**		pErr (OUT)		- Possible Error value (see API ref doc)
************************************************************************/
SipBool sip_setLocalNumInTelUrl
#ifdef ANSI_PROTO
	(TelUrl *pUrl,
	TelLocalNum *pLocal,
	SipError *pErr)
#else
	(pUrl,pLocal,pErr)
	TelUrl *pUrl;
	TelLocalNum *pLocal;
	SipError *pErr;
#endif
{
#ifndef SIP_BY_REFERENCE
	TelLocalNum *pTempLocal;
#endif
	SIPDEBUGFN("Entering sip_setLocalNumInTelUrl\n");
#ifndef SIP_NO_CHECK
	if (pErr == SIP_NULL)
		return SipFail;
	if ( pUrl == SIP_NULL)
	{
		*pErr = E_INV_PARAM;
		return SipFail;
	}
#endif
	if (pUrl->pLocal != SIP_NULL)
		sip_freeTelLocalNum(pUrl->pLocal);
	if (pLocal==SIP_NULL)
	{
		pUrl->pLocal=SIP_NULL;
	}
	else
	{	
#ifdef SIP_BY_REFERENCE
		HSS_LOCKEDINCREF(pLocal->dRefCount);
		pUrl->pLocal=pLocal;
#else
		if (sip_initTelLocalNum(&pTempLocal,pErr) == SipFail)
			return SipFail;
		if (__sip_cloneTelLocalNum(pTempLocal,pLocal,pErr) == SipFail)
			return SipFail;
		pUrl->pLocal=pTempLocal;
#endif
	}	
	SIPDEBUGFN("Exitting sip_setLocalNumInTelUrl\n");
	*pErr = E_NO_ERROR;
	return SipSuccess;
}


/**********************************/
/*apis for TelLocalNum*/
/*******************************/
/***********************************************************************
** Function: sip_getLocalPhoneDigitFromTelLocalNum
** Description: gets the LocalPhoneDigits from the TelLocalNum structure
** Parameters:
**		pLocal (IN)		- TelLocalNum
**		ppPhone (OUT)	- retrieved local phone digit
**		pErr (OUT)		- Possible Error value (see API ref doc)
************************************************************************/
SipBool sip_getLocalPhoneDigitFromTelLocalNum
#ifdef ANSI_PROTO
	(TelLocalNum *pLocal,
	SIP_S8bit **ppPhone,
	SipError *pErr)
#else
	(pLocal, ppPhone, pErr)
	TelLocalNum *pLocal;
	SIP_S8bit **ppPhone;
	SipError *pErr;
#endif
{
#ifndef SIP_BY_REFERENCE
	SIP_U16bit dLength;
#endif
	SIP_S8bit * pTemp_PhoneDigit;
	SIPDEBUGFN( "Entering getLocalPhoneDigitFromTelLocalNum");
#ifndef SIP_NO_CHECK
	if( pErr == SIP_NULL )
		return SipFail;

	if (( pLocal == SIP_NULL) || ( ppPhone == SIP_NULL))
	{
		*pErr = E_INV_PARAM;
		return SipFail;
	}
#endif

	pTemp_PhoneDigit = pLocal->pLocalPhoneDigit;
	if( pTemp_PhoneDigit == SIP_NULL )
	{
		*pErr = E_NO_EXIST;
		return SipFail;
	}
#ifdef SIP_BY_REFERENCE
	*ppPhone = pTemp_PhoneDigit;
#else
	dLength = strlen(pTemp_PhoneDigit);
	*ppPhone = ( SIP_S8bit * )fast_memget(ACCESSOR_MEM_ID,dLength+1,pErr);
	if ( *ppPhone == SIP_NULL )
		return SipFail;

	strcpy(*ppPhone ,pTemp_PhoneDigit );
#endif
	SIPDEBUGFN( "Exiting getLocalPhoneDigitFromTelLocalNum");
	*pErr = E_NO_ERROR;
	return SipSuccess;
}



/***********************************************************************
** Function: sip_setLocalPhoneDigitInTelLocalNum
** Description: sets the LocalPhoneDigits in the TelLocalNum structure
** Parameters:
**		pLocal (IN/OUT)		- TelLocalNum
**		pPhoneNum (IN)		- LocalPhoneDigit to be set
**		pErr (OUT)			- Possible Error value (see API ref doc)
************************************************************************/
SipBool sip_setLocalPhoneDigitInTelLocalNum
#ifdef ANSI_PROTO
	( TelLocalNum *pLocal,
	SIP_S8bit *pPhoneNum,
	SipError *pErr  )
#else
	( pLocal,pPhoneNum,pErr )
	TelLocalNum *pLocal;
	SIP_S8bit *pPhoneNum;
	SipError *pErr;
#endif
{
#ifndef SIP_BY_REFERENCE
	SIP_U32bit dLength;
	SIP_S8bit *pTemp_PhoneDigit;
#endif
	SIPDEBUGFN( "Entering setLocalPhoneDigitInTelLocalNum");
#ifndef SIP_NO_CHECK
	if( pErr == SIP_NULL )
		return SipFail;
	if ( pLocal == SIP_NULL)
	{
		*pErr = E_INV_PARAM;
		return SipFail;
	}
#endif
	if(pLocal->pLocalPhoneDigit !=SIP_NULL)
	{
		if(sip_memfree(ACCESSOR_MEM_ID,(SIP_Pvoid*)&(\
			pLocal->pLocalPhoneDigit),pErr) ==SipFail)
			return SipFail;
	}
#ifdef SIP_BY_REFERENCE
	pLocal->pLocalPhoneDigit = pPhoneNum;
#else
	if( pPhoneNum == SIP_NULL)
		pTemp_PhoneDigit = SIP_NULL;
	else
	{
		dLength = strlen( pPhoneNum );
		pTemp_PhoneDigit = ( SIP_S8bit * )fast_memget(ACCESSOR_MEM_ID,\
			dLength+1,pErr);
		if ( pTemp_PhoneDigit == SIP_NULL )
			return SipFail;

		strcpy(pTemp_PhoneDigit ,pPhoneNum  );
	}

	pLocal->pLocalPhoneDigit = pTemp_PhoneDigit;
#endif
	SIPDEBUGFN( "Exitting setLocalPhoneDigitInTelLocalNum");
	*pErr = E_NO_ERROR;
	return SipSuccess;
}


/***********************************************************************
** Function: sip_getIsdnNoFromTelLocalNum
** Description: gets the isdn sub address from the TelLocalNum structure
** Parameters:
**		pLocal (IN)		- TelLocalNum
**		ppIsdnno (OUT)	- retrieved isdn sub address
**		pErr (OUT)		- Possible Error value (see API ref doc)
************************************************************************/
SipBool sip_getIsdnNoFromTelLocalNum
#ifdef ANSI_PROTO
	(TelLocalNum *pLocal,
	SIP_S8bit **ppIsdnno,
	SipError *pErr)
#else
	(pLocal, ppIsdnno, pErr)
	TelLocalNum *pLocal;
	SIP_S8bit **ppIsdnno;
	SipError *pErr;
#endif
{
#ifndef SIP_BY_REFERENCE
	SIP_U16bit dLength;
#endif
	SIP_S8bit * pTemp_IsdnNo;
	SIPDEBUGFN( "Entering getIsdnNoFromTelLocalNum");
#ifndef SIP_NO_CHECK
	if( pErr == SIP_NULL )
		return SipFail;
	if (( pLocal == SIP_NULL) || (ppIsdnno  == SIP_NULL))
	{
		*pErr = E_INV_PARAM;
		return SipFail;
	}
#endif

	pTemp_IsdnNo = pLocal->pIsdnSubAddr;
	if( pTemp_IsdnNo == SIP_NULL )
	{
		*pErr = E_NO_EXIST;
		return SipFail;
	}
#ifdef SIP_BY_REFERENCE
	*ppIsdnno =pTemp_IsdnNo ;
#else
	dLength = strlen(pTemp_IsdnNo);
	*ppIsdnno = ( SIP_S8bit * )fast_memget(ACCESSOR_MEM_ID,dLength+1,pErr);
	if ( *ppIsdnno == SIP_NULL )
		return SipFail;

	strcpy(*ppIsdnno ,pTemp_IsdnNo );
#endif
	SIPDEBUGFN( "Exiting getLocalPhoneDigitFromTelLocalNum");
	*pErr = E_NO_ERROR;
	return SipSuccess;
}



/***********************************************************************
** Function: sip_setIsdnNoInTelLocalNum
** Description: sets the isdn no in the TelLocalNum structure
** Parameters:
**		pLocal (IN/OUT)		- TelLocalNum
**		pIsdnno (IN)	- isdn no to be set
**		pErr (OUT)		- Possible Error value (see API ref doc)
************************************************************************/
SipBool sip_setIsdnNoInTelLocalNum
#ifdef ANSI_PROTO
	( TelLocalNum *pLocal,
	SIP_S8bit *pIsdnno,
	SipError *pErr  )
#else
	( pLocal,pIsdnno,pErr )
	TelLocalNum *pLocal;
	SIP_S8bit *pIsdnno;
	SipError *pErr;
#endif
{
#ifndef SIP_BY_REFERENCE
	SIP_U32bit dLength;
	SIP_S8bit *pTemp_IsdnNo;
#endif
	SIPDEBUGFN( "Entering setIsdnNoInTelLocalNum");
#ifndef SIP_NO_CHECK
	if( pErr == SIP_NULL )
		return SipFail;
	if ( pLocal == SIP_NULL)
	{
		*pErr = E_INV_PARAM;
		return SipFail;
	}
#endif
	if(pLocal->pIsdnSubAddr !=SIP_NULL)
	{
		if(sip_memfree(ACCESSOR_MEM_ID,(SIP_Pvoid*)&(pLocal->pIsdnSubAddr),\
			pErr) ==SipFail)
			return SipFail;
	}
#ifdef SIP_BY_REFERENCE
	pLocal->pIsdnSubAddr = pIsdnno;
#else
	if( pIsdnno == SIP_NULL)
		pTemp_IsdnNo = SIP_NULL;
	else
	{
		dLength = strlen( pIsdnno );
		pTemp_IsdnNo = ( SIP_S8bit * )fast_memget(ACCESSOR_MEM_ID,\
			dLength+1,pErr);
		if ( pTemp_IsdnNo == SIP_NULL )
			return SipFail;

		strcpy(pTemp_IsdnNo , pIsdnno );
	}

	pLocal->pIsdnSubAddr = pTemp_IsdnNo ;
#endif
	SIPDEBUGFN( "Exitting setIsdnNoInTelLocalNum");
	*pErr = E_NO_ERROR;
	return SipSuccess;
}


/***********************************************************************
** Function: sip_getPostdialFromTelLocalNum
** Description: gets the post dial from the TelLocaLNum structure
** Parameters:
**		pLocal (IN)		- TelLocalNum
**		ppPostd (OUT)	- retreived post dial field
**		pErr (OUT)		- Possible Error value (see API ref doc)
************************************************************************/
SipBool sip_getPostdialFromTelLocalNum
#ifdef ANSI_PROTO
	(TelLocalNum *pLocal,
	SIP_S8bit **ppPostd,
	SipError *pErr)
#else
	(pLocal, ppPostd, pErr)
	TelLocalNum *pLocal;
	SIP_S8bit **ppPostd;
	SipError *pErr;
#endif
{
#ifndef SIP_BY_REFERENCE
	SIP_U16bit dLength;
#endif
	SIP_S8bit * pTemp_Postd;
	SIPDEBUGFN( "Entering getIsdnNoFromTelLocalNum");
#ifndef SIP_NO_CHECK
	if( pErr == SIP_NULL )
		return SipFail;
	if (( pLocal == SIP_NULL) || (ppPostd == SIP_NULL) )
	{
		*pErr = E_INV_PARAM;
		return SipFail;
	}
#endif

	pTemp_Postd = pLocal->pPostDial;
	if( pTemp_Postd == SIP_NULL )
	{
		*pErr = E_NO_EXIST;
		return SipFail;
	}
#ifdef SIP_BY_REFERENCE
	*ppPostd =pTemp_Postd ;
#else
	dLength = strlen(pTemp_Postd);
	*ppPostd = ( SIP_S8bit * )fast_memget(ACCESSOR_MEM_ID,dLength+1,pErr);
	if ( *ppPostd == SIP_NULL )
		return SipFail;

	strcpy(*ppPostd ,pTemp_Postd );
#endif
	SIPDEBUGFN( "Exiting getIsdnNoFromTelLocalNum ");
	*pErr = E_NO_ERROR;
	return SipSuccess;
}


/***********************************************************************
** Function: sip_setPostdialInTelLocalNum
** Description: sets the post dial in   the TelLocalNum structure
** Parameters:
**		pLocal (IN/OUT)		- TelLocalNum
**		ppPostd (OUT)	- retreived post dial field
**		pErr (OUT)		- Possible Error value (see API ref doc)
************************************************************************/
SipBool sip_setPostdialInTelLocalNum
#ifdef ANSI_PROTO
	( TelLocalNum *pLocal,
	SIP_S8bit *pPostd,
	SipError *pErr )
#else
	( pLocal,pPostd,pErr )
	TelLocalNum *pLocal;
	SIP_S8bit *pPostd;
	SipError *pErr;
#endif
{
#ifndef SIP_BY_REFERENCE
	SIP_U32bit dLength;
	SIP_S8bit *pTemp_pPostd;
#endif
	SIPDEBUGFN( "Entering sip_setPostdialInTelLocalNum ");
#ifndef SIP_NO_CHECK
	if( pErr == SIP_NULL )
		return SipFail;
	if ( pLocal == SIP_NULL)
	{
		*pErr = E_INV_PARAM;
		return SipFail;
	}
#endif
	if(pLocal->pPostDial !=SIP_NULL)
	{
		if(sip_memfree(ACCESSOR_MEM_ID,(SIP_Pvoid*)&(pLocal->pPostDial),pErr) \
			==SipFail)
			return SipFail;
	}
#ifdef SIP_BY_REFERENCE
	pLocal->pPostDial = pPostd;
#else
	if( pPostd == SIP_NULL)
		pTemp_pPostd = SIP_NULL;
	else
	{
		dLength = strlen( pPostd );
		pTemp_pPostd = ( SIP_S8bit * )fast_memget(ACCESSOR_MEM_ID,dLength+1,\
			pErr);
		if ( pTemp_pPostd == SIP_NULL )
			return SipFail;

		strcpy( pTemp_pPostd , pPostd );
	}

	pLocal->pPostDial = pTemp_pPostd ;
#endif
	SIPDEBUGFN( "Exitting sip_setPostdialInTelLocalNum");
	*pErr = E_NO_ERROR;
	return SipSuccess;
}

/***********************************************************************
** Function: sip_getParamCountFromTelLocalNum
** Description: gets the number of parameters in TelLocalNum
**             (service provider and future extension are treated as parameters)
** Parameters:
**		pLocal (IN)	- TelLocalNum
**		pCount (OUT)	- number of parameters
**		pErr (OUT)		- Possible Error value (see API ref doc)
************************************************************************/
SipBool sip_getParamCountFromTelLocalNum
#ifdef ANSI_PROTO
	(TelLocalNum *pLocal,SIP_U32bit *pCount,SipError *pErr)
#else
 	(pLocal,pCount,pErr)
	TelLocalNum *pLocal;
 	SIP_U32bit *pCount;
 	SipError *pErr;
#endif
{
	SIPDEBUGFN("Entering function sip_getParamCountFromTelLocalNum");
#ifndef SIP_NO_CHECK
	if( pErr == SIP_NULL )
		return SipFail;

	if ( pLocal == SIP_NULL )
	{
		*pErr = E_INV_PARAM;
		return SipFail;
	}

	if (pCount == SIP_NULL)
	{
		*pErr = E_INV_PARAM;
		return SipFail;
	}
#endif
	if (sip_listSizeOf( &(pLocal)->slParams, pCount , pErr) == SipFail )
	{
		return SipFail;
	}

	*pErr = E_NO_ERROR;
	SIPDEBUGFN("Exitting function sip_getParamCountFromTelLocalNum");
	return SipSuccess;
}




/*****************************************************************************
** Function: sip_getParamAtIndexInTelLocalNum
** Description: gets the Param at the specified index in TelLocalNum
**             (service provider and future extension are treated as parameters)
** Parameters:
**	pLocal (IN)	- TelLocalNum
**	ppParam(OUT)	- retreived Parameter
**	dIndex (IN)		- index at which param is to be retieved
**	pErr (OUT)		- Possible Error value (see API ref doc)
******************************************************************************/
SipBool sip_getParamAtIndexFromTelLocalNum
#ifdef ANSI_PROTO
#ifdef SIP_BY_REFERENCE
	(TelLocalNum *pLocal,SipParam **ppParam, SIP_U32bit dIndex,\
		SipError *pErr)
#else
	(TelLocalNum *pLocal,SipParam *pParam, SIP_U32bit dIndex, \
		SipError *pErr)
#endif
#else
#ifdef SIP_BY_REFERENCE
	(pLocal,ppParam, dIndex, pErr)
	TelLocalNum *pLocal;
	SipParam **ppParam;
	SIP_U32bit dIndex;
	SipError *pErr;
#else
	(pLocal,pParam, dIndex, pErr)
	TelLocalNum *pLocal;
	SipParam *pParam;
	SIP_U32bit dIndex;
	SipError *pErr;
#endif
#endif
{
	SIP_Pvoid element_from_list;
	SipParam *temp_param;
	SIPDEBUGFN("Entering function sip_getParamAtIndexFromTelLocalNum");
#ifndef SIP_NO_CHECK
	if( pErr == SIP_NULL )
		return SipFail;
#ifndef SIP_BY_REFERENCE
	if ( pParam == SIP_NULL )
	{
		*pErr = E_INV_PARAM;
		return SipFail;
	}

#else
	if ( ppParam == SIP_NULL )
	{
		*pErr = E_INV_PARAM;
		return SipFail;
	}

#endif
	if ( pLocal == SIP_NULL )
	{
		*pErr = E_INV_PARAM;
		return SipFail;
	}

#endif
#ifndef SIP_BY_REFERENCE
	if (pParam == SIP_NULL)
#else
	if (ppParam == SIP_NULL)
#endif
	{
		*pErr = E_INV_PARAM;
		return SipFail;
	}

	if (sip_listGetAt( &(pLocal->slParams), dIndex,  \
		&element_from_list, pErr) == SipFail)
		return SipFail;

	if (element_from_list == SIP_NULL)
	{
		*pErr = E_NO_EXIST;
		return SipFail;
	}

	temp_param = (SipParam *)element_from_list;
#ifndef SIP_BY_REFERENCE
	if (__sip_cloneSipParam(pParam, temp_param, pErr) == SipFail)
		return SipFail;
#else
	HSS_LOCKEDINCREF(temp_param->dRefCount);
	*ppParam = temp_param;
#endif

	*pErr = E_NO_ERROR;
	SIPDEBUGFN("Exitting function sip_getParamAtIndexFromTelLocalNum");
	return SipSuccess;
}


/*****************************************************************************
** Function: sip_setParamAtIndexInTelLocalNum
** Description: sets the Param at the specified index in TelLocalNum
**             (service provider and future extension are treated as parameters)
** Parameters:
**	pLocal (IN/OUT)	- TelLocalNum
**	pParam(IN)			- Param to be set
**	dIndex (IN)			- index at which param is set in Tel Local number
**	pErr (OUT)			- Possible Error value (see API ref doc)
******************************************************************************/
SipBool sip_setParamAtIndexInTelLocalNum
#ifdef ANSI_PROTO
	(TelLocalNum *pLocal,SipParam *pParam, SIP_U32bit dIndex, \
		SipError *pErr)
#else
	(pLocal,pParam, dIndex, pErr)
	TelLocalNum *pLocal;
	SipParam *pParam;
	SIP_U32bit dIndex;
	SipError *pErr;
#endif
{
	SipParam *temp_param;
	SIPDEBUGFN("Entering function sip_setParamAtIndexInTelLocalNum");
#ifndef SIP_NO_CHECK
	if( pErr == SIP_NULL )
		return SipFail;

	if ( pLocal == SIP_NULL )
	{
		*pErr = E_INV_PARAM;
		return SipFail;
	}
#endif
	if ( pParam == SIP_NULL )
		temp_param = SIP_NULL;
	else
	{
#ifndef SIP_BY_REFERENCE
		if (sip_initSipParam(&temp_param, pErr) == SipFail)
			return SipFail;
		if (__sip_cloneSipParam(temp_param, pParam, pErr) == SipFail)
		{
			sip_freeSipParam (temp_param);
			return SipFail;
		}
#else
		HSS_LOCKEDINCREF(pParam->dRefCount);
		temp_param = pParam;
#endif
	}

	if( sip_listSetAt( &(pLocal->slParams),  \
		dIndex, (SIP_Pvoid)(temp_param), pErr) == SipFail)
	{
		if (temp_param != SIP_NULL)
#ifndef SIP_BY_REFERENCE
			sip_freeSipParam (temp_param);
#else
		HSS_LOCKEDDECREF(pParam->dRefCount);
#endif
		return SipFail;
	}

	*pErr = E_NO_ERROR;
	SIPDEBUGFN("Exitting function sip_setParamAtIndexInTelLocalNum");
	return SipSuccess;
}




/******************************************************************************
** Function: sip_insertParamAtIndexInTelLocalNum
** Description: inserts the Param at the specified index in TelLocalNum
**             (service provider and future extension are treated as parameters)
** Parameters:
**	pLocal (IN/OUT)- TelLocalNum
**	pParam(IN)		- Param to be inserted
**	dIndex (IN)		- index at which param is inserted in Tel Local number
**	pErr (OUT)		- Possible Error value (see API ref doc)
*******************************************************************************/
SipBool sip_insertParamAtIndexInTelLocalNum
#ifdef ANSI_PROTO
	(TelLocalNum *pLocal, SipParam *pParam, SIP_U32bit dIndex, \
		SipError *pErr)
#else
	(pLocal, pParam, dIndex, pErr)
	TelLocalNum *pLocal;
	SipParam *pParam;
	SIP_U32bit dIndex;
	SipError *pErr;
#endif
{
	SipParam *temp_param;
	SIPDEBUGFN("Entering function sip_insertParamAtIndexInTelLocalNum");
#ifndef SIP_NO_CHECK
	if( pErr == SIP_NULL )
		return SipFail;

	if ( pLocal == SIP_NULL )
	{
		*pErr = E_INV_PARAM;
		return SipFail;
	}

#endif
	if ( pParam == SIP_NULL )
		temp_param = SIP_NULL;
	else
	{
#ifndef SIP_BY_REFERENCE
		if (sip_initSipParam(&temp_param, pErr) == SipFail)
			return SipFail;
		if (__sip_cloneSipParam(temp_param, pParam, pErr) == SipFail)
		{
			sip_freeSipParam (temp_param);
			return SipFail;
		}
#else
		HSS_LOCKEDINCREF(pParam->dRefCount);
		temp_param = pParam;
#endif
	}

	if( sip_listInsertAt( &(pLocal->slParams),  \
		dIndex, (SIP_Pvoid)(temp_param), pErr) == SipFail)
	{
		if (temp_param != SIP_NULL)
#ifndef SIP_BY_REFERENCE
			sip_freeSipParam (temp_param);
#else
		HSS_LOCKEDDECREF(pParam->dRefCount);
#endif
		return SipFail;
	}

	*pErr = E_NO_ERROR;
	SIPDEBUGFN("Exitting function sip_insertParamAtIndexInTelLocalNum");
	return SipSuccess;
}



/***********************************************************************
** Function: sip_deleteParamAtIndexInTelLocalNum
** Description: deletes the param at the specified index in TelLocalNum
**             (service provider and future extension are treated as parameters)
** Parameters:
**	pLocal (IN)	- TelLocalNum
**	dIndex (IN)		- index at which param is deleted in Tel Local number
**	pErr (OUT)		- Possible Error value (see API ref doc)
************************************************************************/
SipBool sip_deleteParamAtIndexInTelLocalNum
#ifdef ANSI_PROTO
(TelLocalNum *pLocal, SIP_U32bit dIndex, SipError *pErr)
#else
(pLocal, dIndex, pErr)
TelLocalNum *pLocal;
SIP_U32bit dIndex;
SipError *pErr;
#endif
{
	SIPDEBUGFN("Entering function sip_deleteParamAtIndexInTelLocalNum");
#ifndef SIP_NO_CHECK
	if( pErr == SIP_NULL )
		return SipFail;

	if ( pLocal == SIP_NULL )
	{
		*pErr = E_INV_PARAM;
		return SipFail;
	}

#endif
	if( sip_listDeleteAt( &(pLocal->slParams), dIndex, pErr) == SipFail)
		return SipFail;

	*pErr = E_NO_ERROR;
	SIPDEBUGFN("Exitting function sip_deleteParamAtIndexInTelLocalNum");
	return SipSuccess;
}


/***************************************************************
** Function: sip_getAreaSpecifierCountFromTelLocalNum
** Description: gets the scheme from the TelUrl structure
** Parameters:
**	pLocal (IN)	- TelLocalNum
**	pCount (OUT)	- number of Tel AreaSpecifiers
**	pErr (OUT)		- Possible Error value (see API ref doc)
************************************************************************/

SipBool sip_getAreaSpecifierCountFromTelLocalNum
#ifdef ANSI_PROTO
	(TelLocalNum *pLocal,
	SIP_U32bit *pCount,
	SipError *pErr)
#else
	(pLocal, pCount, pErr)
	TelLocalNum *pLocal;
	SIP_U32bit *pCount;
	SipError *pErr;
#endif
{
	SIPDEBUGFN ( "Entering getAreaSpecifierCountFromTelLocalNum");
#ifndef SIP_NO_CHECK
	if ( pErr == SIP_NULL )
		return SipFail;

	if ( (pLocal == SIP_NULL) || ( pCount == SIP_NULL ))
	{
		*pErr = E_INV_PARAM;
		return SipFail;
	}
#endif
	if (sip_listSizeOf(&(pLocal->slAreaSpecifier), pCount , pErr) == SipFail )
	{
		return SipFail;
	}

	SIPDEBUGFN ( "Exitting getAreaSpecifierCountFromTelLocalNum");
	*pErr = E_NO_ERROR;
	return SipSuccess;
}

/***********************************************************************
** Function: sip_getAreaSpecifierAtIndexFromTelLocalNum
** Description: gets the AreaSpecifier field at the index from TelLocalNum
** Parameters:
**	pLocal (IN)			- TelLocalNum
**	ppAreaSpecifier(OUT)- retreived AreaSpecifier
**	dIndex (IN)			- index at which AreaSpecifier is to be retrieved
**	pErr (OUT)			- Possible Error value (see API ref doc)
************************************************************************/

SipBool sip_getAreaSpecifierAtIndexFromTelLocalNum
#ifdef ANSI_PROTO
	(TelLocalNum *pLocal,
	 SIP_S8bit **ppAreaSpecifier,
	 SIP_U32bit 	index,
	 SipError 	*pErr )
#else
	(pLocal,ppAreaSpecifier,index,pErr)
	 TelLocalNum *pLocal;
	 SIP_S8bit **ppAreaSpecifier;
  	 SIP_U32bit index;
	 SipError *pErr;
#endif
{
	SIP_Pvoid pElementFromList;
#ifndef SIP_BY_REFERENCE
	SIP_U32bit size;
#endif

	SIPDEBUGFN ( "Entering sip_getAreaSpecifierAtIndexFromTelLocalNum ");
#ifndef SIP_NO_CHECK
	if( pErr == SIP_NULL )
		return SipFail;

	if ( (pLocal == SIP_NULL) || (ppAreaSpecifier == SIP_NULL) )
	{
		*pErr = E_INV_PARAM;
		return SipFail;
	}
#endif
	if( sip_listGetAt(&(pLocal->slAreaSpecifier), index, &pElementFromList, \
		pErr) == SipFail)
		return SipFail;

	if ( pElementFromList == SIP_NULL )
	{
		*pErr = E_NO_EXIST;
		return SipFail;
	}

#ifdef SIP_BY_REFERENCE
	*ppAreaSpecifier = (SIP_S8bit*)pElementFromList;
#else
	size = strlen( (SIP_S8bit * )pElementFromList);
	*ppAreaSpecifier = ( SIP_S8bit * )fast_memget(ACCESSOR_MEM_ID, size +1, \
		pErr);
	if(*ppAreaSpecifier == SIP_NULL)
		return SipFail;

	strcpy(*ppAreaSpecifier, (SIP_S8bit*)pElementFromList);
#endif
	SIPDEBUGFN ( "Exitting sip_getAreaSpecifierAtIndexFromTelLocalNum");
	*pErr = E_NO_ERROR;
	return SipSuccess;
}

/***********************************************************************
** Function: sip_setAreaSpecifierAtIndexFromTelLocalNum
** Description: sets the AreaSpecifier field at the index in TelLocalNum
** Parameters:
**	pLocal (IN/OUT)		- TelLocalNum
**	pAreaSpecifier(IN)	- AreaSpecifier to be set
**	dIndex (IN)			- index at which AreaSpecifier is to be set
**	pErr (OUT)			- Possible Error value (see API ref doc)
************************************************************************/

SipBool  sip_setAreaSpecifierAtIndexInTelLocalNum
#ifdef ANSI_PROTO
	(TelLocalNum *pLocal,
	 SIP_S8bit *pAreaSpecifier,
	 SIP_U32bit 	index,
	 SipError 	*pErr )
#else
	(pLocal,pAreaSpecifier,index,pErr)
	 TelLocalNum *pLocal;
	 SIP_S8bit *pAreaSpecifier;
  	 SIP_U32bit index;
	 SipError *pErr;
#endif
{

	SIP_S8bit * pElementFromList;
	SipError tempErr;		/* used in freeing memory after an pError has happened */

	SIPDEBUGFN ( "Entering sip_setAreaSpecifierAtIndexInTelLocalNum");
#ifndef SIP_NO_CHECK
	if( pErr == SIP_NULL )
		return SipFail;

	if (pLocal == SIP_NULL)
	{
		*pErr = E_INV_PARAM;
		return SipFail;
	}
#endif
	if (pAreaSpecifier  == SIP_NULL )
		pElementFromList = SIP_NULL;
	else
	{
#ifdef SIP_BY_REFERENCE
		pElementFromList = pAreaSpecifier;
#else

		pElementFromList = ( SIP_S8bit * )fast_memget(ACCESSOR_MEM_ID,\
			strlen(pAreaSpecifier) + 1, pErr);
		if( pElementFromList == SIP_NULL )
			return SipFail;

		strcpy(pElementFromList,pAreaSpecifier );
#endif
	}

	if( sip_listSetAt(&(pLocal->slAreaSpecifier), index, pElementFromList, \
		pErr) == SipFail)
	{
		if ( pElementFromList != SIP_NULL )
			sip_memfree(ACCESSOR_MEM_ID, (SIP_Pvoid *)(&(pElementFromList)), \
				&tempErr);
		return SipFail;
	}

	SIPDEBUGFN ( "Exitting sip_setAreaSpecifierAtIndexInTelLocalNum");
	*pErr = E_NO_ERROR;
	return SipSuccess;
}

/***********************************************************************
** Function: sip_insertAreaSpecifierAtIndexInTelLocalNum
** Description: inserts the AreaSpecifier field at the index in TelLocalNum
** Parameters:
**	pLocal (IN/OUT)		- TelLocalNum
**	pAreaSpecifier(IN)	- AreaSpecifier to be inserted
**	dIndex (IN)			- index at which AreaSpecifier is to be inserted
**	pErr (OUT)		- Possible Error value (see API ref doc)
************************************************************************/

SipBool sip_insertAreaSpecifierAtIndexInTelLocalNum
#ifdef ANSI_PROTO
	(TelLocalNum *pLocal,
	 SIP_S8bit *pAreaSpecifier,
	 SIP_U32bit 	index,
	 SipError 	*pErr )
#else
	(pLocal,pAreaSpecifier,index,pErr)
	 TelLocalNum *pLocal;
	 SIP_S8bit *pAreaSpecifier;
    	 SIP_U32bit index;
	 SipError *pErr;
#endif
{

	SIP_S8bit * pElementFromList;
	SipError tempErr;		/* used in freeing memory after an pError has happened */


	SIPDEBUGFN ( "Entering sip_insertAreaSpecifierAtIndexInTelLocalNum");
#ifndef SIP_NO_CHECK
	if( pErr == SIP_NULL )
		return SipFail;

	if ( pLocal == SIP_NULL )
	{
		*pErr = E_INV_PARAM;
		return SipFail;
	}
#endif
	/* copying the Value Headers structure/char*  */
	if (pAreaSpecifier  == SIP_NULL )
		pElementFromList = SIP_NULL;
	else
	{
#ifdef SIP_BY_REFERENCE
		pElementFromList = pAreaSpecifier	;
#else
		pElementFromList = ( SIP_S8bit * )fast_memget(ACCESSOR_MEM_ID,\
			strlen(pAreaSpecifier) + 1, pErr);
		if( pElementFromList == SIP_NULL )
			return SipFail;

		strcpy(pElementFromList,pAreaSpecifier );
#endif
	}

	if( sip_listInsertAt(&(pLocal->slAreaSpecifier), index, pElementFromList, \
		pErr) == SipFail)
	{
		if ( pElementFromList != SIP_NULL )
			sip_memfree(ACCESSOR_MEM_ID, (SIP_Pvoid *)(&(pElementFromList)), \
				&tempErr);
		return SipFail;
	}

	SIPDEBUGFN ( "Exitting sip_insertAreaSpecifierAtIndexInTelLocalNum");

	*pErr = E_NO_ERROR;
	return SipSuccess;
}

/***********************************************************************
** Function: sip_deleteAreaSpecifierAtIndexInTelLocalNum
** Description: deletes the AreaSpecifier field at the index in TelLocalNum
** Parameters:
**	pLocal (IN/OUT)		- TelLocalNum
**	pAreaSpecifier(IN)	- AreaSpecifier to be deleted
**	dIndex (IN)			- index at which AreaSpecifier is to be deleted
**	pErr (OUT)			- Possible Error value (see API ref doc)
************************************************************************/

SipBool sip_deleteAreaSpecifierAtIndexInTelLocalNum
#ifdef ANSI_PROTO
	(TelLocalNum *pLocal,
	  SIP_U32bit 	index,
	  SipError 	*pErr )
#else
	(pLocal,index,pErr)
   	 TelLocalNum *pLocal;
	SIP_U32bit 	index;
	SipError 	*pErr;
#endif
{

	SIPDEBUGFN ( "Entering sip_deleteAreaSpecifierAtIndexInTelLocalNum");
#ifndef SIP_NO_CHECK
	if( pErr == SIP_NULL )
		return SipFail;

	if (pLocal  == SIP_NULL )
	{
		*pErr = E_INV_PARAM;
		return SipFail;
	}
#endif
	if( sip_listDeleteAt(&(pLocal->slAreaSpecifier), index, pErr) == SipFail)
		return SipFail;

	SIPDEBUGFN ( "Exitting sip_deleteAreaSpecifierAtIndexInTelLocalNum");

	*pErr = E_NO_ERROR;
	return SipSuccess;
}




/**********************************/
/*apis for TelGlobalNum*/
/*******************************/
/***********************************************************************
** Function: sip_getBaseNoFromTelGlobalNum
** Description: gets the  base number from TelGlobalNum structure
** Parameters:
**		pGlobal	(IN)- TelGlobalNum
**		ppBase (IN)		- retreived Base number
**		pErr (OUT)		- Possible Error value (see API ref doc)
************************************************************************/
SipBool sip_getBaseNoFromTelGlobalNum
#ifdef ANSI_PROTO
(TelGlobalNum *pGlobal,SIP_S8bit **ppBase, SipError *pErr )
#else
(pGlobal,ppBase, pErr )
TelGlobalNum *pGlobal;
SIP_S8bit **ppBase;
SipError *pErr;
#endif
{
#ifndef SIP_BY_REFERENCE
	SIP_U16bit dLength;
#endif
	SIP_S8bit * pTemp_base;
	SIPDEBUGFN( "Entering getBaseNoFromTelGlobalNum");
#ifndef SIP_NO_CHECK
	if( pErr == SIP_NULL )
		return SipFail;

	if (( pGlobal == SIP_NULL) || ( ppBase == SIP_NULL))
	{
		*pErr = E_INV_PARAM;
		return SipFail;
	}
#endif
	pTemp_base = pGlobal->pBaseNo;


	if( pTemp_base == SIP_NULL )
	{
		*pErr = E_NO_EXIST;
		return SipFail;
	}
#ifdef SIP_BY_REFERENCE
	*ppBase = pTemp_base;
#else

	dLength = strlen(pTemp_base);
	*ppBase = ( SIP_S8bit * )fast_memget(ACCESSOR_MEM_ID,dLength+1,pErr);
	if ( *ppBase == SIP_NULL )
		return SipFail;

	strcpy( *ppBase, pTemp_base );
#endif

	SIPDEBUGFN( "Exitting getBaseNoFromTelGlobalNum");
	*pErr = E_NO_ERROR;
	return SipSuccess;

}

/***********************************************************************
** Function: sip_setBaseNoInTelGlobalNum
** Description: sets the base number in TelGlobalNum structure
** Parameters:
**		pGlobal	(IN/OUT)- TelGlobalNum
**		pBase (IN)		- Base number to set
**		pErr (OUT)		- Possible Error value (see API ref doc)
************************************************************************/
SipBool sip_setBaseNoInTelGlobalNum
#ifdef ANSI_PROTO
	(TelGlobalNum *pGlobal,SIP_S8bit *pBase, SipError *pErr)
#else
	(pGlobal,pBase, pErr)
	TelGlobalNum *pGlobal;
	SIP_S8bit *pBase;
	SipError *pErr;
#endif
{
#ifndef SIP_BY_REFERENCE
	SIP_U32bit dLength;
	SIP_S8bit * pTemp_base;
#endif
	SIPDEBUGFN( "Entering setBaseNoInTelGlobalNum");
#ifndef SIP_NO_CHECK
	if( pErr == SIP_NULL )
		return SipFail;

	if ( pGlobal == SIP_NULL)
	{
		*pErr = E_INV_PARAM;
		return SipFail;
	}
#endif
	if(pGlobal->pBaseNo  !=SIP_NULL)
	{
		if(sip_memfree(ACCESSOR_MEM_ID,(SIP_Pvoid*)&(pGlobal->pBaseNo),pErr) \
			==SipFail)
		return SipFail;
	}
#ifdef SIP_BY_REFERENCE
	pGlobal->pBaseNo = pBase;
#else
	if( pBase == SIP_NULL)
		pTemp_base = SIP_NULL;
	else
	{
		dLength = strlen( pBase);
		pTemp_base = ( SIP_S8bit * )fast_memget(ACCESSOR_MEM_ID,dLength+1,pErr);
		if ( pTemp_base == SIP_NULL )
			return SipFail;

		strcpy( pTemp_base, pBase);
	}
	pGlobal->pBaseNo= pTemp_base;
#endif
	SIPDEBUGFN( "Exitting setBaseNoInTelGlobalNum");
	*pErr = E_NO_ERROR;
	return SipSuccess;
}

/***********************************************************************
** Function: sip_getIsdnNoFromTelGlobalNum
** Description: gets the isdn number from TelGlobalNum
** Parameters:
**		pGlobal	(IN)	- TelGlobalNum
**		ppIsdnno (IN)	- retreived isdn number
**		pErr (OUT)		- Possible Error value (see API ref doc)
************************************************************************/
SipBool sip_getIsdnNoFromTelGlobalNum
#ifdef ANSI_PROTO
	(TelGlobalNum *pGlobal,SIP_S8bit **ppIsdnno, SipError *pErr)
#else
	(pGlobal,ppIsdnno,pErr)
	TelGlobalNum *pGlobal;
	SIP_S8bit **ppIsdnno;
	SipError *pErr;
#endif
{
#ifndef SIP_BY_REFERENCE
	SIP_U16bit dLength;
#endif
	SIP_S8bit * pTemp_isdnno;
	SIPDEBUGFN( "Entering getIsdnNoFromTelGlobalNum");
#ifndef SIP_NO_CHECK
	if( pErr == SIP_NULL )
		return SipFail;

	if (( pGlobal == SIP_NULL) || ( ppIsdnno  == SIP_NULL))
	{
		*pErr = E_INV_PARAM;
		return SipFail;
	}
#endif
	pTemp_isdnno = pGlobal->pIsdnSubAddr;


	if( pTemp_isdnno == SIP_NULL )
	{
		*pErr = E_NO_EXIST;
		return SipFail;
	}
#ifdef SIP_BY_REFERENCE
	*ppIsdnno = pTemp_isdnno;
#else

	dLength = strlen(pTemp_isdnno);
	*ppIsdnno = ( SIP_S8bit * )fast_memget(ACCESSOR_MEM_ID,dLength+1,pErr);
	if ( *ppIsdnno == SIP_NULL )
		return SipFail;

	strcpy( *ppIsdnno, pTemp_isdnno );
#endif

	SIPDEBUGFN( "Exitting getIsdnNoFromTelGlobalNum");
	*pErr = E_NO_ERROR;
	return SipSuccess;

}



/***********************************************************************
** Function: sip_setIsdnNoInTelGlobalNum
** Description: sets the isdn number in TelGlobalNum
** Parameters:
**		pGlobal (IN/OUT)- TelGlobalNum
**		pIsdnno(IN)		- Isdn number to set
**		pErr (OUT)		- Possible Error value (see API ref doc)
************************************************************************/
SipBool sip_setIsdnNoInTelGlobalNum
#ifdef ANSI_PROTO
	(TelGlobalNum *pGlobal,SIP_S8bit *pIsdnno, SipError *pErr)
#else
	(pGlobal,pIsdnno, pErr)
	TelGlobalNum *pGlobal;
	SIP_S8bit *pIsdnno;
	SipError *pErr;
#endif
{
#ifndef SIP_BY_REFERENCE
	SIP_U32bit dLength;
	SIP_S8bit * pTemp_isdnno;
#endif
	SIPDEBUGFN( "Entering setIsdnNoInTelGlobalNum");
#ifndef SIP_NO_CHECK
	if( pErr == SIP_NULL )
		return SipFail;

	if ( pGlobal == SIP_NULL)
	{
		*pErr = E_INV_PARAM;
		return SipFail;
	}
#endif
	if(pGlobal->pIsdnSubAddr  !=SIP_NULL)
	{
		if(sip_memfree(ACCESSOR_MEM_ID,(SIP_Pvoid*)&(pGlobal->pIsdnSubAddr),\
			pErr) ==SipFail)
			return SipFail;
	}
#ifdef SIP_BY_REFERENCE
	pGlobal->pIsdnSubAddr = pIsdnno;
#else
	if( pIsdnno == SIP_NULL)
		pTemp_isdnno = SIP_NULL;
	else
	{
		dLength = strlen( pIsdnno);
		pTemp_isdnno = ( SIP_S8bit * )fast_memget(ACCESSOR_MEM_ID,dLength+1,\
			pErr);
		if ( pTemp_isdnno == SIP_NULL )
			return SipFail;

		strcpy( pTemp_isdnno, pIsdnno);
	}
	pGlobal->pIsdnSubAddr= pTemp_isdnno;
#endif
	SIPDEBUGFN( "Exitting setIsdnNoInTelGlobalNum");
	*pErr = E_NO_ERROR;
	return SipSuccess;
}



/***********************************************************************
** Function: sip_getPostdialFromTelGlobalNum
** Description: gets the PostDial field from TelGlobalNum
** Parameters:
**		pGlobal (IN)	- TelGlobalNum
**		ppPostd(OUT)	- retreived PostDial fiels
**		pErr (OUT)		- Possible Error value (see API ref doc)
************************************************************************/
SipBool sip_getPostdialFromTelGlobalNum
#ifdef ANSI_PROTO
	(TelGlobalNum *pGlobal,SIP_S8bit **ppPostd, SipError *pErr)
#else
	(pGlobal,ppPostd,pErr)
	TelGlobalNum *pGlobal;
	SIP_S8bit **ppPostd;
	SipError *pErr;
#endif
{
#ifndef SIP_BY_REFERENCE
	SIP_U16bit dLength;
#endif
	SIP_S8bit * pTemp_postd;
	SIPDEBUGFN( "Entering getPostdialFromTelGlobalNum");
#ifndef SIP_NO_CHECK
	if( pErr == SIP_NULL )
		return SipFail;

	if (( pGlobal == SIP_NULL) || (ppPostd == SIP_NULL))
	{
		*pErr = E_INV_PARAM;
		return SipFail;
	}
#endif
	pTemp_postd = pGlobal->pPostDial;


	if( pTemp_postd == SIP_NULL )
	{
		*pErr = E_NO_EXIST;
		return SipFail;
	}
#ifdef SIP_BY_REFERENCE
	*ppPostd= pTemp_postd;
#else

	dLength = strlen(pTemp_postd);
	*ppPostd= ( SIP_S8bit * )fast_memget(ACCESSOR_MEM_ID,dLength+1,pErr);
	if ( *ppPostd== SIP_NULL )
		return SipFail;

	strcpy( *ppPostd, pTemp_postd);
#endif

	SIPDEBUGFN( "Exitting getPostDialFromTelGlobalNum");
	*pErr = E_NO_ERROR;
	return SipSuccess;

}


/***********************************************************************
** Function: sip_setPostdialInTelGlobalNum
** Description: sets the PostDial field in TelGlobalNum
** Parameters:
**		pGlobal (IN/OUT)- TelGlobalNum
**		pPostd(IN)		- PostDial to be set
**		pErr (OUT)		- Possible Error value (see API ref doc)
************************************************************************/
SipBool sip_setPostdialInTelGlobalNum
#ifdef ANSI_PROTO
	(TelGlobalNum *pGlobal,SIP_S8bit *pPostd,SipError *pErr )
#else
	(pGlobal,pPostd,pErr )
	TelGlobalNum *pGlobal;
	SIP_S8bit *pPostd;
	SipError *pErr;
#endif
{
#ifndef SIP_BY_REFERENCE
	SIP_U32bit dLength;
	SIP_S8bit * pTemp_postd;
#endif
	SIPDEBUGFN( "Entering setPostDialInTelGlobalNum");
#ifndef SIP_NO_CHECK
	if( pErr == SIP_NULL )
		return SipFail;

	if ( pGlobal == SIP_NULL)
	{
		*pErr = E_INV_PARAM;
		return SipFail;
	}
#endif
	if(pGlobal->pPostDial  !=SIP_NULL)
	{
		if(sip_memfree(ACCESSOR_MEM_ID,(SIP_Pvoid*)&(pGlobal->pPostDial),\
			pErr) ==SipFail)
			return SipFail;
	}
#ifdef SIP_BY_REFERENCE
	pGlobal->pPostDial = pPostd;
#else
	if( pPostd == SIP_NULL)
		pTemp_postd = SIP_NULL;
	else
	{
		dLength = strlen( pPostd);
		pTemp_postd = ( SIP_S8bit * )fast_memget(ACCESSOR_MEM_ID,dLength+1,\
			pErr);
		if ( pTemp_postd == SIP_NULL )
			return SipFail;

		strcpy( pTemp_postd, pPostd);
	}
	pGlobal->pPostDial= pTemp_postd;
#endif
	SIPDEBUGFN( "Exitting setpostdialInTelGlobalNum");
	*pErr = E_NO_ERROR;
	return SipSuccess;
}



/***********************************************************************
** Function: sip_getParamCountFromTelGlobalNum
** Description: gets the number of parameters in TelGlobalNum
**             (service provider and future extension are treated as parameters)
** Parameters:
**		pGlobal (IN)	- TelGlobalNum
**		pCount (OUT)	- number of parameters
**		pErr (OUT)		- Possible Error value (see API ref doc)
************************************************************************/
 SipBool sip_getParamCountFromTelGlobalNum
#ifdef ANSI_PROTO
	(TelGlobalNum *pGlobal,SIP_U32bit *pCount, SipError *pErr)
#else
 	(pGlobal,pCount,pErr)
	TelGlobalNum *pGlobal;
 	SIP_U32bit *pCount;
 	SipError *pErr;
#endif
{
	SIPDEBUGFN("Entering function sip_getParamCountFromTelGlobalNum");
#ifndef SIP_NO_CHECK
	if( pErr == SIP_NULL )
		return SipFail;

	if ( pGlobal == SIP_NULL )
	{
		*pErr = E_INV_PARAM;
		return SipFail;
	}

	if (pCount == SIP_NULL)
	{
		*pErr = E_INV_PARAM;
		return SipFail;
	}
#endif
	if (sip_listSizeOf( &(pGlobal)->slParams, pCount , pErr) == SipFail )
	{
		return SipFail;
	}

	*pErr = E_NO_ERROR;
	SIPDEBUGFN("Exitting function sip_getParamCountFromTelGlobalNum");
	return SipSuccess;
}




/*****************************************************************************
** Function: sip_getParamAtIndexInTelGlobalNum
** Description: gets the Param at the specified index in TelGlobalNum
**             (service provider and future extension are treated as parameters)
** Parameters:
**	pGlobal (IN)	- TelGlobalNum
**	pParam(OUT)		- retreived Parameter
**	dIndex (IN)		- index at which param is to be retieved
**	pErr (OUT)		- Possible Error value (see API ref doc)
******************************************************************************/
SipBool sip_getParamAtIndexFromTelGlobalNum
#ifdef ANSI_PROTO
#ifdef SIP_BY_REFERENCE
	(TelGlobalNum *pGlobal,SipParam **ppParam, SIP_U32bit dIndex,\
		SipError *pErr)
#else
	(TelGlobalNum *pGlobal,SipParam *pParam, SIP_U32bit dIndex, \
		SipError *pErr)
#endif
#else
#ifdef SIP_BY_REFERENCE
	(pGlobal,ppParam, dIndex, pErr)
	TelGlobalNum *pGlobal;
	SipParam **ppParam;
	SIP_U32bit dIndex;
	SipError *pErr;
#else
	(pGlobal,pParam, dIndex, pErr)
	TelGlobalNum *pGlobal;
	SipParam *pParam;
	SIP_U32bit dIndex;
	SipError *pErr;
#endif
#endif
{
	SIP_Pvoid element_from_list;
	SipParam *temp_param;
	SIPDEBUGFN("Entering function sip_getParamAtIndexFromTelGlobalNum");
#ifndef SIP_NO_CHECK
	if( pErr == SIP_NULL )
		return SipFail;

	if ( pGlobal == SIP_NULL )
	{
		*pErr = E_INV_PARAM;
		return SipFail;
	}

#ifndef SIP_BY_REFERENCE
	if (pParam == SIP_NULL)
#else
	if (ppParam == SIP_NULL)
#endif
	{
		*pErr = E_INV_PARAM;
		return SipFail;
	}
#endif

	if (sip_listGetAt( &(pGlobal->slParams), dIndex,  \
		&element_from_list, pErr) == SipFail)
		return SipFail;

	if (element_from_list == SIP_NULL)
	{
		*pErr = E_NO_EXIST;
		return SipFail;
	}

	temp_param = (SipParam *)element_from_list;
#ifndef SIP_BY_REFERENCE
	if (__sip_cloneSipParam(pParam, temp_param, pErr) == SipFail)
		return SipFail;
#else
	HSS_LOCKEDINCREF(temp_param->dRefCount);
	*ppParam = temp_param;
#endif

	*pErr = E_NO_ERROR;
	SIPDEBUGFN("Exitting function sip_getParamAtIndexFromTelGlobalNum");
	return SipSuccess;
}


/*****************************************************************************
** Function: sip_setParamAtIndexInTelGlobalNum
** Description: sets the Param at the specified index in TelGlobalNum
**             (service provider and future extension are treated as parameters)
** Parameters:
**	pGlobal (IN/OUT)	- TelGlobalNum
**	pParam(IN)			- Param to be set
**	dIndex (IN)			- index at which param is set in Tel Global number
**	pErr (OUT)			- Possible Error value (see API ref doc)
******************************************************************************/
SipBool sip_setParamAtIndexInTelGlobalNum
#ifdef ANSI_PROTO
	(TelGlobalNum *pGlobal,SipParam *pParam, SIP_U32bit dIndex, \
		SipError *pErr)
#else
	(pGlobal,pParam, dIndex, pErr)
	TelGlobalNum *pGlobal;
	SipParam *pParam;
	SIP_U32bit dIndex;
	SipError *pErr;
#endif
{
	SipParam *temp_param;
	SIPDEBUGFN("Entering function sip_setParamAtIndexInTelGlobalNum");
#ifndef SIP_NO_CHECK
	if( pErr == SIP_NULL )
		return SipFail;

	if ( pGlobal == SIP_NULL )
	{
		*pErr = E_INV_PARAM;
		return SipFail;
	}
#endif
	if ( pParam == SIP_NULL )
		temp_param = SIP_NULL;
	else
	{
#ifndef SIP_BY_REFERENCE
		if (sip_initSipParam(&temp_param, pErr) == SipFail)
			return SipFail;
		if (__sip_cloneSipParam(temp_param, pParam, pErr) == SipFail)
		{
			sip_freeSipParam (temp_param);
			return SipFail;
		}
#else
		HSS_LOCKEDINCREF(pParam->dRefCount);
		temp_param = pParam;
#endif
	}

	if( sip_listSetAt( &(pGlobal->slParams),  \
		dIndex, (SIP_Pvoid)(temp_param), pErr) == SipFail)
	{
		if (temp_param != SIP_NULL)
#ifndef SIP_BY_REFERENCE
			sip_freeSipParam (temp_param);
#else
		HSS_LOCKEDDECREF(pParam->dRefCount);
#endif
		return SipFail;
	}

	*pErr = E_NO_ERROR;
	SIPDEBUGFN("Exitting function sip_setParamAtIndexInTelGlobalNum");
	return SipSuccess;
}




/******************************************************************************
** Function: sip_insertParamAtIndexInTelGlobalNum
** Description: inserts the Param at the specified index in TelGlobalNum
**             (service provider and future extension are treated as parameters)
** Parameters:
**	pGlobal (IN/OUT)- TelGlobalNum
**	pParam(IN)		- Param to be inserted
**	dIndex (IN)		- index at which param is inserted in Tel Global number
**	pErr (OUT)		- Possible Error value (see API ref doc)
*******************************************************************************/
 SipBool sip_insertParamAtIndexInTelGlobalNum
#ifdef ANSI_PROTO
	(TelGlobalNum *pGlobal, SipParam *pParam, SIP_U32bit dIndex, \
		SipError *pErr)
#else
	(pGlobal, pParam, dIndex, pErr)
	TelGlobalNum *pGlobal;
	SipParam *pParam;
	SIP_U32bit dIndex;
	SipError *pErr;
#endif
{
	SipParam *temp_param;
	SIPDEBUGFN("Entering function sip_insertParamAtIndexInTelGlobalNum");
#ifndef SIP_NO_CHECK
	if( pErr == SIP_NULL )
		return SipFail;

	if ( pGlobal == SIP_NULL )
	{
		*pErr = E_INV_PARAM;
		return SipFail;
	}

#endif
	if ( pParam == SIP_NULL )
		temp_param = SIP_NULL;
	else
	{
#ifndef SIP_BY_REFERENCE
		if (sip_initSipParam(&temp_param, pErr) == SipFail)
			return SipFail;
		if (__sip_cloneSipParam(temp_param, pParam, pErr) == SipFail)
		{
			sip_freeSipParam (temp_param);
			return SipFail;
		}
#else
		HSS_LOCKEDINCREF(pParam->dRefCount);
		temp_param = pParam;
#endif
	}

	if( sip_listInsertAt( &(pGlobal->slParams),  \
		dIndex, (SIP_Pvoid)(temp_param), pErr) == SipFail)
	{
		if (temp_param != SIP_NULL)
#ifndef SIP_BY_REFERENCE
			sip_freeSipParam (temp_param);
#else
		HSS_LOCKEDDECREF(pParam->dRefCount);
#endif
		return SipFail;
	}

	*pErr = E_NO_ERROR;
	SIPDEBUGFN("Exitting function sip_insertParamAtIndexInTelGlobalNum");
	return SipSuccess;
}



/***********************************************************************
** Function: sip_deleteParamAtIndexInTelGlobalNum
** Description: deletes the param at the specified index in TelGlobalNum
**             (service provider and future extension are treated as parameters)
** Parameters:
**	pGlobal (IN)	- TelGlobalNum
**	dIndex (IN)		- index at which param is deleted in Tel Global number
**	pErr (OUT)		- Possible Error value (see API ref doc)
************************************************************************/
 SipBool sip_deleteParamAtIndexInTelGlobalNum
#ifdef ANSI_PROTO
(TelGlobalNum *pGlobal, SIP_U32bit dIndex, SipError *pErr)
#else
(pGlobal, dIndex, pErr)
TelGlobalNum *pGlobal;
SIP_U32bit dIndex;
SipError *pErr;
#endif
{
	SIPDEBUGFN("Entering function sip_deleteParamAtIndexInTelGlobalNum");
#ifndef SIP_NO_CHECK
	if( pErr == SIP_NULL )
		return SipFail;

	if ( pGlobal == SIP_NULL )
	{
		*pErr = E_INV_PARAM;
		return SipFail;
	}

#endif
	if( sip_listDeleteAt( &(pGlobal->slParams), dIndex, pErr) == SipFail)
		return SipFail;

	*pErr = E_NO_ERROR;
	SIPDEBUGFN("Exitting function sip_deleteParamAtIndexInTelGlobalNum");
	return SipSuccess;
}




/***************************************************************
** Function: sip_getAreaSpecifierCountFromTelGlobalNum
** Description: gets the scheme from the TelUrl structure
** Parameters:
**	pGlobal (IN)	- TelGlobalNum
**	pCount (OUT)	- number of Tel AreaSpecifiers
**	pErr (OUT)		- Possible Error value (see API ref doc)
************************************************************************/

SipBool sip_getAreaSpecifierCountFromTelGlobalNum
#ifdef ANSI_PROTO
	(TelGlobalNum *pGlobal,
	SIP_U32bit *pCount,
	SipError *pErr)
#else
	(pGlobal, pCount, pErr)
	TelGlobalNum *pGlobal;
	SIP_U32bit *pCount;
	SipError *pErr;
#endif
{
	SIPDEBUGFN ( "Entering getAreaSpecifierCountFromTelGlobalNum");
#ifndef SIP_NO_CHECK
	if ( pErr == SIP_NULL )
		return SipFail;

	if ( (pGlobal == SIP_NULL) || ( pCount == SIP_NULL ))
	{
		*pErr = E_INV_PARAM;
		return SipFail;
	}
#endif
	if (sip_listSizeOf(&(pGlobal->slAreaSpecifier), pCount , pErr) == SipFail )
	{
		return SipFail;
	}

	SIPDEBUGFN ( "Exitting getAreaSpecifierCountFromTelGlobalNum");
	*pErr = E_NO_ERROR;
	return SipSuccess;
}

/***********************************************************************
** Function: sip_getAreaSpecifierAtIndexFromTelGlobalNum
** Description: gets the AreaSpecifier field at the index from TelGlobalNum
** Parameters:
**	pGlobal (IN)			- TelGlobalNum
**	ppAreaSpecifier(OUT)- retreived AreaSpecifier
**	dIndex (IN)			- index at which AreaSpecifier is to be retrieved
**	pErr (OUT)			- Possible Error value (see API ref doc)
************************************************************************/

SipBool sip_getAreaSpecifierAtIndexFromTelGlobalNum
#ifdef ANSI_PROTO
	(TelGlobalNum *pGlobal,
	 SIP_S8bit **ppAreaSpecifier,
	 SIP_U32bit 	index,
	 SipError 	*pErr )
#else
	(pGlobal,ppAreaSpecifier,index,pErr)
	 TelGlobalNum *pGlobal;
	 SIP_S8bit **ppAreaSpecifier;
    	 SIP_U32bit index;
	 SipError *pErr;
#endif
{
	SIP_Pvoid pElementFromList;
#ifndef SIP_BY_REFERENCE
	SIP_U32bit size;
#endif

	SIPDEBUGFN ( "Entering sip_getAreaSpecifierAtIndexFromTelGlobalNum ");
#ifndef SIP_NO_CHECK
	if( pErr == SIP_NULL )
		return SipFail;

	if ( (pGlobal == SIP_NULL) || (ppAreaSpecifier == SIP_NULL) )
	{
		*pErr = E_INV_PARAM;
		return SipFail;
	}
#endif
	if( sip_listGetAt(&(pGlobal->slAreaSpecifier), index, &pElementFromList, \
		pErr) == SipFail)
		return SipFail;

	if ( pElementFromList == SIP_NULL )
	{
		*pErr = E_NO_EXIST;
		return SipFail;
	}

#ifdef SIP_BY_REFERENCE
	*ppAreaSpecifier = (SIP_S8bit*)pElementFromList;
#else
	size = strlen( (SIP_S8bit * )pElementFromList);
	*ppAreaSpecifier = ( SIP_S8bit * )fast_memget(ACCESSOR_MEM_ID, size +1, \
		pErr);
	if(*ppAreaSpecifier == SIP_NULL)
		return SipFail;

	strcpy(*ppAreaSpecifier, (SIP_S8bit*)pElementFromList);
#endif
	SIPDEBUGFN ( "Exitting sip_getAreaSpecifierAtIndexFromTelGlobalNum");
	*pErr = E_NO_ERROR;
	return SipSuccess;
}

/***********************************************************************
** Function: sip_setAreaSpecifierAtIndexFromTelGlobalNum
** Description: sets the AreaSpecifier field at the index in TelGlobalNum
** Parameters:
**	pGlobal (IN/OUT)		- TelGlobalNum
**	pAreaSpecifier(IN)	- AreaSpecifier to be set
**	dIndex (IN)			- index at which AreaSpecifier is to be set
**	pErr (OUT)			- Possible Error value (see API ref doc)
************************************************************************/

SipBool  sip_setAreaSpecifierAtIndexInTelGlobalNum
#ifdef ANSI_PROTO
	(TelGlobalNum *pGlobal,
	 SIP_S8bit *pAreaSpecifier,
	 SIP_U32bit 	index,
	 SipError 	*pErr )
#else
	(pGlobal,pAreaSpecifier,index,pErr)
	 TelGlobalNum *pGlobal;
	 SIP_S8bit *pAreaSpecifier;
    	 SIP_U32bit index;
	 SipError *pErr;
#endif
{

	SIP_S8bit * pElementFromList;
	SipError tempErr;	/* used in freeing memory after an pError has happened */

	SIPDEBUGFN ( "Entering sip_setAreaSpecifierAtIndexInTelGlobalNum");
#ifndef SIP_NO_CHECK
	if( pErr == SIP_NULL )
		return SipFail;

	if (pGlobal == SIP_NULL)
	{
		*pErr = E_INV_PARAM;
		return SipFail;
	}
#endif
	if (pAreaSpecifier  == SIP_NULL )
		pElementFromList = SIP_NULL;
	else
	{
#ifdef SIP_BY_REFERENCE
		pElementFromList = pAreaSpecifier;
#else

		pElementFromList = ( SIP_S8bit * )fast_memget(ACCESSOR_MEM_ID,\
			strlen(pAreaSpecifier) + 1, pErr);
		if( pElementFromList == SIP_NULL )
			return SipFail;

		strcpy(pElementFromList,pAreaSpecifier );
#endif
	}

	if( sip_listSetAt(&(pGlobal->slAreaSpecifier), index, pElementFromList, \
		pErr) == SipFail)
	{
		if ( pElementFromList != SIP_NULL )
			sip_memfree(ACCESSOR_MEM_ID, (SIP_Pvoid *)(&(pElementFromList)), \
				&tempErr);
		return SipFail;
	}

	SIPDEBUGFN ( "Exitting sip_setAreaSpecifierAtIndexInTelGlobalNum");
	*pErr = E_NO_ERROR;
	return SipSuccess;
}

/***********************************************************************
** Function: sip_insertAreaSpecifierAtIndexInTelGlobalNum
** Description: inserts the AreaSpecifier field at the index in TelGlobalNum
** Parameters:
**	pGlobal (IN/OUT)		- TelGlobalNum
**	pAreaSpecifier(IN)	- AreaSpecifier to be inserted
**	dIndex (IN)			- index at which AreaSpecifier is to be inserted
**	pErr (OUT)		- Possible Error value (see API ref doc)
************************************************************************/

SipBool sip_insertAreaSpecifierAtIndexInTelGlobalNum
#ifdef ANSI_PROTO
	(TelGlobalNum *pGlobal,
	 SIP_S8bit *pAreaSpecifier,
	 SIP_U32bit 	index,
	 SipError 	*pErr )
#else
	(pGlobal,pAreaSpecifier,index,pErr)
	 TelGlobalNum *pGlobal;
	 SIP_S8bit *pAreaSpecifier;
     SIP_U32bit index;
	 SipError *pErr;
#endif
{

	SIP_S8bit * pElementFromList;
	SipError tempErr;		/* used in freeing memory after an pError has happened */


	SIPDEBUGFN ( "Entering sip_insertAreaSpecifierAtIndexInTelGlobalNum");
#ifndef SIP_NO_CHECK
	if( pErr == SIP_NULL )
		return SipFail;

	if ( pGlobal == SIP_NULL )
	{
		*pErr = E_INV_PARAM;
		return SipFail;
	}
#endif
	/* copying the Value Headers structure/char*  */
	if (pAreaSpecifier  == SIP_NULL )
		pElementFromList = SIP_NULL;
	else
	{
#ifdef SIP_BY_REFERENCE
		pElementFromList = pAreaSpecifier	;
#else
		pElementFromList = ( SIP_S8bit * )fast_memget(ACCESSOR_MEM_ID,\
			strlen(pAreaSpecifier) + 1, pErr);
		if( pElementFromList == SIP_NULL )
			return SipFail;

		strcpy(pElementFromList,pAreaSpecifier );
#endif
	}

	if( sip_listInsertAt(&(pGlobal->slAreaSpecifier), index, pElementFromList,\
		pErr) == SipFail)
	{
		if ( pElementFromList != SIP_NULL )
			sip_memfree(ACCESSOR_MEM_ID, (SIP_Pvoid *)(&(pElementFromList)), \
				&tempErr);
		return SipFail;
	}

	SIPDEBUGFN ( "Exitting sip_insertAreaSpecifierAtIndexInTelGlobalNum");

	*pErr = E_NO_ERROR;
	return SipSuccess;
}

/***********************************************************************
** Function: sip_deleteAreaSpecifierAtIndexInTelGlobalNum
** Description: deletes the AreaSpecifier field at the index in TelGlobalNum
** Parameters:
**	pGlobal (IN/OUT)		- TelGlobalNum
**	pAreaSpecifier(IN)	- AreaSpecifier to be deleted
**	dIndex (IN)			- index at which AreaSpecifier is to be deleted
**	pErr (OUT)			- Possible Error value (see API ref doc)
************************************************************************/

SipBool sip_deleteAreaSpecifierAtIndexInTelGlobalNum
#ifdef ANSI_PROTO
	(TelGlobalNum *pGlobal,
	  SIP_U32bit 	index,
	  SipError 	*pErr )
#else
	(pGlobal,index,pErr)
  	TelGlobalNum *pGlobal;
	SIP_U32bit 	index;
	SipError 	*pErr;
#endif
{

	SIPDEBUGFN ( "Entering sip_deleteAreaSpecifierAtIndexInTelGlobalNum");
#ifndef SIP_NO_CHECK
	if( pErr == SIP_NULL )
		return SipFail;

	if (pGlobal  == SIP_NULL )
	{
		*pErr = E_INV_PARAM;
		return SipFail;
	}
#endif
	if( sip_listDeleteAt(&(pGlobal->slAreaSpecifier), index, pErr) == SipFail)
		return SipFail;

	SIPDEBUGFN ( "Exitting sip_deleteAreaSpecifierAtIndexInTelGlobalNum");

	*pErr = E_NO_ERROR;
	return SipSuccess;
}

#endif
/* Ensure Names are not mangled by C++ compilers */
#ifdef __cplusplus
}
#endif
