#include <stdio.h>
#include <stdlib.h>
#include <strings.h>
#include <errno.h>
#include <unistd.h>
#include <netdb.h>

#include <sys/types.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <arpa/inet.h>

#include "clist.h"
#include "srvrlog.h"
#include "poolalloc.h"
#include "xmlparse.h"
#include "lsconfig.h"
#include "execd.h"
#include "ifs.h"
#include "nxosd.h"
#include "fcemacro.h"	/* For IS_HKNIFE_FW */
#include "hknife.h"		/* hot knife (Teja) APIs */

#define POOLCFG_TAG      "PoolCfg"
#define POOL_TAG         "Pool"
#define PORTALLOC_TAG    "PortAlloc"
#define VNET_TAG         "Vnet"
#define NETIF_ROUTE_TAG  "Route"

#define ID_ATTR          "id"
#define NAME_ATTR        "name"
#define INTERFACE_ATTR   "interface"
#define ADDRESS_ATTR     "address"
#define MASK_ATTR        "mask"
#define SIGNALING_ATTR   "signaling"
#define LOW_ATTR         "low"
#define HIGH_ATTR        "high"
#define VNET_ATTR        "vnet"
#define VLAN_ID_ATTR     "vlanid"
#define DEST_IP_ATTR     "dest_ip"
#define GATEWAY_ATTR     "gw"

#ifndef BUFSIZE
#define BUFSIZE        1024
#endif

extern int  poolsXmlCkSum ;

typedef struct _PortRange {
  struct _PortRange *prev;
  struct _PortRange *next;

  unsigned long ip;
  unsigned long mask;
  char interface[INTF_NAME_LEN];
  char linterface[INTF_NAME_LEN];
  int signaling;
  int low;
  int high;
  PortAlloc *portAlloc;
  char vnetIfName[POOL_NAME_LEN]; /* Name of net-if - if any */
  struct _VnetIf  *vnetIf;         /* Pointer to Vnet to reduce lookups */
} PortRange;

// A NetIfRoute is a route that is mapped to a VLAN or a physical port
typedef struct _NetIfRoute {
  struct _NetIfRoute *prev;
  struct _NetIfRoute *next;

  unsigned long ip;
  unsigned long mask;
  unsigned long gw;

} NetIfRoute;

typedef struct _PoolAlloc {
  int id;
  char name[POOL_NAME_LEN];
  PortRange *portRanges;   // a circular list of all port ranges
} PoolAlloc;

typedef struct _VnetIf {
  char name[POOL_NAME_LEN];
  int  vlanId;
  char interface[INTF_NAME_LEN];
  NetIfRoute *route;
} VnetIf;

typedef struct _UserData {
  unsigned int dataExpected;
  char *tagName;
  void (*retrieveValue)(void*, const XML_Char*);
  void *result;

  PoolAlloc *palloc;
  PortRange *prange;
  VnetIf      *vnetIf;
  NetIfRoute *netIfRoute;
} UserData;


static PoolAlloc* poolTable[MAX_POOL_IDS];

typedef struct {
  unsigned long int nextAvail;
  VnetIf             *table[MAX_POOL_IDS];
} VnetIfTable;

static VnetIfTable  vnetIfTable;

extern int iserverPrimary;

unsigned int ckSumPoolFile(unsigned long *cksum);

static int getVlanId( PortRange *prange );


static PortRange* newPortRange () {
  PortRange *prange;

  prange = (PortRange *)calloc(1, sizeof(PortRange));
  if (prange == NULL)
    return NULL;

  ListInitElem(prange);

  return prange;
}

static PoolAlloc* newPoolAlloc () {
  PoolAlloc *palloc;

  palloc = (PoolAlloc *)calloc(1, sizeof(PoolAlloc));
  if (palloc == NULL)
    return NULL;

  return palloc;
}

static VnetIf  *newVnetIfAlloc () {
  VnetIf *vnetIf;

  vnetIf = (VnetIf *)calloc(1, sizeof(VnetIf));
  if (vnetIf == NULL)
    return NULL;

  return vnetIf;
}

static NetIfRoute  *newNetIfRouteAlloc () {
  NetIfRoute *netIfRt;

  netIfRt = (NetIfRoute *)calloc(1, sizeof(NetIfRoute));
  if (netIfRt == NULL)
    return NULL;

  ListInitElem(netIfRt);

  return netIfRt;
}

static void initUserData (UserData *userData) {
  int i;
  memset( userData, 0x00, sizeof( UserData ) );

  for( i = 0; i < MAX_POOL_IDS; i++ ) {
    vnetIfTable.table[i] = NULL;
  }
  vnetIfTable.nextAvail = 0;
}

static void resetUserData (UserData *userData) {
  userData->dataExpected = 0;
  userData->retrieveValue = NULL;
  userData->result = NULL;
  if (userData->tagName)
    free(userData->tagName);
  userData->tagName = NULL;
}


static void readIPAddress (unsigned long *result, const char *value)  {

  in_addr_t ip_addr;

  if( inet_pton( AF_INET, value, (void *)&ip_addr ) > 0 ) {
    *result = ntohl( (unsigned long)ip_addr );
  } else {
    *result = 0;
    NETERROR(MFCE, ("readIPAddress: unable to get IP address for %s", value));
  }
}

static void readMask (unsigned long *result, const char *value)  {
  *result = inet_network(value);
}

/* return 1 if true */
static int readFlag(const char *value) {
  NETDEBUG(MFCE,NETLOG_DEBUG4,("readFlag %s\n",value));
  return (strcasecmp(value,"true") == 0);
}

static void dataHandler (void *ud, const XML_Char *s, int len) {
  // we have no tag data to handle right now
  return;
}


static void extractPoolAttributes (UserData *userData, const char *name, unsigned int nameLen, const char **atts) {
  int j;

  for (j = 0; atts[j] != 0; j += 2) {
    if (!strcmp(ID_ATTR, atts[j])) {
      userData->palloc->id = strtol(atts[j+1], (char **)NULL, 10);
    } else if (!strcmp(NAME_ATTR, atts[j])) {
      nx_strlcpy(userData->palloc->name, atts[j+1], POOL_NAME_LEN);
    }
  }

  NETDEBUG(MFCE, NETLOG_DEBUG4, ("extractPoolAttributes: id = %d, name = %s\n", userData->palloc->id, userData->palloc->name));
}


static void extractPortAllocAttributes (UserData *userData, const char *name, unsigned int nameLen, const char **atts) {
  int j;
  char str[32];
  char mstr[32];

  for (j = 0; atts[j] != 0; j += 2) {
    if (!strcmp(INTERFACE_ATTR, atts[j])) {
      nx_strlcpy(userData->prange->interface, atts[j+1], INTF_NAME_LEN);
    } else if (!strcmp(ADDRESS_ATTR, atts[j])) {
      readIPAddress(&userData->prange->ip, atts[j+1]);
    } else if (!strcmp(MASK_ATTR, atts[j])) {
      readMask(&userData->prange->mask, atts[j+1]);
    } else if (!strcmp(SIGNALING_ATTR, atts[j])) {
      userData->prange->signaling = readFlag(atts[j+1]);
    } else if (!strcmp(LOW_ATTR, atts[j])) {
      userData->prange->low = strtol(atts[j+1], (char **)NULL, 10);
    } else if (!strcmp(HIGH_ATTR, atts[j])) {
      userData->prange->high = strtol(atts[j+1], (char **)NULL, 10);
    } else if (!strcmp(VNET_ATTR, atts[j])) {
      nx_strlcpy(userData->prange->vnetIfName, atts[j+1], POOL_NAME_LEN);
      userData->prange->vnetIf = (VnetIf *)NULL;
    }
  }

  NETDEBUG(MFCE, NETLOG_DEBUG4, ("extractPortAllocAttributes: interface = %s, addr = %s, mask = %s, signaling=%s, low = %d, high = %d\n", userData->prange->interface, FormatIpAddress(userData->prange->ip, str), FormatIpAddress(userData->prange->mask,mstr),userData->prange->signaling ? "true" : "false", userData->prange->low, userData->prange->high));
}

static void 
extractVnetIfAttributes( UserData *userData,
                         const char *name,
                         unsigned int nameLen,
                         const char **atts)
{
  int j;

  for (j = 0; atts[j] != 0; j += 2) {
    if (!strcmp(NAME_ATTR, atts[j])) {
      nx_strlcpy(userData->vnetIf->name, atts[j+1], POOL_NAME_LEN);
    } else if (!strcmp(VLAN_ID_ATTR, atts[j])) {
      userData->vnetIf->vlanId = strtol( atts[j + 1], (char **)NULL, 10 );
    } else if (!strcmp(INTERFACE_ATTR, atts[j])) {
      nx_strlcpy( userData->vnetIf->interface, atts[j + 1], INTF_NAME_LEN );
    } 
  }

  NETDEBUG(MFCE, NETLOG_DEBUG4, 
          ("extractVnetIfAttributes: interface = %s, vlan ID = %d, interface = %s\n",
            userData->vnetIf->name, userData->vnetIf->vlanId, userData->vnetIf->interface));
}

static void
extractNetIfRouteAttributes(	UserData *userData,
								const char *name,
								unsigned int nameLen,
								const char **atts)
{
  int j;
  char str[32];
  char mstr[32];
  char nstr[32];

  for (j = 0; atts[j] != 0; j += 2) {
    if (!strcmp(DEST_IP_ATTR, atts[j])) {
      readIPAddress(&userData->netIfRoute->ip, atts[j+1]);
    } else if (!strcmp(MASK_ATTR, atts[j])) {
      readMask(&userData->netIfRoute->mask, atts[j+1]);
    } else if (!strcmp(GATEWAY_ATTR, atts[j])) {
      readIPAddress(&userData->netIfRoute->gw, atts[j+1]);
    }
  }

  NETDEBUG(MFCE, NETLOG_DEBUG4, ("extractNetIfRouteAttributes: dest ip = %s, mask = %s, gateway =%s\n",
           FormatIpAddress(userData->netIfRoute->ip, str),
           FormatIpAddress(userData->netIfRoute->mask,mstr),
           FormatIpAddress(userData->netIfRoute->gw,nstr)) );
}

static void startElement (void *ud, const char *name, const char **atts) {
  unsigned int nameLen = strlen(name)+1;
  UserData *userData = (UserData*)ud;

  // ignore the root level tag
  if (!strcmp(POOLCFG_TAG, name)) {
    return;
  }

  // check for the new pool tag
  if (!strcmp(POOL_TAG, name)) {
    NETDEBUG(MFCE, NETLOG_DEBUG4, ("startElement: encountered new pool in the config file"));

    userData->palloc = newPoolAlloc();
    if (userData->palloc == NULL) {
      NETERROR(MFCE, ("startElement: cannot allocate memory for pool: %s\n", strerror(errno)));
      return;
    }

    extractPoolAttributes(userData, name, nameLen, atts);
    return;
  }


  // check for the new port alloc tag
  if (!strcmp(PORTALLOC_TAG, name)) {
    NETDEBUG(MFCE, NETLOG_DEBUG4, ("startElement: encountered new portalloc in the config file"));

    // if we were unable to malloc memory earlier, we can't proceed..
    if (userData->palloc == NULL) {
      NETDEBUG(MFCE, NETLOG_DEBUG4, ("pool not allocated, ignoring XML start tag %s\n", name));
      return;
    }

    userData->prange = newPortRange();
    if (userData->prange == NULL) {
      NETERROR(MFCE, ("startElement: cannot allocate memory for portalloc: %s\n", strerror(errno)));
      return;
    }

    extractPortAllocAttributes(userData, name, nameLen, atts);
    return;
  }

  // check for Net I/F 
  if( !strcmp( VNET_TAG, name ) ) {
    NETDEBUG(MFCE, NETLOG_DEBUG4, ("startElement: encountered new net-if in the config file"));
    userData->vnetIf = newVnetIfAlloc();
    if( userData->vnetIf == NULL ) {
      NETERROR(MFCE, ("startElement: cannot allocate memory for net-if: %s\n", strerror(errno)));
      return;
    }

    extractVnetIfAttributes( userData, name, nameLen, atts );
    return;
  }

  if( !strcmp( NETIF_ROUTE_TAG, name ) ) {
    NETDEBUG(MFCE, NETLOG_DEBUG4, ("startElement: encountered new route in the config file"));

    // if we were unable to malloc memory earlier, we can't proceed..
    if (userData->vnetIf == NULL) {
      NETDEBUG(MFCE, NETLOG_DEBUG4, ("net if not allocated, ignoring XML start tag %s\n", name));
      return;
    }

    userData->netIfRoute = newNetIfRouteAlloc();
    if( userData->netIfRoute == NULL ) {
      NETERROR(MFCE, ("startElement: cannot allocate memory for netif route: %s\n", strerror(errno)));
      return;
    }

    extractNetIfRouteAttributes( userData, name, nameLen, atts );
    return;
  }

  NETDEBUG(MFCE, NETLOG_DEBUG4, ("startElement: encountered unknown tag: %s\n", name));
}


static void endElement (void *ud, const char *name) {
  UserData *userData = (UserData*)ud;
  char str[32];
  char str1[32];
  char str2[32];

  resetUserData(userData);

  if (!strcmp(name, POOL_TAG))
  {
    // put the pool in the pool array
    poolTable[userData->palloc->id] = userData->palloc;

    NETDEBUG(MFCE, NETLOG_DEBUG4, ("endElement: created new portalloc for pool %d\n", userData->palloc->id));

    userData->palloc = NULL;
    userData->prange = NULL;
  } else if (!strcmp(name, PORTALLOC_TAG)) {
    if (userData->palloc->portRanges == NULL) {
      userData->palloc->portRanges = userData->prange;
    } else {
      ListInsert(userData->palloc->portRanges, userData->prange);
    }

    NETDEBUG(MFCE, NETLOG_DEBUG4, ("endElement: create new portrange for pool %d: %s:%s:%d:%d\n",
									userData->palloc->id, userData->prange->interface,
									FormatIpAddress(userData->prange->ip, str),
									userData->prange->low,
									userData->prange->high));

    userData->prange = NULL;
  } else if( !strcmp( name, VNET_TAG ) ) {
    vnetIfTable.table[vnetIfTable.nextAvail++] = userData->vnetIf;
  }
  else if( (!strcmp( name, NETIF_ROUTE_TAG )) && (userData->vnetIf != NULL) ) {
    if( userData->vnetIf->route == NULL ) {
      userData->vnetIf->route = userData->netIfRoute;
    } else {
      ListInsert( userData->vnetIf->route, userData->netIfRoute );
    }

    NETDEBUG(MFCE, NETLOG_DEBUG4, 
             ("endElement: create new net route for net I/F %s/%s dest IP = %s/%s GW= %s\n",
             userData->vnetIf->name, 
             userData->vnetIf->interface,
             FormatIpAddress(userData->netIfRoute->ip, str),
             FormatIpAddress(userData->netIfRoute->mask, str1),
             FormatIpAddress(userData->netIfRoute->gw, str2)) );

    userData->netIfRoute = NULL;
  }
}


static void readXML (char *filename) {
  char buf[BUFSIZE];
  XML_Parser parser = XML_ParserCreate(NULL);
  int done;
  FILE *file = NULL;
  UserData userData;

  // open the config file
  if ((file = fopen(filename, "r")) == NULL) {
    NETERROR(MFCE, ("readXML: cannot open %s: %s\n", filename, strerror(errno)));
    return;
  }

  initUserData(&userData);

  XML_SetUserData(parser, &userData);
  XML_SetElementHandler(parser, startElement, endElement);
  XML_SetCharacterDataHandler(parser, dataHandler);

  do {
    size_t len = fread(buf, 1, BUFSIZE, file);
    done = len < BUFSIZE;
    if (XML_Parse(parser, buf, len, done) == 0) {
      NETERROR(MFCE, ("readXML: error parsing %s: %s at %d\n", filename, XML_ErrorString(XML_GetErrorCode(parser)), XML_GetCurrentLineNumber(parser)));
      return;
    }
  } while (!done);

  XML_ParserFree(parser);
  fclose(file);
}


static inline PoolAlloc* getPoolAllocForId (int poolid) {

  if (poolid >= MAX_POOL_IDS || poolid < 0) {
    NETERROR(MFCE, ("getPoolAllocForId: invalid pool id %d\n", poolid));
    return NULL;
  }

  if (poolTable[poolid] == NULL) {
    NETERROR(MFCE, ("getPoolAllocForId: pool id %d empty\n", poolid));
    return NULL;
  }

  return poolTable[poolid];
}


static inline void deleteFromPool (int poolid, PortRange *prange, int clearPortAlloc) {

  // if we are pointing to the element to be deleted, point to the next element instead
  if (poolTable[poolid]->portRanges == prange)
    poolTable[poolid]->portRanges = prange->next;

  // if the only element left is the one to be deleted, delete the pool also
  if (ListIsSingle(prange)) {
    // only one element in the list
    poolTable[poolid]->portRanges = (PortRange *)0xDEADBEEF;
    free(poolTable[poolid]);
    poolTable[poolid] = NULL;
  }

  // unlink the element from the circular list
  ListDelete(prange);

  // free the port range
  if (clearPortAlloc)
    clearPortAllocation(prange->portAlloc);
  free(prange);
}

/**
 * This routine returns the numeric value of the first
 * digit in a string.
 * This routine assumes that a hotknife interface name 
 * has the form of hk0,1 
 *    where 0 is the board number
 *          1 is the port
 * 
 *
 * Parameters:
 *    ifaceName: A string containing a number
 * Returns:
 *  Value of first digit in string. -1 if error
 */
int
getPortnumFromIfacename( char *ifaceName )
{
	char *c;

  // Look for ','
  for( c = ifaceName; (*c != (ulong) NULL) && (*c != ','); c++ );

  if(    (*c != ',')
      || (*c == (ulong) NULL) 
      || !((*(c + 1) >= '0') && (*(c + 1) <= '9')) ) {
    return( -1 );
  }
  
  return( (int)(*(c + 1) - '0') );
}

/**
 * This method initializes the pool allocation library.
 * 
 * Parameters:
 *  xml file path that contains the pool configuration (pools.xml)
 *
 * Returns:
 *  <none>
 */
void initPoolAllocation (char *file) {
  int i, selfid,  count;
  PortRange *prange, *tmp;
  char str[32], mstr[32], nstr[32];
  int netIndex;
  VnetIf  *vnetIf;
  NetIfRoute *rt;

  // initialize the static array that holds the pool structures
  for (i = 0; i < MAX_POOL_IDS; i++) {
    poolTable[i] = NULL;
  }

  readXML(file);

  // validate the configuration ----TODO

  // assign portallocs, if any error happens, delete from the list
  for (count = i = 0; i < MAX_POOL_IDS; i++) {
    if (poolTable[i] == NULL || poolTable[i]->portRanges == NULL)
      continue;

    count++;  // count the number of pools

    prange = poolTable[i]->portRanges;
    if( prange != NULL ) { 
       do {
          tmp = NULL;
          if (prange->signaling == POOL_INTF_MEDIA) { // only allocate ports if a media pool
             prange->portAlloc = initPortAllocation(prange->low, prange->high);
             if (prange->portAlloc == NULL) {
                NETERROR(MFCE, ("initPoolAllocation: unable to portalloc for %s:%s:%d:%d\n",
                        prange->interface, 
                        FormatIpAddress(prange->ip, str),
                        prange->low,
                        prange->high));
                tmp = prange;
             }
          }
          prange = prange->next;
          if (tmp != NULL) {
              deleteFromPool(i, tmp, 0);
          }
        } while (poolTable[i] != NULL && prange != poolTable[i]->portRanges);
     } else {
         NETERROR( MFCE, ("initPoolAllocation: Pool with no port range found.\n") );
     }
  }

  NETDEBUG(MFCE, NETLOG_DEBUG2, ("initPoolAllocation: Num pools configured: %d\n", count));

  // plumb the interfaces configured
  selfid = (((getpid() & 0xffff) << 16) | (pthread_self() & 0xffff));
  for (i = 0; i < MAX_POOL_IDS; i++) {
    if (poolTable[i] == NULL)
      continue;

    prange = poolTable[i]->portRanges;
    if( prange != NULL ) { 
       do {
         char lifname[255];
         if (prange->signaling == POOL_INTF_MEDIA) {
           if( IS_HKNIFE_FW ) {

             strcpy( lifname, prange->interface );  /* Initialize this for now */

             if( prange->vnetIfName[0] != (ulong) NULL ) {
               // Plumb hknife based on net-if information and setup the routes

               // Find the network interface
               for( netIndex = 0; 
                  (netIndex < vnetIfTable.nextAvail) && 
                  (strcmp( vnetIfTable.table[netIndex]->name, prange->vnetIfName ));
                  netIndex++ );
               if( netIndex < vnetIfTable.nextAvail ) {
                 vnetIf = vnetIfTable.table[netIndex];
                 if( hknife_plumb_if( 0, /* blade 0 */
                                      getPortnumFromIfacename( vnetIf->interface ),
                                      vnetIf->vlanId,
                                      prange->ip,
                                      prange->mask ) ) {
                   NETERROR(MFCE, 
                          ("initPoolAllocation: unable to hknife_plumb_if: "
                          "if=%s, vlan=%d, ip=%s mask=%s\n",
                          vnetIf->interface,
                          vnetIf->vlanId,
                          FormatIpAddress(prange->ip, str),
                          FormatIpAddress(prange->mask,mstr)));
                 } else {
                   // Interface successfully plumbed - establish all the routes

                   strcpy( lifname, vnetIf->interface );  // Set the IF names right
                   strcpy( prange->interface, vnetIf->interface );

                   rt = vnetIf->route;
                   if( rt != NULL ) {
                     do {
                       NETDEBUG(MFCE, NETLOG_DEBUG2,
                                ("PoolAllocation: Adding route I/F = %s vlanid = %d dest = %s/%s gw = %s",
                                vnetIf->interface, 
                                vnetIf->vlanId, 
                                FormatIpAddress( rt->ip, str),
                                FormatIpAddress( rt->mask, mstr),
                                FormatIpAddress( rt->gw, nstr) ));

                       if( hknife_add_route( 0, /* Blade id */
                                           getPortnumFromIfacename( vnetIf->interface ),
                                           vnetIf->vlanId,
                                           rt->ip,
                                           rt->mask,
                                           rt->gw ) ) {
                         NETERROR( MFCE, 
                                 ("Error adding route I/F = %s, vlan_id = %d %s/%s  %s\n",
                                 vnetIf->interface,
                                 vnetIf->vlanId,
                                 FormatIpAddress(rt->ip, str),
                                 FormatIpAddress(rt->mask,mstr),
                                 FormatIpAddress(rt->ip, nstr) ));
                       }

                       rt = rt->next;
                     } while( rt != vnetIf->route);
                   }
                 }
               } else {
                 NETERROR(MFCE, 
                        ("initPoolAllocation: unable to hknife_plumb_if: cannot find net-if %s\n",
                        prange->vnetIfName) );
               }
             } else {
               // There is not net I/F name (VLAN) associated with this port
               // perform hknife plumbing based of range interface
               if( hknife_plumb_if( 0, /* blade 0 */
                                  getPortnumFromIfacename( prange->interface ),
                                  0, /* VLAN ID */
                                  prange->ip,
                                  prange->mask ) ) {
                 NETERROR(MFCE, 
                        ("initPoolAllocation: unable to hknife_plumb_if: "
                        "ip=%s, if=%s, mask=%s\n",
                        prange->interface, 
                        FormatIpAddress(prange->ip, str),
                        FormatIpAddress(prange->mask, mstr)));
               }
               strcpy( lifname, prange->interface );
             }
           } else {
             // Not HKNIFE 
                 unsigned short dummy = 0;
	         if ( PlumbIf(prange->interface,prange->ip,prange->mask,lifname, 4096, &dummy) == -1 ) {
	            NETERROR(MFCE, ("initPoolAllocation: unable to PlumbIf: ip=%s, if=%s, mask=%s\n",prange->interface, FormatIpAddress(prange->ip, str),  FormatIpAddress(prange->mask,mstr)));
	         }
           }

           NETDEBUG(MFCE, NETLOG_DEBUG2, ("initPoolAllocation: PlumbIf added lif=%s, if=%s, ip=%s,  mask=%s\n", lifname,prange->interface, FormatIpAddress(prange->ip, str),  FormatIpAddress(prange->mask,mstr)));

	       if (iserverPrimary) {
		     /* StatusChgIf( lifname,1);  up the interface */
           }
           strcpy(prange->linterface,lifname);
         }
         prange = prange->next;
       } while (poolTable[i] != NULL && prange != poolTable[i]->portRanges);
     } else {
         NETERROR( MFCE, ("initPoolAllocation: Pool with no port range found.\n") );
     }

  }
}

/**
 * This method frees all resources allocated in the pool allocation library.
 * Use it during graceful shutdown.
 *
 * Parameters:
 *  <none>
 *
 * Returns:
 *  <none>
 */
void clearPoolAllocation () {
  int poolid, selfid;
  PortRange *prange;

  selfid = (((getpid() & 0xffff) << 16) | (pthread_self() & 0xffff));
  for (poolid = 0; poolid < MAX_POOL_IDS; poolid++) {
    while (poolTable[poolid] != NULL) {
      prange = poolTable[poolid]->portRanges;     
      if( prange != NULL ) { 
 
        if (prange->signaling == POOL_INTF_MEDIA) {
          if ( UnplumbIf(prange->linterface, prange->ip, 0) == -1 ) {
            NETERROR(MFCE, ("clearPoolAllocation: unable to UnplumbIf: lif=%s\n",prange->linterface));
          }
          NETDEBUG(MFCE, NETLOG_DEBUG2, ("clearPoolAllocation: UnplumbIf removed lif=%s\n", prange->linterface));

        }
        // delete from the circular list
        deleteFromPool(poolid, poolTable[poolid]->portRanges, 1);
     } else {
         NETERROR( MFCE, ("initPoolAllocation: Pool with no port range found.\n") );
     }

    }
  }
}


/**
 * This method reads the given pool configuration, compares against the one
 * currently being used, and makes the relevant changes to the internal structures.
 *
 * Changes that would take effect only after an MSW restart:
 *  shrink the port range of an existing pool
 *
 * Changes that would take effect immediately:
 *  extend an existing port range
 *  add a new pool
 *  delete a pool
 *  add a new ip/port range in an existing pool
 *  change the ip address in an existing pool (this would require a MSW restart anyway)
 *  change the interface name in an existing pool (this would require a MSW restart anyway)
 *
 * Note: changing a pool id in the pools.xml essentially means that a new pool is
 *       added and an old pool is deleted
 *
 * Parameters:
 *  xml file path that contains the new pool configuration (pools.xml)
 *
 * Returns:
 *  -1 - if changes were significant enough to require a restart
 *   0 - no changes or if changes were reflected without needing a restart
 */
int reconfigPoolAllocation (char *filename) {

  // ---TODO

  return -1;
}

/**
 * This method reads the given pool configuration, compares against the one
 * currently being used and returns if the config has changes.
 *
 * Parameters:
 *  xml file path that contains the new pool configuration (pools.xml)
 *
 * Returns:
 *  -1 - if there are changes in the file
 *   0 - if no changes in the file
 */
int poolConfigChanged (char *filename) {
  unsigned long   c=0; 

  if (ckSumPoolFile(&c)) {
    /* if we can't compute CheckSum assume a change has happened */
    return -1;
  } else {
    if  (c != poolsXmlCkSum) {
      poolsXmlCkSum = c;
      return -1;
    }
  }
  return 0;
}

/**
 * This method allocates a port from the given pool id. The port allocated
 * is always an even number. The associated (port+1) odd number
 * can also be safely considered to be allocated. It also fills in the interface
 * name and the ip address in passed buffers.
 *
 * If the port and ip passed are already filled in, it tries to allocate the
 * the specified ip/port only. 
 *
 * Parameters:
 *  the pool id
 *  the allocate port number to be filled in
 *  the ip address to be filled in
 *  the interface name to be filled in
 *
 * Returns:
 *   0 if it allocated a valid port number
 *  -1 if no ports could be allocated (invalid poolid, no more ports available)
 */
int allocatePool (int poolid, unsigned short *port, unsigned int *ip, char *intf, int *vlan, dirxn d) {
  PoolAlloc *palloc;
  PortRange *prange;
  int retcode = -1;

  palloc = getPoolAllocForId(poolid);
  if (palloc == NULL) {
    NETERROR(MFCE, ("allocatePool: Unable to find pool %d\n", poolid));
    return retcode;
  }

  // go through all the PortRanges if necessary to allocate a port from this pool
  prange = palloc->portRanges;

  if ((*port != 0) && (*ip != 0)) {
  	do {
    	if ((palloc->portRanges->signaling == POOL_INTF_MEDIA) &&
					(palloc->portRanges->ip == *ip)){ 
				retcode = allocPort(palloc->portRanges->portAlloc, *port, d);
				if (retcode >= 0) {
					nx_strlcpy(intf, palloc->portRanges->interface, INTF_NAME_LEN);
          *vlan = getVlanId( palloc->portRanges );
					retcode = 0;
				}
			}
    	palloc->portRanges = palloc->portRanges->next;  // move to the next port range for the next allocation
  	} while (retcode && (prange != palloc->portRanges));
  }
  else {
  	do {
    	if (palloc->portRanges->signaling == POOL_INTF_MEDIA) {             // only allocate ports if a media pool
				retcode = allocPort(palloc->portRanges->portAlloc, 0, d);

				if (retcode >= 0) {
					*port = retcode;
					*ip = palloc->portRanges->ip;
          *vlan = getVlanId( palloc->portRanges );
					nx_strlcpy(intf, palloc->portRanges->interface, INTF_NAME_LEN);
	
					retcode = 0;
      			}
    	}
    	palloc->portRanges = palloc->portRanges->next;  // move to the next port range for the next allocation
  	} while (retcode && (prange != palloc->portRanges));
  }


  NETDEBUG(MFCE, NETLOG_DEBUG4, ("allocatePool: returning retcode %d\n", retcode));
  return retcode;
}


/**
 * This method returns an interface name that is associated with the
 * given pool id. If there are more than one interfaces associated with 
 * a pool, this chooses a random interface.
 *
 * Parameters:
 *  the poolid
 *  the interface name to be filled in
 *
 * Returns:
 *   a pointer to the interface name (same as the buffer passed in)
 *  NULL if no interface can be found for the given pool id
 */
char* getInterfaceForPool (int poolid, char *intf)
{
  PoolAlloc *palloc;

  palloc = getPoolAllocForId(poolid);
  if (palloc == NULL) {
    NETERROR(MFCE, ("getInterfaceForPool: Unable to find pool %d\n", poolid));
    return NULL;
  }

  nx_strlcpy(intf, palloc->portRanges->interface, INTF_NAME_LEN);
  palloc->portRanges = palloc->portRanges->next;

  NETDEBUG(MFCE, NETLOG_DEBUG4, ("getInterfaceForPool: returning interface %s\n", intf));
  return intf;
}


/**
 * This method frees a previously allocated ip/port in the given pool id
 *
 * Parameters:
 *  the poolid
 *  the port number to be freed
 *  the ip address to be freed
 *
 * Returns:
 *  <none>
 */
void freePool (int poolid, int port, int ip, dirxn d) {
  PoolAlloc *palloc;
  PortRange *prange;
  int found;

  palloc = getPoolAllocForId(poolid);
  if (palloc == NULL) {
    NETERROR(MFCE, ("freePool: Unable to find pool %d\n", poolid));
    return;
  }

  found = 0;
  prange = palloc->portRanges;
  do {
    if (prange->signaling == POOL_INTF_MEDIA) {             // only allocate ports if a media pool
      // find which PortRange in this pool we allocated this port from
      if (prange->ip == ip && prange->portAlloc->low <= port && prange->portAlloc->high >= port) {
	found = 1;
	break;
      }
    }
    prange = prange->next;
  } while (prange != palloc->portRanges);

  if (!found) {
    NETERROR(MFCE, ("freePool: pool id %d does not have ip 0x%x, port %d\n", poolid, ip, port));
    return;
  }

  freePort(prange->portAlloc, port, d);
}


/**
 * This method copies all unique interface names that are configured in pools.xml into
 * the 2-d array passed in. Interface names returned are physical names, not logical.
 *
 * Parameters:
 * itype = 0 media, 1=signaling (POOL_INTF_MEDIA, POOL_INTF_SIG)
 * ifs = the 2-d array that will carry the interface names
 *
 * Returns:
 *  the number of valid elements in the array
 */
int getPooledInterfaces (int iType, char ifs[MAX_POOL_IDS][INTF_NAME_LEN]) {
  int poolid, numIfs, i, found;
  PortRange *prange;
  char intf[INTF_NAME_LEN+1];

  for (numIfs = poolid = 0; poolid < MAX_POOL_IDS; poolid++) {
    if (poolTable[poolid] == NULL)
      continue;

    prange = poolTable[poolid]->portRanges;
    if( prange != NULL ) { 
      do {
        // extract the physical interface name
        if (prange->signaling == iType) {
          nx_strlcpy(intf, prange->interface, strcspn(prange->interface, ":")+1);

          // check if the interface is already in the list
          for (found = i = 0; i < numIfs; i++) {
            if (!strcmp(intf, ifs[i])) {
              found = 1;
              break;
            }
          }
          // if not in the list, copy it in
          if (!found)
          nx_strlcpy(ifs[numIfs++], intf, INTF_NAME_LEN);
        }
        prange = prange->next;
      } while (prange != poolTable[poolid]->portRanges);
    } else {
      NETERROR( MFCE, ("initPoolAllocation: Pool with no port range found.\n") );
    }

  }

  return numIfs;
}


/**
 * This method get the vlan id from a PortRange
 *
 * Parameters:
 * portRange - Port range from whict to extract the VLAN ID
 *
 * Returns:
 *  vlan id - 0 indicates no VLAN
 */
static int
getVlanId( PortRange *prange )
{

  unsigned int netIndex;

  if( prange->vnetIfName[0] != (char)NULL )
  {
    if( prange->vnetIf != (VnetIf *)NULL )
    {
      return( prange->vnetIf->vlanId );
    }
    else
    {
      /* Need to go through all Net IFs */
      for( netIndex = 0; 
        (netIndex < vnetIfTable.nextAvail) && 
        (strcmp( vnetIfTable.table[netIndex]->name, prange->vnetIfName ));
        netIndex++ );

      if( netIndex < vnetIfTable.nextAvail ) {
        prange->vnetIf = vnetIfTable.table[netIndex];
        return( prange->vnetIf->vlanId );
      }
      else
      {
        return( 0 ); /* No VLAN */
      }
    }
  }
  else
  {
    return( 0 ); /* No VLAN */
  }
}


unsigned long crctab[] = {
	0x7fffffff,
	0x77073096,  0xee0e612c,  0x990951ba,  0x076dc419,  0x706af48f,
	0xe963a535,  0x9e6495a3,  0x0edb8832,  0x79dcb8a4,  0xe0d5e91e,
	0x97d2d988,  0x09b64c2b,  0x7eb17cbd,  0xe7b82d07,  0x90bf1d91,
	0x1db71064,  0x6ab020f2,  0xf3b97148,  0x84be41de,  0x1adad47d,
	0x6ddde4eb,  0xf4d4b551,  0x83d385c7,  0x136c9856,  0x646ba8c0,
	0xfd62f97a,  0x8a65c9ec,  0x14015c4f,  0x63066cd9,  0xfa0f3d63,
	0x8d080df5,  0x3b6e20c8,  0x4c69105e,  0xd56041e4,  0xa2677172,
	0x3c03e4d1,  0x4b04d447,  0xd20d85fd,  0xa50ab56b,  0x35b5a8fa,
	0x42b2986c,  0xdbbbc9d6,  0xacbcf940,  0x32d86ce3,  0x45df5c75,
	0xdcd60dcf,  0xabd13d59,  0x26d930ac,  0x51de003a,  0xc8d75180,
	0xbfd06116,  0x21b4f4b5,  0x56b3c423,  0xcfba9599,  0xb8bda50f,
	0x2802b89e,  0x5f058808,  0xc60cd9b2,  0xb10be924,  0x2f6f7c87,
	0x58684c11,  0xc1611dab,  0xb6662d3d,  0x76dc4190,  0x01db7106,
	0x98d220bc,  0xefd5102a,  0x71b18589,  0x06b6b51f,  0x9fbfe4a5,
	0xe8b8d433,  0x7807c9a2,  0x0f00f934,  0x9609a88e,  0xe10e9818,
	0x7f6a0dbb,  0x086d3d2d,  0x91646c97,  0xe6635c01,  0x6b6b51f4,
	0x1c6c6162,  0x856530d8,  0xf262004e,  0x6c0695ed,  0x1b01a57b,
	0x8208f4c1,  0xf50fc457,  0x65b0d9c6,  0x12b7e950,  0x8bbeb8ea,
	0xfcb9887c,  0x62dd1ddf,  0x15da2d49,  0x8cd37cf3,  0xfbd44c65,
	0x4db26158,  0x3ab551ce,  0xa3bc0074,  0xd4bb30e2,  0x4adfa541,
	0x3dd895d7,  0xa4d1c46d,  0xd3d6f4fb,  0x4369e96a,  0x346ed9fc,
	0xad678846,  0xda60b8d0,  0x44042d73,  0x33031de5,  0xaa0a4c5f,
	0xdd0d7cc9,  0x5005713c,  0x270241aa,  0xbe0b1010,  0xc90c2086,
	0x5768b525,  0x206f85b3,  0xb966d409,  0xce61e49f,  0x5edef90e,
	0x29d9c998,  0xb0d09822,  0xc7d7a8b4,  0x59b33d17,  0x2eb40d81,
	0xb7bd5c3b,  0xc0ba6cad,  0xedb88320,  0x9abfb3b6,  0x03b6e20c,
	0x74b1d29a,  0xead54739,  0x9dd277af,  0x04db2615,  0x73dc1683,
	0xe3630b12,  0x94643b84,  0x0d6d6a3e,  0x7a6a5aa8,  0xe40ecf0b,
	0x9309ff9d,  0x0a00ae27,  0x7d079eb1,  0xf00f9344,  0x8708a3d2,
	0x1e01f268,  0x6906c2fe,  0xf762575d,  0x806567cb,  0x196c3671,
	0x6e6b06e7,  0xfed41b76,  0x89d32be0,  0x10da7a5a,  0x67dd4acc,
	0xf9b9df6f,  0x8ebeeff9,  0x17b7be43,  0x60b08ed5,  0xd6d6a3e8,
	0xa1d1937e,  0x38d8c2c4,  0x4fdff252,  0xd1bb67f1,  0xa6bc5767,
	0x3fb506dd,  0x48b2364b,  0xd80d2bda,  0xaf0a1b4c,  0x36034af6,
	0x41047a60,  0xdf60efc3,  0xa867df55,  0x316e8eef,  0x4669be79,
	0xcb61b38c,  0xbc66831a,  0x256fd2a0,  0x5268e236,  0xcc0c7795,
	0xbb0b4703,  0x220216b9,  0x5505262f,  0xc5ba3bbe,  0xb2bd0b28,
	0x2bb45a92,  0x5cb36a04,  0xc2d7ffa7,  0xb5d0cf31,  0x2cd99e8b,
	0x5bdeae1d,  0x9b64c2b0,  0xec63f226,  0x756aa39c,  0x026d930a,
	0x9c0906a9,  0xeb0e363f,  0x72076785,  0x05005713,  0x95bf4a82,
	0xe2b87a14,  0x7bb12bae,  0x0cb61b38,  0x92d28e9b,  0xe5d5be0d,
	0x7cdcefb7,  0x0bdbdf21,  0x86d3d2d4,  0xf1d4e242,  0x68ddb3f8,
	0x1fda836e,  0x81be16cd,  0xf6b9265b,  0x6fb077e1,  0x18b74777,
	0x88085ae6,  0xff0f6a70,  0x66063bca,  0x11010b5c,  0x8f659eff,
	0xf862ae69,  0x616bffd3,  0x166ccf45,  0xa00ae278,  0xd70dd2ee,
	0x4e048354,  0x3903b3c2,  0xa7672661,  0xd06016f7,  0x4969474d,
	0x3e6e77db,  0xaed16a4a,  0xd9d65adc,  0x40df0b66,  0x37d83bf0,
	0xa9bcae53,  0xdebb9ec5,  0x47b2cf7f,  0x30b5ffe9,  0xbdbdf21c,
	0xcabac28a,  0x53b39330,  0x24b4a3a6,  0xbad03605,  0xcdd70693,
	0x54de5729,  0x23d967bf,  0xb3667a2e,  0xc4614ab8,  0x5d681b02,
	0x2a6f2b94,  0xb40bbe37,  0xc30c8ea1,  0x5a05df1b,  0x2d02ef8d
};

static unsigned int crc(FILE *fd, unsigned long *cval, unsigned long  *clen);

unsigned int 
ckSumPoolFile(unsigned long *cksum)
{
  FILE *fp;
  unsigned long len;
  unsigned char *file = "/usr/local/nextone/bin/pools.xml";

  if (NULL == (fp = fopen(file, "rb")))
  {
		NETERROR(MFCE, ("Unable to open %s for reading\n", file));
		return(1);
  }

  if (crc(fp, cksum, &len))
  {
	  NETERROR(MFCE, ("Unable to do checksum of %s\n", file));
	  return(1);
  }

  NETDEBUG(MFCE, NETLOG_DEBUG4, ("%lu bytes read\n", len));
  NETDEBUG(MFCE, NETLOG_DEBUG4, ("The checksum of %s is %#lx\n", file,*cksum));
  return 0;

}

/*
 * crc --
 *	Compute a POSIX.2 checksum.  This routine has been broken out since
 *	it is anticipated that other programs will use it.  It takes a file
 *	descriptor to read from and locations to store the crc and the number
 *	of bytes read.  It returns 0 on success and 1 on failure.  Errno is
 *	set on failure.
 */
static unsigned int
crc(FILE *fd, unsigned long *cval, unsigned long  *clen)
{
	register int i, nr, step;
	register unsigned char *p;
	register unsigned long crcv, total;
	unsigned char buf[1];

	crcv = step = total = 0;
	while((nr = fread(buf, sizeof(char), sizeof(buf), fd) > 0))
		for (total += nr, p = buf; nr--; ++p) {
			if (!(i = crcv >> 24 ^ *p)) {
				i = step++;
				if (step >= sizeof(crctab)/sizeof(crctab[0]))
					step = 0;
			}
			crcv = ((crcv << 8) ^ crctab[i]) & 0xffffffff;
		}
	if (nr < 0)
		return(1);

	*cval = crcv;
	*clen = total;
	return(0);
}


