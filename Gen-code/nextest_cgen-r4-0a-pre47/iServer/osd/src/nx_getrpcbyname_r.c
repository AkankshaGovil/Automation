#include "osdconfig.h"
#include "nxosd.h"

#if HAVE_FUNC_GETRPCBYNAME_R_4
 struct  rpcent*  nx_getrpcbyname_r(const  char  *name,   struct
 rpcent *result, char *buffer, int buflen)
{

  return getrpcbyname_r( name, result, buffer, buflen);

}
#elif  HAVE_FUNC_GETRPCBYNAME_R_5

 struct  rpcent*  nx_getrpcbyname_r(const  char  *name,   struct
 rpcent *result, char *buffer, int buflen)
{

  struct rpcent* res;
  getrpcbyname_r( name, result, buffer, buflen, &res);
  return res;

}
#else
#error getrpcbyname_r not found
#endif
