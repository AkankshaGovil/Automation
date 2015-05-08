#ifndef _HKNIFE_CLI_H_
#define _HKNIFE_CLI_H_

/******************************************************************************
**
** This file contains the hot knife API.
**
*******************************************************************************
*/
#ifndef _clist_h_
	#include "clist.h" /* For ListEntry */
#endif

#ifndef _TEJATYPEDEFS_H
	#include "teja/tejatypedefs.h"
#endif
#ifndef _CPDPCOMMON_H
	#include "teja/cpdpcommon.h"
#endif

int
hknife_connect( char * agent_ip );

void
hknife_close(void);

teja_uint32_t
get_num_from_socket(teja_uint32_t * p_num);

teja_uint32_t
hknife_add_interface(teja_uint32_t blade_id, 
                     teja_uint32_t port_id,
                     teja_uint32_t ip_addr,
                     teja_uint32_t mask,
                     teja_uint32_t nh_id );


teja_uint32_t
hknife_config_eth_port(TejaCPDP_if_state_t port_state, 
                       teja_uint32_t blade_id,
                       teja_uint32_t port_id,
                       teja_uint32_t mtu );


teja_uint32_t
hknife_config_link_state( TejaCPDP_if_state_t blade_id,
                   TejaCPDP_if_state_t port_id,
                   TejaCPDP_link_status_t link_status );

teja_uint32_t
hknife_plumb_if(teja_uint32_t blade_id,
                teja_uint32_t port_id,
                teja_uint32_t vlan_id,
                teja_uint32_t ip_addr,
                teja_uint32_t mask );

teja_uint32_t
hknife_flush( void );

teja_uint32_t
hknife_purge_redirects( void );


teja_uint32_t
hknife_add_redirect( int32_t ingress_ethport,
                     uint16_t    ingress_vlanid,
                     uint32_t ip_daddr,
                     uint16_t l4_dport,
                     uint8_t l4_protocol,
                     uint32_t new_ip_saddr,
                     uint32_t new_l4_sport,
                     uint32_t new_ip_daddr,
                     uint32_t new_l4_dport,
                     int32_t egress_ethport,
                     uint16_t egress_vlanid,
                     TejaCPDP_legkey_t * p_legkey );

teja_uint32_t
hknife_update_redirect( int32_t ingress_blade,
					    uint32_t ip_daddr,
                        uint16_t l4_dport,
                        uint8_t l4_protocol,
                        uint32_t new_ip_saddr,
					    uint32_t new_l4_sport,
                        uint32_t new_ip_daddr,
					    uint32_t new_l4_dport,
                        int32_t egress_blade );
teja_uint32_t
hknife_delete_redirect( int32_t ingress_ethport,
                        uint32_t ingress_vlan,
                        uint32_t ip_daddr,
                        uint16_t l4_dport,
                        uint8_t l4_protocol );

teja_uint32_t
hknife_dnat_relate(  uint32_t dnat_hv,
                     uint32_t dnat_redir_addr,
                     uint32_t other_hv,
                     uint32_t other_redir_addr );

teja_uint32_t
hknife_add_route( uint32_t blade_id,
                  uint32_t port_id,
                  uint32_t vlan_id,
                  uint32_t dest_ip,
                  uint32_t mask,
                  uint32_t gw );



/*
** Constants
*/

#define DEFAULT_ETH_MTU    1500 /* Default Ethernet MTU */

#define HKNIFE_DEFAULT_VLAN_ID  0

/* Hot knife interfaces will be configured starting with this next hop ID */
#define START_NH_ID        20
#define START_L2_ID        30

/* Maximum number of unique Nexthop and ARP entries */
#define HKNIFE_MAX_NUM_NEXTHOPS    64
#define HKNIFE_NUM_NEXTHOP_BUCKETS 16  /* Must be power of 2 */
#define HKNIFE_HASH_MASK           (HKNIFE_NUM_NEXTHOP_BUCKETS - 1)

/* Default Hotknife mamagement IP */
#define HKNIFE_DEF_MGMT_IP  "169.254.0.2"

/* Constants that are returned fron hknife functions */
#define HKNIFE_GOOD        0    /* Operation successful */
#define HKNIFE_BAD         (-1) /* Operation not successful */

/*
** Data Structures
*/

/*
** ARP Nexthop table
** We need to remember the L2 and nexthop ids used in creating routes.
** The following data structures store the L2 and Nexthop IDs in a 
** hash table
*/
typedef struct _ArpNexthopEntry {
	ListEntry hashPtrs;

	teja_uint32_t  gw;      /* Gateway address key - This will be the gateway addr */
	teja_uint32_t  port_id; /* port key */
	teja_uint32_t  vlan_id; /* VLAN Id key */
	teja_uint32_t  ip;      /* Route destination - Only used as a key when looking up 
	                        ** full route
	                        */
	teja_uint32_t  l2_id;
	teja_uint32_t  nh_id;

} ArpNexthopEntry;
	

typedef struct {

	teja_uint32_t nh_id; /* The nexthop ID of the next nexthop entry */
	teja_uint32_t l2_id; /* The L2 id of the next L2 table entry */

	void *arpNexthopHash;

	ArpNexthopEntry *head, *tail; /* Pointers for alloc/free nh entries */
	ArpNexthopEntry arpNextHopEntry[HKNIFE_MAX_NUM_NEXTHOPS];

} HknifeGlobals;

/*
** MACROs
*/

/* If this code runs in multiple threads, these macros must be atomic */
#define HKNIFE_GET_AND_INCR_NH_ID( __nh_id ) \
		*__nh_id = hknife_globals.nh_id++

#define HKNIFE_GET_AND_INCR_L2_ID( __l2_id ) \
		*__l2_id = hknife_globals.l2_id++

#define HKNIFE_COMBINE_VLAN_PORT(p,v) (((v) << 2) | ((p) & 0x0003))


#endif  /* _HKNIFE_CLI_H_ */

