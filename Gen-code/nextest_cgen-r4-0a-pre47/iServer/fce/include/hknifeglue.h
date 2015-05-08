#ifndef _HKNIFEGLUE_H_
#define _HKNIFEGLUE_H_

#include <asm/types.h>

#define IPF_ITYPE_MEDIA_ROUTE		1
#define IPF_ITYPE_PUBLIC			4
#define IPF_ITYPE_PRIVATE			2


#define	NAT_REDIRECT	0x02
#define	IPN_XMAP		0x00400000
#define	IPN_UDP			0x00000002



	union	i6addr
	{
		//TODO: Change for solaris u_32_t	i6[4];
		uint32_t i6[4];
		struct	in_addr	in4;
	};

typedef	struct	fr_clearbm_entry
{
	char			ifname[LIFNAMSIZ];
} fr_clearbm_entry_t;

typedef	struct	fr_interface_type
{
	char			ifname[LIFNAMSIZ];
	uint32_t		itype;
	uint16_t		largest_anon_udp_port;
} fr_interface_type_t;


typedef	struct	fr_pm_entry
{
	uint32_t		lif_ip;				// logical interface ip address
										// (network order)	

	uint16_t		port;				// port	
										// (network order)				

	char			ifname[LIFNAMSIZ];	// name of physical interface

	uint32_t		proto;				// IPPROTO_TCP or IPPROTO_UDP

} fr_pm_entry_t;


typedef struct  leg_key
{
        uint32_t        hv;
        u_long          ipnat_ptr;
} leg_key_t;


typedef struct  ether_header{
	__u8	ether_dhost[6];
	__u8	ether_shost[6];
	__u16	ether_type;
} ether_header_t;

typedef	struct	dnat_relate
{
	leg_key_t	data_nat_key;	// Key for leg of call whose real src ip addr
								// will be used as the destination of packets
								// sent by the other leg

	leg_key_t	other_key;		// Key for leg whose dest ip addr and port
								// will be modified based on packets flowing
								// from the data_nat_leg.
} dnat_relate_t;

	
typedef struct	fastr_info
{
	void *					src_ill;	// ill_t for source outbound interface
	void *					src_ire;	// ire_t for source outbound interface
	void *					src_qif;	// qif_t for source outbound interface

	void *					dst_ire;	// ire_t for destination ip address

	struct	ether_header	eh;			// pre-constructed ethernet header			
} fastr_info_t;

typedef struct frpcmp 
{
  int32_t       frp_cmp;
  uint16_t      frp_port;
  uint16_t      frp_top;
} frpcmp_t;


typedef struct fr_ip
{
  uint32_t      fi_v:4;
  uint32_t      fi_fl:4;
  uint32_t      fi_tos:8;
  uint32_t      fi_ttl:8;
  uint32_t      fi_p:8;
  union i6addr  fi_src;
  union i6addr  fi_dst;
  uint32_t      fi_optmsk;
  uint16_t      fi_secmsk;
  uint16_t      fi_auth;
} fr_ip_t;

typedef struct frtuc
{
  int8_t        ftu_tcpfm;
  int8_t        ftu_tcpf;
  frpcmp_t      ftu_src;
  frpcmp_t      ftu_dst;
} frtuc_t;

	
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

	//TODO: Define for solaris u_32_t			in_flags;				// Usage flags
	uint32_t			in_flags;				// Usage flags

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

	struct in_addr	in_in[2];				// inside ip address and mask.

#define	in_pmin		in_port[0]	/* Also holds static redir port */
#define	in_pmax		in_port[1]
#define	in_nip		in_nextip.s_addr
#define	in_inip		in_in[0].s_addr
#define	in_inmsk	in_in[1].s_addr
#define	in_outip	in_out[0].s_addr
#define	in_outmsk	in_out[1].s_addr
#define	in_srcip	in_src[0].s_addr
#define	in_srcmsk	in_src[1].s_addr

#endif //#define _HKNIFEGLUE_H_
