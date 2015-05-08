#ifndef _SCMRPC_API_H_
#define _SCMRPC_API_H_

#include "call_handle.h"

/* client apis */

int scm_create_client(char *host); /* host is the rpc server */ 
int scm_updatecall (CallHandle *h);
int scm_deletecall (char *id);
int scm_heartbeat (ulong myip, ulong myid);

int scm_encodecall (CallHandle *ch, char **buff);
int scm_rpcsend (char *buff, uint len);

/* batching ...
void *scmflushcalls ();
*/
void scm_delete_client ();

/* server apis */

int  scm_create_server ();
void scm_initcb_updatecall (void (*cb)(CallHandle *h));
void scm_initcb_deletecall (void (*cb)(char *id));
void scm_initcb_heartbeat (void (*cb)(ulong peer_ip, ulong peer_id));

void scm_decodecall(char *buff);

#endif
