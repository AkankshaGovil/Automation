#ifdef __cplusplus
extern "C" {
#endif
#include <ctype.h>

#include "siplist.h"
#include "sipstruct.h"
#include "portlayer.h"
#include "sipfree.h"
#include "sipinit.h"
#include "sipdecode.h"
#include "sdpdecode.h"
#include "sipdecodeintrnl.h"
#include "sipparserinc.h"

	SipBool sip_decodeSdpMessage(SdpMessage **ppSdpMessage, char *buffer,\
			SipError *err)
	{
		int index;
		int flag=0, i,j;
		SipList sdpheaders, GCList;

		SipParseEachSdpLineParam dSdpParseEachParam;
		SipSdpParserParam dSdpParserParam;
		SipError error;

		if(sip_listInit(&sdpheaders, __sip_freeString, err)==SipFail)
			return SipFail;
		i=j=0;

		index = strlen(buffer);

		while (isspace((int)\
					buffer[index]) || \
				(buffer[index]=='\0'))
		{
			index--;
			flag = 1;
		}

		if (flag == 1)
		{
			buffer[index+1]='\0';
			index+=1;
		}

		while(i<=(index))
		{
			if((buffer[i]=='\r')||(buffer[i]=='\n')||\
					(buffer[i]=='\0'))
			{
				SIP_S8bit *temp;

				temp = (SIP_S8bit *) fast_memget(0,\
						sizeof(SIP_S8bit)*(i-j+2),err);
				if(temp==SIP_NULL)
					return SipFail;
				strncpy(temp,&buffer[j],i-j);
				temp[i-j] = '\0';
				temp[i-j+1] = '\0';
				if(sip_listAppend(&sdpheaders,temp,err)==SipFail)
					return SipFail;
				i++;
				j=i;
				if(i>=(index))
				{
					/* Reached end of Body 
					   break out before next check */
					break;
				}
				if((buffer[i-1]=='\r')&&(buffer[i]=='\n'))
				{
					i++;
					j++;
				}
			}
			else i++;
		}
		/* glbSdpParser state used to find if lines being 
		   parsed are part of a slMedia description	
		   state = 0 ==> parsing line for global description
		   state = 1 ==> parsing line for slMedia */

		/* Grab the core stack mutex lock now */
		dSdpParserParam.pError = &error;
		dSdpParserParam.pGCList = &GCList;

		if(sip_listInit((dSdpParserParam.pGCList), sip_freeVoid, err) == \
							SipFail)
			return SipFail;
		/* Parse each line in list */
		if(sip_initSdpMessage(&(dSdpParserParam.pSdpMessage), err)==SipFail)
			return SipFail;

		*(dSdpParserParam.pError) = E_NO_ERROR;
		dSdpParseEachParam.pSdpParam = &dSdpParserParam;
		dSdpParseEachParam.parserState = 0;
		dSdpParseEachParam.repeatState = 0;

		sip_listForEachWithData(&sdpheaders,&glbSipParserSdpHeaderParse,\
				(SIP_Pvoid *) &dSdpParseEachParam, err);
		*ppSdpMessage = dSdpParserParam.pSdpMessage;

		/* free all resources collected by GC in parsing phase */
		sip_listDeleteAll((dSdpParserParam.pGCList), err);
		sip_listDeleteAll(&sdpheaders, err);

		if(*(dSdpParserParam.pError) != E_NO_ERROR)
		{
			sip_freeSdpMessage(dSdpParserParam.pSdpMessage);
			*err = *(dSdpParserParam.pError);
			return SipFail;
		}
		return SipSuccess;
	}


#ifdef __cplusplus
}
#endif
