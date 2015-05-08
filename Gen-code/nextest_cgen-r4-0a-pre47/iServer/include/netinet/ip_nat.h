/*
 * Copyright (C) 1995-2001 by Darren Reed.
 *
 * See the IPFILTER.LICENCE file for details on licencing.
 *
 * @(#)ip_nat.h	1.5 2/4/96
 * $Id: ip_nat.h,v 1.1.1.1.2.4 2004/07/21 20:59:31 sturt Exp $
 */

#ifndef	__IP_NAT_H__
#define	__IP_NAT_H__

#ifdef SOLARIS
#include <netinet/if_ether.h>
#endif

#ifndef SOLARIS
#define SOLARIS (defined(sun) && (defined(__svr4__) || defined(__SVR4)))
#endif

#if defined(__STDC__) || defined(__GNUC__)
#define	SIOCADNAT		_IOWR('r', 60, struct ipnat *)
#define	SIOCRMNAT		_IOW('r', 61, struct ipnat *)
#define	SIOCGNATS		_IOWR('r', 62, struct natstat *)
#define	SIOCGNATL		_IOWR('r', 63, struct natlookup *)
#define	SIOCADNAT_N		_IOWR('r', 100, struct ipnat64 )
#define	SIOCRMNAT_N		_IOW('r', 101, struct ipnat64 )
#define	SIOCMODNAT		_IOW('r', 102, struct ipnat *)
#define	SIOCMODNAT_N	_IOW('r', 103, struct ipnat64 *)
#define	SIOCSPING		_IOW('r', 104, u_int)
#define	SIOCRELATEDN	_IOW('r', 105, struct dnat_relate *)
#define	SIOCRELATEDN_N	_IOW('r', 106, struct dnat_relate64 *)
#else
#define	SIOCADNAT		_IOWR(r, 60, struct ipnat *)
#define	SIOCRMNAT		_IOW(r, 61, struct ipnat *)
#define	SIOCGNATS		_IOWR(r, 62, struct natstat *)
#define	SIOCGNATL		_IOWR(r, 63, struct natlookup *)
#define	SIOCADNAT_N		_IOWR(r, 100, struct ipnat64 )
#define	SIOCRMNAT_N		_IOW(r, 101, struct ipnat64 )
#define	SIOCMODNAT		_IOW(r, 102, struct ipnat *)
#define	SIOCMODNAT_N	_IOW(r, 103, struct ipnat64 *)
#define	SIOCSPING		_IOW(r, 104, u_int)
#define	SIOCRELATEDN	_IOW(r, 105, struct dnat_relate *)
#define	SIOCRELATEDN_N	_IOW(r, 106, struct dnat_relate64 * )
#endif

#define	LARGE_NAT	

			// define LARGE_NAT if you're setting up a system to NAT
			// LARGE numbers of networks/hosts - i.e. in the
			// hundreds or thousands.  In such a case, you should
			// also change the RDR_SIZE and NAT_SIZE below to more
			// appropriate sizes.  The figures below were used for
			// a setup with 1000-2000 networks to NAT.

#ifndef	NAT_SIZE
# define	NAT_SIZE	127
#endif
#ifndef	RDR_SIZE
# define	RDR_SIZE	127
#endif
#ifndef	HOSTMAP_SIZE
# define	HOSTMAP_SIZE	127
#endif
#ifndef	NAT_TABLE_SZ
# define	NAT_TABLE_SZ	127
#endif

#ifdef	LARGE_NAT

#undef	NAT_SIZE
#undef	RDR_SIZE
#undef	NAT_TABLE_SZ
#undef	HOSTMAP_SIZE
#define	NAT_SIZE		16383
#define	RDR_SIZE		16383
#define	NAT_TABLE_SZ	16383
#define	HOSTMAP_SIZE	8191
#endif
#ifndef	APR_LABELLEN
#define	APR_LABELLEN	16
#endif
#define	NAT_HW_CKSUM	0x80000000

//#define	DEF_NAT_AGE	1200     /* 10 minutes (600 seconds) */

#define	DEF_NAT_AGE	30     /* 15 seconds */

struct ap_session;

typedef	struct	nat
{
	u_long				nat_age;
	int					nat_flags;
	u_32_t				nat_sumd[2];
	u_32_t				nat_ipsumd;
	void *				nat_data;

	struct ap_session *	nat_aps;		// proxy session

	struct frentry *	nat_fr;			// filter rule ptr if appropriate

	struct in_addr		nat_outip;		// local ip address outside
										// (network order)

	struct in_addr		nat_inip;		// local ip address inside
										// (network order)

	struct in_addr		nat_oip;		// remote source ip address

	u_short				nat_outport;	// local port outside (network order)
	u_short				nat_inport;		// local port inside (network order)

	u_short				nat_oport;		// other port - i.e. - remote port
										//  nat_dir == OUTBOUND
										//     nat_oport is remote dst port
										//                  (network order)
										//
										//  nat_dir == INBOUND
										//     nat_oport is remote src port
										//                  (network order)

	u_char				nat_p;			// protocol

	int					nat_dir;		// NAT direction

	char				nat_ifname[IFNAMSIZ];
										// name of interface to which nat
										// applies

	void *				nat_ifp;		// ill_t pointer for physical
										// interface to which nat applies
										// (lower layer interface pointer)

	struct ipnat *		ipnat_ptr;		// pointer back to the ipnat_t 
										// entry which caused this
										// active nat_t to be added

	U_QUAD_T			nat_pkts;
	U_QUAD_T			nat_bytes;

	u_int				nat_drop[2];

	u_char				nat_tcpstate[2];

	struct hostmap *	nat_hm;

	struct nat *		nat_next;		// pointer to next nat_t off
										// nat_instance chain

	// nat_table[][] hash chaining mechanism
	// defined by nat_hnext[] and nat_phnext[]

	struct nat *		nat_hnext[2];	// pointer to next nat_t
										//    one for the outlookup chain
										//    and one for the inlookup chain

	struct nat **		nat_phnext[2];	// pointer to previous nat_t's hnext
										//    one for the outlookup chain
										//    and one for the inlookup chain

	#if SOLARIS || defined(__sgi)
		kmutex_t	nat_lock;
	#endif

} nat_t;

// cached info for fastrouting
// of nat media

typedef struct	fastr_info
{
	void *					src_ill;	// ill_t for source outbound interface
	void *					src_ire;	// ire_t for source outbound interface
	void *					src_qif;	// qif_t for source outbound interface

	void *					dst_ire;	// ire_t for destination ip address

	struct	ether_header	eh;			// pre-constructed ethernet header			
} fastr_info_t;

typedef struct	fastr_info64
{
	char					src_ill[8];	// ill_t for source outbound interface
	char					src_ire[8];	// ire_t for source outbound interface
	char					src_qif[8];	// qif_t for source outbound interface

	char					dst_ire[8];	// ire_t for destination ip address

	struct	ether_header	eh;			// pre-constructed ethernet header			
} fastr_info64_t;

typedef struct	leg_key
{
	uint32_t	hv;
	u_long		ipnat_ptr;
} leg_key_t;

typedef struct	leg_key64
{
	uint32_t	hv;
	char		ipnat_ptr[8];
} leg_key64_t;

typedef	struct	dnat_relate
{
	leg_key_t	data_nat_key;	// Key for leg of call whose real src ip addr
								// will be used as the destination of packets
								// sent by the other leg

	leg_key_t	other_key;		// Key for leg whose dest ip addr and port
								// will be modified based on packets flowing
								// from the data_nat_leg.
} dnat_relate_t;

typedef	struct	dnat_relate64
{
	leg_key64_t	data_nat_key;	// Key for leg of call whose real src ip addr
								// will be used as the destination of packets
								// sent by the other leg

	leg_key64_t	other_key;		// Key for leg whose dest ip addr and port
								// will be modified based on packets flowing
								// from the data_nat_leg.
} dnat_relate64_t;

typedef	struct	ipnat
{
	//
	// nat_list navigation :
	//
	//	All the ipnat_t structures allocated in nat_ioctl() via add
	//	functions are chained off a doubly linked list. nat_flist
	//	is the forward anchor and nat_blist is the backward anchor.
	//	The in_lnext field is used to traverse this list, forward.
	//	The in_lprev field is used to traverse this list, backward.
	//	The last entries in_next is NULL.
	//

	struct ipnat *	in_lnext;				// address of next ipnat_t
											// on nat_flist list

	struct ipnat *	in_lprev;				// address of previous ipnat_t
											// on nat_blist list

	//
	// rdr_rules navigation (REDIRECT ipnat_t entries) :
	//
	//	rdr_rules is an array of lists of ipnat_t structures used
	//	for redirection of incoming packets destination info.
	//	The in_rnext and in_prnext fields are used to chain
	//	ipnat_t entries on a given one of these lists.
	//

	struct ipnat *	in_rnext;				// address of next ipnat_t
											// on a rdr_rules hash list
											// for a REDIRECT ipnat_t

	struct ipnat **	in_prnext;				// address of previous ipnat_t's
											// in_rnext which points to us,
											// on this rdr_rules hash list

	//
	// nat_rules navigation (MAP or MAPBLOCK ipnat_t entries) :
	//
	//	nat_rules is an array of lists of ipnat_t structures used
	//	for mapping of outgoing packets source info. The in_mnext and
	//	in_pmnext fields are used to chain ipnat_t entries on a
	//	given one of these lists.
	//

	struct ipnat *	in_mnext;				// address of next ipnat_t
											// on a nat_rules hash list
											// for a MAP ipnat_t

	struct ipnat **	in_pmnext;				// address of previous ipnat_t's
											// in_mnext which points to us,
											// on this nat_rules hash list

	void *			in_ifp;					// ill_t * of interface if found
											// -1 if not found

	void *			in_apr;					// pointer to application
											// proxy information if a proxy
											// label was specified and found

	u_long			in_space;
	u_int			in_use;
	u_int			in_hits;

	struct in_addr	in_nextip;


	u_short			in_ippip;				// IP#'s per IP#

	int				in_fri_inited;			// 0  - if in_fri is not inited
											// 1  - if in_fri is inited

	fastr_info_t	in_fri;					// fastrouting info

	u_32_t			in_flags;				// Usage flags

	u_short			in_ppip;				// ports per IP

	struct in_addr	in_out[2];				// outside ip address and mask
											// in_out specifies what the
											// ip address should be on
											// the outside of the specified
											// interface for the ipnat_t to
											// match. 
											//
											// ( network order )
											//
											// in_out[0] ip address - in_outip
											// in_out[1] ip mask    - in_outmsk

	u_short			in_port[2];				// outside destination port range
											// prior to DNAT 
											//
											// ( network order )
											//
											//       in_port[0]  -  in_pmin
											//       in_port[0]  -  in_pmax
											//
											// For us both ports are the same
											//   ( in_pmin == in_pmax )

	struct in_addr	in_in[2];				// inside ip address and mask.
											// in_in specifies what the
											// destination ip address
											// should be on the inside of
											// the specified interface 
											// after the DNAT operation.
											//
											// (network order)
											//
											// in_in[0] ip address - in_inip
											// in_in[1] ip mask    - in_inmsk

	u_short			in_pnext;				// new destination port for redirect
											// after DNAT
											//
											// (network order)

	int				in_redir;				// 0 if it's a mapping
											// 1 if it's a hard redir

	struct in_addr	in_map_srcip;			// source ip for redirect
											// as packet passes out last
											// interface
											// (network order)

	u_short			in_map_port;			// source port for redirect
											// as packet passes out last
											// interface (network order)

	char			in_ifname[IFNAMSIZ];	// incoming interface name
											// to which redirect applies

	char			in_ifname_out[IFNAMSIZ];// outgoing interface name
											// to which redirect applies

	char			in_p;					// protocol

	struct frtuc	in_tuc;					// port comparitor info

	leg_key_t		in_legkey;			 	// Used during ADNAT (add redirect) processing to
											// relate call legs for data nat capability.
											//
											//  Usage Notes:
											//
											//   Each ADNAT call returns a data nat 
											//   legkey_t value which should be used 
											//   in the new RELATEDN ioctl call to 
											//   relate two call legs if data nat
											//   traversal is needed.

	struct in_addr	in_dnat_dstip;			// datanat dst ip of incoming packets
	u_short			in_dnat_dstport;		// datanat dst port of incoming packets


	struct in_addr	in_real_srcip;			// saved src ip of data prior to natting
	u_short			in_real_srcport;		// saved src port of data prior to natting

	leg_key_t		in_othleg_private;	 	// leg key for other leg of call
											// NOTE : used in kernel only !!

	struct in_addr	in_src[2];				// in_src[0] ip address
											// in_src[1] ip mask

	u_int			in_age[2];				// Aging for NAT entries.
											// Not for TCP

} ipnat_t;

typedef	struct	ipnat64
{
	//
	// nat_list navigation :
	//
	//	All the ipnat_t structures allocated in nat_ioctl() via add
	//	functions are chained off the nat_list. The in_next field
	//	is used to traverse this list. The last entries in_next is NULL.
	//

	char			in_lnext[8];			// address of next ipnat_t
											// on nat_flist list

	char			in_lprev[8];			// address of previous ipnat_t
											// on nat_blist list

	//
	// rdr_rules navigation (REDIRECT ipnat_t entries) :
	//
	//	rdr_rules is an array of lists of ipnat_t structures used
	//	for redirection of incoming packets destination info.
	//	The in_rnext and in_prnext fields are used to chain
	//	ipnat_t entries on a given one of these lists.
	//

	char			in_rnext[8];			// address of next ipnat_t
											// on a rdr_rules hash list
											// for a REDIRECT ipnat_t

	char			in_prnext[8];			// address of previous ipnat_t's
											// in_rnext which points to us,
											// on this rdr_rules hash list

	//
	// nat_rules navigation (MAP or MAPBLOCK ipnat_t entries) :
	//
	//	nat_rules is an array of lists of ipnat_t structures used
	//	for mapping of outgoing packets source info. The in_mnext and
	//	in_pmnext fields are used to chain ipnat_t entries on a
	//	given one of these lists.
	//

	char			in_mnext[8];			// address of next ipnat_t
											// on a nat_rules hash list
											// for a MAP ipnat_t

	char			in_pmnext[8];			// address of previous ipnat_t's
											// in_mnext which points to us,
											// on this nat_rules hash list

	char			in_ifp[8];				// ill_t * of interface if found
											// -1 if not found

	char			in_apr[8];				// pointer to application proxy
											// information if a proxy
											// label was specified and found

	char			in_space[8];
	u_int			in_use;
	u_int			in_hits;

	struct	in_addr	in_nextip;

	u_short			in_ippip;				// IP #'s per IP#

	int				in_fri_inited;			// 0  - if in_fri is not inited
											// 1  - if in_fri is inited

	fastr_info64_t	in_fri;					// fastrouting info

	u_32_t			in_flags;				// Usage flags

	u_short			in_ppip;				// ports per IP

	struct in_addr	in_out[2];				// outside ip address and mask
											// in_out specifies what the
											// ip address should be on
											// the outside of the specified
											// interface for the ipnat_t to
											// match. 
											//
											// ( network order )
											//
											// in_out[0] ip address - in_outip
											// in_out[1] ip mask    - in_outmsk

	u_short			in_port[2];				// outside destination port range
											// prior to DNAT 
											//
											// ( network order )
											//
											//       in_port[0]  -  in_pmin
											//       in_port[0]  -  in_pmax
											//
											// For us both ports are the same
											//   ( in_pmin == in_pmax )

	struct in_addr	in_in[2];				// inside ip address and mask.
											// in_in specifies what the
											// destination ip address
											// should be on the inside of
											// the specified interface 
											// after the DNAT operation.
											//
											// (network order)
											//
											// in_in[0] ip address - in_inip
											// in_in[1] ip mask    - in_inmsk

	u_short			in_pnext;				// new destination port for redirect
											// after DNAT
											//
											// (network order)

	int				in_redir;				// 0 if it's a mapping
											// 1 if it's a hard redir

	struct in_addr	in_map_srcip;			// source ip for redirect
											// as packet passes out last
											// interface
											// (network order)

	u_short			in_map_port;			// source port for redirect
											// as packet passes out last
											// interface (network order)

	char			in_ifname[IFNAMSIZ];	// incoming interface name
											// to which redirect applies

	char			in_ifname_out[IFNAMSIZ];// outgoing interface name
											// to which redirect applies

	char			in_p;					// protocol

	struct frtuc	in_tuc;					// port comparitor info

	leg_key64_t		in_lkey;			 	// Used during ADNAT (add redirect) processing to
											// relate call legs for data nat capability.
											//
											//  Usage Notes:
											//
											//   Each ADNAT call returns a data nat 
											//   legkey_t value which should be used 
											//   in the new RELATEDN ioctl call to 
											//   relate two call legs if data nat
											//   traversal is needed.
					
	struct in_addr	in_dnat_dstip;			// datanat dst ip of incoming packets
	u_short			in_dnat_dstport;		// datanat dst port of incoming packets

	struct in_addr	in_real_srcip;			// saved src ip of data prior to natting
	u_short			in_real_srcport;		// saved src port of data prior to natting

	leg_key_t		in_othleg_private;	 	// leg key for other leg of call
											// NOTE : used in kernel only !! not by user

	struct in_addr	in_src[2];				// in_src[0] ip address
											// in_src[1] ip mask


	u_int			in_age[2];				// Aging for NAT entries.
											// Not for TCP

} ipnat64_t;

#define STRUCT_IPNAT64	1

#define	in_pmin		in_port[0]	/* Also holds static redir port */
#define	in_pmax		in_port[1]
#define	in_nip		in_nextip.s_addr
#define	in_inip		in_in[0].s_addr
#define	in_inmsk	in_in[1].s_addr
#define	in_outip	in_out[0].s_addr
#define	in_outmsk	in_out[1].s_addr
#define	in_srcip	in_src[0].s_addr
#define	in_srcmsk	in_src[1].s_addr
#define	in_scmp		in_tuc.ftu_scmp
#define	in_dcmp		in_tuc.ftu_dcmp
#define	in_stop		in_tuc.ftu_stop
#define	in_dtop		in_tuc.ftu_dtop
#define	in_sport	in_tuc.ftu_sport
#define	in_dport	in_tuc.ftu_dport

#define	NAT_OUTBOUND	0
#define	NAT_INBOUND		1

#define	NAT_MAP			0x01
#define	NAT_REDIRECT	0x02
#define	NAT_BIMAP		(NAT_MAP|NAT_REDIRECT)
#define	NAT_MAPBLK		0x04

/* 0x100 reserved for FI_W_SPORT */
/* 0x200 reserved for FI_W_DPORT */
/* 0x400 reserved for FI_W_SADDR */
/* 0x800 reserved for FI_W_DADDR */
/* 0x1000 reserved for FI_W_NEWFR */

#define	MAPBLK_MINPORT	1024	/* don't use reserved ports for src port */
#define	USABLE_PORTS	(65536 - MAPBLK_MINPORT)

#define	IPN_CMPSIZ	(offsetof(ipnat_t, in_tuc) - offsetof(ipnat_t, in_out))

typedef	struct	natlookup
{
	struct	in_addr	nl_inip;
	struct	in_addr	nl_outip;
	struct	in_addr	nl_realip;
	int	nl_flags;
	u_short	nl_inport;
	u_short	nl_outport;
	u_short	nl_realport;
} natlookup_t;


typedef struct  nat_save   
{
	void	*ipn_next;
	struct	nat	ipn_nat;
	struct	ipnat	ipn_ipnat;
	struct	frentry ipn_fr;
	int	ipn_dsize;
	char	ipn_data[4];
} nat_save_t;

#define	ipn_rule	ipn_nat.nat_fr

typedef	struct	natget
{
	void	*ng_ptr;
	int	ng_sz;
} natget_t;


typedef	struct	hostmap
{
	struct	hostmap	*hm_next;
	struct	hostmap	**hm_pnext;
	struct	ipnat	*hm_ipnat;
	struct	in_addr	hm_realip;
	struct	in_addr	hm_mapip;
	int	hm_ref;
} hostmap_t;


typedef	struct	natstat
{
	u_long			ns_mapped[2];
	u_long			ns_rules;
	u_long			ns_added;
	u_long			ns_expire;
	u_long			ns_inuse;
	u_long			ns_logged;
	u_long			ns_logfail;
	u_long			ns_memfail;
	u_long			ns_badnat;
	nat_t **		ns_table[2];
	hostmap_t **	ns_maptable;
	ipnat_t	*		ns_list;
	void *			ns_apslist;
	u_int			ns_nattab_sz;
	u_int			ns_rultab_sz;
	u_int			ns_rdrtab_sz;
	u_int			ns_hostmap_sz;
	nat_t *			ns_instances;
	u_int			ns_wilds;
	u_long			ns_inadd;
	u_long			ns_outadd;
} natstat_t;

#define	IPN_ANY			0x00000000
#define	IPN_TCP			0x00000001
#define	IPN_UDP			0x00000002
#define	IPN_DELETE		0x00000004
#define	IPN_ICMPERR		0x00000008
#define	IPN_AUTOPORTMAP	0x00000010
#define	IPN_IPRANGE		0x00000020
#define	IPN_FILTER		0x00000040
#define	IPN_SPLIT		0x00000080

#define IPN_DATANAT						0x00000400
#define IPN_DNAT_UPDATE_OTHER_LEG		0x00000800

#define	IPN_NOTSRC		0x00080000
#define	IPN_NOTDST		0x00100000
#define	IPN_FRAG		0x00200000
#define	IPN_XMAP		0x00400000


#define	IPN_TCPUDP		(IPN_TCP|IPN_UDP)
#define	IPN_RF			(IPN_TCPUDP|IPN_DELETE|IPN_ICMPERR)

#define	IPN_USERFLAGS	(	IPN_TCPUDP|IPN_AUTOPORTMAP|IPN_IPRANGE|\
							IPN_SPLIT|IPN_FILTER|IPN_NOTSRC|\
							IPN_NOTDST|IPN_FRAG|IPN_XMAP)

typedef	struct	natlog
{
	struct	in_addr		nl_origip;
	struct	in_addr		nl_outip;
	struct	in_addr		nl_inip;
	u_short				nl_origport;
	u_short				nl_outport;
	u_short				nl_inport;
	u_short				nl_type;
	int					nl_rule;
	U_QUAD_T			nl_pkts;
	U_QUAD_T			nl_bytes;
	u_char				nl_p;
} natlog_t;

#define	NL_NEWMAP	NAT_MAP
#define	NL_NEWRDR	NAT_REDIRECT
#define	NL_NEWBIMAP	NAT_BIMAP
#define	NL_NEWBLOCK	NAT_MAPBLK
#define	NL_FLUSH	0xfffe
#define	NL_EXPIRE	0xffff

#define	NAT_HASH_FN(k,l,m)	(((k) + ((k) >> 12) + l) % (m))

#define	LONG_SUM(in)	(((in) & 0xffff) + ((in) >> 16))

#define	CALC_SUMD(s1, s2, sd) {										\
				uint32_t ls1 = s1;  /* dont fuck with orig s1 */	\
				uint32_t ls2 = s2;  /* dont fuck with orig s2 */	\
																	\
			    (ls1) = ((ls1) & 0xffff) + ((ls1) >> 16);			\
			    (ls2) = ((ls2) & 0xffff) + ((ls2) >> 16);			\
			    /* Do it twice */									\
			    (ls1) = ((ls1) & 0xffff) + ((ls1) >> 16);			\
			    (ls2) = ((ls2) & 0xffff) + ((ls2) >> 16);			\
			    /* Because ~1 == -2, We really need ~1 == -1 */ 	\
			    if ((ls1) > (ls2)) (ls2)--;							\
			    (sd) = (ls2) - (ls1);								\
			    (sd) = ((sd) & 0xffff) + ((sd) >> 16); }

#define NAT_SYSSPACE		0x80000000
#define NAT_LOCKHELD		0x40000000

extern	u_int		ipf_nattable_sz;
extern	u_int		ipf_natrules_sz;
extern	u_int		ipf_rdrrules_sz;
extern	int			fr_nat_lock;
extern	u_long		fr_defnatage;
extern	u_long		fr_defnaticmpage;
extern	nat_t **	nat_table[2];
extern	nat_t *		nat_instances;
extern	ipnat_t	**	nat_rules;
extern	ipnat_t	**	rdr_rules;
extern	ipnat_t	*	nat_flist;
extern	ipnat_t	*	nat_blist;
extern	natstat_t	nat_stats;

#if defined(__OpenBSD__)
	extern	void	nat_ifdetach __P((void *));
#endif

extern	void		ip_natsync(	void *	ifp	);

extern	int			nat_ioctl(	caddr_t	data,
								int		cmd,
								int		mode );

extern	int			nat_init(	void );

extern	nat_t	*	nat_new(	fr_info_t *	fin,
								ip_t *		ip,
								ipnat_t *	np,
								nat_t **	natsave,
								u_int		flags,
								int			direction );

extern	nat_t	*	nat_outlookup(	fr_info_t *		fin,
									u_int			flags,
									u_int			proto_id,
									struct in_addr	src,
									struct in_addr	dst,
									int				rw,
									int				logit,
									int				invert_lookup,
									int *			found_inverted );

extern	nat_t	*	nat_lookupredir(	natlookup_t *	np );

extern	nat_t	*	nat_icmplookup(		ip_t *			ip,
										fr_info_t *		fin,
										int				dir );

extern	nat_t	*	nat_icmp(	ip_t *		ip,
								fr_info_t *	fin,
								u_int *		nflags,
								int			dir );

extern	int			nat_clearlist(	void	);


extern	int			ip_natout(	ip_t *		ip,
								fr_info_t *	fin,
								int			logit,
								int			invert_lookup );

extern	int			ip_natout_mr(	ip_t *		ip,
									fr_info_t *	fin,
									int			locked,
									ipnat_t *	ipnat_ptr );

extern	int			ip_natin(	ip_t *		ip,
								fr_info_t *	fin );

extern	int			ip_natin_mr(	ip_t *			ip,
									fr_info_t *		fin,
									fastr_info_t *	fastr_info,
									ipnat_t **		ipnat_ptr,
									mblk_t **		mp );

extern	void		ip_natunload(void);
extern	void		ip_natexpire(void);

extern	void		nat_log(	struct nat *	nat,
								u_int			type );

extern	void		fix_outcksum(	fr_info_t *		fin,
									u_short *		sum_ptr,
									u_32_t			sumd );

#endif /* __IP_NAT_H__ */
