/******************************************************************************
 ** FUNCTION:
 **	 This file contains the protypes of the functions used to manipulate
 **  Statistics APIs
 ******************************************************************************
 **
 ** FILENAME:
 ** siplist.h
 **
 ** DESCRIPTION:
 **	All modules using SipLists must include this file for the structure
 **	definitions and the function prototypes.
 **
 **
 ** DATE			NAME			REFERENCE		REASON
 ** ----			----			--------		------
 ** 14/01/2000		   Binu K S     		--		 Original
 **
 **
 **	Copyright 1999, Hughes Software Systems, Ltd.
 *****************************************************************************/

#ifndef __SIPSTAT_H_
#define __SIPSTAT_H_

#include "sipcommon.h"
#include "sipstruct.h"
#include  "portlayer.h"
#include "sipfree.h"

#ifdef SIP_MIB
#include "siphash.h"
#include "sipfree.h"
#endif

#ifdef SIP_TXN_LAYER
#include "siptxnstruct.h"
#endif


/* Ensure Names are not mangled by C++ compilers */
#ifdef __cplusplus
extern "C" {
#endif


#ifdef SIP_MIB
typedef struct
{
#ifndef SIP_TXN_LAYER
	SIP_U16bit 	dMaxRetransCount;
#endif
	SIP_U32bit dRetransT1;
}SipConfigStats;

typedef struct
{
	SipStatParam dStats;
	SipConfigStats  *pConfig;
}SipHashStats;

#ifdef NUM_ELEMENTS
#undef NUM_ELEMENTS
#endif

#ifdef NUM_BUCKETS
#undef NUM_BUCKETS
#endif

#define NUM_ELEMENTS	1000
#define NUM_BUCKETS	40

#define NUM_STAT_METHODS	15
#define NUM_STAT_RESPONSECLASS	10
#define NUM_STAT_RESPONSECODE	1000
#define SIP_STAT_METHOD_INVITE	1
#define SIP_STAT_METHOD_ACK	2
#define SIP_STAT_METHOD_BYE	3
#define SIP_STAT_METHOD_REFER	4
#define SIP_STAT_METHOD_REGISTER	5
#define SIP_STAT_METHOD_OPTIONS	6
#define SIP_STAT_METHOD_CANCEL	7
#define SIP_STAT_METHOD_INFO	8
#define SIP_STAT_METHOD_PROPOSE	9
#define SIP_STAT_METHOD_PRACK	10
#define SIP_STAT_METHOD_SUBSCRIBE	11
#define SIP_STAT_METHOD_NOTIFY	12
#define SIP_STAT_METHOD_MESSAGE	13
#define SIP_STAT_METHOD_COMET	14
#define SIP_STAT_METHOD_UPDATE	15
#define SIP_STAT_METHOD_UNKNOWN	16
#define SIP_STAT_RESPCLASS_1XX	1
#define SIP_STAT_RESPCLASS_2XX	2
#define SIP_STAT_RESPCLASS_3XX	3
#define SIP_STAT_RESPCLASS_4XX	4
#define SIP_STAT_RESPCLASS_5XX	5
#define SIP_STAT_RESPCLASS_6XX	6
#define SIP_STAT_RESPCLASS_7XX	7
#define SIP_STAT_RESPCLASS_8XX	8
#define SIP_STAT_RESPCLASS_9XX	9

extern SipStatParam glbSipParserResponseClassParserStats[NUM_STAT_RESPONSECLASS];
extern SipStatParam glbSipParserResponseCodeParserStats[NUM_STAT_RESPONSECODE];

extern SipConfigStats *glbSipParserResponseCodeConfig;
extern SipStatParam glbSipParserUnknownMethodStats;
#endif

extern SIP_U32bit glbSipParserSecondLevelReqParsed;
extern SIP_U32bit glbSipParserSecondLevelProtoErr;

extern SIP_U32bit glbSipParserMemErr;
extern SIP_U32bit glbSipParserNetErr;
extern SIP_U32bit glbSipParserProtoErr;
extern SIP_U32bit glbSipParserApiCount;
extern SIP_U32bit glbSipParserReqParsed;
extern SIP_U32bit glbSipParserRespParsed;
extern SIP_U32bit glbSipParserReqSent;
extern SIP_U32bit glbSipParserRespSent;
#ifdef SIP_THREAD_SAFE
extern synch_id_t glbLockStatisticsMutex;
#endif
/* Types of statistics */
#define SIP_STAT_TYPE_API		0
#define SIP_STAT_TYPE_ERROR		1
#define SIP_STAT_TYPE_INTERNAL	2
/* Counters recording API statistics */
#define SIP_STAT_API_ALL 			0
#define SIP_STAT_API_COUNT			1
#define SIP_STAT_API_REQ_PARSED		2
#define SIP_STAT_API_RESP_PARSED	3
#define SIP_STAT_API_REQ_SENT		4
#define SIP_STAT_API_RESP_SENT		5

#ifdef SIP_MIB
#define SIP_STAT_API_REQUEST		6
#define SIP_STAT_API_RESPONSECLASS	7
#define SIP_STAT_API_RESPONSECODE	8
#define SIP_STAT_API_RESPONSE	 	9
#define SIP_STAT_IN					10
#define SIP_STAT_OUT 				11
#define SIP_STAT_RETRY 				12
#define SIP_PROTOVERSION			"2.0"
#endif

#define SIP_STAT_SECOND_API_REQ_PARSED	11
#define SIP_STAT_SECOND_ERROR_PROTOCOL 	12

/* Counters recording SIP_ERROR statistics */
#define SIP_STAT_ERROR_ALL			0
#define SIP_STAT_ERROR_MEM			1
#define SIP_STAT_ERROR_NETWORK		2
#define SIP_STAT_ERROR_PROTOCOL		3
/* Macros for incerementing and decrmenting counters */
#ifdef SIP_STATISTICS
#ifdef SIP_THREAD_SAFE
#define INC_ERROR_MEM		{\
							fast_lock_synch(0,&glbLockStatisticsMutex, \
								FAST_MXLOCK_EXCLUSIVE ); \
							glbSipParserMemErr++;\
							fast_unlock_synch(0,&glbLockStatisticsMutex );\
							}

#define DEC_ERROR_MEM		{\
							fast_lock_synch(0,&glbLockStatisticsMutex, \
								FAST_MXLOCK_EXCLUSIVE ); \
							glbSipParserMemErr--; \
							fast_unlock_synch(0,&glbLockStatisticsMutex );\
							}
#define INC_ERROR_NETWORK	{ \
							fast_lock_synch(0,&glbLockStatisticsMutex, \
								FAST_MXLOCK_EXCLUSIVE ); \
								glbSipParserNetErr++; \
							fast_unlock_synch(0,&glbLockStatisticsMutex );\
							}
#define DEC_ERROR_NETWORK	{	\
							fast_lock_synch(0,&glbLockStatisticsMutex, \
								FAST_MXLOCK_EXCLUSIVE ); \
								glbSipParserNetErr--; \
							fast_unlock_synch(0,&glbLockStatisticsMutex );\
							}
#define INC_ERROR_PROTOCOL	{ \
							fast_lock_synch(0,&glbLockStatisticsMutex, \
								FAST_MXLOCK_EXCLUSIVE ); \
								glbSipParserProtoErr++; \
							fast_unlock_synch(0,&glbLockStatisticsMutex );\
							}
#define DEC_ERROR_PROTOCOL	{ \
							fast_lock_synch(0,&glbLockStatisticsMutex, \
								FAST_MXLOCK_EXCLUSIVE ); \
								glbSipParserProtoErr--; \
							fast_unlock_synch(0,&glbLockStatisticsMutex );\
							}
#define INC_SECOND_ERROR_PROTOCOL	{ \
							fast_lock_synch(0,&glbLockStatisticsMutex, \
								FAST_MXLOCK_EXCLUSIVE ); \
								glbSipParserSecondLevelProtoErr++; \
							fast_unlock_synch(0,&glbLockStatisticsMutex );\
							}
#define DEC_SECOND_ERROR_PROTOCOL	{ \
							fast_lock_synch(0,&glbLockStatisticsMutex, \
								FAST_MXLOCK_EXCLUSIVE ); \
								glbSipParserSecondLevelProtoErr--; \
							fast_unlock_synch(0,&glbLockStatisticsMutex );\
							}

#define INC_API_COUNT		{ \
							fast_lock_synch(0,&glbLockStatisticsMutex, \
								FAST_MXLOCK_EXCLUSIVE ); \
								glbSipParserApiCount++; \
							fast_unlock_synch(0,&glbLockStatisticsMutex );\
							}
#define DEC_API_COUNT		{ \
							fast_lock_synch(0,&glbLockStatisticsMutex, \
								FAST_MXLOCK_EXCLUSIVE ); \
								glbSipParserApiCount--; \
							fast_unlock_synch(0,&glbLockStatisticsMutex );\
							}
#define INC_API_REQ_PARSED	{ \
								fast_lock_synch(0,&glbLockStatisticsMutex, \
									FAST_MXLOCK_EXCLUSIVE ); \
								glbSipParserReqParsed++; \
								fast_unlock_synch(0,&glbLockStatisticsMutex );\
							}
#define DEC_API_REQ_PARSED	{ \
								fast_lock_synch(0,&glbLockStatisticsMutex, \
									FAST_MXLOCK_EXCLUSIVE ); \
								glbSipParserReqParsed--; \
								fast_unlock_synch(0,&glbLockStatisticsMutex );\
							}

#define INC_SECOND_API_REQ_PARSED	{ \
								fast_lock_synch(0,&glbLockStatisticsMutex, \
									FAST_MXLOCK_EXCLUSIVE ); \
								glbSipParserSecondLevelReqParsed++; \
								fast_unlock_synch(0,&glbLockStatisticsMutex );\
							}
#define DEC_SECOND_API_REQ_PARSED	{ \
								fast_lock_synch(0,&glbLockStatisticsMutex, \
									FAST_MXLOCK_EXCLUSIVE ); \
								glbSipParserSecondLevelReqParsed--; \
								fast_unlock_synch(0,&glbLockStatisticsMutex );\
							}


#define INC_API_RESP_PARSED	{ \
							fast_lock_synch(0,&glbLockStatisticsMutex, \
								FAST_MXLOCK_EXCLUSIVE ); \
								glbSipParserRespParsed++; \
								fast_unlock_synch(0,&glbLockStatisticsMutex );\
							}
#define DEC_API_RESP_PARSED	{ \
								fast_lock_synch(0,&glbLockStatisticsMutex, \
									FAST_MXLOCK_EXCLUSIVE ); \
								glbSipParserRespParsed--; \
								fast_unlock_synch(0,&glbLockStatisticsMutex );\
							}
#define INC_API_REQ_SENT	{ \
								fast_lock_synch(0,&glbLockStatisticsMutex, \
									FAST_MXLOCK_EXCLUSIVE ); \
								glbSipParserReqSent++;\
								fast_unlock_synch(0,&glbLockStatisticsMutex );\
							}
#define DEC_API_REQ_SENT	{	\
								fast_lock_synch(0,&glbLockStatisticsMutex, \
									FAST_MXLOCK_EXCLUSIVE ); \
								glbSipParserReqSent--;\
								fast_unlock_synch(0,&glbLockStatisticsMutex );\
							}
#define INC_API_RESP_SENT 	{	\
								fast_lock_synch(0,&glbLockStatisticsMutex, \
									FAST_MXLOCK_EXCLUSIVE ); \
								glbSipParserRespSent++;\
								fast_unlock_synch(0,&glbLockStatisticsMutex );\
							}

#ifdef SIP_MIB

/* macros for unknown method stats */
#define INC_API_UNKNOWN_PARSED_IN	{ \
								fast_lock_synch(0,&glbLockStatisticsMutex, \
									FAST_MXLOCK_EXCLUSIVE ); \
								glbSipParserUnknownMethodStats.recv++;\
								fast_unlock_synch(0,&glbLockStatisticsMutex);\
								}
#define INC_API_UNKNOWN_PARSED_OUT	{ \
								fast_lock_synch(0,&glbLockStatisticsMutex, \
									FAST_MXLOCK_EXCLUSIVE ); \
								glbSipParserUnknownMethodStats.sendInitial++;\
								fast_unlock_synch(0,&glbLockStatisticsMutex);\
								}
#define INC_API_UNKNOWN_PARSED_RETRY	{ \
								fast_lock_synch(0,&glbLockStatisticsMutex, \
									FAST_MXLOCK_EXCLUSIVE ); \
								glbSipParserUnknownMethodStats.sendRetry++;\
								fast_unlock_synch(0,&glbLockStatisticsMutex);\
								}
/* Macros for response class */
#define INC_API_RESPCLASS_PARSED_IN(X)	{ \
								fast_lock_synch(0,&glbLockStatisticsMutex, \
									FAST_MXLOCK_EXCLUSIVE ); \
								glbSipParserResponseClassParserStats[X].recv++;\
								fast_unlock_synch(0,&glbLockStatisticsMutex );\
								}
#define DEC_API_RESPCLASS_PARSED_IN(X)	{ \
								fast_lock_synch(0,&glbLockStatisticsMutex, \
									FAST_MXLOCK_EXCLUSIVE ); \
								glbSipParserResponseClassParserStats[X].recv--;\
								fast_unlock_synch(0,&glbLockStatisticsMutex );\
								}
#define INC_API_RESPCLASS_PARSED_OUT(X)	{ \
								fast_lock_synch(0,&glbLockStatisticsMutex, \
									FAST_MXLOCK_EXCLUSIVE ); \
								glbSipParserResponseClassParserStats[X].sendInitial++;\
								fast_unlock_synch(0,&glbLockStatisticsMutex );\
								}
#define DEC_API_RESPCLASS_PARSED_OUT(X)	{ \
								fast_lock_synch(0,&glbLockStatisticsMutex, \
									FAST_MXLOCK_EXCLUSIVE ); \
								glbSipParserResponseClassParserStats[X].sendInitial--;\
								fast_unlock_synch(0,&glbLockStatisticsMutex );\
								}
#define INC_API_RESPCLASS_PARSED_RETRY(X)	{ \
								fast_lock_synch(0,&glbLockStatisticsMutex, \
									FAST_MXLOCK_EXCLUSIVE ); \
								glbSipParserResponseClassParserStats[X].sendRetry++;\
								fast_unlock_synch(0,&glbLockStatisticsMutex );\
								}
#define DEC_API_RESPCLASS_PARSED_RETRY(X)	{ \
								fast_lock_synch(0,&glbLockStatisticsMutex, \
									FAST_MXLOCK_EXCLUSIVE ); \
								glbSipParserResponseClassParserStats[X].sendRetry--;\
								fast_unlock_synch(0,&glbLockStatisticsMutex );\
								}



/* Macros for response codes*/
#define INC_API_RESPCODE_PARSED_IN(X)	{ \
										fast_lock_synch(0,&glbLockStatisticsMutex, \
											FAST_MXLOCK_EXCLUSIVE ); \
										glbSipParserResponseCodeParserStats[X].recv++;\
										fast_unlock_synch(0,&glbLockStatisticsMutex );\
										}
#define DEC_API_RESPCODE_PARSED_IN(X)	{ \
										fast_lock_synch(0,&glbLockStatisticsMutex, \
											FAST_MXLOCK_EXCLUSIVE ); \
										glbSipParserResponseCodeParserStats[X].recv--;\
										fast_unlock_synch(0,&glbLockStatisticsMutex );\
										}
#define INC_API_RESPCODE_PARSED_OUT(X)	{ \
										fast_lock_synch(0,&glbLockStatisticsMutex, \
											FAST_MXLOCK_EXCLUSIVE ); \
										glbSipParserResponseCodeParserStats[X].sendInitial++;\
										fast_unlock_synch(0,&glbLockStatisticsMutex );\
										}
#define DEC_API_RESPCODE_PARSED_OUT(X)	{ \
										fast_lock_synch(0,&glbLockStatisticsMutex, \
											FAST_MXLOCK_EXCLUSIVE ); \
										glbSipParserResponseCodeParserStats[X].sendInitial--;\
										fast_unlock_synch(0,&glbLockStatisticsMutex );\
										}
#define INC_API_RESPCODE_PARSED_RETRY(X)	{ \
										fast_lock_synch(0,&glbLockStatisticsMutex, \
											FAST_MXLOCK_EXCLUSIVE ); \
										glbSipParserResponseCodeParserStats[X].sendRetry++;\
										fast_unlock_synch(0,&glbLockStatisticsMutex );\
										}
#define DEC_API_RESPCODE_PARSED_RETRY(X)	{ \
										fast_lock_synch(0,&glbLockStatisticsMutex, \
											FAST_MXLOCK_EXCLUSIVE ); \
										glbSipParserResponseCodeParserStats[X].sendRetry--;\
										fast_unlock_synch(0,&glbLockStatisticsMutex );\
										}
#endif

#else
#define INC_ERROR_MEM		{\
							glbSipParserMemErr++;\
							}

#define DEC_ERROR_MEM		{\
							glbSipParserMemErr--; \
							}
#define INC_ERROR_NETWORK	{ \
								glbSipParserNetErr++; \
							}
#define DEC_ERROR_NETWORK	{	\
								glbSipParserNetErr--; \
							}
#define INC_ERROR_PROTOCOL	{ \
								glbSipParserProtoErr++; \
							}
#define DEC_ERROR_PROTOCOL	{ \
								glbSipParserProtoErr--; \
							}
#define INC_SECOND_ERROR_PROTOCOL	{ \
										glbSipParserSecondLevelProtoErr++; \
									}
#define DEC_SECOND_ERROR_PROTOCOL	{ \
										glbSipParserSecondLevelProtoErr--; \
									}

#define INC_API_COUNT		{ \
								glbSipParserApiCount++; \
							}
#define DEC_API_COUNT		{ \
								glbSipParserApiCount--; \
							}
#define INC_API_REQ_PARSED	{ \
								glbSipParserReqParsed++; \
							}
#define DEC_API_REQ_PARSED	{ \
								glbSipParserReqParsed--; \
							}
#define INC_SECOND_API_REQ_PARSED	{ \
										glbSipParserSecondLevelReqParsed++; \
									}
#define DEC_SECOND_API_REQ_PARSED	{ \
										glbSipParserSecondLevelReqParsed--; \
									}

#define INC_API_RESP_PARSED	{ \
								glbSipParserRespParsed++; \
							}
#define DEC_API_RESP_PARSED	{ \
								glbSipParserRespParsed--; \
							}
#define INC_API_REQ_SENT	{ \
								glbSipParserReqSent++;\
							}
#define DEC_API_REQ_SENT	{	\
								glbSipParserReqSent--;\
							}
#define INC_API_RESP_SENT 	{	\
								glbSipParserRespSent++;\
							}
#ifdef SIP_MIB

/* Macros for unknown method stats*/
#define INC_API_UNKNOWN_PARSED_IN	{ \
								glbSipParserUnknownMethodStats.recv++;\
								}
#define INC_API_UNKNOWN_PARSED_OUT	{ \
								glbSipParserUnknownMethodStats.sendInitial++;\
								}
#define INC_API_UNKNOWN_PARSED_RETRY	{ \
								glbSipParserUnknownMethodStats.sendRetry++;\
								}
/* Macros for response class*/
#define INC_API_RESPCLASS_PARSED_IN(X)	{ \
								glbSipParserResponseClassParserStats[X].recv++;\
								}
#define DEC_API_RESPCLASS_PARSED_IN(X)	{ \
								glbSipParserResponseClassParserStats[X].recv--;\
								}
#define INC_API_RESPCLASS_PARSED_OUT(X)	{ \
								glbSipParserResponseClassParserStats[X].sendInitial++;\
								}
#define DEC_API_RESPCLASS_PARSED_OUT(X)	{ \
								glbSipParserResponseClassParserStats[X].sendInitial--;\
								}
#define INC_API_RESPCLASS_PARSED_RETRY(X)	{ \
								glbSipParserResponseClassParserStats[X].sendRetry++;\
								}
#define DEC_API_RESPCLASS_PARSED_RETRY(X)	{ \
								glbSipParserResponseClassParserStats[X].sendRetry--;\
								}


/* Macros for response code*/
#define INC_API_RESPCODE_PARSED_IN(X)	{ \
										glbSipParserResponseCodeParserStats[X].recv++;\
										}
#define DEC_API_RESPCODE_PARSED_IN(X)	{ \
										glbSipParserResponseCodeParserStats[X].recv--;\
										}
#define INC_API_RESPCODE_PARSED_OUT(X)	{ \
										glbSipParserResponseCodeParserStats[X].sendInitial++;\
										}
#define DEC_API_RESPCODE_PARSED_OUT(X)	{ \
										glbSipParserResponseCodeParserStats[X].sendInitial--;\
										}
#define INC_API_RESPCODE_PARSED_RETRY(X)	{ \
										glbSipParserResponseCodeParserStats[X].sendRetry++;\
										}
#define DEC_API_RESPCODE_PARSED_RETRY(X)	{ \
										glbSipParserResponseCodeParserStats[X].sendRetry--;\
										}
#endif
#endif

#else

#define INC_ERROR_MEM		;
#define DEC_ERROR_MEM		;
#define INC_ERROR_NETWORK	;
#define DEC_ERROR_NETWORK	;
#define INC_ERROR_PROTOCOL	;
#define DEC_ERROR_PROTOCOL	;
#define INC_API_COUNT		;
#define DEC_API_COUNT		;
#define INC_API_REQ_PARSED	;
#define DEC_API_REQ_PARSED	;
#define INC_API_RESP_PARSED	;
#define DEC_API_RESP_PARSED	;
#define INC_API_REQ_SENT	;
#define DEC_API_REQ_SENT	;
#define INC_API_RESP_SENT	;
#ifdef SIP_MIB
#define INC_API_UNKNOWN_PARSED_IN	;
#define INC_API_UNKNOWN_PARSED_OUT	;
#define INC_API_UNKNOWN_PARSED_RETRY ;
#define INC_API_RESPCLASS_PARSED_IN(X)	;
#define DEC_API_RESPCLASS_PARSED_IN(X)	;
#define INC_API_RESPCLASS_PARSED_OUT(X)	;
#define DEC_API_RESPCLASS_PARSED_OUT(X)	;
#define INC_API_RESPCLASS_PARSED_RETRY(X)	;
#define DEC_API_RESPCLASS_PARSED_RETRY(X)	;
#define INC_API_RESPCODE_PARSED_IN(X)	;
#define DEC_API_RESPCODE_PARSED_IN(X)	;
#define INC_API_RESPCODE_PARSED_OUT(X)	;
#define DEC_API_RESPCODE_PARSED_OUT(X)	;
#define INC_API_RESPCODE_PARSED_RETRY(X)	;
#define DEC_API_RESPCODE_PARSED_RETRY(X)	;
#endif

#endif
/* Used in the getStatistics call to reset the counter */
#define SIP_STAT_RESET		0
#define SIP_STAT_NO_RESET	1

/******************************************************************************
 ** FUNCTION: 		sip_initStatistics
 **
 ** DESCRIPTION: 	This API is used to initialize the statistics.
 **
 ** PARAMETERS:
 **	 type 	(IN)	: The type of the statsitics to be initialized
 **	 scope  (IN) 	: The scope of the statsitics to be initialized
 **  err 	(OUT)	: Error variable returned in case of failure.
 **
 ******************************************************************************/
extern SipBool sip_initStatistics _ARGS_((SIP_U8bit type,SIP_U8bit scope, \
	SipError *err));


/******************************************************************************
 ** FUNCTION: 		sip_getStatistics
 **
 ** DESCRIPTION: 	This API is used to get the statistics.
 **
 ** PARAMETERS:
 **	 type 	(IN)	: The type of the statsitics to be extracted
 **	 scope  (IN) 	: The scope of the statsitics to be extracted
 **	 reset 	(IN) 	: This indicates whether the statitics need to be
 **						reset after extracting them
 **	 stats	(OUT) 	:  The statistics are reported into this parameter
 **  err 	(OUT)	: Error variable returned in case of failure.
 **
 ******************************************************************************/
extern SipBool sip_getStatistics _ARGS_((SIP_U8bit type,SIP_U8bit scope,\
	SIP_U8bit reset, SIP_Pvoid stats, SipError *err));



#ifdef SIP_MIB
/******************************************************************************
 ** FUNCTION: 		sip_mib_getStatistics
 **
 ** DESCRIPTION: 	This API is used to get the statistics.
 **
 ** PARAMETERS:
 **	 dType 	(IN)	: The type of the statsitics to be extracted
 **	 dScope  (IN) 	: The scope of the statsitics to be extracted
 **	 reset 	(IN) 	: This indicates whether the statitics need to be
 **						reset after extracting them
 **	 pStats	(OUT) 	: The statistics are reported into this parameter
 **  pErr 	(OUT)	: Error variable returned in case of failure.
 **
 ******************************************************************************/
extern SipBool sip_mib_getStatistics _ARGS_ ((SIP_U8bit dScope, \
	SIP_S8bit* pType, SIP_U8bit reset, SIP_Pvoid pStats, SipError *pErr));



/******************************************************************************
 ** FUNCTION: 		sip_mib_configureStatistics
 **
 ** DESCRIPTION: 	This API is used to get the statistics.
 **
 ** PARAMETERS:
 **	 dType 	(IN)	: The type of the statsitics to be configured
 **	 dScope  (IN) 	: The scope of the statsitics to be configured
 **	 pStats	(OUT) 	: The timer configuration is done using this parameter
 **	 disbale (IN) 	: This indicates whether the statitics need to be
 **						disabled or not
 **  pErr 	(OUT)	: Error variable returned in case of failure.
 **
 ******************************************************************************/
extern SipBool sip_mib_configureStatistics _ARGS_((SIP_U8bit dScope,\
	SIP_S8bit* pType, SIP_Pvoid pStats, SIP_U8bit disable, SipError *pErr));



/******************************************************************************
 ** FUNCTION: 		sip_getProtoVersion
 **
 ** DESCRIPTION: 	This is the function to get teh SIP version supported by
 **					the stack.
 **
 ** PARAMETERS:
 **	 ppVersion (IN)	: The SIP Version isreturned in this parameter.
 **
 ******************************************************************************/
extern SipBool sip_getProtoVersion _ARGS_ ((SIP_S8bit** ppVersion));



/*Internal APIs*/
extern SipBool __sip_incrementStats(SIP_S8bit* pKey,SIP_U8bit dType,\
	SIP_U8bit inOut);
extern SipBool __sip_decrementStats(SIP_S8bit* pKey,SIP_U8bit dType,\
	SIP_U8bit inOut);
SipBool __sip_AddHashElem(SIP_Pvoid pKey,SIP_Pvoid pElem,SipError *pErr);
SIP_U8bit __sip_stringKeyCompareFunction (SIP_Pvoid pKey1, SIP_Pvoid pKey2);
SIP_U32bit __sip_HashFunction(SIP_Pvoid pName );
SipBool __sip_initSipHashElement (SipHashStats **ppHashElem,SipError *pErr);
void __sip_hashKeyFreeFunction (SIP_Pvoid pKey);
void __sip_hashElementFreeFunction(SIP_Pvoid pElement);
void __sip_hashFlush(void);
void __sip_freeSipHashElement (SipHashStats *pHashElem);
void __sip_AddHashElemForKnownMethod(void);
extern SipBool __sip_getConfigStatsForMethod(SIP_S8bit* pType,\
	SipConfigStats **ppConfigStat,SipError *pErr);
extern SipBool __sip_getConfigStatsForResponse(SipConfigStats **ppConfigStat);
extern SIP_U32bit __sip_getCountOfStatType(SIP_U8bit dType,SipError *pErr);
extern SipBool __sip_getKeyOfStatTypeAtIndex(SIP_U8bit dType,SIP_Pvoid* ppKey,\
	SIP_U32bit index, SipError *pErr);
#endif

/* Ensure Names are not mangled by C++ compilers */
#ifdef __cplusplus
}
#endif

#endif
