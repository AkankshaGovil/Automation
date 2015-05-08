
/*

NOTICE:
This document contains information that is proprietary to RADVISION LTD..
No part of this publication may be reproduced in any form whatsoever without
written prior approval by RADVISION LTD..

RADVISION LTD. reserves the right to revise this publication and make changes
without obligation to notify any person of such revisions or changes.

*/

#ifdef __cplusplus
extern "C" {
#endif


#include <ra.h>
#include <hash.h>
#include <catdb.h>
#include <cat.h>


/************************************************************************
 *
 *                              Private constants
 *
 ************************************************************************/

/* Additional number of keys for call - we use this value to make sure
   the hash table is bigger than the number of elements by a reasonable
   size */
#define CAT_ADDITIONAL_KEYS (3)




/************************************************************************
 *
 *                              Private functions
 *
 ************************************************************************/


/* todo: move to somewhere else */
/************************************************************************
 * addrcmp
 * purpose: Compare between 2 transport addresses
 * input  : addr1   - First address
 *          addr2   - Second address
 * output : none
 * return : 0 if both addresses are the same one
 *          Non-zero value othetwise
 ************************************************************************/
int addrcmp(IN cmTransportAddress* addr1, IN cmTransportAddress* addr2)
{
    if ((addr1->ip == addr2->ip) &&
        (addr1->port == addr2->port))
        return 0; /* Match */

    return 1; /* Doesn't match */
}


/************************************************************************
 * addrhash
 * purpose: Calculate a hash value for an address. This function can
 *          be used as part of a hash function, but not as the hash
 *          function itself.
 * input  : addr    - Address to hash
 *          hashKey - Starting hash value
 * output : none
 * return : Calculated hash value
 ************************************************************************/
int addrhash(
    IN cmTransportAddress*  addr,
    IN UINT32               hashKey)
{
    /* We hash each address type differently */
    switch (addr->type)
    {
        case cmTransportTypeIPStrictRoute:
        case cmTransportTypeIPLooseRoute:
        case cmTransportTypeNSAP:
            /* No hashing for these guys - they're not really supported... */
            break;

        case cmTransportTypeIP:
        default:
            /* Hash by IP and PORT fields */
            hashKey = hashKey << 1 ^ addr->ip;
            hashKey = hashKey << 1 ^ addr->port;
            break;
      }

    return hashKey;
}


/************************************************************************
 * catHashKey
 * purpose: CAT key hashing function
 *          It uses different hashing method for different keys
 * input  : param       - Parameter we're going to hash (catKeyStruct)
 *          paramSize   - Size of the parameter
 *          hashSize    - Size of the hash table itself
 * output : none
 * return : Hash value to use
 ************************************************************************/
UINT32 catHashKey(
    IN void *param,
    IN int paramSize,
    IN int hashSize)
{
    catStruct*  key;
    UINT32      hashKey = 0;
    UINT32      bytes, keyValue;
    BYTE*       ptr;

    if (paramSize);
    key = ((catKeyStruct *)param)->key;
    keyValue = ((catKeyStruct *)param)->keyValue;

    /* We take into consideration only the fields that are set in the keyValue */
    if ((keyValue & catCallId) != 0)
    {
        bytes = sizeof(key->callID);
        ptr = (BYTE*)key->callID;
        while (bytes-- > 0) hashKey = hashKey << 1 ^ *ptr++;
    }
    if ((keyValue & catAnswerCall) != 0)
    {
        hashKey = hashKey << 1 ^ (key->answerCall);
    }
    if ((keyValue & catCRV) != 0)
    {
        bytes = sizeof(key->crv);
        ptr = (BYTE*)&key->crv;
        while (bytes-- > 0) hashKey = hashKey << 1 ^ *ptr++;
    }
    if ((keyValue & catRasCRV) != 0)
    {
        bytes = sizeof(key->rasCRV);
        ptr = (BYTE*)&key->rasCRV;
        while (bytes-- > 0) hashKey = hashKey << 1 ^ *ptr++;
    }
    if ((keyValue & catDestCallSignalAddr) != 0)
    {
        hashKey = addrhash(&key->destCallSignalAddr, hashKey);
    }
    if ((keyValue & catRasSrcAddr) != 0)
    {
        hashKey = addrhash(&key->rasSrcAddr, hashKey);
    }
    if ((keyValue & catSrcCallSignalAddr) != 0)
    {
        hashKey = addrhash(&key->srcCallSignalAddr, hashKey);
    }
    if ((keyValue & catCID) != 0)
    {
        bytes = sizeof(key->cid);
        ptr = (BYTE*)key->cid;
        while (bytes-- > 0) hashKey = hashKey << 1 ^ *ptr++;
    }

    return (hashKey % hashSize);
}


/************************************************************************
 * catCompareKey
 * purpose: Comparison function for 2 key structs in CAT
 * input  : givenKey    - The key given by the user (catKeyStruct type)
 *                        The keyValue field of this struct is discarded
 *                        in the comparison process.
 *          storedKey   - Key inside hash table (catKeyStruct type)
 *          keySize     - Size of each key
 * return : TRUE if elements are the same
 *          FALSE otherwise
 ************************************************************************/
BOOL catCompareKey(IN void* givenKey, IN void* storedKey, IN UINT32 keySize)
{
    /* We check the fields one by one, looking for each flag that exist in both keys */
    catStruct*  searchFor;
    catStruct*  trueKey;
    UINT32      keyValue;

    if (keySize);

    searchFor = ((catKeyStruct *)givenKey)->key;
    trueKey = ((catKeyStruct *)storedKey)->key;
    keyValue = ((catKeyStruct *)storedKey)->keyValue;

    /* First let's make sure we're dealing with the same type of key from
       the different types of keys stored inside CAT */
    if ((searchFor->flags & keyValue) == keyValue)
    {
        /* Check each of the key parts that are set in this struct */
        if ((searchFor->flags & trueKey->flags & catCallId) != 0) /* always search for callId. */
            if (memcmp(searchFor->callID, trueKey->callID, sizeof(trueKey->callID)) != 0)
                return FALSE; /* CallID doesn't match */
        if ((keyValue & catAnswerCall) != 0)
            if (searchFor->answerCall != trueKey->answerCall)
                return FALSE; /* answerCall doesn't match */
        if ((keyValue & catCRV) != 0)
            if (searchFor->crv != trueKey->crv)
                return FALSE; /* Q931CRV doesn't match */
        if ((keyValue & catRasCRV) != 0)
            if (searchFor->rasCRV != trueKey->rasCRV)
                return FALSE; /* RAS-CRV doesn't match */
        if ((keyValue & catDestCallSignalAddr) != 0)
            if (addrcmp(&searchFor->destCallSignalAddr, &trueKey->destCallSignalAddr) != 0)
                return FALSE; /* DestAddress doesn't match */
        if ((keyValue & catRasSrcAddr) != 0)
            if (addrcmp(&searchFor->rasSrcAddr, &trueKey->rasSrcAddr) != 0)
                return FALSE; /* RasSrcAddr doesn't match */
        if ((keyValue & catSrcCallSignalAddr) != 0)
            if (addrcmp(&searchFor->srcCallSignalAddr, &trueKey->srcCallSignalAddr) != 0)
                return FALSE; /* SrcAddress doesn't match */
        if ((keyValue & catCID) != 0)
            if (memcmp(searchFor->cid, trueKey->cid, sizeof(trueKey->cid)) != 0)
                return FALSE; /* CID doesn't match */
    }
    else
        return FALSE;

    /* If we're here it means that the elements match */
    return TRUE;
}



/************************************************************************
 *
 *                              Public functions
 *
 ************************************************************************/


/************************************************************************
 * catConstruct
 * purpose: Create a CAT instance
 * input  : numCalls        - Number of calls supported by CAT
 *          isGatekeeper    - Is this a gatekeeper or an endpoint
 *          cidAssociation  - TRUE if we want to associate calls by their CID
 *                            FALSE if we don't want to associate calls by their CID
 *          compare15bitCrv - Use 15bit comparison on CRV instead of 16bit
 *          hLogMgr         - Log manager to use for logging
 * output : none
 * return : Handle to CAT instance on success
 *          NULL on failure
 ************************************************************************/
RVHCAT catConstruct(
    IN UINT32       numCalls,
    IN BOOL         isGatekeeper,
    IN BOOL         compare15bitCrv,
    IN BOOL         cidAssociation,
    IN RVHLOGMGR    hLogMgr)
{
    catModule*  cat;

    /* Allocate the CAT instance */
    cat = (catModule *)malloc(sizeof(catModule));
    if (cat == NULL) return NULL;

    if (isGatekeeper)
        cat->numSimultKeys = 5;
    else
        cat->numSimultKeys = 4;

    cat->log = logRegister(hLogMgr, "CAT", "Call Association Table");
    logPrint(cat->log, RV_DEBUG,
             (cat->log, RV_DEBUG, "catConstruct: compare15bitCrv=%d", compare15bitCrv));
    cat->compare15bitCrv = compare15bitCrv;

    /* Set the keys we can handle */
    cat->keyTypes[0] = catCallId | catAnswerCall | catDestCallSignalAddr;
    cat->keyTypes[1] = catCallId | catCRV;
    if (cidAssociation == TRUE)
        cat->keyTypes[2] = catCID | catAnswerCall | catDestCallSignalAddr;
    else
        cat->keyTypes[2] = catSrcCallSignalAddr | catAnswerCall | catDestCallSignalAddr;
    cat->keyTypes[3] = catRasCRV | catRasSrcAddr;
    cat->keyTypes[4] = catCallId | catAnswerCall;
    cat->keyTypes[5] = catRasCRV | catCallId;


    /* Allocate the hash */
    cat->hash =
        hashConstruct((int)(numCalls * (cat->numSimultKeys + CAT_ADDITIONAL_KEYS)),
                      (int)(numCalls * cat->numSimultKeys),
                      catHashKey,
                      catCompareKey,
                      sizeof(catKeyStruct),
                      sizeof(catDataStruct*),
                      hLogMgr,
                      "CAT HASH");

    /* Allocate the calls */
    cat->calls =
        raConstruct(sizeof(catDataStruct), (int)numCalls, hLogMgr, "CAT CALLS");

    if ((cat->hash == NULL) || (cat->calls == NULL))
    {
        if (cat->hash != NULL) hashDestruct(cat->hash);
        if (cat->calls != NULL) raDestruct(cat->calls);
        return NULL;
    }

    /* Allocate a mutex */
    cat->lock = meiInit();

    return (RVHCAT)cat;
}


/************************************************************************
 * catDestruct
 * purpose: Delete a CAT instance
 * input  : hCat    - CAT instance handle
 * output : none
 * return : Non-negative value on success
 *          Negative value on failure
 ************************************************************************/
int catDestruct(IN RVHCAT hCat)
{
    catModule* cat;

    if (hCat == NULL) return RVERROR;
    cat = (catModule *)hCat;

    hashDestruct(cat->hash);
    raDestruct(cat->calls);
    meiEnd(cat->lock);
    free(cat);
    return 0;
}


/************************************************************************
 * catAdd
 * purpose: Add a new call into a CAT instance
 * input  : hCat    - CAT instance handle
 *          key     - Key structure to associate with the call
 *          hsCall  - Call handle to put
 * output : none
 * return : CAT call handle on success
 *          NULL on failure
 ************************************************************************/
RVHCATCALL catAdd(
    IN RVHCAT       hCat,
    IN catStruct*   key,
    IN HCALL        hsCall)
{
    catModule*      cat = (catModule *)hCat;
    catDataStruct*  call;
    catKeyStruct    hashKey;
    int             i, curKey;

    meiEnter(cat->lock);

    /* Allocate a call in the RA */
    if (raAddExt(cat->calls, (RAElement*)&call) < 0)
    {
        logPrint(cat->log, RV_ERROR,
                 (cat->log, RV_ERROR, "catAdd: Error adding a new call"));
        meiExit(cat->lock);
        return NULL;
    }

    logPrint(cat->log, RV_DEBUG,
             (cat->log, RV_DEBUG, "catAdd: flags=0x%x, hsCall=0x%x, cat=0x%x", key->flags, hsCall, call));

    if (cat->compare15bitCrv)
    {
        key->rasCRV &= 0x7fff;
    }

    /* Set the values we already know inside the CAT call data */
    memset(call, 0, sizeof(catDataStruct));
    memcpy(&call->key, key, sizeof(catStruct));
    call->hsCall = hsCall;
    hashKey.key = &call->key;

    /* Check to see which key combinations are set inside the key struct and update the hash
       table accordingly */
    curKey = 0;
    for (i = 0; i < CAT_KEY_TYPES; i++)
    {
        if ((key->flags & cat->keyTypes[i]) == cat->keyTypes[i])
        {
            /* We've got a new key to add - first let's make sure we're not out of bounds */
            if (curKey >= cat->numSimultKeys)
            {
                logPrint(cat->log, RV_WARN,
                         (cat->log, RV_WARN, "catAdd: Too many simultaneous keys for hsCall=0x%x", hsCall));
                break;
            }

            /* Create a HASH key out of the value and add it in */
            hashKey.keyValue = cat->keyTypes[i];
            call->hashValues[curKey] = hashAdd(cat->hash, &hashKey, &call, FALSE);
            if ( !(call->hashValues[curKey]) )
            {
                logPrint(cat->log, RV_ERROR,
                         (cat->log, RV_ERROR, "catAdd: Counldn't add new call to hash table (hsCall=0x%x, cat=0x%x)", hsCall, call));
            }
            else
                curKey++;
        }
    }

    meiExit(cat->lock);
    return (RVHCATCALL)call;
}


/************************************************************************
 * catDelete
 * purpose: Delete a call from a CAT instance
 * input  : hCat        - CAT instance handle
 *          hCatCall    - CAT call handle
 * output : none
 * return : Non-negative value on success
 *          Negative value on failure
 ************************************************************************/
int catDelete(
    IN RVHCAT       hCat,
    IN RVHCATCALL   hCatCall)
{
    catModule*      cat = (catModule *)hCat;
    catDataStruct*  call = (catDataStruct *)hCatCall;
    int i, status;

    if (hCatCall == NULL) return 0;
    meiEnter(cat->lock);

    logPrint(cat->log, RV_DEBUG,
             (cat->log, RV_DEBUG, "catDelete: cat=0x%x, hsCall=0x%x", call, call->hsCall));

    /* First we'll remove all hash elements */
    i = 0;
    while ((i < cat->numSimultKeys) && (call->hashValues[i] != NULL))
    {
        hashDelete(cat->hash, call->hashValues[i]);
        i++;
    }

    /* Then we remove the call from RA */
    status = raDeleteLocation(cat->calls, raGetByPtr(cat->calls, call));

    meiExit(cat->lock);
    return status;
}


/************************************************************************
 * catFind
 * purpose: Find a CAT call handle by a given key struct that can hold
 *          several different keys
 * input  : hCat        - CAT instance handle
 *          key         - Key structure to look for
 * return : CAT call handle on success
 *          NULL on failure or when a call wasn't found
 ************************************************************************/
RVHCATCALL catFind(
    IN  RVHCAT      hCat,
    IN  catStruct*  key)
{
    catModule*      cat = (catModule *)hCat;
    catKeyStruct    catKey;
    catDataStruct*  call;
    void*           location;
    UINT32          crv;
    int             i;

    catKey.key = key;
    call = NULL;

    /* See if we compare 16bit CRV - we'll have to xor the MSB */
    crv = key->rasCRV;
    if ((key->flags & catRasCRV) && (cat->compare15bitCrv))
        key->rasCRV &= 0x7fff;

    meiEnter(cat->lock);

    /* see if any of the possible keys represent a match */
    for (i = 0; i < CAT_KEY_TYPES; i++)
    {
        if ((key->flags & cat->keyTypes[i]) == cat->keyTypes[i])
        {
            catKey.keyValue = cat->keyTypes[i];
            location = hashFind(cat->hash, &catKey);

            if (location != NULL)
            {
                /* found a match */
                call = *((catDataStruct**)hashGetElement(cat->hash, location));
                break;
            }
        }

    }

    meiExit(cat->lock);

    /* Make sure we didn't mess up with the CRV value */
    key->rasCRV = crv;

    logPrint(cat->log, RV_DEBUG,
             (cat->log, RV_DEBUG, "catFind: cat=0x%x, from keys 0x%x", call, key->flags));

    return (RVHCATCALL)call;
}


/************************************************************************
 * catUpdate
 * purpose: Update a CAT call information with new keys
 * input  : hCat        - CAT instance handle
 *          hCatCall    - CAT call handle to update
 *          key         - Key structure with new information
 * output : none
 * return : Non-negative value on success
 *          Negative value on failure
 ************************************************************************/
int catUpdate(
    IN RVHCAT       hCat,
    IN RVHCATCALL   hCatCall,
    IN catStruct*   key)
{
    catModule*      cat = (catModule *)hCat;
    catDataStruct*  call = (catDataStruct *)hCatCall;
    catKeyStruct    hashKey;
    UINT32          oldFlags;
    int             i, curKey;

    logPrint(cat->log, RV_DEBUG,
             (cat->log, RV_DEBUG, "catUpdate: cat=0x%x, with 0x%x - adding 0x%x keys",
             call, call->key.flags, key->flags));

    if (cat->compare15bitCrv)
    {
        key->rasCRV &= 0x7fff;
    }

    meiEnter(cat->lock);

    /* See if there's any new information at all */
    if ((key->flags & call->key.flags) != key->flags)
    {
        /* We've got some updates in here... */

        /* Get all the new values that we need */
        if (((call->key.flags & catCallId) == 0) && ((key->flags & catCallId) != 0))
            memcpy(call->key.callID, key->callID, sizeof(key->callID));
        if (((call->key.flags & catAnswerCall) == 0) && ((key->flags & catAnswerCall) != 0))
            call->key.answerCall = key->answerCall;
        if (((call->key.flags & catCRV) == 0) && ((key->flags & catCRV) != 0))
            call->key.crv = key->crv;
        if (((call->key.flags & catRasCRV) == 0) && ((key->flags & catRasCRV) != 0))
            call->key.rasCRV = key->rasCRV;
        if (((call->key.flags & catDestCallSignalAddr) == 0) && ((key->flags & catDestCallSignalAddr) != 0))
            memcpy(&call->key.destCallSignalAddr, &key->destCallSignalAddr, sizeof(cmTransportAddress));
        if (((call->key.flags & catRasSrcAddr) == 0) && ((key->flags & catRasSrcAddr) != 0))
            memcpy(&call->key.rasSrcAddr, &key->rasSrcAddr, sizeof(cmTransportAddress));
        if (((call->key.flags & catSrcCallSignalAddr) == 0) && ((key->flags & catSrcCallSignalAddr) != 0))
            memcpy(&call->key.srcCallSignalAddr, &key->srcCallSignalAddr, sizeof(cmTransportAddress));
        if (((call->key.flags & catCID) == 0) && ((key->flags & catCID) != 0))
            memcpy(call->key.cid, key->cid, sizeof(key->cid));

        /* Make sure we also update the flags */
        oldFlags = call->key.flags;
        call->key.flags |= key->flags;

        /* Find out how many key combinations we currently have */
        curKey = 0;
        while ((curKey < cat->numSimultKeys) && (call->hashValues[curKey] != NULL)) curKey++;

        /* Insert the new available key combinations to the hash */
        for (i = 0; i < CAT_KEY_TYPES; i++)
        {
            if (((oldFlags & cat->keyTypes[i]) != cat->keyTypes[i]) &&
                ((key->flags & cat->keyTypes[i]) == cat->keyTypes[i]))
            {
                /* We've got a new key to add - first let's make sure we're not out of bounds */
                if (curKey >= cat->numSimultKeys)
                {
                    logPrint(cat->log, RV_WARN,
                             (cat->log, RV_WARN, "catUpdate: Too many simultaneous keys for cat=0x%x, hsCall=0x%x", call, call->hsCall));
                    break;
                }

                /* Create a HASH key out of the value and add it in */
                hashKey.key = &call->key;
                hashKey.keyValue = cat->keyTypes[i];
                call->hashValues[curKey] = hashAdd(cat->hash, &hashKey, &call, FALSE);
                if ( !(call->hashValues[curKey]) )
                {
                    logPrint(cat->log, RV_ERROR,
                             (cat->log, RV_ERROR, "catUpdate: Counldn't add new call to hash table (hsCall=0x%x, cat=0x%x)", call->hsCall, call));
                }
                else
                    curKey++;
            }
        }
    }

    meiExit(cat->lock);

    return 0;
}


/************************************************************************
 * catGetCallHandle
 * purpose: Return the call handle of a CAT call handle
 * input  : hCat        - CAT instance handle
 *          hCatCall    - CAT call handle
 * output : none
 * return : HCALL for the CAT call handle on success
 *          NULL on failure
 ************************************************************************/
HCALL catGetCallHandle(
    IN RVHCAT       hCat,
    IN RVHCATCALL   hCatCall)
{
    catDataStruct*  call = (catDataStruct *)hCatCall;
    if (hCat);
    if(call)
        return call->hsCall;
    return NULL;
}


/************************************************************************
 * catGetKey
 * purpose: Return the key struct stored inside a CAT call handle
 * input  : hCat        - CAT instance handle
 *          hCatCall    - CAT call handle
 * output : none
 * return : Key struct for the CAT call handle on success
 *          NULL on failure
 ************************************************************************/
catStruct* catGetKey(
    IN RVHCAT       hCat,
    IN RVHCATCALL   hCatCall)
{
    catDataStruct*  call = (catDataStruct *)hCatCall;
    if (hCat);
    return &call->key;
}


/************************************************************************
 * catGetUnsolicitedIRR
 * purpose: Return the unsolicited IRR handle stored inside a CAT call handle
 * input  : hCat            - CAT instance handle
 *          hCatCall        - CAT call handle
 *          unsolicitedIRR  - Handle of the unsolicited IRR to set
 * output : none
 * return : Unsolicited IRR transaction handle for the CAT call handle on success
 *          NULL on failure
 ************************************************************************/
HRAS catGetUnsolicitedIRR(
    IN RVHCAT       hCat,
    IN RVHCATCALL   hCatCall)
{
    HRAS    tx;
    catDataStruct*  call = (catDataStruct *)hCatCall;
    if (hCat);

    meiEnter(((catModule *)hCat)->lock);
    tx = call->unsolicitedIRR;
    meiExit(((catModule *)hCat)->lock);

    return tx;
}


/************************************************************************
 * catSetUnsolicitedIRR
 * purpose: Set the unsolicited IRR handle stored inside a CAT call handle
 * input  : hCat            - CAT instance handle
 *          hCatCall        - CAT call handle
 *          unsolicitedIRR  - Handle of the unsolicited IRR to set
 * output : none
 * return : Non-negative value on success
 *          Negative value on failure
 ************************************************************************/
int catSetUnsolicitedIRR(
    IN RVHCAT       hCat,
    IN RVHCATCALL   hCatCall,
    IN HRAS         unsolicitedIRR)
{
    catDataStruct*  call = (catDataStruct *)hCatCall;
    if (hCat);

    meiEnter(((catModule *)hCat)->lock);
    call->unsolicitedIRR = unsolicitedIRR;
    meiExit(((catModule *)hCat)->lock);

    return 0;
}



#ifdef __cplusplus
}
#endif


