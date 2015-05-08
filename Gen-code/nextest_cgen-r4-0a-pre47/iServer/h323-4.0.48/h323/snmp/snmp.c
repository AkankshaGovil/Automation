#ifdef __cplusplus
extern "C" {
#endif

/* snmp.cpp : Defines the entry point for the console application.*/

#include <stdio.h>
#include <mib.h>
#include <snmputil.h>

int mmRootId[]={0,0,8,341,1};

#define SIZEOF_OID_NODE sizeof(int)

extern tableT * cTable;




h341ErrorT h341RetrieveGlParameter (IN h341AgentHandleT hAgent,
                                    IN h341ParameterName name,
                                    OUT mibDataT *data);


tableT * getTable(h341InstanceHandle hSnmp,h341ParameterName name);


h341ErrorT h341InstGetMRParameter(h341InstanceHandle hSnmp,instanceRequestDataPtrT * requestsPtr,int reqNum)
{
	int ii;
	for (ii=0;ii<reqNum;ii++)
	{
		if (requestsPtr[ii]->reqType==retrieveByIndex)
		{
			requestsPtr[ii]->result=h341InstGetByIndex (hSnmp,requestsPtr[ii]->index,requestsPtr[ii]->indexSize,
				requestsPtr[ii]->name,&requestsPtr[ii]->pData);
		}
		else if (requestsPtr[ii]->reqType==retrieveParam)
		{
			requestsPtr[ii]->result=h341InstGetParameter(hSnmp, requestsPtr[ii]->name,
                &requestsPtr[ii]->pData);
		}
        else if (requestsPtr[ii]->reqType==retrieveNextIndex)
        {
            requestsPtr[ii]->result=h341InstGetNextIndex (hSnmp,requestsPtr[ii]->name,requestsPtr[ii]->index,
                requestsPtr[ii]->indexSize,requestsPtr[ii]->nextIndex,&requestsPtr[ii]->nextIndexSize,
                &requestsPtr[ii]->pData);			
        }
	}
	return (h341ErrorT)0;
}



h341ErrorT h341InstGetNextIndex(h341InstanceHandle hSnmp,h341ParameterName name,UINT8 * index, int indexSize,UINT8 *nextIndex,int *nextSize,
				  mibDataT *data)
{
		tableT *		table= getTable(hSnmp,name);
		if (table==NULL)
			return (h341ErrorT)noSuchField;
		return (h341ErrorT)getNextIndex (table,name,index,indexSize,nextIndex,nextSize,data);			
}


h341ErrorT h341InstGetByIndex (h341InstanceHandle hSnmp ,UINT8 * index, int indexSize, 
				h341ParameterName name,mibDataT *data)
{
	tableT *			table= getTable(hSnmp,name);
				if (table==NULL)
					return (h341ErrorT)noSuchField;
				return (h341ErrorT)getByIndex (table,index,indexSize,name,data);			

}






int checkIfIndex(IN h341AgentHandleT hSnmp ,int *ifIndex)
{
	int ii;
	if(*ifIndex)
	{
		ii = *ifIndex -1;
		if(((h341AgentT*)hSnmp)->info[ii].h341AgentRegs)
			return 0;
		

	}
	else
		ii = 0;
	/* find next valid instance */
	for (;ii<((h341AgentT*)hSnmp)->maxInstanceNumber;ii++)
	{
			if (((h341AgentT*)hSnmp)->info[ii].h341AgentRegs)
			{
				*ifIndex=ii+1;
				return nextIfIndex;
			}
	}
	/* find the first instance */
	for (ii=0;ii<((h341AgentT*)hSnmp)->maxInstanceNumber;ii++)
	{
			if (((h341AgentT*)hSnmp)->info[ii].h341AgentRegs)
			{
				*ifIndex=ii+1;
				break;
			}
	}
		
	return invalidIfIndex;
}



tableT * getTable(h341InstanceHandle hSnmp,h341ParameterName name)
{
	h341InstanceHandleT * hSnmpInst = (h341InstanceHandleT *)hSnmp;
	if ((name>=connectionsTableEntry)&&(name<=connectionsReleaseCompleteReason))	
		return ((h341InstanceHandleT *)hSnmpInst)->H225ConnectionsTable;
	if ((name>=h245ControlChannelMasterSlaveTable)&&(name<=h245ControlChannelIsTunneling))
		return ((h341InstanceHandleT *)hSnmpInst)->ControlTable;
	if ((name>=h245CapExchangeCapabilityTableEntry)&&(name<=h245CapExchangeCapabilityDescriptors))
		return ((h341InstanceHandleT *)hSnmpInst)->CapTable;
	if ((name>=h245LogChannelsChannelTableEntry)&&(name<=h245LogChannelsMediaTableType))
		return ((h341InstanceHandleT *)hSnmpInst)->LCTable;
	if ((name>=h245LogChannelsH225TableEntry)&&(name<=h245LogChannelsSrcTerminalLabel))
		return ((h341InstanceHandleT *)hSnmpInst)->LCH225Table;
	
	return NULL;

}



h341RegisterHandleT getSnmpInstance(IN h341AgentHandleT hSnmp,int ifIndex)
{
	h341RegisterHandleT h341AgentReg;	
	h341AgentReg = (((h341AgentT *)hSnmp)->info[ifIndex-1].h341AgentRegs);
	return h341AgentReg;
}



mibNodeT *
	getEventParameter(IN h341AgentHandleT hSnmp,enumPduTypeT pduType,snmpObjectT * oid,int *pOffset,IN mibNodeT * mibNode,
						  INOUT void * buffer,int *error)
{
	mibNodeT * node;
	int subIdNext,length;
    snmpObjectT * oidNext;
    RequestDataT * requestData;

    if (hSnmp);
    requestData = (RequestDataT * )buffer;

	oidNext = requestData->pResultOid;		
	node=mibNode;	
	length = *pOffset;
	*error=0;
	subIdNext = 10;
	if (pduType == pduGetNext)
	{
		
		while (node!=NULL)
		{
			if (subIdNext <= (int)node->childNum)
			{
				oidNext->id[length]=subIdNext;
				oidNext->length=length+1;
				*pOffset = length;					
				return node;
			}
			node = node->parent;		/*end of table	*/
			length--;
			subIdNext = oid->id[length]+1;
			*error=checkNext;

		}
		
	}
    *error = outOfMib;
	return node;
}


mibNodeT *
	getGlobalParameter(IN h341AgentHandleT hSnmp,enumPduTypeT pduType,snmpObjectT * oid,int *pOffset,IN mibNodeT * mibNode,
						  INOUT void * buffer,int *error)

{
	mibNodeT * node;
	int subIdNext,length;
    RequestDataT * requestData;

    snmpObjectT * oidNext;
    requestData = (RequestDataT * )buffer;

	oidNext = requestData->pResultOid;		

	node=mibNode;	
	length = *pOffset;
	*error=0;
	subIdNext=1;
	
	if (pduType == pduGetNext)
	{

		if (length==oidNext->length) 
		{
			oidNext->id[length]=0;
			oidNext->length++;
			*error =  h341RetrieveGlParameter (hSnmp,node->name,&oidNext->data);	
			return node;
		}

		*error=checkNext;
		while (node->parent!=NULL)
		{
			node = node->parent;		/*end of table	*/
			length--;
			subIdNext = oid->id[length]+1;

			if (subIdNext <= (int)node->childNum)
			{
				oidNext->id[length]=subIdNext;
				oidNext->length=length+1;
				*pOffset = length;			
				return node;
			}

		}
        *error = outOfMib;
		return NULL;
	}else
	{
        RequestDataT * requestData = (RequestDataT *)buffer;
		requestData ->ifIndex = 0;

		*error = noSuchField;
		if ((oid->length == length+1)&&(oid->id[length]==0))		
			*error =  h341RetrieveGlParameter (hSnmp,node->name,&oid->data);						
		return node;
		
	}
}






mibNodeT *
	getTableParameter(IN h341AgentHandleT hSnmp,enumPduTypeT pduType,snmpObjectT * oid,int *pOffset,IN mibNodeT * mibNode,
						  INOUT void * buffer,int *error)

{
	mibNodeT * node;
	int subIdNext,length,ifIndex=0,res;
	h341RegisterHandleT reg;
    RequestDataT * requestData;
    snmpObjectT * oidNext;
    requestData = (RequestDataT * )buffer;

	oidNext = requestData->pResultOid;		

	node=mibNode;	
	length = *pOffset;
	*error=0;
	
	if (pduType == pduGetNext)
	{

		if (length==oidNext->length) 
			subIdNext= (node->childNum!=0);
		else
		{
			if ((length+1) == oidNext->length)							
				subIdNext = oid->id[length];			
			else
			{
				subIdNext = oid->id[length];
				ifIndex=oid->id[length+1]+1;
			}
				
		}

		res = checkIfIndex(hSnmp,&ifIndex);
		if (res ==invalidIfIndex)		
			subIdNext++;
        if (subIdNext <= (int)node->childNum)
        {            
            oidNext->id[length] = subIdNext;				
            oidNext->length=length+2;
            oidNext->id[length+1]=ifIndex;					
            requestData->ifIndex = ifIndex;
            requestData->instReqData.name = (h341ParameterName)(node->name + subIdNext);
            requestData ->instReqData.indexSize=0;
            requestData ->instReqData.reqType = retrieveParam;
            *error = 0;
            return 0;
        }

		*error=checkNext;

		while (node->parent!=NULL)
		{
			node = node->parent;		/*end of table	*/
			length--;
			subIdNext = oid->id[length]+1;

			if (subIdNext <= (int)node->childNum)
			{
				oidNext->id[length]=subIdNext;
				oidNext->length=length+1;
				*pOffset = length;			
				return node;
			}

		}
		*error = outOfMib;
		return NULL;
    }
	else
    {
		*error = noSuchField;
		if (oid->length == length+2)
		{

			subIdNext = oid->id[length];
			ifIndex=oid->id[length+1];
			if (node->childNum >=subIdNext)
			{

				reg=getSnmpInstance( hSnmp,ifIndex);

				if (reg)
				{
					if (pduType==pduSetParam)
                    {
                        mibDataT * data = (mibDataT *)buffer;
						*error = ((h341AgentT *)hSnmp)->h341SetParameter (reg,(h341ParameterName)(node->name + subIdNext),data);
                    }
					else
					{						
						requestData ->ifIndex = ifIndex;
						requestData ->instReqData.name = (h341ParameterName)(node->name + subIdNext);
						requestData ->instReqData.indexSize=0;
						requestData ->instReqData.reqType = retrieveParam;
						*error = 0;
					}
				}

			}

		}
		return node;
		
	}
}




mibNodeT *
	getParameter(IN h341AgentHandleT hSnmp,enumPduTypeT pduType,snmpObjectT * oid,int *pOffset,IN mibNodeT * mibNode,
						  INOUT void * buffer,int *error)
{
	mibNodeT * node;
	int subIdNext,length;
    RequestDataT * requestData;

    snmpObjectT * oidNext;

    if (hSnmp);
    
    requestData = (RequestDataT * )buffer;

	oidNext = requestData->pResultOid;		

	node = mibNode;
	length=*pOffset;

	if (pduType == pduGetNext)
	{
		while(!node->table )
		{
			oidNext->id[length]=1;
			length++;
			node = &node->children[0];
		}
		*error=checkNext;
		*pOffset=length;
		oidNext->length=length;
		return node;
	}
	if (pduType == pduGetNextError)
	{
			node = node->parent;
			while(node!=NULL)
			{				
				subIdNext = oid->id[length-1]+1;		
				if (subIdNext <= (int)node->childNum)
				{
						oidNext->length=length;
						oidNext->id[length-1]=subIdNext;						
						*error=checkNext;
						*pOffset=length-1;
						return node;
				}
				node = node->parent;			
				length--;					
			}
            *error = outOfMib;
			return node;

	}
	return node;
}





int makeIndex(int offset ,snmpObjectT *oid, UINT8 * index,int *indexLength ,int *ifIndex);






mibNodeT *
	getIndexTableParameter(IN h341AgentHandleT hSnmp,enumPduTypeT pduType,snmpObjectT * oid,int *pOffset,IN mibNodeT * mibNode,
						  INOUT void * buffer,int * error)
{

	h341ParameterName name ;
	mibNodeT * node;
	int subIdNext=1,length,indexSize,res;
	UINT8 index[100];
	int offset,ifIndex=0;
	h341RegisterHandleT reg;
    RequestDataT * requestData;
    snmpObjectT * oidNext;
    requestData = (RequestDataT * )buffer;

	oidNext = requestData->pResultOid;		

    
    offset = *pOffset;

	node=mibNode;	
	indexSize = 100;
	if (pduType == pduGetNext)
	{
		length = offset+1;
		if (offset < oid->length)					 		
			subIdNext = oid->id[offset]; /* ii is according to table node */
		
		makeIndex(offset,oid,index,&indexSize,&ifIndex);

		res = checkIfIndex(hSnmp,&ifIndex);
		if (res ==invalidIfIndex)
		{
			subIdNext++;
			indexSize=0;
		}
		if (res == nextIfIndex)
			indexSize=0;		

		if (subIdNext <= (int)node->childNum)
		{
					
            oidNext->id[length-1] = subIdNext;				
			oidNext->length=length;
            requestData->ifIndex = ifIndex;
            requestData->instReqData.name = (h341ParameterName)(node->name + subIdNext);
            requestData ->instReqData.indexSize=indexSize;
            memcpy(requestData ->instReqData.index,index,indexSize);
            requestData ->instReqData.reqType = retrieveNextIndex;
            *error = 0;
            return node;

		}	
		length--;
		while(node->parent!=NULL) /*end of table*/
		{
			length--;/* ii is accord to  node*/
			node = node->parent;			
			subIdNext = oid->id[length]+1;
			if (subIdNext<= (int)node->childNum)
			{
					oidNext->length=length+1;
					oidNext->id[length]=subIdNext;
					*error = checkNext;
					*pOffset = length;
					return node;

			}
		}
		*error = outOfMib;
		return NULL;
	}else
	{
		name = (h341ParameterName)(node->name + oid->id[offset]);
		*error = noSuchField;
		if( !makeIndex(offset,oid,index,&indexSize,&ifIndex))
		{
			reg=getSnmpInstance( hSnmp,ifIndex);
			if (reg)
				if (pduType == pduGetIfIndex)
				{
						RequestDataT * requestData = (RequestDataT *)buffer;
						requestData ->ifIndex = ifIndex;
						requestData ->instReqData.name = name;
						requestData ->instReqData.indexSize=indexSize;
						memcpy(requestData ->instReqData.index,index,30);
						requestData ->instReqData.result = 0;
						requestData ->instReqData.reqType = retrieveByIndex;
						*error = 0;

				}
		}
		return NULL;
	}
}



int makeIndex(int pOffset ,snmpObjectT *oid, UINT8 * index,int *indexLength,int *ifIndex )
{
	int res=0;
	int i;
	int offset;	
	UINT8 *ptr=index;
	UINT8 *pid;
	offset = pOffset;

	if ( offset>=oid->length-1)
	{
		*ifIndex = 0;
		*indexLength = 0;
		return noSuchIndex;

	}
	offset++;
	*ifIndex=oid->id[offset];
	*indexLength = (oid->length-offset) * SIZEOF_OID_NODE;

	pid = (UINT8 *)&oid->id[offset];
	for(i=0; i<(*indexLength); i++)
	{
		ptr[i] = pid[i];
	}

#ifndef __nodebug__
/*	dPrintIndex(index,*indexLength,"index = ");*/
#endif
	return res;
}




int checkRoot(snmpObjectT * oid)
{
	int ii;
	if (oid->length < (sizeof(mmRootId)/sizeof(int)) )
		return FALSE;
	for (ii=0;ii<(sizeof(mmRootId)/sizeof(int));ii++)
	{
		if(mmRootId[ii] != oid->id[ii])
			return FALSE;
	}
	return TRUE;
}

/*
int getNext(h341AgentHandleT hSnmp,snmpObjectT *oid,INOUT void * buffer)
{
	mibNodeT * node;
	mibNodeT * children;
	snmpObjectT *pOid;
	int subId,ii ,error;
	if (!checkRoot(oid))
		return noSuchRoot;
	ii = (sizeof(mmRootId)/sizeof(int));
	node= ((h341AgentT *)hSnmp)->root;
	if (!((h341AgentT *)hSnmp)->instNum)
		return noRegisterInstance;
	pOid = (snmpObjectT*)buffer;
	
	memcpy(pOid->id,oid->id,oid->length*sizeof(int));
	pOid->length = oid->length;
	error = checkNext+1;	
	while ( (error>=checkNext) && (node!=NULL))
	{
		if ((node->table)||(ii==pOid->length))			
			node = node->getH323Parameter(hSnmp,pduGetNext,pOid,&ii,node,buffer,&error );
		else 
		{
			subId = pOid->id[ii];			
			if ((subId == 0)||((int)node->childNum < subId)||(node->childNum==0))										
				node= node->getH323Parameter(hSnmp,pduGetNextError,pOid,&ii,node,buffer,&error);
			
			else
			{
				children  = node->children;
				node = &children[subId-1];
			}
		}
		if (error == checkNext)
			error = checkNext+1;
		else			
			ii++;			
	}
	return error;
}
*/

int getNext(h341AgentHandleT hSnmp,INOUT void * buffer)
{
	mibNodeT * node;
	mibNodeT * children;
	snmpObjectT *pOid;
    RequestDataT * requestData;
	int subId,ii ,error;
    requestData = (RequestDataT *)buffer;
    pOid = requestData->pResultOid;

	if (!checkRoot(requestData->pSourceOid))
		return noSuchRoot;
	ii = (sizeof(mmRootId)/sizeof(int));
	node= ((h341AgentT *)hSnmp)->root;
	if (!((h341AgentT *)hSnmp)->instNum)
		return noRegisterInstance;
    
    memcpy(pOid->id,requestData->pSourceOid->id,requestData->pSourceOid->length*sizeof(int));
    pOid->length = requestData->pSourceOid->length;
    error = outOfMib;	
    while ( (error>=checkNext) && (node!=NULL))
    {
        if ((node->table)||(ii==pOid->length))			
            node = node->getH323Parameter(hSnmp,pduGetNext,pOid,&ii,node,buffer,&error );
        else 
        {
            subId = pOid->id[ii];			
            if ((subId == 0)||((int)node->childNum < subId)||(node->childNum==0))										
                node= node->getH323Parameter(hSnmp,pduGetNextError,pOid,&ii,node,buffer,&error);
            
            else
            {
                children  = node->children;
                node = &children[subId-1];
            }
        }
        if (error == checkNext)
            error = outOfMib;
        else			
            ii++;			
    }
    return error;
    
}









int get(h341AgentHandleT hSnmp,snmpObjectT *oid,INOUT void * buffer)
{
	mibNodeT * node;
	mibNodeT * children;
	int subId,ii,error;
	if (!checkRoot(oid))
		return noSuchRoot;
	ii = (sizeof(mmRootId)/sizeof(int));
	node= ((h341AgentT *)hSnmp)->root;
	if (!((h341AgentT *)hSnmp)->instNum)
		return noRegisterInstance;

	for (;ii<oid->length;ii++)
	{
		subId = oid->id[ii];
		if ((int)node->childNum < subId)
			return noSuchField;

		if (node->table) 
		{
			if (node->getH323Parameter!=NULL)
			{
				node->getH323Parameter(hSnmp,pduGetIfIndex,oid,&ii,node,buffer,&error );			
				return error;
			}

		}
		if(node->childNum==0)
			return noSuchField;
		children  = node->children;
		node = &children[subId-1];
	}
	if (node->getH323Parameter!=NULL)	
	{
		
		node->getH323Parameter(hSnmp,pduGetIfIndex,oid,&ii,node,buffer,&error );
		return error;
	}
	
	return 0;
}


int set(h341AgentHandleT hSnmp,snmpObjectT *oid,INOUT void * buffer)
{
	mibNodeT * node;
	mibNodeT * children;
	int subId,ii,error;
	if (!checkRoot(oid))
		return noSuchRoot;
	ii = (sizeof(mmRootId)/sizeof(int));
	node= ((h341AgentT *)hSnmp)->root;
	if (!((h341AgentT *)hSnmp)->instNum)
		return noRegisterInstance;

	for (;ii<oid->length;ii++)
	{
		subId = oid->id[ii];
		if ((int)node->childNum < subId)
			return noSuchField;

		if (node->table) 
		{
			if (node->getH323Parameter!=NULL)
			{
				node->getH323Parameter(hSnmp,pduSetParam,oid,&ii,node,buffer,&error );			
				return error;
			}

		}
		if(node->childNum==0)
			return noSuchField;
		children  = node->children;
		node = &children[subId-1];
	}
	if (node->getH323Parameter!=NULL)	
	{
		
		node->getH323Parameter(hSnmp,pduSetParam,oid,&ii,node,buffer,&error );
		return error;
	}
	
	return 0;
}






mibNodeT *   mibCreateNode (mibNodeT * parent,int number,int childNumber,h341ParameterName name,
							getH323ParameterT 	getH323Parameter,BOOL table)
{	
	mibNodeT *node;



	if((int)parent->childNum<number)
		return NULL;
	node = &parent->children[number-1];
	node->name=name;
	node->parent=parent;
	node->table=table;
	node->childNum=(UINT8 )childNumber;
	if (!table)
	{
		node->children = (mibNodeT *)malloc(childNumber * sizeof(mibNodeT));
		memset(node->children,0,childNumber * sizeof(mibNodeT));
	}
	node->getH323Parameter=getH323Parameter;
	return node;
}

mibNodeT *   mibCreateNodeL (mibNodeT * parent,int number,int childNumber,h341ParameterName name,
							getH323ParameterT 	getH323Parameter)
{	
	mibNodeT *node;
	if((int)parent->childNum<number)
		return NULL;
	node = &parent->children[number-1];
	node->name=name;
	
	node->childNum=(UINT8 )childNumber;
	node->children = (mibNodeT *)malloc(childNumber * sizeof(mibNodeT));
	memset(node->children,0,childNumber * sizeof(mibNodeT));
	node->getH323Parameter=getH323Parameter;
	return node;
}


int translate(void)
{
	FILE * fp,*fp2;
	char sIn[200],sOut[200],*ptr,*b;
	fp = fopen("text.cpp","rb");
	fp2 = fopen("text1.cpp","w+b");
	if(fp==NULL)
		return -1;
	while(fgets(sIn,100,fp)!=NULL)
	{

		ptr = strchr(sIn,',');
		if(ptr!=NULL)
			*ptr=0;
		b=sIn;
		while((*b==' ')||(*b=='\t'))
			b++;
		if ((*b!=0)&&(*b!='\r') && (*b!='\n') )
		{
			sprintf(sOut," case %s:\n \treturn \"%s\";\n",b,b);
			fwrite(sOut,strlen(sOut),1,fp2);
		}
	}
	fclose(fp);
	fclose(fp2);
	return 0;

}

int createH320Mib(h341AgentT * hSnmp,mibNodeT * parent)
{
	mibNodeT *node;
    if (hSnmp);
	node = mibCreateNode(parent, 2,0, mmH320Root,getEventParameter,FALSE);
	return 0;

}

int createH225Mib(h341AgentT * hSnmp,mibNodeT * parent)
{
	mibNodeT * node,*h225root,*hRASroot;

    if (hSnmp);
    
	node = mibCreateNode(parent, 1,2, mmH323Root,getParameter,FALSE);

	h225root = mibCreateNode(node, 1,4, h225callSignaling,getParameter,FALSE);
	hRASroot = mibCreateNode(node, 2,1, ras,getParameter,FALSE);

	node = mibCreateNode(h225root, 1,1, callSignalConfig,getParameter,FALSE);
	node = mibCreateNode(node, 1,1, callSignalConfigTable,getParameter,FALSE);
	node = mibCreateNode(node, 1,5, callSignalConfigEntry,getTableParameter,TRUE);

	node = mibCreateNode(h225root, 2,1, callSignalStats,getParameter,FALSE);
	node = mibCreateNode(node, 1,1, callSignalStatsTable,getParameter,FALSE);
	node = mibCreateNode(node, 1,25, callSignalStatsEntry,getTableParameter,TRUE);

	node = mibCreateNode(h225root, 3,2, connections,getParameter,FALSE);
	mibCreateNode(node, 1,0, connectionsActiveConnections,getGlobalParameter,TRUE);
	node = 	 mibCreateNode(node, 2,1, connectionsTable,getParameter,FALSE);
	node = 	 mibCreateNode(node, 1,32, connectionsTableEntry,getIndexTableParameter,TRUE);

	node = mibCreateNode(h225root, 4,0, callSignalEvents,getEventParameter,TRUE);

	node = mibCreateNode(hRASroot, 1,1,rasConfiguration,getParameter,FALSE);
	node = mibCreateNode(node, 1,1,rasConfigurationTable,getParameter,FALSE);
	node = mibCreateNode(node, 1,5,rasConfigurationTableEntry,getTableParameter,TRUE);


	return 0;
}


int	createH245Mib(h341AgentT * hSnmp,mibNodeT * parent)
{
	
	mibNodeT * id0;
	mibNodeT * id1;
	mibNodeT * id2;
	mibNodeT * id3;
	mibNodeT * id4;
	mibNodeT * id8;
	mibNodeT * id20;
	mibNodeT * id37;
	mibNodeT * id48;
	mibNodeT * id58;
	mibNodeT * id67;
	mibNodeT * id74;
	mibNodeT * id91;
	mibNodeT * id111;
	mibNodeT * node;
	mibNodeT *h245root;

    if (hSnmp);
    
	node = mibCreateNode(parent, 3,1, mmH245Root,getParameter,FALSE);	
			
	id0 = mibCreateNode(node, 1,/*6*/4, h245,getParameter,FALSE);
	h245root = id0;
	
	id1 = mibCreateNode(id0, 1,1, h245Configuration,getParameter,FALSE);
	id8 = mibCreateNode(id1, 1,1, h245ConfigurationTable,getParameter,FALSE);
		mibCreateNode(id8, 1,10, h245ConfigurationTableEntry,getTableParameter,TRUE);
	id2 = mibCreateNode(id0, 2,2, h245ControlChannel,getParameter,FALSE);
	id20 = mibCreateNode(id2, 1,1, h245ControlChannelStatsTable,getParameter,FALSE);
		mibCreateNode(id20, 1,15, h245ControlChannelStatsTableEntry,getTableParameter,TRUE);
	id37 = mibCreateNode(id2, 2,1, h245ControlChannelMasterSlaveTable,getParameter,FALSE);
		mibCreateNode(id37, 1,9, h245ControlChannelMasterSlaveTableEntry,getIndexTableParameter,TRUE);
	id3 = mibCreateNode(id0, 3,2, h245CapExchange,getParameter,FALSE);
	id48 = mibCreateNode(id3, 1,1, h245CapExchangeStatsTable,getParameter,FALSE);
		mibCreateNode(id48, 1,8, h245CapExchangeStatsTableEntry,getTableParameter,TRUE);
	id58 = mibCreateNode(id3, 2,1, h245CapExchangeCapabilityTable,getParameter,FALSE);
		mibCreateNode(id58, 1,7, h245CapExchangeCapabilityTableEntry,getIndexTableParameter,TRUE);
	id4 = mibCreateNode(id0, 4,4, h245LogChannels,getParameter,FALSE);
	id67 = mibCreateNode(id4, 1,1, h245LogChannelsChannelTable,getParameter,FALSE);
		mibCreateNode(id67, 1,5, h245LogChannelsChannelTableEntry,getIndexTableParameter,TRUE);
	id74 = mibCreateNode(id4, 2,1, h245LogChannelsH225Table,getParameter,FALSE);
		mibCreateNode(id74, 1,15, h245LogChannelsH225TableEntry,getIndexTableParameter,TRUE);
	id91 = mibCreateNode(id4, 3,1, h245LogChannelOpenLogicalChannelTable,getParameter,FALSE);
		mibCreateNode(id91, 1,18, h245LogChannelOpenLogicalChannelTableEntry,getTableParameter,TRUE);
	id111 = mibCreateNode(id4, 4,1, h245LogChannelCloseLogicalChannelTable,getParameter,FALSE);
		mibCreateNode(id111, 1,6, h245LogChannelCloseLogicalChannelTableEntry,getTableParameter,TRUE);

/*
    conference and misk are not implemented, number of h245 subtree is 4 (for creating id0) 
	id5 = mibCreateNode(id0, 5,2, h245Conference,getParameter,FALSE);
	id138 = mibCreateNode(id5, 1,1, h245ConferenceTerminalTable,getParameter,FALSE);
		mibCreateNode(id138, 1,6, h245ConferenceTerminalTableEntry,getIndexTableParameter,TRUE);
	id146 = mibCreateNode(id5, 2,1, h245ConferenceStatsTable,getParameter,FALSE);
		mibCreateNode(id146, 1,15, h245ConferenceStatsTableEntry,getTableParameter,TRUE);
	id6 = mibCreateNode(id0, 6,2, h245Misc,getParameter,FALSE);
	id119 = mibCreateNode(id6, 1,1, h245MiscRoundTripDelayTable,getParameter,FALSE);
		mibCreateNode(id119, 1,6, h245MiscRoundTripDelayTableEntry,getIndexTableParameter,TRUE);
	id127 = mibCreateNode(id6, 2,1, h245MiscMaintenanceLoopTable,getParameter,FALSE);
		mibCreateNode(id127, 1,9, h245MiscMaintenanceLoopTableEntry,getIndexTableParameter,TRUE);
*/ 	
	return 0;
}


h341ErrorT	createMib(h341AgentT* hSnmp)
{
	mibNodeT 	parent;

	parent.childNum=(UINT8)1;
	
	parent.children =  (mibNodeT *) malloc( sizeof(mibNodeT));
	parent.parent=NULL;

	hSnmp->root =  mibCreateNode(&parent, 1,3, mmRoot,getParameter,FALSE);
    hSnmp->root->parent=NULL;
	createH225Mib(hSnmp,hSnmp->root);
	createH245Mib(hSnmp,hSnmp->root);
	createH320Mib(hSnmp,hSnmp->root);
	return (h341ErrorT)0;
}

void destroyMib(mibNodeT * node)
{
	int ii;
	if (node !=NULL)
	{

		if (!node->table)			
		{
			for (ii=0;ii<node->childNum;ii++)			
				destroyMib( &(node->children[ii]));
		}	
		free(node);
		
	}


}


#ifdef CONSOLE

void testtable();

int main(int argc, char* argv[])
{


	mibNodeT	parent;
	fpDebug = fopen("dd.c","wb");

	parent.childNum=(UINT8)1;
	
	parent.children =  (mibNodeT *) malloc( sizeof(mibNodeT));
	parent.parent=NULL;

	hSnmp.root =  mibCreateNode(&parent, 1,3, mmRoot,getParameter,FALSE);
	createH225Mib(&hSnmp,hSnmp.root);
	createH245Mib(&hSnmp,hSnmp.root);
	createH320Mib(&hSnmp,hSnmp.root);

	dPrintMib(hSnmp.root,"0.0.341",2,1);
	testtable();

				{
					 int bufferSize = 1024;
					 snmpObjectT oidNext;
					 int idNext[30];
					 snmpObjectT oid;

					 int id[] = {0,0,8,341,1,1,1,1,1,2};
					 oidNext.id = idNext;
					 oidNext.length=30;
					 oid.id = id;
					 oid.length = sizeof(id)/sizeof(int);
					getNext(&hSnmp,&oid,(char *)&oidNext,&bufferSize);
					dPrintOid(&oidNext,"next oid = ");


				}

		
				{
					 int bufferSize = 1024;
					 snmpObjectT oidNext;
					 int idNext[30];
					 snmpObjectT oid;

					 int id[] = {0,0,8,341,1,1,1,3};
					 oidNext.id = idNext;
					 oidNext.length=30;
					 oid.id = id;
					 oid.length = sizeof(id)/sizeof(int);
					getNext(&hSnmp,&oid,(char *)&oidNext,&bufferSize);
					dPrintOid(&oidNext,"next oid = ");

				}

				 {
					 int bufferSize = 1024;
					 snmpObjectT oidNext;
					 snmpObjectT oid;
					 snmpObjectT pOid;		
					 int pId[30];
					 	 	
					 int idNext[30];
					 int id[] = {0,0,8,341,1};
					 pOid.id=pId;
					 oidNext.id = idNext;
					 oidNext.length=30;
					 oid.id = id;
					 oid.length = sizeof(id)/sizeof(int);


					memcpy(pOid.id,id,sizeof(id));
					pOid.length = oid.length;
					while (!getNext(&hSnmp,&pOid,(char *)&oidNext,&bufferSize))
					{
						dPrintOid(&oidNext,"next oid = ");
						memcpy(pOid.id,oidNext.id,30*sizeof(int));
						pOid.length = oidNext.length;
						oidNext.length = 30;
						
					}
				 }





	fclose(fpDebug);
	return 0;
}








#endif

#ifdef __cplusplus
}
#endif
