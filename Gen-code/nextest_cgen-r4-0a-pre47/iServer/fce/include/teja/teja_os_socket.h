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

#ifndef TEJA_OS_SOCKET_H
#define TEJA_OS_SOCKET_H

#define TEJA_INVALID_SOCKET -1

typedef int TejaSocket;

#ifndef TEJA_EMBEDDED

typedef socklen_t TejaSocklenT;
typedef struct sockaddr TejaSockaddr;
typedef struct sockaddr_in TejaSockaddrIn;

#define teja_accept                accept
#define teja_bind                  bind
#define teja_close_socket          close
#define teja_connect               connect
#define teja_listen                listen
#define teja_open_socket(reserved) socket (AF_INET, SOCK_STREAM, 0)
#define teja_read_from_socket      recv
#define teja_write_to_socket       send

int  teja_is_data_available_on_socket (TejaSocket s);

#endif /* TEJA_EMBEDDED*/

#endif /* TEJA_OS_SOCKET_H */
