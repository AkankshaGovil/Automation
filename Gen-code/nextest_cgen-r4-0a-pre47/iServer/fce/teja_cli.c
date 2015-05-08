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

#include <tejatypedefs.h>
#include <cpdpcommon.h>
#include <cpdpapi.h>
#include <teja_os_include_all.h>
#include <teja_cli.h>

#ifndef TEJA_MAIN

#include "../../include/srvrlog.h"

#include <trace.h>

#define printf	trc_noop

#endif

int teja_cli_send_cmd(TejaSocket socket, char *cmd);

cli_info_t info;

/* *INDENT-OFF* */
char *TejaCPDP_response_code_string[] =
{ 
  "NO_ERR", 
  "ERR_INTERNAL", 
  "ERR_DUPLICATE_ENTRY", 
  "ERR_TABLE_FULL",
  "ERR_ENTRY_NOT_FOUND", 
  "ERR_OUT_OF_RANGE", 
  "ERR_INVALID_ENTRY", 
  "ERR_UNSUPPORTED", 
  "ERR_INVALID_PARAMETERS",
  "ERR_OUT_OF_RESOURCES", 
  "ERR_ENTRY_EXISTS", 
  "ERR_CONFLICTING_ENTRY", 
  "ERR_ENTRY_IN_USE",
  "ERR_INVALID_NEXTHOP_INFO", 
  "ERR_NHID_IN_USE", 
  "ERR_NULL", 
  "ERR_INVALID_ALGO", 
  "ERR_HASH_TABLE_FULL",
  "ERR_INPUT_STRING_TOO_LONG", 
  "ERR_USER_REQUEST_TO_QUIT", 
  "ERR_ALLOCATE_ALERT_FAILED", 
  "ERR_SEND_ALERT_FAILED",
  "ERR_REQUEST_FAIL", 
  "ERR_BAD_ALERT", 
  "ERR_INVALID_ALERT_PARAMETER", 
  "ERR_ENTRY_TYPE_NOT_HANDLED",
  "ERR_BAD_MASK", 
  "ERR_BLADE_UNINITIALIZED", 
  "ERR_ALERT_ALREADY_IN_USE", 
  "ERR_ALERT_FULL",
  "ERR_INVALID_GET_CALL", 
  "ERR_GET_ALL_DONE", 
  "ERR_BUFFER_FULL", 
  "ERR_IP_FRAG" 
};

/* *INDENT-ON* */

teja_uint32_t
add_interface(void)
{
	teja_uint32_t ret_code = 0;
	teja_uint32_t blade_id;
	teja_uint32_t port_id;
	teja_uint32_t nh_id;
	teja_uint32_t ip_addr;
	teja_uint32_t mask;

	if ((ret_code = get_blade_id(&blade_id) != 0)) {
		printf("\nadd_interface: get_blade_id failed error %d", ret_code);
	} else if ((ret_code = get_port_id(&port_id) != 0)) {
		printf("\nadd_interface: get_port_id failed error %d", ret_code);
	} else if ((ret_code = get_ip_addr(&ip_addr) != 0)) {
		printf("\nadd_interface: get_ip_addr failed error %d", ret_code);
	} else if ((ret_code = get_ip_mask(&mask) != 0)) {
		printf("\nadd_interface: get_ip_mask failed error %d", ret_code);
	} else if ((ret_code = get_nh_id(&nh_id)) != 0) {
		printf("\nadd_interface: get_next_hop_id failed with error %d",
				ret_code);
	}
	#ifndef CLI_APP
		else if ((ret_code =
				  TejaCPDP_ipv4_add_interface(info.ctx, blade_id, port_id,
											  ip_addr, mask)) != 0) {
			printf("\nadd_interface:TejaCPDP_add_ipv4_interface:failed error %d", ret_code);
		} else {
			TejaCPDP_next_hop_info_t next_hop_info;

			next_hop_info.blade_id = blade_id;
			next_hop_info.l2_index = TEJACPDP_LAN_L2_INDEX;
			next_hop_info.port_id = port_id;
			next_hop_info.mtu = 1500;
			next_hop_info.flags = 0;
			next_hop_info.next_hop_ip_addr = ip_addr & mask;
			next_hop_info.l2_index_type = 0;

			if ((ret_code =
				 TejaCPDP_ipv4_add_next_hop(	info.ctx,
												nh_id,
												&next_hop_info)) != 0) {
				printf(	"\nadd_interface: TejaCPDP_ipv4_add_next_hop: "
						"failed error %d",
						ret_code);
			} else
				//
				// add route for the LAN 
				//
				if ((ret_code =
					 TejaCPDP_ipv4_add_route(	info.ctx,
												ip_addr & mask,
												mask,
												nh_id)) != 0) {
				printf(	"\nadd_interface: TejaCPDP_ipv4_add_route: "
						"failed error %d",
					   	ret_code);
			} else
				//
				// add route for the local IP address 
				//
				if ((ret_code =
					 TejaCPDP_ipv4_add_route(	info.ctx,
												ip_addr,
												0xFFFFFFFF,
												TEJACPDP_NEXTHOP_LOCAL)) != 0) {
				printf(	"\nadd_interface: TejaCPDP_ipv4_add_route: "
						"failed error %d",
					   	ret_code);
			} else if ((ret_code =
					  TejaCPDP_get_last_transaction_id(info.ctx,
										   &info.last_transaction_id)) != 0) {
				printf("\nadd_interface: Get last transaction ID failed");
			} else {
				info.num_pending_transaction = 4;
				memset( info.request_string, (int) 0, USER_INPUT_BUFFER_SIZE );
				memcpy(	info.request_string,
						"add_interface",
						strlen("add_interface") + 1);
			}
		}
	#else /* CLI_APP */
		else {
			memset( info.request_cmd, (int) 0, USER_INPUT_BUFFER_SIZE );
			sprintf(	info.request_cmd,
						"%c %d %d %d.%d.%d.%d %d.%d.%d.%d %d",
						CMD_ADD_INTERFACE,
						blade_id,
						port_id,
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
			ret_code = teja_cli_send_cmd(	info.cli_app_socket,
										info.request_cmd);
		}
	#endif /* CLI_APP */

	return ret_code;
}

teja_uint32_t
delete_interface(void)
{
	teja_uint32_t ret_code = 0;
	teja_uint32_t blade_id;
	teja_uint32_t port_id;
	teja_uint32_t nh_id;
	teja_uint32_t ip_addr;
	teja_uint32_t mask;

	if ((ret_code = get_blade_id(&blade_id) != 0)) {
		printf("\ndelete_interface: get_blade_id failed error %d", ret_code);
	} else if ((ret_code = get_port_id(&port_id) != 0)) {
		printf("\ndelete_interface: get_port_id failed error %d", ret_code);
	} else if ((ret_code = get_ip_addr(&ip_addr) != 0)) {
		printf("\ndelete_interface: get_ip_addr failed error %d", ret_code);
	} else if ((ret_code = get_ip_mask(&mask) != 0)) {
		printf("\ndelete_interface: get_ip_mask failed error %d", ret_code);
	} else if ((ret_code = get_nh_id(&nh_id)) != 0) {
		printf(	"\ndelete_interface: get_next_hop_id failed with error %d",
				ret_code);
	}
	#ifndef CLI_APP
		else if ((ret_code =
				  TejaCPDP_ipv4_delete_interface(	info.ctx,
													blade_id,
													port_id,
													ip_addr)) != 0) {
			printf(	"\ndelete_interface:TejaCPDP_delete_ipv4_interface:"
					"failed error %d",
				 	ret_code);
		} else {
			if ((ret_code = 
				TejaCPDP_ipv4_delete_route(	info.ctx,
											ip_addr & mask,
											mask)) != 0) {
				printf(	"\ndelete_interface: TejaCPDP_ipv4_delete_route: "
						"failed error %d",
					 	ret_code);
			} else if ((ret_code =
				  TejaCPDP_ipv4_delete_route(	info.ctx,
												ip_addr,
												0xFFFFFFFF)) != 0) {
				printf(	"\ndelete_interface: TejaCPDP_ipv4_delete_route: "
						"failed error %d",
						ret_code);
			} else if ((ret_code = 
				TejaCPDP_ipv4_delete_next_hop(	info.ctx,
												nh_id)) != 0) {
				printf(	"\ndelete_interface: TejaCPDP_ipv4_delete_next_hop: "
						"failed error %d",
					 	ret_code);
			} else if ((ret_code =
				TejaCPDP_get_last_transaction_id(	info.ctx,
											&info.last_transaction_id)) != 0) {
				printf("\ndelete_interface: Get last transaction ID failed");
			} else {
				info.num_pending_transaction = 4;
				memset( info.request_string, (int) 0, USER_INPUT_BUFFER_SIZE );
				memcpy(	info.request_string,
						"delete_interface",
						strlen("delete_interface") + 1);
			}
		}
	#else /* #ifndef CLI_APP */
		else {
			memset( info.request_cmd, (int) 0, USER_INPUT_BUFFER_SIZE );
			sprintf(	info.request_cmd,
						"%c %d %d %d.%d.%d.%d %d.%d.%d.%d %d",
						CMD_DELETE_INTERFACE,
						blade_id,
						port_id,
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
					"delete_interface",
					strlen("delete_interface") + 1);
			ret_code = teja_cli_send_cmd(	info.cli_app_socket,
										info.request_cmd);
		}
	#endif /* #ifndef CLI_APP */

	return ret_code;
}

teja_uint32_t
add_route(void)
{
	teja_uint32_t ret_code;
	teja_uint32_t ip_addr;
	teja_uint32_t mask;
	teja_uint32_t nh_id;

	ret_code = 0;
	if ((ret_code = get_ip_addr(&ip_addr) != 0)) {
		printf("\nadd_route: get_ip_addr failed error %d", ret_code);
	} else if ((ret_code = get_ip_mask(&mask) != 0)) {
		printf("\nadd_route: get_ip_mask failed error %d", ret_code);
	} else {
		// get nexthop id 
		ret_code = get_nh_id(&nh_id);
	}

	if (ret_code == 0) {
		#ifndef CLI_APP
			if ((ret_code =
				TejaCPDP_ipv4_add_route(	info.ctx,
											ip_addr,
											mask,
											nh_id)) != 0) {
				printf(	"\nadd_route: TejaCPDP_ipv4_add_route "
						"failed error %d",
						ret_code);
			} else if ((ret_code =
				  TejaCPDP_get_last_transaction_id(	info.ctx,
										   &info.last_transaction_id)) != 0) {
				printf("\nadd_route: Get last transaction ID failed");
			} else {
				info.num_pending_transaction = 1;
				memset( info.request_string, (int) 0, USER_INPUT_BUFFER_SIZE );
				memcpy(	info.request_string,
						"add_route",
						strlen("add_route") + 1);
			}
		#else /* #ifndef CLI_APP */
			memset( info.request_cmd, (int) 0, USER_INPUT_BUFFER_SIZE );
			sprintf(	info.request_cmd,
						"%c %d.%d.%d.%d %d.%d.%d.%d %d",
						CMD_ADD_ROUTE,
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
					"add_route",
					strlen("add_route") + 1);
			ret_code = teja_cli_send_cmd(	info.cli_app_socket,
										info.request_cmd );

		#endif /* #ifndef CLI_APP */
	}
	return ret_code;
}

teja_uint32_t
update_route(void)
{
	teja_uint32_t ret_code;
	teja_uint32_t route_addr;
	teja_uint32_t mask;
	teja_uint32_t nh_id;

	ret_code = 0;

	if ((ret_code = get_ip_addr(&route_addr) != 0)) {
		printf("\nupdate_route: get_ip_addr failed error %d", ret_code);
	} else if ((ret_code = get_ip_mask(&mask) != 0)) {
		printf("\nupdate_route: get_ip_mask failed error %d", ret_code);
	} else {

		// get nexthop id 
		ret_code = get_nh_id(&nh_id);
	}

	if (ret_code == 0) {
		#ifndef CLI_APP
			if ((ret_code =
			 	TejaCPDP_ipv4_update_route(	info.ctx,
											route_addr,
											mask,
											nh_id)) != 0) {
			printf(	"\nupdate_route: TejaCPDP_ipv4_update_route "
					"failed error %d",
					ret_code);
			} else if ((ret_code =
				  TejaCPDP_get_last_transaction_id(info.ctx,
										   &info.last_transaction_id)) != 0) {
				printf("\nupdate_route: Get last transaction ID failed");
			} else {
				info.num_pending_transaction = 1;
				memset( info.request_string, (int) 0, USER_INPUT_BUFFER_SIZE );
				memcpy(	info.request_string,
						"update_route",
						strlen("update_route") + 1);
			}
		#else /* #ifndef CLI_APP */
			memset( info.request_cmd, (int) 0, USER_INPUT_BUFFER_SIZE );
			sprintf(	info.request_cmd,
						"%c %d.%d.%d.%d %d.%d.%d.%d %d",
						CMD_UPDATE_ROUTE,
						(route_addr >> 24) & 0xff,
						(route_addr >> 16) & 0xff,
						(route_addr >> 8) & 0xff,
						route_addr & 0xff,
						(mask >> 24) & 0xff,
						(mask >> 16) & 0xff,
						(mask >> 8) & 0xff,
						mask & 0xff,
						nh_id);
			memset( info.request_string, (int) 0, USER_INPUT_BUFFER_SIZE );
			memcpy(	info.request_string,
					"update_route",
					strlen("update_route") + 1);
			ret_code = teja_cli_send_cmd(	info.cli_app_socket,
										info.request_cmd);
		#endif /* #ifndef CLI_APP */
	}
	return ret_code;
}

teja_uint32_t
delete_route(void)
{
	teja_uint32_t ret_code;
	teja_uint32_t route_addr;
	teja_uint32_t mask;

	ret_code = 0;

	if ((ret_code = get_ip_addr(&route_addr) != 0)) {
		printf("\ndelete_route: get_ip_addr failed error %d", ret_code);
	} else if ((ret_code = get_ip_mask(&mask) != 0)) {
		printf("\ndelete_route: get_ip_mask failed error %d", ret_code);
	} 
	#ifndef CLI_APP
		else if ((ret_code =
			TejaCPDP_ipv4_delete_route(	info.ctx,
										route_addr,
										mask)) != 0) {
			printf(	"\ndelete_route: TejaCPDP_ipv4_delete_route "
					"failed error %d", ret_code);
		}
		else if ((ret_code =
			TejaCPDP_get_last_transaction_id(	info.ctx,
										&info.last_transaction_id)) != 0) {
			printf("\ndelete_route: Get last transaction ID failed");
		} else {
			info.num_pending_transaction = 1;
			memset( info.request_string, (int) 0, USER_INPUT_BUFFER_SIZE );
			memcpy(	info.request_string,
					"delete_route",
					strlen("delete_route") + 1);
		}
	#else /* #ifndef CLI_APP */
		else {
			memset( info.request_cmd, (int) 0, USER_INPUT_BUFFER_SIZE );
			sprintf(	info.request_cmd,
						"%c %d.%d.%d.%d %d.%d.%d.%d",
						CMD_DELETE_ROUTE,
						(route_addr >> 24) & 0xff,
						(route_addr >> 16) & 0xff,
						(route_addr >> 8) & 0xff,
						route_addr & 0xff,
						(mask >> 24) & 0xff,
						(mask >> 16) & 0xff,
						(mask >> 8) & 0xff,
						mask & 0xff);
			memset( info.request_string, (int) 0, USER_INPUT_BUFFER_SIZE );
			memcpy(	info.request_string,
					"delete_route",
					strlen("delete_route") + 1);
			ret_code = teja_cli_send_cmd(	info.cli_app_socket,
										info.request_cmd);
		}
	#endif /* #ifndef CLI_APP */

	return ret_code;
}

#ifdef TEJA_MAIN

teja_uint32_t
add_redirect(void)
{
	teja_uint32_t ret_code;
	TejaCPDP_redirect_key_t redirect_key;
	TejaCPDP_redirect_info_t redirect_info;

	ret_code = 0;

	// get redirect key of redirect to be added 

	if ((ret_code = get_redirect_key(&redirect_key)) != 0) {
		printf("\nadd_redirect: get_redirect_key failed. error %d", ret_code);
	} else if ((ret_code = get_redirect_info(&redirect_info)) != 0) {
		printf("\nadd_redirect: get_redirect_info failed. error %d", ret_code);
	}
	#ifndef CLI_APP
		else if ((ret_code = TejaCPDP_ipv4_add_redirect(info.ctx,
														&redirect_key,
														&redirect_info)) != 0) {
			printf(	"\nadd_redirect: TejaCPDP_ipv4_add_redirect: "
					"failed error %d", ret_code);
		} else {
			info.num_pending_transaction = 1;
			memset( info.request_string, (int) 0, USER_INPUT_BUFFER_SIZE );
			memcpy(	info.request_string,
					"add_redirect",
					strlen("add_redirect") + 1);
		}
	#else /* #ifndef CLI_APP */
		else {

			memset( info.request_cmd, (int) 0, USER_INPUT_BUFFER_SIZE );
			sprintf( info.request_cmd,
					"%c %d %d.%d.%d.%d %d %d %d.%d.%d.%d %d %d.%d.%d.%d %d %d",
					CMD_ADD_NAT_REDIRECT,
					redirect_key.port_id,
					(redirect_key.ip_daddr >> 24) & 0xff,
					(redirect_key.ip_daddr >> 16) & 0xff,
					(redirect_key.ip_daddr >> 8) & 0xff,
					(redirect_key.ip_daddr) & 0xff,
					redirect_key.l4_dport,
					redirect_key.l4_protocol,
					(redirect_info.new_ip_saddr >> 24) & 0xff,
					(redirect_info.new_ip_saddr >> 16) & 0xff,
					(redirect_info.new_ip_saddr >> 8) & 0xff,
					(redirect_info.new_ip_saddr) & 0xff,
					redirect_info.new_l4_sport,
					(redirect_info.new_ip_daddr >> 24) & 0xff,
					(redirect_info.new_ip_daddr >> 16) & 0xff,
					(redirect_info.new_ip_daddr >> 8) & 0xff,
					(redirect_info.new_ip_daddr) & 0xff,
					redirect_info.new_l4_dport,
					redirect_info.egress_port_id );

			memset( info.request_string, (int) 0, USER_INPUT_BUFFER_SIZE );
			memcpy(	info.request_string,
					"add_redirect",
					strlen("add_redirect") + 1);
			ret_code = teja_cli_send_cmd(	info.cli_app_socket,
										info.request_cmd);
		}
	#endif /* #ifndef CLI_APP */

	return ret_code;
}

#else

teja_uint32_t
add_redirect( int32_t ingress_blade, uint32_t ip_daddr, uint16_t l4_dport, uint8_t l4_protocol,
							uint32_t new_ip_saddr, uint32_t new_l4_sport,
							uint32_t new_ip_daddr, uint32_t new_l4_dport,
							int32_t egress_blade )
{
	teja_uint32_t ret_code;

	ret_code = 0;

	// Fill in redirect info

	memset( info.request_cmd, (int) 0, USER_INPUT_BUFFER_SIZE );
	sprintf( info.request_cmd,
					"%c %d %d.%d.%d.%d %d %d %d.%d.%d.%d %d %d.%d.%d.%d %d %d",
					CMD_ADD_NAT_REDIRECT,
					ingress_blade,
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
					egress_blade );

	memset( info.request_string, (int) 0, USER_INPUT_BUFFER_SIZE );
	memcpy(	info.request_string,
					"add_redirect",
					strlen("add_redirect") + 1);

	if ( ( ret_code = teja_cli_send_cmd(	info.cli_app_socket,
											info.request_cmd) ) != 0 )
		return ret_code;

	return( ret_code = wait_for_response() );
}

#endif

#ifdef TEJA_MAIN

teja_uint32_t
update_redirect(void)
{
	teja_uint32_t ret_code;
	TejaCPDP_redirect_key_t redirect_key;
	TejaCPDP_redirect_info_t redirect_info;

	ret_code = 0;

	// get redirect key of redirect to be updated 

	if ((ret_code = get_redirect_key(&redirect_key)) != 0) {
		printf(	"\nupdate_redirect: get_redirect_key failed. error %d",
				ret_code);
	}

	// get redirect update info 

	if ((ret_code = get_redirect_info(&redirect_info)) != 0) {
		printf(	"\nupdate_redirect: get_redirect_info failed. error %d",
				ret_code);
	}
	#ifndef CLI_APP
		else if ((ret_code =
			  TejaCPDP_ipv4_update_redirect(	info.ctx,
												&redirect_key,
												&redirect_info)) != 0) {
			printf(	"\nupdate_redirect: TejaCPDP_ipv4_update_redirect: "
					"failed error %d",
			   		ret_code);
		} else {
			info.num_pending_transaction = 1;
			memset( info.request_string, (int) 0, USER_INPUT_BUFFER_SIZE );
			memcpy(	info.request_string,
					"update_redirect",
					strlen("update_redirect") + 1);
		}
	#else /* #ifndef CLI_APP */
		else {
			memset( info.request_cmd, (int) 0, USER_INPUT_BUFFER_SIZE );
			sprintf(info.request_cmd,
					"%c %d %d.%d.%d.%d %d %d %d.%d.%d.%d %d %d.%d.%d.%d %d %d",
					CMD_UPDATE_NAT_REDIRECT,
					redirect_key.port_id,
					(redirect_key.ip_daddr >> 24) & 0xff,
					(redirect_key.ip_daddr >> 16) & 0xff,
					(redirect_key.ip_daddr >> 8) & 0xff,
					(redirect_key.ip_daddr) & 0xff,
					redirect_key.l4_dport,
					redirect_key.l4_protocol,
					(redirect_info.new_ip_saddr >> 24) & 0xff,
					(redirect_info.new_ip_saddr >> 16) & 0xff,
					(redirect_info.new_ip_saddr >> 8) & 0xff,
					(redirect_info.new_ip_saddr) & 0xff,
					redirect_info.new_l4_sport,
					(redirect_info.new_ip_daddr >> 24) & 0xff,
					(redirect_info.new_ip_daddr >> 16) & 0xff,
					(redirect_info.new_ip_daddr >> 8) & 0xff,
					(redirect_info.new_ip_daddr) & 0xff,
					redirect_info.new_l4_dport, redirect_info.egress_port_id);

			memset( info.request_string, (int) 0, USER_INPUT_BUFFER_SIZE );
			memcpy(	info.request_string,
					"update_redirect",
					strlen("update_redirect") + 1);
			ret_code = teja_cli_send_cmd(	info.cli_app_socket,
										info.request_cmd);
		}
	#endif /* #ifndef CLI_APP */
	return ret_code;
}

#else

teja_uint32_t
update_redirect( int32_t ingress_blade, uint32_t ip_daddr, uint16_t l4_dport, uint8_t l4_protocol,
							uint32_t new_ip_saddr, uint32_t new_l4_sport,
							uint32_t new_ip_daddr, uint32_t new_l4_dport,
							int32_t egress_blade )
{
	teja_uint32_t ret_code;

	ret_code = 0;

	// get redirect key of redirect to be updated 

	memset( info.request_cmd, (int) 0, USER_INPUT_BUFFER_SIZE );
	sprintf(info.request_cmd,
					"%c %d %d.%d.%d.%d %d %d %d.%d.%d.%d %d %d.%d.%d.%d %d %d",
					CMD_UPDATE_NAT_REDIRECT,
					ingress_blade,
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
					egress_blade );

	memset( info.request_string, (int) 0, USER_INPUT_BUFFER_SIZE );
	memcpy(	info.request_string,
					"update_redirect",
					strlen("update_redirect") + 1);

	if ( ( ret_code = teja_cli_send_cmd(	info.cli_app_socket,
											info.request_cmd) ) != 0 )
		return ret_code;

	return( ret_code = wait_for_response() );
}

#endif

#ifdef TEJA_MAIN

teja_uint32_t
delete_redirect( void )
{
	teja_uint32_t ret_code;
	TejaCPDP_redirect_key_t redirect_key;

	ret_code = 0;

	// get redirect key of redirect to be deleted 

	if ((ret_code = get_redirect_key(&redirect_key)) != 0) {
		printf("\ndelete_redirect: get_redirect_key failed. error %d",
		ret_code);
	}
	#ifndef CLI_APP
		else if ((ret_code =
			TejaCPDP_ipv4_delete_redirect(	info.ctx,
											&redirect_key)) != 0) {
			printf(	"\ndelete_redirect: TejaCPDP_ipv4_delete_redirect: "
					"failed error %d",
					ret_code);
		} else {
			info.num_pending_transaction = 1;
			memset( info.request_string, (int) 0, USER_INPUT_BUFFER_SIZE );
			memcpy(	info.request_string,
					"delete_redirect",
					strlen("delete_redirect") + 1);
		}
	#else /* #ifndef CLI_APP */
		else {
			memset( info.request_cmd, (int) 0, USER_INPUT_BUFFER_SIZE );
			sprintf(info.request_cmd,
				"%c %d %d.%d.%d.%d %d %d",
				CMD_DELETE_NAT_REDIRECT,
				redirect_key.port_id,
				(redirect_key.ip_daddr >> 24) & 0xff,
				(redirect_key.ip_daddr >> 16) & 0xff,
				(redirect_key.ip_daddr >> 8) & 0xff,
				(redirect_key.ip_daddr) & 0xff,
				redirect_key.l4_dport, redirect_key.l4_protocol);
			memset( info.request_string, (int) 0, USER_INPUT_BUFFER_SIZE );
			memcpy(	info.request_string,
					"delete_redirect",
					strlen("delete_redirect") + 1);
			ret_code = teja_cli_send_cmd(	info.cli_app_socket,
										info.request_cmd);
		}
	#endif /* #ifndef CLI_APP */

	return ret_code;
}

#else

teja_uint32_t
delete_redirect( int32_t ingress_blade, uint32_t ip_daddr, uint16_t l4_dport, uint8_t l4_protocol )
{
	teja_uint32_t ret_code;

	ret_code = 0;

	memset( info.request_cmd, (int) 0, USER_INPUT_BUFFER_SIZE );
	sprintf(info.request_cmd,
				"%c %d %d.%d.%d.%d %d %d",
				CMD_DELETE_NAT_REDIRECT,
				ingress_blade,
				(ip_daddr >> 24) & 0xff,
				(ip_daddr >> 16) & 0xff,
				(ip_daddr >> 8) & 0xff,
				(ip_daddr) & 0xff,
				l4_dport,
				l4_protocol );
	memset( info.request_string, (int) 0, USER_INPUT_BUFFER_SIZE );
	memcpy(	info.request_string,
					"delete_redirect",
					strlen("delete_redirect") + 1);

	if ( ( ret_code = teja_cli_send_cmd(	info.cli_app_socket,
											info.request_cmd) ) != 0 )
		return ret_code;

	return( ret_code = wait_for_response() );
}

#endif

teja_uint32_t
add_nexthop(void)
{
	teja_uint32_t ret_code;
	TejaCPDP_next_hop_info_t next_hop_info;
	teja_uint32_t nh_id;

	ret_code = 0;

	// get nexthop id 

	ret_code = get_nh_id(&nh_id);

	// get nexthop info 

	if ((ret_code = get_nexthop_info(&next_hop_info)) != 0) {
		printf("\nadd_nexthop: get_nexthop_info failed. error %d", ret_code);
	}
	#ifndef CLI_APP
		else if ((ret_code =
			  TejaCPDP_ipv4_add_next_hop(	info.ctx,
											nh_id,
											&next_hop_info)) != 0) {
			printf(	"\nadd_nexthop: TejaCPDP_ipv4_add_next_hop: "
					"failed error %d", ret_code);
		} else {
			info.num_pending_transaction = 1;
			memset( info.request_string, (int) 0, USER_INPUT_BUFFER_SIZE );
			memcpy(	info.request_string,
					"add_nexthop",
					strlen("add_nexthop") + 1);
		}
	#else /* #ifndef CLI_APP */
		else {
			memset( info.request_cmd, (int) 0, USER_INPUT_BUFFER_SIZE );
			sprintf(info.request_cmd, "%c %d %d.%d.%d.%d %d %d %d %d %d",
				CMD_ADD_NEXTHOP, nh_id,
				(next_hop_info.next_hop_ip_addr >> 24) & 0xff,
				(next_hop_info.next_hop_ip_addr >> 16) & 0xff,
				(next_hop_info.next_hop_ip_addr >> 8) & 0xff,
				next_hop_info.next_hop_ip_addr & 0xff,
				next_hop_info.blade_id, next_hop_info.port_id,
				next_hop_info.mtu, next_hop_info.flags, next_hop_info.l2_index);
			memset( info.request_string, (int) 0, USER_INPUT_BUFFER_SIZE );
			memcpy(	info.request_string,
					"add_nexthop",
					strlen("add_nexthop") + 1);
			ret_code = teja_cli_send_cmd(	info.cli_app_socket,
										info.request_cmd);
		}
	#endif /* #ifndef CLI_APP */

	return ret_code;
}

teja_uint32_t
update_nexthop(void)
{
	teja_uint32_t ret_code;
	TejaCPDP_next_hop_info_t next_hop_info;
	teja_uint32_t nh_id;

	ret_code = 0;

	// get nexthop id 

	ret_code = get_nh_id(&nh_id);

	// get nexthop info 

	if ((ret_code = get_nexthop_info(&next_hop_info)) != 0) {
		printf(	"\nupdate_nexthop: get_nexthop_info failed. error %d",
				ret_code);
	}
	#ifndef CLI_APP
		else if ((ret_code =
			  TejaCPDP_ipv4_update_next_hop(	info.ctx,
												nh_id,
												&next_hop_info)) != 0) {
			printf(	"\nupdate_nexthop: TejaCPDP_ipv4_update_next_hop: "
					"failed error %d",
					ret_code);
		} else {
			info.num_pending_transaction = 1;
			memset( info.request_string, (int) 0, USER_INPUT_BUFFER_SIZE );
			memcpy(	info.request_string,
					"update_nexthop",
					strlen("update_nexthop") + 1);
		}
	#else /* #ifndef CLI_APP */
		else {

			memset( info.request_cmd, (int) 0, USER_INPUT_BUFFER_SIZE );
			sprintf(info.request_cmd, "%c %d %d.%d.%d.%d %d %d %d %d %d",
				CMD_UPDATE_NEXTHOP, nh_id,
				(next_hop_info.next_hop_ip_addr >> 24) & 0xff,
				(next_hop_info.next_hop_ip_addr >> 16) & 0xff,
				(next_hop_info.next_hop_ip_addr >> 8) & 0xff,
				next_hop_info.next_hop_ip_addr & 0xff,
				next_hop_info.blade_id, next_hop_info.port_id,
				next_hop_info.mtu, next_hop_info.flags, next_hop_info.l2_index);
			memset( info.request_string, (int) 0, USER_INPUT_BUFFER_SIZE );
			memcpy(	info.request_string,
					"update_nexthop",
					strlen("update_nexthop") + 1);
			ret_code = teja_cli_send_cmd(	info.cli_app_socket,
										info.request_cmd);
		}
	#endif /* #ifndef CLI_APP */

	return ret_code;
}

teja_uint32_t
delete_nexthop(void)
{
	teja_uint32_t ret_code;
	teja_uint32_t nh_id;

	ret_code = 0;

	// get nexthop id 

	if ((ret_code = get_nh_id(&nh_id)) != 0) {
		printf("\ndelete_nexthop: get_nh_id: failed error %d", ret_code);
	}
	#ifndef CLI_APP
		else if ((ret_code =
			TejaCPDP_ipv4_delete_next_hop(info.ctx, nh_id)) != 0) {
			printf(	"\ndelete_nexthop: TejaCPDP_ipv4_delete_next_hop: "
				"failed error %d",
			ret_code);
		} else {
			info.num_pending_transaction = 1;
			memset( info.request_string, (int) 0, USER_INPUT_BUFFER_SIZE );
			memcpy(	info.request_string,
					"delete_nexthop",
					strlen("delete_nexthop") + 1);
		}
	#else /* #ifndef CLI_APP */
		else {
			memset( info.request_cmd, (int) 0, USER_INPUT_BUFFER_SIZE );
			sprintf(	info.request_cmd,
						"%c %d",
						CMD_DELETE_NEXTHOP,
						nh_id );
			memset( info.request_string, (int) 0, USER_INPUT_BUFFER_SIZE );
			memcpy(	info.request_string,
					"delete_nexthop",
					strlen("delete_nexthop") + 1);
			ret_code = teja_cli_send_cmd(	info.cli_app_socket,
										info.request_cmd);
		}
	#endif /* #ifndef CLI_APP */

	return ret_code;
}

teja_uint32_t
change_nexthop_mtu(void)
{
	teja_uint32_t ret_code;
	teja_uint32_t nh_id;
	teja_uint32_t mtu;

	ret_code = 0;

	// get nexthop id 

	ret_code = get_nh_id(&nh_id);

	// get nexthop info 

	if ((ret_code = get_mtu(&mtu)) != 0) {
		printf("\nchange_nexthop_mtu: get_mtu failed. error %d", ret_code);
	}
	#ifndef CLI_APP
		else if ((ret_code = TejaCPDP_ipv4_set_mtu(	info.ctx,
													nh_id, mtu)) != 0) {
			printf(	"\nchange_nexthop_mtu: TejaCPDP_ipv4_set_mtu: "
					"failed error %d", ret_code);
		} else {
			info.num_pending_transaction = 1;
			memset( info.request_string, (int) 0, USER_INPUT_BUFFER_SIZE );
			memcpy(	info.request_string,
					"change_nexthop_mtu",
			   		strlen("change_nexthop_mtu") + 1);
		}
	#else /* #ifndef CLI_APP */
		else {
			memset( info.request_cmd, (int) 0, USER_INPUT_BUFFER_SIZE );
			sprintf(info.request_cmd,
					"%c %d %d",
					CMD_CHANGE_NEXTHOP_MTU,
					nh_id,
					mtu);
			memset( info.request_string, (int) 0, USER_INPUT_BUFFER_SIZE );
			memcpy(	info.request_string,
					"change_nexthop_mtu",
			   		strlen("change_nexthop_mtu") + 1);
			ret_code = teja_cli_send_cmd(	info.cli_app_socket,
										info.request_cmd );
		}
	#endif /* #ifndef CLI_APP */

	return ret_code;
}

teja_uint32_t
change_nexthop_flag(void)
{
	teja_uint32_t ret_code;
	teja_uint32_t nh_id;
	teja_uint32_t flag;

	ret_code = 0;

	// get nexthop id 

	ret_code = get_nh_id(&nh_id);

	if ((ret_code = get_flags(&flag)) != 0) {
		printf("\nchange_nexthop_mtu: get_flag failed. error %d", ret_code);
	}
	#ifndef CLI_APP
		else if ((ret_code =
			TejaCPDP_ipv4_set_flag(	info.ctx,
									nh_id,
									flag)) != 0) {
			printf(	"\nchange_nexthop_flag: TejaCPDP_ipv4_set_flag: "
					"failed error %d",
					ret_code);
		} else {
			info.num_pending_transaction = 1;
			memset( info.request_string, (int) 0, USER_INPUT_BUFFER_SIZE );
			memcpy(	info.request_string,
					"change_nexthop_flag",
			   		strlen("change_nexthop_flag") + 1);
		}
	#else /* #ifndef CLI_APP */
		else {
			memset( info.request_cmd, (int) 0, USER_INPUT_BUFFER_SIZE );
			sprintf(info.request_cmd,
					"%c %d %d",
					CMD_CHANGE_NEXTHOP_FLAG,
					nh_id,
					flag);
			memset( info.request_string, (int) 0, USER_INPUT_BUFFER_SIZE );
			memcpy(	info.request_string,
					"change_nexthop_flag",
			   		strlen("change_nexthop_flag") + 1);
			ret_code = teja_cli_send_cmd(	info.cli_app_socket,
										info.request_cmd);
		}
	#endif /* #ifndef CLI_APP */
	return ret_code;
}

teja_uint32_t
add_static_arp(void)
{
	teja_uint32_t ret_code;
	teja_uint32_t port_id;
	teja_uint32_t l2_index;
	teja_uint32_t ip_addr;
	teja_uint32_t vlan_id;
	TejaCPDP_mac_addr_t mac_addr;

	ret_code = 0;

	if ((ret_code = get_l2_index(&l2_index)) != 0) {
		printf("add_static_arp: get_l2_index failed. error %d\n", ret_code);
		return ret_code;
	}
	if ((ret_code = get_port_id(&port_id)) != 0) {
		printf("add_static_arp: get_port_id failed. error %d\n", ret_code);
		return ret_code;
	} 
	if ((ret_code = get_ip_addr(&ip_addr)) != 0) {
		printf("add_static_arp: get_ip_addr failed. error %d\n", ret_code);
	} 
	if ((ret_code = get_mac_addr(&mac_addr)) != 0) {
		printf("add_static_arp: get_mac_addr failed. error %d\n", ret_code);
	}
	if ((ret_code = get_vlan_id(&vlan_id)) != 0) {
		printf("add_static_arp: get_vlan_id failed. error %d\n", ret_code);
	}
	#ifndef CLI_APP
		else if ((ret_code =
			  TejaCPDP_ipv4_add_arp_entry(	info.ctx,
											port_id,
											l2_index,
											ip_addr,
											&mac_addr,
											vlan_id )) != 0) {
			printf(	"add_static_arp: TejaCPDP_ipv4_add_arp_entry "
					"error %d\n",
					ret_code);
		} else {
			info.num_pending_transaction = 1;
			memset( info.request_string, (int) 0, USER_INPUT_BUFFER_SIZE );
			memcpy(	info.request_string,
					"add_static_arp",
					strlen("add_static_arp") + 1);
		}
	#else /* #ifndef CLI_APP */
		else {
			memset( info.request_cmd, (int) 0, USER_INPUT_BUFFER_SIZE );
			sprintf(info.request_cmd,
				"%c %d %d %d.%d.%d.%d %x:%x:%x:%x:%x:%x %d",
				CMD_ADD_STATIC_ARP, l2_index, port_id,
				(ip_addr >> 24) & 0xff, (ip_addr >> 16) & 0xff,
				(ip_addr >> 8) & 0xff, ip_addr & 0xff, mac_addr.mac[0],
				mac_addr.mac[1], mac_addr.mac[2], mac_addr.mac[3],
				mac_addr.mac[4], mac_addr.mac[5],
				vlan_id );
			memset( info.request_string, (int) 0, USER_INPUT_BUFFER_SIZE );
			memcpy(	info.request_string,
					"add_static_arp",
					strlen("add_static_arp") + 1);
			ret_code = teja_cli_send_cmd(	info.cli_app_socket,
										info.request_cmd);
		}
	#endif /* #ifndef CLI_APP */

	return ret_code;
}

teja_uint32_t
create_arp(void)
{
	teja_uint32_t ret_code;
	teja_uint32_t port_id;
	teja_uint32_t l2_index;
	teja_uint32_t ip_addr;
	teja_uint32_t vlan_id;
	teja_uint32_t perm_flag;

	ret_code = 0;

	if ((ret_code = get_l2_index(&l2_index)) != 0) {
		printf(	"create_arp: get_l2_index failed. error %d\n",
				ret_code);
		return ret_code;
	}
	if ((ret_code = get_port_id(&port_id)) != 0) {
		printf("create_arp: get_port_id failed. error %d\n", ret_code);
		return ret_code;
	}
	if ((ret_code = get_ip_addr(&ip_addr)) != 0) {
		printf("create_arp: get_ip_addr failed. error %d\n", ret_code);
		return ret_code;
	}

	if ((ret_code = get_perm_flag(&perm_flag)) != 0) {
		printf("create_arp: get_perm_flag failed. error %d\n", ret_code);
		return ret_code;
	}

	if ((ret_code = get_vlan_id(&vlan_id)) != 0) {
		printf("create_arp: get_vlan_id failed. error %d\n", ret_code);
		return ret_code;
	}

	#ifndef CLI_APP
		else if ((ret_code =
			  TejaCPDP_ipv4_create_arp_entry(	info.ctx,
												port_id,
												l2_index,
												ip_addr,
												perm_flag,
												vlan_id )) != 0) {
			printf(	"create_arp: TejaCPDP_ipv4_create_arp_entry "
					"error %d\n",
			   		ret_code);
		} else {
			info.num_pending_transaction = 1;
			memset( info.request_string, (int) 0, USER_INPUT_BUFFER_SIZE );
			memcpy(	info.request_string,
					"create_arp",
			   		strlen("create_arp") + 1);
		}
	#else /* #ifndef CLI_APP */
		else {
			memset( info.request_cmd, (int) 0, USER_INPUT_BUFFER_SIZE );
			sprintf(info.request_cmd,
				"%c %d %d %d.%d.%d.%d %d %d",
				CMD_CREATE_ARP,
				l2_index,
				port_id,
				(ip_addr >> 24) & 0xff,
				(ip_addr >> 16) & 0xff,
				(ip_addr >> 8) & 0xff,
				ip_addr & 0xff,
				perm_flag,
				vlan_id );
			memset( info.request_string, (int) 0, USER_INPUT_BUFFER_SIZE );
			memcpy(	info.request_string,
					"create_arp",
			   		strlen("create_arp") + 1);
			ret_code = teja_cli_send_cmd(	info.cli_app_socket,
										info.request_cmd);
		}
	#endif /* #ifndef CLI_APP */

	return ret_code;
}

teja_uint32_t
delete_arp(void)
{
	teja_uint32_t ret_code;
	teja_uint32_t l2_index;

	ret_code = 0;

	if ((ret_code = get_l2_index(&l2_index)) != 0) {
		printf("\ndelete_arp: get_l2_index failed. error %d", ret_code);
	}
	#ifndef CLI_APP
		else if ((ret_code = 
			TejaCPDP_ipv4_delete_arp_entry(info.ctx, l2_index)) != 0) {
			printf(	"\ndelete_arp: TejaCPDP_ipv4_delete_arp_entry "
					"error %d", ret_code);
		} else {
			info.num_pending_transaction = 1;
			memset( info.request_string, (int) 0, USER_INPUT_BUFFER_SIZE );
			memcpy(	info.request_string,
					"delete_arp",
					strlen("delete_arp") + 1);
		}
	#else /* #ifndef CLI_APP */
		else {
			memset( info.request_cmd, (int) 0, USER_INPUT_BUFFER_SIZE );
			sprintf(info.request_cmd,
				"%c %d",
				CMD_DELETE_ARP,
				l2_index);
			memset( info.request_string, (int) 0, USER_INPUT_BUFFER_SIZE );
			memcpy(	info.request_string,
					"delete_arp",
					strlen("delete_arp") + 1);
			ret_code = teja_cli_send_cmd(	info.cli_app_socket,
										info.request_cmd);
		}
	#endif /* #ifndef CLI_APP */
	return ret_code;
}

#ifndef CLI_APP

teja_uint32_t
config_blade_num(void)
{
	teja_uint32_t ret_code;
	teja_uint32_t blade_id;

	ret_code = 0;

	// get blade number 

	if ((ret_code = get_blade_id(&blade_id)) != 0) {
		printf(	"\nconfig_blade_num: get_blade_id failed. error %d",
				ret_code);
	} if ((ret_code = TejaCPDP_set_blade_id(info.ctx, blade_id)) != 0) {
		printf(	"\nconfig_blade_num: TejaCPDP_set_blade_id failed error %d",
				ret_code);
	} else {
		info.num_pending_transaction = 1;
		memset( info.request_string, (int) 0, USER_INPUT_BUFFER_SIZE );
		memcpy(	info.request_string,
				"config_blade_num",
				strlen("config_blade_num") + 1);
	}
	return ret_code;
}

#endif /* #ifndef CLI_APP */

teja_uint32_t
display_request(int request)
{
	teja_uint32_t ret_code = 0;
	char side = 0;

	switch (request)
	{
	case INTERFACE:
		#if (L2_TYPE == L2_ETH)
			if ((ret_code = get_side(&side)) != 0) {
				printf(	"\ndelete_arp: get_l2_index failed. error %d",
						ret_code);
			} else {
				#ifndef CLI_APP
					if (side == 'i') {
						ret_code =
							TejaCPDP_ipv4_dump_interface_info(	info.ctx,
															TEJA_SIDE_INGRESS);
					} else {
						ret_code = TejaCPDP_ipv4_dump_interface_info(info.ctx,
															TEJA_SIDE_EGRESS);
					}

				#else /* #ifndef CLI_APP */
					memset( info.request_cmd, (int) 0, USER_INPUT_BUFFER_SIZE );
					sprintf(info.request_cmd,
							"%c %c",
							CMD_DISPLAY_INTERFACE,
							side);
					ret_code = teja_cli_send_cmd(	info.cli_app_socket,
												info.request_cmd);

				#endif /* #ifndef CLI_APP */
			}
		#else /* */
			#ifndef CLI_APP
				ret_code = TejaCPDP_ipv4_dump_interface_info(info.ctx,
															TEJA_SIDE_INGRESS);
			#else /* #ifndef CLI_APP */
				memset( info.request_cmd, (int) 0, USER_INPUT_BUFFER_SIZE );
				sprintf(info.request_cmd,
					"%c %c",
					CMD_DISPLAY_INTERFACE,
						'i');
				ret_code = teja_cli_send_cmd(	info.cli_app_socket,
											info.request_cmd);
			#endif /* #ifndef CLI_APP */
		#endif
		break;
	case PHYSICAL_PORTS:
		#if (L2_TYPE == L2_ETH)
			// this display is only invoked for L2_ETH 

			if ((ret_code = get_side(&side)) != 0) {
				printf(	"\ndelete_arp: get_l2_index failed. error %d",
						ret_code);
			} else {
				#ifndef CLI_APP
					if (side == 'i') {
						ret_code = TejaCPDP_dump_port_info(	info.ctx,
															TEJA_SIDE_INGRESS);
					} else {
						ret_code = TejaCPDP_dump_port_info(	info.ctx,
															TEJA_SIDE_EGRESS);
					}
				#else /* #ifndef CLI_APP */
					memset( info.request_cmd, (int) 0, USER_INPUT_BUFFER_SIZE );
					sprintf(info.request_cmd,
						"%c %c",
						CMD_DISPLAY_PHY_PORTS,
						side);
					ret_code = teja_cli_send_cmd(	info.cli_app_socket,
												info.request_cmd);

				#endif /* #ifndef CLI_APP */
			}
		#endif

		break;
	case ROUTES:
		#ifndef CLI_APP
			ret_code = TejaCPDP_ipv4_dump_routes(info.ctx);
		#else /* #ifndef CLI_APP */
			memset( info.request_cmd, (int) 0, USER_INPUT_BUFFER_SIZE );
			sprintf(info.request_cmd, "%c", CMD_DISPLAY_ROUTES);
			ret_code = teja_cli_send_cmd(info.cli_app_socket, info.request_cmd);

		#endif /* #ifndef CLI_APP */

		break;
	case NEXTHOPS:
		#ifndef CLI_APP
				ret_code = TejaCPDP_ipv4_dump_next_hops(info.ctx);
		#else /* #ifndef CLI_APP */
				memset( info.request_cmd, (int) 0, USER_INPUT_BUFFER_SIZE );
				sprintf(info.request_cmd, "%c", CMD_DISPLAY_NEXTHOPS);
				ret_code = teja_cli_send_cmd(info.cli_app_socket, info.request_cmd);

		#endif /* #ifndef CLI_APP */
		break;
	case NAT_REDIRECTS:
		#ifndef CLI_APP
				ret_code = TejaCPDP_ipv4_dump_redirects(info.ctx);
		#else /* #ifndef CLI_APP */
				memset( info.request_cmd, (int) 0, USER_INPUT_BUFFER_SIZE );
				sprintf(info.request_cmd, "%c", CMD_DISPLAY_NAT_REDIRECTS);
				ret_code = teja_cli_send_cmd(info.cli_app_socket, info.request_cmd);
		#endif /* #ifndef CLI_APP */
		break;
	case DBCAST_TABLE:
		#ifndef CLI_APP
				ret_code = TejaCPDP_ipv4_dump_dbcast_table(info.ctx);
		#else /* #ifndef CLI_APP */
				memset( info.request_cmd, (int) 0, USER_INPUT_BUFFER_SIZE );
				sprintf(info.request_cmd, "%c", CMD_DISPLAY_DB_TABLE);
				ret_code = teja_cli_send_cmd(info.cli_app_socket, info.request_cmd);

		#endif /* #ifndef CLI_APP */
		break;
	case L2_FILTER_TABLE:
		#ifndef CLI_APP
				ret_code = TejaCPDP_dump_l2_filter_table(info.ctx);
		#else /* #ifndef CLI_APP */
				memset( info.request_cmd, (int) 0, USER_INPUT_BUFFER_SIZE );
				sprintf(info.request_cmd, "%c", CMD_DISPLAY_L2_FILTER_TABLE);
				ret_code = teja_cli_send_cmd(info.cli_app_socket, info.request_cmd);

		#endif /* #ifndef CLI_APP */
		break;
	case L2_TABLE:
		#ifndef CLI_APP
				ret_code = TejaCPDP_ipv4_dump_l2_table(info.ctx);
		#else /* #ifndef CLI_APP */
				memset( info.request_cmd, (int) 0, USER_INPUT_BUFFER_SIZE );
				sprintf(info.request_cmd, "%c", CMD_DISPLAY_L2_TABLE);
				ret_code = teja_cli_send_cmd(info.cli_app_socket, info.request_cmd);

		#endif /* #ifndef CLI_APP */
		break;
	case ARP_CACHE:
		#ifndef CLI_APP
				ret_code = TejaCPDP_ipv4_dump_arp_cache(info.ctx);
		#else /* #ifndef CLI_APP */
				memset( info.request_cmd, (int) 0, USER_INPUT_BUFFER_SIZE );
				sprintf(info.request_cmd, "%c", CMD_DISPLAY_ARP_CACHE);
				ret_code = teja_cli_send_cmd(info.cli_app_socket, info.request_cmd);

		#endif /* #ifndef CLI_APP */
		break;
	default:
		printf("\ndisplay_request received incorrect request %d", request);
		return 1;
	}

	if (ret_code != 0) {
		printf("\ndisplay_request: failed to generate request %d", request);
	} else {
		#ifndef CLI_APP
			info.num_pending_transaction = 1;
		#endif

		memset( info.request_string, (int) 0, USER_INPUT_BUFFER_SIZE );
		memcpy(	info.request_string,
				"display_request",
				strlen("display_request") + 1);
	}
	return ret_code;
}

teja_uint32_t
purge_request(int request)
{
	teja_uint32_t ret_code = 0;

	switch (request)
	{
	case ROUTES:
		#ifndef CLI_APP
				ret_code = TejaCPDP_ipv4_purge_routes(info.ctx);
		#else /* #ifndef CLI_APP */
				memset( info.request_cmd, (int) 0, USER_INPUT_BUFFER_SIZE );
				sprintf(info.request_cmd, "%c", CMD_PURGE_ROUTES);
				ret_code = teja_cli_send_cmd(info.cli_app_socket, info.request_cmd);

		#endif /* #ifndef CLI_APP */
		break;
	case RTM:
		#ifndef CLI_APP
				ret_code = TejaCPDP_ipv4_purge_rtm(info.ctx);
		#else /* #ifndef CLI_APP */
				memset( info.request_cmd, (int) 0, USER_INPUT_BUFFER_SIZE );
				sprintf(info.request_cmd, "%c", CMD_PURGE_RTM);
				ret_code = teja_cli_send_cmd(info.cli_app_socket, info.request_cmd);

		#endif /* #ifndef CLI_APP */
		break;
	case L2_TABLE:
		#ifndef CLI_APP
				ret_code = TejaCPDP_ipv4_purge_l2_table(info.ctx);
		#else /* #ifndef CLI_APP */
				memset( info.request_cmd, (int) 0, USER_INPUT_BUFFER_SIZE );
				sprintf(info.request_cmd, "%c", CMD_PURGE_L2_TABLE);
				ret_code = teja_cli_send_cmd(info.cli_app_socket, info.request_cmd);

		#endif /* #ifndef CLI_APP */
		break;
	case ARP_CACHE:
		#ifndef CLI_APP
				ret_code = TejaCPDP_ipv4_purge_arp_cache(info.ctx);
		#else /* #ifndef CLI_APP */
				memset( info.request_cmd, (int) 0, USER_INPUT_BUFFER_SIZE );
				sprintf(info.request_cmd, "%c", CMD_PURGE_ARP_CACHE);
				ret_code = teja_cli_send_cmd(info.cli_app_socket, info.request_cmd);

		#endif /* #ifndef CLI_APP */
		break;
	case NAT_REDIRECTS:
		#ifndef CLI_APP
				ret_code = TejaCPDP_ipv4_purge_redirects(info.ctx);
		#else /* #ifndef CLI_APP */
				memset( info.request_cmd, (int) 0, USER_INPUT_BUFFER_SIZE );
				sprintf(info.request_cmd, "%c", CMD_PURGE_NAT_REDIRECTS );
				ret_code = teja_cli_send_cmd(info.cli_app_socket, info.request_cmd);
		#endif /* #ifndef CLI_APP */
		break;
	default:
		printf("purge_request received incorrect request %d", request);
		return 1;
	}

	if (ret_code != 0) {
		printf("\npurge_request: failed to generate request %d", request);
	} else {
		#ifndef CLI_APP
				info.num_pending_transaction = 1;
		#endif /* #ifndef CLI_APP */

		memset( info.request_string, (int) 0, USER_INPUT_BUFFER_SIZE );
		memcpy(	info.request_string,
				"purge_request",
				strlen("purge_request") + 1);
	}

	return ret_code;
}

void
display_error_string(teja_uint32_t error_code)
{
}

teja_uint32_t
config_eth_port(void)
{
	teja_uint32_t ret_code;
	TejaCPDP_if_state_t port_state;
	teja_uint32_t blade_id;
	teja_uint32_t port_id;
	teja_uint32_t mtu;
	TejaCPDP_if_state_t promiscuous_mode;
	TejaCPDP_mac_addr_t mac_addr;

	ret_code = 0;

	if ((ret_code = get_blade_id(&blade_id) != 0)) {
		printf("\nconfig_eth_port: get_blade_id failed error %d", ret_code);
	} else if ((ret_code = get_port_id(&port_id) != 0)) {
		printf("\nconfig_eth_port: get_port_id failed error %d", ret_code);
	} else if ((ret_code = get_port_state(&port_state) != 0)) {
		printf("\nconfig_eth_port: get_port_state failed error %d", ret_code);
	} else if ((ret_code = get_mtu(&mtu) != 0)) {
		printf("\nconfig_eth_port: get_mtu failed error %d", ret_code);
	} else if ((ret_code = get_promiscuous_mode(&promiscuous_mode) != 0)) {
		printf("\nconfig_eth_port: get_promiscuous_mode failed error %d",
				ret_code);
	} else if ((ret_code = get_mac_addr(&mac_addr) != 0)) {
		printf("\nconfig_eth_port: get_mac_addr failed error %d", ret_code);
	}
	#ifndef CLI_APP
		else if ((ret_code =
			  TejaCPDP_cfg_eth_port_info(	info.ctx,
											blade_id,
											port_id,
											&mac_addr,
											port_state,
										 	promiscuous_mode,
											mtu)) != 0) {
			printf(	"\nconfig_eth_port: TejaCPDP_cfg_eth_port_info "
					"failed error %d",
			   		ret_code);
		} else {
			info.num_pending_transaction = 1;
			memset( info.request_string, (int) 0, USER_INPUT_BUFFER_SIZE );
			memcpy(	info.request_string,
					"config_eth_port",
					strlen("config_eth_port") + 1);
		}
	#else /* #ifndef CLI_APP */
		else {
			memset( info.request_cmd, (int) 0, USER_INPUT_BUFFER_SIZE );
			sprintf(info.request_cmd,
					"%c %d %d %d %d %d %x:%x:%x:%x:%x:%x",
					CMD_CONFIG_PORT,
					blade_id,
					port_id,
					port_state,
					mtu,
					promiscuous_mode,
					mac_addr.mac[0],
					mac_addr.mac[1],
					mac_addr.mac[2],
					mac_addr.mac[3],
					mac_addr.mac[4],
					mac_addr.mac[5]);
			memset( info.request_string, (int) 0, USER_INPUT_BUFFER_SIZE );
			memcpy(	info.request_string,
					"config_eth_port",
					strlen("config_eth_port") + 1);
			ret_code = teja_cli_send_cmd(	info.cli_app_socket,
										info.request_cmd);
		}
	#endif /* #ifndef CLI_APP */

	return ret_code;
}

teja_uint32_t
config_pos_port(void)
{
	teja_uint32_t ret_code;
	TejaCPDP_if_state_t port_state;
	teja_uint32_t blade_id;
	teja_uint32_t port_id;
	teja_uint32_t mtu;

	ret_code = 0;

	if ((ret_code = get_blade_id(&blade_id) != 0)) {
		printf("\nconfig_pos_port: get_blade_id failed error %d", ret_code);
	} else if ((ret_code = get_port_id(&port_id) != 0)) {
		printf("\nconfig_pos_port: get_port_id failed error %d", ret_code);
	} else if ((ret_code = get_port_state(&port_state) != 0)) {
		printf("\nconfig_pos_port: get_port_state failed error %d", ret_code);
	} else if ((ret_code = get_mtu(&mtu) != 0)) {
		printf("\nconfig_pos_port: get_mtu failed error %d", ret_code);
	}
	#ifndef CLI_APP
		else if ((ret_code =
			  TejaCPDP_cfg_pos_port_info(	info.ctx,
											blade_id,
											port_id,
											port_state, mtu)) != 0) {
			printf(	"\nconfig_pos_port: TejaCPDP_cfg_pos_port_info "
					"failed error %d",
			   		ret_code);
		} else {
			info.num_pending_transaction = 1;
			memset( info.request_string, (int) 0, USER_INPUT_BUFFER_SIZE );
			memcpy(	info.request_string,
					"config_pos_port",
					strlen("config_pos_port") + 1);
		}
	#else /* #ifndef CLI_APP */
		else {
			memset( info.request_cmd, (int) 0, USER_INPUT_BUFFER_SIZE );
			sprintf(info.request_cmd,
					"%c %d %d %d %d",
					CMD_CONFIG_PORT,
					blade_id,
					port_id,
					port_state,
					mtu);
			memset( info.request_string, (int) 0, USER_INPUT_BUFFER_SIZE );
			memcpy(	info.request_string,
					"config_pos_port",
					strlen("config_pos_port") + 1);
			ret_code = teja_cli_send_cmd(	info.cli_app_socket,
										info.request_cmd);
		}
	#endif /* #ifndef CLI_APP */

	return ret_code;
}

teja_uint32_t
config_port_state(void)
{
	teja_uint32_t ret_code = 0;
	TejaCPDP_if_state_t port_state;
	TejaCPDP_if_state_t blade_id;
	TejaCPDP_if_state_t port_id;

	if ((ret_code = get_blade_id(&blade_id) != 0)) {
		printf("\nconfig_port_state: get_blade_id failed error %d", ret_code);
	} else if ((ret_code = get_port_id(&port_id) != 0)) {
		printf("\nconfig_port_state: get_port_id failed error %d", ret_code);
	} else if ((ret_code = get_port_state(&port_state) != 0)) {
		printf("\nconfig_port_state: get_port_state failed error %d", ret_code);
	}
	#ifndef CLI_APP
		else if ((ret_code =
			  TejaCPDP_cfg_port_state(	info.ctx,
										blade_id,
										port_id,
										port_state)) != 0) {
			printf(	"\nconfig_port_state: TejaCPDP_cfg_port_state "
					"failed error %d", ret_code);
		} else {
			info.num_pending_transaction = 1;
			memset( info.request_string, (int) 0, USER_INPUT_BUFFER_SIZE );
			memcpy(	info.request_string,
					"config_port_state",
			   		strlen("config_port_state") + 1);
		}
	#else /* #ifndef CLI_APP */
		else {
			memset( info.request_cmd, (int) 0, USER_INPUT_BUFFER_SIZE );
			sprintf(info.request_cmd,
					"%c %d %d %d",
					CMD_CONFIG_PORT_STATE,
					blade_id,
					port_id,
					port_state);
			memset( info.request_string, (int) 0, USER_INPUT_BUFFER_SIZE );
			memcpy(	info.request_string,
					"config_port_state",
			   		strlen("config_port_state") + 1);
			ret_code = teja_cli_send_cmd(	info.cli_app_socket,
										info.request_cmd);
		}
	#endif /* #ifndef CLI_APP */

	return ret_code;
}

teja_uint32_t
config_link_state(void)
{
	teja_uint32_t ret_code;
	TejaCPDP_link_status_t link_status;
	TejaCPDP_if_state_t blade_id;
	TejaCPDP_if_state_t port_id;

	ret_code = 0;

	if ((ret_code = get_blade_id(&blade_id) != 0)) {
		printf("\nconfig_link_state: get_blade_id failed error %d", ret_code);
	} else if ((ret_code = get_port_id(&port_id) != 0)) {
		printf("\nconfig_link_state: get_port_id failed error %d", ret_code);
	} else if ((ret_code = get_link_status(&link_status) != 0)) {
		printf(	"\nconfig_link_state: get_link_status failed error %d",
				ret_code);
	}
	#ifndef CLI_APP
		else if ((ret_code =
			  TejaCPDP_set_link_state(	info.ctx,
										blade_id,
										port_id,
										link_status)) != 0) {
			printf(	"\nconfig_link_state: TejaCPDP_cfg_link_status "
					"failed error %d",
			   		ret_code);
		} else {
			info.num_pending_transaction = 1;
			memset( info.request_string, (int) 0, USER_INPUT_BUFFER_SIZE );
			memcpy(	info.request_string,
					"get_link_status",
					strlen("get_link_status") + 1);
		}
	#else /* #ifndef CLI_APP */
		else {
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
			ret_code = teja_cli_send_cmd(	info.cli_app_socket,
										info.request_cmd);
		}
	#endif /* #ifndef CLI_APP */

	return ret_code;
}

teja_uint32_t
get_sleep_time(void)
{
	teja_uint32_t ret_code;

	#ifndef CLI_APP
		if ((ret_code = TejaCPDP_ipv4_get_sleep_time(info.ctx)) != 0) {
			printf(	"\nconfig_link_state: TejaCPDP_ipv4_get_sleep_time "
					"failed error %d",
			 		ret_code);
		} else {
			info.num_pending_transaction = 1;
			memset( info.request_string, (int) 0, USER_INPUT_BUFFER_SIZE );
			memcpy(	info.request_string,
					"get_sleep_time",
					strlen("get_sleep_time") + 1);
		}
	#else /* #ifndef CLI_APP */
		memset( info.request_cmd, (int) 0, USER_INPUT_BUFFER_SIZE );
		sprintf(info.request_cmd, "%c", CMD_GET_SLEEP_TIME);
		memset( info.request_string, (int) 0, USER_INPUT_BUFFER_SIZE );
		memcpy(	info.request_string,
				"get_sleep_time",
				strlen("get_sleep_time") + 1);
		ret_code = teja_cli_send_cmd(	info.cli_app_socket,
									info.request_cmd);
	#endif /* #ifndef CLI_APP */

	return ret_code;
}

teja_uint32_t
set_sleep_time(void)
{
	teja_uint32_t ret_code;
	teja_uint32_t sleep_time;

	if ((ret_code = get_sleep_time_to_set(&sleep_time) != 0)) {
		printf("\nset_sleep_time: get_num failed error %d", ret_code);
	}
	#ifndef CLI_APP
		else if ((ret_code = 
			TejaCPDP_ipv4_set_sleep_time(	info.ctx,
											sleep_time)) != 0) {
			printf(	"\nset_sleep_time: TejaCPDP_ipv4_set_sleep_time "
					"failed error %d",
					ret_code);
		} else {
			info.num_pending_transaction = 1;
			memset( info.request_string, (int) 0, USER_INPUT_BUFFER_SIZE );
			memcpy(	info.request_string,
					"set_sleep_time",
					strlen("set_sleep_time") + 1);
		}
	#else /* #ifndef CLI_APP */
		else {
			memset( info.request_cmd, (int) 0, USER_INPUT_BUFFER_SIZE );
			sprintf(info.request_cmd,
					"%c %d",
					CMD_SET_SLEEP_TIME,
					sleep_time);
			memset( info.request_string, (int) 0, USER_INPUT_BUFFER_SIZE );
			memcpy(	info.request_string,
					"set_sleep_time",
					strlen("set_sleep_time") + 1);
			ret_code = teja_cli_send_cmd(	info.cli_app_socket,
										info.request_cmd);
		}
	#endif /* #ifndef CLI_APP */

	return ret_code;
}

teja_uint32_t
get_pkts_to_drain(void)
{
	teja_uint32_t ret_code;

	#ifndef CLI_APP
		if ((ret_code = TejaCPDP_ipv4_get_packets_to_drain(info.ctx)) != 0) {
			printf(	"\nget_pkts_to_drain: TejaCPDP_ipv4_get_packets_to_drain"
					" failed error %d",
			 		ret_code);
		} else {
			info.num_pending_transaction = 1;
			memset( info.request_string, (int) 0, USER_INPUT_BUFFER_SIZE );
			memcpy(	info.request_string,
					"get_pkts_to_drain",
			   		strlen("get_pkts_to_drain") + 1);
		}
	#else /* #ifndef CLI_APP */
		memset( info.request_cmd, (int) 0, USER_INPUT_BUFFER_SIZE );
		sprintf(	info.request_cmd,
					"%c",
					CMD_GET_PKTS_TO_DRAIN);
		memset( info.request_string, (int) 0, USER_INPUT_BUFFER_SIZE );
		memcpy(	info.request_string,
				"get_pkts_to_drain",
				strlen("get_pkts_to_drain") + 1);
		ret_code = teja_cli_send_cmd(	info.cli_app_socket,
									info.request_cmd);
	#endif /* #ifndef CLI_APP */

	return ret_code;
}

teja_uint32_t
set_pkts_to_drain(void)
{
	teja_uint32_t ret_code;
	teja_uint32_t pkts_to_drain;

	if ((ret_code = get_pkts_to_drain_to_set(&pkts_to_drain) != 0)) {
		printf("\nset_pkts_to_drain: get_num failed error %d", ret_code);
	} 
	#ifndef CLI_APP
		else if ((ret_code =
			  TejaCPDP_ipv4_set_packets_to_drain(	info.ctx,
													pkts_to_drain)) != 0) {
			printf(	"\nset_sleep_time: TejaCPDP_ipv4_set_packets_to_drain"
					" failed error %d",
					ret_code);
		} else {
			info.num_pending_transaction = 1;
			memset( info.request_string, (int) 0, USER_INPUT_BUFFER_SIZE );
			memcpy(	info.request_string,
					"set_pkts_to_drain",
			   		strlen("set_pkts_to_drain") + 1);
		}
	#else /* #ifndef CLI_APP */
		else {
			memset( info.request_cmd, (int) 0, USER_INPUT_BUFFER_SIZE );
			sprintf(info.request_cmd,
					"%c %d",
					CMD_SET_PKTS_TO_DRAIN,
					pkts_to_drain);
			memset( info.request_string, (int) 0, USER_INPUT_BUFFER_SIZE );
			memcpy(	info.request_string,
					"set_pkts_to_drain",
			   		strlen("set_pkts_to_drain") + 1);
			ret_code = teja_cli_send_cmd(	info.cli_app_socket,
										info.request_cmd);
		}
	#endif /* #ifndef CLI_APP */

	return ret_code;
}

teja_uint32_t
get_queue_depth(void)
{
	teja_uint32_t ret_code;

	#ifndef CLI_APP
		if ((ret_code = TejaCPDP_ipv4_get_queue_depth(info.ctx)) != 0) {
			printf(	"\nconfig_link_state: TejaCPDP_ipv4_get_queue_depth"
					" failed error %d",
			 		ret_code);
		} else {
			info.num_pending_transaction = 1;
			memset( info.request_string, (int) 0, USER_INPUT_BUFFER_SIZE );
			memcpy(	info.request_string,
					"get_queue_depth",
					strlen("get_queue_depth") + 1);
		}
	#else /* #ifndef CLI_APP */
		memset( info.request_cmd, (int) 0, USER_INPUT_BUFFER_SIZE );
		sprintf(info.request_cmd, "%c", CMD_GET_QUEUE_DEPTH);
		memset( info.request_string, (int) 0, USER_INPUT_BUFFER_SIZE );
		memcpy(	info.request_string,
				"get_queue_depth",
				strlen("get_queue_depth") + 1);
		ret_code = teja_cli_send_cmd(	info.cli_app_socket,
									info.request_cmd);
	#endif /* #ifndef CLI_APP */
	return ret_code;
}

teja_uint32_t
get_ipv4_stats(void)
{
	teja_uint32_t ret_code;

	#ifndef CLI_APP
		if ((ret_code = TejaCPDP_ipv4_get_statistics(info.ctx)) != 0) {
			printf(	"\nget_ipv4_stats: TejaCPDP_ipv4_get_statistics "
					"failed error %d", ret_code);
		} else {
			info.num_pending_transaction = 1;
			memset( info.request_string, (int) 0, USER_INPUT_BUFFER_SIZE );
			memcpy(	info.request_string,
					"get_ipv4_stats",
					strlen("get_ipv4_stats") + 1);
		}
	#else /* #ifndef CLI_APP */
		memset( info.request_cmd, (int) 0, USER_INPUT_BUFFER_SIZE );
		sprintf(info.request_cmd,
				"%c",
				CMD_GET_IPV4_STATS);
		memset( info.request_string, (int) 0, USER_INPUT_BUFFER_SIZE );
		memcpy(	info.request_string,
				"get_ipv4_stats",
				strlen("get_ipv4_stats") + 1);
		ret_code = teja_cli_send_cmd(	info.cli_app_socket,
									info.request_cmd);
	#endif /* #ifndef CLI_APP */

	return ret_code;
}

#if (defined(TEJA_MAIN) & defined(CLI_APP)) || !defined(CLI_APP)

teja_uint32_t
top_menu(void)
{
	teja_uint32_t ret_code;

	ret_code = 0;

	#ifndef CLI_APP /* server code */
		while ((info.num_pending_transaction == 0) && (ret_code == 0))
	#else /* #ifndef CLI_APP */
		top_menu_content();
	#endif /* #ifndef CLI_APP */
	{

		#if (L2_TYPE == L2_POS)
			ret_code = get_user_selection(
								&info.top_menu_selection,
								"123456789adcdhijklmnopqrst");
		#else /* */
				ret_code = get_user_selection(
								&info.top_menu_selection,
								"123456789abcdefghijklmnopqrstuvwxyzADUSX");
		#endif /* */

		if (ret_code == 0) {
			switch (info.top_menu_selection)
			{
			case '1':
				ret_code = add_interface();
				break;
			case '2':
				ret_code = delete_interface();
				break;
			case '3':

				#if (L2_TYPE == L2_ETH)
					ret_code = config_eth_port();
				#else /* */
					ret_code = config_pos_port();
				#endif /* */
				break;
			case '4':
				ret_code = config_port_state();
				break;
			case '5':
				ret_code = config_link_state();
				break;
			case '6':
				ret_code = add_route();
				break;
			case '7':
				ret_code = update_route();
				break;
			case '8':
				ret_code = delete_route();
				break;
			case '9':
				ret_code = add_nexthop();
				break;
			case 'a':
				ret_code = update_nexthop();
				break;
			case 'b':
				ret_code = delete_nexthop();
				break;
			case 'c':
				ret_code = change_nexthop_mtu();
				break;
			case 'd':
				ret_code = change_nexthop_flag();
				break;
			case 'e':
				ret_code = add_static_arp();
				break;
			case 'f':
				ret_code = create_arp();
				break;
			case 'g':
				ret_code = delete_arp();
				break;
			case 'h':
				ret_code = get_sleep_time();
				break;
			case 'i':
				ret_code = set_sleep_time();
				break;
			case 'j':
				ret_code = get_pkts_to_drain();
				break;
			case 'k':
				ret_code = set_pkts_to_drain();
				break;
			case 'l':
				ret_code = get_queue_depth();
				break;
			case 'm':
				ret_code = get_ipv4_stats();
				break;
			case 'n':
				ret_code = display_request(INTERFACE);
				break;
			case 'o':
				ret_code = display_request(PHYSICAL_PORTS);
				break;
			case 'p':
				ret_code = display_request(ROUTES);
				break;
			case 'r':
				ret_code = purge_request(ROUTES);
				break;
			case 's':
				ret_code = display_request(NEXTHOPS);
				break;
			case 't':
				ret_code = purge_request(RTM);
				break;
			case 'u':
				ret_code = display_request(DBCAST_TABLE);
				break;
			case 'v':
				ret_code = display_request(L2_FILTER_TABLE);
				break;
			case 'w':
				ret_code = display_request(L2_TABLE);
				break;
			case 'x':
				ret_code = purge_request(L2_TABLE);
				break;
			case 'y':
				ret_code = display_request(ARP_CACHE);
				break;
			case 'z':
				ret_code = purge_request(ARP_CACHE);
				break;
			case 'A':
				ret_code = add_redirect();
				break;
			case 'D':
				ret_code = delete_redirect();
				break;
			case 'U':
				ret_code = update_redirect();
				break;
			case 'S':
				ret_code = display_request(NAT_REDIRECTS);
				break;
			case 'X':
				ret_code = purge_request(NAT_REDIRECTS);
				break;
			case 'q':
				ret_code = USER_REQUEST_TO_QUIT;
				break;
			default:
				printf("\ntop_menu: user_selection processing ERROR!!");
			}
		}
	}	/* while ((info.num_pending_transaction == 0) && (ret_code == 0)) */


	return ret_code;
}

#endif

int
teja_cli_send_cmd(TejaSocket socket, char *cmd)
{
	int size = strlen(cmd) + 1;	/* include null character */
	int ret_code;
	int index = 0;

	if (socket != TEJA_INVALID_SOCKET) {
		#ifdef CLI_APP
			if (info.init_done) {
				printf("CMD: %s \n", cmd);
			}
		#endif

		do {
			if ((ret_code = teja_write_to_socket(socket,
												 &cmd[index],
												 size - index, 0)) < 0) {
				printf(	"teja_cli_send_cmd(info.cli_app_socket, ):"
						"teja_write_to_socket returned %d",
					 	ret_code);
				return -1;
			}
			index += ret_code;
		}
		while (index < size);
	}
	else {
		printf(	"\nteja_cli_send_cmd(info.cli_app_socket, ): Application "
				"socket invalid ");
	}
	return 0;
}
