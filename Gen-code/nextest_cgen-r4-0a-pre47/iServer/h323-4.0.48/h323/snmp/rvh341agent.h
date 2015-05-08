#ifdef __cplusplus
extern "C" {
#endif

#ifndef __H341_AGENT_H
#define __H341_AGENT_H

#include <h341common.h>

DECLARE_OPAQUE(h341AgentHandleT); 
DECLARE_OPAQUE(h341RegisterHandleT); 


/*#define MAX_SUB_AGENT 5*/


typedef int (*h341AgentEvSetParameterT)(
                                 IN h341RegisterHandleT reg,
                                 IN h341ParameterName name,
                                 IN mibDataT *data);

typedef int (*h341AgentEvMRRetrieveT)(  IN h341RegisterHandleT reg,
                                 IN instanceRequestDataPtrT * multReqPtr,
                                 IN int reqNum);




h341ErrorT h341AgentInit(	IN int maxRequestNumber,
                                IN int maxInstanceNumber,
								IN h341AgentEvSetParameterT  h341SetParameter,
								IN h341AgentEvMRRetrieveT	   h341MRRetrieve,
                                OUT h341AgentHandleT  * agentHandle);


void h341AgentEnd(IN h341AgentHandleT hAgent);


int h341AgentRegister(IN h341AgentHandleT hAgent,
                      IN h341RegisterHandleT reg);

void h341AgentUnRegister(IN h341AgentHandleT hAgent,
                         IN int ifIndex);

snmpObjectT * h341AgentGetSubTrees(IN h341AgentHandleT hAgent,
                              OUT int * size); 


int h341AgentGetNext( IN  h341AgentHandleT hAgent,
                        IN  snmpObjectT * oid, 
                        OUT snmpObjectT * oidNext,
                        OUT h341ErrorT  * error,
                        IN  int len);


h341ErrorT h341AgentSetValue(    IN h341AgentHandleT hAgent,
                            IN snmpObjectT * oid, 
                            IN mibDataT *data);


int h341AgentGetValue(    IN h341AgentHandleT hAgent,
                            OUT snmpObjectT * oid,  
                            OUT h341ErrorT  * error,
                            IN int len);



void h341AgentChangeGlobals( IN h341AgentHandleT hAgent,
                        IN h341ParameterName name,
                        IN int offset);






#endif

#ifdef __cplusplus
}
#endif
