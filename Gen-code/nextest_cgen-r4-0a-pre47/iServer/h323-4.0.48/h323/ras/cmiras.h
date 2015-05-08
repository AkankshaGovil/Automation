
/*

NOTICE:
This document contains information that is proprietary to RADVISION LTD..
No part of this publication may be reproduced in any form whatsoever without
written prior approval by RADVISION LTD..

RADVISION LTD. reserves the right to revise this publication and make changes
without obligation to notify any person of such revisions or changes.

*/

#ifndef CMIRAS_H
#define CMIRAS_H

#ifdef __cplusplus
extern "C" {
#endif


#include <log.h>
#include <cat.h>
#include <cmCrossReference.h>
#include <cmras.h>




/* Application's handle of a RAS message */
DECLARE_OPAQUE(HAPPRASMSG);




/************************************************************************
 * rasChanType enumeration
 * rasChanUnicast   - Unicast channel the RAS is listening on
 * rasChanMulticast - Multicast channel the RAS is listening on
 ************************************************************************/
typedef enum
{
    rasChanUnicast,
    rasChanMulticast
} rasChanType;



/************************************************************************
 * cmRASStatistics struct
 * inTransactions   - Statistics about incoming transactions
 * outTransactions  - Statistics about outgoing transactions
 * nonDeleted       - Transactions that are marked as deleted, but not yet
 *                    deleted
 ************************************************************************/
typedef struct
{
    rvStatistics    inTransactions;
    rvStatistics    outTransactions;
    UINT32          nonDeleted;
} cmRASStatisics;






/************************************************************************
 * cmiRASInitialize
 * purpose: Initialize a RAS instance for use by the CM
 * input  : hApp            - Stack's instance handle
 *          logMgr          - Log manager handle to use
 *          hCat            - CAT instance handle
 *          hVal            - Value "forest" to use for ASN.1 messages
 *          rasConfNode     - RAS configuration node id
 *          maxIncomingTx   - Maximum number of incoming RAS transactions
 *          maxOutgoingTx   - Maximum number of outgoing RAS transactions
 *          maxBufSize      - Maximum buffer size for encoded messages
 * output : none
 * return : RAS module handle on success
 *          NULL on failure
 ************************************************************************/
HRASMGR RVCALLCONV cmiRASInitialize(
    IN  HAPP        hApp,
    IN  RVHLOGMGR   logMgr,
    IN  RVHCAT      hCat,
    IN  HPVT        hVal,
    IN  int         rasConfNode,
    IN  UINT32      maxIncomingTx,
    IN  UINT32      maxOutgoingTx,
    IN  UINT32      maxBufSize);


/************************************************************************
 * cmiRASStart
 * purpose: Start a RAS instance for use by the CM
 *          Should be called after cmiRASInitialize() and before any
 *          other function.
 * input  : hRasMgr         - RAS instance handle
 *          rasAddressNode  - RAS listening address node id
 * output : none
 * return : Non negative value on success
 *          Negative value on failure
 ************************************************************************/
int RVCALLCONV cmiRASStart(
    IN  HRASMGR hRasMgr,
    IN  int     rasAddressNode);


/************************************************************************
 * cmiRASStop
 * purpose: Stop a RAS instance from running
 * input  : hRasMgr - RAS instance handle
 * output : none
 * return : Non negative value on success
 *          Negative value on failure
 ************************************************************************/
int RVCALLCONV cmiRASStop(
    IN  HRASMGR hRasMgr);


/************************************************************************
 * cmiRASEnd
 * purpose: Free any resources taken by a RAS instance and stop its
 *          activities.
 * input  : hRasMgr - RAS instance handle
 * output : none
 * return : Non negative value on success
 *          Negative value on failure
 ************************************************************************/
int RVCALLCONV cmiRASEnd(
    IN  HRASMGR hRasMgr);

/************************************************************************
 * cmiRASGetEndpointID
 * purpose: Retrieves the EndpointID stored in the ras 
 *
 * input  : hRasMgr             - RAS instance handle
 *          eId				    - pointer to the buffer for endpoint ID
 *                                buffer should be big enough for longest EID 
 *								  possible (256 byte)
 * output : none
 * return : The length of EID in bytes on success 
 *          Negative value on failure
 ************************************************************************/
int RVCALLCONV cmiRASGetEndpointID(
    IN  HRASMGR hRasMgr,
	IN  void*	eId);


/************************************************************************
 * cmiRASUpdateRegInfo
 * purpose: Update the registration information inside the default RAS
 *          messages. This function should be called whenever the
 *          registration status changes.
 *          It takes most of the information from the RAS configuration
 *          message in the PVT database.
 * input  : hRasMgr             - RAS instance handle
 *          beforeRegistration  - TRUE if called before cmRegister()
 *                                FALSE if called after RCF received
 * output : none
 * return : Non negative value on success
 *          Negative value on failure
 ************************************************************************/
int RVCALLCONV cmiRASUpdateRegInfo(
    IN  HRASMGR hRasMgr,
    IN  BOOL    beforeRegistration);


/************************************************************************
 * cmiRASUpdateCallSignalingAddress
 * purpose: Update the CallSignalling address inside the default RAS
 *          messages. This function should be called whenever this
 *          adress changes.
 *          It takes most of the information from the given nodeId
 * input  : hRasMgr     - RAS instance handle
 *          addressId   - nodeId of the address to set
 *          annexEId    - nodeId of the annexE address to set
 * output : none
 * return : Non negative value on success
 *          Negative value on failure
 ************************************************************************/
int RVCALLCONV cmiRASUpdateCallSignalingAddress(
    IN  HRASMGR hRasMgr,
    IN  int     addressId,
    IN  int     annexEId);


/************************************************************************
 * cmiRASGetHAPP
 * purpose: Get the application's handle of a RAS instance for a
 *          specific stack's RAS transaction handle
 * input  : hsRas       - Stack's handle for the RAS transaction
 * output : none
 * return : The application's handle of the RAS instance on success
 *          NULL on failure
 ************************************************************************/
RVAPI
HAPP RVCALLCONV cmiRASGetHAPP(
    IN  HRAS hsRas);




/************************************************************************
 * cmiRASGetRequest
 * purpose: Gets the pvt node of the current RAS request
 * input  : hsRas       - Stack's handle for the RAS transaction
 * output : none
 * return : pvt node of the current RAS request
 *          or negative value on error
 ************************************************************************/
int RVCALLCONV cmiRASGetRequest(
    IN  HRAS         hsRas);


/************************************************************************
 * cmiRASGetResponse
 * purpose: Gets the pvt node of the current RAS response
 * input  : hsRas       - Stack's handle for the RAS transaction
 * output : none
 * return : pvt node of the current RAS response
 *          or negative value on error
 ************************************************************************/
int RVCALLCONV cmiRASGetResponse(
    IN  HRAS         hsRas);


/************************************************************************
 * cmiRASReceiveMessage
 * purpose: Notify the RAS instance that a message was received from the
 *          network and should be handled.
 * input  : hRasMgr         - RAS instance that needs to handle the message
 *          chanType        - Type of channel to send through
 *          srcAddress      - Address of the sender
 *          messageBuf      - The message buffer to send
 *          messageLength   - The length of the message in bytes
 * output : none
 * return : If an error occurs, the function returns a negative value.
 *          If no error occurs, the function returns a non-negative value.
 ************************************************************************/
int RVCALLCONV cmiRASReceiveMessage(
    IN HRASMGR          hRasMgr,
    IN rasChanType      chanType,
    IN cmRASTransport*  srcAddress,
    IN BYTE*            messageBuf,
    IN UINT32           messageLength);







/************************************************************************
 *
 *                           CALLBACK functions
 *
 ************************************************************************/


/************************************************************************
 * cmiEvRasMessageToSend
 * purpose: Callback function that the RAS calls when it wants to send
 *          out a RAS message through the network.
 * input  : hApp            - Stack's instance handle
 *          chanType        - Type of channel to send through
 *                            (for now, always through unicast)
 *          destAddress     - Address to send the message to
 *          messageBuf      - The message buffer to send
 *          messageLength   - The length of the message in bytes
 * output : none
 * return : If an error occurs, the function returns a negative value.
 *          If no error occurs, the function returns a non-negative value.
 ************************************************************************/
typedef int (RVCALLCONV *cmiEvRasMessageToSend)(
    IN HAPP             hApp,
    IN rasChanType      chanType,
    IN cmRASTransport*  destAddress,
    IN BYTE*            messageBuf,
    IN UINT32           messageLength);


/************************************************************************
 * cmEvRasMessageReceiveFunc
 * purpose: Callback function that the RAS calls when a RAS message
 *          is received.
 *          This callback function can be used between DLLs and is
 *          used by the MIB package.
 * input  : nodeId      - pvt node of the received message
 *          rasTrStage  - Stage of the transaction we're in
 *          hsRas       - Stack's handle for the RAS transaction
 *          hAppRas     - Application's handle of this RAS message type
 * output : none
 * return : If an error occurs, the function returns a negative value.
 *          If no error occurs, the function returns a non-negative value.
 ************************************************************************/
typedef int (RVCALLCONV *cmEvRasMessageReceiveFunc)(
    IN int              nodeId,
    IN cmRASTrStage     rasTrStage,
    IN HRAS             hsRas,
    IN HAPPRASMSG       hAppRas);


/************************************************************************
 * cmEvRasMessageSendFunc
 * purpose: Callback function that the RAS calls when a RAS message
 *          is sent.
 *          This callback function can be used between DLLs and is
 *          used by the MIB package.
 * input  : nodeId      - pvt node of the received message
 *          rasTrStage  - Stage of the transaction we're in
 *          hsRas       - Stack's handle for the RAS transaction
 *          hAppRas     - Application's handle of this RAS message type
 * output : none
 * return : If an error occurs, the function returns a negative value.
 *          If no error occurs, the function returns a non-negative value.
 ************************************************************************/
typedef int (RVCALLCONV *cmEvRasMessageSendFunc)(
    IN int          nodeId,
    IN cmRASTrStage rasTrStage,
    IN HRAS         hsRas,
    IN HAPPRASMSG   hAppRas);


/************************************************************************
 * cmiEvRASEPRequestT
 * purpose: Callback function that the RAS calls when a new incoming
 *          RAS request that is not related to any particular call
 *          is being handled
 * input  : hApp        - Stack instance handle
 *          hsRas       - Stack's handle for the RAS transaction
 *          transaction - The type of transaction that arrived
 *          srcAddress  - Address of the source
 *          notifyCb    - Callback for application about the request
 * output : none
 * return : TRUE if message was processed by this callback and shouldn't
 *          be processed by the manual RAS callbacks
 *          FALSE if message wasn't processed by this callback
 ************************************************************************/
typedef BOOL (RVCALLCONV *cmiEvRASEPRequestT)(
    IN  HAPP                hApp,
    IN  HRAS                hsRas,
    IN  cmRASTransaction    transaction,
    IN  cmRASTransport*     srcAddress,
    IN  cmEvAutoRASRequestT notifyCb);


/************************************************************************
 * cmiEvRASCallRequestT
 * purpose: Callback function that the RAS calls when a new incoming
 *          RAS request that is related to a specific call that
 *          is being handled
 * input  : hApp        - Stack instance handle
 *          hsRas       - Stack's handle for the RAS transaction
 *          hsCall      - Stack's call handle
 *          transaction - The type of transaction that arrived
 *          srcAddress  - Address of the source
 *          haCall      - Application's call handle
 *          notifyCb    - Callback for application about the request
 * output : none
 * return : TRUE if message was processed by this callback and shouldn't
 *          be processed by the manual RAS callbacks
 *          FALSE if message wasn't processed by this callback
 ************************************************************************/
typedef BOOL (RVCALLCONV *cmiEvRASCallRequestT)(
    IN  HAPP                hApp,
    IN  HRAS                hsRas,
    IN  HCALL               hsCall,
    IN  cmRASTransaction    transaction,
    IN  cmRASTransport*     srcAddress,
    IN  HAPPCALL            haCall,
    IN  cmEvAutoRASRequestT notifyCb);


/************************************************************************
 * cmiEvRASResponseT
 * purpose: Callback function that the RAS calls when a new incoming
 *          RAS response has to be handled
 * input  : haRas       - The application's RAS transaction handle
 *          hsRas       - Stack's handle for the RAS transaction
 *          trState     - Transaction stage we're in
 * output : none
 * return : non-negative value on success
 *          negative value on failure
 ************************************************************************/
typedef int (RVCALLCONV *cmiEvRASResponseT)(
    IN  HAPPRAS          haRas,
    IN  HRAS             hsRas,
    IN  cmRASTrStage     trStage);



/************************************************************************
 * cmRasMessageEvent struct
 * This struct holds the events of received and sent messages.
 * It is used by the MIB to get information about the RAS messages.
 * hAppRasMsg               - Application's handle for RAS messages
 * cmEvRasMessageReceive    - Callback function for received RAS messages
 * cmEvRasMessageSend       - Callback function for sent RAS messages
 ************************************************************************/
typedef struct
{
    HAPPRASMSG                  hAppRasMsg;
    cmEvRasMessageReceiveFunc   cmEvRasMessageReceive;
    cmEvRasMessageSendFunc      cmEvRasMessageSend;
} cmRasMessageEvent;



/************************************************************************
 * cmiRASSetSendEventHandler
 * purpose: Set the callback function to use for sending messages through
 *          the network.
 * input  : hRasMgr         - RAS module to set
 *          evMessageToSend - Callback function to use
 * output : none
 * return : If an error occurs, the function returns a negative value.
 *          If no error occurs, the function returns a non-negative value.
 ************************************************************************/
int RVCALLCONV cmiRASSetSendEventHandler(
    IN  HRASMGR                 hRasMgr,
    IN  cmiEvRasMessageToSend   evMessageToSend);


/************************************************************************
 * cmiRASSetTrEventHandler
 * purpose: Set the callback function to use for responses to RAS
 *          transactions.
 *          It is called per transaction if the stack wishes to be
 *          notified of the response.
 * input  : hRas            - The application's RAS transaction handle
 *                            to deal with
 *          cmEvRASResponse - The callback to use
 * output : none
 * return : If an error occurs, the function returns a negative value.
 *          If no error occurs, the function returns a non-negative value.
 ************************************************************************/
int RVCALLCONV cmiRASSetTrEventHandler(
    IN  HRAS                hRas,
    IN  cmiEvRASResponseT   cmEvRASResponse);


/************************************************************************
 * cmiRASSetEPTrEventHandler
 * purpose: Set the callback function to use for new RAS requests that
 *          are not related to specific calls.
 * input  : hApp            - Application's handle for a stack instance
 *          cmEvRASRequest  - The callback to use
 * output : none
 * return : If an error occurs, the function returns a negative value.
 *          If no error occurs, the function returns a non-negative value.
 ************************************************************************/
int RVCALLCONV cmiRASSetEPTrEventHandler(
    IN  HAPP                hApp,
    IN  cmiEvRASEPRequestT  cmEvRASEPRequest);


/************************************************************************
 * cmiRASSetCallTrEventHandler
 * purpose: Set the callback function to use for new RAS requests that
 *          are related to specific calls.
 * input  : hApp            - Application's handle for a stack instance
 *          cmEvRASRequest  - The callback to use
 * output : none
 * return : If an error occurs, the function returns a negative value.
 *          If no error occurs, the function returns a non-negative value.
 ************************************************************************/
int RVCALLCONV cmiRASSetCallTrEventHandler(
    IN  HAPP                    hApp,
    IN  cmiEvRASCallRequestT    cmEvRASCallRequest);


/************************************************************************
 * cmRASMsgSetEventHandler
 * purpose: Set the callback function to use for RAS messages.
 *          Used specifically by the MIB.
 * input  : hApp            - Application's handle for a stack instance
 *          cmRASMsgEvent   - The callbacks to use
 *          size            - Size of the callbacks struct
 * output : none
 * return : If an error occurs, the function returns a negative value.
 *          If no error occurs, the function returns a non-negative value.
 * note   : This function is part of an API that wasn't published. It
 *          is used for H235v1.
 ************************************************************************/
RVAPI
int RVCALLCONV cmRASMsgSetEventHandler(
    IN  HAPP                hApp,
    IN  cmRasMessageEvent*  cmRASMsgEvent,
    IN  int                 size);



/************************************************************************
 * cmiRASGetProperty
 * purpose: Gets the pvt node of the RAS property DB
 * input  : hsRas       - Stack's handle for the RAS transaction
 * output : none
 * return : pvt node of the RAS property DB
 *          or negative value on error
 ************************************************************************/
int cmiRASGetProperty(
    IN  HRAS         hsRas);


/************************************************************************
 * cmiEvRASNewCallT
 * purpose: Callback function that the RAS calls when a new call is detected
 * input  : hApp        - Application's handle for a stack instance
 *        : lphCatCall  - the pointer where to put the CAT handle
 *        : callObj     - call parameters
 * output : none
 * return : Non-negative value on success
 *          Negative value on failure
 ************************************************************************/
typedef int (*cmiEvRASNewCallT)(
    IN HAPP         hApp,
    IN RVHCATCALL*  lphCatCall,
    IN catStruct*   callObj);



/************************************************************************
 * cmiRASSetNewCallEventHandler
 * purpose: Set the callback function to use when RAS detects new call
 *
 * input  : hApp            - Application's handle for a stack instance
 *          cmiEvRASNewCall - The callback for new call
 * output : none
 * return : If an error occurs, the function returns a negative value.
 *          If no error occurs, the function returns a non-negative value.
 ************************************************************************/
int cmiRASSetNewCallEventHandler(
    IN  HAPP                hApp,
    IN  cmiEvRASNewCallT    cmiEvRASNewCall);





/************************************************************************
 * cmiRASGetStatistics
 * purpose: Returns some statistics about the RAS module of the stack
 * input  : hApp   - Application's handle for a stack instance
 * output : stats  - Statistics found
 * return : If an error occurs, the function returns a negative value.
 *          If no error occurs, the function returns a non-negative value.
 ************************************************************************/
RVAPI
int RVCALLCONV cmiRASGetStatisics(IN HAPP hApp, OUT cmRASStatisics* stats);




#ifdef __cplusplus
}
#endif

#endif  /* CMIRAS_H */

