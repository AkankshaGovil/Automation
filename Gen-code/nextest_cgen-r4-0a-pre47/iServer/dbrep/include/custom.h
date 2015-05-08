/* include customh */

#ifndef	__custom_h
#define	__custom_h

#ifndef	INADDR_NONE
#define	INADDR_NONE	0xffffffff	/* should have been in <netinet/in.h> */
#endif

/* Older resolvers do not have gethostbyname2() */
#ifndef	HAVE_GETHOSTBYNAME2
#define	gethostbyname2(host,family)		gethostbyname((host))
#endif

/* The structure returned by recvfrom_flags() */
#ifndef NETOID_LINUX
struct in_pktinfo{
  struct in_addr	ipi_addr;	/* dst IPv4 address */
  int				ipi_ifindex;/* received interface index */
};
#endif // NETOID_LINUX

/* We need the newer CMSG_LEN() and CMSG_SPACE() macros, but few
   implementations support them today.  These two macros really need
    an ALIGN() macro, but each implementation does this differently. */
#ifndef	CMSG_LEN
#define	CMSG_LEN(size)		(sizeof(struct cmsghdr) + (size))
#endif
#ifndef	CMSG_SPACE
#define	CMSG_SPACE(size)	(sizeof(struct cmsghdr) + (size))
#endif

/* Posix.1g requires the SUN_LEN() macro but not all implementations DefinE
   it (yet).  Note that this 4.4BSD macro works regardless whether there is
   a length field or not. */
#ifndef	SUN_LEN
# define	SUN_LEN(su) \
	(sizeof(*(su)) - sizeof((su)->sun_path) + strlen((su)->sun_path))
#endif

/* Posix.1g renames "Unix domain" as "local IPC".
   But not all systems DefinE AF_LOCAL and AF_LOCAL (yet). */
#ifndef	AF_LOCAL
#define AF_LOCAL	AF_UNIX
#endif
#ifndef	PF_LOCAL
#define PF_LOCAL	PF_UNIX
#endif

/* Posix.1g requires that an #include of <poll.h> DefinE INFTIM, but many
   systems still DefinE it in <sys/stropts.h>.  We don't want to include
   all the streams stuff if it's not needed, so we just DefinE INFTIM here.
   This is the standard value, but there's no guarantee it is -1. */
#ifndef INFTIM
#define INFTIM          (-1)    /* infinite poll timeout */
#ifdef	HAVE_POLL_H
#define	INFTIM_UNPH		/* tell unpxti.h we defined it */
#endif
#endif

/* Following could be derived from SOMAXCONN in <sys/socket.h>, but many
   kernels still #define it as 5, while actually supporting many more */
#define	LISTENQ		1024	/* 2nd argument to listen() */

/* Miscellaneous constants */
#define	MAXLINE		4096	/* max text line length */
#define	MAXSOCKADDR  	128	/* max socket address structure size */
#define	BUFFSIZE	8192	/* buffer size for reads and writes */

#if 0
/* Define some port number that can be used for client-servers */
#define	SERV_PORT	 9877		/* TCP and UDP client-servers */
#define	SERV_PORT_STR	"9877"		/* TCP and UDP client-servers */
#define	UNIXSTR_PATH	"/tmp/unix.str"	/* Unix domain stream cli-serv */
#define	UNIXDG_PATH	"/tmp/unix.dg"	/* Unix domain datagram cli-serv */
#endif

/* Following shortens all the type casts of pointer arguments */
#define	SA	struct sockaddr

#define	FILE_MODE	(S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH)
					/* default file access permissions for new files */
#define	DIR_MODE	(FILE_MODE | S_IXUSR | S_IXGRP | S_IXOTH)
					/* default permissions for new directories */

typedef	void	Sigfunc(int);	/* for signal handlers */

#define	min(a,b)	((a) < (b) ? (a) : (b))
#define	max(a,b)	((a) > (b) ? (a) : (b))

#ifndef	HAVE_ISFDTYPE_PROTO
int		 isfdtype(int, int);
#endif

#ifndef	HAVE_PSELECT_PROTO
int		 pselect(int, fd_set *, fd_set *, fd_set *,
				 const struct timespec *, const sigset_t *);
#endif

#ifndef	HAVE_SOCKATMARK_PROTO
int		 sockatmark(int);
#endif

#endif	/* __custom_h */
