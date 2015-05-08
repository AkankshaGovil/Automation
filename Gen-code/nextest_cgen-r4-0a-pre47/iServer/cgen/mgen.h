#ifndef _mgen_h_
#define _mgen_h_
#include "nxosdtypes.h"

typedef enum commands {
    stop = 0,
    begin,
    mstat,
    dtmf
} commands;

// Message sent by controller to mgen
typedef struct
{
	commands command;

	unsigned short rcvport;
	unsigned long rcvip;

	unsigned long sndip;
	unsigned short sndport;
} MgenControlMsg;

typedef struct
{
    unsigned char       digit;
    unsigned int        duration;
    unsigned int        volume;
} DtmfInfo;
    

// Stats sent from mgen to controller 
typedef struct 
{
	longlong_t 	txBytes;
	longlong_t 	txPkts;
	float		txBitRate;	
	float		txPktRate;	
	longlong_t 	rxBytes;
	longlong_t 	rxPkts;
	float		rxBitRate;	
	float		rxPktRate;	
	unsigned long sentToIp;
	unsigned short sentToPort;
	unsigned long receivingFromIp;
	unsigned short receivingFromPort;
	int			fromPortChanged;
} MgenStats;

// does not contain csrc's, cc should be set to 0
// if there are no csrc's appended to this header
typedef struct
{
#if 0
	unsigned int version:2;
	unsigned int p:1;
	unsigned int x:1;
	unsigned int cc:4;
	unsigned int m:1;
	unsigned int pt:7;
#endif
    unsigned char first;
    unsigned char second;
	unsigned short seqno;
	unsigned int timestamp;
	unsigned int ssrc;

} RtpHeader;

#define NIncInt(_x_)	do { int y; y = ntohl(_x_); y++; _x_ = htonl(y); } while (0)
#define NIncShort(_x_)	do { short y; y = ntohs(_x_); y++; _x_ = htons(y); } while (0)

//Verbosity levels

#define  V_GLOBAL	1		// Activate global level stats verbosity
#define  V_THREAD	2		// Activate per thread level stats verbosity
#define  V_CALL		3		// Activate per call level stats verbosity	

#endif /* _mgen_h_ */
