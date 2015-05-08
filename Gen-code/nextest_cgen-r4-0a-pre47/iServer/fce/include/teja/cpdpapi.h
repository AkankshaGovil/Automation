/********************************************************************/
/*                                                                  */
/* Copyright (C) Teja Technologies, Inc. 1998-2003.                 */
/*                                                                  */
/* All Rights Reserved.                                             */
/*                                                                  */
/* This software is the property of Teja Technologies, Inc.  It is  */
/* furnished under a specific licensing agreement.  It may be used  */
/* or copied only under terms of the licensing agreement.           */
/*                                                                  */
/* For more information, contact info@teja.com                      */
/*                                                                  */
/********************************************************************/

#ifndef _CPDPAPI_H
#define _CPDPAPI_H

/* File Descriptor, Signal and Timer Registration API */
int TejaCPDP_register_fd(
	TejaCPDP_ctx *ctx,
	int fd);

int TejaCPDP_deregister_fd(
	TejaCPDP_ctx *ctx,
	int fd);

int TejaCPDP_register_signal(
	TejaCPDP_ctx *ctx,
	int sigcode);

int TejaCPDP_deregister_signal(
	TejaCPDP_ctx *ctx,
	int sigcode);

int TejaCPDP_register_timer(
	TejaCPDP_ctx *ctx,
	double timeout);

int TejaCPDP_deregister_timer(
	TejaCPDP_ctx *ctx);

/* Blade and Interface Management Service API */

int TejaCPDP_set_blade_id(
	TejaCPDP_ctx *ctx,
	teja_uint32_t blade_id);

int TejaCPDP_cfg_eth_port_info(
	TejaCPDP_ctx *ctx,
	teja_uint32_t blade_id,
	teja_uint32_t port_id,
	TejaCPDP_mac_addr_t * mac,
	TejaCPDP_if_state_t state,
	TejaCPDP_if_state_t promiscuous_mode,
	teja_uint32_t mtu);

int TejaCPDP_cfg_pos_port_info(
	TejaCPDP_ctx *ctx,
	teja_uint32_t blade_id,
	teja_uint32_t port_id,
	TejaCPDP_if_state_t state,
	teja_uint32_t mtu);

int TejaCPDP_cfg_port_state(
	TejaCPDP_ctx *ctx,
	teja_uint32_t blade_id,
	teja_uint32_t port_id,
	TejaCPDP_if_state_t state);

int TejaCPDP_set_link_state(
	TejaCPDP_ctx *ctx,
	teja_uint32_t blade_id,
	teja_uint32_t port_id,
	TejaCPDP_if_state_t state);

int TejaCPDP_dump_port_info(
	TejaCPDP_ctx *ctx,
	TejaCPDP_side_t side);

int TejaCPDP_dump_l2_filter_table(
	TejaCPDP_ctx *ctx);

int TejaCPDP_ipv4_add_interface(
	TejaCPDP_ctx *ctx,
	teja_uint32_t blade_id,
	teja_uint32_t port_id,
	teja_uint32_t ip_addr, 
	teja_uint32_t ip_mask);

int TejaCPDP_ipv4_delete_interface(
	TejaCPDP_ctx *ctx,
	teja_uint32_t blade_id,
	teja_uint32_t port_id,
	teja_uint32_t ip_addr);

int TejaCPDP_ipv4_dump_interface_info(
	TejaCPDP_ctx *ctx,
	TejaCPDP_side_t side);

int TejaCPDP_ipv4_dump_dbcast_table(
	TejaCPDP_ctx *ctx);


/* ARP Management Service API */
int TejaCPDP_ipv4_add_arp_entry(
	TejaCPDP_ctx *ctx,
	teja_uint32_t out_port_id,
	teja_uint32_t l2_index, 
	teja_uint32_t next_hop_ip_addr, 
	TejaCPDP_mac_addr_t * mac);

int TejaCPDP_ipv4_create_arp_entry(
	TejaCPDP_ctx *ctx,
	teja_uint32_t out_port_id,
	teja_uint32_t l2_index, 
	teja_uint32_t next_hop_ip_addr);

int TejaCPDP_ipv4_delete_arp_entry(
	TejaCPDP_ctx *ctx,
	teja_uint32_t l2_index);

int TejaCPDP_ipv4_purge_arp_cache(
	TejaCPDP_ctx *ctx);

int TejaCPDP_ipv4_dump_arp_cache(
	TejaCPDP_ctx *ctx);

int TejaCPDP_ipv4_dump_l2_table(
	TejaCPDP_ctx *ctx);

int TejaCPDP_ipv4_purge_l2_table(
	TejaCPDP_ctx *ctx);

/* IPv4 Route Management Service API */

int TejaCPDP_ipv4_add_route(
	TejaCPDP_ctx *ctx,
	teja_uint32_t ip_addr, 
	teja_uint32_t net_mask, 
	teja_uint32_t next_hop_id);

int TejaCPDP_ipv4_delete_route(
	TejaCPDP_ctx *ctx,
	teja_uint32_t ip_addr, 
	teja_uint32_t net_mask);

int TejaCPDP_ipv4_update_route(
	TejaCPDP_ctx *ctx,
	teja_uint32_t ip_addr, 
	teja_uint32_t net_mask, 
	teja_uint32_t next_hop_id);

int TejaCPDP_ipv4_purge_routes(
	TejaCPDP_ctx *ctx);

int TejaCPDP_ipv4_dump_routes(
	TejaCPDP_ctx *ctx);

int TejaCPDP_ipv4_add_next_hop(
	TejaCPDP_ctx *ctx,
	teja_uint32_t next_hop_id,
	TejaCPDP_next_hop_info_t *next_hop_info);

int TejaCPDP_ipv4_update_next_hop(
	TejaCPDP_ctx *ctx,
	teja_uint32_t next_hop_id, 
	TejaCPDP_next_hop_info_t *next_hop_info);

int TejaCPDP_ipv4_delete_next_hop(
	TejaCPDP_ctx *ctx,
	teja_uint32_t next_hop_id);

int TejaCPDP_ipv4_dump_next_hops(
	TejaCPDP_ctx *ctx);

int TejaCPDP_ipv4_add_redirect(
	TejaCPDP_ctx *ctx,
	TejaCPDP_redirect_key_t *redirect_key,
	TejaCPDP_redirect_info_t *redirect_info );

int TejaCPDP_ipv4_update_redirect(
	TejaCPDP_ctx *ctx,
	TejaCPDP_redirect_key_t *redirect_key,
	TejaCPDP_redirect_info_t *redirect_info );

int TejaCPDP_ipv4_delete_redirect(
	TejaCPDP_ctx *ctx,
	TejaCPDP_redirect_key_t *redirect_key );

int TejaCPDP_ipv4_dump_redirects(
	TejaCPDP_ctx *ctx );

int TejaCPDP_ipv4_purge_redirects(
	TejaCPDP_ctx *ctx);

int TejaCPDP_ipv4_purge_rtm(
	TejaCPDP_ctx *ctx);

int TejaCPDP_ipv4_set_mtu(
	TejaCPDP_ctx *ctx,
	teja_uint32_t next_hop_id,
	teja_uint32_t mtu);

int TejaCPDP_ipv4_set_flag(
	TejaCPDP_ctx *ctx,
	teja_uint32_t next_hop_id,
	teja_uint32_t flags);

int TejaCPDP_ipv4_get_sleep_time(
	TejaCPDP_ctx *ctx);

int TejaCPDP_ipv4_set_sleep_time(
	TejaCPDP_ctx *ctx,
	teja_uint32_t sleep_time);

int TejaCPDP_ipv4_get_queue_depth(
	TejaCPDP_ctx *ctx);

int TejaCPDP_ipv4_get_packets_to_drain(
	TejaCPDP_ctx *ctx);

int TejaCPDP_ipv4_set_packets_to_drain(
	TejaCPDP_ctx *ctx,
	teja_uint32_t packets_to_drain);

int TejaCPDP_ipv4_get_statistics(
	TejaCPDP_ctx *ctx);

/* Correlation */
int TejaCPDP_get_last_transaction_id(
	TejaCPDP_ctx *ctx, 
	teja_uint32_t *id);

int TejaCPDP_cancel_transaction(
	TejaCPDP_ctx *ctx, 
	teja_uint32_t id);

int
wait_for_response_add_redirect( teja_uint32_t *	p_hk_hv,
								teja_uint32_t * p_hk_redir_addr );
int
wait_for_response_add_arp( teja_uint32_t *	p_l2_index );

int
wait_for_response(void);	/* self expanatory */


/* packet transmission */
int TejaCPDP_send_packet(
	TejaCPDP_ctx *ctx, 
	teja_uint32_t out_blade_id, 
	teja_uint32_t out_port_id, 
	teja_uint32_t len, 
	const teja_uint8_t *pkt);


#endif /* _CPDPAPI_H */
