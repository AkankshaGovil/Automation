#ifndef _gisq_h_
#define _gisq_h_

/*
 * GISQ related definitions 
 */

#define GIS_SRVR_MSG_TYPE			0x1

/* msg types */
typedef enum
{
	GISQMSG_RSA = 1,
	GISQMSG_CALLDROP,
	GISQMSG_CALLPRINT,
	GISQMSG_CLICMD,
	GISQMSG_OK,
	GISQMSG_ERROR,

	GISQMSG_MAX, 
} GISQMSG;

typedef struct
{
	unsigned long	realmId;
	unsigned short	protocols;
	unsigned short	newStates;

}QMsgSigStateChg;

typedef struct
{
}QMsgCallDrop;

typedef struct
{
	QMsgHdr 		hdr;
	union
	{
		QMsgSigStateChg		stateChange;
		QMsgCallDrop		callDrop;
	}m;
} QGisSrvrMsg;


static QDispatchTable   gisQDispatchTable;
static QDesc gisQ;
static void* GisQReader(void *arq);



#endif /*_gisq_h_ */
