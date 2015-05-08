
/*****************************************************************************************
 ** FUNCTION:
 **             This file implements Trace/Error routines
 **
 ******************************************************************************************
 **
 ** FILENAME:
 ** sipcommon.h
 **
 ** DESCRIPTION:
 ** Trace implementation is here. 
 **
 ** DATE         NAME                    REFERENCE               REASON
 ** ----         ----                    --------                ------
 ** 19-11-99   Arjun Roychowdhury                                  Creation
 **
 **     Copyright 1999, Hughes Software Systems, Ltd. 
 *******************************************************************************************/


#include "sipcommon.h"
#include "siptrace.h"
#include "portlayer.h"







SipTraceLevel curSipLevel = SIP_UndefTrace;
SipTraceType  curSipType;
SipErrorLevel curSipErrLevel = SIP_UndefError;

/*****************************************************************
** FUNCTION: sip_getTraceLevel
** Gets default Trace Level
**
** DESCRIPTION:
*****************************************************************/
SipBool sip_getTraceLevel
#ifdef ANSI_PROTO
	(SipTraceLevel  *lev,
	 SipError	*err)
#else
	(lev, err)
	SipTraceLevel	*lev;
	SipError	*err;
#endif
{
#ifndef SIP_TRACE
	SipTraceLevel	*dummy_lev;
	dummy_lev = lev;
	*err = E_TRACE_DISABLED;
	return SipFail;
#else
	*lev = curSipLevel;
	*err = E_NO_ERROR;
	return SipSuccess;
#endif
}

/*****************************************************************
** FUNCTION: sip_getTraceType
** Gets default Trace Type
**
** DESCRIPTION:
*****************************************************************/
SipBool sip_getTraceType
#ifdef ANSI_PROTO
	(SipTraceType  *dType,
	 SipError	*err)
#else
	(dType, err)
	SipTraceType	*dType;
	SipError	*err;
#endif
{
#ifndef SIP_TRACE
	SipTraceType	*dummy_dType;
	dummy_dType = dType;	
	*err = E_TRACE_DISABLED;
	return SipFail;
#else

	*dType = curSipType;
	*err = E_NO_ERROR;
	return SipSuccess;
#endif
}

/*****************************************************************
** FUNCTION: sip_setErrorLevel
** Sets default Error Level
**
** DESCRIPTION:
*****************************************************************/

SipBool sip_setErrorLevel
#ifdef ANSI_PROTO
	(SipErrorLevel  lev,
	 SipError	*err)
#else
	(lev, err)
	SipErrorLevel	lev;
	SipError     	*err;
#endif
{
#ifndef SIP_ERROR
	SipErrorLevel	dummy_lev;
	dummy_lev = lev;
	*err = E_ERROR_DISABLED;
	return SipFail;
#else

	if (lev != SIP_None )
	{
			if ( (!(lev & SIP_Major )) &&
				 (!(lev & SIP_Minor )) &&
				 (!(lev & SIP_Critical)) )
			{
				*err = E_INV_ERRORLEVEL;
				return SipFail;
			}
	}
	
	curSipErrLevel = lev;
	*err = E_NO_ERROR;
	return SipSuccess;
#endif
}


/*****************************************************************
** FUNCTION: sip_getErrorLevel
** Gets default Error Level
**
** DESCRIPTION:
*****************************************************************/

SipBool sip_getErrorLevel
#ifdef ANSI_PROTO
	(SipErrorLevel*  lev,
	 SipError	*err)
#else
	(lev, err)
	SipErrorLevel	*lev;
	SipError     	*err;
#endif
{
#ifndef SIP_ERROR
	SipErrorLevel	*dummy_lev;
	dummy_lev = lev;
	*err = E_ERROR_DISABLED;
	return SipFail;
#else
	*lev = curSipErrLevel;
	*err = E_NO_ERROR;
	return SipSuccess;
#endif
}



/*****************************************************************
** FUNCTION: sip_setTraceLevel
** Sets default Trace Level
**
** DESCRIPTION:
*****************************************************************/

SipBool sip_setTraceLevel
#ifdef ANSI_PROTO
	(SipTraceLevel  lev,
	 SipError	*err)
#else
	(lev, err)
	SipTraceLevel	lev;
	SipError	*err;
#endif
{
#ifndef SIP_TRACE
	SipTraceLevel	dummy_lev;
	dummy_lev = lev;
	*err = E_TRACE_DISABLED;
	return SipFail;
#else

	SIPDEBUGFN("Entering function: sip_setTraceLevel ");
	if (lev > SIP_TraceLevel) 
	{
		*err = E_INV_TRACELEVEL;
		SIPDEBUGFN("Exiting function: sip_setTraceLevel ");
		return SipFail;
	}
	curSipLevel = lev;
	SIPDEBUGFN("Exiting function: sip_setTraceLevel ");
	return SipSuccess;
#endif
}

/*****************************************************************
** FUNCTION: sip_setTraceType
** Sets default Trace Type
**
** DESCRIPTION:
*****************************************************************/

SipBool sip_setTraceType
#ifdef ANSI_PROTO
	(SipTraceType   dType,
	 SipError	*err)
#else
	(dType, err)
	SipTraceType	dType;
	SipError	*err;
#endif
{
#ifndef SIP_TRACE
	SipTraceType	dummy_dType;
	dummy_dType = dType;
	*err = E_TRACE_DISABLED;
	return SipFail;
#else

	SIPDEBUGFN("Entering function: sip_setTraceType ");
	if ( dType & ~(SIP_All))
	{
		*err = E_INV_TRACETYPE;
		SIPDEBUGFN("Exiting function: sip_setTraceType ");
		return SipFail;
	}
	curSipType = dType;
	SIPDEBUGFN("Exiting function: sip_setTraceType ");
	return SipSuccess;
#endif
}

/*****************************************************************
** FUNCTION: sip_trace
** Logs a Trace Event
**
** DESCRIPTION:
** Basically, flags are valid only if dLevel is SIP_Brief
** In case dLevel is SIP_None, then we dont log
** In case dLevel is SIP_All, we log all and dont check for flags
*****************************************************************/

SipBool sip_trace
#ifdef ANSI_PROTO
	(SipTraceLevel		lev,
	 SipTraceType		dType,
	 SIP_S8bit		*fmt
	 )
#else
	(lev, dType, fmt)
	SipTraceLevel		lev;
	SipTraceType		dType;
	SIP_S8bit		*fmt;
#endif
{
	char *pTimeString;
	SipError err;
	SIPDEBUGFN("Entering function: sip_trace ");
	/* Do not record this message if curr lev is less than pRequest lev */
	if (curSipLevel == SIP_UndefTrace)
		return SipFail;

	if (curSipLevel == SIP_None) 
	{
		SIPDEBUGFN("Exiting function: sip_trace ");
		return SipSuccess;   /* If None, Flags dont matter */
	}
	if (curSipLevel <  lev ) 
	{
		SIPDEBUGFN("Exiting function: sip_trace ");
		return SipSuccess;
	}

	/* For Detailed, all messages, irrespective of Flags are set */
   
	if (curSipLevel != SIP_Detailed )
		if ( !(curSipType & dType)) 
		{
	  		SIPDEBUGFN("Exiting function: sip_trace ");
			return SipSuccess;
		}

	/* NEXTONE - added NexTone stuff for logging */
	if (SipDebugLoc() == 0)
	goto _nextone_syslog;
	else if (SipDebugLoc() == 1)
	goto _hss_log;
	else return SipSuccess;

_nextone_syslog:
	NetLogDebugSprintf("----------------------------------------------------"\
							"---------------------------\n");
	if(dType & SIP_Init)
		NetLogDebugSprintf("SIP STACK SIP_TRACE Type: INIT     ");
	else if(dType & SIP_Incoming)
		NetLogDebugSprintf("SIP STACK SIP_TRACE Type: INCOMING ");
	else if(dType & SIP_Outgoing)
		NetLogDebugSprintf("SIP STACK SIP_TRACE Type: OUTGOING ");
	else if(dType & SIP_SysError)
		NetLogDebugSprintf("SIP STACK SIP_TRACE Type: SIP_ERROR    ");
	pTimeString = (char *)fast_memget(0,30,&err);
	sip_getTimeString((char **)&pTimeString);

	NetLogDebugSprintf("Generated at: %s",pTimeString);
	NetLogDebugSprintf("%s\n",fmt);
	NetLogDebugSprintf("----------------------------------------------------"\
	"---------------------------\n");
	fast_memfree (0, pTimeString, NULL);
	return SipSuccess;

_hss_log:
	/* HSS standard logging code */
	/* NEXTONE - end of logging change*/

	PrintFunction("----------------------------------------------------"\
		"---------------------------\n");
	if(dType & SIP_Init)
		PrintFunction("SIP STACK SIP_TRACE Type: INIT     ");
	else if(dType & SIP_Incoming)
		PrintFunction("SIP STACK SIP_TRACE Type: INCOMING ");
	else if(dType & SIP_Outgoing)
		PrintFunction("SIP STACK SIP_TRACE Type: OUTGOING ");
	else if(dType & SIP_SysError)
		PrintFunction("SIP STACK SIP_TRACE Type: SIP_ERROR    ");
	pTimeString = (char *)fast_memget(NON_SPECIFIC_MEM_ID,SIP_MAX_CTIME_R_BUF_SIZE,&err);
	sip_getTimeString((char **)&pTimeString);

	PrintFunction("Generated at: %s",pTimeString);
	PrintFunction("%s\n",fmt);
	PrintFunction("----------------------------------------------------"\
		"---------------------------\n");

	/* NEXTONE - added fflush */
	fflush(stdout);

	fast_memfree (NON_SPECIFIC_MEM_ID, pTimeString, NULL);
	return SipSuccess;
}



