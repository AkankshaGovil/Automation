#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <net/if.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include "nxosd.h"
#include "nxioctl.h"
#include <fcntl.h>
#include <ctype.h>

#include <errno.h>

#ifdef NETOID_LINUX
#include <linux/sockios.h>
#include <linux/netlink.h>
#include <linux/rtnetlink.h>
#include <linux/if_vlan.h>
#endif

#include "ifs.h"
#ifndef LIFNAMSIZ 
#define LIFNAMSIZ IFNAMSIZ
#define lifreq ifreq
#define lifr_name ifr_name
#define lifr_addr ifr_addr


#endif //LIFNAMSIZ

#ifndef SIOCGLIFNETMASK
#define SIOCGLIFNETMASK SIOCGIFNETMASK
#endif //SIOCGLILFNETMASK

#define CONFIGMSG_LEN 128

#ifdef NETOID_LINUX
static int WaitTillIfUp(int skfd, char *ifName, int timeout /*in ms*/);
static int WaitTillIfGone(int skfd, char *ifName, int timeout /*in ms*/);
static int AddVLAN(int skfd, char *pifname, int vlanid);
static int DeleteVLAN(int skfd, char *vifname);
static int readnl(int sock, char *buf, int seq, pid_t pid);
#define NL_BUFSIZE      8192
#endif

#ifndef SIOCLIFADDIF
#include <stdlib.h> // get index specific code uses atoi
#define MAX_LOGICAL_INF		255
unsigned char searcher[MAX_LOGICAL_INF+1];
// Returns first unused logical index.
int get_ifindex(char *parent, int *nExist)
{
    int i = 0;
    char *lindex;
	struct ifi_info *iflist;
	iflist = get_ifi_info2(AF_INET, 1);
        memset(searcher, 0, sizeof(searcher));
	int non_numeric = 0, numeric = 0;
	*nExist = -1; // -1 indicates the interface itself does not exist
	while (iflist){
		if (strncmp(iflist->ifi_name, parent, strlen(parent))!=0){
			iflist = iflist->ifi_next;
			continue; //skip;
		}
		if (*nExist == -1) *nExist = 0;
		lindex = strchr(iflist->ifi_name, IF_DELIM_NORMAL);
		if (!lindex || !*lindex){
			iflist = iflist->ifi_next;
			non_numeric++;
			continue;
		}

		{
		  // if ifi_name is VLAN If and parent is not or vice versa, go back
		  if ((strchr(iflist->ifi_name, IF_DELIM_VLAN) &&
		      !strchr(parent, IF_DELIM_VLAN)) ||
		      (!strchr(iflist->ifi_name, IF_DELIM_VLAN) &&
		      strchr(parent, IF_DELIM_VLAN))) {
			iflist = iflist->ifi_next;
			continue;
		  }
		}

		lindex++;
		//Assumption: *parent contains the physical device name, the part of an interface name after a colon is always an integer
		i= atoi(lindex);
		if (i>-1 && i <MAX_LOGICAL_INF){ 
		  searcher[i] = 1;
		  numeric++;
		}else
		  non_numeric++;
		iflist = iflist->ifi_next;
	}
	if ( (numeric + non_numeric) < MAX_LOGICAL_INF-1){ 
	  for (i = 0; i < MAX_LOGICAL_INF-1; i++){
	    if (searcher[i] == 0)
	      break;
	  }
	}else
	  return -1; /* FAILURE!!! out of ip addresses */

    if (*nExist != -1)
      *nExist = numeric;

    return i;
}

#endif //SIOCLIFADDIF


static int addif(	char *pifname, 
					unsigned long ip, 
					unsigned long nm,
					char *lifname,
					unsigned short vlanid,
					unsigned short rtgTblId);

static int deleteif(char *name,
					unsigned long ip,
					unsigned short rtgTblId);

static int setifflags(	char *name, 
						short flag);

static int set_ip_using(int 		s, 
						const char 	*name, 
						int 		c, 
						unsigned long ip);

static int get_flag(int	  s,
					char *ifname, 
					short *flag);

static int set_flag(int	  s,
					char *ifname, 
					short flag);

static int clr_flag(int	  s,
					char *ifname, 
					short flag);

static int IF_ULIPtostring (unsigned long ipaddress, char *ipstr);

static int MakeExecdCall(char *cmd);

/*
 * Interface APIs
 */

//#define IsLif(x)       	(strchr((x), IF_DELIM_VLAN) || strchr((x), IF_DELIM_NORMAL))
#define IsLif(x)       	(strchr((x), IF_DELIM_NORMAL))

/*
 * PlumbIf
 * 		Creates and interface given host byte order ipaddres/netmask
 * Params:
 * pifname - name of physical interface on which to plumb the interface
 * ip - host byte ordered IP address of new interface
 * nm - host byte ordered network mask
 * Return value:
 *  Success - name of newly plumbed interface in lifname and 0 
 *  Failure -  -1
 */
int
PlumbIf(char 			*pifname, 
		unsigned long 	ip, 
		unsigned long 	nm, 
		char 			*lifname,
		unsigned short 	vlanid,
		unsigned short 	rtgTblId)
{
	struct ifi_info  *ifi_head2=NULL, *ifp=NULL;
	struct ifi_info  *ifp_PHY = NULL;
	int		rc = 0;

    if (ip && strlen(pifname)) 
	{
		ifi_head2 = initIfs2(1);
		if ((ifp = matchIf(ifi_head2, htonl(ip))))
		{
			nx_strlcpy(lifname, ifp->ifi_name, IFI_NAME);
		}
		else
		{
			ifp_PHY = findIfByIfname(ifi_head2, pifname);
			if (ifp_PHY == NULL)
			{
				freeIfs(ifi_head2);
				return rc;
			}
			if ((rc = addif(pifname, ip, nm, lifname, vlanid, rtgTblId))< 0)
			{
				freeIfs(ifi_head2);
				return rc;
			}
		}
		freeIfs(ifi_head2);
    }

    return rc;
}


/*
 * UnplumbIf
 * Unplumbs a given interface if its a logical interface
 * Param
 *  lifname : name of logical interface to be removed
 * Return value
 *  0  - sucess
 *  -1 - failure
 */
int
UnplumbIf(char *lifname, unsigned long ip, unsigned short rtgTblId)
{
	struct ifi_info *ifp;
	int 	rc = 0;
	struct ifi_info  *ifi_head2=NULL;

	if (strlen(lifname) && IsLif(lifname))
	{
		ifi_head2 = initIfs2(1);
		ifp = findIfByIfname(ifi_head2, lifname);
		if (ifp != NULL)
		{
        		rc = deleteif(lifname, htonl(ifp->ifi_addr->sin_addr.s_addr), rtgTblId);
		} else if (ip) // when the i/f was deleted but clear ip rules etc
		{
        		rc = deleteif(lifname, ip, rtgTblId);
		}
		freeIfs(ifi_head2);
	}
	return rc;
}

int EditGatewayRoute(char *ifname,
                     unsigned short vlanid,
                     unsigned short rtgTblId,
                     unsigned long newGateway,
                     unsigned long oldGateway)
{
    char configMsg[CONFIGMSG_LEN];
    char ipstr[20] = "";

    if (newGateway)
    {
      IF_ULIPtostring(newGateway, ipstr);
      sprintf(configMsg, "ip route change default via %s table %d dev %s.%d",
               ipstr, rtgTblId, ifname, vlanid);
    } else
    {
      sprintf(configMsg, "ip route change default table %d dev %s.%d",
               rtgTblId, ifname, vlanid);
    }

    MakeExecdCall(configMsg);

    sprintf(configMsg, "ip route flush cache");
    MakeExecdCall(configMsg);

    return 0;
}

/*
 * StatusChgIf 
 * Sets logical interface to up/down 
 * Params:
 *  lifname    - name of logical interface in transition
 *  new_status - 0/1 for down/up respectively
 * Return value:
 *  new status of the interface 
 */
int 
StatusChgIf(char *lifname, unsigned short new_status)
{
    unsigned short curr_status=0, flags=0, newflags=-1;

    /* allow only UP for physical interface */
    if ( strlen(lifname) && 
		( IsLif(lifname) || (!IsLif(lifname) && new_status) ))
    {
        flags = GetIfFlags(lifname);
        curr_status = (flags & IFF_UP) ?  1 : 0;
        if (curr_status != new_status)
        {
			setifflags(lifname, new_status); 
            flags = GetIfFlags(lifname);
            newflags = (flags & IFF_UP) ?  1 : 0;
        }
    }
    return newflags;
}

/*
 * GetIfFlags
 * Get status flags set on interface
 * Params:
 * ifname - interface name
 * Return value:
 *  Failure - 0 
 *  Success - non-zero status flags
 */
unsigned short
GetIfFlags(char *ifname)
{
	int						sockfd;
	struct ifreq			ifr;
    
	if (( sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0 )
	{
		return 0;
	}

    memcpy(ifr.ifr_name, ifname, IFNAMSIZ);
    if ( ioctl(sockfd, SIOCGIFFLAGS, &ifr) < 0 )
    {
        return 0;
    }

    close(sockfd);
    return(ifr.ifr_flags);
}

unsigned long
GetIfIpAddr(char *ifname)
{
	int						sockfd;
	struct ifreq			ifr;
    struct sockaddr_in      ip;
    
	if (( sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0 )
	{
		return 0;
	}

    memcpy(ifr.ifr_name, ifname, IFNAMSIZ);
    if ( ioctl(sockfd, SIOCGIFADDR, &ifr) < 0 )
    {
        return 0;
    }

    if (ifr.ifr_addr.sa_family == AF_INET){
        memset(&ip, 0, sizeof(struct sockaddr_in));
        memcpy(&ip, &ifr.ifr_addr, sizeof(struct sockaddr_in));
    }

    close(sockfd);
    return(ntohl(ip.sin_addr.s_addr));
}

/* 
 * returns 0 on failure. Unlike rest of our functions
 */

unsigned long
GetIfIpMask(char *ifname)
{
  int  sockfd;
  struct lifreq  lifr;
  struct sockaddr_in	ip;

  sockfd = socket(AF_INET, SOCK_DGRAM, 0);

  memset (&lifr, 0, sizeof(lifr));
  memcpy(lifr.lifr_name, ifname, LIFNAMSIZ);

  if (ioctl(sockfd, SIOCGLIFNETMASK, &lifr) < 0)
  {
	  return 0;
  }

  memcpy(&ip, &lifr.lifr_addr, sizeof(struct sockaddr_in));

  close(sockfd);
  return (ntohl(ip.sin_addr.s_addr));
}

static int
addif(	char *pifname, 
		unsigned long ip, 
		unsigned long nm,
		char *lifname,
		unsigned short vlanid,
		unsigned short rtgTblId)
{
  struct lifreq  lifr;
//  struct sockaddr_in sin; Variable not used.
  unsigned long bc;
  int skfd;
  char configMsg[CONFIGMSG_LEN];
  int isVLANIf = 0;
  int isNewVLANIf = 0;
  char vlanIfName[IFNAMSIZ + 1];

  skfd = socket(AF_INET, SOCK_DGRAM, 0);
  if (skfd < 0)
  {
		fprintf(stderr, "socket() failed: %s\n", strerror(errno));
		return -1;
  }

  bc = nm ? (ip | ~nm) : 0;

  memset (&lifr, 0, sizeof(lifr));
  nx_strlcpy(lifr.lifr_name, pifname, IFNAMSIZ);
#ifdef SIOCLIFADDIF
  if (ioctl(skfd, SIOCLIFADDIF, &lifr) < 0)
  {
	  close(skfd);
	  return -1;
  }
#else // then use the function defined to get the next logical number
  int lindex;
  int nExist = 0;
  short flags = 0;

#ifdef NETOID_LINUX
  if (IsVLANIdValid(vlanid))
  {
    isVLANIf = 1;
    strcat(lifr.lifr_name, IF_DELIM_VLAN_STR);
    sprintf(lifr.lifr_name, "%s%d", lifr.lifr_name, vlanid);
    strcpy(vlanIfName, lifr.lifr_name);

    if ((lindex = get_ifindex(vlanIfName, &nExist))<0){
	  close(skfd);
	  return -1;
    }

    if ((nExist < 0) ||
        ((nExist == 0) && !(get_flag(skfd, pifname, &flags) & IFF_UP)))
    {
      if (if_nametoindex(vlanIfName) > 0)
      {
        if (DeleteVLAN(skfd, vlanIfName) < 0)
        {
          sprintf(configMsg, "/usr/sbin/vconfig rem %s", vlanIfName);
          MakeExecdCall(configMsg);
        }
        WaitTillIfGone(skfd, vlanIfName, 1000);
      }

      if (AddVLAN(skfd, pifname, vlanid) < 0)
      {
        sprintf(configMsg, "/usr/sbin/vconfig add %s %d", pifname, vlanid);
        MakeExecdCall(configMsg);
      }

      isNewVLANIf = 1;
    }

    if (WaitTillIfUp(skfd, vlanIfName, 2000) < 0)
    {
      close(skfd);
      return -1;
    }
  }
  else 
#endif
  {
    if ((lindex = get_ifindex(pifname, &nExist))<0){
	  close(skfd);
	  return -1;
    }
  }

  char ifindex[IFNAMSIZ] ;

  if (sprintf(ifindex,"%d", lindex) + strlen(pifname) < IFNAMSIZ-1){
  	strcat(lifr.lifr_name, IF_DELIM_NORMAL_STR);
  	strcat(lifr.lifr_name, ifindex);
  } else{
  	close(skfd);
  	return -1;
  }


#endif // SIOCLIFADDIF

  if (set_ip_using(skfd, lifr.lifr_name, SIOCSIFADDR, htonl(ip))==-1)
	  goto err;

  if (nm)
  {
    if(set_ip_using(skfd, lifr.lifr_name, SIOCSIFNETMASK, htonl(nm))==-1)
	  goto err;

    if(set_ip_using(skfd, lifr.lifr_name, SIOCSIFBRDADDR, htonl(bc))==-1)
	  goto err;
  }

  if (isVLANIf)
  {
    char VIF_IP[16] = "";
    char ipstr[20] = "";

    IF_ULIPtostring (ip, ipstr);
    strcpy(VIF_IP, ipstr);

    if (IsRtgTblIdValid(rtgTblId))
    {
      sprintf(configMsg, "ip rule add from %s table %d", VIF_IP, rtgTblId);
      MakeExecdCall(configMsg);

      if (isNewVLANIf)
      {
        //sprintf(configMsg, "ip route add src %s dev %s table %d", VIF_IP, vlanIfName, rtgTblId);
        sprintf(configMsg, "ip route add dev %s table %d", vlanIfName, rtgTblId);
        MakeExecdCall(configMsg);
      } else
      {
        //sprintf(configMsg, "ip route append src %s dev %s table %d", VIF_IP, vlanIfName, rtgTblId);
        //MakeExecdCall(configMsg);
      }

      sprintf(configMsg, "ip route flush cache");
      MakeExecdCall(configMsg);
    }
  }

  if (lifname)
  {
	  nx_strlcpy(lifname, lifr.lifr_name, IFNAMSIZ);
  }
	
  close(skfd);
  return 0;

err:
  deleteif(lifr.lifr_name, ip, rtgTblId);
  close(skfd);
  return -1;
}

static int
deleteif(char *name,
		unsigned long ip,
		unsigned short rtgTblId)
{
  struct lifreq  lifr;
  int skfd;
  char configMsg[CONFIGMSG_LEN];
  char tempVIFName[IFNAMSIZ + 1]; //copy of name that will be modified as needed
  int nExist = 0;
  int lindex;

  skfd = socket(AF_INET, SOCK_DGRAM, 0);
  if (skfd < 0)
  {
	fprintf(stderr, "socket() failed: %s\n", strerror(errno));
	return -1;
  }

  memset (&lifr, 0, sizeof(lifr));
  memcpy(lifr.lifr_name, name, IFNAMSIZ);
  
#ifdef SIOCLIFREMOVEIF  //ioctl to remove interface exists
  if (ioctl(skfd, SIOCLIFREMOVEIF, &lifr) < 0)
  {
	close(skfd);
	return -1;
  }
#else //if not then achieve that by marking the logical interface down

  char *vlanid_p = NULL;
  char VIF_IP[16] = "";
  char ipstr[20] = "";

#ifdef NETOID_LINUX
  IF_ULIPtostring(ip, ipstr);
  strcpy(VIF_IP, ipstr);
  strcpy(tempVIFName, name);

  if ((vlanid_p = strrchr(tempVIFName, IF_DELIM_VLAN)) != NULL)
  {
    int vlanid;
    char *delim_p = NULL;

    if ((delim_p = strrchr(vlanid_p+1, IF_DELIM_NORMAL)) != NULL)
    {
      *delim_p = '\0';
    }

    // Get rid of the ip routing rules
    vlanid = atoi(vlanid_p + 1);

    if (IsRtgTblIdValid(rtgTblId))
    {
      //sprintf(configMsg, "ip route del src %s dev %s table %d", VIF_IP, tempVIFName, rtgTblId);
      //sprintf(configMsg, "ip route del src %s dev %s table %d", VIF_IP, tempVIFName, rtgTblId);
      //MakeExecdCall(configMsg);

      sprintf(configMsg, "ip rule del from %s table %d", VIF_IP, rtgTblId);
      MakeExecdCall(configMsg);

      sprintf(configMsg, "ip route flush cache");
      MakeExecdCall(configMsg);
    }
  }
#endif

  // Doing it here for linux because deleting primary will delete secondaries
  // So, we won't delete the PHY.VLAN if more than 1 VIF exist
  if (vlanid_p != NULL)
  {
    get_ifindex(tempVIFName, &nExist);
  }

// Assumption: lifr contains the logical interface name
  lifr.ifr_flags = 0;
  if (ioctl(skfd, SIOCSIFFLAGS, &lifr) < 0)
  {
  	if (vlanid_p == NULL) // For linux, we need to wait to delete PHY.VLAN
  	{
  	  close(skfd);
  	  return -1;
  	}
  }

  if ((vlanid_p != NULL) && (nExist <= 1))
  {
    nExist = 0;
    // Get rid of this PHY.VLAN if this was last VIF
    if ((lindex = get_ifindex(tempVIFName, &nExist)) >= 0)
    {
      if (nExist <= 0)
      {
        DeleteVLAN(skfd, tempVIFName);
        WaitTillIfGone(skfd, tempVIFName, 1000);
      }
    }
  }

#endif //SIOCLIFREMOVEIF

  close(skfd);

  return 0;
}

static int
setifflags(char *name, short flag)
{
  int skfd;
  short ifflag = IFF_UP;

  skfd = socket(AF_INET, SOCK_DGRAM, 0);
  if (skfd < 0)
  {
		fprintf(stderr, "socket() failed: %s\n", strerror(errno));
		return -1;
  }

  if (flag)
  {
	  set_flag(skfd, name, ifflag);
  }
  else
  {
	  clr_flag(skfd, name, ifflag);
  }

  close(skfd);
  return 0;
}

static int 
set_ip_using(int s, const char *name, int c, unsigned long ip)
{
    struct ifreq ifr;
    struct sockaddr_in sin;

    nx_strlcpy(ifr.ifr_name, name, IFNAMSIZ);
    memset(&sin, 0, sizeof(struct sockaddr));
    sin.sin_family = AF_INET;
    sin.sin_addr.s_addr = ip;
    memcpy(&ifr.ifr_addr, &sin, sizeof(struct sockaddr));

    if (ioctl(s, c, &ifr) < 0)
	{
		return -1;
	}
    return 0;
}

static int 
get_flag(int s, char *ifname, short *flag)
{
    struct ifreq ifr;

    nx_strlcpy(ifr.ifr_name, ifname, IFNAMSIZ);
    if (ioctl(s, SIOCGIFFLAGS, &ifr) < 0) 
	{
		fprintf(stderr, "%s: unknown interface: %s\n",
				ifname, strerror(errno));
		return (-1);
    }

    return ifr.ifr_flags;
}

static int 
set_flag(int s, char *ifname, short flag)
{
    struct ifreq ifr;

    nx_strlcpy(ifr.ifr_name, ifname, IFNAMSIZ);
    if (ioctl(s, SIOCGIFFLAGS, &ifr) < 0) 
	{
		fprintf(stderr, "%s: unknown interface: %s\n",
				ifname, strerror(errno));
		return (-1);
    }
    nx_strlcpy(ifr.ifr_name, ifname, IFNAMSIZ);
    ifr.ifr_flags |= flag;
    if (ioctl(s, SIOCSIFFLAGS, &ifr) < 0) 
	{
		perror("SIOCSIFFLAGS");
		return -1;
    }
    return (0);
}

/* Clear a certain interface flag. */
static int 
clr_flag(int s, char *ifname, short flag)
{
    struct ifreq ifr;
//    int fd;    //Varnot used

    nx_strlcpy(ifr.ifr_name, ifname, IFNAMSIZ);
    if (ioctl(s, SIOCGIFFLAGS, &ifr) < 0) 
	{
		fprintf(stderr, "%s: unknown interface: %s\n",
			ifname, strerror(errno));
		return -1;
    }
    nx_strlcpy(ifr.ifr_name, ifname, IFNAMSIZ);
    ifr.ifr_flags &= ~flag;
    if (ioctl(s, SIOCSIFFLAGS, &ifr) < 0) 
	{
		perror("SIOCSIFFLAGS");
		return -1;
    }
    return (0);
}

//wrapper functions for unit tests
int test_addif(	char *pifname, 
					unsigned long ip, 
					unsigned long nm,
		char *lifname){
  unsigned short dummy = 0;
  return addif(pifname, ip, nm, lifname, VLANID_NONE, dummy); /* added dummy vlanid (NO VLAN) */
}
int test_deleteif(char *name){

  return deleteif(name, 0, 0);
}

static int
MakeExecdCall(char *cmd)
{
  // lets keep it as system() since CLI is not such a huge process
  system((const char *)cmd);
  return 0;
}

static int
IF_ULIPtostring (unsigned long ipaddress, char *ipstr)
{
  sprintf(ipstr, "%u.%u.%u.%u",
			(unsigned int)(ipaddress&0xff000000)>>24,
			(unsigned int)(ipaddress&0x00ff0000)>>16,
			(unsigned int)(ipaddress&0x0000ff00)>>8,
			(unsigned int)(ipaddress&0x000000ff));
  return 0;
}

#ifdef NETOID_LINUX
static int
WaitTillIfUp(int skfd, char *ifName, int timeout /*in ms*/)
{
      struct ifreq ifr;
      int waited = 0;
      struct timespec ts;

      ts.tv_sec = 0;
      ts.tv_nsec = 30000000; /* 30 millisecond. */

      memset (&ifr, 0, sizeof(ifr));
      nx_strlcpy(ifr.ifr_name, ifName, IFNAMSIZ);

      while (waited < timeout)
      {
        if (ioctl(skfd, SIOCGIFFLAGS, &ifr) < 0)
        {
          return -1;
        }

        if (ifr.ifr_flags & IFF_UP)
        {
  	    //printf("WaitTillIfUp() : waited %d ms\n", waited);
            nanosleep (&ts, NULL);  // Just trying
  	    return 0;
        }

        nanosleep (&ts, NULL);
        waited += 30;
      }

      return -1;
}

static int
WaitTillIfGone(int skfd, char *ifName, int timeout /*in ms*/)
{
      struct ifreq ifr;
      int waited = 0;
      struct timespec ts;

      ts.tv_sec = 0;
      ts.tv_nsec = 30000000; /* 30 millisecond. */

      memset (&ifr, 0, sizeof(ifr));
      nx_strlcpy(ifr.ifr_name, ifName, IFNAMSIZ);

      while (waited < timeout)
      {
        ifr.ifr_flags = 0;

        if (ioctl(skfd, SIOCGIFFLAGS, &ifr) < 0)
        {
          //printf("WaitTillIfGone() : waited %d ms\n", waited);
          nanosleep (&ts, NULL);  // Just trying
          return 0;
        }

        if (ifr.ifr_flags & IFF_UP)
        {
            nanosleep (&ts, NULL);
            waited += 30;
  	    continue;
        }

        break;
      }

      //printf("WaitTillIfGone() : waited %d ms\n", waited);
      nanosleep (&ts, NULL);  // Just trying
      return 0;
}

static int
AddVLAN(int skfd, char *pifname, int vlanid)
{
      struct vlan_ioctl_args if_request;

      memset(&if_request, 0, sizeof(struct vlan_ioctl_args));
      strcpy(if_request.device1, pifname);
      if_request.u.VID = vlanid;
      if_request.cmd = ADD_VLAN_CMD;

      if (ioctl(skfd, SIOCSIFVLAN, &if_request) < 0) {
        return -1;
      }

      return 0;
}

static int
DeleteVLAN(int skfd, char *vifname)
{
      struct vlan_ioctl_args if_request;

      memset(&if_request, 0, sizeof(struct vlan_ioctl_args));
      strcpy(if_request.device1, vifname);
      if_request.cmd = DEL_VLAN_CMD;

      if (ioctl(skfd, SIOCSIFVLAN, &if_request) < 0) {
        return -1;
      }

      return 0;
}

int IsPrimaryInterface(char *lifname, unsigned long ip)
{
	int 	sock = 0,  nread = 0, attrlen = 0;
	static int 	seq = 1;
	char	buf[NL_BUFSIZE];
	struct nlmsghdr	*nlhdrp = NULL;
	struct ifaddrmsg *ifaddrp = NULL;
	struct rtattr	*attrp = NULL;
	char ipstr[20] = "";
	char ipIFA[20] = "";
	
	if (!lifname || !ip)
	{
	  return -1; // invalid info
	}

	IF_ULIPtostring (ip, ipstr);

	//make a udp netlink routing socket
	if( (sock = socket(AF_NETLINK, SOCK_DGRAM, NETLINK_ROUTE)) < 0)
		return -1;

	memset(buf, 0, NL_BUFSIZE);	//clear buffer
	
	//the netlink header (linux/netlink.h)
	nlhdrp = (struct nlmsghdr *)buf;
	ifaddrp = (struct ifaddrmsg *)NLMSG_DATA(nlhdrp);
	nlhdrp->nlmsg_len = NLMSG_LENGTH(sizeof(struct ifaddrmsg));
	nlhdrp->nlmsg_type = RTM_GETADDR;
	//nlhdrp->nlmsg_flags = NLM_F_DUMP | NLM_F_REQUEST;
	nlhdrp->nlmsg_flags = NLM_F_MATCH | NLM_F_REQUEST;
	nlhdrp->nlmsg_seq = seq;
	nlhdrp->nlmsg_pid = getpid();

	ifaddrp->ifa_family = AF_INET;
	ifaddrp->ifa_prefixlen = 0;
	ifaddrp->ifa_flags = IFA_F_SECONDARY;
	ifaddrp->ifa_scope = RT_SCOPE_LINK;
	ifaddrp->ifa_index = if_nametoindex(lifname);
	//printf("looking for i/f --> index=%d, name=%s\n", ifaddrp->ifa_index, lifname);
	
	//send the netlink command
	if(write(sock, nlhdrp, nlhdrp->nlmsg_len) < 0)
		return -1;
	
	//read the reply
	if((nread = readnl(sock, buf, seq, getpid())) == -1)
		return -1;

	seq++;

	//loop thru data buffer returned and print out info 
	for( ;  NLMSG_OK(nlhdrp, nread); nlhdrp = NLMSG_NEXT(nlhdrp, nread)){
		//print out data from the ifaddrmsg structure
		ifaddrp = (struct ifaddrmsg *)NLMSG_DATA(nlhdrp);

		//get a pointer to attribute buffer and calculate it's length
		attrp = (struct rtattr *)IFA_RTA(ifaddrp);
		attrlen = IFA_PAYLOAD(nlhdrp);
		
		//now move on to the attribute buffer
		for( ; RTA_OK(attrp, attrlen); attrp = RTA_NEXT(attrp, attrlen) ){
			
			//process the data based on the type
			switch(attrp->rta_type){
				case IFA_ADDRESS:	//ascii string with inteface name
					{
					unsigned char *data = (char *)RTA_DATA(attrp);
					sprintf(ipIFA, "%d.%d.%d.%d", data[0], data[1], data[2], data[3]);
					//printf("--%s--", ipIFA);
					//printf("--%s--\n", (ifaddrp->ifa_flags & IFA_F_SECONDARY) ? "I am secondary" : "I am primary");
					if (!strcmp(ipstr, ipIFA))
					{
					  return ((ifaddrp->ifa_flags & IFA_F_SECONDARY) ? 0 : 1);
					}
					}
					break;
			}
		}
	}
	
	return -1; // IP not found
}

static int
readnl(int sock, char *buf, int seq, pid_t pid)
{
	int	nread = 0, ntread = 0;
	u_char	flag = 0;
	char	*tmp = buf;
	struct nlmsghdr	*nlhdrp = NULL;
	
	//read the reply, check for multi-part reply
	//fill up buffer to NL_BUFSIZE bytes at most
	do{ 
	    	//we can't read more than NL_BUFSIZE bytes	
		if( (nread = read(sock, tmp, NL_BUFSIZE - ntread )) < 0)
			return -1;
		
		nlhdrp = (struct nlmsghdr *)tmp;
		
        	//FIRST ALWAYS: check to make sure the message structure is ok
        	//then check to make sure type isn't error type
		if( (NLMSG_OK(nlhdrp, nread) == 0) || (nlhdrp->nlmsg_type == NLMSG_ERROR) )
               		return -1;
		
		//if this is the last message then it contains no data
		//so we dont want to return it
		if(nlhdrp->nlmsg_type == NLMSG_DONE)
			flag = 1;
		else{
			tmp += nread;
			ntread += nread;
		}
		
		//in this case we never had a multipart message 
		if( (nlhdrp->nlmsg_flags & NLM_F_MULTI) == 0 )	
			flag = 1;
		
	}while(nlhdrp->nlmsg_seq != seq || nlhdrp->nlmsg_pid != pid || flag == 0);
                                
	return ntread;
}

#endif
