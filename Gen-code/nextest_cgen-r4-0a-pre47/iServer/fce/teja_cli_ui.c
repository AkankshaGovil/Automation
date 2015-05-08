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

extern cli_info_t info;

#ifdef CLI_APP
extern char *init_eth_commands[];
extern char *init_pos_commands[];
#endif /* */

char user_input_str[USER_INPUT_BUFFER_SIZE + 1];

int
get_char(char *next_char)
{
	int new_char;

	#ifdef CLI_APP
		if (!info.init_done) {
			while (1) {
				if (info.init_cmd_index2 == 0) {

					#if (L2_TYPE == L2_POS)
						printf("\n%s",
								init_pos_commands[info.init_cmd_index1]);
					#else /* */
						printf("\n%s",
								init_eth_commands[info.init_cmd_index1]);

					#endif /* */
				}

				#if (L2_TYPE == L2_POS)
					*next_char =
						init_pos_commands[info.init_cmd_index1][info.init_cmd_index2];
				#else /* */
					*next_char =
						init_eth_commands[info.init_cmd_index1][info.init_cmd_index2];
				#endif /* */

				if (*next_char == '\0') {
					if (info.init_cmd_index2 == 0) {
						info.init_done = 1;
						top_menu_content();
						break;
					} else {
						info.init_cmd_index1 = info.init_cmd_index1 + 1;
						info.init_cmd_index2 = 0;
						*next_char = '\n';
						break;
					}
				} else {
					info.init_cmd_index2 = info.init_cmd_index2 + 1;
					break;
				}
			}
		}

		if (info.init_done) {
			new_char = getchar();

			if (new_char == EOF) {
				*next_char = 0xFF;
			} else {
				*next_char = (char) new_char;
			}
		}

	#else /* */
		{
			int n;

			if (info.cli_app_socket == TEJA_INVALID_SOCKET) {
				printf("\nget_char(): CLI not connected yet!");
			} else {
				if ((n = teja_read_from_socket(	info.cli_app_socket,
												next_char,
												1,
												0)) < 0) {
					printf(	"\nget_char(): teja_read_from_socket "
							"returned error");
					return -1;
				} else if (n == 1) {
					// printf("\nget_char () got %c", *next_char); 
				} else {
					return -1;
				}
			}
		}
	#endif /* */

	return 0;
}

teja_uint32_t
get_user_input(void)
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
					if ((ret_code = get_char(&user_input_str[index])) != 0) {
						break;
					}

					if ((strrchr(" \t\n", user_input_str[index]) == NULL)) {
						break;
					}

					if (user_input_str[index] == '\n') {
						info.line_num = info.line_num + 1;
					}
				}
			} else if ((ret_code = get_char(&user_input_str[index])) != 0) {
				break;
			}

			// char in user_input_str set to 0xFF when EOF encountered 

			if (((unsigned char) user_input_str[index] == 0xFF) ||
				(strrchr(" \t\n", user_input_str[index]) != NULL)) {

				// end of line or field (space/tab) or file 

				if (user_input_str[index] == '\n') {
					info.line_num = info.line_num + 1;
				}
				break;
			}
			index = index + 1;
		}

		// char in user_input_str set to 0xFF when EOF encountered 

		if ((strrchr(" \t\n", user_input_str[index]) == NULL) &&
			((unsigned char) user_input_str[index] != 0xFF)) {
			printf("\nInput string too long!!\n");
			ret_code = ERR_INPUT_STRING_TOO_LONG;
		} else if ((unsigned char) user_input_str[0] == 0xFF) {
			printf("\nReached Unexpected End Of File!!\n");
			ret_code = INTERNAL;
		} else if (user_input_str[0] == '#') {

			// it is a comment. skip the entire line 

			// char in user_input_str set to 0xFF when EOF encountered 

			while ((user_input_str[index] != '\n') &&
				   ((unsigned char) user_input_str[index] != 0xFF)) {

				if ((ret_code = get_char(&user_input_str[index])) != 0) {
					break;
				}

				if (user_input_str[index] == '\n') {
					info.line_num++;
				}
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
get_user_selection(char *p_user_selection, char *p_choices)
{
	teja_uint32_t ret_code;
	int first_time;
	int scan_code;

	ret_code = 0;
	scan_code = 0;
	first_time = 1;
	while (1) {

		if (!first_time) {
			printf("Invalid Selection. Try Again...\n ");
			printf("\nChoice: ");
		}

		ret_code = get_user_input();

		if (ret_code != 0) {
			break;
		}

		scan_code = sscanf(user_input_str, "%c", p_user_selection);

		if ( (scan_code == 1) && 
			 (strrchr(p_choices, *p_user_selection) != NULL)) {
			break;
		}

		first_time = 0;
	}
	return ret_code;
}

teja_uint32_t
get_num(teja_uint32_t * p_num, char *str)
{
	teja_uint32_t ret_code;
	int first_time;
	int scan_code;

	ret_code = 0;
	first_time = 1;
	while (1) {
		if (!first_time) {
			printf("Error. Try Again...\n ");
		}

		#ifdef CLI_APP
			if (info.init_done) {
				printf("\n%s ", str);
			}
		#endif /* #ifdef CLI_APP */

		ret_code = get_user_input();

		if (ret_code != 0) {
			return ret_code;
		}

		scan_code = sscanf(user_input_str, "%d", p_num);

		if (scan_code == 1) {
			break;
		}

		scan_code = sscanf(user_input_str, "0x%x", p_num);

		if (scan_code == 1) {
			break;
		}

		first_time = 0;
	}
	return ret_code;
}

teja_uint32_t
get_char_w_display(char *p_char, char *str)
{
	teja_uint32_t ret_code;
	int first_time;
	int scan_code;

	ret_code = 0;
	first_time = 1;
	while (1) {
		if (!first_time) {
			printf("Error. Try Again...\n ");
		}

		#ifdef CLI_APP
			if (info.init_done) {
				printf("\n%s ", str);
			}
		#endif /* #ifdef CLI_APP */

		ret_code = get_user_input();

		if (ret_code != 0) {
			return ret_code;
		}

		scan_code = sscanf(user_input_str, "%c", p_char);

		if (scan_code == 1) {
			break;
		}

		first_time = 0;
	}
	return ret_code;
}

teja_uint32_t
get_dot_num(teja_uint32_t * p_num, char *str)
{
	teja_uint32_t ret_code;
	int first_time;
	int scan_code;
	unsigned short int num1;
	unsigned short int num2;
	unsigned short int num3;
	unsigned short int num4;

	ret_code = 0;
	first_time = 1;
	while (1) {
		if (!first_time) {
			printf("Error. Try Again...\n ");
		}

		#ifdef CLI_APP
			if (info.init_done) {
				printf("\n%s ", str);
			}
		#endif /* #ifdef CLI_APP */

		ret_code = get_user_input();

		if (ret_code != 0) {
			return ret_code;
		}

		scan_code =
			sscanf(	user_input_str,
					"%hd.%hd.%hd.%hd",
					&num1,
					&num2,
					&num3,
					&num4);

		if (scan_code == 4) {
			break;
		}

		first_time = 0;
	}

	*p_num = (teja_uint32_t) (num1 << 24 | num2 << 16 | num3 << 8 | num4);
	return ret_code;
}

teja_uint32_t
get_blade_id(teja_uint32_t * p_blade_id)
{
	return get_num(p_blade_id, "\nBlade ID: ");
}

teja_uint32_t
get_port_id(teja_uint32_t * p_port_id)
{
	return get_num(p_port_id, "\nPort ID: ");
}

teja_uint32_t
get_spec_port_id(teja_uint32_t * p_port_id, char *spec)
{
	char port_string[24];

	sprintf(port_string, "%s Port ID: ", spec);

	return get_num(p_port_id, port_string);
}

teja_uint32_t
get_spec_ip_addr(teja_uint32_t * p_ip_addr, char *spec)
{
	char ip_addr_string[48];

	sprintf(ip_addr_string, "%s IP Addr (192.168.1.1): ", spec);

	return get_dot_num(p_ip_addr, ip_addr_string);
}

teja_uint32_t
get_spec_l4_port(teja_uint32_t * p_l4_port, char *spec)
{
	char l4_port_string[32];

	sprintf(l4_port_string, "%s Layer 4 port: ", spec);

	return get_num(p_l4_port, l4_port_string);
}

teja_uint32_t
get_spec_l4_protocol(teja_uint32_t * p_l4_protocol)
{
	return get_num(p_l4_protocol, "Layer 4 protocol ( 6 = TCP, 17 = UDP ): ");
}

teja_uint32_t
get_ip_addr(teja_uint32_t * p_ip_addr)
{
	return get_dot_num(p_ip_addr, "\nIP Addr (192.168.1.1): ");
}

teja_uint32_t
get_ip_mask(teja_uint32_t * p_ip_mask)
{
	return get_dot_num(p_ip_mask, "\nIP Mask (255.255.0.0): ");
}

teja_uint32_t
get_l2_index(teja_uint32_t * p_l2_index)
{
	teja_uint32_t ret_code = 0;
	int done = 0;

	while (!done) {
		ret_code = get_num(p_l2_index, "\nL2 Index: ");

		if (ret_code == 0) {
			if ((*p_l2_index >= L2_INDEX_AUTO_ALLOC_FIRST) &&
				(*p_l2_index <= L2_INDEX_AUTO_ALLOC_LAST)) {
				printf(	"\nValue reserved for Local Routes. Please "
						"choose value outside (%d-%d) range...",
					 	L2_INDEX_AUTO_ALLOC_FIRST,
						L2_INDEX_AUTO_ALLOC_LAST);
			} else {
				done = 1;
			}
		} else
			break;
	}

	return ret_code;
}

teja_uint32_t
get_nh_id(teja_uint32_t * p_nh_id)
{
	teja_uint32_t ret_code = 0;
	int done = 0;

	while (!done) {
		ret_code = get_num(p_nh_id, "\nNextHop ID: ");
		if (ret_code == 0) {
			if ((*p_nh_id >= NH_ID_AUTO_ALLOC_FIRST) &&
				(*p_nh_id <= NH_ID_AUTO_ALLOC_LAST)) {
				printf(	"\nValue reserved for Local Routes. "
						"Please choose value outside (%d-%d) range...",
						NH_ID_AUTO_ALLOC_FIRST,
						NH_ID_AUTO_ALLOC_LAST);
			} else {
				done = 1;
			}
		} else
			break;
	}
	return ret_code;
}

teja_uint32_t
get_mtu(teja_uint32_t * p_mtu)
{
	return get_num(p_mtu, "\nMTU: ");
}

teja_uint32_t
get_vlan_id(teja_uint32_t * p_vlan_id )
{
    teja_uint32_t ret_code;

    ret_code = get_num(p_vlan_id, "\nVlan Id: ");
    if ( ret_code == 0 ) {
		if ( *p_vlan_id > 4095 )
			return -1;
	}

	return ret_code;
}


teja_uint32_t
get_sleep_time_to_set(teja_uint32_t * p_sleep_time)
{
	return get_num(p_sleep_time, "\nSleep Time (sec): ");
}

teja_uint32_t
get_pkts_to_drain_to_set(teja_uint32_t * p_pkts_to_drain)
{
	return get_num(p_pkts_to_drain, "\nPackets to Drain: ");
}

teja_uint32_t
get_flags(teja_uint32_t * p_flags)
{
	teja_uint32_t flags;
	teja_uint32_t ret_code = 0;
	int done = 0;

	while (!done) {
		ret_code =
			get_num(&flags,
					"\nFlags(FORWARD:0,LOCAL:1,DOWN:2,DROP:"
					"3,BROADCAST:4,MULTICAST:5): ");

		if (ret_code == 0) {
			switch (flags)
			{
			case 0:
				*p_flags = 0;
				done = 1;
				break;
			case 1:
				*p_flags = TEJACPDP_NEXT_HOP_FLAG_LOCAL;
				done = 1;
				break;
			case 2:
				*p_flags = TEJACPDP_NEXT_HOP_FLAG_DOWN;
				done = 1;
				break;
			case 3:
				*p_flags = TEJACPDP_NEXT_HOP_FLAG_DROP;
				done = 1;
				break;
			case 4:
				*p_flags = TEJACPDP_NEXT_HOP_FLAG_BROADCAST;
				break;
			case 5:
				*p_flags = TEJACPDP_NEXT_HOP_FLAG_MULTICAST;
				done = 1;
				break;
			default:
				printf("\nIncorrect value supplied. Try Again...");
			}
		} else
			break;
	}
	return ret_code;
}

teja_uint32_t
get_perm_flag(teja_uint32_t * p_flag)
{
	teja_uint32_t flag;
	teja_uint32_t ret_code = 0;
	int done = 0;

	while (!done) {
		ret_code = get_num(	&flag,
							"\nFlag (DYNAMIC:0,PERM:1): " );

		if (ret_code == 0) {
			switch (flag)
			{
			case 0:
				*p_flag = 0;
				done = 1;
				break;
			case 1:
				*p_flag = 1;
				done = 1;
				break;
			default:
				printf("\nIncorrect value supplied. Try Again...");
			}
		} else
			break;
	}

	return ret_code;
}

teja_uint32_t
get_port_state(TejaCPDP_if_state_t * p_port_state)
{
	teja_uint32_t state;
	teja_uint32_t ret_code = 0;
	int done = 0;

	while (!done) {
		ret_code = get_num(&state, "\nPort State (1:ENABLE/0:DISABLE): ");

		if (ret_code == 0) {
			switch (state)
			{
			case 0:
				*p_port_state = TEJA_IF_DISABLE;
				done = 1;
				break;
			case 1:
				*p_port_state = TEJA_IF_ENABLE;
				done = 1;
				break;
			default:
				printf("\nIncorrect value supplied. Try Again...");
			}
		} else
			break;
	}
	return ret_code;
}

teja_uint32_t
get_link_status(TejaCPDP_link_status_t * p_link_status)
{
	teja_uint32_t status;
	teja_uint32_t ret_code = 0;
	int done = 0;

	while (!done) {
		ret_code = get_num(&status, "\nLink Status (1:UP/0:DOWN): ");
		if (ret_code == 0) {
			switch (status) {
			case 0:
				*p_link_status = TEJA_IF_DOWN;
				done = 1;
				break;
			case 1:
				*p_link_status = TEJA_IF_UP;
				done = 1;
				break;
			default:
				printf("\nIncorrect value supplied. Try Again...");
			}
		} else
			break;
	}
	return ret_code;
}

teja_uint32_t
get_promiscuous_mode(TejaCPDP_if_state_t * p_promiscuous_mode)
{
	teja_uint32_t mode;
	teja_uint32_t ret_code = 0;
	int done = 0;

	while (!done) {
		ret_code = get_num(&mode, "\nPromiscuous Mode(1:ENABLE/0:DISABLE): ");

		if (ret_code == 0) {
			switch (mode)
			{
			case 0:
				*p_promiscuous_mode = TEJA_IF_DISABLE;
				done = 1;
				break;
			case 1:
				*p_promiscuous_mode = TEJA_IF_ENABLE;
				done = 1;
				break;
			default:
				printf("\nIncorrect value supplied. Try Again...");
			}
		} else
			break;
	}
	return ret_code;
}

teja_uint32_t
get_mac_addr(TejaCPDP_mac_addr_t * p_mac_addr)
{
	teja_uint32_t ret_code;
	int first_time;
	int scan_code;
	unsigned short i, j, k, l, m, n;

	ret_code = 0;

	// get mac_address 

	first_time = 1;

	while (1) {
		if (!first_time) {
			printf("Error. Try Again...\n ");
		}

		#ifdef CLI_APP
			if (info.init_done) {
				printf("\nEthernet MAC Addr (e.g. ab:12:cd:34:ef:56): ");
			}
		#endif /* #ifdef CLI_APP */

		ret_code = get_user_input();

		if (ret_code != 0) {
			return ret_code;
		}

		scan_code =
			sscanf(	user_input_str,
					"%hx:%hx:%hx:%hx:%hx:%hx",
					&i, &j, &k, &l, &m, &n);

		p_mac_addr->mac[0] = i & 0xFF;
		p_mac_addr->mac[1] = j & 0xFF;
		p_mac_addr->mac[2] = k & 0xFF;
		p_mac_addr->mac[3] = l & 0xFF;
		p_mac_addr->mac[4] = m & 0xFF;
		p_mac_addr->mac[5] = n & 0xFF;

		if (scan_code == 6) {
			break;
		}
		first_time = 0;
	}
	return ret_code;
}

teja_uint32_t
get_redirect_info(TejaCPDP_redirect_info_t * p_redirect_info)
{
	teja_uint32_t ret_code;

	ret_code = 0;

	if ((ret_code = get_spec_ip_addr(	&p_redirect_info->new_ip_saddr,
										"Egress src") != 0)) {
		printf("\nget_redirect_info: get_spec_ip_addr failed error %d",
		ret_code);
	} else if ((ret_code =
		  get_spec_l4_port(	&p_redirect_info->new_l4_sport,
							"Egress src") != 0)) {
		printf("\nget_redirect_info: get_spec_l4_port failed error %d",
				ret_code);
	}
	else if ((ret_code =
		  get_spec_ip_addr(	&p_redirect_info->new_ip_daddr,
							"Egress dst") != 0)) {
		printf(	"\nget_redirect_info: get_spec_ip_addr failed error %d",
				ret_code);
	} else if ((ret_code =
		  get_spec_l4_port(	&p_redirect_info->new_l4_dport,
							"Egress dst") != 0)) {
		printf(	"\nget_redirect_info: get_spec_l4_port failed error %d",
				ret_code);
	} else if ((ret_code =
		  get_spec_port_id(	&p_redirect_info->egress_port_id,
							"Egress l1") != 0)) {
		printf(	"\nget_redirect_info: get_spec_port_id failed error %d",
				ret_code);
	}
	return ret_code;
}

teja_uint32_t
get_redirect_key(TejaCPDP_redirect_key_t * p_redirect_key)
{
	teja_uint32_t ret_code;

	ret_code = 0;

	// get ip address from user 

	if ((ret_code = get_spec_port_id(	&p_redirect_key->port_id,
										"Ingress l1") != 0)) {
		printf(	"\nget_redirect_key: get_spec_port_id failed error %d",
				ret_code);
	} else if ((ret_code =
		  get_spec_ip_addr(	&p_redirect_key->ip_daddr,
							"Ingress dst") != 0)) {
		printf(	"\nget_redirect_key: get_spec_ip_addr failed error %d",
				ret_code);
	} else if ((ret_code =
		  get_spec_l4_port(	&p_redirect_key->l4_dport,
							"Ingress dst") != 0)) {
		printf(	"\nget_redirect_key: get_spec_l4_port failed error %d",
				ret_code);
	} else if ((ret_code = 
		get_spec_l4_protocol(	&p_redirect_key->l4_protocol) != 0)) {
		printf("\nget_redirect_key: get_spec_l4_protocol failed error %d",
				ret_code);
	}
	return ret_code;
}

teja_uint32_t
get_nexthop_info(TejaCPDP_next_hop_info_t * p_next_hop_info)
{
	teja_uint32_t ret_code;

	ret_code = 0;

	// get ip address from user 

	if ((ret_code = get_ip_addr(&p_next_hop_info->next_hop_ip_addr) != 0)) {
		printf("\nget_nexthop_info: get_ip_addr failed error %d", ret_code);
	} else if ((ret_code = get_blade_id(&p_next_hop_info->blade_id) != 0)) {
		printf("\nget_nexthop_info: get_blade_id failed error %d", ret_code);
	} else if ((ret_code = get_port_id(&p_next_hop_info->port_id) != 0)) {
		printf("\nget_nexthop_info: get_port_id failed error %d", ret_code);
	} else if ((ret_code = get_mtu(&p_next_hop_info->mtu) != 0)) {
		printf("\nget_nexthop_info: get_mtu failed error %d", ret_code);
	} else if ((ret_code = get_flags(&p_next_hop_info->flags) != 0)) {
		printf("\nget_nexthop_info: get_flags failed error %d", ret_code);
	} else if ((ret_code = get_l2_index(&p_next_hop_info->l2_index) != 0)) {
		printf("\nget_nexthop_info: get_l2_index failed error %d", ret_code);
	}

	return ret_code;
}

teja_uint32_t
get_side(char *p_side)
{
	teja_uint32_t ret_code = 0;
	int done = 0;

	while (!done) {
		ret_code = get_char_w_display(p_side, "\nWhich Side (I/E): ");

		if (ret_code == 0) {
			switch (tolower(*p_side)) {
			case 'i':
			case 'e':
				*p_side = tolower(*p_side);
				done = 1;
				break;
			default:
				printf("\nIncorrect value supplied. Try Again...");
				break;
			}
		} else
			break;
	}
	return ret_code;
}
