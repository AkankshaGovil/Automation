#ifndef _timedef_h_
#define _timedef_h_
#include <sys/time.h>

// struct timeval is defined in <sys/time.h>
typedef struct timeval timedef;

// void timedef_sub( timedef *tp1, timedef *tp2, timedef *res) returns (*tp1 - *tp2) in res
// void timedef_add( timedef *tp1, timedef *tp2, timedef *res) returns (*tp1 + *tp2) in res
// int timedef_cmp( timedef *tp1, timedef *tp2) returns (1, -1, 0) for (*tp1 > *tp2, *tp1 < *tp2, *tp1 = *tp2)
// void timedef_cur( timedef *tp) fills the current in *tp
// int timedef_iszero(timedef *tp) checks whether (*tp == 0)
// time_t timedef_sec(timedef *tp) returns the seconds value of (*tp)
// time_t timedef_usec(timedef *tp) returns the micro-seconds value of (*tp)
// time_t timedef_msec(timedef *tp) returns the milliseconds value of (*tp)
// timedef_msec does not check for out of bound exception, should be used for time much less than 180 days.

#define timedef_sub(tp1, tp2, res) 	do { \
										(res)->tv_sec = (tp1)->tv_sec - (tp2)->tv_sec; \
										(res)->tv_usec = (tp1)->tv_usec - (tp2)->tv_usec; \
										if ( (res)->tv_usec < 0 ) { \
											(res)->tv_usec += 1000000; \
											(res)->tv_sec -= 1; \
										} \
									} while(0)

#define timedef_add(tp1, tp2, res) 	do { \
										(res)->tv_sec = (tp1)->tv_sec + (tp2)->tv_sec; \
										(res)->tv_usec = (tp1)->tv_usec + (tp2)->tv_usec; \
										if ( (res)->tv_usec > 1000000 ) { \
											(res)->tv_usec -= 1000000; \
											(res)->tv_sec += 1; \
										} \
									} while(0)

#define timedef_cmp(tp1, tp2) 	( (tp1)->tv_sec > (tp2)->tv_sec ? 1 : \
									( (tp1)->tv_sec < (tp2)->tv_sec ? -1 : \
										( (tp1)->tv_usec > (tp2)->tv_usec ? 1 : \
											( (tp1)->tv_usec < (tp2)->tv_usec ? -1 : \
												0 \
											) \
										) \
									) \
								)

#define timedef_cur(tp) 	gettimeofday(tp, NULL)

#define	timedef_iszero(tp)	(((tp)->tv_sec == 0) && ((tp)->tv_usec == 0)) 	
#define timedef_rndsec(tp)  ((tp)->tv_usec >= 500000 ? (tp)->tv_sec + 1 : (tp)->tv_sec)
#define timedef_sec(tp)		((tp)->tv_sec)
#define timedef_msec(tp)	((time_t)((tp)->tv_usec/1000))
#define timedef_usec(tp)	((tp)->tv_usec)
#endif /* _timedef_h_ */
