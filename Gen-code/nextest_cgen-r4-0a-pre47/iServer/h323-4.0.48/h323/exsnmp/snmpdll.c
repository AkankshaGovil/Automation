#include <windows.h>
#include <snmp.h>
#include <process.h>
#include <stdio.h>

#include <rvh341agent.h>
#define     MM_VIEW_OFFSET       30
#define     WM_REGISTER_SUBAG    WM_USER+101
#define     WM_UNREGISTER_SUBAG    WM_USER+102


#define		WM_RETRIEVE_NEXT_INDEX   WM_USER+103
#define		WM_RETRIEVE_BY_INDEX   WM_USER+104
#define		WM_RETRIEVE				WM_USER+108
#define     WM_CHANGE_GLOBAL		WM_USER+106
#define     WM_SET_PARAMETER		WM_USER+107
#define		WM_RETRIEVE_MR			WM_USER+105

#define		noSuchWindow  50
#define     waitTimeout       50
#define     INSTANCE_NUMBER 5

#define     RequestNumber   40
typedef HANDLE h341SubAgentHandleT;
typedef DWORD   h341SubAgentIdT;


typedef struct
{
    h341SubAgentIdT h341SubAgentId; /* treadId of snmp instance thread*/
    h341SubAgentHandleT h341SubAgentHandle;       /* handle of event*/
    int                 h341SubAgentIndex;
}h341AgentRegT;
h341AgentRegT instReg[INSTANCE_NUMBER];



HANDLE mmhFile,hFile,hEvent,hEventWait;
LPVOID lpMapAddress;

/*
    winData2mibData transtates Windows data format mibDataT  format
*/
void   winData2mibData(AsnObjectSyntax *value,mibDataT *data)
{
    switch(value->asnType)
    {
    case ASN_INTEGER32:
        data->type=asnInt;
        data->valueSize = value->asnValue.number;
        break;
/*

    case  ASN_UNSIGNED32:
    case ASN_COUNTER64:
    case ASN_COUNTER32:
    case ASN_GAUGE32:
*/
    case ASN_OCTETSTRING:

        data->type=asnMibOctetString;
        data->value = value->asnValue.string.stream;
        data->valueSize = value->asnValue.string.length;
        break;
    case ASN_IPADDRESS:
        data->type=asnAddressString;
        data->value = value->asnValue.address.stream;
        data->valueSize = value->asnValue.address.length;
        break;
    default:
    break;
    }
}

/*
    Imitate Windows SendMessage function
*/
int snmpSendMessage(   HANDLE hHandle,
                    UINT uMsg,
                    WPARAM wParam,
                    LPARAM lParam)
{
    char *ptr;
    int res;
    ptr = lpMapAddress;

    ptr+=4;
    memcpy(ptr,(char *)&uMsg,sizeof(UINT));
    ptr+=sizeof(UINT);
    memcpy(ptr,(char *)&wParam,sizeof(WPARAM));
    ptr+=sizeof(WPARAM);
    memcpy(ptr,(char *)&lParam,sizeof(LPARAM));
    ptr+=sizeof(LPARAM);
    SetEvent(hHandle);

    res = WaitForSingleObject(hEventWait,INFINITE);
    if (res == WAIT_TIMEOUT)
        return waitTimeout;
    else
        return 0;

}

/*
    Callback function is called  to pass get and get Next
    request and corresponding data to instance.
*/

int h341MRRetrieve(h341RegisterHandleT regH,instanceRequestDataPtrT * instRequestsPtr,int reqNum)
{
    int ii;
    h341AgentRegT * reg = (h341AgentRegT *)regH;
    char * ptr;


    ptr = lpMapAddress;
    ptr+=MM_VIEW_OFFSET;

    memcpy(ptr,(char *)&reqNum,4);
    ptr = ptr+4;

    for (ii=0;ii<reqNum;ii++)
    {
        memcpy(ptr,(char *)&instRequestsPtr[ii]->reqType,4);
        ptr=ptr+4;
        memcpy(ptr,(char *)&instRequestsPtr[ii]->name,4);
        ptr=ptr+4;
        memcpy(ptr,(char *)&instRequestsPtr[ii]->indexSize,4);
        ptr=ptr+4;
        if (instRequestsPtr[ii]->indexSize)
        {
            memcpy(ptr,instRequestsPtr[ii]->index,instRequestsPtr[ii]->indexSize);
            ptr+=instRequestsPtr[ii]->indexSize;
        }

    }
    if (snmpSendMessage(   (HANDLE)reg->h341SubAgentHandle,WM_RETRIEVE_MR,0,0))
    {

        return waitTimeout;
    }

    ptr = lpMapAddress;
    ptr+=4;

    for (ii=0;ii<reqNum;ii++)
    {
        memcpy((char *)&instRequestsPtr[ii]->result,ptr,4);
        ptr+=4;

        if (!instRequestsPtr[ii]->result)
        {

            if (instRequestsPtr[ii]->reqType==retrieveNextIndex)
            {
                memcpy((char *)&instRequestsPtr[ii]->nextIndexSize,ptr,4);
                ptr+=4;
                memcpy(instRequestsPtr[ii]->nextIndex,ptr,instRequestsPtr[ii]->nextIndexSize);
                ptr+=instRequestsPtr[ii]->nextIndexSize;

            }

            memcpy((char *)&instRequestsPtr[ii]->pData.type,ptr,4);
            ptr+=4;
            if (instRequestsPtr[ii]->pData.type!=asnError)
            {
                memcpy((char *)&instRequestsPtr[ii]->pData.valueSize,ptr,4);
                ptr+=4;
                if((instRequestsPtr[ii]->pData.type==asnMibOctetString)||
                    (instRequestsPtr[ii]->pData.type==asnAddressString))
                {
                    memcpy(instRequestsPtr[ii]->pData.value,ptr,instRequestsPtr[ii]->pData.valueSize);
                    ptr+=instRequestsPtr[ii]->pData.valueSize;
                }
            }
        }
    }


    return 0;
}



/*
    Callback function is called  to pass set request and corresponding data to
    instance.
*/
h341ErrorT hAppl341SetParameter(h341RegisterHandleT regH,h341ParameterName name,mibDataT * data)
{
    h341AgentRegT * reg = (h341AgentRegT *)regH;
    char *ptr;
    h341ErrorT res;

    ptr = lpMapAddress;
    ptr+=MM_VIEW_OFFSET;
    memcpy(ptr,(char *)&name,4);
    ptr = ptr+4;
    memcpy(ptr,(char *)&data->type,4);
    ptr=ptr+4;
    memcpy(ptr,(char *)&data->valueSize,4);
    ptr=ptr+4;
    if ((data->type == asnMibOctetString)||(data->type == asnAddressString))
        memcpy(ptr,data->value,data->valueSize);

    if (snmpSendMessage(   (HANDLE)reg->h341SubAgentHandle,WM_SET_PARAMETER,0,0))
        return waitTimeout;
    ptr = lpMapAddress;
    ptr+=MM_VIEW_OFFSET;
    memcpy((char *)&res,ptr,4);

    return res;
}



LRESULT RVCALLCONV
h341AgentWnd(HWND hWnd,UINT uMsg,WPARAM wParam,LPARAM lParam);

void WINAPI SnmpExtensionClose();



HINSTANCE hinst;
int count;
h341AgentHandleT hAgent;

BOOL WINAPI DLLMain(
                    HINSTANCE hinstDLL,
                    DWORD  dwReason,
                    LPVOID lpReserved)
{
    hinst = hinstDLL;
    switch(dwReason)
    {
    case DLL_PROCESS_ATTACH:

        break;
    case DLL_PROCESS_DETACH:

        SnmpExtensionClose();

        break;
    case DLL_THREAD_ATTACH:
    case DLL_THREAD_DETACH:
    default:
        break;

    } // end switch()

    return TRUE;

}




/*
    SnmpExtensionInitEx is called by NT SNMP for retrieving all mib subtrees,
    supprted by present extension subagent.

*/
BOOL WINAPI SnmpExtensionInitEx(
                                AsnObjectIdentifier *pNextSupportedRegion  )
{
    snmpObjectT * subtrees;
    int size;
    subtrees = h341AgentGetSubTrees(hAgent,&size);
    if (count==size)
        return FALSE;
    pNextSupportedRegion->ids = subtrees[count].id;
    pNextSupportedRegion->idLength = subtrees[count].length;


    count++;
    return TRUE;

}

/*
    Thread function, waiting notifications from the instances.
*/
unsigned __stdcall theStackThread(void*v)
{
    int res;
    MSG msg;
    DWORD threadId;
    threadId = GetCurrentThreadId();

    memcpy(lpMapAddress ,(char *)&threadId,sizeof(DWORD));
    while(GetMessage(&msg,NULL,0,0))
    {
        if(msg.hwnd==NULL)
        {
            res=h341AgentWnd(NULL,msg.message,msg.wParam,msg.lParam);
            if(!res)
            {
                h341AgentWnd(NULL,WM_UNREGISTER_SUBAG,msg.wParam,msg.lParam);
            }
        }
        else
            DispatchMessage(&msg);
    }

return 0;
}

void WINAPI SnmpExtensionClose()
{
        UnmapViewOfFile(lpMapAddress);
        CloseHandle(mmhFile);
        CloseHandle(hFile);
        CloseHandle(hEventWait);
        h341AgentEnd(hAgent);

}
/*
SnmpExtensionInit is called by NT SNMP service during initialization.
Initialization of snmp subagent module and all handles,using for
syncronization and communication with subinstances is perfomed here.
*/
BOOL WINAPI  SnmpExtensionInit(
                               IN  DWORD               dwTimeZeroReference,
                               OUT HANDLE              *hPollForTrapEvent,
                               OUT AsnObjectIdentifier *supportedView)
{

    int size,res;
    int ii;
    snmpObjectT * subtrees;
    char szTmpFile[256];
    char szMMFile[256];
    HANDLE hFile;

    SECURITY_DESCRIPTOR SD;
    SECURITY_ATTRIBUTES sa;


    if (!InitializeSecurityDescriptor(&SD, SECURITY_DESCRIPTOR_REVISION))
        return FALSE;


    if (!SetSecurityDescriptorDacl(&SD, TRUE, (PACL) NULL, FALSE))
        return FALSE;

    sa.nLength = sizeof(sa);
    sa.lpSecurityDescriptor = &SD;
    sa.bInheritHandle = TRUE;



    count = 0;

    for (ii = 0;ii< INSTANCE_NUMBER;ii++)
        memset(&instReg[ii],0,sizeof(h341AgentRegT));
    /*
        Initialization of  subAgent module.
        hAppl341SetParameter,h341MRRetrieve - callback functions to communicate
        with instances.

    */
    res =  h341AgentInit(RequestNumber,INSTANCE_NUMBER,
                            hAppl341SetParameter,h341MRRetrieve,&hAgent);
    if(res)
        return FALSE;
    /*
        h341 mib subtrees, subAgent module supports.
    */
    subtrees = h341AgentGetSubTrees(hAgent,&size);
    supportedView->ids = subtrees[0].id;
    supportedView->idLength = subtrees[0].length;
    count++;


    *hPollForTrapEvent=NULL;


    /*
        Create mapped file to pass data between subagent and instance
    */
    GetTempPath (256, szTmpFile);
    GetTempFileName (szTmpFile,"PW",0,szMMFile);
    hFile =CreateFile (szMMFile,GENERIC_WRITE | GENERIC_READ,
        FILE_SHARE_WRITE | FILE_SHARE_READ,
        &sa,
        CREATE_ALWAYS,
        FILE_ATTRIBUTE_TEMPORARY,NULL);

    if(hFile==INVALID_HANDLE_VALUE)
        return FALSE;
    mmhFile = CreateFileMapping (hFile,
        &sa,
        PAGE_READWRITE,
        0,
        2048,
        "AgentInstComm");


    if (mmhFile==NULL)
        return FALSE;


    lpMapAddress = MapViewOfFile(mmhFile,FILE_MAP_ALL_ACCESS,0,0,0);




    if (lpMapAddress == NULL)
        return FALSE;
    /*
        Create event for waiting notifications from instances
        (registrations,global parameters changes)
    */
    hEventWait = CreateEvent(&sa,FALSE,FALSE,"AgentInstWait");
    if (hEventWait==NULL)
        return FALSE;
    _beginthreadex(NULL,0,theStackThread, NULL,0,NULL);

    return TRUE;

} // end SnmpExtensionInit()






LRESULT RVCALLCONV
h341AgentWnd(HWND hWnd,UINT uMsg,WPARAM wParam,LPARAM lParam)
{
    switch(uMsg)
    {
    case WM_REGISTER_SUBAG:  /* send message */
        {
            h341AgentRegT * reg;
            char s[20];
            int index,ii;

            for (ii = 0;ii< INSTANCE_NUMBER;ii++)
            {
                if ((instReg[ii].h341SubAgentId==0)||(instReg[ii].h341SubAgentId==(h341SubAgentIdT)lParam))
                {
                    reg = &instReg[ii];
                    /*instance builds unique name for waiting request event,using
                    thread id ,create event and sends thread id as lParam
                    to ex agent. Ex agent,using lParam, creates name , opens
                    corresponding event, and stores event handle and registered
                    instance thread id for following communication with instance
                    */
                    instReg[ii].h341SubAgentId = lParam;
                    sprintf(s,"AgentNtf%d",lParam);
                    reg->h341SubAgentHandle = (h341SubAgentHandleT)OpenEvent(EVENT_ALL_ACCESS,FALSE,s);
					/* get an index - instance number */
                    index= h341AgentRegister( hAgent,(h341RegisterHandleT)reg );
                    reg->h341SubAgentIndex = index;
	               /* pass index (instance number) to instance.
					The index is placed in the second 4 bytes of the mapped file  
				    */
					*(int *)((char *)lpMapAddress + 4)= index;
					SetEvent(reg->h341SubAgentHandle);

                    return index;
                }
            }
            return 0;

        }
    case WM_UNREGISTER_SUBAG:
        {
            char s[20];
            int ii;
            HANDLE unregEv;
            for (ii = 0;ii< INSTANCE_NUMBER;ii++)
            {
                if ((LPARAM)instReg[ii].h341SubAgentId==lParam)
                {
                    /*
                        close handle to waiting request event of
                        unregistered instance
                    */
                    instReg[ii].h341SubAgentId = 0;
                    sprintf(s,"AgentNtf%d",lParam);
                    CloseHandle((HANDLE)instReg[ii].h341SubAgentHandle);
                    if (instReg[ii].h341SubAgentIndex)
                        h341AgentUnRegister( hAgent,(int)instReg[ii].h341SubAgentIndex );
                    instReg[ii].h341SubAgentIndex=0;
                }
            }

            /*
                    Ex agent,using lParam, creates name for instance wating unregistration
                    notification event, opens  corresponding event and notify
                    instance about unregistration.
            */

            sprintf(s,"unreg%d",lParam);
            unregEv = OpenEvent(EVENT_ALL_ACCESS,FALSE,s);
            SetEvent(unregEv);
            CloseHandle(unregEv);

        }

        break;
    case WM_CHANGE_GLOBAL:
        /*
            Get notification that global parameter was changed.
        */
        h341AgentChangeGlobals(hAgent,(h341ParameterName)lParam,(int)wParam);
        break;
    default:return DefWindowProc(hWnd,uMsg,wParam,lParam);
    }
    return 1l;
}


/*
 SnmpExtensionQuery is called by NT SNMP service upon receiving request from the
 network.This function translates perameters and passes  request to the
 RV snmp subagent module.
*/

BOOL WINAPI SnmpExtensionQuery(
                               IN BYTE                   requestType,
                               IN OUT RFC1157VarBindList *variableBindings,
                               OUT AsnInteger            *errorStatus,
                               OUT AsnInteger            *errorIndex)
{
    unsigned ii;
    AsnObjectIdentifier  temp;
    int nid[RequestNumber][MAX_ID_SIZE];
    *errorStatus = SNMP_ERRORSTATUS_NOERROR;

    if (requestType == SNMP_PDU_GETNEXT)
    {
        int res;
        snmpObjectT oidMr[RequestNumber],oidMrNext[RequestNumber];
        h341ErrorT error[RequestNumber];
        char buffer[4096];
        int offset=0;

        if (variableBindings->len > RequestNumber )
        {
            *errorStatus = SNMP_ERRORSTATUS_GENERR;
            *errorIndex=0;
            return FALSE;
        }

        for (ii=0;ii<variableBindings->len;ii++)
        {
            oidMr[ii].id = variableBindings->list[ii].name.ids;
            oidMr[ii].length = variableBindings->list[ii].name.idLength;

            oidMrNext[ii].id = &nid[ii][0];
            oidMrNext[ii].data.value=buffer+offset;
            oidMrNext[ii].data.valueSize=100;

            offset+=100;
        }
        *errorIndex = RequestNumber;

        res = h341AgentGetNext(hAgent,oidMr,oidMrNext,error,variableBindings->len);
        if (res)
        {
            *errorStatus = SNMP_ERRORSTATUS_GENERR;
            *errorIndex=0;
            return TRUE;
        }
        for (ii=0;ii<variableBindings->len;ii++)
        {
            if (!error[ii])
            {
                temp.ids = oidMrNext[ii].id;
                temp.idLength = oidMrNext[ii].length;
                SnmpUtilOidFree( &variableBindings->list[ii].name );
                SnmpUtilOidAppend( &variableBindings->list[ii].name, &temp );
                if ((oidMrNext[ii].data.type==asnMibOctetString)||
                    (oidMrNext[ii].data.type==asnAddressString))
                {
                    variableBindings->list[ii].value.asnValue.string.length = oidMrNext[ii].data.valueSize;
                    variableBindings->list[ii].value.asnValue.string.dynamic = FALSE;
                    variableBindings->list[ii].value.asnValue.string.stream=NULL;
                    if (oidMrNext[ii].data.type==asnMibOctetString)
                        variableBindings->list[ii].value.asnType=ASN_OCTETSTRING;
                    else
                        variableBindings->list[ii].value.asnType=ASN_IPADDRESS;
                    if (oidMrNext[ii].data.valueSize)
                    {
                        variableBindings->list[ii].value.asnValue.string.stream= SnmpUtilMemAlloc(oidMrNext[ii].data.valueSize);
                        if (variableBindings->list[ii].value.asnValue.string.stream!=NULL)
                        {
                            memcpy(variableBindings->list[ii].value.asnValue.string.stream,oidMrNext[ii].data.value,
                            oidMrNext[ii].data.valueSize);
                            variableBindings->list[ii].value.asnValue.string.dynamic = TRUE;
                        }
                    }

                }
                else if (oidMrNext[ii].data.type==asnInt)
                {
                    variableBindings->list[ii].value.asnValue.number = oidMrNext[ii].data.valueSize;
                    variableBindings->list[ii].value.asnType=ASN_INTEGER;
                }
            }
            else
            {
                if (error[ii]==outOfMibRc)
                    *errorStatus = SNMP_ERRORSTATUS_NOSUCHNAME;
                else
                    *errorStatus =SNMP_ERRORSTATUS_GENERR;
                *errorIndex = ii;
                break;
            }
        }
        return TRUE;
    }
    if (requestType == SNMP_PDU_GET)
    {
        snmpObjectT oidMr[ RequestNumber];
        h341ErrorT error[RequestNumber];
        char buffer[4096];
        int offset=0;
        if (variableBindings->len > RequestNumber )
        {
            *errorStatus = SNMP_ERRORSTATUS_GENERR;
            *errorIndex=0;
            return FALSE;
        }
        for (ii=0;ii<variableBindings->len;ii++)
        {
            oidMr[ii].id = variableBindings->list[ii].name.ids;
            oidMr[ii].length = variableBindings->list[ii].name.idLength;
            oidMr[ii].data.value=buffer+offset;
            oidMr[ii].data.valueSize=100;
            offset+=100;
        }
        if (!h341AgentGetValue( hAgent,oidMr,  error,variableBindings->len))
        {
            for (ii=0;ii<variableBindings->len;ii++)
            {
                if (!error[ii])
                {
                    if ((oidMr[ii].data.type==asnMibOctetString)||
                        (oidMr[ii].data.type==asnAddressString))

                    {
                        variableBindings->list[ii].value.asnValue.string.length = oidMr[ii].data.valueSize;
                        variableBindings->list[ii].value.asnValue.string.stream=NULL;
                        if (oidMr[ii].data.type==asnMibOctetString)
                            variableBindings->list[ii].value.asnType=ASN_OCTETSTRING;
                        else
                            variableBindings->list[ii].value.asnType=ASN_IPADDRESS;

                        variableBindings->list[ii].value.asnValue.string.dynamic = FALSE;
                        if (oidMr[ii].data.valueSize)
                        {

                            variableBindings->list[ii].value.asnValue.string.stream= SnmpUtilMemAlloc(oidMr[ii].data.valueSize);
                            if (variableBindings->list[ii].value.asnValue.string.stream!=NULL)
                            {
                                memcpy(variableBindings->list[ii].value.asnValue.string.stream,oidMr[ii].data.value,
                                oidMr[ii].data.valueSize);
                                variableBindings->list[ii].value.asnValue.string.dynamic = TRUE;
                            }
                        }

                    }
                    else if (oidMr[ii].data.type==asnInt)
                    {
                        variableBindings->list[ii].value.asnValue.number = oidMr[ii].data.valueSize;
                        variableBindings->list[ii].value.asnType=ASN_INTEGER;
                    }
                }
                else
                {
                    *errorStatus =SNMP_ERRORSTATUS_GENERR;
                    *errorIndex = ii;
                    break;
                }
            }


        }
        return TRUE;
    }
    if (requestType == SNMP_PDU_SET)
    {
        snmpObjectT oid;
        h341ErrorT error;
        mibDataT  data;
        if (variableBindings->len > 1 )
        {
            *errorStatus = SNMP_ERRORSTATUS_GENERR;
            *errorIndex=0;
            return FALSE;
        }
        oid.id = variableBindings->list[0].name.ids;
        oid.length = variableBindings->list[0].name.idLength;

        winData2mibData(&variableBindings->list[0].value,&data);
        error =  h341AgentSetValue(  hAgent,&oid,&data);
        if (error)
        {
            *errorStatus = SNMP_ERRORSTATUS_GENERR;
            *errorIndex=0;

        }
        return TRUE;
    }

    return FALSE;

 }



BOOL WINAPI SnmpExtensionTrap(
                              OUT AsnObjectIdentifier *enterprise,
                              OUT AsnInteger          *genericTrap,
                              OUT AsnInteger          *specificTrap,
                              OUT AsnTimeticks        *timeStamp,
                              OUT RFC1157VarBindList  *variableBindings)
{
    return FALSE;
}
