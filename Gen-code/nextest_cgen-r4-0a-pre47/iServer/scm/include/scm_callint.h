#ifndef _SCMCALLINT_H_
#define _SCMCALLINT_H_

#define SCMCALL_IsStable(callHandle)	(callHandle->state == Sip_sConnectedAck)

extern cache_t scmCallCache;

extern int SCMCALL_HandleNewElement(SCM_QueueItem *elt);
extern int SCMCALL_HandleDeleteData(void *data);
extern int SCMCALL_ReplicateWorker(void *data);
extern int SCMCALL_DeleteWorker(void *data);

#endif /* _SCMCALLINT_H_ */
