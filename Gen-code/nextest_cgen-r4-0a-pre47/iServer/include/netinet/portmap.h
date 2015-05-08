#ifndef __PORTMAP_H
#define __PORTMAP_H

#include <sys/types.h>
#include <netinet/in.h>
#include <inttypes.h>

#define	MAX_LIFS_PER_QIF	256		// The maximum # of logical interfaces
									// per physical interface


#define		IPF_PM_PORT_CLOSED		0
#define		IPF_PM_PORT_OPEN		1

#define BITN(n)						(htons(1 << (n)))
#define BIT_SET(x, n)				((x) |= (BITN(n)))
#define BIT_RESET(x, n)				((x) &= ~(BITN(n)))
#define BIT_TEST(x,n)				(((x) & (BITN(n))) ? 1 : 0)
#define BIT_SETVAL(x, n, v)			((v) ? BIT_SET(x,n) : BIT_RESET(x,n))
#define BIT_COPY(x1, n1, x2, n2)	(BIT_SETVAL(x1, n1, BIT_TEST(x2, n2)))
#define BIT_MATCH(x1, x2, n)		(BIT_TEST(x1, n) ^ BIT_TEST(x2, n))
#define BITS_TEST(x, m)				(((x) & htons(m)) == htons(m))

// x is an array of bytes

#define BITABYTE(x, n)  			(x[(n)/8])
#define BITABIT(x, n)				(1 << ((n) & 0x7))
#define BITA_SET(x, n)				( BITABYTE(x,n) |= BITABIT(x, n))
#define BITA_TEST(x, n)				( BITABYTE(x,n) & BITABIT(x, n) )
#define BITA_RESET(x, n)			( BITABYTE(x,n) &= ~BITABIT(x, n) )

//
// logical interface port map
//
//   There is one of these structures for each
//   logical interface on a physical interface.
//   The LifMap_t is allocated for a logical 
//   interface when the first hole is added for it.
//

typedef	struct	_LifMap
{
	struct _LifMap *	next;			// pointer to next LifMap_t
										// chained off of a qif->Lif[xx].
										// There will only be more than one
										// if hash for two logical interface
										// ip addresses misses

	uint32_t			ip_addr;		// ip address of logical interface
										// (network order)

	char				tcp_pm[8192];	// portmap for tcp
	char				udp_pm[8192];	// portmap for udp

} LifMap_t;

extern void		clearLifMaps(	LifMap_t ** LifMapPtrArray,
								char *		ifname );

extern void		freeLifMaps(	LifMap_t **	lifMapPtrArray,
								char *		ifname );

extern int		addLifPort(		LifMap_t **		lifMapPtrArray,
								char *			ifname,
								uint32_t 		ipa,
								uint16_t		port,
								int				proto );

extern int		remLifPort(		LifMap_t **		lifMapPtrArray,
								char *			ifname,
								uint32_t 		ipa,
								uint16_t		port,
								int				proto );

extern int		checkLifPort(	LifMap_t **		lifMapPtrArray,
								uint32_t		ipa,			
								uint16_t		port,
								int				proto );


#endif
