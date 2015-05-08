
/*

NOTICE:
This document contains information that is proprietary to RADVISION LTD..
No part of this publication may be reproduced in any form whatsoever without
written prior approval by RADVISION LTD..

RADVISION LTD. reserves the right to revise this publication and make changes
without obligation to notify any person of such revisions or changes.

*/

#ifndef CATDB_H
#define CATDB_H

#ifdef __cplusplus
extern "C" {
#endif


#include <mei.h>
#include <cat.h>



/* Number of supported key types in CAT */
#define CAT_KEY_TYPES (6)




/************************************************************************
 * catKeyStruct
 * This struct holds the keys that we use for the hash table itself
 * The information stored here is minimal - we want to make sure that
 * the hash table is kept small.
 * keyValue - The key enumeration we're dealing with
 *            This is the enumeration value from keyTypes[] array of the
 *            catModule struct.
 * key      - The actual key information
 ************************************************************************/
typedef struct
{
    UINT32      keyValue;
    catStruct*  key;
} catKeyStruct;




/************************************************************************
 * catDataStruct
 * This struct holds the information associated with a call inside CAT.
 * It holds the handle of the call and the IRR transactions for that
 * call.
 * key              - Keys information used to find the call
 * hashValues       - Hash locations of all keys handled by this call
 *
 * hsCall           - Call handle for the call
 * unsolicitedIRR   - Handle to an unsolicited IRR message for this call
 *                    This handle is created by cmRASDummyRequest()
 *                    for backward compatibility. It is recommended
 *                    not to use such calls.
 ************************************************************************/
typedef struct
{
    catStruct       key;
    void*           hashValues[5];

    /* User related information */
    HCALL           hsCall;
    HRAS            unsolicitedIRR;
} catDataStruct;




/************************************************************************
 * catModule
 * This struct holds the CAT instance information.
 * log              - Log instance used for logging messages
 * keyTypes         - Key types supported by this CAT instance (we've got
 *                    different types for CID association or srcAddress
 *                    association on Version 1 incoming calls.
 * lock             - Mutex to use
 * hash             - Hash table used by CAT
 * calls            - RA holding call information and the call's keys
 * numSimultKeys    - Number of simulataneous keys
 * compare15bitCrv  - Use 15bit comparison on CRV instead of 16bit
 ************************************************************************/
typedef struct
{
    RVHLOG  log;
    UINT32  keyTypes[CAT_KEY_TYPES];
    HMEI    lock;
    HHASH   hash;
    HRA     calls;
    int     numSimultKeys;
    BOOL    compare15bitCrv;
} catModule;



#ifdef __cplusplus
}
#endif

#endif  /* CATDB_H */

