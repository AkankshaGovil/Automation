#ifndef _SCM_CALL_H_
#define _SCM_CALL_H_

extern int SCMCALL_Init (void);
extern int SCMCALL_Replicate (CallHandle *callHandle);
extern int SCMCALL_Delete (CallHandle *callHandle);
extern int SCMCALL_CheckState (char *callid);

#endif /* _SCM_CALLAPI_H_ */
