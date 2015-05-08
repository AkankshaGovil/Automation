#ifndef __POOLALLOC_H
#define __POOLALLOC_H

#include "portalloc.h"


#define INTF_NAME_LEN    16
#define POOL_NAME_LEN    64
#define MAX_POOL_IDS    256
#define POOL_INTF_SIG     1
#define POOL_INTF_MEDIA   0

#define POOLS_CFG_FILE  "pools.xml"

extern int	poolsXmlCkSum;

/**
 * This method initializes the pool allocation library.
 * 
 * Parameters:
 *  xml file path that contains the pool configuration (pools.xml)
 *
 * Returns:
 *  <none>
 */
extern void initPoolAllocation (char*);

/**
 * This routine returns the numeric value of the first
 * digit in a string.
 * This routine assumes that a hotknife interface name 
 * has the form of hk0,1 
 *    where 0 is the board number
 *          1 is the port
 *
 * Parameters:
 *    ifaceName: A string containing a number
 * Returns:
 *  Value of first digit in string. -1 if error
 */
extern int getPortnumFromIfacename( char *ifaceName );

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
extern void clearPoolAllocation ();

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
extern int reconfigPoolAllocation (char*);

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
extern int poolConfigChanged (char*);

/**
 * This method allocates a port from the given pool id. The port allocated
 * is always an even number. The associated (port+1) odd number
 * can also be safely considered to be allocated. It also fills in the interface
 * name and the ip address in passed buffers.
 *
 * Parameters:
 *  the pool id
 *  the allocate port number to be filled in
 *  the ip address to be filled in
 *  the interface name to be filled in
 *  the dirxn of the port (whether it is rx/tx)
 *
 * Returns:
 *   0 if it allocated a valid port number
 *  -1 if no ports could be allocated (invalid poolid, no more ports available)
 */
extern int allocatePool (int poolid, unsigned short *port, unsigned int *ip, char *intf, int *vlan, dirxn d); 

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
extern char* getInterfaceForPool (int, char*);

/**
 * This method frees a previously allocated ip/port in the given pool id
 *
 * Parameters:
 *  the poolid
 *  the port number to be freed
 *  the ip address to be freed
 *  the dirxn of the port (whether it is rx/tx)
 *
 * Returns:
 *  <none>
 */
extern void freePool (int, int, int, dirxn);


/**
 * This method copies all unique interface names that are configured in pools.xml into
 * the 2-d array passed in. Interface names returned are physical names, not logical.
 *
 * Parameters:
 *  iType = 0 media, 1=signaling
 *  ifs = the 2-d array that will carry the interface names
 *
 * Returns:
 *  the number of valid elements in the array
 */
extern int getPooledInterfaces (int iType, char [MAX_POOL_IDS][INTF_NAME_LEN]);

extern unsigned int ckSumPoolFile(unsigned long  *sum);

#endif
