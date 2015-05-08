/* include libfunch */
/* Our own header.  Tabs are set for 4 spaces, not 8 */

#ifndef	__libfunc_h
#define	__libfunc_h

	/* prototypes for our own library functions */
int	 connect_nonb(int, const SA *, socklen_t, int);
int	 connect_timeo(int, const SA *, socklen_t, int);
void	 daemon_init(int fdflag);
#define CLOSE_ALL_FD	0
#define	LEAVE_IO_FD		1
void	 daemon_inetd(const char *, int);
void	 dg_cli(FILE *, int, const SA *, socklen_t);
void	 dg_echo(int, SA *, socklen_t);
char	*gf_time(void);
void	 heartbeat_cli(int, int, int);
void	 heartbeat_serv(int, int, int);
struct addrinfo *host_serv(const char *, const char *, int, int);
int	 	inet_srcrt_add(char *, int);
u_char  *inet_srcrt_init(void);
void	 inet_srcrt_print(u_char *, int);
char   **my_addrs(int *);
int		 hreadable_timeo(int, int);
ssize_t	 readline(int, void *, size_t);
ssize_t	 readn(int, void *, size_t);
ssize_t	 read_fd(int, void *, size_t, int *);
ssize_t	 recvfrom_flags(int, void *, size_t, int *, SA *, socklen_t *,
		 struct in_pktinfo *);
Sigfunc *signal_intr(int, Sigfunc *);
int	 sock_bind_wild(int, int);
int	 sock_cmp_addr(const SA *, const SA *, socklen_t);
int	 sock_cmp_port(const SA *, const SA *, socklen_t);
int	 sock_get_port(const SA *, socklen_t);
void	 sock_set_addr(SA *, socklen_t, const void *);
void	 sock_set_port(SA *, socklen_t, int);
void	 sock_set_wild(SA *, socklen_t);
char	*sock_ntop(const SA *, socklen_t, char *, int);
char	*sock_ntop_host(const SA *, socklen_t);
int	 sockfd_to_family(int);
void str_echo(int);
void str_cli(FILE *, int);
int	 tcp_connect(const char *, const char *);
int	 tcp_listen(const char *, const char *, socklen_t *);
int  tcp_serv(in_port_t, int);
void tv_sub(struct timeval *, struct timeval *);
int	 udp_client(const char *, const char *, void **, socklen_t *);
int	 udp_connect(const char *, const char *);
int	 udp_server(const char *, const char *, socklen_t *);
int	 writable_timeo(int, int);
ssize_t	 writen(int, const void *, size_t);
ssize_t	 write_fd(int, void *, size_t, int);

/* My functions */
int	 unix_serv(const char *, int );
void	msleep(uint32_t milliseconds);

unsigned short	in_cksum(unsigned short *, int);

#endif /* __libfunc_h */
