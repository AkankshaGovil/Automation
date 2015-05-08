#include "scmrpcprog.h"
#include <stdio.h>
#include <stdlib.h> /* getenv, exit */
#include <signal.h>
#include <rpc/pmap_clnt.h> /* for pmap_unset */
#include <string.h> /* strcmp */
#include <unistd.h> /* setsid */
#include <sys/types.h>
#include <memory.h>
#include <stropts.h>
#include <sys/resource.h> /* rlimit */
#include <syslog.h>

#ifndef SIG_PF
#define	SIG_PF void(*)(int)
#endif

#define	_RPCSVC_CLOSEDOWN 120

/* States a server can be in wrt request */

#define	_IDLE 0
#define	_SERVED 1

static int _rpcsvcstate = _IDLE;	/* Set when a request is serviced */
static int _rpcsvccount = 0;		/* Number of requests being serviced */

static void *
_scm_updatecall_1(scm_updatecall_1_argument  *argp, struct svc_req *rqstp)
{
	void *p;

	p = scm_updatecall_1_svc(argp->arg1, argp->bEvP, argp->nEvP, rqstp);

	/* lose the call handle to the application */ 

	argp->arg1 = NULL;

	return (p);
}

static void *
_scm_deletecall_1(ScmrpcCallid *argp, struct svc_req *rqstp)
{
	return (scm_deletecall_1_svc(argp, rqstp));
}

static int *
_scm_heartbeat_1(scm_heartbeat_1_argument *argp, struct svc_req *rqstp)
{
	return (scm_heartbeat_1_svc(argp->arg1, argp->arg2, rqstp));
}

static void *
_scm_send_opq_1(OpqBuff *argp, struct svc_req *rqstp)
{
	return (scm_send_opq_1_svc(argp, rqstp));
}

void
scm_prog_1(struct svc_req *rqstp, register SVCXPRT *transp)
{
	union {
		scm_updatecall_1_argument scm_updatecall_1_arg;
		ScmrpcCallid scm_deletecall_1_arg;
		scm_heartbeat_1_argument scm_heartbeat_1_arg;
		OpqBuff scm_send_opq_1_arg;
	} argument;
	char *result;
	xdrproc_t _xdr_argument, _xdr_result;
	char *(*local)(char *, struct svc_req *);

	_rpcsvccount++;
	switch (rqstp->rq_proc) {
	case NULLPROC:
		(void) svc_sendreply(transp,
			(xdrproc_t) xdr_void, (char *)NULL);
		_rpcsvccount--;
		_rpcsvcstate = _SERVED;
		return;

	case SCM_UPDATECALL:
		_xdr_argument = (xdrproc_t) xdr_scm_updatecall_1_argument;
		_xdr_result = (xdrproc_t) xdr_void;
		local = (char *(*)(char *, struct svc_req *)) _scm_updatecall_1;
		break;

	case SCM_DELETECALL:
		_xdr_argument = (xdrproc_t) xdr_ScmrpcCallid;
		_xdr_result = (xdrproc_t) xdr_void;
		local = (char *(*)(char *, struct svc_req *)) _scm_deletecall_1;
		break;

	case SCM_HEARTBEAT:
		_xdr_argument = (xdrproc_t) xdr_scm_heartbeat_1_argument;
		_xdr_result = (xdrproc_t) xdr_int;
		local = (char *(*)(char *, struct svc_req *)) _scm_heartbeat_1;
		break;

	case SCM_SEND_OPQ:
		_xdr_argument = (xdrproc_t) xdr_OpqBuff;
		_xdr_result = (xdrproc_t) xdr_void;
		local = (char *(*)(char *, struct svc_req *)) _scm_send_opq_1;
		break;

	default:
		svcerr_noproc(transp);
		_rpcsvccount--;
		_rpcsvcstate = _SERVED;
		return;
	}
	(void) memset((char *)&argument, 0, sizeof (argument));
	if (!svc_getargs(transp, _xdr_argument, (caddr_t) &argument)) {
		svcerr_decode(transp);
		_rpcsvccount--;
		_rpcsvcstate = _SERVED;
		return;
	}
	result = (*local)((char *)&argument, rqstp);
	if (_xdr_result && result != NULL && !svc_sendreply(transp, _xdr_result, result)) {
		svcerr_systemerr(transp);
	}

	if (!svc_freeargs(transp, _xdr_argument, (caddr_t) &argument)) {
		NETERROR(MSCMRPC, ("unable to free arguments\n"));
		exit(1);
	}
	_rpcsvccount--;
	_rpcsvcstate = _SERVED;
	return;
}
