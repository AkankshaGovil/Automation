#include <stdio.h>
#include <stdlib.h>
#define __USE_GNU
#include <dlfcn.h>
#undef __USE_GNU
#include "scmrpcprog.h"
#include "scmrpc_api.h"

static CLIENT *clnt = NULL;
static int ch_buff[CALL_SENDBUFF_LEN/4]; 

int scm_create_client(char *host)
{
	void  *result_1;

	scm_delete_client ();

	clnt = clnt_create(host, SCM_PROG, SCM_VERS, "tcp");
	if (clnt == (CLIENT *) NULL) {
		NETERROR(MSCMRPC, ("%s\n", clnt_spcreateerror("scm_create_client")));
		return (-1);
	}
	return 0;
}

int scm_updatecall(CallHandle *ch)
{
	Dl_info dlinfo;
	char *bEvP_name, *nEvP_name;

	if(clnt)
	{

		/* get bridge/network event processor function names */ 

		if (dladdr(ch->bridgeEventProcessor, &dlinfo))
			bEvP_name = (char *)dlinfo.dli_sname;
		else
			bEvP_name = NULL;

		if (dladdr(ch->networkEventProcessor, &dlinfo))
			nEvP_name = (char *)dlinfo.dli_sname;
		else
			nEvP_name = NULL;

		if (scm_updatecall_1(ch, bEvP_name, nEvP_name, clnt)!=NULL)
			return (0);
		else 
		{
			NETERROR(MSCMRPC, ("%s\n", clnt_sperror(clnt, "scm_updatecall")));
			return (-1);
		}
	}
	else
	{
		NETERROR(MSCMRPC, ("uninitialized client - create client first\n"));
		return (-1);
	}
}

int scm_deletecall(char *id)
{
	ScmrpcCallid callid;
	if(clnt)
	{
		if(id)
		{
			memcpy(callid.id, id, CALL_ID_LEN);
			if(scm_deletecall_1(&callid, clnt)!=NULL)
			{
				return (0);
			}
			else
			{
				NETERROR(MSCMRPC, ("%s\n", 
					clnt_sperror(clnt, "scm_deletecall")));
				return (-1);
			}
		}
		else
		{
			NETERROR(MSCMRPC, ("cannot delete callid = NULL\n"));
			return (-1);
		}
	}
	else
	{
		NETERROR(MSCMRPC, ("uninitialized client - create client first\n"));
		return (-1);
	}
}

int scm_heartbeat(ulong myip, ulong myid)
{
	int *res;

	if(clnt)
	{
		res = scm_heartbeat_1(myip, myid, clnt);
		if(res)
			return(*res);
		else
		{
			NETERROR(MSCMRPC, ("%s\n", clnt_sperror(clnt, "scm_heartbeat")));
			return (-1);
		}
	}
	else
	{
		NETERROR(MSCMRPC, ("uninitialized client - create client first\n"));
		return (-1);
	}
}

int scm_encodecall(CallHandle *ch, char **buff)
{
	XDR	xhandle;
	int size;

	xdrmem_create(&xhandle, (char *)ch_buff, CALL_SENDBUFF_LEN, XDR_ENCODE);   
	if (xdr_CallHandle(&xhandle, ch) != TRUE)
	{
		*buff = NULL;
		size = 0;
	}
	else
	{
		*buff = (char *)ch_buff;
		size = xdr_getpos(&xhandle); 
	}

	return (size);
}

int scm_rpcsend (char *buff, uint len)
{
	OpqBuff op;

	op.OpqBuff_len = len;
	op.OpqBuff_val = buff;

	if (scm_send_opq_1 (&op, clnt) != NULL)
	{
		return (0);
	}
	else
	{
		NETERROR(MSCMRPC, ("%s\n", clnt_sperror(clnt, "scm_deletecall")));
		return (-1);
	}
}

void scm_delete_client ()
{
	if(clnt)
	{
		NETDEBUG(MSCMRPC, NETLOG_DEBUG2,
			("existing client, deleting\n"));
		clnt_destroy(clnt);
	}
	clnt = NULL;
};
