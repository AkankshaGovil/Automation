#include <stdio.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <errno.h>
#include <string.h>
#include <poll.h>
#include <sys/resource.h>
#include <pthread.h>
#include "mgen.h"
#include <malloc.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <unistd.h>
#include "nxosd.h"

// poll array for the lines is maintained outside
struct pollfd *pollA;
struct pollfd *pollS;

enum Bool {
    False = 0,
    True
};

// Structs
typedef struct
{
	int badSendingPort;
	int badSequenceNo;
} LineError;

typedef struct
{
	struct sockaddr_in 	addr;
	struct sockaddr_in 	saddr;
	LineError 			errors;
	// last sequence number is stored here
	RtpHeader 			sndrtpHdr;
	RtpHeader 			rcvrtpHdr;
} Line;

typedef struct 
{
	longlong_t numbytes;
	longlong_t numpkts;
	longlong_t onumbytes;
	longlong_t onumpkts;
	unsigned long skip_sleep;
	unsigned long oskip_sleep;
} Stats;

typedef struct
{
	int start;
	int n;
	Stats *stat_p;
} TxRxThreadArg;

// Variables

// Flags
int 			asyncPorts = 1; // use asymetric ports (send/recv on diff ports)
int 			debug = 0; 
int 			controllerFd = -1;
int 			loopback = 0;
int  			realtime = 1;
int  			automode = 0;	// Automated testing mode disabled by default
int  			execmode = 0;	// Is enabled if mgen is started by sgen or gen
int             sendDtmf = 0;   // Flag for sending DTMF tone
int             dtmfLock = 0;

// IP addresses and ports
unsigned long 	controllerIp = -1;
unsigned short	chanLocalPort = 49200;
unsigned short 	startLocalPort = 49200;
unsigned long 	chanLocalIp = 0;
unsigned short 	mgenPort = 49160;

// Statistics and counters
Stats 			*SndStatsp, *RcvStatsp;
Stats 			gs, gr;	// Global send stats and global receive stats
int 			totalCalls = 0;
MgenStats		mgenStats;	// Stats sent to controller

// Misc
int 			nCalls = 1;		// Default number of calls
int 			nthreads = 1;
int  			verbosity = V_CALL;				
longlong_t 		payloadInterval = 30000000; // nanoseconds
int 			payloadLen = 30;	// not including RTP header
int 			LeftOver = 0;
int *			linesRx;
Line *			lines;
hrtime_t 		curtim, oldtim;
DtmfInfo        dtmfInfo;
char            dtmfEnd = False;

// Function declarations
int				usage(void);
char *			ULIPtostring(unsigned long ipaddress);
int				ParseArguments(int argc, char **argv);
int				PrintStartState(void);
int				PrintTxRxStats(void);
int				PrintStats(void);
int				ReadInput(void);
int 			NetSetMaxFds(void);
int 			InitPollFds(void);
int 			InitStats(void);
int 			InitRtpHeader(RtpHeader *rtpHeader, int line, int dtmfFlag);
void * 			Connect(void *args);
void * 			Send(void *args);
void * 			Receive(void *args);
int 			LaunchThread(void *(*fn)(void*), void *arg);
int 			LaunchTxRxThreads(void *(*fn)(void *), Stats *stp);
int             MakePayload(char *buf, int *len);
int             sgenInform(int command);

int
main(int argc, char **argv)
{
	ParseArguments(argc, argv);
#ifndef NETOID_LINUX
	if (realtime) 
	{
		ThreadInitRT();
	}
#endif //NETOID_LINUX
	InitStats();

	NetSetMaxFds();

	PrintStartState();

	InitPollFds();

	if (controllerIp != (unsigned long)-1)
	{
		LaunchThread(Connect, NULL);
	}
	else if (loopback)
	{
		LaunchThread(Receive, NULL);
	}

	while (ReadInput());
}

int
ParseArguments(int argc, char **argv)
{
//	struct sockaddr_in addr, from;
//	char buff[1024];
//	int i, one = 1, rc, fromlen = sizeof(struct sockaddr_in);
	char hname[256];
	int herror;

	while (--argc > 0)
    {
		if ((argv[argc][0] == '-') || (argv[argc][0] == '+'))
		{
	  		switch (argv[argc][1])
	  		{
	  		case 'h':
				usage();
	       		exit(0);

	  		case 'n':
				if (argv[argc + 1])
				{
	       			nCalls = atoi(argv[argc+1]);
				}
	       		break;

			case 'b':
				if (argv[argc + 1])
				{
					payloadLen = atoi( argv[argc+1] );
					if (argv[argc + 2])
					{
					   payloadInterval = atoll( argv[argc+2] );
					}
				}
	       		break;

		  	case 'c':
				if (argv[argc + 1])
				{
					controllerIp = ntohl( inet_addr( argv[argc+1] ) );
				}
		       	break;

			case 'L':
				if (argv[argc + 1])
				{
		   			chanLocalIp = ntohl(inet_addr(argv[argc + 1]));
				}
	       		break;

			case 'M':
				if (argv[argc + 1])
				{
					mgenPort = atoi(argv[argc+1]);
				}
				break;

		  	case 'm':
				if (argv[argc + 1])
				{
					chanLocalPort = atoi(argv[argc+1]);
					startLocalPort = chanLocalPort;
				}
	       		break;

	  		case 'v':
				if (argv[argc + 1])
				{
		   			verbosity = atoi(argv[argc+1]);
				}
		   		break;

	  		case 'x':
				if (argv[argc + 1])
				{
		   			nthreads = atoi(argv[argc+1]);
				}
		   		break;

	  		case 'o':
		   		loopback = 1;
		   		break;

	  		case 'R':
		   		realtime = 0;
	       		break;

		  	case 'r':
		   		debug = 1;
	       		break;

	  		case 's':
	           	asyncPorts = 0;
	       		break;

	  		case 'T':
	           	automode = 1;
	       		break;

			// This option is only to be used internally by 
			// gen or sgen if they start mgen. So it is not 
			// advertised in usage. When used, mgen does not
			// read stdin.
	  		case 'z':
	           	execmode = 1;
	       		break;

	  		default:
	       		break;
	  		}
		}
    }

	if (chanLocalIp == 0)
	{
		gethostname(hname, sizeof(hname));
		chanLocalIp = ResolveDNS(hname, &herror);
	}

	// Don't print per thread/call stats
	if (automode)
	{
		verbosity = 0;
	}
}

// Call this stats function after number of threads have been determined 
int
InitStats(void)
{
	/* If numcalls can not be equally divided among the threads
	 * we need an extra thread for leftover calls */
	SndStatsp = calloc(nthreads + 1, sizeof(Stats));
	RcvStatsp = calloc(nthreads + 1, sizeof(Stats));
}

int
NetSetMaxFds(void)
{
    struct rlimit rl_data;

    getrlimit(RLIMIT_NOFILE, &rl_data);

    rl_data.rlim_cur = rl_data.rlim_max - 1;

    setrlimit(RLIMIT_NOFILE, &rl_data);

    memset(&rl_data, (int32_t) 0, sizeof(struct rlimit));

    getrlimit(RLIMIT_NOFILE, &rl_data);

    fprintf(stdout, "Maximum file descriptors set to %d\n",
	   (uint32_t) rl_data.rlim_cur);
}

int 
PrintStartState(void)
{
	fprintf(stdout,
            "\nNumCalls            :    %d"
            "\nLocalIP             :    %s"
            "\nMediaStartPort      :    %d"
            "\nControllerIP        :    %s"
            "\nControllerPort      :    %d"
            "\nPayloadLen(bytes)   :    %d"
            "\nPayloadInterval(ns) :    %lld"
            "\nSendReceiveSamePort :    %s"
            "\nLoopback            :    %s"
            "\nAutoTestingMode     :    %s"
			"\n\n", 
			nCalls, ULIPtostring(chanLocalIp), startLocalPort,
			ULIPtostring(controllerIp), mgenPort, payloadLen, payloadInterval,
			asyncPorts ? "no" : "yes",
			loopback ? "yes" : "no",
			automode ? "yes" : "no");
}

int
InitPollFds(void)
{
	int i, one = 1, rc, fromlen = sizeof(struct sockaddr_in);
	struct sockaddr_in addr, from;

	pollA = (struct pollfd *)malloc(nCalls*sizeof(struct pollfd));

	if (asyncPorts) 
	{
	  pollS = (struct pollfd *)malloc(nCalls*sizeof(struct pollfd));
	} 
	else 
	{
	  pollS = pollA;
	}

	// bind ports for ncalls
	for (i = 0; i < nCalls; i++)
	{
		if ((pollA[i].fd = socket(AF_INET, SOCK_DGRAM, 0)) < 0)
		{
			fprintf(stderr, 
					"Recv socket error, call %d, errno %s\n",
					 i, strerror(errno));
		}
		if (asyncPorts) 
		{
		  	if ((pollS[i].fd = socket(AF_INET, SOCK_DGRAM, 0)) < 0)
		    {
				fprintf(stderr, 
						"Send socket error, call %d, errno %s\n",
						i, strerror(errno));
		    }
		} 

		setsockopt(pollA[i].fd, SOL_SOCKET, SO_REUSEADDR,
			(void *)&one, sizeof(one));

		memset(&addr, 0, sizeof(addr));
		addr.sin_family = AF_INET;
		addr.sin_addr.s_addr = htonl (chanLocalIp);
		addr.sin_port  = htons (chanLocalPort);

		if (bind(pollA[i].fd, (struct sockaddr *)&addr, sizeof(addr)) < 0)
		{
			fprintf(stderr, 
					"Bind error, call %d, port %d, errno %s\n",
					i, chanLocalPort, strerror(errno));
		}
		if(asyncPorts)
		{
			memset(&addr, 0, sizeof(addr));
			addr.sin_family = AF_INET;
			addr.sin_addr.s_addr = htonl (chanLocalIp);
			//addr.sin_port  = htons (chanLocalPort);

			if (bind(pollS[i].fd, (struct sockaddr *)&addr, sizeof(addr)) < 0)
			{
				fprintf(stderr, 
					"Bind error, call %d, port %d, errno %s\n",
					i, chanLocalPort, strerror(errno));
			}
		}

		chanLocalPort +=2;
	}
	
	if (i > 0)
	{
		chanLocalPort -=2;
	}

	lines = (Line *)malloc(nCalls*sizeof(Line));
	memset(lines, 0, nCalls*sizeof(Line));

	linesRx = (int *)malloc(nCalls*sizeof(int));

	return 0;
}

int
LaunchThread(void *(*fn)(void*), void *arg)
{
	ThreadLaunch(fn, arg, 1);
}

int
ReadInput(void)
{
	static char s[1024];
	int len;
	// If mgen has been started from gen or sgen, 
	// it should not read stdin
	if (execmode)
	{
		sleep(2);
		return 1;
	}

	if (fgets(s, sizeof(s), stdin) == NULL)
	{
		return -1;
	}
	len = strlen(s);	
	if (s[len-1]=='\n')
		s[len-1]='\0';
	PrintStats();

	return 1;
}

int
PrintStats(void)
{
	int n = 0, i;
	int badSendingPort = 0, badSequenceNo = 0;
	int ssrc;

	memset(linesRx, 0, nCalls*sizeof(int));

	// print stats
	for (i=0; i<nCalls; i++)
	{
		if (lines[i].addr.sin_addr.s_addr != 0)
		{
			n++;
		}
		if (verbosity >= V_CALL) 
		{
			fprintf(stdout, "Line %d, rx seq no. %d, to/from %s/%d\n", i, 
					ntohs(lines[i].rcvrtpHdr.seqno), 
					ULIPtostring(ntohl(lines[i].addr.sin_addr.s_addr)), 
					ntohs(lines[i].addr.sin_port));
		}
		badSendingPort += lines[i].errors.badSendingPort;
		badSequenceNo += lines[i].errors.badSequenceNo;
		
		// mark the received line
		ssrc = ntohl(lines[i].rcvrtpHdr.ssrc);
		if ((ssrc < 0) ||
			(ssrc > nCalls))
		{
			fprintf(stderr, "Bad ssrc %d on line %d\n", ssrc, i);
		}
		else if (ssrc > 0)
		{
			if (linesRx[--ssrc] == 1)
			{
				fprintf(stderr, 
					"Duplicate traffic for ssrc %d on line %d\n", ssrc, i);
			}

			linesRx[ssrc] = 1;
		}
	}

	if (execmode)
	{
		fflush(stderr);
	}

	PrintTxRxStats();	
}

int
PrintTxRxStats(void)
{
	int i;
	Stats *stp;
	float txBitRate;
	float txPktRate;
	float rxBitRate;
	float rxPktRate;
	float delta;

	curtim = nx_gethrtime();

	if (verbosity >= V_GLOBAL || automode) 
	{
		delta = (float) (curtim - oldtim);

		/* Tx Stats */
		for (i = 0; i <(nthreads + LeftOver); i++) 
		{
			stp = (SndStatsp + i);
			if (verbosity >= V_THREAD) 
			{
				fprintf(stdout, 
					"Thread %d: Bytes sent = %d, Pkts sent = %d\n", i+1, 
					stp->numbytes, stp->numpkts);
				fprintf(stdout, 
					"Thread %d: Sent Bit Rate = %f Kbits/s, Sent Pkt Rate = %f Kpkts/s\n", i+1,
					((float)(stp->numbytes - stp->onumbytes))/((float)(curtim - oldtim))*8*1000000, 
					((float)(stp->numpkts - stp->onumpkts))/((float)(curtim - oldtim))*1000000 );
			}
			gs.numbytes += stp->onumbytes = stp->numbytes;
			gs.numpkts += stp->onumpkts = stp->numpkts;
			/* Make this 0, otherwise even if media is not flowing, it will keep incrementing the stats */
			stp->numbytes = 0;
			stp->numpkts = 0;
		}
		
		fprintf (stdout, 
				"\nTxBytes            :    %lld"
				"\nTxPkts             :    %lld", 
				gs.numbytes, gs.numpkts);

		txBitRate = ((float)(gs.numbytes - gs.onumbytes))/delta*8*1000000;
		txPktRate = ((float)(gs.numpkts - gs.onumpkts))/delta*1000000;

		fprintf(stdout, 
				"\nTxBitRate(Kbits/s) :    %f"
				"\nTxPktRate(Kpkts/s) :    %f", 
				txBitRate, txPktRate);

		if (execmode)
		{
			if((gs.numbytes - gs.onumbytes) > 0)
			{
				mgenStats.txBytes = gs.numbytes;
				mgenStats.txPkts = gs.numpkts;
			}
			else
			{
				mgenStats.txBytes = 0;
				mgenStats.txPkts = 0;
			}
			mgenStats.txBitRate = txBitRate;
			mgenStats.txPktRate = txPktRate;
		}

		gs.onumbytes = gs.numbytes;
		gs.onumpkts = gs.numpkts;
	
		/* Rx Stats */
		for (i = 0; i <(nthreads + LeftOver); i++) 
		{
			stp = (RcvStatsp + i);
			if (verbosity >= V_THREAD) 
			{
				fprintf(stdout, "Thread %d: Bytes rcvd = %lu, Pkts rcvd = %lu\n", 
                		i+1, stp->numbytes, stp->numpkts);
				fprintf(stdout, 
					"Thread %d: Recv Bit Rate = %f Kbits/s, Recv Pkts Rate = %f Kpkts/s\n", i+1,
					((float)(stp->numbytes - stp->onumbytes))/((float)(curtim - oldtim))*8*1000000, 
					((float)(stp->numpkts - stp->onumpkts))/((float)(curtim - oldtim))*1000000 );
			}
			gr.numbytes += stp->onumbytes = stp->numbytes;
			gr.numpkts += stp->onumpkts = stp->numpkts;
			/* Make this 0, otherwise even if media is not flowing, it will keep incrementing the stats */
			stp->numbytes = 0;
			stp->numpkts = 0;
		}
	
		fprintf (stdout, 
			    "\nRxBytes            :    %lld"
				"\nRxPkts             :    %lld", 
				gr.numbytes, gr.numpkts);

		rxBitRate = ((float)(gr.numbytes - gr.onumbytes))/delta*8*1000000;
		rxPktRate = ((float)(gr.numpkts - gr.onumpkts))/delta*1000000;

		fprintf(stdout, 
				"\nRxBitRate(Kbits/s) :    %f"
				"\nRxPktRate(Kpkts/s) :    %f"
				"\n\n", rxBitRate, rxPktRate);

		if (execmode)
		{
			if((gr.numbytes - gr.onumbytes) > 0)
			{
				mgenStats.rxBytes = gr.numbytes;
				mgenStats.rxPkts = gr.numpkts;
			}
			/* Set the media status to 0, if no media is flowing */
			else
			{
				mgenStats.rxBytes = 0; 
				mgenStats.rxPkts = 0;
			}
			mgenStats.rxBitRate = rxBitRate;
			mgenStats.rxPktRate = rxPktRate;
		}

		gr.onumbytes = gr.numbytes;
		gr.onumpkts = gr.numpkts;
	}
	oldtim = curtim;

	if (execmode)
	{
		fflush(stdout);
	}
}

void *
Connect(void *arg)
{
	struct sockaddr_in addr = { 0 };
	int ncalls = 0, rc, command;
	MgenControlMsg mcm;
	int line, tmpi;
	short tmps;
    DtmfInfo info, rinfo;
#ifndef NETOID_LINUX

	if (realtime) 
	{
		ThreadSetRT();
	}
#endif // NETOID_LINUX
	controllerFd = socket(AF_INET, SOCK_STREAM, 0);

	addr.sin_addr.s_addr = htonl(controllerIp);
	addr.sin_port = htons(mgenPort);
	addr.sin_family = AF_INET;

	// connect to the controller
	if (connect(controllerFd, (struct sockaddr *)&addr, sizeof(addr)) < 0)
	{
		fprintf(stderr, "Error connecting to controller, errno %s\n",
				strerror(errno));
	}

	tmpi = htonl(chanLocalIp);
	write(controllerFd, &tmpi, 4);
	tmps = htons(startLocalPort);
	write(controllerFd, &tmps, 2);
	tmpi = htonl(nCalls);
	write(controllerFd, &tmpi, 4);

	rc = read(controllerFd, &ncalls, 4);
	ncalls = ntohl(ncalls);

	LaunchTxRxThreads(Receive, RcvStatsp);
	LaunchTxRxThreads(Send, SndStatsp);

	// listen on the fd for data
	while (rc > 0)
	{
		memset(&mcm, 0xff, sizeof(MgenControlMsg));
		rc = read(controllerFd, (char *)&mcm, sizeof(MgenControlMsg));

		// Command to send mgen stats to controller
		if (ntohl(mcm.command) == 2)
		{
			// Send stats back to controller
            sgenInform(2);
			PrintStats();
			write(controllerFd, (char *) &mgenStats, sizeof(mgenStats));
			continue;
		}
        else if (ntohl(mcm.command) == dtmf)
        {
            // Send DTMF digit in the RTP packet
            dtmfLock = 1;
            rc = read(controllerFd, (char *)&info, sizeof(DtmfInfo));
            dtmfInfo.digit = info.digit;
            dtmfInfo.duration = ntohl(info.duration);
            dtmfInfo.volume = ntohl(info.volume);
            rinfo.volume = dtmfInfo.volume;
            rinfo.duration = dtmfInfo.duration;
            rinfo.digit = dtmfInfo.digit;
			InitRtpHeader(&lines[line].sndrtpHdr, line+1, 1);
            sendDtmf = 1;
            dtmfLock = 0;
            continue;
        }

		line = (ntohs(mcm.rcvport) - startLocalPort)/2;

//		if ((line < 0) || (line >= nCalls))
		if (line < 0)
		{
			fprintf(stderr, "Bad line %d\n", line);
			continue;
		}

		if (ntohl(mcm.rcvip) != chanLocalIp)
		{
			fprintf(stderr, "Bad IP %x\n", mcm.rcvip);
			continue;
		}

		if (ntohl(mcm.command) == 1)
		{
			// add/update command
			memset(&lines[line], 0, sizeof(Line));
			lines[line].addr.sin_port = mcm.rcvport;
			lines[line].addr.sin_family = AF_INET;

			lines[line].saddr.sin_port = mcm.sndport;
			lines[line].saddr.sin_family = AF_INET;

			if (!automode)
			{
				fprintf(stdout, "Added line %d, send %s/%d, recv %s/%d\n", 
					line, ULIPtostring(ntohl(mcm.sndip)), ntohs(mcm.sndport), 
					ULIPtostring(ntohl(mcm.rcvip)), ntohs(mcm.rcvport));
			}

			// update the seqno
			InitRtpHeader(&lines[line].sndrtpHdr, line+1, 0);

			// For receiver, line number rxed may have
			// no connection to its own line
			InitRtpHeader(&lines[line].rcvrtpHdr, 0, 0);

			totalCalls++;
			// set s_addr last because that's what send uses to determine if to send
			lines[line].saddr.sin_addr.s_addr = mcm.sndip;
			lines[line].addr.sin_addr.s_addr = mcm.rcvip;
		}
		else
		{
			// delete command
			memset(&lines[line], 0, sizeof(Line));
			if (!automode)
			{
				fprintf(stdout, "Deleted line %d\n", line);
			}
		}
	}

	fprintf(stderr, "No connection to controller...exiting\n\n");

	PrintStats();

	exit (0);
}

void *
Receive(void *arg)
{
	struct sockaddr_in addr, from;
	unsigned char buff[1024];
	int i, one = 1, rc, fromlen = sizeof(struct sockaddr_in);
	struct msghdr msg;
	struct iovec bufs[2];
	unsigned short expectedSeqNo, rxedSeqNo;
	TxRxThreadArg *targ = (TxRxThreadArg *)arg;
	int start, end;
	Stats *recv_stp;
    unsigned char signalType;
    unsigned int volume;
    static unsigned int duration;
    DtmfInfo info;
#ifndef NETOID_LINUX

	if (realtime) 
	{
		ThreadSetRT();
	}
#endif //NETOID_LINUX
	curtim = oldtim = nx_gethrtime();

	start = targ->start;
	end = targ->n+start;
	recv_stp = targ->stat_p;
    duration = 0;
	/* Initializing flags in mgenStat before it starts receiving media */
	mgenStats.fromPortChanged = 0;			//flag used to indiacte the media is received from a different port now
	mgenStats.receivingFromPort = -1;		//indicates that mgen has not yet started receiving media

        /* Ticket-29838: Fix for the media verification failures in Nextest */
        fflush(stdout);

_poll:
	for (i=start; i<end; i++)
	{
		pollA[i].events = POLLIN;
		pollA[i].revents = 0;
	}

	// select on these ports and send back whatever we receive on them
	rc = poll(pollA+start, targ->n, -1);

	if (rc < 0)
	{
		fprintf(stderr, "Receive poll error, errno %s\n", strerror(errno));
		goto _poll;
	}

	// On all sockets which have read set, do the read/write
	for (i=start; i<end; i++)
	{
		if (pollA[i].revents & POLLIN)
		{
			expectedSeqNo = ntohs(lines[i].rcvrtpHdr.seqno) + 1;

			bufs[0].iov_base = (char *)&lines[i].rcvrtpHdr;
			bufs[0].iov_len = sizeof(RtpHeader);
			bufs[1].iov_base = buff;
			bufs[1].iov_len = 1024;

			msg.msg_name = (char *)&from;
			msg.msg_namelen = fromlen;
			msg.msg_iov = bufs;
			msg.msg_iovlen = 2;

			if ((rc = recvmsg(pollA[i].fd, &msg, 0)) > 0) {
				/* Subtract rtp header from the udp payload. Assumed rtp header */
				recv_stp->numbytes += (rc - sizeof(RtpHeader));
				recv_stp->numpkts += 1;
				/* get the IP information of the transmiting endpoint from the packet received */
				mgenStats.receivingFromIp = ntohl(from.sin_addr.s_addr);
				if((mgenStats.receivingFromPort != ntohs(from.sin_port)) && (mgenStats.receivingFromPort != (unsigned short)-1))
				{
					mgenStats.fromPortChanged = 1;
				}
				mgenStats.receivingFromPort = ntohs(from.sin_port);
			}
#if 0
			/* get the IP information of the transmiting endpoint from the packet received */
			mgenStats.receivingFromIp = ntohl(from.sin_addr.s_addr);
			mgenStats.receivingFromPort = ntohs(from.sin_port);
#endif

            if (lines[i].rcvrtpHdr.second == 0x65) // Payload type for DTMF
            {
                duration += (buff[2] * 256) + buff[3];
                if (buff[1] & 0x80)
                {
                    signalType = buff[0];
                    volume = buff[1] & 0x3f;
                    info.digit = signalType;
                    info.duration = htonl(duration);
                    info.volume = htonl(volume);
                    sgenInform(dtmf);
                    if (write(controllerFd, (char *)&info, sizeof(DtmfInfo)) < 0)
                    {
                        perror("SendMgenDtmf() - no connection with mgen, write error ");
                        return NULL;
                    }
                    duration = 0;
                }
            }

			//rc = recvfrom(pollA[i].fd, buff, 1024, 0,
			//		(struct sockaddr *)&from, &fromlen);

			//if (lines[i].addr.sin_addr.s_addr &&
			//	((from.sin_addr.s_addr != lines[i].addr.sin_addr.s_addr) ||
			//	(from.sin_port != lines[i].addr.sin_port)))
			//{
			//	lines[i].errors.badSendingPort++;
			//}

			rxedSeqNo = ntohs(lines[i].rcvrtpHdr.seqno);
			
			if (expectedSeqNo < rxedSeqNo)
			{
				lines[i].errors.badSequenceNo +=
					(rxedSeqNo-expectedSeqNo);

				// bug_7403

				if (debug)
					fprintf(stdout, "\n In mgen: bad seq num: %d for call %d.", lines[i].errors.badSequenceNo,i);
			}
			else if ((expectedSeqNo > rxedSeqNo) &&
					(expectedSeqNo < (rxedSeqNo+5)))
			{
				lines[i].errors.badSequenceNo --;
			}

			if (loopback)
			{
				sendto(pollA[i].fd, buff, rc, 0,
					(struct sockaddr *)&from, fromlen);
			}
		}
	}
	goto _poll;
}

void *
Send(void *arg)
{
	struct sockaddr_in addr;
	char buff[1024];
	int i, n = 0, rc, tolen = sizeof(struct sockaddr_in);
	struct msghdr msg;
	struct iovec bufs[2];
	hrtime_t shoottime, spenttime, time_margin = 1000;
	struct timespec delay;
	TxRxThreadArg *targ = (TxRxThreadArg *)arg;
	int start, end;
	int loopcnt=0;
	int payload_len = 0;
	Stats *stp;

	memset(buff, 0x71, 1024);
#ifndef NETOID_LINUX
	if (realtime) 
	{
		ThreadSetRT();
	}
#endif //NETOID_LINUX
	start = targ->start;
	end = targ->n+start;
	stp = targ->stat_p;

	delay.tv_sec = 0; 		/* Assume that interpacket gap is less than 1 sec */
	shoottime = nx_gethrtime();

_poll:
	// Start counting execution time from here
	// Deduct it from the interval in the end
	shoottime += payloadInterval;

	n = 0;
	for (i=start; i<end; i++)
	{
		if (lines[i].addr.sin_addr.s_addr)
		{
			pollS[i].events = POLLOUT;
			n++;
		}
		else
		{
			pollS[i].events = 0;
		}

		pollS[i].revents = 0;
	}

	if (n > 0)
	{
		rc = poll(pollS+start, targ->n, -1);
	}
	else
	{
		sleep(1);
		goto _poll;
	}

	if (rc <= 0)
	{
		fprintf(stderr, "Send poll error, errno %s\n", strerror(errno));
		goto _poll;
	}

	// On all sockets which have read set, do the read/write
	for (i=start; i<end; i++)
	{
		if (pollS[i].revents & POLLOUT)
		{
			NIncShort(lines[i].sndrtpHdr.seqno); 
            
            if (dtmfLock)
            {
                goto _poll;
            }
            MakePayload(buff, &payload_len);

			bufs[0].iov_base = (char *)&lines[i].sndrtpHdr;
			bufs[0].iov_len = sizeof(RtpHeader);
			bufs[1].iov_base = buff;
			bufs[1].iov_len = payload_len;

			msg.msg_name = (char *)&lines[i].saddr;
			msg.msg_namelen = sizeof(struct sockaddr_in);
			msg.msg_iov = bufs;
			msg.msg_iovlen = 2;

			if (msg.msg_namelen) 
			{
				if ((rc = sendmsg(pollS[i].fd, &msg, 0)) < 0) 
				{
			    	fprintf(stderr, "Sendmsg error, errno %s\n", 
							strerror(errno));
			  	}
			  	else 
				{
			    	(stp->numpkts)++;
			    	(stp->numbytes) += payloadLen;	// RTP hdr len not included
			  	}
			} 
			else 
			{
			 	fprintf(stderr,"Line %d already closed\n",i);
			}
            if (sendDtmf)
            {
                if (buff[1] & 0x80)
                {
                    sendDtmf = False;
                    InitRtpHeader(&lines[i].sndrtpHdr,i, 0);
                }
            }
			/* fill information about who the packet was send to */
			mgenStats.sentToIp = ntohl(lines[i].saddr.sin_addr.s_addr);		
			mgenStats.sentToPort = ntohs(lines[i].saddr.sin_port);		
		}
	}

	spenttime = nx_gethrtime();
	
	if ((shoottime - spenttime - time_margin) > 0)  /* time_margin is 1 microsec */
	{
		delay.tv_nsec = (shoottime-nx_gethrtime());
		nanosleep(&delay, NULL);
	}
	else {
		(stp->skip_sleep)++;
	}

	goto _poll;
}

int
LaunchTxRxThreads(void *(*fn)(void *), Stats *stp)
{
	int i, start, n, left;
	TxRxThreadArg *arg;

	// Launch nthreads in all. Each thread
	// will be doing the task of sending to its assigned
	// set of sockets, and at the assigned rate
	start = 0;
	n = nCalls/nthreads;
	left = nCalls%nthreads;

	for (i = 0; i<nthreads; i++)
	{
		arg = (TxRxThreadArg *)malloc(sizeof(TxRxThreadArg));
		arg->start = start;
		arg->n = n;
		arg->stat_p = (stp + i);
		start += n;
		LaunchThread(fn, arg);
	}

	if (left)
	{
//bug:		start += n;
		LeftOver = 1;
		arg = (TxRxThreadArg *)malloc(sizeof(TxRxThreadArg));
		arg->start = start;
		arg->n = left;
		arg->stat_p = (stp + i);
		LaunchThread(fn, arg);
	}
}

int
InitRtpHeader(RtpHeader *rtpHeader, int line, int dtmfFlag)
{
	memset(rtpHeader, 0, sizeof(RtpHeader));
#if 0
	rtpHeader->version = 0x2;
    rtpHeader->p = 0x1;
    rtpHeader->x = 0x1;
    rtpHeader->cc = 0x1;
    rtpHeader->m = 0x0;
	rtpHeader->pt = 0x60;
#endif
	rtpHeader->first = 0x80;
    if (dtmfFlag)
        rtpHeader->second = 0x65;
    else
        rtpHeader->second = 0x60;
	rtpHeader->timestamp = htonl(time(0));
	rtpHeader->ssrc = htonl(line);
}

int
MakePayload(char *buf, int *len)
{
    int i;
    char   a[10];
    static counter;
    if (sendDtmf)
    {
        buf[0] = dtmfInfo.digit;
        buf[1] = dtmfInfo.volume;
        //sprintf(a, "%x", dtmfInfo.volume);
        buf[1] = buf[1] & 0x3f;
        counter++;
        if (counter == 4)
        {
            buf[1] = buf[1] | 0x80;
            counter = 0;
        }
        i = (dtmfInfo.duration)/1024; /* divide by 4*256 */
        sprintf(a, "%x", i);
        buf[2] = i;
        i = dtmfInfo.duration/4;
        sprintf(a, "%x", i);
        buf[3] = i;
        *len = 4;
    }
    else
    {
        memset(buf, 0x71, 1024);
        *len = payloadLen;
    }

    return 0;
}

char *
ULIPtostring(unsigned long ipaddress)
{
	static char outstring[16];

	sprintf(outstring, "%u.%u.%u.%u",
			(ipaddress & 0xff000000) >> 24,
			(ipaddress & 0x00ff0000) >> 16,
			(ipaddress & 0x0000ff00) >> 8, 
			(ipaddress & 0x000000ff));

	return outstring;
}

int 
usage(void)
{
	fprintf(stdout, 
"mgen [-n <no. of calls>] - default 1\n");
	fprintf( stdout, 
"     [-h] - help for usage\n");
	fprintf( stdout, 
"     [-c <controller (gen/sgen) IP addr>] - default -1\n");
	fprintf( stdout, 
"     [-M <controller (gen/sgen) port>] - default 49160\n");
	fprintf( stdout, 
"     [-L <local IP addr>] - default local IP\n");
	fprintf( stdout, 
"     [-m <media start port>] - default 49200\n");
	fprintf( stdout, 
"     [-x <media threads>]\n");
	fprintf( stdout, 
"     [-b <RTP payload length (bytes)> <payload interval (nanosec>] - default 30, 30000000\n");
	fprintf( stdout, 
"     [-R] - do not run in realtime\n");
	fprintf( stdout, 
"     [-s] - send and receive on same port\n");
	fprintf( stdout, 
"     [-o] - loopback\n");
	fprintf( stdout, 
"     [-T] - run in automated testing mode\n");
	fprintf( stdout, 
"     [-v <verbosity level>] - for printing stats, ignored with option -T\n");
}

int
sgenInform(int command)
{
	MgenControlMsg mcm; 
    
    mcm.command = htonl(command); 
    
    if (write(controllerFd, (char *)&mcm, sizeof(MgenControlMsg)) < 4)
    {
        perror("mgenInform() - no connection with mgen, write error ");
        return -1;
    }

	return 0;
}
