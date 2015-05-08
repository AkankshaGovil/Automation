
/*
 * IP address manipulation functions.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "ipc.h"
#include "ipstring.h"

char * 
IPtostring (IPaddr ipaddress)
{
  static char outstring[16];

  sprintf(outstring,"%d.%d.%d.%d",ipaddress.uc[0],ipaddress.uc[1],
				ipaddress.uc[2],ipaddress.uc[3]);
  return outstring;
}


char * 
ULIPtostring (unsigned long ipaddress)
{
  static char outstring[16];

  sprintf(outstring,"%u.%u.%u.%u",
			(unsigned int)(ipaddress&0xff000000)>>24,
			(unsigned int)(ipaddress&0x00ff0000)>>16,
			(unsigned int)(ipaddress&0x0000ff00)>>8,
			(unsigned int)(ipaddress&0x000000ff));
  return outstring;
}

unsigned long 
StringToIp(char *str)
{
    char tmpstr[20];
    char *p;
    int i = 0;
    unsigned long ip = 0;

    strncpy(tmpstr,str,20);
    tmpstr[19] = '\0';
    p = strtok(tmpstr,".");

    while(p && i <4)
    {
        ip = ip <<8;
        ip|= atoi(p) & 0xff;
        p = strtok(NULL,".");
        i++;
    }
    return ip;

}
