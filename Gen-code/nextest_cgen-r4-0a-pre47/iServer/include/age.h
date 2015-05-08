#ifndef _age_h_
#define _age_h

/*
 * AGE.H
 */

/* CACHE_AGE
 * Maximum age reachable by each cache entry 
 * in seconds.
 */
#define CACHE_TIMEOUT_DEFAULT	900
#define CACHE_AGE_TIME cacheTimeout
#define RRQTTL_DEFAULT          CACHE_TIMEOUT_DEFAULT     /* secs */
#define CALLTTL_DEFAULT			CACHE_TIMEOUT_DEFAULT     /* secs */
/* CACHE_POLL
 * Time interval after which the aging daemon
 * goes thru the cache
 * in seconds.
 */
#define CACHE_POLL_TIME 15 /* For debugging */
#undef CACHE_POLL_TIME
#define CACHE_POLL_TIME 240 /* Used Value */

extern time_t LastCachePollTime, PresentTime, RemainingTime;

#define IsAged(X, present)  (((int)((present) - (X)) >= CACHE_AGE_TIME) ? 1 : 0)
#define IsGkScheduled(X, present)  (((int)((present) - (X)) >= 0) ? 1 : 0)

#endif /* _age_h_ */
