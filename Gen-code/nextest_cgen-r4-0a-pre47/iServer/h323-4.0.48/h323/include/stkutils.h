#ifdef __cplusplus
extern "C" {
#endif



/*

NOTICE:
This document contains information that is proprietary to RADVISION LTD.
No part of this publication may be reproduced in any form whatsoever without
written prior approval by RADVISION LTD.

RADVISION LTD. reserves the right to revise this publication and make changes
without obligation to notify any person of such revisions or changes.

*/

/****************************************************************************

  stkutils.h  --  Stack Utility Interface

  Module Author:  Oz Solomonovich
  This Comment:   20-Jul-1997

****************************************************************************/


#ifndef __STKUTILS_H
#define __STKUTILS_H



#include <rvcommon.h>


                        /* == OID Manipulation == */

typedef enum
{
    OID_NumberForm,             /* e.g.:  0 2 3         */
    OID_NameForm,               /* e.g.:  itu-t 2 3     */
    OID_NameAndNumberForm       /* e.g.:  itu-t(0) 2 3  */
} OID_form;


/*=========================================================================**
**  == utlEncodeOID() ==                                                   **
**                                                                         **
**  Converts an OID string into a stack internal representation.           **
**                                                                         **
**  PARAMETERS:                                                            **
**      oidSize         The length, in bytes, of the target OID buffer.    **
**                                                                         **
**      oid             A pointer to the target OID buffer, or NULL.       **
**                                                                         **
**      buff            A pointer to the string to encode.                 **
**                                                                         **
**  RETURNS:                                                               **
**      The length, in bytes, of the resulting target OID string, or a     **
**      negative integer error code.                                       **
**                                                                         **
**  Use this function to convert OID string into the representation used   **
**  by stack components, such as PVT.                                      **
**                                                                         **
**  In order to pre-calculate the size needed for the target buffer, you   **
**  can pass a NULL value in the <oid> parameter.  In this case, the       **
**  <oidSize> parameter is ignored, and function will not attempt to fill  **
**  out a target buffer.  The return value will indicate the needed        **
**  length.                                                                **
**                                                                         **
**=========================================================================*/

RVAPI
int RVCALLCONV utlEncodeOID(
            OUT   int           oidSize,
            OUT   char *        oid,
            IN    const char *  buff
            );


/*=========================================================================**
**  == utlDecodeOID() ==                                                   **
**                                                                         **
**  Converts a stack-internal OID representation into a readable string.   **
**                                                                         **
**  PARAMETERS:                                                            **
**      oidSize         The length, in bytes, of the encoded OID value.    **
**                                                                         **
**      oid             A pointer to the encoded OID value buffer.         **
**                                                                         **
**      buffSize        The length, in bytes, of the target buffer.        **
**                                                                         **
**      buff            A pointer to the target decoding buffer, or NULL.  **
**                                                                         **
**      f               The required for for the target parameter.         **
**                      (See remarks below).                               **
**                                                                         **
**  RETURNS:                                                               **
**      The length, in bytes, of the resulting decoded OID string (with-   **
**      out the ending ASCIIZ), or a negative integer error code.          **
**                                                                         **
**  Use this function to decode OID values returned by stack components    **
**  such as PVT.                                                           **
**                                                                         **
**  Three target format types are available, as expressed in the OID_form  **
**  enumeration:                                                           **
**  o OID_NameForm, such as "0 2 3"                                        **
**  o OID_NumberForm, such as "itu-t 2 3"                                  **
**  o OID_NameAndNumberForm, such as "itu-t(0) 2 3"                        **
**  Not all values can be decoded into name forms.                         **
**                                                                         **
**  In order to pre-calculate the size needed for the target buffer, you   **
**  can pass a NULL value in the <buff> parameter.  In this case, the      **
**  <buffSize> parameter is ignored, and function will not attempt to fill **
**  out a target buffer.  The return value will indicate the needed        **
**  length.                                                                **
**                                                                         **
**=========================================================================*/

RVAPI
int RVCALLCONV utlDecodeOID(
        IN   int            oidSize,
        IN   const char *   oid,
        OUT  int            buffSize,
        OUT  char *         buff,
            IN   OID_form       f
        );

/*=========================================================================**
**  == utlDecodeOIDInt() ==                                                **
**                                                                         **
**  Converts a BER OID representation into an array of components.         **
**                                                                         **
**  PARAMETERS:                                                            **
**      oidSize         The length, in bytes, of the encoded OID value.    **
**                                                                         **
**      oid             A pointer to the encoded OID value buffer.         **
**                                                                         **
**      buffSize        The length, in integers of the target buffe        **
**                                                                         **
**      buff            A pointer to the target decoding buffer, or NULL.  **
**                      The turget buffer is an array of integers          **
**                                                                         **
**                                                                         **
**  RETURNS:                                                               **
**      The length, in integers, of the decoded part of OID,               **
**      or a negative integer error code.                                  **
**                                                                         **
**  Use this function to decode OID values returned by stack components    **
**  such as PVT.                                                           **
**                                                                         **
**                                                                         **
**=========================================================================*/

RVAPI
int RVCALLCONV utlDecodeOIDInt(
        IN  int             oidSize,
        IN  char*           oid,
        IN  int             buffSize,
        IN  int *           buff);



                     /* == BMP String Manipulation == */

/*=========================================================================**
**  == utlChr2Bmp() ==                                                     **
**                                                                         **
**  Converts an ASCII string into a BMP string.                            **
**                                                                         **
**  PARAMETERS:                                                            **
**      str             The string to convert.                             **
**                                                                         **
**      bmpStr          The target BMP string buffer.                      **
**                                                                         **
**  RETURNS:                                                               **
**      The length in bytes of the BMP string, or a negative integer error **
**      code.                                                              **
**                                                                         **
**=========================================================================*/

RVAPI
int RVCALLCONV utlChr2Bmp(
            IN   const char *   str,
            OUT  BYTE *         bmpStr
            );


/*=========================================================================**
**  == utlChrn2Bmp() ==                                                    **
**                                                                         **
**  Converts an ASCII string into a BMP string, with size constraints.     **
**                                                                         **
**  PARAMETERS:                                                            **
**      str             The string to convert.                             **
**                                                                         **
**      maxStrLen       The maximum number of wide characters allowed in   **
**                      the target BMP string.                             **
**                      Note that 1 wide character == 2 bytes.             **
**                                                                         **
**      bmpStr          The target BMP string buffer.                      **
**                                                                         **
**  RETURNS:                                                               **
**      The length in bytes of the BMP string, or a negative integer error **
**      code.                                                              **
**                                                                         **
**=========================================================================*/

RVAPI
int RVCALLCONV utlChrn2Bmp(
            IN   const char *   str,
            IN   int            maxStrLen,
            OUT  BYTE *         bmpStr
            );


/*=========================================================================**
**  == utlBmp2Chr() ==                                                     **
**                                                                         **
**  Converts a BMP string into an ASCII representation.                    **
**                                                                         **
**  PARAMETERS:                                                            **
**      str             The target string buffer.                          **
**                                                                         **
**      bmpStr          The source BMP string.                             **
**                                                                         **
**      bmpLen          The length of the BMP string, in bytes.            **
**                                                                         **
**  RETURNS:                                                               **
**      Zero or a negative integer error code.                             **
**                                                                         **
**  This function is only capable of converting BMP strings that use only  **
**  characters of values 0-255.  This is done by stripping the high-order  **
**  byte.  BMP strings with characters of values >=256, have no ASCII      **
**  representation, and cannot be converted.                               **
**                                                                         **
**=========================================================================*/

RVAPI
int RVCALLCONV utlBmp2Chr(
            OUT  char *         str,
            IN   const BYTE *   bmpStr,
            IN   int            bmpLen
            );



#endif  /* __STKUTILS_H */
#ifdef __cplusplus
}
#endif



