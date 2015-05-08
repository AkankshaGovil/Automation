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

#ifndef _CPDPAPICB_H
#define _CPDPAPICB_H

/* data structures */

typedef struct TejaCPDP_ipv4_statistics {
	teja_uint32_t fpRcvdPkts; 
	teja_uint32_t fpDecapRcvdPkts; 
	teja_uint32_t fpIPv4RcvdPkts; 
	teja_uint32_t fpFwdPkts; 
	teja_uint32_t fpDecapDropPkts; 
	teja_uint32_t fpIPv4DropPkts; 
	teja_uint32_t fpDecapExcpPkts; 
	teja_uint32_t fpDecapIPv6ExcpPkts; 
	teja_uint32_t fpDecapArpExcpPkts; 
	teja_uint32_t fpDecapPPPDiscoveryExcpPkts; 
	teja_uint32_t fpDecapPPPSessionExcpPkts; 
	teja_uint32_t fpIPv4ExcpPkts; 
	teja_uint32_t fpDbcastExcpPkts; 
	teja_uint32_t fpInvalidL2EntryExcpPkts; 
	teja_uint32_t fpBadHeaderPkts; 
	teja_uint32_t fpBadTotalLengthPkts; 
	teja_uint32_t fpBadTTLPkts; 
	teja_uint32_t fpLengthTooSmallPkts; 
	teja_uint32_t fpNoRedirectPkts; 
	teja_uint32_t fpBadOutputPortPkts; 
	teja_uint32_t fpBadSendTxPkts; 
	teja_uint32_t epInvalidHeaderPkts;
	teja_uint32_t epInvalidAddressPkts; 
	teja_uint32_t epRcvdPkts; 
	teja_uint32_t epFwdPkts; 
	teja_uint32_t epLocalDeliveryPkts; 
	teja_uint32_t epNoRouteDPPkts; 
	teja_uint32_t epNoRouteCPPkts; 
	teja_uint32_t epNoL2IndexPkts;
	teja_uint32_t epInterfaceDownPkts;
	teja_uint32_t epFragReqdPkts; 
	teja_uint32_t epLimitedBcastPkts; 
	teja_uint32_t epLimitedMcastPkts; 
	teja_uint32_t epIncomingArpRequestPkts; 
	teja_uint32_t epIncomingArpReplyPkts; 
	teja_uint32_t epOutgoingArpRequestPkts; 
	teja_uint32_t epOutgoingArpReplyPkts; 
	teja_uint32_t epArpAlertAllocErr; 
	teja_uint32_t epCreatedICMPMsgPkts; 
	teja_uint32_t epICMPSendFailed; 
	teja_uint32_t epCreatedICMPDestUnReachErrorPkts; 
	teja_uint32_t epCreatedTimeExceedErrorPkts; 
	teja_uint32_t epCreatedParamProblemErrorPkts; 
	teja_uint32_t epCreatedRedirectErrorPkts; 
	teja_uint32_t numberOfRoutes; 
	teja_uint32_t numberOfNextHops; 
	teja_uint32_t rx_packets_received; 
	teja_uint32_t rx_packets_dropped; 
	teja_uint32_t rx_packets_dropped_ctx_st_7; 
	teja_uint32_t rx_packets_dropped_ssp; 
	teja_uint32_t rx_packets_dropped_sesp; 
	teja_uint32_t rx_packets_dropped_mpp; 
	teja_uint32_t rx_packets_dropped_drop_in_error; 
	teja_uint32_t rx_packets_exception; 
	teja_uint32_t rx_bytes_received; 
} TejaCPDP_ipv4_statistics_t;

typedef struct TejaCPDP_ipv6_statistics {
	teja_uint32_t fpInRcvdPktsHi;
	teja_uint32_t fpInRcvdPktsLo;
	teja_uint32_t fpInOctetsHi;
	teja_uint32_t fpInOctetsLo;
	teja_uint32_t fpInForwDatagramsHi;
	teja_uint32_t fpInForwDatagramsLo;
	teja_uint32_t fpInAddrErrorsHi;
	teja_uint32_t fpInAddrErrorsLo;
	teja_uint32_t fpInTruncatedHi;
	teja_uint32_t fpInTruncatedLo;
	teja_uint32_t fpInHdrErrorsHi;
	teja_uint32_t fpInHdrErrorsLo;
	teja_uint32_t fpInDiscardsHi;
	teja_uint32_t fpInDiscardsLo;
	teja_uint32_t fpInNoRoutesHi;
	teja_uint32_t fpInNoRoutesLo;
	teja_uint32_t fpOutTransmitsHi;
	teja_uint32_t fpOutTransmitsLo;
	teja_uint32_t fpOutOctetsHi;
	teja_uint32_t fpOutOctetsLo;
	teja_uint32_t epInDgrams;
	teja_uint32_t epInDelivers;
	teja_uint32_t epInNoRoutes;
	teja_uint32_t epInMulticastPkts;
	teja_uint32_t epInMulticastOctets;
	teja_uint32_t epInDiscards;
	teja_uint32_t epOutRequests;
	teja_uint32_t epOutNoRoutes;
	teja_uint32_t epOutMulticastPkts;
	teja_uint32_t epOutMulticastOctets;
	teja_uint32_t epOutDiscards;
	teja_uint32_t epOutTransmits;
	teja_uint32_t numberOfRoutes;
	teja_uint32_t numberOfNextHops;
} TejaCPDP_ipv6_statistics_t;


/* function prototypes */

/* Transport Library Control */

int TejaCPDP_trans_lib_init_cb(
	TejaCPDP_ctx *ctx);

int TejaCPDP_trans_lib_destroy_cb();

int TejaCPDP_process_fd_cb(
	int fd);

int TejaCPDP_process_signal_cb(
	int sigcode);

int TejaCPDP_process_timer_cb();



/* Response Callback Functions */
void TejaCPDP_generic_cb(
	teja_uint32_t id,
	teja_uint32_t response_code);

void TejaCPDP_add_redirect_cb(
	teja_uint32_t		id,
	teja_uint32_t		response_code,
	TejaCPDP_legkey_t * legkey);

void TejaCPDP_ipv4_sleep_time_cb(
	teja_uint32_t id,
	teja_uint32_t response_code,
	teja_uint32_t sleep_time);

void TejaCPDP_ipv4_queue_depth_cb(
	teja_uint32_t id,
	teja_uint32_t response_code,
	teja_uint32_t queue_depth);

void TejaCPDP_ipv4_packets_to_drain_cb(
	teja_uint32_t id,
	teja_uint32_t response_code,
	teja_uint32_t packet_to_drain);

void TejaCPDP_ipv4_statistics_cb(
	teja_uint32_t id,
	teja_uint32_t response_code,
	TejaCPDP_ipv4_statistics_t *statistics);

/* packet reception */

void TejaCPDP_receive_packet_cb(
	teja_uint32_t in_blade_id,
	teja_uint32_t in_port_id,
	teja_uint32_t len, 
	const teja_uint8_t *pkt);

void TejaCPDP_no_direct_route_exception_packet_cb (
	teja_uint32_t in_blade_id,
	teja_uint32_t in_port_id, 
	teja_uint32_t out_blade_id,
	teja_uint32_t out_port_id,
	teja_uint32_t dest_ip_addr,
	teja_uint32_t len, 
	const teja_uint8_t * pkt);


#endif /* _CPDPAPICB_H */
