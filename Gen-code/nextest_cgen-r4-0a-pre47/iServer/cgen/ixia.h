/////////////////////////////////////////////////////////////////////
//
//	Name:
//		ixia.h
//
//	Description:
//		This contains includes and structures used by gen
//		when interfaceing to the IXIA load generator.
//
/////////////////////////////////////////////////////////////////////

// #define IXIA_WRITE_TO_FILE
// #define IXIA_USE_EXPECT_LIB
#define IXIA_SEPARATE_PROC

#define IXIA_TRUE  1
#define IXIA_FALSE 0

#define IXIA_BATCH_LEN            10
#define IXIA_TCLSH                "/usr/local/bin/tcl"
#define IXIA_DEF_TESTSCRIPT       "./ixiaScripts/hkDefaultTest.tcl"
#define IXIA_TESTSCRIPT_NAME_LEN  256
#define IXIA_COMMAND_LEN          (IXIA_TESTSCRIPT_NAME_LEN + 32)
#define IXIA_PIPEREAD_SIZE        (64 * 1024) 

// #define IXIA_DEBUG(_msg...)  printf(_msg)
#define IXIA_DEBUG(_msg...)

#if defined( IXIA_USE_EXPECT_LIB )
	#define IXIA_OPEN( _f_ )            \
		do {                            \
			ixiaGlobal.ixia_fp = exp_popen( (_f_) );        \
			if( !ixiaGlobal.ixia_fp )                         \
			{                                     \
				fprintf (stderr, "exp_popen failed for %s\n", IXIA_TCLSH); \
				exit (-1);                        \
			}                                     \
		} while( 0 )

	#define IXIA_SEND_INSTR( c... )  \
		fprintf( ixiaGlobal.ixia_fp, c );  \
		fflush( ixiaGlobal.ixia_fp )

	#define IXIA_WAIT_DONE()       \
		exp_fexpectl( ixiaGlobal.ixia_fp, exp_glob, "Done.", 1, exp_end )

	#define IXIA_CLOSE

#elif defined( IXIA_WRITE_TO_FILE )

	#define IXIA_OPEN(_f_ )  ixiaGlobal.ixia_fp = fopen( "./tclCommands", "w" )

	#define IXIA_SEND_INSTR( c... )            \
	     fprintf( (ixiaGlobal.ixia_fp), c );   \
	     fflush( (ixiaGlobal.ixia_fp) )

	#define IXIA_WAIT_DONE()

	#define IXIA_CLOSE fclose( ixiaGlobal.ixia_fp )

#elif defined( IXIA_SEPARATE_PROC )

	#define IXIA_OPEN(_f_)                                   \
		do {                                                           \
			IXIA_DEBUG( "Opening ixia pipe\n" );     \
			if( pipe( ixiaGlobal.ixiaPipe ) == -1 )                      \
			{                                                            \
				fprintf( stderr, "Cannot open pipe to ixia process\n" );   \
				exit( -1 );                                                \
			}                                                            \
			IXIA_DEBUG( "Forking\n" );                                   \
			if( !(ixiaGlobal.pid = fork()) )                             \
			{                                                            \
				/* Ixia Process (child) */                                 \
				IXIA_DEBUG( "In child\n" );                           \
				close( ixiaGlobal.ixiaPipe[1] ); /* Close unused end (stdout) */ \
				ixiaMain();                                                \
				exit( 0 );                                                 \
			}                                                            \
			if( ixiaGlobal.pid < 0 )                                     \
			{                                                            \
				IXIA_DEBUG( "Fork failed!! Error = %d\n", ixiaGlobal.pid ); \
				exit( 0 );                                           \
			}                                                            \
			IXIA_DEBUG( "In parent (Child pid = %d)\n", ixiaGlobal.pid ); \
			close( ixiaGlobal.ixiaPipe[0] ); /* Close unused end (stdin) */ \
		} while( 0 )

	#define IXIA_SEND_INSTR( c... )                         \
		do {                                                \
			char cmdStr[IXIA_COMMAND_LEN];                  \
			sprintf( cmdStr, c );                           \
			if( write( ixiaGlobal.ixiaPipe[1], cmdStr, strlen( cmdStr ) + 1 ) < 0 )    \
			{                                               \
				fprintf( stderr, "Write to Ixia Process failed: errno = %d\n", errno ); \
			}                                               \
		} while( 0 )

	#define IXIA_WAIT_DONE()     while(0)

	#define IXIA_CLOSE()   ixiaShutdown()

#else
	#error "One of IXIA_USE_EXPECT_LIB, IXIA_WRITE_TO_FILE or IXIA_SEPARATE_PROC"
	#error "must be #defined."
#endif


//
// Structure to hold Ixia information
//
typedef struct {

	FILE *ixia_fp;
	int  pid;
	int  ixiaPipe[2];

}ixiaInfo;

extern ixiaInfo ixiaGlobal;

//
// Structure for reading pipe
//
typedef struct {

	unsigned short int indx;         /* Current index into buffer */
	unsigned short int length;       /* Length of read buffer */
	/* Buffer to receive data */
	char buffer[IXIA_PIPEREAD_SIZE]; 
	/* Buffer to store commands spanning more than one buffer */
	char partialBuf[IXIA_COMMAND_LEN]; 

}ixiaReadBuffer;

//
// Structure for accumulating data before sending to ixia Process
//
typedef struct {

	unsigned long int numCalls;

	unsigned long int srcIp;
	unsigned long int srcIpDelta;
	unsigned long int dstIp;
	unsigned long int dstIpDelta;
	unsigned short int srcPort;
	unsigned short int srcPortDelta;
	unsigned short int dstPort;
	unsigned short int dstPortDelta;

} ixiaCallAccum;

void ixiaInitCallAccum( void );
void ixiaSendAccumCalls( void );
void ixiaAccumCall( unsigned long int srcIp, unsigned long int dstIp,
                    unsigned short int srcPort, unsigned short int dstPort );




	
