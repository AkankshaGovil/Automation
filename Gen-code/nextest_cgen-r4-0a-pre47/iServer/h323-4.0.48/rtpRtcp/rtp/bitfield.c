#ifdef __cplusplus
extern "C" {
#endif



/*

NOTICE:
This document contains information that is proprietary to RADVISION LTD.
No part of this publication may be reproduced in any form whatsoever without
written prior approval by RADVISION LTD..

RADVISION LTD. reserves the right to revise this publication and make changes
without obligation to notify any person of such revisions or changes.

*/


/****************************************************************************

  bitfield.c  --  Bit-field manipulation

  This Comment:  10-Mar-1997

  Abstract:      Misc. routines for bit field manipulation.

  Platforms:     All.

  Known Bugs:    None.

****************************************************************************/


#include "bitfield.h"


/*===========================================================================
**  == bitfieldSet() ==                                                    **
**                                                                         **
**  Sets a sequence of bits in an integer to a specified value.            **
**                                                                         **
**  PARAMETERS:                                                            **
**      value      The integer value to work on.                           **
**                                                                         **
**      bitfield   The bit sequence to insert into the integer.            **
**                                                                         **
**      nStartBit  This value specifies the list-segnificant bit index     **
**                 for the bit sequence.  e.g. if nStartBit is set to 1,   **
**                 and nBits is set to 3, the bit sequence will occupy     **
**                 bits 1 through 4.                                       **
**                                                                         **
**      nBits      The number of bits in the bit sequence.                 **
**                                                                         **
**  RETURNS:                                                               **
**      The new integer value.                                             **
**                                                                         **
**=========================================================================*/

UINT32 bitfieldSet(
    IN  UINT32  value,
    IN  UINT32  bitfield,
    IN  int     nStartBit,
    IN  int     nBits)
{
    int mask = (1 << nBits) - 1;

    return (value & ~(mask << nStartBit)) +
           ((bitfield & mask) << nStartBit);
}

/*===========================================================================
**  == bitfieldGet() ==                                                    **
**                                                                         **
**  Retreives the sequence of bits in an integer.                          **
**                                                                         **
**  PARAMETERS:                                                            **
**      value      The integer value to work on.                           **
**                                                                         **
**      nStartBit  This value specifies the list-segnificant bit index     **
**                 of the bit sequence.  e.g. if nStartBit is set to 1,    **
**                 and nBits is set to 3, the bit sequence in bits 1       **
**                 through 4 is used.                                      **
**                                                                         **
**      nBits      The number of bits in the bit sequence.                 **
**                                                                         **
**  RETURNS:                                                               **
**      Returns the bit sequence.  The sequence will occupy bits           **
**      0..(Bits-1).                                                       **
**                                                                         **
**=========================================================================*/

UINT32 bitfieldGet(
    IN  UINT32  value,
    IN  int     nStartBit,
    IN  int     nBits)
{
    int mask = (1 << nBits) - 1;

    return (value >> nStartBit) & mask;
}
#ifdef __cplusplus
}
#endif



