#ifndef _ispd_common_h_
#define _ispd_common_h_

/*
 * This header defines ispd-related structures
 * used in parsing the server.cfg. The code
 * utilizing these structures is in the sconfig
 * directory.
 */

#define MAX_ISPD_PEERS		8

#define MAX_ISPD_IFNAMELEN      128
#define MAX_ISPD_ROUTERLEN      128
#define MAX_ISPD_IPLEN          128

typedef struct _ispd_interface
{

	char	name[ MAX_ISPD_IFNAMELEN ];			// interface device name - "hme0", "znb0"
	char	router[ MAX_ISPD_ROUTERLEN ];			// name or ip address of router
	char	ip[ MAX_ISPD_IPLEN ];				// real ip address used on interface
	char	vip[ MAX_ISPD_IPLEN ];				// virtual ip address used on interface
	int		defined;				// if non-zero the interface is defined.
} ispd_interface_t;

typedef struct _ispd_ctl_interface
{
	char	name[ MAX_ISPD_IFNAMELEN ];			// Name of control interface
	char	ip[ MAX_ISPD_IPLEN ];				// real ip address used on interface
	char	peer_iservers[MAX_ISPD_PEERS][MAX_ISPD_IPLEN];
									// Array of Name or ip addresses of other
									//  iServers on control subnet.
	int		peer_count;				// Count of peer iServers
	int		defined;				// if non-zero the interface is defined.
} ispd_ctl_interface_t;

typedef enum _ispd_server_type
{
	ISPD_TYPE_STANDBY 		= 1,
	ISPD_TYPE_ACTIVE  		= 2,
	ISPD_TYPE_DISABLED  	= 3,
} ispd_server_type_t;

#define ISERVER_STATE_SHM_KEY	2

extern int *p_iserver_state;

#endif /* _ispd_common_h_ */
