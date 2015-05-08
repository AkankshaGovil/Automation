#include "gis.h"
#include "iwfsm.h"
#include "callutils.h"


int IWF_IsSipEvent(int event)
{
	return 1;	
}

char *sipEvent2Str(int event,char *str)
{
	sprintf(str,"%s", (char*) GetSipBridgeEvent(event));
	return str;
}

void generateTag(char *str)
{
	static int i = 1;
	sprintf(str,"%d",i++);
}

int	sip2SccErrorCode(int finalResponseCode)
{
	int rc;

	switch(finalResponseCode)
	{
		case 404:
			rc = SCC_errorDestinationUnreachable;
			break;
		case 486:
			rc = SCC_errorBusy;
			break;
		default:
			rc = 0;
			break;
	}
	return rc;

}

extern int cause2Sip[128];

int h3232SipRespCode(int cause,int error)
{
    int isdncode = cause - 1;	// actual ISDN code
    
    // Map ISDN cause code to SIP response code
    if (VALID_ISDNCODE(isdncode))
    {
        return codemap[isdncode].sipcode;
    }

    // Get response code based on call error
    return getSipCode(error);
}

// this function is called from IWF as well as Radius code to convert
// the SIP response to an ISDN cause code
int
iwfConvertSipCodeToCause(int respCode)
{
    if (VALID_SIPCODE(respCode))
    {
        return (codemap[CODEMAP_SIPINDEX(respCode)].isdncode + 1);
    }

    return 0;
}

char * sanitizeSipNumber(char *number)
{
	char *p;
	if(number)
	{
		if(p = strchr(number,';'))	
		{
			*p = '\0';
		}	
	}
	return number;
}
