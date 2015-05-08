#ifdef __cplusplus
extern "C" {
#endif

#include <cm.h>
#include <mib.h>
#include <cmmib.h>

typedef enum
{ 
  INCOMMING,
  OUTGOING
} directionT;

#define H245_CONTROL_KEY_SIZE   20
#define H245_CAPEXC_KEY_SIZE    12
#define H245_LC_KEY_SIZE        12
#define H245_LC_H225_KEY_SIZE   4

void int2index(int entry,UINT8 * key);

void transportAddress2Index(cmTransportAddress *tr,UINT8 * key);

void transportAddress2String(cmTransportAddress *tr,UINT8 * key);
void buildCapExchangeCapabilityTableIndex(int ifIndex, int entry, directionT, UINT8 * key);

int bool2truth(int value)
{
    if(value)
        return 1;
    else
        return 2;
}





typedef struct
{
    UINT8 key[H245_CONTROL_KEY_SIZE];/*23];*/
    HCALL hsCall;
    int controlChannelIndex;
}controlElT;



BOOL controlCompare(RAElement element, void *param)
{
    return (((controlElT *)element)->hsCall == (HCALL)param);
}


int tableCtrlGet(RAElement  data,h341ParameterName name,mibDataT *mibdata)
{
    mibdata->type=asnError;
    switch (name)
    {
    case h245ControlChannelIndex:
        mibdata->valueSize = ((controlElT *)data)->controlChannelIndex;
        mibdata->type=asnInt;
        break;
    case h245ControlChannelSrcAddressTag:             /* fore next are not accessible*/
    case h245ControlChannelSrcTransporTAddress:
    case h245ControlChannelDesTAddressTag:
    case h245ControlChannelDestTransporTAddress:
        return checkNext;

    case h245ControlChannelMSDState:
        mibdata->type=asnInt;
        if (mibGetControlParameters(((controlElT *)data)->hsCall,0,
            enumh245ControlChannelMSDState,&mibdata->valueSize,mibdata->value))
            mibdata->valueSize=0;
        break;
    case h245ControlChannelTerminalType:
        mibdata->type=asnInt;
        if (mibGetControlParameters(((controlElT *)data)->hsCall,0,
            enumh245ControlChannelTerminalType,&mibdata->valueSize,mibdata->value))
            mibdata->valueSize=0;
        break;
    case h245ControlChannelNumberOfMSDRetries:
        mibdata->type=asnInt;
        if (mibGetControlParameters(((controlElT *)data)->hsCall,0,
            enumh245ControlChannelNumberOfMSDRetries,&mibdata->valueSize,mibdata->value))
            mibdata->valueSize=0;
        break;
    case h245ControlChannelIsTunneling:
        mibdata->type=asnInt;
        if (mibGetControlParameters(((controlElT *)data)->hsCall,0,
            enumh245ControlChannelIsTunneling,&mibdata->valueSize,mibdata->value))
            mibdata->valueSize=0;
        break;
    default:
        return notImplemented;
    }
    return 0;
}

 /****************************************************************
 *tableControlGetIndex - get an index of the Control Table
 *parameters : .
 *  Input    : data - element in the table     
 *****************************************************************/
UINT8 * tableCtrlGetIndex(RAElement     data)
{
    return ((controlElT *)data)->key;
}


tableT *  contolTableConstruct(int nElements)
{
    return tableConstruct(nElements,H245_CONTROL_KEY_SIZE,sizeof(controlElT),tableCtrlGet,controlCompare,tableCtrlGetIndex);

}

int tableCtrlSetDefault(RAElement data,h341ParameterName name,mibDataT * mibData)
{
    if (name == h245ControlChannelIndex)
        ((controlElT *)data)->controlChannelIndex = mibData->valueSize;

    return 0;
}


typedef struct
{
    UINT8 key[H245_CAPEXC_KEY_SIZE];
    HCALL hsCall;
}capabilityElT;


BOOL capabilityCompare(RAElement element, void *param)
{
    return (((capabilityElT *)element)->hsCall == (HCALL)param);
}


/*==========================================================================.
 * h245CapExchageCapabilityTableGet 
 * purpose : get parameter from h245CapExchageCapabilityTable
 *
 *---------------------------------------------------------------------------*/
int h245CapExchageCapabilityTableGet(RAElement   data,h341ParameterName name,mibDataT *mibdata)
{
    int directionIn=0;
    UINT8 * key = ((capabilityElT *)data)->key;
    HCALL hsCall = ((capabilityElT *)data)->hsCall;


	/* key[0] - ifIndex
	   key[1] - h245ControlChannelIndex
	   key[2] - h245CapabilityExchangeDirection
     */ 
	switch (key[2])
    {
	case INCOMMING:
        directionIn = 1;
		break;
    case OUTGOING:
        directionIn = 0;
	}
    mibdata->type=asnError;

    switch(name)
    {

    case h245CapExchangeDirection: /* not accessible */
        return checkNext;
    case h245CapExchangeState:
        if (!mibGetControlParameters(hsCall, directionIn,enumh245CapExchangeState,
                                     &mibdata->valueSize,mibdata->value))
            mibdata->type=asnInt;
        break;

    case h245CapExchangeProtocolId:
        mibdata->type=asnMibOctetString;
        if (mibGetControlParameters(hsCall, directionIn,enumh245CapExchangeProtocolId,
                                     &mibdata->valueSize,mibdata->value))

            mibdata->valueSize=0;
        break;
    case h245CapExchangeRejectCause:
        mibdata->type=asnInt;
        if (mibGetControlParameters(hsCall, directionIn,enumh245CapExchangeRejectCause,
                                     &mibdata->valueSize,mibdata->value))
            mibdata->valueSize=0;
        break;

    case h245CapExchangeMultiplexCapability:
    case h245CapExchangeCapability:
    case h245CapExchangeCapabilityDescriptors:

    default:
        return notImplemented;
    }
    return 0;
}

/*=========================================================
 *tableCapGetIndex - get an index of the Capability Table 
 *---------------------------------------------------------*/
UINT8 * tableCapGetIndex(RAElement  data)
{
    return ((capabilityElT *)data)->key;
}


tableT *  capTableConstruct(int nElements)
{
	return tableConstruct(nElements, H245_CAPEXC_KEY_SIZE, sizeof(capabilityElT), h245CapExchageCapabilityTableGet,NULL, tableCapGetIndex);

}




typedef struct
{
    UINT8 key[H245_LC_KEY_SIZE];/*23];*/
    HCHAN hsChan;
    int   lcIndex;
}logChanElT;

int tableLcSetDefault(RAElement data,h341ParameterName name,mibDataT * mibData)
{
    if (name == h245LogChannelsIndex)
        ((logChanElT *)data)->lcIndex = mibData->valueSize;
    return 0;
}

int tableLCGet(RAElement    data,h341ParameterName name,mibDataT *mibdata)
{
    mibdata->type=asnError;

    switch (name)
    {
    case h245LogChannelsChannelNumber:
    case h245LogChannelsChannelDirection:
        return checkNext;

    case h245LogChannelsIndex:
        mibdata->valueSize=((logChanElT *)data)->lcIndex;
        mibdata->type=asnInt;
        break;

    case h245LogChannelsChannelState:
        mibdata->type=asnInt;
        if (mibGetChannelParameters(((logChanElT *)data)->hsChan,enumh245LogChannelsChannelState,
                                     &mibdata->valueSize,mibdata->value))
            mibdata->valueSize=0;
            break;
    case h245LogChannelsMediaTableType:
        mibdata->type=asnInt;
        if (mibGetChannelParameters(((logChanElT *)data)->hsChan,enumh245LogChannelsMediaTableType,
                                     &mibdata->valueSize,mibdata->value))
            mibdata->valueSize=0;
            break;

    default:
        return notImplemented;
    }
    return 0;
}


/*****************************************************************
 *tableLCIndex - get an index of the LogicalChannel Table
 *parameters : 
 *  Input    : data - element in the table     
 *****************************************************************/
UINT8 * tableLCGetIndex(RAElement   data)
{
    return ((logChanElT *)data)->key;
}



BOOL hsChanCompare(RAElement element, void *param)
{
    return (((logChanElT *)element)->hsChan == (HCHAN)param);
}

tableT *  lcTableConstruct(int nElements)
{
    return tableConstruct(nElements,H245_LC_KEY_SIZE,sizeof(logChanElT),tableLCGet,hsChanCompare,tableLCGetIndex);

}



typedef struct
{
    UINT8 key[H245_LC_H225_KEY_SIZE];/*23];*/
    HCHAN hsChan;
}logH225ChanElT;


int tableLCH225Get(RAElement    data,h341ParameterName name,mibDataT *mibdata)
{
    mibdata->type=asnError;

    switch (name)
    {
    case h245LogChannelsSessionId:
        mibdata->valueSize=cmChannelSessionId(((logH225ChanElT *)data)->hsChan);
        mibdata->type=asnInt;
        break;
    case h245LogChannelsAssociateSessionId:
        mibdata->type=asnInt;
        if (mibGetChannelParameters(((logH225ChanElT *)data)->hsChan,enumh245LogChannelsAssociateSessionId,
                    &mibdata->valueSize,mibdata->value))
                    mibdata->valueSize=0;
                    break;

    case h245LogChannelsMediaGuaranteedDelivery:
        mibdata->type=asnInt;
        if (!mibGetChannelParameters(((logH225ChanElT *)data)->hsChan,enumh245LogChannelsMediaGuaranteedDelivery,
                    &mibdata->valueSize,mibdata->value))
            mibdata->valueSize = bool2truth(mibdata->valueSize);
        else
            mibdata->valueSize =0;
        break;
    case h245LogChannelsMediaControlGuaranteedDelivery:
        mibdata->type=asnInt;
        mibdata->valueSize=0;
        if (!mibGetChannelParameters(((logH225ChanElT *)data)->hsChan,enumh245LogChannelsMediaControlGuaranteedDelivery,
                    &mibdata->valueSize,mibdata->value))
                mibdata->valueSize = bool2truth(mibdata->valueSize);
                    break;
    case h245LogChannelsSilenceSuppression:
        mibdata->type=asnInt;
        mibdata->valueSize=0;
        if (!mibGetChannelParameters(((logH225ChanElT *)data)->hsChan,enumh245LogChannelsSilenceSuppression,
                    &mibdata->valueSize,mibdata->value))
                    mibdata->valueSize = bool2truth(mibdata->valueSize);
                    break;
    case h245LogChannelsDestination:
        mibdata->type=asnMibOctetString;
        if (mibGetChannelParameters(((logH225ChanElT *)data)->hsChan,enumh245LogChannelsDestination,
                    &mibdata->valueSize,mibdata->value))
                    mibdata->valueSize=0;
                    break;
    case h245LogChannelsSrcTerminalLabel:
        mibdata->type=asnMibOctetString;
        if (mibGetChannelParameters(((logH225ChanElT *)data)->hsChan,enumh245LogChannelsSrcTerminalLabel,
                    &mibdata->valueSize,mibdata->value))
                    mibdata->valueSize=0;
                    break;

    case h245LogChannelsMediaChannel:
        {
            cmTransportAddress media;
            int length;
            mibdata->type=asnAddressString;
            if (!mibGetChannelParameters(((logH225ChanElT *)data)->hsChan,enumh245LogChannelsMediaChannel,
                    &length,(UINT8 *)&media))
            {
                    transportAddress2String(&media,mibdata->value);
                    mibdata->valueSize=6;

            }
            else
                mibdata->valueSize=0;
            break;

        }
    case h245LogChannelsMediaControlChannel:
        {
            cmTransportAddress mediaControl;
            int length;
            mibdata->type=asnAddressString;
            if (!mibGetChannelParameters(((logH225ChanElT *)data)->hsChan,enumh245LogChannelsMediaControlChannel,
                    &length,(UINT8 *)&mediaControl))
            {

                    transportAddress2String(&mediaControl,mibdata->value);
                    mibdata->valueSize=6;

            }
            else
                mibdata->valueSize=0;
            break;

        }
    case h245LogChannelsDynamicRTPPayloadType:
        mibdata->type=asnInt;
        if (mibGetChannelParameters(((logH225ChanElT *)data)->hsChan,enumh245LogChannelsDynamicRTPPayloadType,
                    &mibdata->valueSize,mibdata->value))
                    mibdata->valueSize=0;
                    break;

    case h245LogChannelsH261aVideoPacketization:
        mibdata->type=asnInt;
        if (mibGetChannelParameters(((logH225ChanElT *)data)->hsChan,enumh245LogChannelsH261aVideoPacketization,
                    &mibdata->valueSize,mibdata->value))
                    mibdata->valueSize=0;
                    break;
    case h245LogChannelsRTPPayloadType:
        mibdata->type=asnInt;
        if (mibGetChannelParameters(((logH225ChanElT *)data)->hsChan,enumh245LogChannelsRTPPayloadType,
                    &mibdata->valueSize,mibdata->value))
                    break;
    case h245LogChannelsRTPPayloadDescriptor:
    case h245LogChannelsTransportCapability:
    case h245LogChannelsRedundancyEncoding:
    default:
        return notImplemented;
    }
    return 0;
}

UINT8 * tableLCH225GetIndex(RAElement   data)
{
    return ((logH225ChanElT *)data)->key;
}

BOOL hsChanH225Compare(RAElement element, void *param)
{
    return (((logH225ChanElT *)element)->hsChan == (HCHAN)param);
}



tableT *lcH225TableConstruct(int nElements)
{
    return tableConstruct(nElements,H245_LC_H225_KEY_SIZE,sizeof(logH225ChanElT),tableLCH225Get,hsChanH225Compare,tableLCH225GetIndex);

}


/*********************************************************************************
 * h341InstAddControl -  add info from all the Control and Capabilities Tables
 * Parameters
 * hSnmp - handle to Snmp
 * hsCall - handle to the call
 *********************************************************************************/
h341ErrorT h341InstAddControl(h341InstanceHandle hSnmp,HCALL hsCall)
{
    int  res,entry,isTunn;
    cmTransportAddress trSrc,trDest;
    controlElT data;
    mibDataT mibData;
    capabilityElT elIn;
    capabilityElT elOut;
    UINT8 * index;
	int ifIndex = ((h341InstanceHandleT *)hSnmp)->ifIndex;
	/*
	 * Is the info about a call already written in the control table?
	 */
    index = tableFindIndex(((h341InstanceHandleT *)hSnmp)->ControlTable,(void *) hsCall);
    if (index!=NULL)
    {
        res = cmCallGetH245Address(hsCall,&trSrc,&trDest,&isTunn);
        if (res<0)
            return (h341ErrorT)invalidCall;
        transportAddress2Index(&trSrc,data.key);
        transportAddress2Index(&trDest,&data.key[10]);
        if(!memcpy(index,data.key,sizeof(data.key)))
            return (h341ErrorT)0;
        tableUpdateIndex(((h341InstanceHandleT *)hSnmp)->ControlTable,index,data.key,H245_CONTROL_KEY_SIZE);
        return (h341ErrorT)0;
    }
    res = cmCallGetH245Address(hsCall,&trSrc,&trDest,&isTunn);
    if (res<0)
        return (h341ErrorT)invalidCall;
    data.hsCall=hsCall;
    transportAddress2Index(&trSrc,data.key);
    transportAddress2Index(&trDest,&data.key[10]);

    res = tableAdd( ((h341InstanceHandleT *)hSnmp)->ControlTable,(RAElement)&data,&entry);

    if (!res)
    {

        mibData.valueSize=entry;
        tableSetDefault(((h341InstanceHandleT *)hSnmp)->ControlTable,entry,h245ControlChannelIndex,
                    &mibData,tableCtrlSetDefault);

        buildCapExchangeCapabilityTableIndex(ifIndex, entry, INCOMMING, (UINT8 *)&elIn.key);
        buildCapExchangeCapabilityTableIndex(ifIndex, entry, OUTGOING, (UINT8 *)&elOut.key);

            elIn.hsCall=hsCall;
            elOut.hsCall=hsCall;
            tableAdd( ((h341InstanceHandleT *)hSnmp)->CapTable,(RAElement)&elIn,&entry);
            tableAdd( ((h341InstanceHandleT *)hSnmp)->CapTable,(RAElement)&elOut,&entry);
    }
    return (h341ErrorT)res;

}

/*********************************************************************************
 * h341InstDeleteControl -  delete info from all the Control and Capabilities Tables
 * Parameters
 * hSnmp - handle to Snmp
 * hsCall - handle to the call
 *********************************************************************************/
h341ErrorT     h341InstDeleteControl(h341InstanceHandle hSnmp,HCALL hsCall)
{

    UINT8 * index;
    int entry,res;
    capabilityElT elIn;
    capabilityElT elOut;
    mibDataT mibData;
	int ifIndex = ((h341InstanceHandleT *)hSnmp)->ifIndex;
	/*
	 * Is there info about a call?
	 */
    index = tableFindIndex(((h341InstanceHandleT *)hSnmp)->ControlTable,(void *) hsCall);

    if (index!=NULL)
    {
       res = getByIndex (((h341InstanceHandleT *)hSnmp)->ControlTable ,index, H245_CONTROL_KEY_SIZE,
                h245ControlChannelIndex,&mibData);
        if(!res)
        {
            entry = mibData.valueSize;
			buildCapExchangeCapabilityTableIndex(ifIndex, 0, INCOMMING, elIn.key);
			buildCapExchangeCapabilityTableIndex(ifIndex, 0, OUTGOING,  elOut.key);

			/*
			 * Remove info from Capabilities (IN and OUT) tables
			 */
            tableDelete(((h341InstanceHandleT *)hSnmp)->CapTable,elIn.key,H245_CAPEXC_KEY_SIZE);
            tableDelete(((h341InstanceHandleT *)hSnmp)->CapTable,elOut.key,H245_CAPEXC_KEY_SIZE);
        }

		/*
		 * Remove info from control (calls) table
		 */
        tableDelete(((h341InstanceHandleT *)hSnmp)->ControlTable,index,H245_CONTROL_KEY_SIZE);
    }
    return (h341ErrorT)0;

}



h341ErrorT h341InstAddNewLogicalChannel(h341InstanceHandle hSnmp, HCHAN hsChan)
{

    int res=TRUE,entry;
    UINT32  temp;
    HCALL hsCall;
    /*cmTransportAddress trSrc,trDest;
    controlElT data;*/
    logChanElT logChan;
    logH225ChanElT h225logChan;
    mibDataT mibData;
    int lcn;
    UINT8 * index;

    mibData.valueSize = 0;

    cmChannelGetCallHandles(hsChan,&hsCall,NULL);

/*
    res = cmCallGetH245Address(hsCall,&trSrc,&trDest,&temp);
    if (res<0)
        return invalidCall;
    data.hsCall=hsCall;
    transportAddress2Index(&trSrc,data.key);
    transportAddress2Index(&trDest,&data.key[10]);
*/
    index = tableFindIndex(((h341InstanceHandleT *)hSnmp)->ControlTable,(void *) hsCall);

    if (index!=NULL)

       res = getByIndex (((h341InstanceHandleT *)hSnmp)->ControlTable ,index, H245_CONTROL_KEY_SIZE,
                h245ControlChannelIndex,&mibData);


    if (!res)
    {

            memset(logChan.key,0,12);
            int2index( mibData.valueSize,logChan.key);
            lcn = cmChannelGetNumber(hsChan);
            int2index( lcn,&logChan.key[4]);
            cmChannelGetOrigin(hsChan,(int *)&temp);
            if (temp)
                logChan.key[11] = 2;
            else
                logChan.key[11] = 1;
            logChan.hsChan = hsChan;
            res = tableAdd( ((h341InstanceHandleT *)hSnmp)->LCTable,(RAElement)&logChan,&entry);
            mibData.valueSize=entry;
            if (!res)
            {
                tableSetDefault(((h341InstanceHandleT *)hSnmp)->LCTable,entry,h245LogChannelsIndex,
                    &mibData,tableLcSetDefault);
                int2index(entry,h225logChan.key);
                h225logChan.hsChan = hsChan;

                res =tableAdd( ((h341InstanceHandleT *)hSnmp)->LCH225Table,(RAElement)&h225logChan,&entry);
            }


    }

    return (h341ErrorT)res;
}

h341ErrorT     h341InstDeleteLogicalChannel(h341InstanceHandle hSnmp,HCHAN hsChan)
{
    UINT8 * index;
    index = tableFindIndex(((h341InstanceHandleT *)hSnmp)->LCH225Table,(void *) hsChan);
    if (index!=NULL)
    {
        tableDelete(((h341InstanceHandleT *)hSnmp)->LCH225Table,index,H245_LC_H225_KEY_SIZE);
    }
    index = tableFindIndex(((h341InstanceHandleT *)hSnmp)->LCTable,(void *) hsChan);
    if (index!=NULL)
    {
        tableDelete(((h341InstanceHandleT *)hSnmp)->LCTable,index,H245_LC_KEY_SIZE);
    }
    return (h341ErrorT)0;
}

void  buildCapExchangeCapabilityTableIndex(int ifIndex, int entry, directionT direction, UINT8  *key)
{
	int *pIfIndex = (int *)key;
	int *ph245ControlChannelIndex = (int *)(key + sizeof(int));
	int *pCapExchangeDirection = (int *)(key + 2*sizeof(int));

	memset(key, 0, H245_CAPEXC_KEY_SIZE );

	/* set ifIndex part */
	*pIfIndex = ifIndex;

	/* set h245ControlChannel index part */
	*ph245ControlChannelIndex = entry;

	/* set direction index part */
	if (direction==INCOMMING)
	  *pCapExchangeDirection=1;
	else if (direction==OUTGOING)
	  *pCapExchangeDirection=2;
}

#ifdef __cplusplus
}
#endif
