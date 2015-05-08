#ifdef __cplusplus
extern "C" {
#endif


#ifndef __SNMPI_H
#define  __SNMPI_H

#include <h341common.h>
#include <cm.h>


DECLARE_OPAQUE(h341InstanceHandle); 
DECLARE_OPAQUE(h341AgentComHandle); 
DECLARE_OPAQUE(h341ApplHandle); 








typedef void (*h341InstEvChangeGlobalsT)(IN h341AgentComHandle hAgentCom,
                                  IN h341ParameterName name,
                                  IN int offset);




typedef BOOL (*h341InstEvReadWriteSetT)(IN h341ApplHandle h341hApp,
                                    IN h341ParameterName name,
                                    IN char * value,
                                    IN int size);




h341ErrorT h341InstInit(	    IN	h341ApplHandle hApp,
						        IN	int maxCalls,
						        IN	int maxChannels,
						        IN	h341InstEvReadWriteSetT  h341EvReadWriteSet,					
						        IN  h341InstEvChangeGlobalsT   h341ChangeGlobals,
                                OUT h341InstanceHandle * h341InstHandle);
							




h341ErrorT h341InstAddNewCall(  
                          IN h341InstanceHandle h341hInst,
                          IN HCALL hsCall);


h341ErrorT	   h341InstDeleteCall(IN h341InstanceHandle h341hInst,
                          IN HCALL hsCall);


h341ErrorT h341InstAddNewLogicalChannel(
                                    IN h341InstanceHandle h341hInst,
                                    IN HCHAN hsChan);


h341ErrorT	   h341InstDeleteLogicalChannel(IN h341InstanceHandle h341hInst,
                                    IN HCHAN hsChan);	

h341ErrorT h341InstAddControl(IN h341InstanceHandle h341hInst,                          
                          IN HCALL hsCall);

h341ErrorT	   h341InstDeleteControl(IN h341InstanceHandle h341hInst,
                             IN HCALL hsCall);




void h341InstEnd(IN h341InstanceHandle h341hInst);

h341ErrorT h341InstGetParameter(IN h341InstanceHandle h341hInst,
                            IN h341ParameterName name,
                            OUT mibDataT *data);

h341ErrorT h341InstGetNextIndex(IN h341InstanceHandle h341hInst,
                            IN h341ParameterName name,
                            IN UINT8 * index, 
                            IN int indexSize,
                            OUT UINT8 *nextIndex,
                            OUT int *nextSize,
				            OUT mibDataT *data);

h341ErrorT h341InstGetByIndex (IN h341InstanceHandle h341hInst ,
                           IN UINT8 * index, 
                           IN int indexSize, 
				           IN h341ParameterName name,
                           OUT mibDataT *data);

h341ErrorT h341InstGetMRParameter(IN h341InstanceHandle hSnmp,
                              INOUT instanceRequestDataPtrT * requestsPtr,
                              IN int reqNum);

h341ErrorT h341InstSetRequest(IN h341InstanceHandle h341hInst,
                            IN h341ParameterName name,
                            IN mibDataT *data);




void    h341InstSetAgentComHandle (IN h341InstanceHandle h341hInst,
                               IN h341AgentComHandle   handle);


void h341SetInstanceNumber(h341InstanceHandle hSnmp,int index);


#endif

#ifdef __cplusplus
}              
#endif







