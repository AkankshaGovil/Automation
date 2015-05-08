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

#ifndef _CPDPCOMMON_H
#define _CPDPCOMMON_H

/* Reserved Next Hop Identifiers */
#define TEJACPDP_NEXTHOP_BROADCAST      0xFFFFFFFB
#define TEJACPDP_NEXTHOP_MULTICAST      0xFFFFFFFC
#define TEJACPDP_NEXTHOP_DROP           0xFFFFFFFD
#define TEJACPDP_NEXTHOP_LOCAL          0xFFFFFFFE

/* Reserved L2 Index */
#define TEJACPDP_LAN_L2_INDEX           0

typedef void TejaCPDP_ctx;

typedef struct TejaCPDP_mac_addr {
	teja_uint8_t mac[6]; 
} TejaCPDP_mac_addr_t;

typedef enum TejaCPDP_if_state {
	TEJA_IF_DISABLE,
	TEJA_IF_ENABLE
} TejaCPDP_if_state_t;

typedef enum TejaCPDP_side {
	TEJA_SIDE_INGRESS,
	TEJA_SIDE_EGRESS
} TejaCPDP_side_t;

typedef enum TejaCPDP_next_hop_info_flags {
	/* no flag set means forward */
	TEJACPDP_NEXT_HOP_FLAG_LOCAL = 1,
	TEJACPDP_NEXT_HOP_FLAG_DOWN = 2, 
	TEJACPDP_NEXT_HOP_FLAG_DROP = 4, 
	TEJACPDP_NEXT_HOP_FLAG_BROADCAST = 8,
	TEJACPDP_NEXT_HOP_FLAG_MULTICAST = 0x10
} TejaCPDP_next_hop_info_flags_t;


typedef enum TejaCPDP_link_status {
	TEJA_IF_DOWN,
	TEJA_IF_UP
} TejaCPDP_link_status_t;

typedef enum TejaCPDP_response_code {
	NO_ERR = 0,
	ERR_INTERNAL,
	ERR_DUPLICATE_ENTRY,
	ERR_TABLE_FULL,
	ERR_ENTRY_NOT_FOUND,
	ERR_OUT_OF_RANGE,
	ERR_INVALID_ENTRY,
	ERR_UNSUPPORTED,
	ERR_INVALID_PARAMETERS,
	ERR_OUT_OF_RESOURCES,
	ERR_ENTRY_EXISTS,
	ERR_CONFLICTING_ENTRY,
	ERR_ENTRY_IN_USE,
	ERR_INVALID_NEXTHOP_INFO,
	ERR_NHID_IN_USE,
	ERR_NULL,
	ERR_INVALID_ALGO,
	ERR_HASH_TABLE_FULL,
	ERR_INPUT_STRING_TOO_LONG,
	ERR_USER_REQUEST_TO_QUIT,
	ERR_ALLOCATE_ALERT_FAILED,
	ERR_SEND_ALERT_FAILED,
	ERR_REQUEST_FAIL,
	ERR_BAD_ALERT,
	ERR_INVALID_ALERT_PARAMETER,
	ERR_ENTRY_TYPE_NOT_HANDLED,
	ERR_BAD_MASK,
	ERR_BLADE_UNINITIALIZED,
	ERR_ALERT_ALREADY_IN_USE,
	ERR_ALERT_FULL,
	ERR_INVALID_GET_CALL,
	ERR_GET_ALL_DONE,
	ERR_BUFFER_FULL,
	ERR_IP_FRAG
} TejaCPDP_response_code;

typedef enum TejaCPDP_error_code {
	TEJACPDP_INTERNAL_ERR = 1,
	TEJACPDP_INVALID,
	TEJACPDP_NOMEM,
	TEJACPDP_INCORRECT_PORT_TYPE,
	TEJACPDP_INVALID_ID,
	TEJACPDP_MAX_REACHED
} TejaCPDP_error_code_t;

typedef struct TejaCPDP_next_hop_info {
	teja_uint32_t blade_id;
	teja_uint32_t l2_index; 
	teja_uint32_t port_id;
	teja_uint32_t mtu;
	teja_uint32_t flags;
	teja_uint32_t next_hop_ip_addr;
	teja_uint32_t l2_index_type;
} TejaCPDP_next_hop_info_t;

typedef struct TejaCPDP_redirect_info {
	teja_uint32_t new_ip_saddr;
	teja_uint32_t new_l4_sport;
	teja_uint32_t new_ip_daddr;
	teja_uint32_t new_l4_dport;
	teja_uint32_t egress_port_id;
} TejaCPDP_redirect_info_t;

typedef struct TejaCPDP_redirect_key {
	teja_uint32_t port_id;
	teja_uint32_t ip_daddr;
	teja_uint32_t l4_dport;
	teja_uint32_t l4_protocol;
} TejaCPDP_redirect_key_t;

typedef struct TejaCPDP_legkey {
	teja_uint32_t   hv;
	teja_uint32_t   redir_addr;
} TejaCPDP_legkey_t;


#endif /* _CPDPCOMMON_H */
