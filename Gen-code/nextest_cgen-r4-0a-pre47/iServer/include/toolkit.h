/*
 * This cryptographic library is offered under the following license:
 * 
 *      "Cylink Corporation, through its wholly owned subsidiary Caro-Kann
 * Corporation, holds exclusive sublicensing rights to the following U.S.
 * patents owned by the Leland Stanford Junior University:
 *  
 *         Cryptographic Apparatus and Method
 *         ("Hellman-Diffie") .................................. No. 
4,200,770
 *  
 *         Public Key Cryptographic Apparatus
 *         and Method ("Hellman-Merkle") .................. No. 4,218, 582
 *  
 *         In order to promote the widespread use of these inventions from
 * Stanford University and adoption of the ISAKMP reference by the IETF
 * community, Cisco has acquired the right under its sublicense from Cylink 
to
 * offer the ISAKMP reference implementation to all third parties on a 
royalty
 * free basis.  This royalty free privilege is limited to use of the ISAKMP
 * reference implementation in accordance with proposed, pending or 
approved
 * IETF standards, and applies only when used with Diffie-Hellman key
 * exchange, the Digital Signature Standard, or any other public key
 * techniques which are publicly available for commercial use on a royalty
 * free basis.  Any use of the ISAKMP reference implementation which does 
not
 * satisfy these conditions and incorporates the practice of public key may
 * require a separate patent license to the Stanford Patents which must be
 * negotiated with Cylink's subsidiary, Caro-Kann Corporation."
 * 
 * Disclaimer of All Warranties  THIS SOFTWARE IS BEING PROVIDED "AS IS", 
 * WITHOUT ANY EXPRESS OR IMPLIED WARRANTY OF ANY KIND WHATSOEVER.
 * IN PARTICULAR, WITHOUT LIMITATION ON THE GENERALITY OF THE FOREGOING,  
 * CYLINK MAKES NO REPRESENTATION OR WARRANTY OF ANY KIND CONCERNING 
 * THE MERCHANTABILITY OF THIS SOFTWARE OR ITS FITNESS FOR ANY PARTICULAR
 * PURPOSE.
 *
 * Cylink or its representatives shall not be liable for tort, indirect, 
 * special or consequential damages such as loss of profits or loss of 
goodwill 
 * from the use or inability to use the software for any purpose or for any 
 * reason whatsoever.
 *
 *******************************************************************
 * This cryptographic library incorporates the BigNum multiprecision 
 * integer math library by Colin Plumb.
 *
 * BigNum has been licensed by Cylink Corporation 
 * from Philip Zimmermann.
 *
 * For BigNum licensing information, please contact Philip Zimmermann
 * (prz@acm.org, +1 303 541-0140). 
 *******************************************************************
 */
/****************************************************************************
*  FILENAME:  toolkit.h       PRODUCT NAME: CRYPTOGRAPHIC TOOLKIT
*
*  FILE STATUS:
*
*  DESCRIPTION:     Cryptographic Toolkit Functions Header File
*
*  USAGE:            File should be included to use Toolkit Functions
*
*
*         Copyright (c) Cylink Corporation 1994. All rights reserved.
*
*  REVISION  HISTORY:
*
*  23 Aug 94  KPZ     Initial release
*  24 Sep 94    KPZ Added prototypes of Toolkit functions
*  14 Oct 94    GKL Second version (big endian support)
*  08 Dec 94    GKL Added YIELD_context to GenDSSParameters
*
****************************************************************************/

#ifndef TOOLKIT_H     /* Prevent multiple inclusions of same header file */
#define TOOLKIT_H


/* Error types */

#define SUCCESS       0      /* no errors */
#define ERR_DATA -1      /* generic data error */
#define ERR_ALLOC       -2      /* insufficient memory */
#define ERR_INPUT_LEN  -3      /* invalid length for input data (zero bytes) */
#define ERR_DSS_LEN     -4      /* invalid length for dss_p */
#define ERR_DH_LEN        -5      /* invalid length for DH_modulus */
#define ERR_BLOCK_LEN     -7      /* invalid length for input block for ECB/CBC */
#define ERR_HASH_LEN    -8      /* invalid length for hash_result */
#define ERR_MODE    -9      /* invalid value of encryption mode */
#define ERR_NUMBER        -10     /* invalid number of testings (zero) */
#define ERR_POSITION     -11     /* invalid value of  triplet_position   */
#define ERR_COUNT     -12     /* invalid iteration count (zero) */
#define ERR_SIGNATURE       -21     /* signature is not valid */
#define ERR_PRIME       -22     /* number is not prime */
#define ERR_WEAK        -23     /* weak key */
#define ERR_INPUT_VALUE -24     /* invalid input value */
#define ERR_CANCEL      -30     /* canceled by user */
#define ERR_MODULUS_ZERO -31    /* invalid modulo */
#define ERR_UNSUPPORTED  -40     /* unsupported crypto method */


/* Lengths of variables */
#define DH_LENGTH_MIN    64  /* 512-bit minimal length for DH functions */
#define DSS_LENGTH_MIN   64  /* 512-bit minimal length for DSS functions */
#define DSS_LENGTH_MAX  128  /* 1024-bit maximal length for DSS functions */
#define SHA_LENGTH       20  /* 160-bit length for SHA hash result */
#define SHABLOCKLEN	 64

/* Number of random bases for Miller test */
#define TEST_COUNT        40

/* Encryption modes */
/* Encryption modes */
#define ECB_MODE	0  /* Elecronic CodeBook */
#define CBC_MODE	1  /* Cipher Block Chaining */
#define CFB1_MODE	2  /* 1-bit Cipher FeedBack */
#define CFB8_MODE	3  /* 8-bit Cipher FeedBack */
#define CFB64_MODE	4  /* 64-bit Cipher FeedBack */
#define OFB1_MODE	5  /* 1-bit Output FeedBack */
#define OFB8_MODE	6  /* 8-bit Output FeedBack */
#define OFB64_MODE	7  /* 64-bit Output FeedBack */

/* Crypto methods */
#define ONE_TO_ONE	0x00
#define DES40_CFB8  0x01
#define DES40_CFB64 0x02
#define DES40_CBC64 0x03
#define DES40_ECB   0x04
#define TDES_CFB8   0x07
#define DES56_CFB8  0x0A
#define DES56_CFB64 0x0B
#define DES56_CBC64 0x0C
#define DES56_ECB   0x0D


/****************************************************************************
*  INCLUDE FILES
****************************************************************************/

/* system files */
#include "cylink.h"

/* callback function */
#ifdef VXD
typedef int (* YIELD_PROC)( void );
#else
typedef int (* YIELD_PROC)(int ); /*TKL00601*/
#endif

typedef struct {                   /*TKL00601*/
       YIELD_PROC yield_proc;
  void *     handle;         /* Application specific information */
}YIELD_context;


/* Secure Hash Algorithm structure */
typedef struct
{
    ulong state[ 5 ];      /* state */
      ulong count[ 2 ];          /* number of bits */
 uchar buffer[ 64 ];     /* input buffer */
} SHA_context;

typedef struct {
    ULONG Numbytes;
    ULONG Numblocks[2];  /* each block contains 64 bytes */
    ULONG Mblock[16];
    ULONG buffer[5];
} SHA_CTX;

#ifdef  __cplusplus
extern  "C" {
#endif
/* Copy Cylink DSS Common Parameters */    /*TKL01201*/
   int GetDSSPQG(ushort dss_p_bytes,
                 uchar  *dss_p,
          uchar  *dss_q,
          uchar  *dss_g);

/* Compute a Secure Hash Function */
 int SHA( uchar   *message, ushort message_bytes,
                           uchar  *hash_result );
/* Initialize Secure Hash Function */
 void SHAInit( SHA_context *hash_context );

/* Update Secure Hash Function */
 int SHAUpdate( SHA_context *hash_context,
                uchar        *message,
                ushort      message_bytes );
/* Finalize Secure Hash Function */
 int SHAFinal( SHA_context *hash_context,
               uchar       *hash_result );

/* do SHA according to the FIPS */

int fipsSHAInit(SHA_CTX *);
int fipsSHAUpdate(SHA_CTX *, BYTE *, int);
int fipsSHAFinal(SHA_CTX *, BYTE *);

/* Compute a DSS Signature */
 int GenDSSSignature( ushort dss_p_bytes, uchar  *dss_p,
                uchar  *dss_q,      uchar  *dss_g,
                      uchar  *dss_x,      uchar  *dss_k,
                      uchar  *r,          uchar  *s,
                      uchar  *hash_result );
/* Verify a DSS Signature */
 int VerDSSSignature( ushort dss_p_bytes, uchar  *dss_p,
                      uchar  *dss_q,      uchar  *dss_g,
                      uchar  *dss_y,      uchar  *r,
                      uchar  *s,          uchar  *hash_result);
/* Initialize Random number Generator */
 int InitRand( ushort SEED_bytes, uchar  *SEED,
                                  uchar  *RVAL );
/* Generate random number */
 int GenRand( ushort A_bytes, uchar  *A,
                           uchar  *RVAL );
/* Compute DSS public/secret number pair */
 int GenDSSKey( ushort dss_p_bytes, uchar  *dss_p,
                uchar  *dss_q,      uchar  *dss_g,
                uchar  *dss_x,      uchar  *dss_y,
                                    uchar  *XKEY );
/* Generate secret number */
 int GenDSSNumber( uchar *dss_k, uchar *dss_q,
                                                              uchar *KKEY );

/* Compute a Diffie-Hellman Shared number */
 int GetDHSharedNumber( ushort DH_modulus_bytes, uchar  *DH_secret,
                        uchar  *DH_public,       uchar  *DH_shared,
                        uchar  *DH_modulus );
/* Set Key by Diffie_Hellman shared number */
 int SetDESKAPPAKey( ushort DH_modulus_bytes, uchar  *DH_shared,
                     uchar  *K );
/* Expand KAPPA key */
 void KAPPAKeyExpand( uchar *K, uchar *K1 );

/* Expand DES key */
 void DESKeyExpand( uchar *key, uchar *K1 );

/* Encrypt a block of data with single KAPPA */
 int KAPPAEncrypt( uchar  *kappa_iv,     uchar  *kappa_key,
                   ushort kappa_mode,    uchar  *input_array,
                   uchar  *output_array, ushort input_array_bytes );
/* Encrypt a block of data with single DES */
 int DESEncrypt( uchar  *des_iv,       uchar  *des_key,
                 ushort des_mode,      uchar  *input_array,
                 uchar  *output_array, ushort input_array_bytes );

/* Decrypt a block of data with single KAPPA */
 int KAPPADecrypt( uchar  *kappa_iv,   uchar  *kappa_key,
                   ushort kappa_mode,  uchar  *data_array,
                                       ushort data_array_bytes );

/* Decrypt a block of data with single DES */
 int DESDecrypt( uchar  *des_iv,  uchar  *des_key,
                 ushort des_mode, uchar  *data_array,
                                 ushort data_array_bytes );

/* One-Time-Pad Signature with a Diffie-Hellman shared number */
 int DHOneTimePad( ushort DH_modulus_bytes, uchar  *DH_shared,
                   uchar  *X,               uchar  *Y );

/* Compute Diffie-Hellman keys */
 int GetDHKey( ushort DH_modulus_bytes, uchar *DH_secret,
               uchar *DH_public,        ushort key1_bytes,
               uchar *key1,             ushort key2_bytes,
               uchar *key2,                     uchar *DH_modulus );

/* Compute a Diffie-Hellman pair */
int GenDHPair( ushort DH_modulus_bytes, uchar  *DH_secret,
               uchar  *DH_public,       uchar  *DH_base,
               uchar  *DH_modulus,      uchar  *RVAL );

/* Do Diffie-Hellman MSB-1st (the ones above do it LSB-1st) */

int genDHParameters (ushort, uchar *, uchar *, uchar *, uchar *, uchar *);
int genDHSharedSecret (ushort, uchar *, uchar *, uchar *, uchar *);

 int GetPasswordKeySHA( ushort Password_bytes, uchar  *Password,
                                               uchar  *salt,          ushort Count,
                                            uchar  *K,             uchar  *IV );

/* Generate DSS Common Parameters */
 int GenDSSParameters( ushort dss_p_bytes, uchar  *dss_p,
                                          uchar  *dss_q,      uchar  *dss_g,
                                      uchar  *RVAL, YIELD_context *yield_cont ); /*TKL00701*/

/* Produce a Shamir Key-Sharing Triplet for Secret Number */
int GenShamirTriplet( ushort SecretNumber_bytes, uchar *SecretNumber,
                                       uchar *first_value,        uchar *second_value,
                                         uchar *third_value,        uchar *RVAL );

/* Reconstract a Secret Number from Shamir Key-Sharing Duplex */
int GetNumberShamirDuplex( ushort SecretNumber_bytes,
                              uchar  *value_A,
                                ushort A_position,                                                      uchar  *value_B,
                                ushort B_position,
                              uchar  *SecretNumber );
int SFDHEncrypt( ushort DH_modulus_bytes,
                                          uchar  *DH_modulus,
                                     uchar  *DH_base,
                                        uchar  *DH_public,
                                      uchar  *DH_random_public,
                                       uchar  *DH_shared,
                                      uchar  *RVAL );
int SFDHDecrypt( ushort DH_modulus_bytes,
                                        uchar  *DH_modulus,
                                     uchar  *DH_secret,
                                      uchar  *DH_random_public,
                                       uchar  *DH_shared );
/* Check DES key weakness */
int CheckDESKeyWeakness( uchar *key );

int SetCipherKey( ushort DH_shared_bytes,
				  uchar  *DH_shared,
				 uchar  *Key,
				 ushort cryptoMethod );

#ifdef  __cplusplus
}
#endif


#endif /* TOOLKIT_H */

