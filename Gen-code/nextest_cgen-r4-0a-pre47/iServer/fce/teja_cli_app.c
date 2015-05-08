/********************************************************************/
//
// Copyright (C) Teja Technologies, Inc. 1998-2003.  
//
// All Rights Reserved.  
//
// This software is the property of Teja Technologies, Inc.  It is 
// furnished under a specific licensing agreement.  It may be used 
// or copied only under terms of the licensing agreement.  
//
// For more information, contact info@teja.com 
//
/********************************************************************/
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stdlib.h>

#include <tejatypedefs.h>
#include <cpdpcommon.h>
#include <cpdpapi.h>
#include <cpdpapicb.h>
#include <teja_os_include_all.h>
#include <teja_cli.h>

#include "poll.h"

#ifndef TEJA_MAIN

#include "../../include/srvrlog.h"

#include <trace.h>

#define printf	trc_noop

#endif

extern cli_info_t info;
extern char user_input_str[];
extern char *TejaCPDP_response_code_string[];

char *init_eth_commands[] ={
	"#configure ports",
	"# blade port state  mtu  promiscuous      MAC    ",
	"3  0     0    1    1500    0          0:0:a:1:0:a",
	"3  0     1    1    1500    0          0:0:a:2:0:a", "#change link state",
	"# blade port state",
	"5   0    0     1  ", "5   0    1     1  ", "# add interface",
	"# blade port   addr          mask      nh_id",
	"1   0    0   10.1.0.10  255.255.255.0    5",
	"1   0    1   10.2.0.10  255.255.255.0    6", "# add Static ARP l2 index",
	"# l2_index  port   addr      MAC      ",
	"e  1          0  10.1.0.1  0:0:a:1:0:1",
	"e  2          1  10.2.0.2  0:0:a:2:0:2", "# add nexthop",
	"#  nh_id   addr   blade port mtu flags(0-4) l2_index",
	"9  1     10.1.0.1   0      0 1500     0          1   ",
	"9  2     10.2.0.2   0      1 1500     0          2   ", "# add route",
	"#  addr       mask             nexthop_id",
	"6  10.1.0.1  255.255.255.255       1      ",
	"6  10.2.0.2  255.255.255.255       2      ",
	"6  12.1.0.0  255.255.0.0           1      ",
	"6  13.1.1.0  255.255.255.0         2      ",
	"6  14.1.1.2  255.255.255.254       1      ", ""
};

char *init_pos_commands[] = {
	"#configure ports",
	"# blade port state  mtu  ",
	"3  0     0    1    1500  ", "3  0     1    1    1500  ",
	"#change link state",
	"# blade port state", "5   0    0     1  ", "5   0    1     1  ",
	"# add interface",
	"# blade port   addr          mask      nh_id",
	"1   0    0   10.1.0.10  255.255.255.0   5",
	"1   0    1   10.2.0.10  255.255.255.0   6", "# add nexthop",
	"#  nh_id   addr   blade port mtu flags(0-4) l2_index",
	"9  1     10.1.0.1   0      0 1500     0          1   ",
	"9  2     10.2.0.2   0      1 1500     0          2   ", "# add route",
	"#  addr       mask          nexthop_id",
	"6  12.1.0.0  255.255.0.0       1      ",
	"6  13.1.1.0  255.255.255.0     2      ",
	"6  14.1.1.2  255.255.255.254   1      ", ""
};

void
top_menu_content(void)
{
	if (info.init_done) {
		printf(	"\nChoose one of the following to configure Blade (ID %d)\n\n"
			   	"    1. Add Interface                2. Delete Interface\n"
			#if (L2_TYPE == L2_POS)
		   		"    3. Configure POS Port\n"
			#elif (L2_TYPE == L2_ETH)
		   		"    3. Configure Ethernet Port\n"
			#else /* */
				#error L2_TYPE not defined
			#endif /* */
				"    4. Change Port State            5. Change Link State\n"
				"    6. Add Routes                   7. Update Routes\n"
				"    8. Delete Routes                9. Add Next Hop\n"
				"    a. Update Next Hop              b. Delete Next Hop\n"
				"    c. Change Next Hop MTU          d. Change Next Hop Flag\n"
			#if (L2_TYPE == L2_ETH)
				"    e. Add Static ARP Entry         f. Create Dynamic ARP Entry\n"
				"    g. Delete ARP Entry\n"
			#endif /* */
				"    h. Get Sleep Time               i. Set Sleep Time\n"
				"    j. Get Pkts To Drain            k. Set Pkts To Drain\n"
				"    l. Get Queue Depth              m. Get IPv4 Stats\n"
				"    n. Display Interface            o. Display Ports\n"
				"    p. Display Routes               r. Purge Routes\n"
				"    s. Display Next Hops            t. Purge RTM\n"
			#if (L2_TYPE == L2_ETH)
				"    u. Display DBCastTable          v. Display L2 Filter Table\n"
				"    w. Display L2 Table             x. Purge L2 Table\n"
				"    y. Display ARP Cache            z. Purge ARP Cache\n"
			#endif /* */
				"    q. Quit" "\nChoice: ",
				info.blade_id );
	}
}

#if 0
int
get_char_from_socket(char *new_char)
{
	char next_char;
	int n;

	if (info.cli_app_socket == TEJA_INVALID_SOCKET) {
		printf("get_char_from_socket(): CLI not connected yet!\n");
	} else {

		do {
			if ((n = recv(	info.cli_app_socket,
							&next_char,
							1,
							0)) < 0) {
				printf(	"get_char_from_socket(): recv() - returned error\n");
				return -1;
			}

			// else { printf("get_char_from_socket () got %c\n", next_char); } 

		} while (n == 0);
	}

	*new_char = next_char;
	return 0;
}
#endif

int
get_char_from_socket(char *new_char)
{
	char 			next_char;
	int 			n, nready;
	struct pollfd	filedes;

	if (info.cli_app_socket == TEJA_INVALID_SOCKET) {
		printf("get_char_from_socket(): CLI not connected yet!\n");
	} else {

		memset( &filedes, (int) 0, sizeof( struct pollfd ) );
		filedes.fd = info.cli_app_socket;
		filedes.events = POLLIN;

		while(1) {
again:
			nready = poll( &filedes, 1, 2000 );
			if ( (errno == EINTR) || (errno == EAGAIN) )
				goto again;

			if ( nready < 0 ) {
				printf( "Poll error for sockfd %d\n", filedes.fd );
				break;
			} else if ( nready == 1 ) {
				if ( filedes.revents & POLLNVAL ) {
					printf( "Poll error - POLLNVAL - for sockfd %d\n", filedes.fd );
					break;
				}

				if ( filedes.revents & POLLIN ) {

					if ((n = recv(	info.cli_app_socket,
									&next_char,
									1,
									0)) < 0) {
						printf(	"get_char_from_socket(): recv() returned error\n");
						break;
					}
					*new_char = next_char;
					return 0;
				}
			}
		}
	}

	return -1;
}

teja_uint32_t
get_input_from_socket(void)
{
	int index;

	teja_uint32_t ret_code;

	ret_code = 0;
	while (ret_code == 0) {
		index = 0;

		while (index < USER_INPUT_BUFFER_SIZE) {
			if (index == 0) {
				// eat all the empty characters 
				while (1) {
					ret_code = get_char_from_socket(&user_input_str[index]);
					if (ret_code != 0)
						return -1;
					if ((strrchr(" \t\n", user_input_str[index]) == NULL))
						break;
				}
			} else {
				ret_code = get_char_from_socket(&user_input_str[index]);
				if (ret_code != 0)
					return -1;
			}

			if ((user_input_str[index] == EOF) ||
				(strrchr(" \t\n", user_input_str[index]) != NULL)) {
				break;
			}

			index = index + 1;
		}

		if ((strrchr(" \t\n", user_input_str[index]) == NULL) &&
			(user_input_str[index] != EOF)) {
			printf("\nInput string too long!!\n");
			ret_code = ERR_INPUT_STRING_TOO_LONG;
		} else if (user_input_str[0] == EOF) {
			printf("\nReached Unexpected End Of File!!\n");
			ret_code = INTERNAL;
		} else if (user_input_str[0] == '#') {

			// it is a comment. skip the entire line 

			while (	(user_input_str[index] != '\n') &&
					(user_input_str[index] != EOF)) {
				ret_code = get_char_from_socket(&user_input_str[index]);
				if (ret_code != 0)
					return -1;
			}
		} else {

			 // null terminate the string 

			user_input_str[index] = '\0';
			break;
		}
	}
	return ret_code;
}

teja_uint32_t
get_num_from_socket(teja_uint32_t * p_num)
{
	teja_uint32_t ret_code;
	int scan_code;

	ret_code = 0;
	ret_code = get_input_from_socket();

	if (ret_code != 0) {
		return ret_code;
	}

	scan_code = sscanf(user_input_str, "%d", p_num);

	if (scan_code != 1) {
		ret_code = -1;
	}

	return ret_code;
}

int
generic_response(void)
{
	teja_uint32_t response_code;
	int ret_code = 0;

	if ((ret_code = get_num_from_socket(&response_code)) != 0) {
		return( -1 );
	}

	return( response_code );
}

int
ipv4_sleep_time_response(void)
{
	teja_uint32_t response_code,
	              sleep_time;
	int ret_code = 0;

	if ((ret_code = get_num_from_socket(&response_code)) != 0) {
		printf("ipv4_sleep_time_response():failed to receive response\n");
	} else if ((ret_code = get_num_from_socket(&sleep_time)) != 0) {
		printf("ipv4_sleep_time_response():failed to receive response\n");
	} else if (response_code == 0) {
		printf("%s: SUCCESS\n", info.request_string);
		printf("sleep time %d\n", sleep_time);
	} else {
		printf("%s: FAILED response_code %s\n", info.request_string,
			   TejaCPDP_response_code_string[response_code]);
	}
	return ret_code;
}

int
ipv4_queue_depth_response(void)
{
	teja_uint32_t response_code, queue_depth;
	int ret_code = 0;

	if ((ret_code = get_num_from_socket(&response_code)) != 0) {
		printf("ipv4_queue_depth_response():failed to receive response\n");
	} else if ((ret_code = get_num_from_socket(&queue_depth)) != 0) {
		printf("ipv4_queue_depth_response():failed to receive response\n");
	} else if (response_code == 0) {
		printf("%s: SUCCESS\n", info.request_string);
		printf("queue depth %d\n", queue_depth);
	} else {
		printf("%s: FAILED response_code %s\n", info.request_string,
			   TejaCPDP_response_code_string[response_code]);
	}

	return ret_code;
}

int
ipv4_packets_to_drain_response(void)
{
	teja_uint32_t response_code,
	              packet_to_drain;
	int ret_code = 0;

	if ((ret_code = get_num_from_socket(&response_code)) != 0) {
		printf("ipv4_packet_to_drain_response():failed to receive response\n");
	} else if ((ret_code = get_num_from_socket(&packet_to_drain)) != 0) {
		printf("ipv4_packet_to_drain_response():failed to receive response\n");
	} else if (response_code == 0) {
		printf("%s: SUCCESS\n", info.request_string);
		printf("packets to drain %d\n", packet_to_drain);
	} else {
		printf("%s: FAILED response_code %s\n", info.request_string,
			   TejaCPDP_response_code_string[response_code]);
	}
	return ret_code;
}

int
ipv4_statistics_response(void)
{
	TejaCPDP_ipv4_statistics_t statistics;
	teja_uint32_t response_code;
	int ret_code = 0;

	if ((ret_code = get_num_from_socket(&response_code)) != 0) {
		printf("ipv4_statistics_response ():failed to receive response\n");
	} else if ((ret_code = 
		get_num_from_socket(&statistics.fpRcvdPkts)) != 0) {
		printf("ipv4_statistics_response ():failed to receive response\n");
	} else if ((ret_code = 
		get_num_from_socket(&statistics.fpDecapRcvdPkts)) != 0) {
		printf("ipv4_statistics_response ():failed to receive response\n");
	} else if ((ret_code = 
		get_num_from_socket(&statistics.fpIPv4RcvdPkts)) != 0) {
		printf("ipv4_statistics_response ():failed to receive response\n");
	} else if ((ret_code = 
		get_num_from_socket(&statistics.fpFwdPkts)) != 0) {
		printf("ipv4_statistics_response ():failed to receive response\n");
	} else if ((ret_code = 
		get_num_from_socket(&statistics.fpDecapDropPkts)) != 0) {
		printf("ipv4_statistics_response ():failed to receive response\n");
	} else if ((ret_code = 
		get_num_from_socket(&statistics.fpIPv4DropPkts)) != 0) {
		printf("ipv4_statistics_response ():failed to receive response\n");
	} else if ((ret_code = 
		get_num_from_socket(&statistics.fpDecapExcpPkts)) != 0) {
		printf("ipv4_statistics_response ():failed to receive response\n");
	} else if ((ret_code = 
		get_num_from_socket(&statistics.fpDecapIPv6ExcpPkts)) != 0) {
		printf("ipv4_statistics_response ():failed to receive response\n");
	} else if ((ret_code = 
		get_num_from_socket(&statistics.fpDecapArpExcpPkts)) != 0) {
		printf("ipv4_statistics_response ():failed to receive response\n");
	} else if ((ret_code = 
		get_num_from_socket(&statistics.fpDecapPPPDiscoveryExcpPkts)) != 0) {
		printf("ipv4_statistics_response ():failed to receive response\n");
	} else if ((ret_code = 
		get_num_from_socket(&statistics.fpDecapPPPSessionExcpPkts)) != 0) {
		printf("ipv4_statistics_response ():failed to receive response\n");
	} else if ((ret_code = 
		get_num_from_socket(&statistics.fpIPv4ExcpPkts)) != 0) {
		printf("ipv4_statistics_response ():failed to receive response\n");
	} else if ((ret_code = 
		get_num_from_socket(&statistics.fpDbcastExcpPkts)) != 0) {
		printf("ipv4_statistics_response ():failed to receive response\n");
	} else if ((ret_code = 
		get_num_from_socket(&statistics.fpInvalidL2EntryExcpPkts)) != 0) {
		printf("ipv4_statistics_response ():failed to receive response\n");
	} else if ((ret_code = 
		get_num_from_socket(&statistics.fpBadHeaderPkts)) != 0) {
		printf("ipv4_statistics_response ():failed to receive response\n");
	} else if ((ret_code = 
		get_num_from_socket(&statistics.fpBadTotalLengthPkts)) != 0) {
		printf("ipv4_statistics_response ():failed to receive response\n");
	} else if ((ret_code = 
		get_num_from_socket(&statistics.fpBadTTLPkts)) != 0) {
		printf("ipv4_statistics_response ():failed to receive response\n");
	} else if ((ret_code = 
		get_num_from_socket(&statistics.fpLengthTooSmallPkts)) != 0) {
		printf("ipv4_statistics_response ():failed to receive response\n");
	} else if ((ret_code = 
		get_num_from_socket(&statistics.fpNoRedirectPkts)) != 0) {
		printf("ipv4_statistics_response ():failed to receive response\n");
	} else if ((ret_code = 
		get_num_from_socket(&statistics.fpBadOutputPortPkts)) != 0) {
		printf("ipv4_statistics_response ():failed to receive response\n");
	} else if ((ret_code = 
		get_num_from_socket(&statistics.fpBadSendTxPkts)) != 0) {
		printf("ipv4_statistics_response ():failed to receive response\n");
	} else if ((ret_code = 
		get_num_from_socket(&statistics.epInvalidHeaderPkts)) != 0) {
		printf("ipv4_statistics_response ():failed to receive response\n");
	} else if ((ret_code = 
		get_num_from_socket(&statistics.epInvalidAddressPkts)) != 0) {
		printf("ipv4_statistics_response ():failed to receive response\n");
	} else if ((ret_code = 
		get_num_from_socket(&statistics.epRcvdPkts)) != 0) {
		printf("ipv4_statistics_response ():failed to receive response\n");
	} else if ((ret_code = 
		get_num_from_socket(&statistics.epFwdPkts)) != 0) {
		printf("ipv4_statistics_response ():failed to receive response\n");
	} else if ((ret_code = 
		get_num_from_socket(&statistics.epLocalDeliveryPkts)) != 0) {
		printf("ipv4_statistics_response ():failed to receive response\n");
	} else if ((ret_code = 
		get_num_from_socket(&statistics.epNoRouteDPPkts)) != 0) {
		printf("ipv4_statistics_response ():failed to receive response\n");
	} else if ((ret_code = 
		get_num_from_socket(&statistics.epNoRouteCPPkts)) != 0) {
		printf("ipv4_statistics_response ():failed to receive response\n");
	} else if ((ret_code = 
		get_num_from_socket(&statistics.epNoL2IndexPkts)) != 0) {
		printf("ipv4_statistics_response ():failed to receive response\n");
	} else if ((ret_code = 
		get_num_from_socket(&statistics.epInterfaceDownPkts)) != 0) {
		printf("ipv4_statistics_response ():failed to receive response\n");
	} else if ((ret_code = 
		get_num_from_socket(&statistics.epFragReqdPkts)) != 0) {
		printf("ipv4_statistics_response ():failed to receive response\n");
	} else if ((ret_code = 
		get_num_from_socket(&statistics.epLimitedBcastPkts)) != 0) {
		printf("ipv4_statistics_response ():failed to receive response\n");
	} else if ((ret_code = 
		get_num_from_socket(&statistics.epLimitedMcastPkts)) != 0) {
		printf("ipv4_statistics_response ():failed to receive response\n");
	} else if ((ret_code = 
		get_num_from_socket(&statistics.epIncomingArpRequestPkts)) != 0) {
		printf("ipv4_statistics_response ():failed to receive response\n");
	} else if ((ret_code = 
		get_num_from_socket(&statistics.epIncomingArpReplyPkts)) != 0) {
		printf("ipv4_statistics_response ():failed to receive response\n");
	} else if ((ret_code = 
		get_num_from_socket(&statistics.epOutgoingArpRequestPkts)) != 0) {
		printf("ipv4_statistics_response ():failed to receive response\n");
	} else if ((ret_code = 
		get_num_from_socket(&statistics.epOutgoingArpReplyPkts)) != 0) {
		printf("ipv4_statistics_response ():failed to receive response\n");
	} else if ((ret_code = 
		get_num_from_socket(&statistics.epArpAlertAllocErr)) != 0) {
		printf("ipv4_statistics_response ():failed to receive response\n");
	} else if ((ret_code = 
		get_num_from_socket(&statistics.epCreatedICMPMsgPkts)) != 0) {
		printf("ipv4_statistics_response ():failed to receive response\n");
	} else if ((ret_code = 
		get_num_from_socket(&statistics.epICMPSendFailed)) != 0) {
		printf("ipv4_statistics_response ():failed to receive response\n");
	} else if ((ret_code =
		get_num_from_socket(&statistics.epCreatedICMPDestUnReachErrorPkts)) != 0) {
		printf("ipv4_statistics_response ():failed to receive response\n");
	} else if ((ret_code =
		get_num_from_socket(&statistics.epCreatedTimeExceedErrorPkts)) != 0) {
		printf("ipv4_statistics_response ():failed to receive response\n");
	} else if ((ret_code =
		get_num_from_socket(&statistics.epCreatedParamProblemErrorPkts)) != 0) {
		printf("ipv4_statistics_response ():failed to receive response\n");
	} else if ((ret_code =
		get_num_from_socket(&statistics.epCreatedRedirectErrorPkts)) != 0) {
		printf("ipv4_statistics_response ():failed to receive response\n");
	} else if ((ret_code = 
		get_num_from_socket(&statistics.numberOfRoutes)) != 0) {
		printf("ipv4_statistics_response ():failed to receive response\n");
	} else if ((ret_code = 
		get_num_from_socket(&statistics.numberOfNextHops)) != 0) {
		printf("ipv4_statistics_response ():failed to receive response\n");
	} else if ((ret_code = 
		get_num_from_socket(&statistics.rx_packets_received)) != 0) {
		printf("ipv4_statistics_response ():failed to receive response\n");
	} else if ((ret_code = 
		get_num_from_socket(&statistics.rx_packets_dropped)) != 0) {
		printf("ipv4_statistics_response ():failed to receive response\n");
	} else if ((ret_code = 
		get_num_from_socket(&statistics.rx_packets_dropped_ctx_st_7)) != 0) {
		printf("ipv4_statistics_response ():failed to receive response\n");
	} else if ((ret_code = 
		get_num_from_socket(&statistics.rx_packets_dropped_ssp)) != 0) {
		printf("ipv4_statistics_response ():failed to receive response\n");
	} else if ((ret_code = 
		get_num_from_socket(&statistics.rx_packets_dropped_sesp)) != 0) {
		printf("ipv4_statistics_response ():failed to receive response\n");
	} else if ((ret_code = 
		get_num_from_socket(&statistics.rx_packets_dropped_mpp)) != 0) {
		printf("ipv4_statistics_response ():failed to receive response\n");
	} else if ((ret_code = 
		get_num_from_socket(&statistics.rx_packets_dropped_drop_in_error)) != 0) {
		printf("ipv4_statistics_response ():failed to receive response\n");
	} else if ((ret_code = 
		get_num_from_socket(&statistics.rx_packets_exception)) != 0) {
		printf("ipv4_statistics_response ():failed to receive response\n");
	} else if ((ret_code = 
		get_num_from_socket(&statistics.rx_bytes_received)) != 0) {
		printf("ipv4_statistics_response ():failed to receive response\n");
	} else if (response_code == 0) {
		printf("%s: SUCCESS\n", info.request_string);
		printf("  IPv4 Fast Path Statistics\n\n" );

		printf("    Total Rcvd Pkts                   %d\n",
				statistics.fpRcvdPkts);
		if ( statistics.fpDecapRcvdPkts != 0 )
		printf("          Decap Rcvd Pkts             %d\n",
				statistics.fpDecapRcvdPkts);
		if ( statistics.fpIPv4RcvdPkts != 0 )
		printf("          IPv4 Rcvd Pkts              %d\n",
				statistics.fpIPv4RcvdPkts);

		printf("    FwdPkts                           %d\n",
				statistics.fpFwdPkts);

		if ( statistics.fpDecapDropPkts != 0 )
		printf("    Decap Drop Pkts                   %d\n",
				statistics.fpDecapDropPkts);
		if ( statistics.fpIPv4DropPkts != 0 )
		printf("    IPv4 Drop Pkts                    %d\n",
				statistics.fpIPv4DropPkts);
		if ( statistics.fpDecapExcpPkts != 0 )
		printf("    Decap Excp Pkts                   %d\n",
				statistics.fpDecapExcpPkts);
		if ( statistics.fpDecapIPv6ExcpPkts != 0 )
		printf("          IPv6 Excp Pkts              %d\n",
				statistics.fpDecapIPv6ExcpPkts);
		if ( statistics.fpDecapArpExcpPkts != 0 )
		printf("          Arp Excp Pkts               %d\n",
				statistics.fpDecapArpExcpPkts);
		if ( statistics.fpDecapPPPDiscoveryExcpPkts != 0 )
		printf("          PPP Discovery Excp Pkts     %d\n",
				statistics.fpDecapPPPDiscoveryExcpPkts);
		if ( statistics.fpDecapPPPSessionExcpPkts != 0 )
		printf("          PPP Session Excp Pkts       %d\n",
				statistics.fpDecapPPPSessionExcpPkts);
		if ( statistics.fpIPv4ExcpPkts != 0 )
		printf("    IPv4 Excp Pkts                    %d\n",
				statistics.fpIPv4ExcpPkts);

		if ( statistics.fpDbcastExcpPkts != 0 )
		printf("        Dbcast Excp Pkts              %d\n",
				statistics.fpDbcastExcpPkts);

		if ( statistics.fpInvalidL2EntryExcpPkts != 0 )
		printf("        Invalid L2 Entry Excp Pkts    %d\n",
				statistics.fpInvalidL2EntryExcpPkts);

		if ( statistics.fpBadHeaderPkts != 0 )
			printf("    BadHeaderPkts                     %d\n", statistics.fpBadHeaderPkts);
		if ( statistics.fpBadTotalLengthPkts != 0 )
			printf("    BadTotalLengthPkts                %d\n", statistics.fpBadTotalLengthPkts);
		if ( statistics.fpBadTTLPkts != 0 )
			printf("    BadTTLPkts                        %d\n", statistics.fpBadTTLPkts);
		if ( statistics.fpLengthTooSmallPkts != 0 )
			printf("    LengthTooSmallPkts                %d\n", statistics.fpLengthTooSmallPkts);
		if ( statistics.fpNoRedirectPkts != 0 )
			printf("    fpNoRedirectPkts                  %d\n", statistics.fpNoRedirectPkts);
		if ( statistics.fpBadOutputPortPkts != 0 )
			printf("    fpBadOutputPortPkts               %d\n", statistics.fpBadOutputPortPkts);
		if ( statistics.fpBadSendTxPkts != 0 )
			printf("    fpBadSendTxPkts                   %d\n", statistics.fpBadSendTxPkts);

		printf("\n  Exception Path Statistics\n\n" );

		if ( statistics.epInvalidHeaderPkts != 0 )
			printf("    InvalidHeaderPkts                 %d\n", statistics.epInvalidHeaderPkts);
		if ( statistics.epInvalidAddressPkts != 0 )
			printf("    InvalidAddressPkts                %d\n", statistics.epInvalidAddressPkts);
		if ( statistics.epRcvdPkts != 0 )
			printf("    RcvdPkts                          %d\n", statistics.epRcvdPkts);
		if ( statistics.epFwdPkts != 0 )
			printf("    FwdPkts                           %d\n", statistics.epFwdPkts);
		if ( statistics.epLocalDeliveryPkts != 0 )
			printf("    LocalDeliveryPkts                 %d\n", statistics.epLocalDeliveryPkts);
		if ( statistics.epNoRouteDPPkts != 0 )
			printf("    NoRoutePkts ( data plane )        %d\n", statistics.epNoRouteDPPkts);
		if ( statistics.epNoRouteCPPkts != 0 )
			printf("    NoRoutePkts ( control plane )     %d\n", statistics.epNoRouteCPPkts);
		if ( statistics.epNoL2IndexPkts != 0 )
			printf("    No L2 Index found pkts            %d\n", statistics.epNoL2IndexPkts);
		if ( statistics.epInterfaceDownPkts != 0 )
			printf("    Interface down pkts               %d\n", statistics.epInterfaceDownPkts);
		if ( statistics.epFragReqdPkts != 0 )
			printf("    Fragment Reqd Pkts                %d\n", statistics.epFragReqdPkts);
		if ( statistics.epLimitedBcastPkts != 0 )
			printf("    Limited Broadcast Pkts            %d\n", statistics.epLimitedBcastPkts);
		if ( statistics.epLimitedMcastPkts != 0 )
			printf("    Limited Multicast Pkts            %d\n", statistics.epLimitedMcastPkts);
		if ( statistics.epIncomingArpRequestPkts != 0 )
			printf("    Incoming Arp Request Packets      %d\n", statistics.epIncomingArpRequestPkts);
		if ( statistics.epIncomingArpReplyPkts != 0 )
			printf("    Incoming Arp Reply Packets        %d\n", statistics.epIncomingArpReplyPkts);
		if ( statistics.epOutgoingArpRequestPkts != 0 )
			printf("    Outgoing Arp Request Packets      %d\n", statistics.epOutgoingArpRequestPkts);
		if ( statistics.epOutgoingArpReplyPkts != 0 )
			printf("    Outgoing Arp Reply Packets        %d\n", statistics.epOutgoingArpReplyPkts);
		if ( statistics.epArpAlertAllocErr != 0 )
			printf("    Arp Alert Alloc Errors            %d\n", statistics.epArpAlertAllocErr);
		if ( statistics.epCreatedICMPMsgPkts != 0 )
			printf("    ICMP Msg Pkts sent                %d\n", statistics.epCreatedICMPMsgPkts);
		if ( statistics.epICMPSendFailed != 0 )
			printf("    ICMP Send Failed Pkts             %d\n", statistics.epICMPSendFailed);
		if ( statistics.epCreatedICMPDestUnReachErrorPkts != 0 )
			printf("    ICMP DestUnReachErrorPkts sent    %d\n", statistics.epCreatedICMPDestUnReachErrorPkts);
		if ( statistics.epCreatedTimeExceedErrorPkts != 0 )
			printf("    ICMP TimeExceedPErrorkts sent     %d\n", statistics.epCreatedTimeExceedErrorPkts);
		if ( statistics.epCreatedParamProblemErrorPkts != 0 )
		    printf("    ICMP ParamProblemError Pkts sent  %d\n", statistics.epCreatedParamProblemErrorPkts);
		if ( statistics.epCreatedRedirectErrorPkts != 0 )
			printf("    ICMP RedirectError Pkts sent      %d\n", statistics.epCreatedRedirectErrorPkts);

		printf("\n  General Statistics\n\n" );

		printf("    numberOfRoutes                    %d\n", statistics.numberOfRoutes);
		printf("    numberOfNextHops                  %d\n", statistics.numberOfNextHops);

		printf("\n  Rx Packet Statistics\n\n" );
		printf("    packets received                  %d\n", statistics.rx_packets_received);
		printf("    packets dropped                   %d\n",
		 				(statistics.rx_packets_dropped_ctx_st_7 + statistics.rx_packets_dropped_ssp +
						 statistics.rx_packets_dropped_sesp + statistics.rx_packets_dropped_mpp +
						 statistics.rx_packets_dropped_drop_in_error) );
		if ( statistics.rx_packets_dropped_ctx_st_7 != 0 )
	    	printf("                    (ctx_st = 7)      %d\n", statistics.rx_packets_dropped_ctx_st_7);
		if ( statistics.rx_packets_dropped_ssp != 0 )
			printf("                    (ssp processing)  %d\n", statistics.rx_packets_dropped_ssp);
		if ( statistics.rx_packets_dropped_sesp != 0 )
			printf("                    (sesp processing) %d\n", statistics.rx_packets_dropped_sesp);
		if ( statistics.rx_packets_dropped_mpp != 0 )
			printf("                    (mpp processing)  %d\n", statistics.rx_packets_dropped_mpp);
		if ( statistics.rx_packets_dropped_drop_in_error != 0 )
			printf("                    (drop in error)   %d\n", statistics.rx_packets_dropped_drop_in_error);
		printf("    packet exceptions                 %d\n", statistics.rx_packets_exception);
		printf("    bytes received                    %d\n", statistics.rx_bytes_received);
	} else {
		printf(	"\n%s: FAILED response_code %s\n",
				info.request_string,
				TejaCPDP_response_code_string[response_code]);
	}
	return ret_code;
}

int
add_redirect_response(	uint32_t * p_hk_hv,
						uint32_t * p_hk_redir_addr )
{
	TejaCPDP_legkey_t legkey;
	teja_uint32_t response_code;
	int ret_code = 0;

	if ((ret_code = get_num_from_socket(&response_code)) != 0) {
		printf("add_redirect_response ():failed to receive response\n");
	} else if ((ret_code = 
		get_num_from_socket(&legkey.hv)) != 0) {
		printf("add_redirect_response ():failed to receive response\n");
	} else if ((ret_code = 
		get_num_from_socket(&legkey.redir_addr)) != 0) {
		printf("add_redirect_response ():failed to receive response\n");
	}

	*p_hk_hv = legkey.hv;
	*p_hk_redir_addr = legkey.redir_addr;
	return ret_code;
}

static int
add_arp_response( uint32_t * p_l2_index )
{
	teja_uint32_t response_code, l2_index;
	int ret_code = 0;

	if ((ret_code = get_num_from_socket(&response_code)) != 0) {
		printf("add_arp_response():failed to receive response\n");
	} else if ((ret_code = get_num_from_socket(&l2_index)) != 0) {
		printf("add_arp_response():failed to receive response\n");
	} else if (response_code == 0) {
		*p_l2_index = l2_index;
	} else {
		printf("%s: FAILED response_code %s\n", info.request_string,
			   TejaCPDP_response_code_string[response_code]);
	}

	return ret_code;
}

int
wait_for_response_add_redirect( uint32_t * p_hk_hv, 
								uint32_t * p_hk_redir_addr )
{
	char reply_id;
	int ret_code = 0;

	if (get_char_from_socket(&reply_id) != 0) {
		printf("wait_for_response_add_redirect(): get_char() returned error\n");
		ret_code = -1;
	} else {
		switch (reply_id)
		{
		case REPLY_ADD_REDIRECT:
			ret_code = add_redirect_response( p_hk_hv, p_hk_redir_addr );
			break;
		default:
			ret_code = -1;
			printf(	"wait_for_response_add_redirect() received invalid reply id %c\n",
					reply_id);
		}
	}
	return ret_code;
}

int
wait_for_response_add_arp( uint32_t * p_l2_index )
{
	char reply_id;
	int ret_code = 0;

	if (get_char_from_socket(&reply_id) != 0) {
		printf("wait_for_response_add_arp(): get_char() returned error\n");
		ret_code = -1;
	} else {
		switch (reply_id)
		{
		case REPLY_ADD_ARP:
			ret_code = add_arp_response( p_l2_index );
			break;
		default:
			ret_code = -1;
			printf(	"wait_for_response_add_arp() received invalid reply id %c\n",
					reply_id);
		}
	}
	return ret_code;
}

int
wait_for_response(void)
{
	char reply_id;
	int ret_code = 0;

	if (get_char_from_socket(&reply_id) != 0) {
		printf("wait_for_response(): get_char() returned error\n");
		ret_code = -1;
	} else {
		switch (reply_id)
		{
		case REPLY_GENERIC:
			ret_code = generic_response();
			break;
		case REPLY_IPV4_SLEEP_TIME:
			ret_code = ipv4_sleep_time_response();
			break;
		case REPLY_IPV4_QUEUE_DEPTH:
			ret_code = ipv4_queue_depth_response();
			break;
		case REPLY_IPV4_PACKETS_TO_DRAIN:
			ret_code = ipv4_packets_to_drain_response();
			break;
		case REPLY_IPV4_STATS:
			ret_code = ipv4_statistics_response();
			break;
		default:
			ret_code = -1;
			printf(	"wait_for_response() received invalid reply id %c\n",
					reply_id);
		}
	}
	return ret_code;
}

void
printf_usage(void)
{
	printf("\nUsage: cli_app [-options] [ < script]\n");
	printf("options:\n");
	printf(" -i <addr>   IP addr where CLI Agent is runnning. Default (127.0.0.1)\n");
	printf(" -p          Preconfigure blade before user interface comes up.\n");
	printf(" < script    Redirect a script file.\n");
	printf(" -h          Print usage.\n");
}

#ifdef TEJA_MAIN

int
extract_args(int argc, char **argv)
{
	int index = 1;

	info.agent_ip_addr[0] = '\0';

	if (argc > 1) {
		while (index < argc) {
			if (strcmp(argv[index], "-h") == 0) {
				printf_usage();
				return -1;
			} else if (strcmp(argv[index], "-i") == 0) {
				index++;
				if (strlen(argv[index]) > USER_INPUT_BUFFER_SIZE - 1) {
					printf("-ip_addr option parameter too long\n");
					printf_usage();
					return -1;
				}
				strcpy(info.agent_ip_addr, argv[index]);
			} else if (strcmp(argv[index], "-p") == 0) {
				info.init_done = 0;
			} else {
				printf("unknown option\n");
				printf_usage();
				return -1;
			}
			index++;
		}
	}

	/*
	 * initialize to default if not already initialized 
	 */

	if (info.agent_ip_addr[0] == '\0') {
		strcpy(info.agent_ip_addr, "127.0.0.1");
	}
	return 0;
}

int
teja_connect_to_agent(void)
{
	TejaSockaddrIn serv_addr;
	int addr;

	teja_uint32_t agent_code = 0;
	teja_uint32_t client_number;

	if ((info.cli_app_socket = 
		socket(AF_INET, SOCK_STREAM, 0)) == TEJA_INVALID_SOCKET) {
		printf("Failed to allocate socket\n");
		return -1;
	}

	serv_addr.sin_family = TEJA_AF_INET;

	if ( ( addr = inet_addr( info.agent_ip_addr ) ) < 0 ) {
		printf ("Failed to allocate socket\n");
		return -1;
	}

	serv_addr.sin_addr.s_addr = addr;
	serv_addr.sin_port = htons(CLI_AGENT_LISTEN_PORT);

	if ( teja_connect( info.cli_app_socket,
						(TejaSockaddr *) &serv_addr,
						sizeof(serv_addr)) < 0) {
		printf("Failed to connect to IP address %s Port %d\n",
			   info.agent_ip_addr, CLI_AGENT_LISTEN_PORT);
		return -1;
	} 

	if ( get_num_from_socket(&info.blade_id) != 0) {
		printf("teja_connect_to_agent(): failed to receive response\n");
		return -1;
	} 

	if (get_num_from_socket(&agent_code) != 0) {
		printf("teja_connect_to_agent(): failed to receive response\n");
		return -1;
	} 

	if (agent_code == CLI_AGENT_RESERVE_SUCCESS) {

		if ( get_num_from_socket(&client_number) != 0) {
			printf("teja_connect_to_agent(): unable to get client # from server\n");
			return -1;
		}

		printf("\nEstablished connection with Blade ID %d @ %s\n",
			   info.blade_id, info.agent_ip_addr);
	} else {
		printf( "\nService refused by Blade ID %d @ %s. "
				"Agent in use with other CLI\n",
			 	info.blade_id,
				info.agent_ip_addr);
		return -1;
	}
	return 0;
}

#else

int
teja_connect_to_agent( char * agent_ip )
{
	TejaSockaddrIn serv_addr;
	int addr;

	teja_uint32_t agent_code = 0;
	teja_uint32_t client_number;

	info.top_menu_selection = 'q';
	info.init_done = 1;
	info.blade_id = 0;
	info.cli_app_socket = TEJA_INVALID_SOCKET;

	strcpy( info.agent_ip_addr, agent_ip );


	if ((info.cli_app_socket = 
		socket(AF_INET, SOCK_STREAM, 0)) == TEJA_INVALID_SOCKET) {
		printf("Failed to allocate socket\n");
		return -1;
	}

	serv_addr.sin_family = TEJA_AF_INET;

	if ( ( addr = inet_addr( info.agent_ip_addr ) ) < 0 ) {
		printf ("Failed to allocate socket\n");
		return -1;
	}

	serv_addr.sin_addr.s_addr = addr;
	serv_addr.sin_port = htons(CLI_AGENT_LISTEN_PORT);

	if ( teja_connect( info.cli_app_socket,
						(TejaSockaddr *) &serv_addr,
						sizeof(serv_addr)) < 0) {
		printf("Failed to connect to IP address %s Port %d\n",
			   info.agent_ip_addr, CLI_AGENT_LISTEN_PORT);
		return -1;
	} 

	if ( get_num_from_socket(&info.blade_id) != 0) {
		printf("teja_connect_to_agent(): failed to receive response\n");
		return -1;
	} 

	if ( get_num_from_socket(&agent_code) != 0) {
		printf("teja_connect_to_agent(): failed to receive response\n");
		return -1;
	} 

	if (agent_code == CLI_AGENT_RESERVE_SUCCESS) {

		if ( get_num_from_socket(&client_number) != 0) {
			printf("teja_connect_to_agent(): unable to get client # from server\n");
			return -1;
		}

		printf("\nEstablished connection with Blade ID %d @ %s\n",
			   info.blade_id, info.agent_ip_addr);
	} else {
		printf( "\nService refused by Blade ID %d @ %s. "
				"Agent in use with other CLI\n",
			 	info.blade_id,
				info.agent_ip_addr);
		return -1;
	}
	return 0;
}

#endif

void
teja_close_connection(void)
{
	teja_close_socket(info.cli_app_socket);
}

#ifdef TEJA_MAIN

int
main(int argc, char **argv)
{
	teja_uint32_t ret_code;

	info.top_menu_selection = 'q';
	info.init_done = 1;
	info.blade_id = 0;
	info.cli_app_socket = TEJA_INVALID_SOCKET;

	if ((ret_code = extract_args(argc, argv)) != 0) {
		return 0;
	}
	else if ((ret_code = teja_connect_to_agent()) != 0) {
		return 0;
	} else {
		do {
			ret_code = top_menu();

			if (ret_code != 0) {
				if (ret_code == USER_REQUEST_TO_QUIT) {
					teja_close_socket(info.cli_app_socket);
					break;
				} else {
					printf("cli_app: returning due to error\n");
				}
			} else {
				ret_code = wait_for_response();
			}
		} while (ret_code == 0);
	}
	return 0;
}

#endif
