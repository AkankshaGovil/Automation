#ifndef _ADDRTYPE_H_
#define _ADDRTYPE_H_

#define IPADDR 1
#define HOSTNAME 2
#define DN 3
#define ABS_DN 4

int DetermineAddrType(char *host);

#endif /* _ADDRTYPE_H_ */
