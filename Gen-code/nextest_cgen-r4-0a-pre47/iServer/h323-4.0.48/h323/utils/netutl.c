#ifdef __cplusplus
extern "C" {
#endif



/*

NOTICE:
This document contains information that is proprietary to RADVISION LTD.
No part of this publication may be reproduced in any form whatsoever without
written prior approval by RADVISION LTD.

RADVISION LTD reserves the right to revise this publication and make changes
without obligation to notify any person of such revisions or changes.

*/

/****************************************************************************

  netutl.c  --  Network utilities

  Module Author: Oz Solomonovich
  This Comment:  26-Dec-1996

  Abstract:      IP and network byte order conversion utilities.

  Platforms:     All platforms supported.  The network byte order conversion
                 utilities determine the host byte order at run-time.

  Known Bugs:    None.

****************************************************************************/


#include <netutl.h>

#define CHAR(i) ((char *)(&i))

BOOL isMulticastIP(UINT32 ip)
{
#define IN_CLASSD(i) (((i) & 0xf0000000ul) == 0xe0000000ul)
    ip=rv_ntohl(ip);
    return IN_CLASSD(ip);
}

UINT32 rv_htonl(UINT32 host)
{
    UINT32 order = 0x00010203, net=0;
    int i;

    for (i = 0; i < 4; i++)
    {
        int index = CHAR(order)[i];
        CHAR(net)[i] = CHAR(host)[index];
    }

    return net;
}

UINT32 rv_ntohl(UINT32 net)
{
    UINT32 order = 0x00010203, host=0;
    int i;

    for (i = 0; i < 4; i++)
    {
        int index = CHAR(order)[i];
        CHAR(host)[index] = CHAR(net)[i];
    }

    return host;
}

UINT16 rv_htons(UINT16 host)
{
    UINT16 order = 0x0001, net=0;
    int i;

    for (i = 0; i < 2; i++)
    {
        int index = CHAR(order)[i];
        CHAR(net)[i] = CHAR(host)[index];
    }

    return net;
}

UINT16 rv_ntohs(UINT16 net)
{
    UINT16 order = 0x0001, host=0;
    int i;

    for (i = 0; i < 2; i++)
    {
        int index = CHAR(order)[i];
        CHAR(host)[index] = CHAR(net)[i];
    }

    return host;
}


UINT32 ip_to_uint32(const char *cp)
{
    const char *p = cp;
    UINT32 addr[4], maxvals[4];
    UINT32 net_addr = 0, last_val;
    int n = 0, i = 0;

    memset(maxvals, 0xFF, sizeof maxvals);

    for (;;)
    {
        if (*p == 0 || *p == '.' || n >= 4)
            return CONVERSION_ERROR;

        addr[n]  = 0;
        last_val = 0;

        if (n)
        {
            maxvals[n] = maxvals[n - 1] >> 8;
            maxvals[n - 1] = 255;
        }

        do
        {
            if (!((*p >= '0')  &&  (*p <= '9')))
                return CONVERSION_ERROR;

            addr[n] *= 10;
            addr[n] += (*p - '0');
            if (addr[n] < last_val)  /* overflow check  */
                return CONVERSION_ERROR;
            last_val = addr[n];

            p++;

            if (*p == 0) goto stop;
        } while (*p != '.');

        p++;
        n++;
    }

stop:

    for (i = 0; i <= n; i++)
        if (addr[n] > maxvals[n])
            return CONVERSION_ERROR;

        net_addr = addr[n];
        if (n >= 1) net_addr += (addr[0] << 24);
        if (n >= 2) net_addr += (addr[1] << 16);
        if (n >= 3) net_addr += (addr[2] <<  8);

        return rv_htonl(net_addr);
}


void uint32_to_ip(UINT32 addr, char *ip)
{
    char ip_buffer[16];
    int curr_num, curr_dig;
    UINT32 num;
    char *p = &(ip_buffer[14]);

    addr = rv_ntohl(addr);
    strcpy(ip_buffer, "xxx.xxx.xxx.xxx");
    for (curr_num = 0; curr_num <= 3; curr_num++)
    {
        num = addr & 0xFF;
        addr >>= 8;
        for (curr_dig = 0; curr_dig <= 2; curr_dig++)
        {
            *p = (char)(((char)(num % 10) + '0'));
            num /= 10;
            p--;
            if (num == 0)
                break;
        }
        if (curr_num < 3)
            *p-- = '.';
    }
    strcpy(ip,(p+1));
}


#ifdef __cplusplus
}
#endif



