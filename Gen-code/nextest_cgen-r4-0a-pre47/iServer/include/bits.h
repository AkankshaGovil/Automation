#ifndef _bits_h_
#define _bits_h_

#include <sys/types.h>
#include <netinet/in.h>
#include <inttypes.h>

#define BITN(n)         			(htons(1 << (n)))
#define BIT_SET(x, n)   			((x) |= (BITN(n)))
#define BIT_RESET(x, n) 			((x) &= ~(BITN(n)))
#define BIT_TEST(x,n)   			(((x) & (BITN(n))) ? 1 : 0)
#define BIT_SETVAL(x, n, v) 		((v) ? BIT_SET(x,n) : BIT_RESET(x,n))
#define BIT_COPY(x1, n1, x2, n2) 	(BIT_SETVAL(x1, n1, BIT_TEST(x2, n2)))
#define BIT_MATCH(x1, x2, n) 		(BIT_TEST(x1, n) ^ BIT_TEST(x2, n))
#define BITS_TEST(x, m) 			(((x) & htons(m)) == htons(m))

/* x is an array of bytes */
#define BITABYTE(x, n)				(x[(n)/8])
#define BITABIT(x, n)				(1 << ((n) & 0x7))
#define BITA_SET(x, n)				( BITABYTE(x,n) |= BITABIT(x, n))
#define BITA_TEST(x, n) 			( BITABYTE(x,n) & BITABIT(x, n) )
#define BITA_RESET(x, n) 			( BITABYTE(x,n) &= ~BITABIT(x, n) )

#endif /* _bits_h_ */
