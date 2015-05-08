#ifndef _generic_h_
#define _generic_h_

#ifndef NETOID_LINUX
#ifndef SUNOS
#include <string.h>
#define bzero(s, n)	memset(s, 0, n)
#define bcopy(s, d, n)	memcpy(d, s, n)
#define write(sock, buf, n) send(sock, (char *)buf, n, 0)
#define read(sock, buf, n) recv(sock, (char *)buf, n, 0)
#endif
#ifdef SUNOS
typedef struct sockaddr PROTOID_SOCKADDR; 
#else
typedef struct sockaddr_in PROTOID_SOCKADDR; 
#endif
typedef char PROTOID_PKT;
#else
#include <sys/socket.h>
#include <netdb.h>
#include <net/if.h>
#include <netinet/in.h>
#include <sys/ioctl.h>
typedef struct sockaddr PROTOID_SOCKADDR; 
typedef unsigned char PROTOID_PKT;
#endif /* NETOID_LINUX */

#define LCHAR(x)	((x)/4+1)

#endif	/* _generic_h_ */
