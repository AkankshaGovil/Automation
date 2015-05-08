#ifdef __cplusplus
extern "C" {
#endif

#ifndef __MIB_H
#define __MIB_H
#include <snmpI.h>
#include <rvh341agent.h>
#include <ra.h>

typedef struct mibNodeStruct * pMibNode;
typedef struct mibNodeStruct  mibNodeT;

typedef enum 
{
	pduGetParam=1,
	pduGetNext,
	pduGetNextError,
	pduSetParam,
	pduGetIfIndex
}enumPduTypeT;


#define noSuchRoot  1
#define noSuchField 2
#define noSuchIndex 3

#define lastRaw		5

#define invalidIfIndex 7
#define nextIfIndex 8
#define invalidIndex 9
#define noRegisterInstance 10
#define noRequestNumber	11
#define invalidData 12
#define notImplemented 20
#define checkNext 20
#define outOfMib  21  
#define noSuchInstance 50


typedef mibNodeT * 
	(  * getH323ParameterT)( h341AgentHandleT hSnmp,enumPduTypeT pduType,snmpObjectT * oid,int *offset,mibNodeT * node,
						   void * buffer, int * error);


struct mibNodeStruct
{
	mibNodeT *  children;
	mibNodeT *  parent;
	UINT8 childNum;
	BOOL table;
	getH323ParameterT 	getH323Parameter;
	h341ParameterName name;
};




typedef int  sortDataHandleT ;
DECLARE_OPAQUE(CALLH);
DECLARE_OPAQUE(tableHandleT);

typedef int (* tableGetDataT)(RAElement data,h341ParameterName name,mibDataT * mibData);
typedef int ( * tableCompareT)(UINT8 *key,int kSize ,int pos,tableHandleT table);
typedef UINT8 *(* tableGetIndexT)(RAElement 	data);
typedef int (*tableSetDefaultValT)(RAElement data,h341ParameterName name,mibDataT * mibData);


typedef struct
{
	sortDataHandleT *sortData; /* sorted array of n elements,that each element is pointer to
								data*/
	int				size;
	int				keySize;
	HRA 			data; /*array of element,that each element contains snmp index(key) and h323stack handle
							to data or data itself*/
	tableGetDataT   get;
	tableCompareT	compare;
	tableGetIndexT  getIndex;
}tableT;



tableT *  tableConstruct(int nElements,int keySize,int elementSize,tableGetDataT   get,RAECompare	compare,
						 tableGetIndexT getIndex);
void tableDestruct(tableT *table);
int tableAdd(tableT *table,RAElement 	data,int *entry);
int tableDelete(tableT *table,UINT8 * index,int iSize);
int tableSetDefault(tableT *table,int pos,h341ParameterName name,mibDataT *data,tableSetDefaultValT  setValue);
UINT8 * tableFindIndex(tableT *table,void * 	data);
int  tableUpdateIndex(tableT *table,UINT8 * index,UINT8 *  newkey,int iSize);





int getNextIndex (tableT * table ,h341ParameterName name,UINT8 * index, int indexSize,
				  UINT8 *nextIndex,int *nextSize,mibDataT *data);
int getByIndex (tableT * table ,UINT8 * index, int indexSize, 
				h341ParameterName name,mibDataT *data);
				


typedef struct
{
	int ifIndex;
	snmpObjectT *pSourceOid;
    snmpObjectT *pResultOid;
    instanceRequestDataT instReqData;
}RequestDataT;



typedef RequestDataT * RequestDataPtrT;

typedef struct
{

	h341ApplHandle h341hApp;
	tableT *  H225ConnectionsTable;
	tableT *  HTestTable;
	tableT *  ControlTable;
	tableT *  CapTable;
	tableT *  LCTable;	
	tableT *  LCH225Table;	
	h341InstEvReadWriteSetT  h341EvReadWriteSet;	
    h341InstEvChangeGlobalsT   h341ChangeGlobals;
    h341AgentComHandle   agentComHandle;   
	int ifIndex;

}h341InstanceHandleT;





typedef struct 
{
	h341RegisterHandleT h341AgentRegs;
	instanceRequestDataPtrT * iRequestsPtr;
	int reqNum;
}h341RegisterInfoT;

typedef struct
{
	h341RegisterInfoT  *info;/* array containing instance information*/
	mibNodeT *root;
	snmpObjectT subtrees[SUPPORT_SUBTREE_SIZE];
/*
	h341RetrieveNextIndexT h341RetrieveNextIndex;
	h341RetrieveByIndexT  h341RetrieveByIndex;
	h341RetrieveParameterT h341RetrieveParameter;
*/
	h341AgentEvSetParameterT	   h341SetParameter;	
	h341AgentEvMRRetrieveT		   h341MRRetrieve;	 
	int instNum;	    /* number of snmp instance connected to subAgent*/
	int activeConnections;
	RequestDataT * multReq; /* snmp agent requests  array allocated in initialization time*/
	int maxRequestNumber;  /* max size of snmp agent requests  array*/
    int maxInstanceNumber;
}h341AgentT;




tableT *  h225tableConstruct(int nElements);
int get(h341AgentHandleT hAgent,snmpObjectT *oid,INOUT void * buffer);
int set(h341AgentHandleT hSnmp,snmpObjectT *oid,INOUT void * buffer);
int getNext(h341AgentHandleT hAgent,INOUT void * buffer);

h341ErrorT createMib(h341AgentT * h341handle);
h341RegisterHandleT getSnmpInstance(IN h341AgentHandleT hSnmp,int ifIndex);
void destroyMib(mibNodeT * node);


#endif

#ifdef __cplusplus
}
#endif
