
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <unistd.h>
#include <signal.h>
#include <string.h>
#include <ctype.h> /* For isprint */
#include <sys/wait.h>

#include "expect.h"

#include "ixia.h"

/* Global pipe between cgen and Ixia process */
int ixiaPipe[2];

/* Global to this file */
ixiaCallAccum callAccum;

/* Buffer to read Ixia script commands */
ixiaReadBuffer pipeBuffer;

/*
** Local Prototypes
*/
void ixiaTerminate( int sig );


//
// Routine: isIxiaCmd
//
// Purpose: Verifies if a command is an Ixia Script command
//
// Input Parameters: str - String to be verified
//
// Outputs Parameters: NONE
//
// Return Values: 1 - It's good 0 - Bad
//
// Notes: 
//

int
isIxiaCmd( char *str )
{

	char *c;
	int len;

	if(    !strncmp( str, "hkTestCallUp", 12 ) 
	    || !strncmp( str, "source", 6 ) )
	{
		len = strlen( str );
		if( len <= IXIA_COMMAND_LEN )
		{
			for( c = &str[0]; *c != 0; c += 1 )
			{
				if( !isprint( *c ) )
				{
					if( !(*c == 0x0A) && !(*c == 0x0D) ) /* Allow CRLF */
					{
						goto isIxiaCmd_bad;
					}
				}
			}
		}
		else
		{
			goto isIxiaCmd_bad;
		}
	}
	else
	{
		goto isIxiaCmd_bad;
	}
	return( 1 );

isIxiaCmd_bad:
	return( 0 );
}

//
// Routine: ixiaInitReadBuffer
//
// Purpose: Initialized the buffer that holds the commands read off the pipe
//
// Input Parameters: NONE
//
// Output Parameters: NONE
//
// Return Values: NONE
//
// Notes:
//
#if 0
static void
ixiaInitReadBuffer( void )
{
	pipeBuffer.indx = 0;
}
#endif

//
// Routine: ixiaReadCmd
//
// Purpose: Reads a script command from the pipe
//
// Input Parameters: NONE
//
// Output Parameters: cmd - If a valid command is read, this value will be
//                          modified to contain a pointer to the command string.
//
// Return Values: 1 -> A valid command has been recvd. 0 -> Pipe closed
//
// Notes: Blocking
//
int
ixiaReadCmd( char **str )
{
	int partialCmd;
	unsigned short int startIndx;
	char *c;

	startIndx = pipeBuffer.indx;
	partialCmd = IXIA_FALSE;

	IXIA_DEBUG( "Entering ixiaReadCmd()\n" );

	if( pipeBuffer.indx == 0 )
	{
		IXIA_DEBUG( "Reading gen pipe ..." );
		pipeBuffer.length = read( ixiaGlobal.ixiaPipe[0],
		                          pipeBuffer.buffer,
		                          IXIA_PIPEREAD_SIZE );
		IXIA_DEBUG( "Got data.\n    Len = %d - %s\n", pipeBuffer.length, pipeBuffer.buffer );

		if( pipeBuffer.length <= 0 )
		{
			return( 0 ); /* Bad - or Exit */
		}
	}


	while( pipeBuffer.buffer[pipeBuffer.indx] != 0 )
	{
		c = &pipeBuffer.buffer[pipeBuffer.indx];
		if( isprint( *c ) )
		{
			pipeBuffer.indx++;
		}
		else
		{
			if( !(*c == 0x0A) && !(*c == 0x0D) ) /* Allow CRLF */
			{
				startIndx = pipeBuffer.indx++; /* Data is garbage */
			}
			else
			{
				pipeBuffer.indx++;
			}
		}

		if( pipeBuffer.indx >= pipeBuffer.length )
		{
			/* Need to save partial command in a buffer 
			** and do a new read 
			*/
			partialCmd = IXIA_TRUE;

			memcpy( pipeBuffer.partialBuf,
			        &pipeBuffer.buffer[startIndx],
			        pipeBuffer.length - startIndx );

			/* NULL Terminate string in partial buffer */
			pipeBuffer.partialBuf[IXIA_PIPEREAD_SIZE - startIndx] = 0;			

			pipeBuffer.length = read( ixiaGlobal.ixiaPipe[0],
		                              pipeBuffer.buffer,
		                              IXIA_PIPEREAD_SIZE );

			if( pipeBuffer.length <= 0 )
			{
				return( 0 ); /* Bad - or Exit */
			}

			pipeBuffer.indx = 0;
		}
	}

	/*
	** We should have a command by now
	*/
	if( !partialCmd )
	{
		if( (pipeBuffer.indx + 1) < pipeBuffer.length )
		{
			pipeBuffer.indx++; /* Point to non-zero element */
		}
		else
		{
			pipeBuffer.indx = 0;
		}
		*str = &pipeBuffer.buffer[startIndx];
		return( 1 );
	}
	else
	{
		/* Assemble partial command */
		memcpy( &pipeBuffer.partialBuf[strlen( pipeBuffer.partialBuf )],
		        &pipeBuffer.buffer[0], strlen( &pipeBuffer.buffer[0] ) );

		*str = &pipeBuffer.partialBuf[0];
		return( 1 );
	}
}


//
// Routine: ixiaMain
//
// Purpose: Main Ixia routine, called after fork(). This process shuttles 
//          ixia Script commands from gen to TCL
//
// Inputs: NONE
//
// Outputs: NONE
//
// Notes:
//
void
ixiaMain( void )
{
	char *thisCmd;

	IXIA_DEBUG( "Entering ixiaMain()\n" );

	signal( SIGINT, ixiaTerminate );

	ixiaGlobal.ixia_fp = (FILE *)exp_popen( IXIA_TCLSH );
	IXIA_DEBUG( "Done calling exp_popen() Result = %p\n", ixiaGlobal.ixia_fp );

	if (!ixiaGlobal.ixia_fp)
	{
		fprintf (stderr, "exp_popen failed for %s\n", IXIA_TCLSH);
		exit (-1);
	}

	while( 1 )
	{

		if( (ixiaReadCmd( &thisCmd ) > 0) )
		{
			IXIA_DEBUG( "Got Command %s\n", thisCmd );
			fprintf( ixiaGlobal.ixia_fp, "%s", thisCmd );
			IXIA_DEBUG( "Sent command to TCL. Waiting for Done.\n" );
			exp_fexpectl( ixiaGlobal.ixia_fp, exp_glob, "Done.", 1, exp_end );
			IXIA_DEBUG( "Got Done\n" );
		}
		else
		{
			break; /* Get out of loop */
		}
	}
}

//
// Routine: ixiaTerminate
//
// Purpose: Closes all connections and does cleanup before closing
//
// Input Parameters: NONE
//
// Output Parameters: NONE
//
// Return Values: NONE
//
// Notes:
//
void
ixiaTerminate( int sig )
{

	fprintf( ixiaGlobal.ixia_fp, "hkCloseConnection\n" );
	exp_fexpectl( ixiaGlobal.ixia_fp, exp_glob, "Done.", 1, exp_end );
	fprintf( ixiaGlobal.ixia_fp, "exit\n" );

	exit(0);

}

//
//  The following routines run in the parent gen process and deal with accumulating
//  calls and sending one Ixia script command after a number of call have built up
//
	

//
// Routine: ixiaInitCallAccum
//
// Purpose: Initialize the call accumulator
//
// Input Parameters: NONE
//
// Output Parameters: NONE
//
// Return Values: NONE
//
// Notes:
//
void
ixiaInitCallAccum( void )
{
	memset( &callAccum, 0x00, sizeof( ixiaCallAccum ) );
}
	

//
// Routine: ixiaSendAccumCalls
//
// Purpose: Sends the accmulated calls
//
// Input Parameters: NONE
//
// Output Parameters: NONE
//
// Return Values: NONE
//
// Notes:
//
void
ixiaSendAccumCalls( void )
{

	IXIA_SEND_INSTR( "hkSetupCallBatch %lu 0x%0lx %ld 0x%0x %d 0x%0lx %ld 0x%0x %d\n",
	                 callAccum.numCalls,
	                 callAccum.srcIp, *(signed long int *)&callAccum.srcIpDelta,
	                 callAccum.srcPort, *(signed short *)&callAccum.srcPortDelta,
	                 callAccum.dstIp, *(signed long int *)&callAccum.dstIpDelta,
	                 callAccum.dstPort, *(signed short int *)&callAccum.dstPortDelta );

	callAccum.numCalls = 0;
}



//
// Routine: ixiaAccumCall
//
// Purpose: This routine accumulates calls and determines a pattern between
//          successive calls. It keeps track of the number of calls that meet
//          the pattern. If the pattern is ever broken by a new call, the 
//          existing calls are send ans a new pattern is determined.
//
// Input Parameters:
//    srcIp - The source IP address of this media connection
//    dstIp - The destination IP of the new media call
//    srcPort - The source port of the media connection
//    dstPort - The destination port of the media connection
//
// Output Parameters:
//
// Return Values:
//
// Notes:
//
void
ixiaAccumCall( unsigned long int srcIp, unsigned long int dstIp,
               unsigned short int srcPort, unsigned short int dstPort )
{

	switch( callAccum.numCalls )
	{
		case 0:
			/* First media call with this pattern - just store Address */
			callAccum.srcIp = srcIp;
			callAccum.dstIp = dstIp;
			callAccum.srcPort = srcPort;
			callAccum.dstPort = dstPort;

		break;

		case 1:

			/* We can now find the pattern */
			callAccum.srcIpDelta = srcIp - callAccum.srcIp;
			callAccum.dstIpDelta = dstIp - callAccum.dstIp;
			callAccum.srcPortDelta = srcPort - callAccum.srcPort;
			callAccum.dstPortDelta = dstPort - callAccum.dstPort;

		break;

		default:
			/* See if this media call breaks the pattern */
			if(!((srcIp == ((callAccum.numCalls * callAccum.srcIpDelta)
			                                           + callAccum.srcIp))
			&& (dstIp == ((callAccum.numCalls * callAccum.dstIpDelta)
			                                           + callAccum.dstIp))
			&& (srcPort == ((callAccum.numCalls * callAccum.srcPortDelta)
			                                           + callAccum.srcPort))
			&& (dstPort == ((callAccum.numCalls * callAccum.dstPortDelta)
			                                           + callAccum.dstPort))) )
			{
				/* Pattern does not match - send and start a new pattern */
				ixiaSendAccumCalls();

				callAccum.srcIp = srcIp;
				callAccum.dstIp = dstIp;
				callAccum.srcPort = srcPort;
				callAccum.dstPort = dstPort;

				callAccum.numCalls = 0;
			}
		break;
	}

	callAccum.numCalls++;

}


//
// Routine: ixiaShutdown
//
// Purpose: Sends a shutdown message to the gen handler process
//
// Input Parameters:
//
// Output Parameters:
//
// Return Values:
//
// Notes: This routine is called from the Parent process (cgen/sgen)
//
void
ixiaShutdown( void )
{
	int trash;

	close( ixiaGlobal.ixiaPipe[0] );
	close( ixiaGlobal.ixiaPipe[1] );
	kill( ixiaGlobal.pid, SIGINT );
	waitpid( ixiaGlobal.pid, &trash, 0 );
}

//
// Routine used for testing
//
#if 0
int
main()
{
	if( pipe( ixiaPipe ) == -1 )
	{
		printf( "Error- cannot open pipe to Ixia\n" );
		exit( 1 );
	}

	if( !fork() )
	{
		/* Child */
		ixiaMain();
		exit( 0 );
	}
	else
	{
		/* Parent - Try to write to the Pipe */
		write( ixiaPipe[1], "./ixiaScripts/myTxTest.tcl", 28 );
		wait( NULL );
	}
}
#endif

//
// Routine:
//
// Purpose:
//
// Input Parameters:
//
// Output Parameters:
//
// Return Values:
//
// Notes:
//
