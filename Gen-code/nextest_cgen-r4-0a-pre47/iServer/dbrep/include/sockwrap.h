/* include sockwraph */
/* Our own header.  Tabs are set for 4 spaces, not 8 */

#ifndef	__sockwrap_h
#define	__sockwrap_h

		/* prototypes for our socket wrapper functions: see {Sec errors} */
int	 Accept(int, SA *, socklen_t *);
void	 Bind(int, const SA *, socklen_t);
int	 RSConnect(int, int, int, const SA *, socklen_t, int attempts, int ms);
void	 Getpeername(int, SA *, socklen_t *);
void	 Getsockname(int, SA *, socklen_t *);
void	 Getsockopt(int, int, int, void *, socklen_t *);
int	 Isfdtype(int, int);
void	 Listen(int, int);
int	 Poll(struct pollfd *, unsigned long, int);
ssize_t	 Readline(int, void *, size_t);
ssize_t	 Readn(int, void *, size_t);
ssize_t	 Recv(int, void *, size_t, int);
ssize_t	 Recvfrom(int, void *, size_t, int, SA *, socklen_t *);
ssize_t	 Recvmsg(int, struct msghdr *, int);
int	 Select(int, fd_set *, fd_set *, fd_set *, struct timeval *);
void	 Send(int, const void *, size_t, int);
void	 Sendto(int, const void *, size_t, int, const SA *, socklen_t);
void	 Sendmsg(int, const struct msghdr *, int);
void	 Setsockopt(int, int, int, const void *, socklen_t);
void	 Shutdown(int, int);
int	 Sockatmark(int);
int	 Socket(int, int, int);
void	 Socketpair(int, int, int, int *);
void	 Writen(int, void *, size_t);

#endif	/* __sockwrap_h */
