#include "ipc.h"
#include "bits.h"
#include "serverdb.h"
#include "mem.h"
#include "entry.h"

/* The crucial match functions... which make the cache
 * independent of usage.
 */
/* Just match the phone... */
int
gPhoneMatch(void *x, void *opaque)
{
     CacheTableInfo *info = (CacheTableInfo *)x;
     char *phone = (char *)opaque;

     if (BIT_TEST(info->data.sflags|info->data.dflags, 
					ISSET_PHONE))
     {
	  return (strcmp(info->data.phone, phone));
     }

     return -1;
}

int
gVpnPhoneMatch(void *x, void *opaque)
{
     CacheTableInfo *info = (CacheTableInfo *)x;
     char *phone = (char *)opaque;

     if (BIT_TEST(info->data.sflags|info->data.dflags, 
					ISSET_VPNPHONE))
     {
	  return (strcmp(info->data.vpnPhone, phone));
     }

     return -1;
}
