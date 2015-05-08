#ifndef _packets_h_
#define _packets_h_

#include <sys/types.h>

extern ssize_t uiio_read (int fd, char *buf, size_t min, size_t max);
extern ssize_t uiio_recvfrom (int fd, char *buf, size_t min, size_t max, struct sockaddr_in *from, int *fromlen);

int ExecuteScript(char *cmd, char *o, int n);
int ExecuteScript2(char *cmd);

extern int GisSetUdpSockOpts (int fd);
int GisSetSockOpts(int fd);
	

#endif /* _packets_h_ */
