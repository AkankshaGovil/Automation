#ifndef __IPFILTER_H
#define __IPFILTER_H


#include <unistd.h>
#include <sys/types.h>
#include <sys/systeminfo.h>
#include <sys/time.h>
#include <sys/uio.h>
#include <sys/stat.h>
#include <sys/file.h>
#include <fcntl.h>
#include <stdio.h>
#include <netdb.h>
#include <errno.h>
#include <stdlib.h>
#include <pthread.h>
#include <signal.h>
#include <sys/protosw.h>
#include <sys/socket.h>
#include <sys/filio.h>
#include <string.h>

#include <net/if.h>

#if SOLARIS_REL >= 8
	#include <netinet/ip6.h>
#endif

#ifdef _PATH_KMEM
	#define KMEM	_PATH_KMEM
#else
	#define KMEM	"/dev/kmem"
#endif

#include <stdarg.h>

#define _SYS_VARARGS_H

#ifdef _IPFILTER_SYS_INCLUDE_

#if SOLARIS_REL > 8 && !defined(KERNEL)

	/*   Work around iserver avl.h and system avl.h name collision 
 	 *   since avl.h has been added in Solaris 9
 	 */

	extern void *memset(void*, int, size_t);

	#define _AVL_H
	#define _AVL_IMPL_H

	#ifndef _LP64

		struct sys_avl_node {
			struct sys_avl_node *avl_child[2];	/* left/right children */
			struct sys_avl_node *avl_parent;	/* this node's parent */
			unsigned short avl_child_index;	/* my index in parent's avl_child[] */
			short avl_balance;		/* balance value: -1, 0, +1 */
		};

		#define	AVL_XPARENT(n)		((n)->avl_parent)
		#define	AVL_SETPARENT(n, p)	((n)->avl_parent = (p))

		#define	AVL_XCHILD(n)		((n)->avl_child_index)
		#define	AVL_SETCHILD(n, c)	((n)->avl_child_index = (unsigned short)(c))

		#define	AVL_XBALANCE(n)		((n)->avl_balance)
		#define	AVL_SETBALANCE(n, b)	((n)->avl_balance = (short)(b))

	#else /* _LP64 */

	  /*
	   * for 64 bit machines, avl_pcb contains parent pointer, balance and child_index
	   * values packed in the following manner:
	   *
	   * |63                                  3|        2        |1          0 |
	   * |-------------------------------------|-----------------|-------------|
	   * |      avl_parent hi order bits       | avl_child_index | avl_balance |
	   * |                                     |                 |     + 1     |
	   * |-------------------------------------|-----------------|-------------|
	   *
	   */
	   struct sys_avl_node {
		   struct sys_avl_node *avl_child[2];	/* left/right children nodes */
		   uintptr_t avl_pcb;		/* parent, child_index, balance */
	   };

	   /*
	    * macros to extract/set fields in avl_pcb
	    *
	    * pointer to the parent of the current node is the high order bits
	    */
		#define	AVL_XPARENT(n)		((struct avl_node *)((n)->avl_pcb & ~7))
		#define	AVL_SETPARENT(n, p)						\
			((n)->avl_pcb = (((n)->avl_pcb & 7) | (uintptr_t)(p)))

	   /*
	 	* index of this node in its parent's avl_child[]: bit #2
	 	*/
		#define	AVL_XCHILD(n)		(((n)->avl_pcb >> 2) & 1)
		#define	AVL_SETCHILD(n, c)						\
			((n)->avl_pcb = (uintptr_t)(((n)->avl_pcb & ~4) | ((c) << 2)))

	   /*
	 	* balance indication for a node, lowest 2 bits. A valid balance is
	 	* -1, 0, or +1, and is encoded by adding 1 to the value to get the
	 	* unsigned values of 0, 1, 2.
	 	*/
		#define	AVL_XBALANCE(n)		(((n)->avl_pcb & 3) - 1)
		#define	AVL_SETBALANCE(n, b)						\
			((n)->avl_pcb = (uintptr_t)((((n)->avl_pcb & ~3) | ((b) + 1))))

	#endif /* _LP64 */

	typedef struct sys_avl_node avl_node_t;

	struct sys_avl_tree {
		struct sys_avl_node *avl_root;  /* root node in tree */
		int (*avl_compar)(const void *, const void *);
		size_t avl_offset;      /* offsetof(type, avl_link_t field) */
		ulong_t avl_numnodes;       /* number of nodes in the tree */
		size_t avl_size;        /* sizeof user type struct */
	};

	typedef struct sys_avl_tree avl_tree_t;

#endif

#include <netinet/ip_compat.h>
#include <netinet/ip_fil.h>
#include <netinet/ip_nat.h>

#else

typedef struct frpcmp 
{
	int32_t		frp_cmp;
	uint16_t	frp_port;
	uint16_t	frp_top;
} frpcmp_t;

typedef struct frtuc
{
	int8_t		ftu_tcpfm;
	int8_t		ftu_tcpf;
	frpcmp_t	ftu_src;
	frpcmp_t	ftu_dst;
} frtuc_t;

union	i6addr
{
	uint32_t	i6[4];
	struct in_addr	in4;
};

typedef struct fr_ip
{
	uint32_t		fi_v:4;
	uint32_t		fi_fl:4;
	uint32_t		fi_tos:8;
	uint32_t		fi_ttl:8;
	uint32_t		fi_p:8;
	union i6addr	fi_src;
	union i6addr	fi_dst;
	uint32_t		fi_optmsk;
	uint16_t		fi_secmsk;
	uint16_t		fi_auth;
} fr_ip_t;

#endif

#ifndef APR_LABELLEN
#define APR_LABELLEN 16
#endif

#if ( !defined( STRUCT_IPNAT64 ) ) 

typedef struct  ipnat64
{
    char    	in_next[8];
    char    	in_rnext[8];
    char    	in_prnext[8];
    char    	in_mnext[8];
    char    	in_pmnext[8];
    char    	in_ifp[8];
    char    	in_apr[8];
    char    	in_space[8];
    uint32_t   	in_use;
    uint32_t   	in_hits;
    struct  	in_addr in_nextip;
    uint16_t 	in_pnext;
    uint16_t 	in_ippip;   /* IP #'s per IP# */
    uint32_t	in_flags;   /* From here to in_dport must be reflected */
    uint16_t	in_spare;
    uint16_t	in_ppip;    /* ports per IP */
    uint16_t	in_port[2]; /* correctly in IPN_CMPSIZ */
    struct		in_addr in_in[2];
    struct		in_addr in_out[2];
    struct		in_addr in_src[2];
    struct		frtuc   in_tuc;
    uint32_t	in_age[2];  /* Aging for NAT entries. Not for TCP */
    uint32_t	in_redir; /* 0 if it's a mapping, 1 if it's a hard redir */
    char		in_ifname[IFNAMSIZ];
    char		in_plabel[APR_LABELLEN];    /* proxy label */
    char		in_p;   /* protocol */
} ipnat64_t;

#define SIOCADNAT_N _IOW('r', 100, struct ipnat64 )
#define SIOCRMNAT_N _IOW('r', 101, struct ipnat64 )

#endif

#if ( !defined( STRUCT_FRENTRY64 ) ) 

typedef struct  frdest64    
{
    char    fd_ifp[8];
    union   i6addr  fd_ip6;
    char    fd_ifname[LIFNAMSIZ];
    char    fd_mp[8];           /* cache resolver for to/dup-to */
} frdest64_t;

typedef struct  frentry64
{
    char			fr_next[8];
    char			fr_grp[8];
    long long   	fr_ref;     /* reference count - for grouping */
    char			fr_ifas[4][8];
    /*
     * These are only incremented when a packet  matches this rule and
     * it is the last match
     */
    char        	fr_hits[8];
    char        	fr_bytes[8];
    /*
     * Fields after this may not change whilst in the kernel.
     */
    struct  fr_ip   fr_ip;
    struct  fr_ip   fr_mip; /* mask structure */


    u_short			fr_icmpm;   /* data for ICMP packets (mask) */
    u_short			fr_icmp;

    u_int			fr_age[2];  /* aging for state */
    frtuc_t			fr_tuc;
    uint32_t		fr_group;   /* group to which this rule belongs */
    uint32_t		fr_grhead;  /* group # which this rule starts */
    uint32_t		fr_flags;   /* per-rule flags && options (see below) */
    u_int			fr_skip;    /* # of rules to skip */
    u_int			fr_loglevel;    /* syslog log facility + priority */
    u_int			unused1;
    char			fr_func[8]; /* call this function */

    int fr_sap;     /* For solaris only */

    u_char			fr_icode;   /* return ICMP code */
    char			fr_ifnames[4][LIFNAMSIZ];

    struct frdest64	fr_tif; /* "to" interface */
    struct frdest64	fr_dif; /* duplicate packet interfaces */
    u_int			fr_cksum;   /* checksum on filter rules for performance */
    u_int			unused2;
} frentry64_t;

#define    SIOCADAFR_N _IOW('r', 100, struct frentry64 )
#define    SIOCRMAFR_N _IOW('r', 101, struct frentry64 )

#endif

#include "ifs.h"
#include "srvrlog.h"

#include "serverp.h"

#include "lsconfig.h"
#include "log.h"
#include "firewallcontrol.h"

#ifndef FALSE
#define FALSE 0
#endif
 
#ifndef TRUE
#define TRUE 1
#endif

#define	IPNAT_DEVICE	"/dev/ipnat"
#define	IPF_DEVICE		"/dev/ipf"

#define	MAX_MEDIA_IFACE	8
#define	MAX_INTERFACES	8

extern void			IpfilterInit( void );

extern int		IpfilterOpenHole(	uint32_t	protocol,
									uint32_t	mediaIpAddress,
									uint16_t	mediaPort,
									eFCETrafficDirection direction,
									uint32_t 	sessionId,
									uint32_t 	sessionIdLeg2,
									int numTranslations,
									uint32_t *	returnedMediaIpAddress,
									uint16_t *	returnedMediaPort,
									int       	allocatePrivatePorts );

extern int		IpfilterReopenHole(	uint32_t	protocol,
									uint32_t	mediaIpAddress,
									uint16_t	mediaPort,
									eFCETrafficDirection direction,
									uint32_t 	sessionId,
									uint32_t 	sessionIdLeg2,
									int numTranslations,
									uint32_t *	returnedMediaIpAddress,
									uint16_t *	returnedMediaPort,
									int       	allocatePrivatePorts );

extern int		IpfilterCloseHole(	uint32_t holeId );

extern int		IpfilterCloseSession( uint32_t sessionId );

extern int32_t		IpfilterShutdown( void );

extern int32_t		IpfilterReconfig( void );

extern uint32_t		IpfilterGetPublicAddress(	uint32_t addr );
extern uint32_t		IpfilterGetPrivateAddress(   void );

extern void   		IpfilterServerActive( void );

extern void   		IpfilterServerInactive( void );

#include "readconfig.h"
#include "fceutils.h"
#include "trace.h"

#ifdef sparc
	#define IPNAT_T		ipnat64_t
	#define FRENTRY_T	frentry64_t
#else
	#define IPNAT_T		ipnat_t
	#define FRENTRY_T	frentry_t
#endif


#endif
