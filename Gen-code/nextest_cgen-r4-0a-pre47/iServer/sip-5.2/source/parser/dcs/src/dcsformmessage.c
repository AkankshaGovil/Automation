/***********************************************************************
 ** FUNCTION:
 **             Forms a Text SIP Message from SIP Structure
 *********************************************************************
 **
 ** FILENAME:
 ** dcssipformmessage.c
 **
 ** DESCRIPTION:
 ** This file contains code to convert from structures to SIP Text
 ** Entry function is : sip_formMessage
 **
 ** DATE        NAME                    REFERENCE               REASON
 ** ----        ----                    ---------              --------
 ** 
 ** 6/12/00   	 Mahesh Govind         Initial Creation
 **              Seshashayi
 **
 **
 **     Copyright 2000, Hughes Software Systems, Ltd.
 *********************************************************************/


#include "sipcommon.h"
#include "sipstruct.h"
#include "sipstatistics.h"
#include "portlayer.h"
#include "sipformmessage.h"
#include "sipinit.h"
#include "sipdecode.h"
#include "dcsformmessage.h"

#define CRLF "\r\n"
#define NUMREDIR 20

#ifdef STRCAT
#undef STRCAT
#endif

#define STRCAT(e,a,b) \
do \
{ \
	if ((b!=SIP_NULL))\
	{\
	SIP_U32bit srcLength;\
	SIP_S8bit* pTemp;\
	if(a)a += strlen(a);\
	srcLength = strlen(b);\
	pTemp = a + srcLength;\
	if((!e)||(pTemp < e))\
	memcpy (a,b,srcLength);\
	else\
	{\
	*err = E_BUF_OVERFLOW;\
	return SipFail;\
	}\
	a = pTemp;\
	*a = 0;\
	}\
}\
while(0)
/*****************************************************************
** FUNCTION: sip_dcs_formSingleGeneralHeader
** 
**
** DESCRIPTION: Converts a SipGeneralHeader to Text
*****************************************************************/

SipBool sip_dcs_formSingleGeneralHeader 
#ifdef ANSI_PROTO
	(
	 SIP_S8bit	*pEndBuff,
	 en_HeaderType dType,
	 SIP_U32bit ndx,
	 en_AdditionMode mode,
	 en_HeaderForm form,
	 SipGeneralHeader *g, 
	 SIP_S8bit **ppOut, 
	 SipError *err)
#else
	(pEndBuff,dType, ndx, mode, form,g,ppOut,err)
	SIP_S8bit	*pEndBuff;
	en_HeaderType dType;
	SIP_U32bit ndx;
	en_AdditionMode mode;
	en_HeaderForm form;
	SipGeneralHeader *g;
	SIP_S8bit **ppOut;
	SipError *err;
#endif
{
	SipBool res;
	SIP_S8bit *out;
	en_HeaderForm dummy;
	SIPDEBUGFN("Entering sip_dcs_formSingleGeneralHeader"); 	
	out=*ppOut;
	dummy = form;
	
	
	switch (dType)
	{
		case SipHdrTypeDcsMediaAuthorization:
		{
			SipDcsMediaAuthorizationHeader *a;

			res = sip_listGetAt (&(g->slDcsMediaAuthorizationHdr), ndx, \
				(SIP_Pvoid *) &a, err);
			if ( res == SipFail)
			{
				return SipFail;
			}

			if(mode==SipModeNone) {}
			else if(mode==SipModeNew)
				STRCAT (pEndBuff,out, "P-Media-Authorization: ");
			else
				STRCAT (pEndBuff,out, ",");
			
			STRCAT (pEndBuff,out, a->pAuth);
			STRCAT (pEndBuff,out, CRLF);
			break;
		}

		case SipHdrTypeDcsBillingId:
		{
			if (g->pDcsBillingIdHdr == SIP_NULL)
			{
				*err =  E_INV_PARAM;
				return SipFail;
			}
			if(mode==SipModeNew) 
				STRCAT (pEndBuff,out, "Dcs-Billing-Id: ");
			/* Single Instance Header */	
			
			STRCAT (pEndBuff,out, g->pDcsBillingIdHdr->pId);
			STRCAT (pEndBuff,out, "/");
			STRCAT (pEndBuff,out, g->pDcsBillingIdHdr->pFEId);
			STRCAT (pEndBuff,out, CRLF);
			break;
		}

		case SipHdrTypeDcsBillingInfo:
		{
			if (g->pDcsBillingInfoHdr == SIP_NULL)
			{
				*err =  E_INV_PARAM;
				return SipFail;
			}
			if(mode==SipModeNew) 
				STRCAT (pEndBuff,out, "Dcs-Billing-Info: ");
			
			STRCAT (pEndBuff,out, g->pDcsBillingInfoHdr->pHost);
			if (g->pDcsBillingInfoHdr->pPort!=SIP_NULL) 
			{
				SIP_S8bit pPort[SIP_MAX_PORT_SIZE];
				STRCAT (pEndBuff,out, ":");
				HSS_SNPRINTF\
					((char *)pPort,SIP_MAX_PORT_SIZE, "%u",*(g->pDcsBillingInfoHdr->pPort));
				pPort[SIP_MAX_PORT_SIZE-1]='\0';
				STRCAT (pEndBuff,out, pPort);
			}
			if (SipFail==sip_formSipDcsAcctEntryList\
				(pEndBuff,&out, &((g->pDcsBillingInfoHdr)->slAcctEntry), \
				(char *)",", 0, err))
			{	
				return SipFail;
			}
		
			STRCAT (pEndBuff,out, CRLF);
			break;
		}

		case SipHdrTypeDcsLaes:
		{
			if (g->pDcsLaesHdr == SIP_NULL)
			{
				*err =  E_INV_PARAM;
				return SipFail;
			}

			if(mode==SipModeNew) 
				STRCAT ( pEndBuff,out, "Dcs-Laes: ");

			if (g->pDcsLaesHdr->pSignatureHost!=SIP_NULL) 
				STRCAT ( pEndBuff,out, g->pDcsLaesHdr->pSignatureHost);

			if (g->pDcsLaesHdr->pSignaturePort!=SIP_NULL) 
			{
				SIP_S8bit pPort[SIP_MAX_PORT_SIZE];
				STRCAT ( pEndBuff,out, ":");
				HSS_SNPRINTF\
				((char *)pPort,SIP_MAX_PORT_SIZE, "%u",*(g->pDcsLaesHdr->pSignaturePort));
				pPort[SIP_MAX_PORT_SIZE-1]='\0';
				STRCAT (pEndBuff,out, pPort);
			}
			if (g->pDcsLaesHdr->pContentHost!=SIP_NULL) 
			{
				STRCAT (pEndBuff,out, ",");
				STRCAT ( pEndBuff,out, g->pDcsLaesHdr->pContentHost);
			}

			if (g->pDcsLaesHdr->pContentPort!=SIP_NULL) 
			{
				SIP_S8bit pPort[SIP_MAX_PORT_SIZE];
				STRCAT ( pEndBuff,out, ":");
				HSS_SNPRINTF\
					((char *)pPort,SIP_MAX_PORT_SIZE, "%u", *(g->pDcsLaesHdr->pContentPort));
				pPort[SIP_MAX_PORT_SIZE-1]='\0';
				STRCAT (pEndBuff,out, pPort);
			}
			
			if (g->pDcsLaesHdr->pKey!=SIP_NULL) 
			{
				STRCAT ( pEndBuff,out, ";");
				STRCAT ( pEndBuff,out, g->pDcsLaesHdr->pKey);
			}

			STRCAT ( pEndBuff,out, CRLF);
			break;
		}

		case SipHdrTypeDcsGate:
		{
			SIP_S8bit pPort[SIP_MAX_PORT_SIZE];
			if (g->pDcsGateHdr == SIP_NULL)
			{
				*err =  E_INV_PARAM;
				return SipFail;
			}
			if(mode==SipModeNew) 
				STRCAT ( pEndBuff,out, "Dcs-Gate: ");

			if (g->pDcsGateHdr->pHost!=SIP_NULL) 
				STRCAT ( pEndBuff,out, g->pDcsGateHdr->pHost);

			STRCAT ( pEndBuff,out, ":");
	
			if (g->pDcsGateHdr->pPort!=SIP_NULL) 
			{
				HSS_SNPRINTF((char *)pPort,SIP_MAX_PORT_SIZE, "%u", *(g->pDcsGateHdr->pPort));
				pPort[SIP_MAX_PORT_SIZE-1]='\0';
				STRCAT ( pEndBuff,out,pPort );
			}

			STRCAT ( pEndBuff,out, "/");

			if (g->pDcsGateHdr->pId!=SIP_NULL) 
				STRCAT ( pEndBuff,out, g->pDcsGateHdr->pId);

			if (g->pDcsGateHdr->pKey!=SIP_NULL) 
			{
				STRCAT ( pEndBuff,out, ";");
				STRCAT ( pEndBuff,out, g->pDcsGateHdr->pKey);
			}

			if (g->pDcsGateHdr->pCipherSuite!=SIP_NULL) 
			{
				STRCAT ( pEndBuff,out, ";");
				STRCAT ( pEndBuff,out, g->pDcsGateHdr->pCipherSuite);
			}

								
			if (g->pDcsGateHdr->pStrength!=SIP_NULL) 
			{		
				STRCAT ( pEndBuff,out, " ");
				STRCAT ( pEndBuff,out, g->pDcsGateHdr->pStrength);
			}	

			STRCAT ( pEndBuff,out, CRLF);
			break;
		}
	
		case SipHdrTypeDcsRemotePartyId:
		{
			SipDcsRemotePartyIdHeader *a;

			res = sip_listGetAt (&(g->slDcsRemotePartyIdHdr), ndx, \
				(SIP_Pvoid *) &a, err);
			if ( res == SipFail)
			{
				return SipFail;
			}
			if(mode==SipModeNone) {}
			else if(mode==SipModeNew)
				STRCAT ( pEndBuff,out, "Remote-Party-Id: ");
			else		
				STRCAT ( pEndBuff,out, ",");

			if (a->pDispName !=SIP_NULL)  
			{
				STRCAT ( pEndBuff,out, a->pDispName);
				if (strcmp(a->pDispName,""))
					STRCAT ( pEndBuff,out," ");
			}
				
			if (a->pAddrSpec !=SIP_NULL)
			{
				STRCAT ( pEndBuff,out,"<"); 
				if (SipFail ==sip_formAddrSpec \
										(pEndBuff,&out, a->pAddrSpec, err ))
				{
					return SipFail;
				}
				STRCAT ( pEndBuff,out,">"); 
			} /* of if addrspec */

			if (SipFail==sip_formSipParamList\
					(pEndBuff,&out, &(a->slParams), (SIP_S8bit *) ";", 1, err))
			{
				return SipFail;
			}
			
			STRCAT ( pEndBuff,out,CRLF);
			break;
		}
	
		case SipHdrTypeDcsRpidPrivacy:
		{
			SipDcsRpidPrivacyHeader *a;
			res = sip_listGetAt (&(g->slDcsRpidPrivacyHdr), ndx, \
				(SIP_Pvoid *) &a, err);
			if ( res == SipFail)
			{
				return SipFail;
			}
			if(mode==SipModeNone) {}
			else if(mode==SipModeNew)
				STRCAT ( pEndBuff,out, "RPID-Privacy:");
			else 
				STRCAT ( pEndBuff,out, ",");

			if (SipFail==sip_formSipParamList\
				(pEndBuff,&out, &(a->slParams), (SIP_S8bit *) ";", 0, err))
			{
				return SipFail;
			}
			STRCAT ( pEndBuff,out, CRLF);
			break;
		}

		case SipHdrTypeDcsAnonymity:
		{
			SipDcsAnonymityHeader *a;
			res = sip_listGetAt \
						(&(g->slDcsAnonymityHdr), ndx, (SIP_Pvoid *) &a, err);
			if ( res == SipFail)
			{
				return SipFail;
			}
			if(mode==SipModeNone) {}
			else if(mode==SipModeNew)
				STRCAT ( pEndBuff,out, "Anonymity: ");
			else 
				STRCAT ( pEndBuff,out, ",");

			STRCAT ( pEndBuff,out, a->pTag);
			STRCAT ( pEndBuff,out, CRLF);
			break;
		}

		case SipHdrTypeDcsState:
		{
			SipDcsStateHeader *a;
			res = sip_listGetAt\
							(&(g->slDcsStateHdr), ndx, (SIP_Pvoid *) &a, err);
			if ( res == SipFail)
			{
				return SipFail;
			}
			if(mode==SipModeNone) {}
			else if(mode==SipModeNew)
				STRCAT ( pEndBuff,out, "State: ");
			else 
				STRCAT ( pEndBuff,out, ",");

			STRCAT ( pEndBuff,out, a->pHost);
		
			if (SipFail==sip_formSipParamList\
					(pEndBuff,&out, &(a->slParams), (SIP_S8bit *) ";", 1, err))
			{
				return SipFail;
			}
			
			STRCAT ( pEndBuff,out, CRLF);
			break;
		}
		default:
			break;
	}
	if(pEndBuff) *ppOut = out;
	SIPDEBUGFN("Exiting sip_dcs_formSingleGeneralHeader"); 	
	return SipSuccess;
}

/*****************************************************************
** FUNCTION: sip_dcs_formSingleRequestHeader
** 
**
** DESCRIPTION: Converts a SipReqHeader to pText
*****************************************************************/

SipBool sip_dcs_formSingleRequestHeader 
#ifdef ANSI_PROTO
	 (SIP_S8bit	*pEndBuff,
	 en_HeaderType dType,
	 SIP_U32bit ndx,
	 en_AdditionMode mode,
	 en_HeaderForm form,
	 SipReqHeader *s, 
	 SIP_S8bit **ppOut, 
	 SipError *err)
#else
	(pEndBuff,dType, ndx, mode, form,s,ppOut,err)
	SIP_S8bit	*pEndBuff;
	en_HeaderType dType;
	SIP_U32bit ndx;
	en_AdditionMode mode;
	en_HeaderForm form;
	SipReqHeader *s;
	SIP_S8bit **ppOut;
	SipError *err;
#endif
{
	en_HeaderForm dummy_form;
	SIP_U32bit dummy_ndx;
	en_AdditionMode dummy_mode;
	SIP_S8bit *out;
	SIPDEBUGFN("Entering sip_dcs_formSingleRequestHeader"); 	

	dummy_form = form;
	dummy_ndx=ndx;
	dummy_mode=mode;
	out=*ppOut;
	
	switch (dType)
	{
		case SipHdrTypeDcsOsps:
		{
			if (s->pDcsOspsHdr == SIP_NULL)
			{
				*err =  E_INV_PARAM;
				return SipFail;
			}
			if(mode==SipModeNew) 
				STRCAT ( pEndBuff,out, "Dcs-Osps: ");

			if (s->pDcsOspsHdr->pTag!=SIP_NULL) 
			{
				if (s->pDcsOspsHdr->pTag[0]==' ')
					STRCAT ( pEndBuff,out, &(s->pDcsOspsHdr->pTag[1]));
				else
					STRCAT ( pEndBuff,out, s->pDcsOspsHdr->pTag);
			}
			STRCAT ( pEndBuff,out, CRLF);
			break;
		}


		case SipHdrTypeDcsTracePartyId:
		{
			if (s->pDcsTracePartyIdHdr == SIP_NULL)
			{
				*err =  E_INV_PARAM;
				return SipFail;
			}
			if(mode==SipModeNew) 
				STRCAT ( pEndBuff,out, "Dcs-Trace-Party-Id: ");
			STRCAT ( pEndBuff,out, "<");
			if (SipFail==sip_formAddrSpec \
					(pEndBuff,&out, s->pDcsTracePartyIdHdr->pAddrSpec, err))
			{
				return SipFail;
			}
			STRCAT ( pEndBuff,out, ">");
			STRCAT ( pEndBuff,out, CRLF);
			break;
		}

		case SipHdrTypeDcsRedirect:
		{
			if (s->pDcsRedirectHdr == SIP_NULL)
			{
				*err =  E_INV_PARAM;
				return SipFail;
			}
			if(mode==SipModeNew) 
				STRCAT ( pEndBuff,out, "Dcs-Redirect: ");
				
			{
				SipUrl *su;
				su = s->pDcsRedirectHdr->pCalledId;
				STRCAT ( pEndBuff,out, "<");
				STRCAT ( pEndBuff,out,"sip:");
				if (su->pUser) 
					STRCAT ( pEndBuff,out, su->pUser);
				if (su->pPassword !=SIP_NULL)
				{
					STRCAT ( pEndBuff,out, ":");
					STRCAT ( pEndBuff,out, su->pPassword);
				}
				/* if pUser was there, add an @ after pUser:[passwd] */
				if (su->pUser !=SIP_NULL) STRCAT ( pEndBuff,out,"@");
				if (su->pHost !=SIP_NULL ) 
					STRCAT ( pEndBuff,out, su->pHost);
				/* see if its pHost or pHost:dPort */
				if (su->dPort != SIP_NULL)
				{
					SIP_S8bit porttext[SIP_MAX_PORT_SIZE]; /* max val of dPort = 65535 */
					STRCAT ( pEndBuff,out, ":");
					HSS_SNPRINTF((char *)porttext, SIP_MAX_PORT_SIZE, "%u", *(su->dPort) );
					porttext[SIP_MAX_PORT_SIZE-1]='\0';
					STRCAT ( pEndBuff,out, porttext);
				}
		
				if (SipFail==sip_formSipParamList\
					(pEndBuff,&out, &(su->slParam), (SIP_S8bit *) ";",1, err))
				{
					return SipFail;
				}
		
				STRCAT ( pEndBuff,out, ">");
				STRCAT ( pEndBuff,out, " ");
		
				su = s->pDcsRedirectHdr->pRedirector;
				STRCAT ( pEndBuff,out, "<");
				STRCAT ( pEndBuff,out,"sip:");
				if (su->pUser) 
					STRCAT ( pEndBuff,out, su->pUser);
			
				if (su->pPassword !=SIP_NULL)
				{
					STRCAT ( pEndBuff,out, ":");
					STRCAT ( pEndBuff,out, su->pPassword);
				}
				/* if pUser was there, add an @ after pUser:[passwd] */
				if (su->pUser !=SIP_NULL) STRCAT ( pEndBuff,out,"@");
				if (su->pHost !=SIP_NULL ) 
						STRCAT ( pEndBuff,out, su->pHost);
				/* see if its pHost or pHost:dPort */
				if (su->dPort != SIP_NULL)
				{
					SIP_S8bit porttext[SIP_MAX_PORT_SIZE]; /* max val of dPort = 65535 */
					STRCAT ( pEndBuff,out, ":");
					HSS_SNPRINTF((char *)porttext, SIP_MAX_PORT_SIZE, "%u", *(su->dPort) );
					porttext[SIP_MAX_PORT_SIZE-1]='\0';
					STRCAT ( pEndBuff,out, porttext);
				}
			
				if (SipFail==sip_formSipParamList\
					(pEndBuff,&out, &(su->slParam), (SIP_S8bit *) ";",1, err))
				{
					return SipFail;
				}
		
				STRCAT ( pEndBuff,out, ">");
			}
	
			STRCAT ( pEndBuff,out, " ");
			{
				char dNum[NUMREDIR]; 
				HSS_SNPRINTF(dNum,NUMREDIR,"%u", (s->pDcsRedirectHdr->dNum));
				STRCAT ( pEndBuff,out, dNum);
			}
		
			STRCAT (pEndBuff, out, CRLF);
			break;
		default :
			break;
		}
	}
	if(pEndBuff) *ppOut = out;
	SIPDEBUGFN("Exiting sip_dcs_formSingleRequestHeader"); 	
	return SipSuccess;
}

/*****************************************************************
** FUNCTION: sip_dcs_formSingleResponseHeader
** 
**
** DESCRIPTION: Forms a single response Header
*****************************************************************/

SipBool sip_dcs_formSingleResponseHeader 
#ifdef ANSI_PROTO
(
	SIP_S8bit	*pEndBuff,
	en_HeaderType dType,
	SIP_U32bit ndx,
	en_AdditionMode mode,
	en_HeaderForm form,
	SipRespHeader *s, 
	SIP_S8bit **ppOut, 
	SipError *err)
#else
	(pEndBuff,dType, ndx, mode, form,s, ppOut, err)
	SIP_S8bit	*pEndBuff;
	en_HeaderType dType;
	SIP_U32bit ndx;
	en_AdditionMode mode;
	en_HeaderForm form;
	SipRespHeader *s;
	SIP_S8bit **ppOut;
	SipError *err;
#endif
{
	SipBool res;
	en_HeaderForm dummy;
	SIP_S8bit	*out=SIP_NULL;
	dummy = form;
	SIPDEBUGFN("Entering sip_dcs_formSingleResponseHeader"); 	

	out=*ppOut;
	
	switch (dType)
	{
		case SipHdrTypeSession:
		{
			SipSessionHeader *a=SIP_NULL;
			res = sip_listGetAt\
							(&(s->slSessionHdr), ndx, (SIP_Pvoid *) &a, err);
			if ( res == SipFail)
			{
				return SipFail;
			}
			if(mode==SipModeNone) {}
			else if(mode==SipModeNew)
				STRCAT ( pEndBuff,out, "Session: ");
			else 
				STRCAT ( pEndBuff,out, ",");

			STRCAT ( pEndBuff,out, a->pTag);
			STRCAT ( pEndBuff,out, CRLF);
			break;
		}
		default:
			break;
	}
	if(pEndBuff) *ppOut = out;
	SIPDEBUGFN("Exiting sip_dcs_formSingleResponseHeader"); 	
	return SipSuccess;
}


/*****************************************************************
** FUNCTION: sip_formSipDcsAcctEntryList
** 
**
** DESCRIPTION: Converts a list of SipDcsAcctEntries to text
*****************************************************************/

SipBool sip_formSipDcsAcctEntryList
#ifdef ANSI_PROTO
	(SIP_S8bit	*pEndBuff,
	SIP_S8bit 	**ppOut, 
	SipList 	*list, 
	SIP_S8bit 	*separator,
	SIP_U8bit	leadingsep,
	SipError 	*err)
#else
	(pEndBuff, ppOut, list, separator, leadingsep, err)
	SIP_S8bit *pEndBuff;
	SIP_S8bit **ppOut;
	SipList *list;
	SIP_S8bit *separator;
	SIP_U8bit leadingsep;
	SipError *err;
#endif
{
	SipDcsAcctEntry *entry;
	SIP_U32bit listSize,listIter = 0;
	SIP_S8bit	*out=SIP_NULL;
	SIPDEBUGFN("Entering sip_formSipDcsAcctEntryList"); 	

	out=*ppOut;

	sip_listSizeOf(list, &listSize, err);
	if (listSize == 0)
		return SipSuccess;
	STRCAT (pEndBuff,out, " ");
	while (listIter < listSize)
	{
		sip_listGetAt (list, listIter, (SIP_Pvoid *) &entry, err);
		if((listIter!=0)||(leadingsep!=0))
 			STRCAT (pEndBuff,out, separator);
		STRCAT (pEndBuff,out, "<");
		STRCAT (pEndBuff,out, entry->pChargeNum);
		STRCAT (pEndBuff,out, ">/<");
		STRCAT (pEndBuff,out, entry->pCallingNum);
		STRCAT (pEndBuff,out, ">/<");
		STRCAT (pEndBuff,out, entry->pCalledNum);
		STRCAT (pEndBuff,out, ">");
		if (entry->pRoutingNum != SIP_NULL)
		{
			STRCAT (pEndBuff,out, "/<");
			STRCAT (pEndBuff,out, entry->pRoutingNum);
			STRCAT (pEndBuff,out, ">/<");
			STRCAT (pEndBuff,out, entry->pLocationRoutingNum);
			STRCAT (pEndBuff,out, ">");
		}
		listIter++;
	} /* while */	
	if(pEndBuff) *ppOut = out;
	SIPDEBUGFN("Exiting sip_formSipDcsAcctEntryList"); 	
	return SipSuccess;
}
