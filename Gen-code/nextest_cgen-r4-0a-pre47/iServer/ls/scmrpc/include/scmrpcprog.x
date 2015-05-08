#ifdef RPC_HDR
%
%#include "calldefs.h"
%#include "call_handle.h"
%
%#define CALL_SENDBUFF_LEN	16384
%
#endif /* RPC_HDR */

struct _ScmrpcCallid {
	char id[CALL_ID_LEN];
};

typedef struct _ScmrpcCallid ScmrpcCallid;

typedef opaque OpqBuff<CALL_SENDBUFF_LEN>;
typedef CallHandle  *pCallHandle;
typedef ScmrpcCallid *pScmrpcCallid;
typedef OpqBuff *pOpqBuff;
program SCM_PROG {
	version SCM_VERS {

	/* b/nEvP = bridge/network event processor fn name */
	void SCM_UPDATECALL(pCallHandle , string bEvP<>, string nEvP<>) = 1;

	void SCM_DELETECALL(pScmrpcCallid) = 2;
	int  SCM_HEARTBEAT(unsigned long, unsigned long) = 3;
	void SCM_SEND_OPQ(pOpqBuff) = 4;
	} = 1;
} = 0x20000076;
