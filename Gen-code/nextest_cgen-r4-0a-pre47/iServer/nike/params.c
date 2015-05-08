/* 
 * Netoid 500 Encryption Parameters.
 * The DH params are from the IKE Rfc, 
 * and the DSA params are from FIPS 180-1
 * example. The hardness and impact of these is
 * unknown.
 */
#include <stdio.h>
#include "dh.h"
#include "dsa.h"
#include "bn.h"
#include "cryptoparams.h"
/* 
 * For now, just take some example parameters from here and
 * there ;-), and we will hardcode them into the aloid
 * and netoid.
 */

static unsigned char dh_def_p[96] =
{ 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
  0xc9, 0x0f, 0xda, 0xa2, 0x21, 0x68, 0xc2, 0x34,
  0xc4, 0xc6, 0x62, 0x8b, 0x80, 0xdc, 0x1c, 0xd1,
  0x29, 0x02, 0x4e, 0x08, 0x8a, 0x67, 0xcc, 0x74,
  0x02, 0x0b, 0xbe, 0xa6, 0x3b, 0x13, 0x9b, 0x22,
  0x51, 0x4a, 0x08, 0x79, 0x8e, 0x34, 0x04, 0xdd,
  0xef, 0x95, 0x19, 0xb3, 0xcd, 0x3a, 0x43, 0x1b,
  0x30, 0x2b, 0x0a, 0x6d, 0xf2, 0x5f, 0x14, 0x37,
  0x4f, 0xe1, 0x35, 0x6d, 0x6d, 0x51, 0xc2, 0x45,
  0xe4, 0x85, 0xb5, 0x76, 0x62, 0x5e, 0x7e, 0xc6,
  0xf4, 0x4c, 0x42, 0xe9, 0xa6, 0x3a, 0x36, 0x20,
  0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff
};

static unsigned char dh_test_p[64] =
{ 0x97, 0x9B, 0x41, 0xF1, 0x62, 0x65, 0x45, 0x91, 
  0xE5, 0x46, 0x9C, 0x72, 0x2D, 0x2E, 0x7D, 0xEE, 
  0xA4, 0xB7, 0xFB, 0x77, 0x84, 0x23, 0x2E, 0xFE, 
  0xE7, 0x45, 0xE6, 0xE5, 0xD7, 0xC5, 0x27, 0xF1, 
  0x37, 0x30, 0x7D, 0xEF, 0x30, 0x38, 0x10, 0xBD, 
  0x1B, 0x53, 0x67, 0x4B, 0x8E, 0xF8, 0xEB, 0xDC, 
  0x7C, 0x40, 0xCB, 0x54, 0x78, 0x5B, 0x05, 0x11, 
  0xB9, 0x1C, 0x9E, 0x25, 0x36, 0x1D, 0x85, 0x73 
};

static unsigned char dh_def_g[96] =
{ 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x02
};


DHParams Netoid500DHParams[] = {
     {dh_def_p, 96, dh_def_g, 96},
     {dh_test_p, 64, dh_def_g, 96}
};

static unsigned char dsa_def_p[64] = 
{ 
     0x8d, 0xf2, 0xa4, 0x94, 0x49, 0x22, 0x76, 0xaa, 
     0x3d, 0x25, 0x75, 0x9b, 0xb0, 0x68, 0x69, 0xcb,
     0xea, 0xc0, 0xd8, 0x3a, 0xfb, 0x8d, 0x0c, 0xf7,
     0xcb, 0xb8, 0x32, 0x4f, 0x0d, 0x78, 0x82, 0xe5,
     0xd0, 0x76, 0x2f, 0xc5, 0xb7, 0x21, 0x0e, 0xaf,
     0xc2, 0xe9, 0xad, 0xac, 0x32, 0xab, 0x7a, 0xac,
     0x49, 0x69, 0x3d, 0xfb, 0xf8, 0x37, 0x24, 0xc2,
     0xec, 0x07, 0x36, 0xee, 0x31, 0xc8, 0x02, 0x91
};
 
static unsigned char dsa_def_q[20] =    
{
     0xc7, 0x73, 0x21, 0x8c, 0x73, 0x7e, 0xc8, 0xee, 
     0x99, 0x3b, 0x4f, 0x2d, 0xed, 0x30, 0xf4, 0x8e,
     0xda, 0xce, 0x91, 0x5f
};

static unsigned char dsa_def_g[64] =
{
     0x62, 0x6d, 0x02, 0x78, 0x39, 0xea, 0x0a, 0x13,
     0x41, 0x31, 0x63, 0xa5, 0x5b, 0x4c, 0xb5, 0x00,
     0x29, 0x9d, 0x55, 0x22, 0x95, 0x6c, 0xef, 0xcb,
     0x3b, 0xff, 0x10, 0xf3, 0x99, 0xce, 0x2c, 0x2e,
     0x71, 0xcb, 0x9d, 0xe5, 0xfa, 0x24, 0xba, 0xbf,
     0x58, 0xe5, 0xb7, 0x95, 0x21, 0x92, 0x5c, 0x9c,
     0xc4, 0x2e, 0x9f, 0x6f, 0x46, 0x4b, 0x08, 0x8c,
     0xc5, 0x72, 0xaf, 0x53, 0xe6, 0xd7, 0x88, 0x02
};

DSAParams Netoid500DSAParams[] = {
     dsa_def_p, 64, dsa_def_q, 20, dsa_def_g, 64
};

/*
 * Convert the parameters specified by the dhp
 * argument, and fill up the already allocated
 * dh structure with them
 */
int
DH_SetParams(DHParams *dhp, DH *dh)
{
     dh->p = BN_new();
     dh->g = BN_new();

     BN_bin2bn(dhp->p, dhp->plen, dh->p);
     BN_bin2bn(dhp->g, dhp->glen, dh->g);

#if 0
     /* print whatever we converted */
     printf("p %s\n", BN_bn2hex(dh->p));
     printf("\n");
     printf("g %s\n", BN_bn2hex(dh->g));
#endif /* DBG */

     return 1;
}

/*
 * Convert the parameters specified by the dsap
 * argument, and fill up the already allocated
 * dsa structure with them
 */
int
DSA_SetParams(DSAParams *dsap, DSA *dsa)
{
     dsa->p = BN_new();
     dsa->q = BN_new();
     dsa->g = BN_new();

     BN_bin2bn(dsap->p, dsap->plen, dsa->p);
     BN_bin2bn(dsap->q, dsap->qlen, dsa->q);
     BN_bin2bn(dsap->g, dsap->glen, dsa->g);

     return 1;
}

#ifdef TEST
int
main()
{
     DH *dh;

     dh = DH_new();
     
     DH_SetParams(&Netoid500DHParams[0], dh);
}
#endif /* TEST */