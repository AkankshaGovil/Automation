#ifndef _ifs_h_
#define _ifs_h_

#include "unpifi.h"

#define IF_DELIM_NORMAL       ':'
#define IF_DELIM_NORMAL_STR   ":"
#define IF_DELIM_VLAN         '.'
#define IF_DELIM_VLAN_STR     "."

#define VLANID_MAX_LEN        4
#define VLANID_VALID_MIN      0
#define VLANID_VALID_MIN_STR  "0"
#define VLANID_VALID_MAX      4094
#define VLANID_VALID_MAX_STR  "4094"
//#define VLANID_RESERVED       4095
#define VLANID_NONE           4096    /* VLAN not configured on this realm */
#define VLANID_INVALID        4097    /* For INVALID input checks */
#define IsVLANIdValid(x)      (((x) >= VLANID_VALID_MIN) && ((x) <= VLANID_VALID_MAX))

#define RTG_TBL_MIN 1
#define RTG_TBL_MAX 252
#define IsRtgTblIdValid(x)      (((x) >= RTG_TBL_MIN) && ((x) <= RTG_TBL_MAX))

extern struct ifi_info *ifihead;

struct ifi_info *initIfs();

/* call this to get the aliased interface also */
struct ifi_info *initIfs2();

struct ifi_info * 
	findIf(struct ifi_info *ifihead, unsigned long ipaddr);

struct ifi_info *
matchIf(struct ifi_info *ifihead, unsigned long ipaddr);

struct ifi_info *
matchIfName(struct ifi_info *ifihead, char *);

struct ifi_info *
findIfByIfname( struct ifi_info *ifihead,
                char * ifname );

char *
GetNextIfname(	struct ifi_info *ifihead, 
				char * ifname );

void
freeIfs(struct ifi_info *ifi_head);

int
checkMask(unsigned long mask);

int
checkIfName(struct ifi_info  *ifi_head, char *ifname);

int
PlumbIf(char 			*pifname, 
		unsigned long 	ip, 
		unsigned long 	nm, 
		char 			*lifname,
		unsigned short 	vlanid,
		unsigned short 	rtgTblId);

int UnplumbIf(char *lifname, unsigned long ip, unsigned short rtgTblId);

int EditGatewayRoute(char *ifname,
                     unsigned short vlanid,
                     unsigned short rtgTblId,
                     unsigned long newGateway,
                     unsigned long oldGateway);
int 
StatusChgIf(char *lifname, unsigned short new_status);

unsigned short GetIfFlags(char *ifname);

unsigned long GetIfIpAddr(char *ifname);

extern long unsigned int GetIfIpMask (char *ifname);

extern long unsigned int getLocalIf (struct ifi_info *ifihead);

extern int IsPrimaryInterface(char *lifname, unsigned long ip);

#endif /* _ifs_h_ */
