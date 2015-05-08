#ifndef __errfunc_h
#define __errfunc_h

void	 err_dump(const char *, ...);
void	 err_msg(const char *, ...);
void	 err_warn(const char *, ...);
void	 err_quit(const char *, ...);
void	 err_ret(const char *, ...);
void	 err_sys(const char *, ...);

#endif	/* __errfunc_h */
