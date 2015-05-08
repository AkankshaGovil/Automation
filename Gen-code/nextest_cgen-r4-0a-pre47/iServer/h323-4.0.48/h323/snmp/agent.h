#ifdef __cplusplus
extern "C" {
#endif

#ifndef __AGENT_H
#define __AGENT_H

#include <h341common.h>

DECLARE_OPAQUE(h341AgentHandleT); 
DECLARE_OPAQUE(h341RegisterHandleT); 


/*#define MAX_SUB_AGENT 5*/


typedef int (*h341SetParameterT)(
                                 IN h341RegisterHandleT reg,
                                 IN h341ParameterName name,
                                 IN mibDataT *data);

typedef int (*h341MRRetrieveT)(  IN h341RegisterHandleT reg,
                                 IN instanceRequestDataPtrT * multReqPtr,
                                 IN int reqNum);




h341AgentHandleT h341AgentInit(	IN int maxRequestNumber,
                                IN int maxInstanceNumber,
								IN h341SetParameterT  h341SetParameter,
								IN h341MRRetrieveT	   h341MRRetrieve	);


void h341AgentEnd(IN h341AgentHandleT hAgent);


int h341AgentRegister(IN h341AgentHandleT hAgent,
                      IN h341RegisterHandleT reg);

void h341AgentUnRegister(IN h341AgentHandleT hAgent,
                         IN int ifIndex);

snmpObjectT * h341GetSubTrees(IN h341AgentHandleT hAgent,
                              OUT int * size); 


int h341GetNext( IN  h341AgentHandleT hAgent,
                        IN  snmpObjectT * oid, 
                        OUT snmpObjectT * oidNext,
                        OUT h341ErrorT  * error,
                        IN  int len);


h341ErrorT h341SetValue(    IN h341AgentHandleT hAgent,
                            IN snmpObjectT * oid, 
                            IN mibDataT *data);


int h341GetValue(    IN h341AgentHandleT hAgent,
                            OUT snmpObjectT * oid,  
                            OUT int  * error,
                            IN int len);



void h341ChangeGlobals( IN h341AgentHandleT hAgent,
                        IN h341ParameterName name,
                        IN int offset);



/*h341ErrorT h341RetrieveGlParameter (IN h341AgentHandleT hAgent,
                                    IN h341ParameterName name,
                                    OUT mibDataT *data);

*/



#endif

#ifdef __cplusplus
}
#endif
