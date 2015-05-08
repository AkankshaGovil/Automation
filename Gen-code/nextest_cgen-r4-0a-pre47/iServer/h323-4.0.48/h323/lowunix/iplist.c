#ifdef __cplusplus
extern "C" {
#endif



/*
***********************************************************************************

NOTICE:
This document contains information that is proprietary to RADVISION LTD..
No part of this publication may be reproduced in any form whatsoever without
written prior approval by RADVISION LTD..

RADVISION LTD. reserves the right to revise this publication and make changes
without obligation to notify any person of such revisions or changes.

***********************************************************************************
*/

/*
  ipList.h

  Before using it you should attach Ip addresses:

  struct in_addr ip;
  char ipstr[18], estr[16], char[8];
  int mask, i;

  ..........

  get IP address to 'ip' variable
  and IP's MASK into 'mask' variable
  'i' is ip address number
  ........

  inet_ntoa_b ( ip, ipStr );
  sprintf (eStr,"ei%d",i);
  sprintf (name,"string%d",i);
  if ((usrNetIfAttach (eStr, ipStr)) < 0)
     printf ("\nusrNetIfAttach error");
  if ((usrNetIfConfig (eStr, ipStr,name,mask)) < 0)
     printf ("\nusrNetIfConfig error");

*/

#ifdef __CADUL__
#include <pna.h>
#define __Iunistd
#endif

#include <rvinternal.h>
#include "iplist.h"
#define IPLIST_MAX_LENGTH 10

UINT32 ipList[IPLIST_MAX_LENGTH];
UINT32 *ipListPtrs[IPLIST_MAX_LENGTH];

#if ( defined(__VXWORKS__) || defined(__PSOSPNA__) ) 
static BYTE LOCAL_IP_BUF[] = {0x7f, 0x00, 0x00, 0x01};
#endif 


#ifdef __VXWORKS__
#include <sys/types.h>
#include <vxWorks.h>
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <net/if.h>
#include <netinet/in_var.h>
#include <sockLib.h>
#include <routeLib.h>
#include <net/protosw.h>
#include <inetLib.h>
#endif  /* __VXWORKS__ */



#ifdef __VXWORKS__

UINT32 **lpGetIps(void)
{
  FAST struct in_ifaddr *ia = 0;
  struct sockaddr_in *tmpAddr;
  int i=0;

  for (ia=in_ifaddr; ia != NULL; ia=ia->ia_next){
    tmpAddr = IA_SIN(ia);
    if ( (tmpAddr->sin_addr).s_addr != (*(UINT32*)LOCAL_IP_BUF)) {
      ipList[i]= (tmpAddr->sin_addr).s_addr;
      ipListPtrs[i]=&ipList[i];
      i++;
    }
  }
  ipListPtrs[i]=NULL;
  return ipListPtrs;
}


#elif defined ( __PSOSPNA__ )/* __VXWORKS__ */


UINT32 **lpGetIps(void)
{
#define satosin(sa)     ((struct sockaddr_in *)(sa))
  int s, i;
  long rc;
  struct ifreq ifr;

/* create any type of socket */
  s = socket(AF_INET, SOCK_DGRAM, 0);

  for (ifr.ifr_ifno = 1, rc = 0, i = 0; !rc; ifr.ifr_ifno++)  /* Start from 1, not from 0 - from debugging */
      {
       satosin(&ifr.ifr_addr)->sin_family = AF_INET;
       rc = ioctl(s, SIOCGIFADDR, (char *)&ifr);  /* Get the IP address of the pNA+ interface */
       if ( !rc )
          {
           if ( satosin(&ifr.ifr_addr)->sin_addr.s_addr != (*(UINT32*)LOCAL_IP_BUF))
        {
                 ipList[i]= satosin(&ifr.ifr_addr)->sin_addr.s_addr;
                 ipListPtrs[i]=&ipList[i];
                 i++;
                }
      }
      }

  close(s);

  ipListPtrs[i]=NULL;
  return ipListPtrs;
}
/*-----------------1/16/99 5:15PM-------------------
 * This function is absent in pSOS and will be implemented
 * --------------------------------------------------*/
unsigned long inet_addr(char * addr)
{
    return (unsigned long) ip_to_uint32(addr);
}

/*-----------------1/16/99 5:19PM-------------------
 * This function is not implemented in pSOS and should be implemented for the specific hardware platform
 * In this particular implementation will construct host name out of IP address
 * This function should be replaced in the customer's implementation
 * --------------------------------------------------*/
int gethostname(char * outhostname, int nLen)
{
#define satosin(sa)     ((struct sockaddr_in *)(sa))
  int s, i;
  long rc;
  struct ifreq ifr;
  char  hostname[9];
  BYTE * p;

/* create any type of socket */
  s = socket(AF_INET, SOCK_DGRAM, 0);

  for (ifr.ifr_ifno = 1, rc = 0, i = 0; !rc; ifr.ifr_ifno++)
      {
       satosin(&ifr.ifr_addr)->sin_family = AF_INET;
       rc = ioctl(s, SIOCGIFADDR, (char *)&ifr);  /* Get the IP address of the pNA+ interface */
       if ( !rc )
          if ( satosin(&ifr.ifr_addr)->sin_addr.s_addr != (*(UINT32*)LOCAL_IP_BUF))
         break;
      }

  close(s);

    if ( !rc )                  /* address was successfully retreived */
       {
        p = (BYTE *)&satosin(&ifr.ifr_addr)->sin_addr.s_addr;
        sprintf(hostname, "%02X%02X%02X%02X", p[0], p[1], p[2], p[3]);
        hostname[8] = 0;
        strncpy(outhostname, hostname, (size_t)nLen);   /* AL - 06/16/99 - (size_t) */
        return 0;
       }
    else
       return -1;
}
#else

int RV_dummyIpList = 0;

#endif  /* __VXWORKS__ / __PSOSPNA__*/


#ifdef __cplusplus
}
#endif



