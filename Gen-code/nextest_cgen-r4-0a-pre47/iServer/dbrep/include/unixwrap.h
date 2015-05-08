/* include unixwraph */
/* Our own header.  Tabs are set for 4 spaces, not 8 */

#ifndef	__unixwrap_h
#define	__unixwrap_h

		/* prototypes for our Unix wrapper functions: see {Sec errors} */
void	*Calloc(size_t, size_t);
void	 Close(int);
void	 Dup2(int, int);
int	 Fcntl(int, int, int);
void	 Gettimeofday(struct timeval *, void *);
int	 Ioctl(int, int, void *);
pid_t	 Fork(void);
void	*Malloc(size_t);
void	 Free(void *);
void	 Mktemp(char *);
void	*Mmap(void *, size_t, int, int, int, off_t);
int	 Open(const char *, int, mode_t);
void	 Pipe(int *fds);
int	 Mkfifo(const char *, mode_t);
ssize_t	 Read(int, void *, size_t);
void	 Sigaddset(sigset_t *, int);
void	 Sigdelset(sigset_t *, int);
void	 Sigemptyset(sigset_t *);
void	 Sigfillset(sigset_t *);
int	 Sigismember(const sigset_t *, int);
void	 Sigpending(sigset_t *);
void	 Sigprocmask(int, const sigset_t *, sigset_t *);
char	*Strdup(const char *);
long	 Sysconf(int);
void	 Sysctl(int *, u_int, void *, size_t *, void *, size_t);
void	 Unlink(const char *);
pid_t	 Wait(int *);
pid_t	 Waitpid(pid_t, int *, int);
void	 Write(int, void *, size_t);
long 	 GetIpAddr(char *ifname);
int		 System(const char *);

#endif	/* __unixwrap_h */
