#define _GNU_SOURCE
#include <dlfcn.h>
#include "scmrpcprog.h"
#include "call_handle.h"
#ifndef RTLD_SELF
#define RTLD_SELF RTLD_DEFAULT
#endif
#include <rpc/rpc.h>
#include "nxosd.h"

/* extern functions */
extern void scm_prog_1(struct svc_req *, register SVCXPRT *);

/* static functions */
static void *scm_thread_handle(void *p);
static CallHandle *scm_decodecall(char *buff, int len);

/* static data */
typedef void (*CbUpdateCall)(CallHandle *);
typedef void (*CbDeleteCall)(char *);
typedef void (*CbHeartbeat)(ulong, ulong);

static CbUpdateCall cb_updatecall = NULL;
static CbDeleteCall cb_deletecall = NULL;
static CbHeartbeat  cb_heartbeat  = NULL;


int scm_create_server ()
{
	pthread_t t_handle;
	int ret;

	ret = pthread_create(&t_handle, NULL, scm_thread_handle, NULL);
	if(ret!=0)
	{
		NETERROR(MSCMRPC, ("unable to create thread error = %d\n", ret));
		return (-1);
	}
	return (0);
}

void scm_initcb_updatecall(CbUpdateCall cb)
{
	cb_updatecall = cb;
}

void scm_initcb_deletecall(CbDeleteCall cb)
{
	cb_deletecall = cb;
}

void scm_initcb_heartbeat(CbHeartbeat  cb)
{
	cb_heartbeat = cb;
}

void *
scm_updatecall_1_svc(CallHandle *arg1, char *bpr, char *npr,
						struct svc_req *rqstp)
{
	static char res;

	if (cb_updatecall)
	{
		/* get bridge/network event processor fn ptrs */		

		if (bpr && (strlen(bpr) > 0))
			arg1->bridgeEventProcessor = dlsym(RTLD_SELF, bpr);

		if (npr && (strlen(npr) > 0))
			arg1->networkEventProcessor = dlsym(RTLD_SELF, npr);

		cb_updatecall(arg1);
	}

	return((void *)&res);
}

void *scm_deletecall_1_svc(ScmrpcCallid *arg1, struct svc_req *rqstp)
{
	static char res;

	if(cb_deletecall)
	{
		cb_deletecall(arg1->id);
	}

	return((void *)&res);
}

int *scm_heartbeat_1_svc(ulong arg1, ulong arg2, struct svc_req *rqstp)
{
	static int result = 0;

	if(cb_heartbeat)
	{
		cb_heartbeat(arg1, arg2);
	}

	return (&result);
}


void *
scm_send_opq_1_svc(OpqBuff *arg1, struct svc_req *rqstp)
{
	static char res;
	CallHandle *ch;

	ch = scm_decodecall(arg1->OpqBuff_val, arg1->OpqBuff_len);

	if((cb_updatecall != NULL) && (ch != NULL))
	{
		cb_updatecall(ch);
	}

	return((void *)&res);
}

static void *scm_thread_handle (void *p)
{
	int info;


	if(!nx_svctcp_create(scm_prog_1, SCM_PROG, SCM_VERS))
	{
		NETERROR(MSCMRPC, ("svctcp_create failed\n"));
		return(NULL);
	}
	
#ifndef NETOID_LINUX

	info = 1;

	if(!rpc_control(RPC_SVC_USE_POLLFD, &info))
	{
		NETERROR(MSCMRPC, ("rpc_control failed to set RPC_SVC_USE_POLLFD\n"));
	}

#endif

	svc_run();

	NETDEBUG(MSCMRPC, NETLOG_DEBUG1, ("svc_run returned\n"));
	return(NULL);
}

static CallHandle *scm_decodecall(char *buff, int len)
{
	XDR xhandle;
	CallHandle *ch;

	ch = (CallHandle *)malloc(sizeof(CallHandle));
	memset(ch, 0, sizeof(CallHandle));

	xdrmem_create(&xhandle, buff, len, XDR_DECODE);

	if (xdr_CallHandle(&xhandle, ch) != TRUE)
	{
		free(ch);
		ch = NULL;
	}

	return (ch);
}
