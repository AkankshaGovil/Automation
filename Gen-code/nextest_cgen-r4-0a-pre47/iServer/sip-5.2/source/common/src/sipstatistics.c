/***********************************************************************
 ** FUNCTION:
 **             Statistics APIs for stack

 *********************************************************************
 **
 ** FILENAME:
 ** sipfree.c
 **
 ** DESCRIPTION:
 ** This file contains dCodeNum to free all structures
 **
 ** DATE        NAME                    REFERENCE               REASON
 ** ----        ----                    ---------              --------
 ** 14/01/2000	KS Binu										  Original
 **
 **     Copyright 1999, Hughes Software Systems, Ltd.
 *********************************************************************/
#ifdef __cplusplus
extern "C"
{
#endif


#include "sipstatistics.h"
#include <portlayer.h>

#ifdef SIP_MIB
SipHash glbstatHash;
#ifdef SIP_THREAD_SAFE
synch_id_t glbLockRequestStatisticsMutex;
#endif
SipList glbslRequest;
#endif

#ifdef SIP_MIB
SipStatParam glbSipParserResponseClassParserStats[NUM_STAT_RESPONSECLASS];
SipStatParam glbSipParserResponseCodeParserStats[NUM_STAT_RESPONSECODE];

SipConfigStats *glbSipParserResponseCodeConfig =SIP_NULL;
SipStatParam glbSipParserUnknownMethodStats;
#endif

SIP_U32bit glbSipParserSecondLevelReqParsed;
SIP_U32bit glbSipParserSecondLevelProtoErr;
SIP_U32bit glbSipParserMemErr;
SIP_U32bit glbSipParserNetErr;
SIP_U32bit glbSipParserProtoErr;
SIP_U32bit glbSipParserApiCount;
SIP_U32bit glbSipParserReqParsed;
SIP_U32bit glbSipParserRespParsed;
SIP_U32bit glbSipParserReqSent;
SIP_U32bit glbSipParserRespSent;
#ifdef SIP_THREAD_SAFE
synch_id_t glbLockStatisticsMutex;
#endif

/*****************************************************************
** FUNCTION:sip_initStatistics
**
**
** DESCRIPTION: Initialize the Statistics elements
*******************************************************************/

SipBool sip_initStatistics
#ifdef ANSI_PROTO
(SIP_U8bit dType,
 SIP_U8bit scope,
  SipError *err)
#else
 (dType, scope, err)
 SIP_U8bit dType;
 SIP_U8bit scope;
  SipError *err;
#endif
{
#ifdef SIP_MIB
	SIP_U32bit dIndex;
#endif
#ifndef SIP_STATISTICS
#ifdef SIP_MIB
	SIP_U32bit dummy_dIndex;
#endif
	SIP_U8bit dummy_dType,dummy_scope;
	dummy_dType = dType;
	dummy_scope = scope;
#ifdef SIP_MIB
	dummy_dIndex = dIndex;
#endif
	*err = E_STATS_DISABLED;
	return SipFail;
#else
	switch(dType)
	{
		case SIP_STAT_TYPE_API	:
		switch(scope)
		{
			case SIP_STAT_API_ALL :
				glbSipParserApiCount=0;
				glbSipParserReqParsed=0;
				glbSipParserRespParsed=0;
				glbSipParserReqSent=0;
				glbSipParserRespSent=0;

				glbSipParserSecondLevelReqParsed=0;

#ifdef SIP_MIB
				sip_hashInit (&glbstatHash, __sip_HashFunction,\
					__sip_stringKeyCompareFunction, \
					__sip_hashElementFreeFunction, __sip_hashKeyFreeFunction, \
					NUM_BUCKETS, NUM_ELEMENTS, err);
#ifdef SIP_THREAD_SAFE
				fast_lock_synch(0,&glbLockRequestStatisticsMutex,\
					FAST_MXLOCK_EXCLUSIVE );
#endif
				__sip_AddHashElemForKnownMethod();

				glbSipParserUnknownMethodStats.recv=0;
				glbSipParserUnknownMethodStats.sendInitial=0;
				glbSipParserUnknownMethodStats.sendRetry=0;

#ifdef SIP_THREAD_SAFE
				fast_unlock_synch(0,&glbLockRequestStatisticsMutex);
#endif

				for (dIndex = 0; dIndex < NUM_STAT_RESPONSECLASS;dIndex++)
				{
					glbSipParserResponseClassParserStats[dIndex].recv = 0;
					glbSipParserResponseClassParserStats[dIndex].sendInitial= 0;
					glbSipParserResponseClassParserStats[dIndex].sendRetry= 0;
				}

				for (dIndex = 0; dIndex < NUM_STAT_RESPONSECODE;dIndex++)
				{
					glbSipParserResponseCodeParserStats[dIndex].recv = 0;
					glbSipParserResponseCodeParserStats[dIndex].sendInitial= 0;
					glbSipParserResponseCodeParserStats[dIndex].sendRetry= 0;
					glbSipParserResponseCodeParserStats[dIndex].dGiveCallback=SipFail;
				}
#endif

				break;
			case SIP_STAT_API_COUNT :
				glbSipParserApiCount=0;
				break;
			case SIP_STAT_API_REQ_PARSED :
				glbSipParserReqParsed=0;
				break;

			case SIP_STAT_SECOND_API_REQ_PARSED :
				glbSipParserSecondLevelReqParsed=0;
				break;

			case SIP_STAT_API_RESP_PARSED :
				glbSipParserRespParsed=0;
				break;
			case SIP_STAT_API_REQ_SENT :
				glbSipParserReqSent=0;
				break;
			case SIP_STAT_API_RESP_SENT :
				glbSipParserRespSent=0;
				break;

#ifdef SIP_MIB
			case SIP_STAT_API_REQUEST:
				sip_hashInit (&glbstatHash, __sip_HashFunction,\
					__sip_stringKeyCompareFunction, \
					__sip_hashElementFreeFunction, __sip_hashKeyFreeFunction, \
					NUM_BUCKETS, NUM_ELEMENTS, err);

#ifdef SIP_THREAD_SAFE
				fast_lock_synch(0,&glbLockRequestStatisticsMutex,\
					FAST_MXLOCK_EXCLUSIVE );
#endif
				__sip_AddHashElemForKnownMethod();

				glbSipParserUnknownMethodStats.recv=0;
				glbSipParserUnknownMethodStats.sendInitial=0;
				glbSipParserUnknownMethodStats.sendRetry=0;

#ifdef SIP_THREAD_SAFE
				fast_unlock_synch(0,&glbLockRequestStatisticsMutex);
#endif
				break;
			case SIP_STAT_API_RESPONSECLASS:
				for (dIndex = 0; dIndex < NUM_STAT_RESPONSECLASS;dIndex++)
				{
					glbSipParserResponseClassParserStats[dIndex].recv = 0;
					glbSipParserResponseClassParserStats[dIndex].sendInitial= 0;
					glbSipParserResponseClassParserStats[dIndex].sendRetry= 0;
				}

				break;
			case SIP_STAT_API_RESPONSECODE:
				for (dIndex = 0; dIndex < NUM_STAT_RESPONSECODE;dIndex++)
				{
					glbSipParserResponseCodeParserStats[dIndex].recv = 0;
					glbSipParserResponseCodeParserStats[dIndex].sendInitial= 0;
					glbSipParserResponseCodeParserStats[dIndex].sendRetry= 0;
				}
				break;
#endif

			default:
				*err = E_INV_PARAM;
				return SipFail;
		}
		break;
	case SIP_STAT_TYPE_ERROR:
		switch(scope)
		{
			case SIP_STAT_ERROR_ALL :
				glbSipParserMemErr=0;
				glbSipParserNetErr=0;
				glbSipParserProtoErr=0;

				glbSipParserSecondLevelProtoErr=0;

				break;
			case SIP_STAT_ERROR_MEM :
				glbSipParserMemErr=0;
				break;
			case SIP_STAT_ERROR_NETWORK :
				glbSipParserNetErr=0;
				break;
			case SIP_STAT_ERROR_PROTOCOL :
				glbSipParserProtoErr=0;
				break;

			case SIP_STAT_SECOND_ERROR_PROTOCOL :
				glbSipParserSecondLevelProtoErr=0;
				break;

			default:
				*err = E_INV_PARAM;
				return SipFail;
		}
		break;
	case SIP_STAT_TYPE_INTERNAL:
		break;
	default:
		*err = E_INV_STATSTYPE;
		return SipFail;
	}
*err = E_NO_ERROR;
return SipSuccess;
#endif
}

/*****************************************************************
** FUNCTION:sip_getStatistics
**
**
** DESCRIPTION: Get the statistics for the a given type and scope
*******************************************************************/

SipBool sip_getStatistics
#ifdef ANSI_PROTO
 (SIP_U8bit dType,SIP_U8bit scope, SIP_U8bit reset, SIP_Pvoid pStats, SipError *err)
#else
	(dType, scope, reset, pStats, err)
	SIP_U8bit dType;
	SIP_U8bit scope;
	SIP_U8bit reset;
	SIP_Pvoid pStats;
	SipError *err;
#endif
{
#ifndef SIP_STATISTICS
	SIP_U8bit dummy_dType,dummy_scope,dummy_reset;
	SIP_Pvoid dummy_stats;
	dummy_dType = dType;
	dummy_scope = scope;
	dummy_reset = reset;
	dummy_stats = pStats;
	*err =  E_STATS_DISABLED;
	return SipFail;
#else
	switch(dType)
	{
		case SIP_STAT_TYPE_API	:
			switch(scope)
			{
				case SIP_STAT_API_COUNT :
					*((SIP_U32bit *)pStats)=glbSipParserApiCount;
					if(reset == SIP_STAT_RESET)
						glbSipParserApiCount = 0;
					break;
				case SIP_STAT_API_REQ_PARSED :
					*((SIP_U32bit *)pStats)=glbSipParserReqParsed;
					if(reset == SIP_STAT_RESET)
						glbSipParserReqParsed = 0;
					break;

				case SIP_STAT_SECOND_API_REQ_PARSED :
					*((SIP_U32bit *)pStats)=glbSipParserSecondLevelReqParsed;
					if(reset == SIP_STAT_RESET)
						glbSipParserSecondLevelReqParsed = 0;
					break;

				case SIP_STAT_API_RESP_PARSED :
					*((SIP_U32bit *)pStats)=glbSipParserRespParsed;
					if(reset == SIP_STAT_RESET)
						glbSipParserRespParsed = 0;
					break;
				case SIP_STAT_API_REQ_SENT :
					*((SIP_U32bit *)pStats)=glbSipParserReqSent;
					if(reset == SIP_STAT_RESET)
						glbSipParserReqSent = 0;
					break;
				case SIP_STAT_API_RESP_SENT :
					*((SIP_U32bit *)pStats)=glbSipParserRespSent;
					if(reset == SIP_STAT_RESET)
						glbSipParserRespSent = 0;
					break;

				default:
					*err = E_INV_PARAM;
					return SipFail;
			}
			break;
		case SIP_STAT_TYPE_ERROR:
			switch(scope)
			{
				case SIP_STAT_ERROR_MEM :
					*((SIP_U32bit *)pStats)=glbSipParserMemErr;
					if(reset == SIP_STAT_RESET)
						glbSipParserMemErr = 0;
					break;
				case SIP_STAT_ERROR_NETWORK :
					*((SIP_U32bit *)pStats)=glbSipParserNetErr;
					if(reset == SIP_STAT_RESET)
						glbSipParserNetErr = 0;
					break;
				case SIP_STAT_ERROR_PROTOCOL :
					*((SIP_U32bit *)pStats)=glbSipParserProtoErr;
					if(reset == SIP_STAT_RESET)
						glbSipParserProtoErr = 0;
					break;

				case SIP_STAT_SECOND_ERROR_PROTOCOL :
					*((SIP_U32bit *)pStats)=glbSipParserSecondLevelProtoErr;
					if(reset == SIP_STAT_RESET)
						glbSipParserSecondLevelProtoErr = 0;
					break;

				default:
					*err = E_INV_PARAM;
					return SipFail;
			}
			break;
		case SIP_STAT_TYPE_INTERNAL:
		default:
			*err = E_INV_PARAM;
			return SipFail;
	}
	*err = E_NO_ERROR;
	return SipSuccess;
#endif
}

#ifdef SIP_MIB
/*****************************************************************
** FUNCTION:sip_getProtocalVersion
**
**
** DESCRIPTION: Get the protocol version of the SIP Stack
*******************************************************************/
SipBool sip_getProtoVersion
#ifdef ANSI_PROTO
(SIP_S8bit** ppVersion)
#else
(ppVersion)
SIP_S8bit** ppVersion;
#endif
{
	*ppVersion = sip_strdup(SIP_PROTOVERSION, NON_SPECIFIC_MEM_ID);
	return SipSuccess;
}



/*****************************************************************
** FUNCTION:sip_mib_getStatistics
**
**
** DESCRIPTION: Get the mib statistics for the a given type and scope
*******************************************************************/

SipBool sip_mib_getStatistics
#ifdef ANSI_PROTO
 (SIP_U8bit dScope, SIP_S8bit* pType, SIP_U8bit reset, SIP_Pvoid pStats, SipError *pErr)
#else
	(dScope, pType, reset, pStats, pErr)
	SIP_U8bit dScope;
	SIP_S8bit* pType;
	SIP_U8bit reset;
	SIP_Pvoid pStats;
	SipError *pErr;
#endif
{

	SipStatParam *pTempStatParam;
	SipStatParam dStats;
	SipHashStats *pStatInfo;
	SIP_U32bit dCodeNum;
	SIP_U8bit dStatsType;
	pTempStatParam = (SipStatParam*) pStats;
	switch(dScope)
	{
		case SIP_STAT_API_REQUEST:
#ifdef SIP_THREAD_SAFE
			fast_lock_synch(0,&glbLockRequestStatisticsMutex,\
				FAST_MXLOCK_EXCLUSIVE);
#endif
			if(strcasecmp(pType, "UNKNOWN") ==0)
			{
				pTempStatParam->recv=glbSipParserUnknownMethodStats.recv;
				pTempStatParam->sendInitial=glbSipParserUnknownMethodStats.\
					sendInitial;
				pTempStatParam->sendRetry=glbSipParserUnknownMethodStats.\
					sendRetry;
				if ( SIP_STAT_RESET == reset )
				{
					glbSipParserUnknownMethodStats.recv = 0;
					glbSipParserUnknownMethodStats.sendInitial= 0;
					glbSipParserUnknownMethodStats.sendRetry = 0;
				}
			}
			else
			{

				pStatInfo = (SipHashStats *)sip_hashFetch \
					(&glbstatHash,pType);
				if ( SIP_NULL == pStatInfo )
				{
#ifdef SIP_THREAD_SAFE
					fast_unlock_synch(0,&glbLockRequestStatisticsMutex);
#endif
					*pErr = E_INV_PARAM;
					return SipFail;
				}
				pTempStatParam->recv=pStatInfo->dStats.recv;
				pTempStatParam->sendInitial=pStatInfo->dStats.sendInitial;
				pTempStatParam->sendRetry=pStatInfo->dStats.sendRetry;
				if ( SIP_STAT_RESET == reset )
				{
					pStatInfo->dStats.recv = 0;
					pStatInfo->dStats.sendInitial= 0;
					pStatInfo->dStats.sendRetry = 0;
				}
				sip_hashRelease(&glbstatHash, (SIP_Pvoid)(pType));
			}

#ifdef SIP_THREAD_SAFE
			fast_unlock_synch(0,&glbLockRequestStatisticsMutex);
#endif
			break;

		case SIP_STAT_API_RESPONSECLASS:

			if ( strcasecmp(pType,"1XX") == 0 )
			{
				dStatsType = SIP_STAT_RESPCLASS_1XX;
			}
			else if ( strcasecmp(pType,"2XX") == 0 )
			{
				dStatsType = SIP_STAT_RESPCLASS_2XX;
			}
			else if ( strcasecmp(pType,"3XX") == 0 )
			{
				dStatsType = SIP_STAT_RESPCLASS_3XX;
			}
			else if ( strcasecmp(pType,"4XX") == 0 )
			{
				dStatsType = SIP_STAT_RESPCLASS_4XX;
			}
			else if ( strcasecmp(pType,"5XX") == 0 )
			{
				dStatsType = SIP_STAT_RESPCLASS_5XX;
			}
			else if ( strcasecmp(pType,"6XX") == 0 )
			{
				dStatsType = SIP_STAT_RESPCLASS_6XX;
			}
			else if ( strcasecmp(pType,"7XX") == 0 )
			{
				dStatsType = SIP_STAT_RESPCLASS_7XX;
			}
			else if ( strcasecmp(pType,"8XX") == 0 )
			{
				dStatsType = SIP_STAT_RESPCLASS_8XX;
			}
			else if ( strcasecmp(pType,"9XX") == 0 )
			{
				dStatsType = SIP_STAT_RESPCLASS_9XX;
			}
			else
			{
				*pErr = E_INV_PARAM;
				return SipFail;
			}

			dStats = glbSipParserResponseClassParserStats[dStatsType];
			pTempStatParam->recv=dStats.recv;
			pTempStatParam->sendInitial=dStats.sendInitial;
			pTempStatParam->sendRetry=dStats.sendRetry;

			if(reset == SIP_STAT_RESET)
			{
				glbSipParserResponseClassParserStats[dStatsType].recv = 0;
				glbSipParserResponseClassParserStats[dStatsType].sendInitial= 0;
				glbSipParserResponseClassParserStats[dStatsType].sendRetry = 0;
			}
			break;

		case SIP_STAT_API_RESPONSECODE:
			dCodeNum = STRTOU32CAP(pType,pErr);
			if ( *pErr != E_NO_ERROR )
			{
				*pErr = E_INV_PARAM;
				return SipFail;
			}
			dStats = glbSipParserResponseCodeParserStats[dCodeNum];
			pTempStatParam->recv=dStats.recv;
			pTempStatParam->sendInitial=dStats.sendInitial;
			pTempStatParam->sendRetry=dStats.sendRetry;
			if ( dCodeNum < NUM_STAT_RESPONSECODE )
			{
				if(reset == SIP_STAT_RESET)
				{
					glbSipParserResponseCodeParserStats\
							[dCodeNum].recv = 0;
					glbSipParserResponseCodeParserStats\
							[dCodeNum].sendInitial= 0;
					glbSipParserResponseCodeParserStats\
							[dCodeNum].sendRetry = 0;
				}
			}
			break;
		default:
			*pErr = E_INV_PARAM;
			return SipFail;
		}

	*pErr = E_NO_ERROR;
	return SipSuccess;
}


/*****************************************************************
** FUNCTION:sip_mib_configureStatistics
**
**
** DESCRIPTION: Configure Statistics for a method/response
*******************************************************************/
SipBool sip_mib_configureStatistics
#ifdef ANSI_PROTO
( SIP_U8bit dScope, SIP_S8bit* pType, SIP_Pvoid pStats, SIP_U8bit disable, SipError *pErr)
#else
(dScope,pType,pStats,disable,pErr)
SIP_U8bit dScope;
SIP_S8bit*	pType;
SIP_Pvoid *pStats;
SIP_U8bit disable;
SipError *pErr;
#endif
{
	SipHashStats* pStatInfo = SIP_NULL;
	SipConfigStats *pConfParam = (SipConfigStats*)pStats;
	switch(dScope)
	{
		case SIP_STAT_API_REQUEST:
#ifdef SIP_THREAD_SAFE
			fast_lock_synch(0,&glbLockRequestStatisticsMutex,\
			FAST_MXLOCK_EXCLUSIVE );
#endif
			if(disable)
			{
				sip_hashRemove(&glbstatHash, (SIP_Pvoid)(pType));
#ifdef SIP_THREAD_SAFE
				fast_unlock_synch(0,&glbLockRequestStatisticsMutex);
#endif
				return SipSuccess;
			}

			/* fetch the hash element */
			pStatInfo = (SipHashStats*)sip_hashFetch (&glbstatHash,pType);
			/* if hash fetch fails */
			if ( SIP_NULL == pStatInfo )
			{
				__sip_initSipHashElement((SipHashStats**)&pStatInfo,pErr);
				if(pConfParam != SIP_NULL)
				{
				if(pStatInfo->pConfig == SIP_NULL)
				{
					pStatInfo->pConfig = (SipConfigStats*)fast_memget(\
						DECODE_MEM_ID,sizeof(SipConfigStats),pErr);
				}
#ifndef SIP_TXN_LAYER
				pStatInfo->pConfig->dMaxRetransCount = pConfParam->\
						dMaxRetransCount;
#endif
				pStatInfo->pConfig->dRetransT1 = pConfParam->dRetransT1;
				}
				__sip_AddHashElem(pType,pStatInfo,pErr);
			}
			else
			{
				if(pConfParam != SIP_NULL)
				{
				if(pStatInfo->pConfig == SIP_NULL)
				{
					pStatInfo->pConfig = (SipConfigStats*)fast_memget(\
						DECODE_MEM_ID,sizeof(SipConfigStats),pErr);
				}
#ifndef SIP_TXN_LAYER
				pStatInfo->pConfig->dMaxRetransCount = pConfParam->\
				dMaxRetransCount;
#endif
				pStatInfo->pConfig->dRetransT1 = pConfParam->dRetransT1;
				}
			}
			sip_hashRelease(&glbstatHash, (SIP_Pvoid)(pType));
#ifdef SIP_THREAD_SAFE
			fast_unlock_synch(0,&glbLockRequestStatisticsMutex);
#endif
			break;

		case SIP_STAT_API_RESPONSE:
				if(disable)
				{
					fast_memfree(0,glbSipParserResponseCodeConfig,SIP_NULL);
					glbSipParserResponseCodeConfig = SIP_NULL;
				}
				if(pConfParam ==SIP_NULL)
				{
					return SipFail;
				}
				/* set the config for response */
				if(glbSipParserResponseCodeConfig == SIP_NULL)
				{
					glbSipParserResponseCodeConfig = (SipConfigStats *) \
					fast_memget (DECODE_MEM_ID, sizeof(SipConfigStats), pErr);
				}
#ifndef SIP_TXN_LAYER
				glbSipParserResponseCodeConfig->dMaxRetransCount = \
				pConfParam->dMaxRetransCount;
#endif
				glbSipParserResponseCodeConfig->dRetransT1 = pConfParam->\
				dRetransT1;
				break;
		case SIP_STAT_API_RESPONSECODE:
			{
				/*User wants to enable/disable invoation of callbacks when 
					a message with a particular response code is decoded/sent*/
				SIP_S32bit dResponseCode=0;
				
				/*Fetch the response code */
				dResponseCode=atoi(pType);
				if (!dResponseCode)
				{
					/*Could not convert, return SipFail*/
					*pErr=E_INV_PARAM;
					return SipFail;
				}
				
				if (disable)
				{
					/*User wants to disable invocation of allbacks*/
					glbSipParserResponseCodeParserStats[dResponseCode].\
						dGiveCallback = SipFail;
				}
				else
				{
					/*User wants to disable invocation of allbacks*/
					glbSipParserResponseCodeParserStats[dResponseCode].\
						dGiveCallback = SipSuccess;
				}
			}	
			break;
		default:
			*pErr = E_INV_PARAM;
			return SipFail;
	}

	*pErr = E_NO_ERROR;
	return SipSuccess;
}

/*****************************************************************
** FUNCTION:__sip_incrementStats
**
**
** DESCRIPTION:
*******************************************************************/
SipBool __sip_incrementStats
#ifdef ANSI_PROTO
(SIP_S8bit* pKey,SIP_U8bit dType,SIP_U8bit inOut)
#else
(pKey,dType,inOut)
SIP_S8bit* pKey;
SIP_U8bit dType;
SIP_U8bit inOut;
#endif
{
	SipHashStats *pStatInfo = SIP_NULL;
	switch(dType)
	{
		case SIP_STAT_API_REQUEST:
		{
#ifdef SIP_THREAD_SAFE
			fast_lock_synch(0,&glbLockRequestStatisticsMutex,\
			FAST_MXLOCK_EXCLUSIVE );
#endif
			pStatInfo = (SipHashStats*)sip_hashFetch (&glbstatHash,\
			(SIP_Pvoid)pKey);

			if ( SIP_NULL == pStatInfo )
			{
#ifdef SIP_THREAD_SAFE
				fast_unlock_synch(0,&glbLockRequestStatisticsMutex);
#endif
				return SipFail;
			}
			switch(inOut)
			{
				case SIP_STAT_IN:
					pStatInfo->dStats.recv++;
					break;

				case SIP_STAT_OUT:
					pStatInfo->dStats.sendInitial++;
					break;

				case SIP_STAT_RETRY:
					pStatInfo->dStats.sendRetry++;
					break;

				default:
					break;
			}

			sip_hashRelease(&glbstatHash, (SIP_Pvoid)(pKey));
#ifdef SIP_THREAD_SAFE
			fast_unlock_synch(0,&glbLockRequestStatisticsMutex);
#endif
			break;
		}
		default:
			return SipFail;
	}
	return SipSuccess;
}

/*****************************************************************
** FUNCTION:__sip_decrementStats
**
**
** DESCRIPTION:
*******************************************************************/
SipBool __sip_decrementStats
#ifdef ANSI_PROTO
(SIP_S8bit* pKey,SIP_U8bit dType,SIP_U8bit pInOut)
#else
(pKey,dType,pInOut)
SIP_S8bit* pKey;
SIP_U8bit dType;
SIP_U8bit pInOut;
#endif
{
	SipHashStats *pStatInfo = SIP_NULL;
	switch(dType)
	{
		case SIP_STAT_API_REQUEST:
		{
#ifdef SIP_THREAD_SAFE
			fast_lock_synch(0,&glbLockRequestStatisticsMutex,\
			FAST_MXLOCK_EXCLUSIVE );
#endif
			pStatInfo = (SipHashStats *)sip_hashFetch (&glbstatHash,\
			(SIP_Pvoid)pKey);
			if ( SIP_NULL == pStatInfo )
			{
#ifdef SIP_THREAD_SAFE
				fast_unlock_synch(0,&glbLockRequestStatisticsMutex);
#endif
				return SipFail;
			}
			switch(pInOut)
			{
				case SIP_STAT_IN:
					pStatInfo->dStats.recv++;
					break;

				case SIP_STAT_OUT:
					pStatInfo->dStats.sendInitial++;
					break;

				case SIP_STAT_RETRY:
					pStatInfo->dStats.sendRetry++;
					break;

				default:
					break;
			}

			sip_hashRelease(&glbstatHash, (SIP_Pvoid)(pKey));
#ifdef SIP_THREAD_SAFE
			fast_unlock_synch(0,&glbLockRequestStatisticsMutex);
#endif
			break;
		}
		default:
			return SipFail;
	}
	return SipSuccess;
}

/*****************************************************************
** FUNCTION: __sip_HashFunction
**
**
** DESCRIPTION: Calculate the Hash Value
*******************************************************************/
SIP_U32bit __sip_HashFunction
#ifdef ANSI_PROTO
(SIP_Pvoid pName)
#else
(pName)
SIP_Pvoid pName;
#endif
{
    /*unsigned long   h = 0, g;*/
    SIP_U32bit dH =0,dG;
	/*unsigned char* pname = (unsigned char*)pName;*/
	SIP_U8bit* pName1 = (SIP_U8bit*)pName;

    while ( *pName1 )
    {
        dH = ( dH << 4 ) + *pName1++;
        if ( (dG = dH & 0xf0000000) )
            dH ^= dG >> 24;
        dH &= ~dG;
    }
    return dH;
}

/*****************************************************************
** FUNCTION: __sip_stringKeyCompareFunction
**
**
** DESCRIPTION: Key Compare functions
*******************************************************************/
SIP_U8bit __sip_stringKeyCompareFunction
#ifdef ANSI_PROTO
(SIP_Pvoid pKey1, SIP_Pvoid pKey2)
#else
(pKey1,pKey2)
SIP_Pvoid pKey1;
SIP_Pvoid pKey2;
#endif
{
	return strcmp((SIP_S8bit*) pKey1, (SIP_S8bit*) pKey2);
}

/*****************************************************************
** FUNCTION: __sip_hashElementFreeFunction
**
**
** DESCRIPTION:
*******************************************************************/
void __sip_hashElementFreeFunction
#ifdef ANSI_PROTO
(SIP_Pvoid pElement)
#else
(pElement)
SIP_Pvoid pElement;
#endif
{
	__sip_freeSipHashElement((SipHashStats *)pElement);
}

/*****************************************************************
** FUNCTION:__sip_hashKeyFreeFunction
**
**
** DESCRIPTION:
*******************************************************************/
void __sip_hashKeyFreeFunction
#ifdef ANSI_PROTO
(SIP_Pvoid pKey)
#else
(pKey)
SIP_Pvoid pKey;
#endif
{
  SipError err=E_NO_ERROR ;
	fast_memfree(NON_SPECIFIC_MEM_ID,pKey,&err);
}

/*****************************************************************
** FUNCTION:__sip_AddHashElem
**
**
** DESCRIPTION: Add element into the Hash Table
*******************************************************************/
SipBool __sip_AddHashElem
#ifdef ANSI_PROTO
(SIP_Pvoid pKey,SIP_Pvoid pElem, SipError *pErr)
#else
(pKey,pElem,pErr)
SIP_Pvoid pKey;
SIP_Pvoid pElem;
SipError *pErr;
#endif
{
	SipError *pDummyErr;
	pDummyErr = pErr;
	if(pElem == SIP_NULL)
		return SipFail;

	if ( sip_hashAdd(&glbstatHash, ((SIP_Pvoid)pElem), \
			((SIP_Pvoid)STRDUP((SIP_S8bit*)pKey))) != SipSuccess )
	{
		__sip_freeSipHashElement((SipHashStats *)pElem);
		return SipFail;
	}

	return SipSuccess;
}

/*****************************************************************
** FUNCTION:__sip_AddHashElemForKnownMethod
**
**
** DESCRIPTION: Add element into the Hash Table
*******************************************************************/
void __sip_AddHashElemForKnownMethod
#ifdef ANSI_PROTO
(void)
#else
(void)
#endif
{
	SipError dErr;
	SipHashStats *pHashElem = SIP_NULL;
	__sip_initSipHashElement((SipHashStats**)&pHashElem,&dErr);
	__sip_AddHashElem((SIP_Pvoid)"INVITE",pHashElem,&dErr);
	pHashElem = SIP_NULL;
	__sip_initSipHashElement((SipHashStats**)&pHashElem,&dErr);
	__sip_AddHashElem((SIP_Pvoid)"ACK",pHashElem,&dErr);
	pHashElem = SIP_NULL;
	__sip_initSipHashElement((SipHashStats**)&pHashElem,&dErr);
	__sip_AddHashElem((SIP_Pvoid)"BYE",pHashElem,&dErr);
	pHashElem = SIP_NULL;
	__sip_initSipHashElement((SipHashStats**)&pHashElem,&dErr);
	__sip_AddHashElem((SIP_Pvoid)"REFER",pHashElem,&dErr);
	pHashElem = SIP_NULL;
	__sip_initSipHashElement((SipHashStats**)&pHashElem,&dErr);
	__sip_AddHashElem((SIP_Pvoid)"REGISTER",pHashElem,&dErr);
	pHashElem = SIP_NULL;
	__sip_initSipHashElement((SipHashStats**)&pHashElem,&dErr);
	__sip_AddHashElem((SIP_Pvoid)"OPTIONS",pHashElem,&dErr);
	pHashElem = SIP_NULL;
	__sip_initSipHashElement((SipHashStats**)&pHashElem,&dErr);
	__sip_AddHashElem((SIP_Pvoid)"CANCEL",pHashElem,&dErr);
	pHashElem = SIP_NULL;
	__sip_initSipHashElement((SipHashStats**)&pHashElem,&dErr);
	__sip_AddHashElem((SIP_Pvoid)"INFO",pHashElem,&dErr);
	pHashElem = SIP_NULL;
	__sip_initSipHashElement((SipHashStats**)&pHashElem,&dErr);
	__sip_AddHashElem((SIP_Pvoid)"PROPOSE",pHashElem,&dErr);
	pHashElem = SIP_NULL;
	__sip_initSipHashElement((SipHashStats**)&pHashElem,&dErr);
	__sip_AddHashElem((SIP_Pvoid)"PRACK",pHashElem,&dErr);
	pHashElem = SIP_NULL;
	__sip_initSipHashElement((SipHashStats**)&pHashElem,&dErr);
	__sip_AddHashElem((SIP_Pvoid)"SUBSCRIBE",pHashElem,&dErr);
	pHashElem = SIP_NULL;
	__sip_initSipHashElement((SipHashStats**)&pHashElem,&dErr);
	__sip_AddHashElem((SIP_Pvoid)"NOTIFY",pHashElem,&dErr);
	pHashElem = SIP_NULL;
	__sip_initSipHashElement((SipHashStats**)&pHashElem,&dErr);
	__sip_AddHashElem((SIP_Pvoid)"MESSAGE",pHashElem,&dErr);
	pHashElem = SIP_NULL;
	__sip_initSipHashElement((SipHashStats**)&pHashElem,&dErr);
	__sip_AddHashElem((SIP_Pvoid)"COMET",pHashElem,&dErr);
	pHashElem = SIP_NULL;
	__sip_initSipHashElement((SipHashStats**)&pHashElem,&dErr);
	__sip_AddHashElem((SIP_Pvoid)"UPDATE",pHashElem,&dErr);
	pHashElem = SIP_NULL;
	__sip_initSipHashElement((SipHashStats**)&pHashElem,&dErr);
	__sip_AddHashElem((SIP_Pvoid)"PUBLISH",pHashElem,&dErr);
	pHashElem = SIP_NULL;
}


/*****************************************************************
** FUNCTION: __sip_hashFlush
**
**
** DESCRIPTION: Flushed all the elements in the Hash Table
*******************************************************************/
void __sip_hashFlush(void)
{
	SipHashIterator dIterator;
	SIP_Pvoid tempkey;
	SipError err;

	/*Flush the Hash*/
#ifdef SIP_THREAD_SAFE
	fast_lock_synch(0,&glbLockRequestStatisticsMutex,FAST_MXLOCK_EXCLUSIVE);
#endif

	sip_hashInitIterator(&glbstatHash, &dIterator);
	while (dIterator.pCurrentElement != NULL)
	{
		tempkey = dIterator.pCurrentElement->pKey;
		sip_hashNext(&glbstatHash, &dIterator);
		sip_hashRemove(&glbstatHash,(SIP_Pvoid)tempkey);
	}
	sip_hashFree(&glbstatHash,&err);
#ifdef SIP_THREAD_SAFE
	fast_unlock_synch(0,&glbLockRequestStatisticsMutex);
#endif
	return;
}


/*****************************************************************
** FUNCTION:__sip_getConfigStatsForMethod
**
**
** DESCRIPTION: Initialize the Hash Element
**
******************************************************************/
SipBool __sip_getConfigStatsForMethod
#ifdef ANSI_PROTO
(SIP_S8bit* pType,SipConfigStats **ppConfigStat,SipError *pErr)
#else
(pType,ppConfigStat,pErr)
SIP_S8bit* pType;
SipConfigStats **ppConfigStat;
SipError *pErr;
#endif
{
	SipHashStats *pStatInfo = SIP_NULL;
#ifdef SIP_THREAD_SAFE
	fast_lock_synch(0,&glbLockRequestStatisticsMutex,FAST_MXLOCK_EXCLUSIVE);
#endif

	pStatInfo = (SipHashStats*)sip_hashFetch (&glbstatHash,\
				(SIP_Pvoid)pType);
	if(pStatInfo == SIP_NULL)
	{
#ifdef SIP_THREAD_SAFE
		fast_unlock_synch(0,&glbLockRequestStatisticsMutex);
#endif
		return SipFail;

	}
	if(pStatInfo->pConfig == SIP_NULL)
		*ppConfigStat = SIP_NULL;
	else
	{
		*ppConfigStat = pStatInfo->pConfig;
	}
	sip_hashRelease(&glbstatHash, (SIP_Pvoid)(pType));
#ifdef SIP_THREAD_SAFE
	fast_unlock_synch(0,&glbLockRequestStatisticsMutex);
#endif
	*pErr = E_NO_ERROR;
	return SipSuccess;
}

/*****************************************************************
** FUNCTION:__sip_getConfigStatsForResponse
**
**
** DESCRIPTION: Initialize the Hash Element
**
******************************************************************/
SipBool __sip_getConfigStatsForResponse
#ifdef ANSI_PROTO
(SipConfigStats **ppConfigStat)
#else
(ppConfigStat)
SipConfigStats **ppConfigStat;
#endif
{
#ifdef SIP_THREAD_SAFE
	fast_lock_synch(0,&glbLockRequestStatisticsMutex,FAST_MXLOCK_EXCLUSIVE);
#endif
	*ppConfigStat = glbSipParserResponseCodeConfig;
#ifdef SIP_THREAD_SAFE
	fast_unlock_synch(0,&glbLockRequestStatisticsMutex);
#endif
	return SipSuccess;
}


/*****************************************************************
** FUNCTION:__sip_initSipHashElement
**
**
** DESCRIPTION: Initialize the Hash Element
**
******************************************************************/
SipBool __sip_initSipHashElement
#ifdef ANSI_PROTO
(SipHashStats **ppHashElem,SipError *pErr)
#else
(ppHashElem,pErr)
SipHashStats **ppHashElem;
SipError *pErr;
#endif
{
	SipHashStats *pStatInfo;
	*ppHashElem = (SipHashStats *) fast_memget(DECODE_MEM_ID,\
                		sizeof(SipHashStats),pErr);
    if ( *ppHashElem == SIP_NULL )
    {
		*pErr = E_NO_MEM;
		return SipFail;
	}
	pStatInfo = *ppHashElem;
	pStatInfo->dStats.recv =0;
	pStatInfo->dStats.sendInitial =0;
	pStatInfo->dStats.sendRetry =0;
	pStatInfo->pConfig = SIP_NULL;

	*pErr = E_NO_ERROR;
	return SipSuccess;
}
/*****************************************************************
** FUNCTION:__sip_freeSipHashElement
**
**
** DESCRIPTION: Free the Hash Element
**
******************************************************************/
void __sip_freeSipHashElement
#ifdef ANSI_PROTO
(SipHashStats *pHashElem)
#else
(pHashElem)
SipHashStats *pHashElem;
#endif
{
	if(pHashElem->pConfig != SIP_NULL)
		fast_memfree(DECODE_MEM_ID,pHashElem->pConfig,SIP_NULL);
	fast_memfree(DECODE_MEM_ID,pHashElem,SIP_NULL);
}
#endif
#ifdef __cplusplus
}
#endif

