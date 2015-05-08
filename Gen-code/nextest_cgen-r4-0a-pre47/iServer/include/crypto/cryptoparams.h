#ifndef _cryptoparams_h_
#define _cryptoparams_h_

/***********************************************************
 * DH Parameters: p and g
 * 
 ***********************************************************/
typedef struct DHParams
{
	unsigned char *p;	/* prime */
	unsigned short plen;	/* prime len */
	unsigned char *g;	/* prime */
	unsigned short glen;		/* generator */
} DHParams;

/***********************************************************
 * DSA Parameters: p, q and g
 *
 ***********************************************************/
typedef struct DSAParms
{
	unsigned char *p;
	unsigned short plen;	/* prime len */
	unsigned char *q;
	unsigned short qlen;	/* prime len */
	unsigned char *g;
	unsigned short glen;	/* prime len */
} DSAParams;

extern DSAParams Netoid500DSAParams[];
extern DHParams Netoid500DHParams[];

#define N500SecParams 0

/* #define N500DH_SetParams(dh) DH_SetParams(&Netoid500DHParams[N500SecParams], dh) */
#define N500DH_SetParams(dh) DH_SetParams(&Netoid500DHParams[1], dh)
#define N500DSA_SetParams(dsa) DSA_SetParams(&Netoid500DSAParams[N500SecParams], dsa)

int DH_SetParams(DHParams *dhp, DH *dh);

int DSA_SetParams(DSAParams *dsap, DSA *dsa);

#endif /* _cryptoparams_h_ */
