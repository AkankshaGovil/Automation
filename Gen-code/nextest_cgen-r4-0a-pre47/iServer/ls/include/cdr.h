#ifndef _cdr_h_
#define _cdr_h_

#include <sys/mman.h>
#include <sys/fcntl.h>

// cdr types
#define CDRMINDCTIFIXED	1
#define CDRMINDCTIDAILY	2
#define CDRMINDCTISEQ	3
#define CDRNEXTONETIME  4

#define CDRDIRNAMELEN	256

#define BILLING_NONE	0		// no billing
#define BILLING_POSTPAID 1		
#define BILLING_PREPAID  2
#define BILLING_CISCOPREPAID  3

// cdr formats - dont change them, as the jserver will croaks...
#define CDRFORMAT_XML		0
#define CDRFORMAT_MIND		1
#define CDRFORMAT_SYSLOG	2

#define MAX_CDR_TIMER_VALUE     500000   // over a year

// cdr duration resolution
#define CDR_RES_SEC		0
#define CDR_RES_MSEC	1

#define CDRSEQFNAME	"/databases/cdrseqno"

#define CDRSTART1	0x1
#define CDRSTART2	0x2
#define CDREND1		0x4
#define CDREND2		0x8
#define CDRHUNT		0x10

struct CallHandleStruct;

typedef struct
{
	struct CallHandleStruct *callHandle;
	int flag;
} CdrArgs;

typedef CdrArgs *pCdrArgs;

void CdrArgsFree(CdrArgs *args);
void CdrEnd(void);

CdrArgs * CdrArgsDup(struct CallHandleStruct *callHandle, int flag);
int ConfEnd(void);
int CdrInit(void);
int CdrCfg(void);
int CdrFlush(void);
int CdrRotateFile(void);
int CdrCountFiles(char *dir, char *pattern);
int CdrMoveTempFiles(char *dir, char *textn, char *extn, char *exclude);
int cdrQueueCdr(void *arg);

// A CDR is not valid if hunts has been exceeded
// and it has a previous cdr buffered.
#define BillCallIsCdrValid(callHandle) \
		(!((callHandle->nhunts > SYSTEM_MAX_HUNTS) && callHandle->prevcdr))


extern void BillCallSetFromPrevCdr (struct CallHandleStruct *callHandle);

extern int BillCallPrevCdr (struct CallHandleStruct *callHandle);
extern int BillCall (struct CallHandleStruct *callHandle, int flag);
extern int BillCallWorker(struct CallHandleStruct *callHandle, int flag);

#endif
