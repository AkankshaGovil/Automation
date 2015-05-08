#include "call_handle.h"
#include "norepl.h"

static int foo = 0;

/* encode objp as integer 0 - used when a pointer needs to be set to NULL
 * so that the data pointed to is not replicated */

#define REPLICATE_NULL(x) bool_t xdr_##x (register XDR *xdrs, x *objp) \
{ \
	if (!xdr_int(xdrs, &foo)) \
		return (FALSE); \
	return (TRUE); \
}

/* encode as - defined explicitly to get around declaring the typedef'd types in
 * the .x files */ 

#define ENCODE_AS(x, y, z) bool_t xdr_##x (register XDR *xdrs, x *objp) \
{ \
	return(y (xdrs, (z *)objp)); \
}

#define ENCODE_UNION_AS(x, y, z, f) bool_t xdr_##x (register XDR *xdrs, x *objp) \
{ \
	return(y (xdrs, (z *)(&(objp->f)))); \
}

REPLICATE_NULL(pMFCP_Session)
REPLICATE_NULL(pListEntry)
REPLICATE_NULL(List)
REPLICATE_NULL(pCdrArgs)
REPLICATE_NULL(pheader_url_list)
REPLICATE_NULL(pSipEventHandle)
REPLICATE_NULL(SCC_EventProcessor)

ENCODE_AS(SCC_CallLeg, xdr_enum, enum_t)
ENCODE_AS(SCC_CallState, xdr_enum, enum_t)
ENCODE_AS(SCC_CallHandleType, xdr_enum, enum_t)
ENCODE_AS(Address_e, xdr_enum, enum_t)
ENCODE_AS(FCEStatusStruct, xdr_u_char, u_char)
ENCODE_UNION_AS(Handle, xdr_SipCallHandle, SipCallHandle, sipCallHandle)
ENCODE_UNION_AS(IPaddr, xdr_u_long, unsigned long, l)

