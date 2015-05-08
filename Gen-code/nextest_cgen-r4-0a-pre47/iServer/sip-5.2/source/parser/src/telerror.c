/***********************************************************************
 ** FUNCTION:
 **            This file contains the error routine required by Tel.Y 

 *********************************************************************
 **
 ** FILENAME:
 ** telerror.c
 **
 ** DESCRIPTION:
 **            This file contains the error routine required by Tel.Y 
 **
 ** DATE        NAME                    REFERENCE               REASON
 ** ----        ----                    ---------              --------
 ** 
 ** 5-1-2001   Mahesh Govind			Initial creation	Support For Tel Url
 **
 ** Copyright 2001, Hughes Software Systems, Ltd.
 *********************************************************************/
#include "sipstruct.h"
#include "portlayer.h"
#include "siptrace.h"
#include "telstruct.h"
#include "telfree.h"
#include "teldecodeintrnl.h"
#include "sipparserinc.h"

extern SIP_S8bit hex_to_char(SIP_S8bit *hex_input);


/***********************************************************************
 ** FUNCTION: glbSipParserTelerror
 ***********************************************************************
 ** Function reqd by the yacc generated parser to handle parse errors.
 ***********************************************************************/
int glbSipParserTelerror
#ifdef ANSI_PROTO
(const char *s)
#else
(s)
const char *s;
#endif
{
	const char *dummy;
	dummy=s;
	SIPDEBUG("SIP DECODE MESSAGE Syntax error in Telparse\n");
	sip_error (SIP_Minor, "Syntax error while parsing Tel/related\n");
	sip_error(SIP_Minor, s);
	return 1;
}

/*****************************************************************
 ** FUNCTION: escapeCharacters
 *******************************************************************
 ** Function  used to escape a certain set of characters
 ********************************************************************/

SIP_S8bit* escapeCharacters
#ifdef ANSI_PROTO
(SIP_S8bit* input,SipError* pErr)
#else
(input,pErr)
SIP_S8bit* input;
SipError* pErr;
#endif
{
	SIP_S8bit escapedCharSet[]={'/','?',':','@','&','=','+','$',',', \
		'<','>' ,'#','%','"','{','}','|','\\','^','[',']','`',' '};
	SIP_U32bit i=0,k=0,x=0;
    SIP_U32bit string_length;
    SIP_S8bit *output = SIP_NULL;
    string_length = strlen(input);
	output=(SIP_S8bit*)fast_memget(0,((string_length*3)+1),pErr);
	if(output==SIP_NULL)
		return SIP_NULL;
	strcpy(output,"");

	for(i=0,x=0;i<string_length;i++,x++)
	{
      output[x]=input[i];
      for(k=0;k<sizeof(escapedCharSet);k++)
      {
        if(input[i]==escapedCharSet[k])
        {
          HSS_SNPRINTF(&output[x],4,"%%%x",input[i]);
          x=x+2;
          break;
        }
      }
	}
	output[x]='\0';
	return(output);
}

/*****************************************************************
** FUNCTION: hex_to_char
*******************************************************************
** Function  converts a hex char to a literal
********************************************************************/
	
SIP_S8bit hex_to_char
#ifdef ANSI_PROTO 
(SIP_S8bit *hex_input)
#else
(hex_input)
SIP_S8bit *hex_input;
#endif
{
  	SIP_S8bit digit;

    digit = (hex_input[0] >= 'A' ? ((hex_input[0] & 0xdf) - 'A')+10 \
        : (hex_input[0] - '0'));
  	digit *= 16;
  	digit += (hex_input[1] >= 'A' ? ((hex_input[1] & 0xdf) - 'A')+10 \
        : (hex_input[1] - '0'));
  	return(digit);
}

/***********************************************************************
** FUNCTION:unescapeCharacters 
************************************************************************
** Function that unescapes the escaped charartes and converts them into
** literal
***********************************************************************/

SIP_S8bit* unescapeCharacters
#ifdef ANSI_PROTO 
(SIP_S8bit* url)
#else
(url)
SIP_S8bit* url;
#endif
{
  	int x,y;

   	for (x=0,y=0; url[y]; ++x,++y) 
  	{
      if((url[x] = url[y]) == '%') 
      {
        url[x] = hex_to_char(&url[y+1]);
        y+=2;
      }
  	}
  	url[x] = '\0';
  	return(url);
}
