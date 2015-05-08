#ifndef _NOREPL_H_
#define _NOREPL_H_

bool_t xdr_pMFCP_Session(register XDR *xdrs, pMFCP_Session *objp);
bool_t xdr_SCC_EventProcessor(register XDR *xdrs, SCC_EventProcessor *objp);
bool_t xdr_pListEntry(register XDR *xdrs, pListEntry *objp);
bool_t xdr_List(register XDR *xdrs, List *objp);
bool_t xdr_pCdrArgs(register XDR *xdrs, pCdrArgs *objp);
bool_t xdr_pheader_url_list(register XDR *xdrs, pheader_url_list *objp);
bool_t xdr_CallRealmInfo(register XDR *xdrs, CallRealmInfo *objp);

#endif
