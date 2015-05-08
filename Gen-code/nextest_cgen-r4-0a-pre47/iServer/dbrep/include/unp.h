/* include unph */
/* Our own header.  Tabs are set for 4 spaces, not 8 */

#ifndef	__unp_h
#define	__unp_h

#include	"config.h"	/* configuration options for current OS */
				/* "../config.h" is generated by configure */

/* If anything changes in the following list of #includes, must change
   acsite.m4 also, for configure's tests. */

#include	<sys/types.h>	/* basic system data types */
#include	<sys/socket.h>	/* basic socket definitions */
#include	<sys/time.h>	/* timeval{} for select() */
#include	<time.h>	/* timespec{} for pselect() */
#include	<netinet/in.h>	/* sockaddr_in{} and other Internet defns */
#include	<arpa/inet.h>	/* inet(3) functions */
#include	<errno.h>
#include	<fcntl.h>	/* for nonblocking */
#include	<netdb.h>
#include	<signal.h>
#include	<stdio.h>
#include	<stdlib.h>
#include	<string.h>
#include	<sys/stat.h>	/* for S_xxx file mode constants */
#include	<sys/uio.h>	/* for iovec{} and readv/writev */
#include	<unistd.h>
#include	<sys/wait.h>
#include	<sys/un.h>	/* for Unix domain sockets */

# include	<sys/select.h>	/* for convenience */
# include	<poll.h>	/* for convenience */
# include	<strings.h>	/* for convenience */

/* Three headers are normally needed for socket/file ioctl's:
 * <sys/ioctl.h>, <sys/filio.h>, and <sys/sockio.h>.
 */
#include	"nxioctl.h"
# include	<pthread.h>

/* These headers are present in the package */
#include 	"custom.h"
#include 	"../libmcast/include/mcast.h"
#include 	"libfunc.h"
#include 	"libwrap.h"
#include 	"unixwrap.h"
#include 	"iowrap.h"
#include 	"sockwrap.h"
#include 	"errfunc.h"
#include	"sighdl.h"
#include	"unpthread.h"

/* Add malloc */
#include 	"malloc.h"

#endif	/* __unp_h */