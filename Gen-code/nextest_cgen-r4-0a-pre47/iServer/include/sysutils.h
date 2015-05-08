#ifndef _SYSUTILS_H_
#define _SYSUTILS_H_

extern void sys_utils_init( void );

extern int sys_popen( const char *command, int exec_shell_flag);
extern int sys_pclose( int fd );

#endif
