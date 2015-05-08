#include <windows.h>
#include <process.h>
#include <applSnmpI.h>
#include <cm.h>



BOOL APIENTRY DllMain( HANDLE hModule,
                       DWORD  ul_reason_for_call,
                       LPVOID lpReserved
					 )
{
    return TRUE;
}



h341InstanceHandle hSnmp=NULL;
HANDLE hMapFile,hEvent,stopEventSnmp,hEventWait,hEventUnreg;
LPVOID lpMapAddress;
DWORD treadId;
DWORD snmpThreadId;

void changeGlobals( h341AgentComHandle hAgentCom,h341ParameterName lParam,int wParam);


#define     WM_REGISTER_SUBAG    WM_USER+101
#define     WM_UNREGISTER_SUBAG    WM_USER+102
#define		WM_RETRIEVE_NEXT_INDEX   WM_USER+103
#define		WM_RETRIEVE_BY_INDEX   WM_USER+104
#define		WM_RETRIEVE				WM_USER+108
#define     WM_CHANGE_GLOBAL		WM_USER+106
#define     WM_SET_PARAMETER		WM_USER+107
#define		WM_RETRIEVE_MR			WM_USER+105


/*
    h341InstanceWnd is called on receiving notification from subagent
    about new request,using event for synchronization,retrieves parameters
    and its values from mapped file,processes the request and puts response into
    the same mapped file, sets event to break waiting loop of subagent.

*/


LRESULT RVCALLCONV
		h341InstanceWnd(HWND hWnd,UINT uMsg,WPARAM wParam,LPARAM lParam)
{
switch(uMsg)
{
			case WM_SET_PARAMETER:
				{
				char buff[1024];
			    mibDataT data;
                int error;
                char * ptr;
				h341ParameterName name;
				int bufferSize=1024;
                data.value  = buff;
                ptr = lpMapAddress ;
                ptr=ptr+30;

                memcpy((char *)&name,ptr,4);
                ptr+=4;

                memcpy((char *)&data.type,ptr,4);
                ptr+=4;

                memcpy((char *)&data.valueSize,ptr,4);
                ptr+=4;

                if ((data.type == asnMibOctetString)||(data.type == asnAddressString))
                    memcpy(data.value,ptr,data.valueSize);

				error = h341InstSetRequest(hSnmp, name,&data);

                ptr = lpMapAddress ;
                ptr=ptr+30;

                memcpy(ptr,(char *)&error,4);
                ptr+=4;

				}
			break;
            case WM_RETRIEVE_MR:
                {
                    char buff[1024];
                    char temp[2048];
                    char * ptr,*ptrTemp;
                    int res;
                    int reqNum,ii;

                    instanceRequestDataT instRequests;

                    mibDataT data;
                    ptrTemp = temp;

                    ptr = lpMapAddress ;
                    ptr=ptr+30;
                    memcpy((char *)&reqNum,ptr,4);
                    ptr+=4;

                    for(ii=0;ii<reqNum;ii++)
                    {
                        data.value=buff;
                        data.valueSize=1024;

                        memcpy((char *)&instRequests.reqType,ptr,4);
                        ptr+=4;
                        memcpy((char *)&instRequests.name,ptr,4);
                        ptr+=4;
                        memcpy((char *)&instRequests.indexSize,ptr,4);
                        ptr+=4;


                        if( instRequests.indexSize)
                        {
                            memcpy((char *)&instRequests.index,ptr,instRequests.indexSize);
                            ptr+=instRequests.indexSize;

                        }
                        if (instRequests.reqType==retrieveByIndex)
                        {
                            res=h341InstGetByIndex (hSnmp,instRequests.index,instRequests.indexSize,instRequests.name,&data);
                            memcpy(ptrTemp,(char *)&res,4);
                            ptrTemp +=4;
                            if(!res)
                            {
                                memcpy(ptrTemp,(char *)&data.type,4);
                                ptrTemp +=4;
                                if (data.type!=asnError)
                                {
                                    memcpy(ptrTemp,(char *)&data.valueSize,4);
                                    ptrTemp +=4;
                                    if ((data.type==asnMibOctetString)||(data.type==asnAddressString))
                                    {
                                        memcpy(ptrTemp,data.value,data.valueSize);
                                        ptrTemp+=data.valueSize;
                                    }
                                }
                            }

                        }
                        else
                            if (instRequests.reqType==retrieveParam)
                            {
                                res=h341InstGetParameter(hSnmp, instRequests.name,&data);
                                memcpy(ptrTemp,(char *)&res,4);
                                ptrTemp +=4;
                                if(!res)
                                {
                                    memcpy(ptrTemp,(char *)&data.type,4);
                                    ptrTemp +=4;
                                    if (data.type!=asnError)
                                    {
                                        memcpy(ptrTemp,(char *)&data.valueSize,4);
                                        ptrTemp +=4;
                                        if ((data.type==asnMibOctetString)||(data.type==asnAddressString))
                                        {
                                            memcpy(ptrTemp,data.value,data.valueSize);
                                            ptrTemp+=data.valueSize;
                                        }
                                    }
                                }

                            }
                            else if (instRequests.reqType==retrieveNextIndex)
                            {
                                res=h341InstGetNextIndex (hSnmp,instRequests.name,instRequests.index,instRequests.indexSize,instRequests.nextIndex,&instRequests.nextIndexSize,&data);
                                memcpy(ptrTemp,(char *)&res,4);
                                ptrTemp=ptrTemp+4;
                                if (!res)
                                {
                                    memcpy(ptrTemp,(char *)&instRequests.nextIndexSize,4);
                                    ptrTemp=ptrTemp+4;
                                    memcpy(ptrTemp,instRequests.nextIndex,instRequests.nextIndexSize);
                                    ptrTemp=ptrTemp+instRequests.nextIndexSize;

                                    memcpy(ptrTemp,(char *)&data.type,4);
                                    ptrTemp=ptrTemp+4;
                                    if (data.type!=asnError)
                                    {
                                        memcpy(ptrTemp,(char *)&data.valueSize,4);
                                        ptrTemp=ptrTemp+4;
                                        if ((data.type==asnMibOctetString)||(data.type==asnAddressString))
                                        {
                                            memcpy(ptrTemp,data.value,data.valueSize);
                                            ptrTemp+=data.valueSize;
                                        }
                                    }
                                }
                            }

                    }
                    ptr = lpMapAddress;
                    ptr+=4;
                    memcpy(ptr,temp,(ptrTemp-temp));
                }
                break;
            case WM_RETRIEVE_NEXT_INDEX:  /* send message */
			{
				char buff[1024];
				int res;
				h341ParameterName name;
				UINT8  index[100],nextIndex[100];

				int indexSize,nextIndexSize;
                char *ptr;
				mibDataT data;
				data.value=buff;
				data.valueSize=1024;


                 ptr = lpMapAddress ;
                 ptr=ptr+30;
                 memcpy((char *)&name,ptr,4);
                 ptr+=4;
                 memcpy((char *)&indexSize,ptr,4);
                 ptr+=4;
                 memcpy(index,ptr,indexSize);
				 res=h341InstGetNextIndex (hSnmp,name,index,indexSize,nextIndex,&nextIndexSize,&data);
                 ptr = lpMapAddress;
                 ptr=ptr+4;
                 memcpy(ptr,(char *)&res,4);
                 ptr=ptr+4;
                 if (!res)
                 {
                    memcpy(ptr,(char *)&nextIndexSize,4);
                    ptr=ptr+4;
                    memcpy(ptr,nextIndex,nextIndexSize);
                    ptr=ptr+nextIndexSize;

                    memcpy(ptr,(char *)&data.type,4);
                    ptr=ptr+4;
                    if (data.type!=asnError)
                    {
                        memcpy(ptr,(char *)&data.valueSize,4);
                        ptr=ptr+4;
                        if ((data.type==asnMibOctetString)||(data.type==asnAddressString))
                            memcpy(ptr,data.value,data.valueSize);
                    }
                 }


			}
			break;
		case WM_RETRIEVE_BY_INDEX:
			{
				char buff[1024];

				int res;
				h341ParameterName name;
				UINT8  index[100];
				int indexSize;
				mibDataT data;
                char *ptr;

				data.value=buff;
				data.valueSize=1024;


                 ptr = lpMapAddress ;
                 ptr=ptr+30;
                 memcpy((char *)&name,ptr,4);
                 ptr+=4;
                 memcpy((char *)&indexSize,ptr,4);
                 ptr+=4;
                 memcpy(index,ptr,indexSize);


				res=h341InstGetByIndex (hSnmp,index,indexSize,name,&data);


                 ptr = lpMapAddress;
                 ptr=ptr+4;
                 memcpy(ptr,(char *)&res,4);
                 ptr=ptr+4;
                 if (!res)
                 {
                    memcpy(ptr,(char *)&data.type,4);
                    ptr=ptr+4;
                    if (data.type!=asnError)
                    {
                        memcpy(ptr,(char *)&data.valueSize,4);
                        ptr=ptr+4;
                        if ((data.type==asnMibOctetString)||(data.type==asnAddressString))
                            memcpy(ptr,data.value,data.valueSize);
                    }
                 }



			}

		break;

		case  WM_RETRIEVE:
			{
				char buff[1024];
				int res;
				h341ParameterName name;
				char *ptr;


				mibDataT data;
				data.value=buff;
				data.valueSize=1024;

				name = (h341ParameterName)lParam;
				res=h341InstGetParameter(hSnmp, name,&data);

                 ptr = lpMapAddress;
                 ptr=ptr+4;
                 memcpy(ptr,(char *)&res,4);
                 ptr=ptr+4;
                 if (!res)
                 {

                    memcpy(ptr,(char *)&data.type,4);
                    ptr=ptr+4;
                    if (data.type!=asnError)
                    {
                        memcpy(ptr,(char *)&data.valueSize,4);
                        ptr=ptr+4;
                        if ((data.type==asnMibOctetString)||(data.type==asnAddressString))
                            memcpy(ptr,data.value,data.valueSize);
                    }
                 }


			}

			break;
    default:return DefWindowProc(hWnd,uMsg,wParam,lParam);
}
return 1l;
}

void h341InstUnRegister(h341InstanceHandle  hSnmp)
{
    DWORD ownerId;
    ownerId = GetCurrentThreadId();
	PostThreadMessage(treadId,WM_UNREGISTER_SUBAG,0,ownerId);
    UnmapViewOfFile(lpMapAddress);
    CloseHandle(hMapFile);

}
/*
changeGlobals is the callback function for setting global parameters value,call  the
PostThreadMessage to the registration tread of subagent,using  its tread id ,
received in time of initialization
*/

void changeGlobals(IN h341AgentComHandle hAgentCom,h341ParameterName lParam,int wParam)
{
	 PostThreadMessage((DWORD)hAgentCom,WM_CHANGE_GLOBAL,(WPARAM)wParam,lParam);
}

/*====================================================
 * h341InstRegister is called for registration..
 * Parameter : hSnmp - SNMP handle  .
 * Return    :  0 if failed to register an instance.
 *              instance number - if registered OK
 *=====================================================*/
int h341InstRegister(h341InstanceHandle  hSnmp)
{

	char s[20];
	int error;
	int res;
    int ifIndex;
    DWORD ownerId;
    SECURITY_DESCRIPTOR SD;
    SECURITY_ATTRIBUTES sa;


    if (!InitializeSecurityDescriptor(&SD, SECURITY_DESCRIPTOR_REVISION))
        return 0;

    /* Add a null DACL to the security descriptor. */
    if (!SetSecurityDescriptorDacl(&SD, TRUE, (PACL) NULL, FALSE))
        return 0;

    sa.nLength = sizeof(sa);
    sa.lpSecurityDescriptor = &SD;
    sa.bInheritHandle = TRUE;
/*
    try to open mapped file,created by the agent,to communicate with it
*/
    hMapFile = OpenFileMapping(FILE_MAP_ALL_ACCESS, FALSE,"AgentInstComm");

    if (hMapFile == NULL)
    {
        error= GetLastError();
        return 0;
    }



    lpMapAddress = MapViewOfFile(hMapFile,FILE_MAP_ALL_ACCESS,0,0,0);

    if (lpMapAddress == NULL)
        return 0;
    ownerId = GetCurrentThreadId();
    sprintf(s,"AgentNtf%d",ownerId);
/*
    create synchronization named events.Names are based on thread id of the current tread.
    This thread id is passed to subagent in registration message for  subagent be able to
    open the same event
    The system automatically resets the state to nonsignaled after a single waiting thread has been released. .
    The event initial state is NON signaled.  
*/
    hEvent = CreateEvent(&sa,FALSE,FALSE,s);
    if (hEvent==NULL)
        return 0;

    sprintf(s,"unreg%d",ownerId);
    hEventUnreg  = CreateEvent(&sa,FALSE,FALSE,s);
    if (hEventUnreg==NULL)
        return FALSE;

    hEventWait = OpenEvent(EVENT_ALL_ACCESS,FALSE,"AgentInstWait");
    if (hEventWait==NULL)
        return 0;

/*
    read from mapped file thread id of the subagent registration thread
    4 first bytes of the mapped file view subagent uses to store tread id
    of registration tread and instances must get it for  communication.
*/

    memcpy((char *)&treadId,lpMapAddress,4);
    h341InstSetAgentComHandle (hSnmp,(h341AgentComHandle)treadId);

	/* send regisration message to subAgent thread */
	res = PostThreadMessage(treadId,WM_REGISTER_SUBAG,0,(LPARAM)ownerId);
	if(!res)
	{
	   int error=GetLastError();
	   return 0;
	}

	/*h.e wait for "reply" from sub agent with instance number (ifIndex)) */
	WaitForSingleObject(hEvent, INFINITE);

	/* now we can get ifIndex  from second 4 bytes of mapped file */
	ifIndex=*(int *)((char *)lpMapAddress + 4);

	return ifIndex;
}




unsigned __stdcall snmpThread(void * argv)
{

    int index;
    HCFG hCfg;
    int isString,maxCalls,maxChannels;
    h341ApplHandle hApp = (h341ApplHandle)argv;
    MibEventT mibEvent = 
	{(h341AddNewCallT)			h341InstAddNewCall,
	 (h341DeleteCallT)			h341InstDeleteCall,
	 (h341AddControlT)			h341InstAddControl,
     (h341DeleteControlT)		h341InstDeleteControl,
	 (h341AddNewLogicalChannelT)h341InstAddNewLogicalChannel,
     (h341DeleteLogicalChannelT)h341InstDeleteLogicalChannel};

    hCfg=cmGetConfiguration((HAPP)hApp);
    if (ciGetValue(hCfg,"system.maxCalls" ,&isString,&maxCalls)<0)
        maxCalls = 10;
    if (ciGetValue(hCfg,"system.maxChannels" ,&isString,&maxChannels)<0)
        maxChannels = 1;
    index = h341InstInit(hApp,maxCalls,maxChannels,NULL,changeGlobals,&hSnmp);
    if (index)
        return 0;


    index= h341InstRegister(  hSnmp);

    if (index)
    {
	   h341SetInstanceNumber(hSnmp, index);
	} 
	else
    {
        h341InstEnd(hSnmp);
        hSnmp=NULL;
        return 0;
    }

    cmMibEventSet((HAPP)hApp,&mibEvent,(HMibHandleT)hSnmp);



    {
        HANDLE lpHandles[3];
        lpHandles[0] = hEvent;
        lpHandles[1] = stopEventSnmp;
        lpHandles[2] = hEventUnreg;
        while (TRUE)
        {
            index = WaitForMultipleObjects(3,lpHandles,FALSE,INFINITE);
            index-=WAIT_OBJECT_0 ;
            if (index ==1)
                h341InstUnRegister(hSnmp);
            else if(index==0)
            {
                char *ptr;
                UINT uMsg;
                WPARAM wParam;
                LPARAM lParam;
                ptr = lpMapAddress ;
                ptr +=4;
                memcpy((char *)&uMsg,ptr,sizeof(UINT));
                ptr+=sizeof(UINT);
                memcpy((char *)&wParam,ptr,sizeof(WPARAM));
                ptr+=sizeof(WPARAM);
                memcpy((char *)&lParam,ptr,sizeof(LPARAM));
                ptr+=sizeof(LPARAM);
				/* pass the request to instance */
                cmMeiEnter((HAPP)hApp);
                h341InstanceWnd(NULL,uMsg, wParam, lParam);
                cmMeiExit((HAPP)hApp);
				/* inform the subagent that the "answer" is ready */
                SetEvent(hEventWait);
            }
            else if (index==2)
                break;
        }
    }
    CloseHandle(stopEventSnmp);
    CloseHandle(hEventWait);
    CloseHandle(hEvent);
    CloseHandle(hEventUnreg);
    memset(&mibEvent,0,sizeof(MibEventT));
    cmMibEventSet((HAPP)hApp,&mibEvent,NULL);

    h341InstEnd(hSnmp);
    hSnmp = NULL;
    return 1;
 }




RVAPI
void RVCALLCONV applSnmpInstanceInit(IN h341ApplHandle hApp)
{
    stopEventSnmp = CreateEvent(NULL,FALSE,FALSE,NULL);
	_beginthreadex(NULL,0, snmpThread, (void *)hApp,0,&snmpThreadId );
}

RVAPI
void RVCALLCONV applSnmpInstanceEnd()
{
    SetEvent(stopEventSnmp);
}



