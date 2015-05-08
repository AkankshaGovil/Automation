/* include libwraph */
/* Our own header.  Tabs are set for 4 spaces, not 8 */

#ifndef	__libwrap_h
#define	__libwrap_h

	/* prototypes for our own library wrapper functions */
void	 Connect_timeo(int, const SA *, socklen_t, int);
struct addrinfo *Host_serv(const char *, const char *, int, int);
const char		*Inet_ntop(int, const void *, char *, size_t);
void			 Inet_pton(int, const char *, void *);
char			*If_indextoname(unsigned int, char *);
unsigned int		 If_nametoindex(const char *);
struct if_nameindex	*If_nameindex(void);
char   **My_addrs(int *);
ssize_t	 Read_fd(int, void *, size_t, int *);
int	 Readable_timeo(int, int);
ssize_t	 Recvfrom_flags(int, void *, size_t, int *, SA *, socklen_t *,
		 struct in_pktinfo *);
Sigfunc *RSSignal(int, Sigfunc *);
Sigfunc *Signal_intr(int, Sigfunc *);
int	 Sock_bind_wild(int, int);
char	*Sock_ntop(const SA *, socklen_t);
char	*Sock_ntop_r(const SA *, socklen_t, char *, int);
char	*Sock_ntop_host(const SA *, socklen_t);
int	 Sockfd_to_family(int);
int	 Tcp_connect(const char *, const char *);
int	 Tcp_listen(const char *, const char *, socklen_t *);
int	 Udp_client(const char *, const char *, void **, socklen_t *);
int	 Udp_connect(const char *, const char *);
int	 Udp_server(const char *, const char *, socklen_t *);
ssize_t	 Write_fd(int, void *, size_t, int);
int	 Writable_timeo(int, int);

#endif	/* __libwrap_h */
