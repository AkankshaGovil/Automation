/*******************************************************************
 ** FUNCTION:
 **	 	This file has common definitions used by all modules
 *******************************************************************
 **
 ** FILENAME:
 ** sipcommon.h
 **
 ** DESCRIPTION:
 **	All common wrappers are defined here
 **	It is expected that all programs use datatypes defined here
 **
 ** DATE			NAME			REFERENCE		REASON
 ** ----			----			--------		------
 ** 18-11-99		Arjun Roychowdhury				    Creation
 **
 **	Copyright 1999, Hughes Software Systems, Ltd. 
 *******************************************************************/

#ifndef __SIPCOMMON_H__
#define __SIPCOMMON_H__

#ifndef SIP_NO_FILESYS_DEP
#include <stdio.h>
#endif

/* Ensure Names are not mangled by C++ compilers */
#ifdef __cplusplus
extern "C" {
#endif 


/* This part ID is automatically inserted by the packaging script */
#define SIP_PART_ID "1-000-5-0208-0501-14-0502-000"
/* This is the stack release version id and would be modified 
   by the packaging script */
#define SIP_STACK_VERSION_ID "SIP_SOLARIS_C_5_2"

#ifdef ANSI_PROTO
#define _ARGS_(x)      x
#define _CONST_        const
#else
#define _ARGS_(x)      ()
#define _CONST_
#endif

/* This is the size of the buffer allocated by the stack when converting
   a SIP message structure to text. This will be the limit on the size
   of the SIP messages that can be sent using the stack */
#ifndef SIP_MAX_MSG_SIZE
#define  SIP_MAX_MSG_SIZE (3000)
#endif

/* This is the size of the buffer allocated by the stack when converting
   a SIP message structure to text. This will be the limit on the size
   of the SIP messages that can be sent using the stack */
#ifndef SIP_MAXU16BIT
#define  SIP_MAXU16BIT (65535)
#endif
   
/* This is the limit on the size of the buffer allocated by functions that
   return SIP headers or SDP lines as strings. This is not a limit on the
   size of the headers that can be formed by the stack. */
#ifndef SIP_MAX_HDR_SIZE
#define SIP_MAX_HDR_SIZE (1000)
#endif

#define SIP_NULL NULL

/*
 ** Basic Data Types 
 */

typedef 	unsigned char 		SIP_U8bit;
typedef 	unsigned short		SIP_U16bit;
typedef		unsigned int		SIP_U32bit;

typedef 	char			SIP_S8bit;
typedef		short				SIP_S16bit;
typedef		int					SIP_S32bit;

typedef		void *				SIP_Pvoid;


#define SipErrorLevel	SIP_S32bit
#define SIP_UndefError	(-1)

/*
 ** Enumerations
 */

typedef enum
{
	SipSuccess		=  1,
	SipFail			=  0
} SipBool;



typedef enum 
{
#define 	SipErrorBase		2000
	E_INV_HEADER     	       	=  (SipErrorBase +0),
	E_INV_INDEX 		        =  (SipErrorBase +1),          
	E_NO_MEM        			=  (SipErrorBase +2),  
	E_INV_TYPE    				=  (SipErrorBase +3),      
	E_INV_TRACELEVEL       		=  (SipErrorBase +4),
	E_INV_TRACETYPE           	=  (SipErrorBase +5),
	E_INV_ERRORLEVEL           	=  (SipErrorBase +6),
	E_INV_STATSTYPE          	=  (SipErrorBase +7),
	E_TRACE_DISABLED           	=  (SipErrorBase +8), 
	E_ERROR_DISABLED          	=  (SipErrorBase +9),
	E_STATS_DISABLED          	=  (SipErrorBase +10),
	E_DUP_ENTRY          		=  (SipErrorBase +11),
	E_NO_EXIST          		=  (SipErrorBase +12),
	E_SYSTEM_ERROR          	=  (SipErrorBase +13),
	E_MEM_ERROR          		=  (SipErrorBase +14),
	E_INV_PARAM          		=  (SipErrorBase +15),
	E_NO_ERROR          		=  (SipErrorBase +16),
	E_PARSER_ERROR				=  (SipErrorBase +17),
	/* Timer Errors */
	E_TIMER_DUPLICATE			=  (SipErrorBase+18),
	E_TIMER_FULL				=  (SipErrorBase+19),
	E_TIMER_NO_EXIST			=  (SipErrorBase+20),

	/* Added errors for possible incomplete messages*/
	E_MAYBE_INCOMPLETE			=  (SipErrorBase+21),
	E_INCOMPLETE				=  (SipErrorBase+22),
	/* Added not-implemented error */
	E_NOT_IMPLEMENTED			= (SipErrorBase+23),

	/*Added errors for message based support */
	E_SEND_FAILED				= (SipErrorBase+24),
	E_TIME_OUT					= (SipErrorBase+25),
	E_HASH_FAIL					= (SipErrorBase+26),
	E_RECV_FAIL					= (SipErrorBase+27),

	/*Error defined for data overflow. */
	E_INV_RANGE					= (SipErrorBase+28),
	
	/* Transaction does not exist Applicable for TxnLayer */
	E_TXN_NO_EXIST					= (SipErrorBase+29),
	E_TXN_EXISTS					= (SipErrorBase+30),
	E_TXN_INV_STATE					= (SipErrorBase+31),
	E_TXN_INV_MSG					= (SipErrorBase+32),
	E_BUF_OVERFLOW					= (SipErrorBase+33),
	E_TXN_NETWORK_SEND_ERR			= (SipErrorBase+34), /* if SEND_TO_NETWORK
                                                            call fails when
                                                            TXN layer is enabled
                                                            then this error code
                                                            should be returned.
                                                          */
											
	E_SECND_INSERTION_SINGLE_INST_HDR					= (SipErrorBase+35),
	E_FORMING_QUOTING_STRING					= (SipErrorBase+36),

	/*
	 * This last error code only serves as an indicator for the last
	 * error code. No other purpose is served.
	 */
	E_LAST_ERROR					= (SipErrorBase+37)
} SipError;

typedef enum 
{
	SIP_UndefTrace	    = -1, 
	SIP_None    		= 0,
	SIP_Brief    		= 1,
	SIP_Detailed       	= 2,
	SIP_TraceLevel     	= 2
} SipTraceLevel;

typedef enum 
{
	SIP_Init              = 0x01,
	SIP_Incoming          = 0x02,
	SIP_Outgoing          = 0x04,
	SIP_SysError          = 0x08,
	SIP_All	              = 0x01 | 0x02 | 0x04 | 0x08			
} SipTraceType;

#define SIP_Major		(0x10)
#define SIP_Minor		(0x20)
#define SIP_Critical	(0x40)

#define TIMER_CALLLEG		0
#define TIMER_TRANSACTION	1
#define TIMER_CALLLEG_GE300	2 /* to stop ret for resps with code >=300 */
#define TIMER_RPR			3
#define TIMER_NONE			4 /* Initial value */

/* 
 * SIP_BASE_10, SIP_BASE_16 used as base values
 * in strtoul, etc functions
 */ 
#define SIP_BASE_10		10 
#define	SIP_BASE_16		16

#define SIP_MAX_PORT_SIZE	6  /* Port size - 6 characters (65535) */
#define SIP_NO_DAYS		7  /* 7 days in a week */
#define SIP_MAX_DAY_LEN		4  /* max len of day string is 4 ("MON",etc)*/
#define SIP_NO_MONTHS		12 /* 12 months in a year */ 
#define SIP_MAX_MONTH_LEN	4  /* max len of month string is 4 ("JUN",etc)*/
#define SIP_RESP_CODE_LEN	4  /* Response code length is 4 ("1XX","2XX") */
#define SIP_MAX_MONTH_DAYS	31 /* maximum days in a month */
#define SIP_CLEN_SIZE		30 /* Content Length header size 30 bytes */
#define SIP_DEFAULT_PORT	5060 /* default port of SIP - 5060 */

#define SIP_TRACE_BUF_SIZE	200
#define SIP_FIRST_20_BYTES 	21  /* first 20 bytes */

#define SIP_IPADDR_SIZE		16  /* max Ip address size 16 */

#define SIP_DEFAULT_T1		500 /* 500 milliseconds retrans timer T1 */
#define SIP_DEFAULT_T2		4000 /* 4000 milliseconds retrans timer T2 */

#define SIP_DEFAULT_MAX_RETRANS 10 
#define SIP_DEFAULT_INV_RETRANS 6

#define SIP_ASCII_CHAR_SET	256 /* Ascii characters set size */

#define SIP_ADDR_TYPE_IPV6	6 /* IPv6 address type */
#define SIP_ADDR_TYPE_IPV4	4	/* IPV4 address type */
#define SIP_ADDR_TYPE_IP_DEF 0 /* Default IP address type */

#define SIP_59_SECS			59 /* 59 seconds */
#define SIP_59_MIN			59 /* 59 minutes */
#define SIP_23_HRS			23 /* 23 Hours */


/* Ensure Names are not mangled by C++ compilers */
#ifdef __cplusplus
}
#endif 

#endif
