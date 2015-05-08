/************************************************************************/
/*                                                                      */
/* Copyright (C) Teja Technologies, Inc. 1998-2002.                     */
/*                                                                      */
/* All Rights Reserved.                                                 */
/*                                                                      */
/* This software is the property of Teja Technologies, Inc.  It is      */
/* furnished under a specific licensing agreement.  It may be used or   */
/* copied only under terms of the licensing agreement.                  */
/*                                                                      */
/* For more information, contact info@teja.com                          */
/*                                                                      */
/************************************************************************/

#ifndef TEJA_OS_TCPIP_H
#define TEJA_OS_TCPIP_H

#ifndef TEJA_EMBEDDED

typedef struct in_addr TejaInAddr;

#define TEJA_AF_INET AF_INET
#define TEJA_INADDR_ANY INADDR_ANY

#define teja_get_host_name gethostname
#define teja_htonl htonl
#define teja_htons htons

int teja_get_ip_address_by_name_or_address (char *name, int *addr);

#endif /* TEJA_EMBEDDED*/

#endif /* TEJA_OS_TCPIP_H */
