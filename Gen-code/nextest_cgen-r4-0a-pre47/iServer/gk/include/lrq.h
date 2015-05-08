#ifndef _lrq_h_
#define _lrq_h_

int GKHandleLCF(
	    IN	HAPPRAS 		haRas,
		IN	HRAS			hsRas);

int GKHandleLRJ(
		IN	HAPPRAS 		haRas,
		IN	HRAS			hsRas,
		IN	cmRASReason		reason);

int GKHandleLRQTimeout(
		IN	HAPPRAS 		haRas,
		IN	HRAS			hsRas);

int GKHandleLRQ(
		IN	HRAS			hsRas,
		IN	HCALL			hsCall,
		OUT	LPHAPPRAS		lphaRas,
		IN	cmTransportAddress	*srcAddress,
		IN	HAPPCALL		haCall);

int LRQSetParamsFromHandle(LRQHandle *handle, HRAS hsRas);
int GkHandleLRQFailure(
		IN      HAPPRAS                 haRas,
		IN      HRAS                    hsRas,
		IN      cmRASReason             reason);
int LRQSetParamsFromHandle(LRQHandle *handle, HRAS hsRas);
int GkSendLRQ(PhoNode *rfphonodep, ARQHandle *arqHandle2, void *data);

#endif /* _lrq_h_ */
