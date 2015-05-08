/********************************************************************/
//
// Copyright (C) Teja Technologies, Inc. 1998-2003.  
//
//
// All Rights Reserved.  
//
// This software is the property of Teja Technologies, Inc.  It is 
// furnished under a specific licensing agreement.  It may be used 
// or copied only under terms of the licensing agreement.  
//
// For more information, contact info@teja.com 
//
//
/********************************************************************/
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stdlib.h>

#include <unistd.h>  /* For hashtable.h */
#include <hashtable.h>

#include <tejatypedefs.h>
#include <cpdpcommon.h>
#include <cpdpapi.h>
#include <teja_os_include_all.h>
#include <teja_cli.h>
#include <hknife.h>

#ifndef TEJA_MAIN

#include "../../include/srvrlog.h"

#include <trace.h>

#define printf	trc_noop

#endif

/*
** Data structure to keep track of hknife globals suchas next NH and L2 id */
static HknifeGlobals  hknife_globals;

/*
** Local routines
*/
static void hknife_init_globals( void );

static teja_uint32_t
hknife_add_arp_entry( teja_uint32_t l2_id,
                      teja_uint32_t port_id,
                      teja_uint32_t vlan_id,
                      teja_uint32_t gw,
                      teja_uint32_t * p_l2_index );

static teja_uint32_t
hknife_install_route( teja_uint32_t dest_ip,
                      teja_uint32_t mask,
                      teja_uint32_t port_id,
                      teja_uint32_t vlan,
                      teja_uint32_t nh_id );

static teja_uint32_t
hknife_install_nexthop( teja_uint32_t nh_id, 
                        teja_uint32_t gw,
                        teja_uint32_t blade_id,
                        teja_uint32_t vlan_port,
                        teja_uint32_t mtu,
                        teja_uint32_t flags,
                        teja_uint32_t l2_id );

static teja_uint32_t
hknife_install_port_default( teja_uint32_t gw, 
                             teja_uint32_t blade_id, 
                             teja_uint32_t vlan_port,
                             teja_uint32_t mtu,
                             teja_uint32_t flags, /* 0 -> Forward */
                             teja_uint32_t l2_id );

static void *
hknife_nhhash_alloc( size_t size );

static void 
hknife_nhhash_free( void *ptr );

static int
hknife_nhhash_hash( void *key );

static void *
hknife_nhhash_data2key( void *ptr );

static int 
hknife_nhhash_route_keycmp( void *key1, void *key2 );

static int 
hknife_nhhash_l2_keycmp( void *key1, void *key2 );

static void
hknife_init_nhhash_entries( void );

static ArpNexthopEntry *
hknife_alloc_nhhash_entry( void );

static void
hknife_free_nhhash_entry( ArpNexthopEntry *node );





int teja_cli_send_cmd(TejaSocket socket, char *cmd);

cli_info_t info;

/* *INDENT-OFF* */
extern char *TejaCPDP_response_code_string[];

/* *INDENT-ON* */

int
hknife_connect( char * agent_ip )
{
	TejaSockaddrIn serv_addr;
	int addr;
	teja_uint32_t connect_rc;
	teja_uint32_t client_number;
	teja_uint32_t retval;

	info.top_menu_selection = 'q';
	info.init_done = 1;
	info.blade_id = 0;
	info.cli_app_socket = TEJA_INVALID_SOCKET;

	strcpy( info.agent_ip_addr, agent_ip );


	if ((info.cli_app_socket = 
		socket(AF_INET, SOCK_STREAM, 0)) == TEJA_INVALID_SOCKET) {
		trc_error(MFCE, "HKC -> hknife_connect - failed to allocate socket\n");
		return -1;
	}

	serv_addr.sin_family = TEJA_AF_INET;

	if ( ( addr = inet_addr( info.agent_ip_addr ) ) < 0 ) {
		trc_error(MFCE, 
			"HKC -> hknife_connect - inet_addr() failed for ip %s\n",
			info.agent_ip_addr );
		return -1;
	}

	serv_addr.sin_addr.s_addr = addr;
	serv_addr.sin_port = htons(CLI_AGENT_LISTEN_PORT);

	if ( teja_connect( info.cli_app_socket,
						(TejaSockaddr *) &serv_addr,
						sizeof(serv_addr)) < 0) {
		trc_error(MFCE, 
			"HKC -> hknife_connect - connect() failed for ip %s:%d\n",
		   info.agent_ip_addr, CLI_AGENT_LISTEN_PORT);
		return -1;
	} 

	if ( (retval = get_num_from_socket(&info.blade_id)) != 0) {
		trc_error(MFCE, 
			"HKC -> hknife_connect - retrieval of info.blade_id failed - error %d\n",
			retval );
		return -1;
	} 

	if ( (retval = get_num_from_socket(&connect_rc)) != 0) {
		trc_error(MFCE,
			"HKC -> hknife_connect - retrieval of connect return code - error %d\n",
			retval );
		return -1;
	}

	if (connect_rc == CLI_AGENT_RESERVE_SUCCESS) {
		if ( (retval = get_num_from_socket(&client_number)) != 0) {
			trc_error(MFCE,
				"HKC -> hknife_connect - retrieval of client number failed - error %d\n",
				retval );
			return -1;
		}

		trc_error(MFCE,
			"HKC -> hknife_connect - Established connection with Blade ID %d @ %s - client %d\n",
			info.blade_id, info.agent_ip_addr, client_number );
	} else {
		trc_error(MFCE,
			"HKC -> hknife_connect - Service refused by Blade ID %d @ %s. connect_rc %d\n",
			 	info.blade_id,
				info.agent_ip_addr,
				connect_rc );
		return -1;
	}
	return 0;
}

static teja_uint32_t
hknife_add_interface(teja_uint32_t blade_id, 
                     teja_uint32_t vlan_port,
                     teja_uint32_t ip_addr,
                     teja_uint32_t mask,
                     teja_uint32_t nh_id )
{
	teja_uint32_t ret_code = 0;
	teja_uint32_t port_id = vlan_port & 0x3;
	teja_uint32_t vlan_id = (vlan_port >> 2) & 0xfff;

	memset( info.request_cmd, (int) 0, USER_INPUT_BUFFER_SIZE );
	sprintf(	info.request_cmd,
				"%c %d %d %d %d.%d.%d.%d %d.%d.%d.%d %d",
				CMD_ADD_INTERFACE,
				blade_id,
				port_id,
				vlan_id,
				(ip_addr >> 24) & 0xff,
				(ip_addr >> 16) & 0xff,
				(ip_addr >> 8) & 0xff,
				ip_addr & 0xff,
				(mask >> 24) & 0xff,
				(mask >> 16) & 0xff,
				(mask >> 8) & 0xff,
				mask & 0xff,
				nh_id);

	trc_debug(MFCE,
			NETLOG_DEBUG2,
			"HKC -> add_interface - hk%1d,%1d - vlan %4d - ip %03d.%03d.%03d.%03d/%03d.%03d.%03d.%03d - nh_id %d\n",
			blade_id,
			(vlan_port & 0x00000003),
			((vlan_port >> 2) & 0x00000fff),
			(ip_addr >> 24) & 0xff,
			(ip_addr >> 16) & 0xff,
			(ip_addr >> 8) & 0xff,
			ip_addr & 0xff,
			(mask >> 24) & 0xff,
			(mask >> 16) & 0xff,
			(mask >> 8) & 0xff,
			mask & 0xff,
			nh_id);

	memset( info.request_string, (int) 0, USER_INPUT_BUFFER_SIZE );
	memcpy(	info.request_string,
			"add_interface",
			strlen("add_interface") + 1);

	if ( ( ret_code = teja_cli_send_cmd( info.cli_app_socket,
	                                     info.request_cmd) ) != 0 ) {
		trc_error(MFCE,
			"HKC -> add_interface - hk%1d,%1d - vlan %4d - ip %03d.%03d.%03d.%03d/%03d.%03d.%03d.%03d - nh_id %d\n",
			blade_id,
			(vlan_port & 0x00000003),
			((vlan_port >> 2) & 0x00000fff),
			(ip_addr >> 24) & 0xff,
			(ip_addr >> 16) & 0xff,
			(ip_addr >> 8) & 0xff,
			ip_addr & 0xff,
			(mask >> 24) & 0xff,
			(mask >> 16) & 0xff,
			(mask >> 8) & 0xff,
			mask & 0xff,
			nh_id);
		trc_error(MFCE,
			"HKC <- add_interface - send failed - rc %d\n",
			ret_code );
		return ret_code;
	}

	if ( (ret_code = wait_for_response()) != 0 ) {
		trc_error(MFCE,
			"HKC -> add_interface - hk%1d,%1d - vlan %4d - ip %03d.%03d.%03d.%03d/%03d.%03d.%03d.%03d - nh_id %d\n",
			blade_id,
			(vlan_port & 0x00000003),
			((vlan_port >> 2) & 0x00000fff),
			(ip_addr >> 24) & 0xff,
			(ip_addr >> 16) & 0xff,
			(ip_addr >> 8) & 0xff,
			ip_addr & 0xff,
			(mask >> 24) & 0xff,
			(mask >> 16) & 0xff,
			(mask >> 8) & 0xff,
			mask & 0xff,
			nh_id);
		trc_error(MFCE,
			"HKC <- add_interface - response failed - rc %d\n",
			ret_code );
	}

	trc_debug(MFCE,
			NETLOG_DEBUG2,
			"HKC <- add_interface - successful\n" );
	return( ret_code );
}

teja_uint32_t
hknife_config_eth_port(TejaCPDP_if_state_t port_state, 
                       teja_uint32_t blade_id,
                       teja_uint32_t port_id,
                       teja_uint32_t mtu )
{
	teja_uint32_t ret_code;
	TejaCPDP_if_state_t promiscuous_mode = 0;

	ret_code = 0;

	memset( info.request_cmd, (int) 0, USER_INPUT_BUFFER_SIZE );
	sprintf(info.request_cmd,
			"%c %d %d %d %d %d %x:%x:%x:%x:%x:%x",
			CMD_CONFIG_PORT,
			blade_id,
			port_id,
			port_state,
			mtu,
			promiscuous_mode,
			(teja_uint8_t)0, /* mac_addr.mac[0] */
			(teja_uint8_t)0, /* mac_addr.mac[1] */
			(teja_uint8_t)0, /* mac_addr.mac[2] */
			(teja_uint8_t)0, /* mac_addr.mac[3] */
			(teja_uint8_t)0, /* mac_addr.mac[4] */
			(teja_uint8_t)0  /* mac_addr.mac[5] */);

	trc_debug(MFCE,
			NETLOG_DEBUG2,
			"HKC -> config_eth_port - hk%1d,%1d - state %d, mtu %d, promiscuous %d\n",
			blade_id,
			port_id,
			port_state,
			mtu,
			promiscuous_mode );

	memset( info.request_string, (int) 0, USER_INPUT_BUFFER_SIZE );
	memcpy(	info.request_string,
			"config_eth_port",
			strlen("config_eth_port") + 1);

	if ( ( ret_code = teja_cli_send_cmd( info.cli_app_socket,
                                         info.request_cmd) ) != 0 ) {
		trc_error(MFCE,
			"HKC -> config_eth_port - hk%1d,%1d - state %d, mtu %d, promiscuous %d\n",
			blade_id,
			port_id,
			port_state,
			mtu,
			promiscuous_mode );
		trc_error(MFCE,
				"HKC <- config_eth_port - send failed - rc %d\n",
				ret_code );
        return ret_code;
	}

	if ( (ret_code = wait_for_response()) != 0 ) {
		trc_error(MFCE,
			"HKC -> config_eth_port - hk%1d,%1d - state %d, mtu %d, promiscuous %d\n",
			blade_id,
			port_id,
			port_state,
			mtu,
			promiscuous_mode );
		trc_error(MFCE,
				"HKC <- config_eth_port - response failed - rc %d\n",
				ret_code );
        return ret_code;
	} else {
		trc_debug(MFCE,
			NETLOG_DEBUG2,
			"HKC <- config_eth_port - success\n",
			ret_code );
    }

	return ret_code;
}

teja_uint32_t
hknife_config_link_state( TejaCPDP_if_state_t blade_id,
                   TejaCPDP_if_state_t port_id,
                   TejaCPDP_link_status_t link_status )
{
	teja_uint32_t ret_code;

	ret_code = 0;

	memset( info.request_cmd, (int) 0, USER_INPUT_BUFFER_SIZE );
	sprintf(info.request_cmd,
			"%c %d %d %d",
			CMD_CONFIG_LINK_STATE,
			blade_id,
			port_id,
			link_status);
	memset( info.request_string, (int) 0, USER_INPUT_BUFFER_SIZE );
	memcpy(	info.request_string,
			"get_link_status",
			strlen("get_link_status") + 1);

	trc_debug(MFCE,
			NETLOG_DEBUG2,
			"HKC -> config_link_state - hk%1d,%1d - link_status %d\n",
			blade_id,
			port_id,
			link_status );

    if ( ( ret_code = teja_cli_send_cmd( info.cli_app_socket,
                                         info.request_cmd) ) != 0 ) {
		trc_error(MFCE,
			"HKC -> config_link_state - hk%1d,%1d - link_status %d\n",
			blade_id,
			port_id,
			link_status );
		trc_error(MFCE,
				"HKC <- config_link_state - send failed - rc %d\n",
				ret_code );
        return ret_code;
	}

	if ( (ret_code = wait_for_response()) != 0 ) {
		trc_error(MFCE,
			"HKC -> config_link_state - hk%1d,%1d - link_status %d\n",
			blade_id,
			port_id,
			link_status );
		trc_error(MFCE,
				"HKC <- config_link_state - response failed - rc %d\n",
				ret_code );
        	return( ret_code );
	} else {
		trc_debug(MFCE,
			NETLOG_DEBUG2,
			"HKC <- config_link_state - success\n" );
	}

	return( ret_code );
}

//
//	Function 	:
//		hknife_plumb_if()
//
//	Purpose		:
//
//		This function plumbe an interface on the hot knife (radisys) card
//
//	Return		:
//		0 -> OK, !0 -> Bad
//
teja_uint32_t
hknife_plumb_if(teja_uint32_t blade_id,
                teja_uint32_t port_id,
                teja_uint32_t vlan_id,
                teja_uint32_t ip_addr,
                teja_uint32_t mask )
{
	teja_uint32_t ret_code;
	teja_uint32_t nh_id;

	/* Configure Port */
	ret_code = hknife_config_eth_port( 1, /* Enabled */
	                                   blade_id, /* Blade ID */
	                                   port_id,
	                                   DEFAULT_ETH_MTU );
	if( ret_code != 0 ) {
		return( ret_code );
	}

	/* Change Link state to up */
	ret_code = hknife_config_link_state( blade_id,
	                                     port_id,
	                                     1 /* Up */ );

	if( ret_code != 0 ) {
		return( ret_code );
	}

	/* Add the IP address */
	HKNIFE_GET_AND_INCR_NH_ID( &nh_id );
	ret_code = hknife_add_interface( blade_id, 
	                                 HKNIFE_COMBINE_VLAN_PORT( port_id, vlan_id ),
	                                 ip_addr,
	                                 mask,
	                                 nh_id );

	return( ret_code );
}

//
//	Function 	:
//		hknife_flush()
//
//	Purpose		:
//
//		Deletes all interfaces, routes, and redirects on the hot knife. Called
//		during gis startup
//
//	Return		:
//		0 -> OK, !0 -> Bad
//
teja_uint32_t
hknife_flush( void )
{
	teja_uint32_t ret_code;

	/* Initialize globals */
	hknife_init_globals();

	/* Bring interfaces down - purge interfaces */
	memset( info.request_cmd, (int) 0, USER_INPUT_BUFFER_SIZE );
	sprintf(info.request_cmd, "%c", CMD_PURGE_PORTS);

	trc_debug(MFCE,
			NETLOG_DEBUG2,
			"HKC -> purge_ports\n" );

	ret_code = teja_cli_send_cmd(info.cli_app_socket, info.request_cmd);
	if( ret_code != 0 ) {
		trc_debug(MFCE,
			NETLOG_DEBUG2,
			"HKC <- purge_ports - send failed - rc %d\n", ret_code );
		return( ret_code );
	}
	if( (ret_code = wait_for_response()) != 0 ) {
		trc_debug(MFCE,
			NETLOG_DEBUG2,
			"HKC <- purge_ports - response failed - rc %d\n", ret_code );
		return( ret_code );
	} else {
		trc_debug(MFCE,
			NETLOG_DEBUG2,
			"HKC <- purge_ports - success\n" );
	}

	/* Purge Redirects */

	trc_debug(MFCE,
			NETLOG_DEBUG2,
			"HKC -> purge_redirects\n" );

	ret_code = hknife_purge_redirects();
	if( ret_code != 0 ) {
		return( ret_code );
	}

	/* Purge L2 Table  */
	memset( info.request_cmd, (int) 0, USER_INPUT_BUFFER_SIZE );
	sprintf( info.request_cmd, "%c", CMD_PURGE_L2_TABLE );

	trc_debug(MFCE,
			NETLOG_DEBUG2,
			"HKC -> purge_l2_table\n" );

	ret_code = teja_cli_send_cmd(info.cli_app_socket, info.request_cmd);
	if( ret_code != 0 ) {
		trc_debug(MFCE,
			NETLOG_DEBUG2,
			"HKC <- purge_l2_table - send failed - rc %d\n", ret_code );
		return( ret_code );
	}
	if( (ret_code = wait_for_response()) != 0 ) {
		trc_debug(MFCE,
			NETLOG_DEBUG2,
			"HKC <- purge_l2_table - response failed - rc %d\n", ret_code );
		return( ret_code );
	} else {
		trc_debug(MFCE,
			NETLOG_DEBUG2,
			"HKC <- purge_l2_table - success\n" );
	}

	/* Purge RTM - should take care of both nexthop and routing table */
	memset( info.request_cmd, (int) 0, USER_INPUT_BUFFER_SIZE );
	sprintf(info.request_cmd, "%c", CMD_PURGE_RTM);

	trc_debug(MFCE,
			NETLOG_DEBUG2,
			"HKC -> purge_RTM\n" );

	ret_code = teja_cli_send_cmd(info.cli_app_socket, info.request_cmd);
	if( ret_code != 0 ) {
		trc_debug(MFCE,
			NETLOG_DEBUG2,
			"HKC <- purge_RTM - send failed - rc %d\n", ret_code );
		return( ret_code );
	}

	if( (ret_code = wait_for_response()) != 0 ) {
		trc_debug(MFCE,
			NETLOG_DEBUG2,
			"HKC <- purge_RTM - response failed - rc %d\n", ret_code );
		return( ret_code );
	} else {
		trc_debug(MFCE,
			NETLOG_DEBUG2,
			"HKC <- purge_RTM - success\n" );
	}
	
	/* Purge ARP Cache */
	memset( info.request_cmd, (int) 0, USER_INPUT_BUFFER_SIZE );
	sprintf(info.request_cmd, "%c", CMD_PURGE_ARP_CACHE);

	trc_debug(MFCE,
			NETLOG_DEBUG2,
			"HKC -> purge_arp_cache\n" );

	ret_code = teja_cli_send_cmd(info.cli_app_socket, info.request_cmd);
	if( ret_code != 0 ) {
		trc_debug(MFCE,
			NETLOG_DEBUG2,
			"HKC <- purge_arp_cache - send failed - rc %d\n", ret_code );
		return( ret_code );
	}
	if( (ret_code = wait_for_response()) != 0 ) {
		trc_debug(MFCE,
			NETLOG_DEBUG2,
			"HKC <- purge_arp_cache - response failed - rc %d\n", ret_code );
		return( ret_code );
	} else {
		trc_debug(MFCE,
			NETLOG_DEBUG2,
			"HKC <- purge_arp_cache - success\n" );
	}

	return( HKNIFE_GOOD );
}

//
//	Function 	:
//		hknife_purge_redirects()
//
//	Purpose		:
//
//		Deletes all existing redirects on the hot knife
//
//	Return		:
//		0 -> OK, !0 -> Bad
//
teja_uint32_t
hknife_purge_redirects( void )
{
	teja_uint32_t ret_code;
	
	memset( info.request_cmd, (int) 0, USER_INPUT_BUFFER_SIZE );
 	sprintf(info.request_cmd, "%c", CMD_PURGE_NAT_REDIRECTS );

    if ( ( ret_code = teja_cli_send_cmd( info.cli_app_socket,
                                         info.request_cmd) ) != 0 ) {
		trc_debug(MFCE,
			NETLOG_DEBUG2,
			"HKC <- purge_redirects - send failed - rc %d\n", ret_code );
        return ret_code;
	}

    if ( (ret_code = wait_for_response()) != 0 ) {
		trc_debug(MFCE,
			NETLOG_DEBUG2,
			"HKC <- purge_redirects - response failed - rc %d\n", ret_code );
	} else {
		trc_debug(MFCE,
			NETLOG_DEBUG2,
			"HKC <- purge_redirects - success\n" );
	}

	return ret_code;
}

//
//	Function 	:
//		hknife_add_redirect()
//
//	Purpose		:
//
//		Adds a redirect
//
//	Return		:
//		0 -> OK, !0 -> Bad
//
teja_uint32_t
hknife_add_redirect(int32_t ingress_ethport,
                    uint16_t ingress_vlanid,
                    uint32_t ip_daddr,
                    uint16_t l4_dport,
                    uint8_t  l4_protocol,
                    uint32_t new_ip_saddr,
                    uint32_t new_l4_sport,
                    uint32_t new_ip_daddr,
                    uint32_t new_l4_dport,
                    int32_t  egress_ethport,
                    uint16_t egress_vlanid,
                    TejaCPDP_legkey_t *p_legkey )
{
	teja_uint32_t ret_code;

	ret_code = 0;

	// Fill in redirect info

	memset( info.request_cmd, (int) 0, USER_INPUT_BUFFER_SIZE );
	sprintf( info.request_cmd,
					"%c %d %d.%d.%d.%d %d %d %d %d.%d.%d.%d %d %d.%d.%d.%d %d %d %d",
					CMD_ADD_NAT_REDIRECT,
					ingress_ethport,
					(ip_daddr >> 24) & 0xff,
					(ip_daddr >> 16) & 0xff,
					(ip_daddr >> 8) & 0xff,
					(ip_daddr) & 0xff,
					l4_dport,
					l4_protocol,
					ingress_vlanid,
					(new_ip_saddr >> 24) & 0xff,
					(new_ip_saddr >> 16) & 0xff,
					(new_ip_saddr >> 8) & 0xff,
					(new_ip_saddr) & 0xff,
					new_l4_sport,
					(new_ip_daddr >> 24) & 0xff,
					(new_ip_daddr >> 16) & 0xff,
					(new_ip_daddr >> 8) & 0xff,
					(new_ip_daddr) & 0xff,
					new_l4_dport,
					egress_ethport,
					egress_vlanid );

	memset( info.request_string, (int) 0, USER_INPUT_BUFFER_SIZE );
	memcpy(	info.request_string,
	        "add_redirect",
	        strlen("add_redirect") + 1);

	if ( ( ret_code = teja_cli_send_cmd( info.cli_app_socket,
	                                     info.request_cmd) ) != 0 )
		return ret_code;

	return( ret_code = wait_for_response_add_redirect( &p_legkey->hv, &p_legkey->redir_addr ) );
}

//
//	Function 	:
//		hknife_update_redirect()
//
//	Purpose		:
//
//		Update an existing redirect
//
//	Return		:
//		0 -> OK, !0 -> Bad
//
teja_uint32_t
hknife_update_redirect(int32_t		ingress_ethport,
					uint32_t		ip_daddr,
					uint16_t		l4_dport,
					uint8_t			l4_protocol,
					uint32_t		new_ip_saddr,
					uint32_t		new_l4_sport,
					uint32_t		new_ip_daddr,
					uint32_t		new_l4_dport,
					int32_t 		egress_ethport )
{
	teja_uint32_t ret_code;

	ret_code = 0;

	// Fill in redirect info

	memset( info.request_cmd, (int) 0, USER_INPUT_BUFFER_SIZE );
	sprintf( info.request_cmd,
					"%c %d %d.%d.%d.%d %d %d %d.%d.%d.%d %d %d.%d.%d.%d %d %d",
					CMD_UPDATE_NAT_REDIRECT,
					ingress_ethport,
					(ip_daddr >> 24) & 0xff,
					(ip_daddr >> 16) & 0xff,
					(ip_daddr >> 8) & 0xff,
					(ip_daddr) & 0xff,
					l4_dport,
					l4_protocol,
					(new_ip_saddr >> 24) & 0xff,
					(new_ip_saddr >> 16) & 0xff,
					(new_ip_saddr >> 8) & 0xff,
					(new_ip_saddr) & 0xff,
					new_l4_sport,
					(new_ip_daddr >> 24) & 0xff,
					(new_ip_daddr >> 16) & 0xff,
					(new_ip_daddr >> 8) & 0xff,
					(new_ip_daddr) & 0xff,
					new_l4_dport,
					egress_ethport );

	memset( info.request_string, (int) 0, USER_INPUT_BUFFER_SIZE );
	memcpy(	info.request_string,
	        "update_redirect",
	        strlen("update_redirect") + 1);

	if ( ( ret_code = teja_cli_send_cmd( info.cli_app_socket,
	                                     info.request_cmd) ) != 0 )
		return ret_code;

	return( ret_code = wait_for_response() );
}

//
//	Function 	:
//		hknife_delete_redirect()
//
//	Purpose		:
//
//		Deletes a redirect
//
//	Return		:
//		0 -> OK, !0 -> Bad
//
teja_uint32_t
hknife_delete_redirect( int32_t ingress_ethport, uint32_t ingress_vlan, uint32_t ip_daddr, uint16_t l4_dport, uint8_t l4_protocol )
{
	teja_uint32_t ret_code;

	ret_code = 0;

	memset( info.request_cmd, (int) 0, USER_INPUT_BUFFER_SIZE );
	sprintf(info.request_cmd,
	        "%c %d %d.%d.%d.%d %d %d %d",
	        CMD_DELETE_NAT_REDIRECT,
	        ingress_ethport,
	        (ip_daddr >> 24) & 0xff,
	        (ip_daddr >> 16) & 0xff,
	        (ip_daddr >> 8) & 0xff,
	        (ip_daddr) & 0xff,
	        l4_dport,
	        l4_protocol,
          ingress_vlan );
	memset( info.request_string, (int) 0, USER_INPUT_BUFFER_SIZE );
	memcpy(	info.request_string,
					"delete_redirect",
					strlen("delete_redirect") + 1);

	if( (ret_code = teja_cli_send_cmd( info.cli_app_socket,
	                                   info.request_cmd) ) != 0 )
		return ret_code;

	return( ret_code = wait_for_response() );
}

//
//	Function 	:
//		hknife_dnat_relate()
//
//	Purpose		:
//
//		Relates two existing redirects for data nat purposes
//
//	Return		:
//		0 -> OK, !0 -> Bad
//
teja_uint32_t
hknife_dnat_relate(  uint32_t dnat_hv,
                     uint32_t dnat_redir_addr,
                     uint32_t other_hv,
                     uint32_t other_redir_addr )
{
	teja_uint32_t ret_code;

	ret_code = 0;

	memset( info.request_cmd, (int) 0, USER_INPUT_BUFFER_SIZE );
	sprintf(info.request_cmd,
	        "%c %d %d %d %d",
	        CMD_RELATE_DNAT,
	        dnat_hv,
	        dnat_redir_addr,
	        other_hv,
	        other_redir_addr );
	memset( info.request_string, (int) 0, USER_INPUT_BUFFER_SIZE );
	memcpy(	info.request_string, "relate_dnat", strlen("relate_dnat") + 1);

	if( (ret_code = teja_cli_send_cmd( info.cli_app_socket,
	                                   info.request_cmd) ) != 0 )
		return ret_code;

	return( ret_code = wait_for_response() );
}

//
//	Function 	:
//		hknife_close()
//
//	Purpose		:
//
//		Closes the connection between hknife and the host
//
//	Return		:
//		0 -> OK, !0 -> Bad
//
void
hknife_close(void)
{
	teja_close_socket(info.cli_app_socket);
}

//
//	Function 	:
//		hknife_add_route()
//
//	Purpose		:
//
//		Adds a route to the hot knife card. Currently, this routine adds
//    global routes normally and port default routes if the route is
//    a default route (ip == mask == 0x00000000).
//    In the near future, this routine will be upgraded to add a route
//    on an interface and a vlan
//
//	Return		:
//		0 -> OK, !0 -> Bad
//
teja_uint32_t
hknife_add_route( teja_uint32_t blade_id, /* Blade id */
                  teja_uint32_t port_id,  /* Port */
                  teja_uint32_t vlan_id,  /* VLAN id */
                  teja_uint32_t dest_ip,  /* Destination IP address */
                  teja_uint32_t mask,     /* Route mask */
                  teja_uint32_t gw )      /* gateway */
{

	ArpNexthopEntry *nh_entry, *new_nh_entry;
	teja_uint32_t l2_index;

	trc_debug(MFCE,
		NETLOG_DEBUG2,
		"HKC -> add_route - hk%1d,%1d - vlan %4d - ip %03d.%03d.%03d.%03d/%03d.%03d.%03d.%03d - gw %03d.%03d.%03d.%03d\n",
		blade_id,
		port_id,
		vlan_id,
		(dest_ip >> 24) & 0xff,
		(dest_ip >> 16) & 0xff,
		(dest_ip >>  8) & 0xff,
		(dest_ip >>  0) & 0xff,
		(mask    >> 24) & 0xff,
		(mask    >> 16) & 0xff,
		(mask    >>  8) & 0xff,
		(mask    >>  0) & 0xff,
		(gw      >> 24) & 0xff,
		(gw      >> 16) & 0xff,
		(gw      >>  8) & 0xff,
		(gw      >>  0) & 0xff );

	new_nh_entry = hknife_alloc_nhhash_entry();
	if( new_nh_entry == NULL ) {
		trc_error(MFCE,
			"HKC <- add_route - failed hknife_alloc_nhhash_entry() returns NULL\n" );
		return( HKNIFE_BAD );
	}

	new_nh_entry->gw = gw;
	new_nh_entry->port_id = port_id;
	new_nh_entry->vlan_id = vlan_id;
	new_nh_entry->ip = dest_ip;

	nh_entry = HashFind( hknife_globals.arpNexthopHash,
	                    (void *)new_nh_entry,
	                    hknife_nhhash_hash,
	                    hknife_nhhash_data2key,
	                    hknife_nhhash_l2_keycmp,
	                    0);

	if( nh_entry == NULL ) {

		/* Did not find old one we could use - use new one */
		HKNIFE_GET_AND_INCR_L2_ID( &new_nh_entry->l2_id );
		HKNIFE_GET_AND_INCR_NH_ID( &new_nh_entry->nh_id );

		/* First add the dest IP to the ARP table */
		if( hknife_add_arp_entry( new_nh_entry->l2_id, port_id, vlan_id, gw, &l2_index ) ) {
			/* Bad */
		    trc_error(MFCE,
			   "HKC <- add_route - failed hknife_add_arp() - nh_entry == NULL\n" );
			hknife_free_nhhash_entry( new_nh_entry );
			return( HKNIFE_BAD );
		}

		if( (dest_ip == 0x00000000) && (mask == 0x00000000) ) {
			/*
			** Port default route
			*/
			if( hknife_install_port_default( gw, 
			                                 blade_id, 
			                                 HKNIFE_COMBINE_VLAN_PORT( port_id, vlan_id ),
			                                 DEFAULT_ETH_MTU,
			                                 0, /* 0 -> Forward */
			                                 new_nh_entry->l2_id ) ) {
				/* Bad */
				trc_error(MFCE,
			   		"HKC <- add_route - failed hknife_install_port_default() - nh_entry == NULL\n" );
				hknife_free_nhhash_entry( new_nh_entry );
				return( HKNIFE_BAD );
			}
		} else {
			/*
			** Normal route
			*/

			/* Add the Nexthop */
			if( hknife_install_nexthop( new_nh_entry->nh_id, 
			                            gw,
			                            blade_id,
			                            HKNIFE_COMBINE_VLAN_PORT( port_id, vlan_id ),
			                            DEFAULT_ETH_MTU,
			                            0, /* 0 = FORWARD */
			                            new_nh_entry->l2_id ) ) {
				/* Bad */
				trc_error(MFCE,
			   		"HKC <- add_route - failed hknife_install_nexthop() - nh_entry == NULL\n" );
				hknife_free_nhhash_entry( new_nh_entry );
				return( HKNIFE_BAD );
			}

			/* Add the route */
			if( hknife_install_route( dest_ip, mask, port_id, vlan_id, 
			                          new_nh_entry->nh_id ) ) {
				/* Bad */
				trc_error(MFCE,
			   		"HKC <- add_route - failed hknife_install_route() - nh_entry == NULL\n" );

				hknife_free_nhhash_entry( new_nh_entry );
				return( HKNIFE_BAD );
			}
		}

		/* Add the new entry to the hash */
		HashInsert( hknife_globals.arpNexthopHash,
		            (void *)new_nh_entry,
		            hknife_nhhash_hash,
		            hknife_nhhash_data2key,
		            hknife_nhhash_route_keycmp,
		            0 );
		
	} else {
		/*
		** The L2 and NH ID was found
		*/
		if( (dest_ip == 0x00000000) && (mask == 0x00000000) ) {
			/*
			** Port default route
			*/
			hknife_free_nhhash_entry( new_nh_entry );

			if( hknife_install_port_default( gw, 
			                                 blade_id, 
			                                 HKNIFE_COMBINE_VLAN_PORT( port_id, vlan_id ),
			                                 DEFAULT_ETH_MTU,
			                                 0, /* 0 -> Forward */
			                                 nh_entry->l2_id ) ) {
				/* Bad */
				trc_error(MFCE,
			   		"HKC <- add_route - failed hknife_install_port_default() - nh_entry != NULL\n" );
				return( HKNIFE_BAD );
			}
		} else {
			/*
			** Normal route
			*/

			/* Add the route if not a DUP */
			if( (nh_entry->ip != dest_ip) &&
			    !HashFind( hknife_globals.arpNexthopHash,
	                    (void *)new_nh_entry,
	                    hknife_nhhash_hash,
	                    hknife_nhhash_data2key,
	                    hknife_nhhash_route_keycmp,
	                    0) ) {
				if( hknife_install_route( dest_ip, mask, port_id, vlan_id,
				                           nh_entry->nh_id ) ) {
					/* Bad */
					trc_error(MFCE,
			   			"HKC <- add_route - failed hknife_install_route() - nh_entry != NULL\n" );
					hknife_free_nhhash_entry( new_nh_entry );
					return( HKNIFE_BAD );
				}

				HashInsert( hknife_globals.arpNexthopHash,
				            (void *)new_nh_entry,
				            hknife_nhhash_hash,
				            hknife_nhhash_data2key,
				            hknife_nhhash_route_keycmp,
				            0 );

			} else {
				hknife_free_nhhash_entry( new_nh_entry );
			}
		}
	}

	trc_debug(MFCE,
		NETLOG_DEBUG2,
  		"HKC <- add_route - successful\n" );

	return( HKNIFE_GOOD );
}
	
/******************************************************************************
**
**  Local routines
**
*******************************************************************************
*/

//
//	Function 	:
//		hknife_init_globals()
//
//	Purpose		:
//
//		Initializes data structures that are global to this file
//
//	Return		:
//		NONE
//

static void hknife_init_globals( void )
{
	hknife_globals.nh_id = START_NH_ID;
	hknife_globals.l2_id = START_L2_ID;

	if( hknife_globals.arpNexthopHash != NULL ) {
		HashDestroy( hknife_globals.arpNexthopHash,
		             HKNIFE_NUM_NEXTHOP_BUCKETS,
		             hknife_nhhash_free );
	}

	hknife_globals.arpNexthopHash = HashCreate( HKNIFE_NUM_NEXTHOP_BUCKETS,
	                                            0,
	                                            hknife_nhhash_alloc,
	                                            hknife_nhhash_free );

	hknife_init_nhhash_entries( );

}

//
//	Function 	:
//		hknife_add_arp_entry()
//
//	Purpose		:
//
//		Provides a wrapper around the teja sprintf command
//    for adding a dynamic ARP entry
//
//	Return		:
//		0 -> good !0 -> bad
//

static teja_uint32_t
hknife_add_arp_entry( teja_uint32_t l2_id,
                      teja_uint32_t port_id,
					  teja_uint32_t vlan_id,
                      teja_uint32_t gw,
					  teja_uint32_t * p_l2_index )
{

	teja_uint32_t ret_code;

	ret_code = HKNIFE_GOOD;

	if ( vlan_id > 4095 ) {
		trc_error(MFCE,
			"HKC !! add_arp_entry - invalid vlan_id %d\n", vlan_id  );
		return -1;
	}

	// Fill in arp info

	memset( info.request_cmd, (int) 0, USER_INPUT_BUFFER_SIZE );
	sprintf( info.request_cmd,
	         "%c %d %d %d %d.%d.%d.%d %d",
	         CMD_CREATE_ARP,
	         l2_id,
	         port_id,
			 vlan_id,
	         (gw >> 24) & 0xff,
	         (gw >> 16) & 0xff,
	         (gw >> 8) & 0xff,
	         (gw) & 0xff,
			 1 );

	memset( info.request_string, (int) 0, USER_INPUT_BUFFER_SIZE );
	memcpy(	info.request_string,
	        "create_arp",
	        strlen("create_arp") + 1);

	trc_debug(MFCE,
			NETLOG_DEBUG2,
			"HKC -> add_arp_entry - l2_id %d, port_id %d, gw %03d.%03d.%03d.%03d, perm %d, vlan_id %d\n",
	         l2_id,
	         port_id,
	         (gw >> 24) & 0xff,
	         (gw >> 16) & 0xff,
	         (gw >> 8) & 0xff,
	         (gw) & 0xff,
			 1,
			 vlan_id );

	if ( ( ret_code = teja_cli_send_cmd( info.cli_app_socket,
	                                     info.request_cmd) ) != 0 ) {
		trc_debug(MFCE,
			NETLOG_DEBUG2,
			"HKC <- add_arp_entry - send failed - rc %d\n", ret_code );
		return ret_code;
	}

	if ( (ret_code = wait_for_response_add_arp( p_l2_index ) ) != 0 ) {
		trc_debug(MFCE,
			NETLOG_DEBUG2,
			"HKC <- add_arp_entry - response failed - rc %d\n", ret_code );
	} else {
		trc_debug(MFCE,
			NETLOG_DEBUG2,
			"HKC <- add_arp_entry - success - l2_id %d\n", *p_l2_index );
	}
	return ret_code;
}

//
//	Function 	:
//		hknife_install_nexthop()
//
//	Purpose		:
//
//		Provides a wrapper around the teja sprintf command
//    for adding a nexthop entry
//
//	Return		:
//		0 -> good !0 -> bad
//
static teja_uint32_t
hknife_install_nexthop( teja_uint32_t nh_id, 
                        teja_uint32_t gw,
                        teja_uint32_t blade_id,
                        teja_uint32_t vlan_port,
                        teja_uint32_t mtu,
                        teja_uint32_t flags,
                        teja_uint32_t l2_id )
{
	teja_uint32_t ret_code;

	ret_code = HKNIFE_GOOD;

	// Fill in redirect info

	memset( info.request_cmd, (int) 0, USER_INPUT_BUFFER_SIZE );
	sprintf( info.request_cmd,
	         "%c %d %d.%d.%d.%d %d %d %d %d %d",
	         CMD_ADD_NEXTHOP,
	         nh_id,
	         (gw >> 24) & 0xff,
	         (gw >> 16) & 0xff,
	         (gw >> 8) & 0xff,
	         (gw) & 0xff,
	         blade_id,
	         vlan_port,
	         mtu,
	         flags,
	         l2_id );

	memset( info.request_string, (int) 0, USER_INPUT_BUFFER_SIZE );
	memcpy(	info.request_string, "add_nexthop", strlen("add_nexthop") + 1);

	trc_debug(MFCE,
			NETLOG_DEBUG2,
			"HKC -> install_nexthop - nh_id %d, gw %03d.%03d.%03d.%03d, blade %d, port_id %d, vlan_id %4d, mtu %d, flags %d, l2_id %d\n",
	         nh_id,
	         (gw >> 24) & 0xff,
	         (gw >> 16) & 0xff,
	         (gw >> 8) & 0xff,
	         (gw) & 0xff,
	         blade_id,
	         vlan_port & 0x00000003,
	         (vlan_port >> 2) & 0x00000fff,
	         mtu,
	         flags,
	         l2_id );

	if( (ret_code = teja_cli_send_cmd( info.cli_app_socket,
	                                   info.request_cmd) ) != 0 ) {
		trc_debug(MFCE,
			NETLOG_DEBUG2,
			"HKC <- install_nexthop - send failed - rc %d\n", ret_code );
		return ret_code;
	}

	if ( (ret_code = wait_for_response()) != 0 ) {
		trc_debug(MFCE,
			NETLOG_DEBUG2,
			"HKC <- install_nexthop - response failed - rc %d\n", ret_code );
	} else {
		trc_debug(MFCE,
			NETLOG_DEBUG2,
			"HKC <- install_nexthop - success\n" );
	}
	return ret_code;
}

//
//	Function 	:
//		hknife_install_route()
//
//	Purpose		:
//
//		Provides a wrapper around the teja sprintf command
//    for adding a route
//
//	Return		:
//		0 -> good !0 -> bad
//
static teja_uint32_t
hknife_install_route( teja_uint32_t dest_ip,
                      teja_uint32_t mask,
                      teja_uint32_t port_id,
                      teja_uint32_t vlan,
                      teja_uint32_t nh_id )
{

	teja_uint32_t ret_code;

	ret_code = HKNIFE_GOOD;

	// Fill in redirect info

	memset( info.request_cmd, (int) 0, USER_INPUT_BUFFER_SIZE );
	sprintf( info.request_cmd,
	         "%c %d.%d.%d.%d %d.%d.%d.%d %d %d %d",
	         CMD_ADD_ROUTE,
	         (dest_ip >> 24) & 0xff,
	         (dest_ip >> 16) & 0xff,
	         (dest_ip >> 8) & 0xff,
	         (dest_ip) & 0xff,
	         (mask >> 24) & 0xff,
	         (mask >> 16) & 0xff,
	         (mask >> 8) & 0xff,
	         (mask) & 0xff,
	         port_id,
	         vlan,
	         nh_id );

	trc_debug(MFCE,
			NETLOG_DEBUG2,
			"HKC -> install_route - ip %03d.%03d.%03d.%03d/%03d.%03d.%03d.%03d, port %d, vlan %d, nh_id %d\n",
	         (dest_ip >> 24) & 0xff,
	         (dest_ip >> 16) & 0xff,
	         (dest_ip >> 8) & 0xff,
	         (dest_ip) & 0xff,
	         (mask >> 24) & 0xff,
	         (mask >> 16) & 0xff,
	         (mask >> 8) & 0xff,
	         (mask) & 0xff,
			 port_id,
			 vlan,
	         nh_id );

	memset( info.request_string, (int) 0, USER_INPUT_BUFFER_SIZE );
	memcpy(	info.request_string,
					"add_route",
					strlen("add_route") + 1);

	if ( ( ret_code = teja_cli_send_cmd(	info.cli_app_socket,
											info.request_cmd) ) != 0 ) {
		trc_error(MFCE,
			"HKC -> install_route - ip %03d.%03d.%03d.%03d/%03d.%03d.%03d.%03d, port %d, vlan %d, nh_id %d\n",
	         (dest_ip >> 24) & 0xff,
	         (dest_ip >> 16) & 0xff,
	         (dest_ip >> 8) & 0xff,
	         (dest_ip) & 0xff,
	         (mask >> 24) & 0xff,
	         (mask >> 16) & 0xff,
	         (mask >> 8) & 0xff,
	         (mask) & 0xff,
			 port_id,
			 vlan,
	         nh_id );
		trc_error(MFCE,
			"HKC <- install_route - send failed - rc %d\n", ret_code );
		return ret_code;
	}

	if ( (ret_code = wait_for_response()) != 0 ) {
		trc_error(MFCE,
			"HKC -> install_route - ip %03d.%03d.%03d.%03d/%03d.%03d.%03d.%03d, port %d, vlan %d, nh_id %d\n",
	         (dest_ip >> 24) & 0xff,
	         (dest_ip >> 16) & 0xff,
	         (dest_ip >> 8) & 0xff,
	         (dest_ip) & 0xff,
	         (mask >> 24) & 0xff,
	         (mask >> 16) & 0xff,
	         (mask >> 8) & 0xff,
	         (mask) & 0xff,
			 port_id,
			 vlan,
	         nh_id );
		trc_error(MFCE,
			"HKC <- install_route - response failed - rc %d\n", ret_code );
	}
	return ret_code;
}

//
//	Function 	:
//		hknife_install_port_default()
//
//	Purpose		:
//
//		Provides a wrapper around the teja sprintf command
//    for adding a port default route
//
//	Return		:
//		0 -> good !0 -> bad
//
static teja_uint32_t
hknife_install_port_default( teja_uint32_t gw, 
                             teja_uint32_t blade_id, 
                             teja_uint32_t vlan_port,
                             teja_uint32_t mtu,
                             teja_uint32_t flags, /* 0 -> Forward */
                             teja_uint32_t l2_id )
{
	teja_uint32_t ret_code;

	ret_code = HKNIFE_GOOD;

	// Fill in redirect info

	memset( info.request_cmd, (int) 0, USER_INPUT_BUFFER_SIZE );
	sprintf( info.request_cmd,
	         "%c %d %d.%d.%d.%d %d %d %d %d %d",
	         CMD_ADD_PORT_DEFAULT,
	         0xB0B, /* nh_id - Value is not used */
	         (gw >> 24) & 0xff,
	         (gw >> 16) & 0xff,
	         (gw >> 8) & 0xff,
	         (gw) & 0xff,
	         blade_id,
	         vlan_port,
	         mtu,
	         flags,
	         l2_id );

	memset( info.request_string, (int) 0, USER_INPUT_BUFFER_SIZE );
	memcpy(	info.request_string,
					"add_port_default",
					strlen("add_port_default") + 1);

	trc_debug(MFCE,
			NETLOG_DEBUG2,
			"HKC -> install_port_default - gw %03d.%03d.%03d.%03d blade %d, port %d, vlan %d, mtu %d, flags %d, l2_id %d\n",
	         (gw >> 24) & 0xff,
	         (gw >> 16) & 0xff,
	         (gw >> 8) & 0xff,
	         (gw) & 0xff,
	         blade_id,
	         vlan_port & 0x00000003,
	         (vlan_port >> 2) & 0x00000fff,
	         mtu,
	         flags,
	         l2_id );

	if( (ret_code = teja_cli_send_cmd( info.cli_app_socket,
	                                   info.request_cmd) ) != 0 ) {
		trc_error(MFCE,
			"HKC -> install_port_default - gw %03d.%03d.%03d.%03d blade %d, port %d, vlan %d, mtu %d, flags %d, l2_id %d\n",
	         (gw >> 24) & 0xff,
	         (gw >> 16) & 0xff,
	         (gw >> 8) & 0xff,
	         (gw) & 0xff,
	         blade_id,
	         vlan_port & 0x00000003,
	         (vlan_port >> 2) & 0x00000fff,
	         mtu,
	         flags,
	         l2_id );
		trc_error(MFCE,
			"HKC <- install_port_default - send failed - rc %d\n", ret_code );
		return ret_code;
	}

	trc_debug(MFCE,
			NETLOG_DEBUG2,
			"HKC -- install_port_default - waiting for response\n");

	if ( (ret_code = wait_for_response()) != 0 ) {
		trc_error(MFCE,
			"HKC -> install_port_default - gw %03d.%03d.%03d.%03d blade %d, port %d, vlan %d, mtu %d, flags %d, l2_id %d\n",
	         (gw >> 24) & 0xff,
	         (gw >> 16) & 0xff,
	         (gw >> 8) & 0xff,
	         (gw) & 0xff,
	         blade_id,
	         vlan_port & 0x00000003,
	         (vlan_port >> 2) & 0x00000fff,
	         mtu,
	         flags,
	         l2_id );
		trc_error(MFCE,
			"HKC <- install_port_default - response failed - rc %d\n", ret_code );
	}

	trc_debug(MFCE,
			NETLOG_DEBUG2,
			"HKC <- install_port_default - successful\n");

	return ret_code;
}


/******************************************************************************
**
**  Routines for the ARP/Nexthop Hash
**
*******************************************************************************
*/

//
//	Function 	:
//		hknife_nhhash_alloc()
//
//	Purpose		:
//
//		Allocates a ArpNexthopEntry
//
//	Return		:
//		Pointer to new ArpNexthopEntry NULL if out
//
static void *
hknife_nhhash_alloc( size_t size )
{
	return( (void *)malloc( size ) );
}

//
//	Function 	:
//		hknife_nhhash_free()
//
//	Purpose		:
//
//		Frees a ArpNexthopEntry
//
//	Return		:
//		Pointer to new ArpNexthopEntry NULL if out
//
static void 
hknife_nhhash_free( void *ptr )
{

	free( ptr );

}

//
//	Function 	:
//		hknife_nhhash_hash()
//
//	Purpose		:
//
//		Hashes a ArpNexthopEntry
//
//	Return		:
//		Pointer to new ArpNexthopEntry NULL if out
//
static int
hknife_nhhash_hash( void *key )
{
	unsigned long int hash;

	hash = ((ArpNexthopEntry *)key)->gw ^
	       (((ArpNexthopEntry *)key)->gw >> 8) ^
	       (((ArpNexthopEntry *)key)->gw >> 16) ^
	       (((ArpNexthopEntry *)key)->gw >> 24);

	return( (int)(hash & HKNIFE_HASH_MASK) );

}

//
//	Function 	:
//		hknife_nhhash_data2key()
//
//	Purpose		:
//
//		Converts a ArpNexthopEntry into an ArpNexthopEntryKey
//
//	Return		:
//		Pointer to new ArpNexthopEntry NULL if out
//
static void *
hknife_nhhash_data2key( void *ptr )
{
	return( ptr );
}

//
//	Function 	:
//		hknife_nhhash_l2_keycmp()
//
//	Purpose		:
//
//		Compares two ArpNexthopEntry Keys. This compare routine
//    is used when finding the nh_id/l2_id only
//
//	Return		:
//		>0 if key1 > key2
//    =0 if key1 == key2
//    <0 if key1 < key2
//
static int 
hknife_nhhash_l2_keycmp( void *key1, void *key2 )
{
	int result;

	result = ((ArpNexthopEntry *)key1)->gw - ((ArpNexthopEntry *)key2)->gw;
	if( result == 0 ) {
		/* Compare port */
		result = ((ArpNexthopEntry *)key1)->port_id - ((ArpNexthopEntry *)key2)->port_id;
		if( result == 0 ) {
			/* Conpare VLANs */
			result = ((ArpNexthopEntry *)key1)->vlan_id - ((ArpNexthopEntry *)key2)->vlan_id;
			if( result != 0 ) {
				result = (result < 0) ? -10 : 10;
			}
		} else {
			result = (result < 0) ? -100 : 100;
		}
	} else {
		result = (result < 0) ? -1000 : 1000;
	}

	return( result );
}


//
//	Function 	:
//		hknife_nhhash_route_keycmp()
//
//	Purpose		:
//
//		Compares two ArpNexthopEntry Keys. This compare routine
//    is used when finding if the route exists
//
//	Return		:
//		>0 if key1 > key2
//    =0 if key1 == key2
//    <0 if key1 < key2
//
static int 
hknife_nhhash_route_keycmp( void *key1, void *key2 )
{
	int result;

	result = ((ArpNexthopEntry *)key1)->gw - ((ArpNexthopEntry *)key2)->gw;
	if( result == 0 ) {
		/* Compare port */
		result = ((ArpNexthopEntry *)key1)->port_id - ((ArpNexthopEntry *)key2)->port_id;
		if( result == 0 ) {
			/* Conpare VLANs */
			result = ((ArpNexthopEntry *)key1)->vlan_id - ((ArpNexthopEntry *)key2)->vlan_id;
			if( result == 0 ) {
				/* Compare ip */
				result = ((ArpNexthopEntry *)key1)->ip - ((ArpNexthopEntry *)key2)->ip;
				if( result != 0 ) {
					result = (result < 0) ? -1 : 1;
				}
			} else {
				result = (result < 0) ? -10 : 10;
			}
		} else {
			result = (result < 0) ? -100 : 100;
		}
	} else {
		result = (result < 0) ? -1000 : 1000;
	}

	return( result );
}


//
//	Function 	:
//		hknife_nhhash_hashtest()
//
//	Purpose		:
//
//		Tests the hash table implementation
//
//	Return		:
//		NONE
//
#if 0
static void
hknife_nhhash_hashtest( void )
{

	int i;
	ArpNexthopEntry testData1, testData2, testData3;
	ArpNexthopEntry *entry[HKNIFE_MAX_NUM_NEXTHOPS], *extraEntry;

	testData1.ip = 1;
	testData2.ip = 2;
	testData3.ip = 2;

	HashInsert( hknife_globals.arpNexthopHash,
	            (void *)&testData1,
	            hknife_nhhash_hash,
	            hknife_nhhash_data2key,
	            hknife_nhhash_l2_keycmp,
	            0 );

	HashInsert( hknife_globals.arpNexthopHash,
	            (void *)&testData2,
	            hknife_nhhash_hash,
	            hknife_nhhash_data2key,
	            hknife_nhhash_l2_keycmp,
	            0 );

	HashFind( hknife_globals.arpNexthopHash,
	          (void *)&testData3,
	          hknife_nhhash_hash,
	          hknife_nhhash_data2key,
	          hknife_nhhash_l2_keycmp,
	          0);

	HashDestroy( hknife_globals.arpNexthopHash,
		           HKNIFE_NUM_NEXTHOP_BUCKETS,
		           hknife_nhhash_free );

	hknife_init_nhhash_entries();

	for( i = 0; i < HKNIFE_MAX_NUM_NEXTHOPS; i++ )
	{
		entry[i] = hknife_alloc_nhhash_entry();
	}

	extraEntry = hknife_alloc_nhhash_entry();

	for( i = 0; i < HKNIFE_MAX_NUM_NEXTHOPS; i++ )
	{
		hknife_free_nhhash_entry( entry[i] );
	}

	for( i = 0; i < HKNIFE_MAX_NUM_NEXTHOPS; i++ )
	{
		entry[i] = hknife_alloc_nhhash_entry();
	}
	
}
#endif /* 0 */

//
//	Function 	:
//		hknife_init_nhhash_entries()
//
//	Purpose		:
//
//		Tests the hash table implementation
//
//	Return		:
//		NONE
//

static void
hknife_init_nhhash_entries( void )
{

	int i;

	/* Link the array */
	for( i = 0; (i + 1) < HKNIFE_MAX_NUM_NEXTHOPS; i++ ) {
		hknife_globals.arpNextHopEntry[i].hashPtrs.next = 
		                        (void *)&hknife_globals.arpNextHopEntry[i + 1];
	}

	hknife_globals.arpNextHopEntry[i].hashPtrs.next = 0x00000000;

	hknife_globals.head = &hknife_globals.arpNextHopEntry[0];
	hknife_globals.tail = 
	            &hknife_globals.arpNextHopEntry[HKNIFE_MAX_NUM_NEXTHOPS - 1];
}

//
//	Function 	:
//		hknife_alloc_nhhash_entry()
//
//	Purpose		:
//
//		Allocates a nhhash node
//
//	Return		:
//		NONE
//
static ArpNexthopEntry *
hknife_alloc_nhhash_entry( void )
{

	ArpNexthopEntry *result;

	result = hknife_globals.head;
	if( hknife_globals.head != NULL ) {
		hknife_globals.head = 
		                (ArpNexthopEntry *)(hknife_globals.head->hashPtrs.next);
	}

	return( result );
}


//
//	Function 	:
//		hknife_free_nhhash_entry()
//
//	Purpose		:
//
//		Frees a nhhash node
//
//	Return		:
//		NONE
//
static void
hknife_free_nhhash_entry( ArpNexthopEntry *node )
{
	if( hknife_globals.head != NULL ) {
		hknife_globals.tail->hashPtrs.next = (void *)node;
		hknife_globals.tail = node;
	} else {
		hknife_globals.tail = hknife_globals.head = (void *)node;
	}

	node->hashPtrs.next = (void *)0x00000000;
}


