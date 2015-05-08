#ifdef __cplusplus
extern "C" {
#endif

#include <snmpI.h>
#include <mib.h>
#include <snmputil.h>






	tableT *  contolTableConstruct(int maxCalls);
	tableT *  capTableConstruct(int maxCalls);
	tableT *  lcTableConstruct(int maxChan);
	tableT *   lcH225TableConstruct(int maxChan);


snmpObjectT * h341AgentGetSubTrees(h341AgentHandleT hSnmp,int * size)
{
	if (hSnmp==NULL)
		return NULL;
	*size=SUPPORT_SUBTREE_SIZE;
	return ((h341AgentT *)hSnmp)->subtrees;
}



void h341SetInstanceNumber(h341InstanceHandle hSnmp,int index)
{
	((h341InstanceHandleT *)hSnmp)->ifIndex=index;

}





void h341InstEnd(h341InstanceHandle hInstHandle)
{
	h341InstanceHandleT * hSnmp = (h341InstanceHandleT *)hInstHandle;
	tableDestruct( (hSnmp)->H225ConnectionsTable);
	tableDestruct(hSnmp->ControlTable );
	tableDestruct(hSnmp->CapTable);
	tableDestruct(hSnmp->LCTable );
	tableDestruct(hSnmp->LCH225Table);
	free(hSnmp);
}


BOOL isContinue(int errorCode)
{
    if (
        (errorCode == lastRaw)||
        (errorCode == noSuchField)||
        (errorCode == noSuchIndex) ||
        (errorCode == checkNext))

        return TRUE;
    return FALSE;
}

/**********************************************************************
 * h341AgentGetNext
 * 
 *********************************************************************/
int h341AgentGetNext( h341AgentHandleT hAgent,snmpObjectT * oid, snmpObjectT * oidNext,
                       h341ErrorT * error,int variableBindingsLen)
{
	int ii,res,result,ifIndex;
	h341AgentT * hSnmp = (h341AgentT *)hAgent;
    if(hAgent==NULL)
        return RVERROR;

	/*
	 * Are there any registered instances? (applications)
	 */
    if (!hSnmp->instNum)
        return noRegisterInstanceRc;

	/*
	 * Check the number of requested parameters
	 */
    if (hSnmp->maxRequestNumber<variableBindingsLen)
		return noRequestNumber;
	for (ii=0;ii<variableBindingsLen;ii++)
	{
       hSnmp->multReq[ii].pResultOid = &oidNext[ii];
       hSnmp->multReq[ii].pSourceOid = &oid[ii];
       memcpy(&hSnmp->multReq[ii].instReqData.pData,&oidNext[ii].data,sizeof(mibDataT));

    }

    while(TRUE)
    {
	    for (ii=0;ii<variableBindingsLen;ii++)
	    {
			/*
			 * get next index to request
			 */
		    res = getNext(hAgent,(char *)&(hSnmp->multReq[ii]));
            if(!res)
            {
    			ifIndex = hSnmp->multReq[ii].ifIndex;
			    hSnmp->info[ifIndex-1].iRequestsPtr[hSnmp->info[ifIndex-1].reqNum]=&hSnmp->multReq[ii].instReqData;
			    hSnmp->info[ifIndex-1].reqNum++;
            }
            else
                hSnmp->multReq[ii].instReqData.result = res;
        }

		/*
		 * cycle through all possible (supported) instances
		 */
	    for (ii=0;ii<((h341AgentT*)hSnmp)->maxInstanceNumber;ii++)
	    {
		    if (hSnmp->info[ii].reqNum)
		    {
			    res = hSnmp->h341MRRetrieve(hSnmp->info[ii].h341AgentRegs,hSnmp->info[ii].iRequestsPtr,hSnmp->info[ii].reqNum);
                if (res == noSuchInstance)
                {
                    int jj;
                    for (jj=0;jj<hSnmp->info[ii].reqNum;jj++)
                        hSnmp->info[ii].iRequestsPtr[jj]->result=checkNext;

                }
			    hSnmp->info[ii].reqNum=0;
		    }
	    }
        result=0;
        for (ii=0;ii<variableBindingsLen;ii++)
	    {
            if (isContinue(hSnmp->multReq[ii].instReqData.result))
            {
               int length;
               length = hSnmp->multReq[ii].pResultOid->length;
               hSnmp->multReq[ii].pResultOid->id[length-1]++;/* increase if index*/
               hSnmp->multReq[ii].pSourceOid= hSnmp->multReq[ii].pResultOid;
               hSnmp->multReq[ii].instReqData.indexSize=0;
               /*replace not implemented OID to the same OID from the next ifIndex instance*/
                result = checkNext;
            }
        }
        if (result!=checkNext)
            break;

    }
    for (ii=0;ii<variableBindingsLen;ii++)
    {
        if (hSnmp->multReq[ii].instReqData.reqType==retrieveNextIndex)
        {
            int length,jj;
			int oidSize = hSnmp->multReq[ii].instReqData.nextIndexSize / sizeof(int);
			int *pOid = (int *)hSnmp->multReq[ii].instReqData.nextIndex;
            length = hSnmp->multReq[ii].pResultOid->length;
            for (jj=0; jj<oidSize; jj++)
            {
                hSnmp->multReq[ii].pResultOid->id[length+jj]=pOid[jj]; 
            } 
			
			/* increase oid length by the Size obtained from instance */ 
            hSnmp->multReq[ii].pResultOid->length += oidSize;
        }
        error[ii] = (h341ErrorT)hSnmp->multReq[ii].instReqData.result;
        oidNext[ii].data.valueSize = hSnmp->multReq[ii].instReqData.pData.valueSize;
        oidNext[ii].data.type = hSnmp->multReq[ii].instReqData.pData.type;
    }
    return 0;
}



h341ErrorT h341AgentSetValue(    IN h341AgentHandleT hAgent,
                            IN snmpObjectT * oid,
                            IN mibDataT *data)
{
    int res;
    res = set(hAgent,oid, (char *)data);
    return (h341ErrorT)res;
}



int h341AgentGetValue( h341AgentHandleT hAgent,snmpObjectT * oid,
                         OUT h341ErrorT  * error,int len)
{
	int ii,res=0,ifIndex;
	h341AgentT * hSnmp = (h341AgentT *)hAgent;
	if (hSnmp->maxRequestNumber<len)
		return noRequestNumber;
	for (ii=0;ii<len;ii++)
	{
		 res = get(hAgent,&oid[ii], (char *)&(hSnmp->multReq[ii]));
		 if (!res)
		 {
			ifIndex = hSnmp->multReq[ii].ifIndex;
			hSnmp->multReq[ii].pSourceOid = &oid[ii];
            memcpy(&hSnmp->multReq[ii].instReqData.pData ,&oid[ii].data,sizeof(mibDataT));
            if (ifIndex)
            {
			    hSnmp->info[ifIndex-1].iRequestsPtr[hSnmp->info[ifIndex-1].reqNum]=&hSnmp->multReq[ii].instReqData;
			    hSnmp->info[ifIndex-1].reqNum++;
            }
            else
                 hSnmp->multReq[ii].instReqData.result =0;/*for global parameters*/
		 }

	}
	for (ii=0;ii<((h341AgentT*)hSnmp)->maxInstanceNumber;ii++)
	{
		if (hSnmp->info[ii].reqNum)
		{
			res = hSnmp->h341MRRetrieve(hSnmp->info[ii].h341AgentRegs,hSnmp->info[ii].iRequestsPtr,hSnmp->info[ii].reqNum);
			hSnmp->info[ii].reqNum=0;

		}
	}
	for (ii=0;ii<len;ii++)
	{
        error[ii] =(h341ErrorT) hSnmp->multReq[ii].instReqData.result;
        oid[ii].data.type = hSnmp->multReq[ii].instReqData.pData.type;
        oid[ii].data.valueSize = hSnmp->multReq[ii].instReqData.pData.valueSize;
    }
	return res;
}







h341ErrorT h341InstInit(	IN	h341ApplHandle hApp,
						IN	int maxCalls,
						IN	int maxChannels,
						IN	h341InstEvReadWriteSetT  h341EvReadWriteSet,
						IN  h341InstEvChangeGlobalsT   h341ChangeGlobals,
                        OUT h341InstanceHandle * instHandle)
{



	h341InstanceHandleT * hSnmp = (h341InstanceHandleT *)malloc(sizeof(h341InstanceHandleT));
	if (hSnmp==NULL)
		return noMemoryRc;

	hSnmp->h341hApp=hApp;
	hSnmp->H225ConnectionsTable = h225tableConstruct(maxCalls);

	hSnmp->ControlTable = contolTableConstruct(maxCalls);
	hSnmp->CapTable = capTableConstruct(maxCalls*2);
	hSnmp->LCTable = lcTableConstruct(maxCalls*maxChannels);
	hSnmp->LCH225Table = lcH225TableConstruct(maxCalls*maxChannels);
    hSnmp->h341ChangeGlobals=NULL;
	hSnmp->h341EvReadWriteSet=h341EvReadWriteSet;
    hSnmp->h341ChangeGlobals = h341ChangeGlobals;
    hSnmp->agentComHandle=NULL;
    *instHandle = (h341InstanceHandle)hSnmp;
	return (h341ErrorT)0;
}


void    h341InstSetAgentComHandle (IN h341InstanceHandle h341hInst,
                               IN h341AgentComHandle   handle)
{
	h341InstanceHandleT * hSnmp = (h341InstanceHandleT *)h341hInst;
	if (hSnmp==NULL)
		return ;

    hSnmp->agentComHandle=handle;
}


h341ErrorT   h341AgentInit(int maxRequestNumber,
                               int maxInstanceNumber,
								h341AgentEvSetParameterT  h341SetParameter,
								h341AgentEvMRRetrieveT		h341MRRetrieve,
                                OUT h341AgentHandleT  * agentHandle)
{
	int ii;
    h341ErrorT res;
	h341AgentT *hSnmp;
	char *subtrees[SUPPORT_SUBTREE_SIZE]={(char*)"0.0.8.341.1.1.1",(char*)"0.0.8.341.1.1.2",
                                          (char*)"0.0.8.341.1.3.1.1",(char*)"0.0.8.341.1.3.1.2",
                                          (char*)"0.0.8.341.1.3.1.4"};

	hSnmp=(h341AgentT *)malloc(sizeof(h341AgentT));
    if (hSnmp==NULL)
        return noMemoryRc;
    hSnmp->info = (h341RegisterInfoT*)malloc(sizeof(h341RegisterInfoT)*maxInstanceNumber);
    if (hSnmp->info==NULL)
    {
        free(hSnmp);
        return noMemoryRc;
    }
    hSnmp->maxInstanceNumber = maxInstanceNumber;
	for(ii=0;ii<maxInstanceNumber;ii++)
	{
		hSnmp->info[ii].h341AgentRegs=NULL;
		hSnmp->info[ii].reqNum=0;
	}

	for (ii=0;ii<SUPPORT_SUBTREE_SIZE;ii++)
	{
		hSnmp->subtrees[ii].id = (int *)malloc(MAX_ID_SIZE * sizeof(int));
        if(hSnmp->subtrees[ii].id !=NULL)
		    hSnmp->subtrees[ii].length=str2oid(subtrees[ii],hSnmp->subtrees[ii].id);

	}
	hSnmp->multReq = (RequestDataT*)malloc(maxRequestNumber *sizeof(RequestDataT));
    if (hSnmp->multReq==NULL)
    {

	    for (ii=0;ii<SUPPORT_SUBTREE_SIZE;ii++)
	    {
		    if (hSnmp->subtrees[ii].id !=NULL)
                free(hSnmp->subtrees[ii].id);

	    }
        free(hSnmp->info);
        free(hSnmp);
        return noMemoryRc;

    }
	hSnmp->maxRequestNumber = maxRequestNumber;
	res = createMib(hSnmp);
    if (res == noMemoryRc)
    {
        free(hSnmp->multReq);
	    for (ii=0;ii<SUPPORT_SUBTREE_SIZE;ii++)
	    {
		    if (hSnmp->subtrees[ii].id !=NULL)
                free(hSnmp->subtrees[ii].id);

	    }
        free(hSnmp->info);
        free(hSnmp);
        return noMemoryRc;

    }


    dPrintMib(hSnmp->root,(char*)"0.0.8.341",2,1);
	hSnmp->h341SetParameter = h341SetParameter;
	hSnmp->h341MRRetrieve = h341MRRetrieve;
	hSnmp->instNum=0;
	hSnmp->activeConnections=0;
    * agentHandle = (h341AgentHandleT)hSnmp;
	return (h341ErrorT)0;
}




void h341AgentEnd(h341AgentHandleT hSnmp)
{
    int ii;

	for (ii=0;ii<SUPPORT_SUBTREE_SIZE;ii++)
	{
        if(((h341AgentT *)hSnmp)->subtrees[ii].id !=NULL)
            free(((h341AgentT *)hSnmp)->subtrees[ii].id );
	}
    free(((h341AgentT *)hSnmp)->info);
	free(((h341AgentT *)hSnmp)->multReq);
	destroyMib(((h341AgentT *)hSnmp)->root);
	free((h341AgentT *)hSnmp);

}


int h341AgentRegister(h341AgentHandleT hAgent,h341RegisterHandleT reg)
{
	int ii;
	h341AgentT * hSnmp= (h341AgentT *)hAgent;
	for(ii=0;ii<((h341AgentT*)hSnmp)->maxInstanceNumber;ii++)
		if (hSnmp->info[ii].h341AgentRegs==NULL)
		{
			hSnmp->info[ii].iRequestsPtr = (instanceRequestDataPtrT *)malloc(hSnmp->maxRequestNumber *sizeof(instanceRequestDataPtrT));
			if (hSnmp->info[ii].iRequestsPtr==NULL)
				return 0;
			hSnmp->info[ii].h341AgentRegs=reg;
			hSnmp->info[ii].reqNum=0;
			hSnmp->instNum++;
			return ii+1;
		}

	return 0;
}

void h341AgentUnRegister(h341AgentHandleT hAgent,int ifIndex)
{
	h341AgentT * hSnmp= (h341AgentT *)hAgent;

	if (ifIndex)
	{
		if (hSnmp->info[ifIndex-1].h341AgentRegs!=NULL)
		{
			hSnmp->info[ifIndex-1].h341AgentRegs=NULL;
			free(hSnmp->info[ifIndex-1].iRequestsPtr);
			hSnmp->instNum--;
		}
	}
}

h341ErrorT h341RetrieveGlParameter (h341AgentHandleT hAgent,h341ParameterName name,mibDataT * data)
{
	switch(name)
	{
	case connectionsActiveConnections:
		data->valueSize=((h341AgentT*)hAgent)->activeConnections;
        data->type = asnInt;
		return (h341ErrorT)0;
	default:
		return (h341ErrorT)noSuchField;
	}
}

void h341AgentChangeGlobals(h341AgentHandleT hAgent,h341ParameterName name,int offset)
{
	switch(name)
	{
	case connectionsActiveConnections:
		((h341AgentT*)hAgent)->activeConnections += offset;
		break;
	default:
		break;
	}

}


#ifdef __cplusplus
}
#endif
