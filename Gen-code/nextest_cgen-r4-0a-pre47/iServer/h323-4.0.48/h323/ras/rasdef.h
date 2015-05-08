
/*

NOTICE:
This document contains information that is proprietary to RADVISION LTD..
No part of this publication may be reproduced in any form whatsoever without
written prior approval by RADVISION LTD..

RADVISION LTD. reserves the right to revise this publication and make changes
without obligation to notify any person of such revisions or changes.

*/

#ifndef RASDEF_H
#define RASDEF_H

#ifdef __cplusplus
extern "C" {
#endif

#include <rpool.h>
#include <hash.h>
#include <ema.h>
#include <mti.h>
#include <mei.h>
#include <log.h>
#include <q931asn1.h>
#include <cmiAddOns.h>
#include <cmiras.h>
#include <intutils.h>



/************************************************************************
 * Type of transaction
 * RAS_OUT_TX   - Outgoing transaction
 * RAS_IN_TX    - Incoming transaction
 ************************************************************************/
#define RAS_OUT_TX    (1)
#define RAS_IN_TX     (2)


/************************************************************************
 * rasTxMsgType enumeration
 * rasTxMsgRequest - Incoming request message
 * rasTxMsgReject  - Incoming reject response
 * rasTxMsgConfirm - Incoming confirm response
 * rasTxMsgOther   - Other type (RIP, IRR,...)
 ************************************************************************/
typedef enum
{
    rasTxMsgRequest,
    rasTxMsgReject,
    rasTxMsgConfirm,
    rasTxMsgOther
} rasTxMsgType;


/************************************************************************
 * rasTxState enumeration for both incoming and outgoing transactions
 * rasTxStateIdle           - Idle state of transaction
 * rasTxStateReplySent      - Incoming transaction's reply was sent
 * rasTxStateRipSent        - Incoming transaction's RIP was sent
 * rasTxStateRequestSent    - Outgoing transaction's request was sent
 * rasTxStateReplyReceived  - Outgoing transaction's reply was received
 * rasTxStateRipReceived    - Outgoing transaction's RIP was received
 * rasTxStateTimedOut       - Outgoing transaction has timedout
 ************************************************************************/
typedef enum
{
    rasTxStateIdle,
    rasTxStateReplySent,
    rasTxStateRipSent,
    rasTxStateRequestSent,
    rasTxStateReplyReceived,
    rasTxStateRipReceived,
    rasTxStateTimedOut
} rasTxState;


/************************************************************************
 * rasResponseSet enumeration for incoming transactions
 * rasNoResponseSet   - No response created for transaction
 * rasConfirmSet      - Confirm response created for Teansaction
 * rasRejectSet       - Reject response created for Teansaction
 ************************************************************************/
typedef enum
{
    rasNoResponseSet,
    rasConfirmSet,
    rasRejectSet
} rasResponseSet;


/************************************************************************
 * rasMessages enumeration
 * This enumeration holds all the supported message types known to the
 * RAS instance. The specific enumeration value is the index of the
 * message inside RasMessage CHOICE in the H225 ASN.1 definition.
 ************************************************************************/
typedef enum
{
    rasMsgGatekeeperRequest = 0,
    rasMsgGatekeeperConfirm,
    rasMsgGatekeeperReject,
    rasMsgRegistrationRequest,
    rasMsgRegistrationConfirm,
    rasMsgRegistrationReject,
    rasMsgUnregistrationRequest,
    rasMsgUnregistrationConfirm,
    rasMsgUnregistrationReject,
    rasMsgAdmissionRequest,
    rasMsgAdmissionConfirm,
    rasMsgAdmissionReject,
    rasMsgBandwidthRequest,
    rasMsgBandwidthConfirm,
    rasMsgBandwidthReject,
    rasMsgDisengageRequest,
    rasMsgDisengageConfirm,
    rasMsgDisengageReject,
    rasMsgLocationRequest,
    rasMsgLocationConfirm,
    rasMsgLocationReject,
    rasMsgInfoRequest,
    rasMsgInfoRequestResponse,
    rasMsgNonStandardMessage,
    rasMsgUnknownMessageResponse,
    rasMsgRequestInProgress,
    rasMsgResourcesAvailableIndicate,
    rasMsgResourcesAvailableConfirm,
    rasMsgInfoRequestAck,
    rasMsgInfoRequestNak,
    rasMsgServiceControlIndication,
    rasMsgServiceControlResponse,
    rasMsgLast /* Must be the last in enum */
} rasMessages;


/********************************************************************************************
 * Description of THREAD_RasLocalStorage
 * This structure holds thread related information for RAS
 * bufferSize   - The size of allocated buffer. We might need that when we'll have to realloc
 * buffer       - The encoding/decoding buffer
 ********************************************************************************************/
typedef struct
{
    UINT32  bufferSize;
    BYTE*   buffer;
} THREAD_RasLocalStorage;


/************************************************************************
 * rasMessageInfoStruct
 * This struct holds static information about a specific message type.
 * trType       - Indication of the transaction type: Incoming or Outgoing
 *                Can be RAS_OUT_TX, RAS_IN_TX, RVERROR
 * msgType      - Type of the message
 * transaction  - The transaction that this message is associated with
 *                IRR is handled separately.
 ************************************************************************/
typedef struct
{
    int                 trType;
    rasTxMsgType        msgType;
    cmRASTransaction    transaction;
} rasMessageInfoStruct;



/* Static informatino about the message types we're dealing with */
extern const rasMessageInfoStruct rasMessageInfo[];



/************************************************************************
 * rasOutTx
 * Outgoing transaction struct
 * hashValue        - Handle to the hash value in the outgoing hash table
 * state            - Current state of the transaction
 * transactionType  - Type of transaction we're dealing with
 * destAddress      - Destination address of this transaction
 * isMulticast      - Is this a multicast transaction or a unicast one
 * retryCount       - Number of retries left for this transaction
 * txProperty       - Property DB of the transaction - part of the API
 * encodedMsg       - Encoded message handle in RPOOL
 * timer            - Timer handle used for retries
 * evResponse       - Callback that is called on responses
 * hsCall           - Stack's call handle for this transaction
 *                    NULL if not applicable
 * hMsgContext      - Message context of the incoming response message
 *                    This parameter is used by security applications
 *                    that want to pass information for later scrunity
 *                    of the application.
 ************************************************************************/
typedef struct
{
    void*               hashValue;
    rasTxState          state;
    cmRASTransaction    transactionType;
    cmRASTransport      destAddress;
    BOOL                isMulticast;
    INT32               retryCount;
    int                 txProperty;
    void*               encodedMsg;
    HTI                 timer;
    cmiEvRASResponseT   evResponse;
    HCALL               hsCall;
    void*               hMsgContext;
    int			sd; /* NexTone */
} rasOutTx;


/************************************************************************
 * rasInTx
 * Incoming transaction struct
 * hashValue        - Handle to the hash value in the outgoing hash table
 * state            - Current state of the transaction
 * transactionType  - Type of transaction we're dealing with
 * destAddress      - Destination address of this transaction
 * isMulticast      - Is this a multicast transaction or a unicast one
 * stopTime         - Absolute stop time of this transaction
 *                    After that time, this transaction should be closed
 * ripStopTime      - Absolute stop time of RIP message on this transaction
 *                    used to calculate a retransmission of a RIP message
 * txProperty       - Property DB of the transaction - part of the API
 * responseSet      - Indicates if a response message was already built or not
 * encodedMsg       - Encoded message handle in RPOOL of the reply
 * hsCall           - Call handle associated with the transaction
 * hMsgContext      - Message context of the incoming request message
 *                    This parameter is used by security applications
 *                    that want to pass information for later scrunity
 *                    of the application.
 * next             - Next transaction in the closed transactions list
 *                    If this is set to NULL, then the transaction wasn't
 *                    closed by the application
 ************************************************************************/
typedef struct rasInTx_tag rasInTx;
struct rasInTx_tag
{
    void*               hashValue;
    rasTxState          state;
    cmRASTransaction    transactionType;
    cmRASTransport      destAddress;
    BOOL                isMulticast;
    int                 stopTime;
    int                 ripStopTime;
    int                 txProperty;
    rasResponseSet      responseSet;
    void*               encodedMsg;
    HCALL               hsCall;
    void*               hMsgContext;
    rasInTx*            next;
    int			sd; /* NexTone */
};


/************************************************************************
 * rasInTxKey
 * Incoming transaction key struct
 * This is the key used for the hash table
 * seqNumber    - Sequence number of the incoming transaction
 * msgType      - Type of message invoking this incoming transaction
 * address      - Address of sender
 ************************************************************************/
typedef struct
{
    UINT32              seqNumber;
    rasMessages         msgType;
    cmTransportAddress  address;
} rasInTxKey;


/************************************************************************
 * rasDecoderStruct struct
 * This struct holds information needed for fast decoding of the first
 * 2 fields inside a RAS message. It is calculated on initialization of
 * the RAS instance.
 * firstExtField    - Number of the first extended field in the list
 *                    if fields of RasMessage CHOICE
 * numOfMessages    - Number of RasMessage types in the CHOICE
 * numOfOptFields   - Number of the optional fields inside each type of
 *                    RasMessage
 ************************************************************************/
typedef struct
{
    UINT32  firstExtField;
    UINT32  numOfMessages;
    UINT32* numOfOptFields;
} rasDecoderStruct;



/************************************************************************
 * rasModule struct
 * This struct holds all the necessary information of the RAS.
 * It is returned by cmiRASInitialize() as the module handle.
 * app              - Stack instance handle
 * log              - Log handle to use for messages
 * logChan          - Log handle of the ASN.1 messages
 * confNode         - Configuration node id for RAS parameters
 * requestSeqNum    - Last sequence number used for outgoing transactions
 * decoder          - Information needed to decode the first 2 fields of
 *                    any RAS message.
 * bufferSize       - Buffer size to use for encoding/decoding messages
 *
 * timers           - Timer pool used for outgoing transactions retransmissions
 * hCat             - CAT instance handle
 * hVal             - Value tree used for property db and ASN.1 messages
 * synMessage       - RAS message syntax tree
 * synProperty      - RAS property database syntax tree
 * inHash           - Incoming transactions hash table
 * inRa             - Incoming transactions array
 * outHash          - Outgoing transactions hash table
 * outRa            - Outgoing transactions array
 * messages         - Stored messages for retransmissions
 *
 * lockInHash       - Mutex handle of incoming hash table
 * lockOutHash      - Mutex handle of outgoing hash table
 * lockMessages     - Mutex handle of retransmissions database
 * lockGarbage      - Mutex handle of garbage collection mechanism
 *
 * defaultMessages  - Default RAS messages to set for outgoing requests
 *                    and replies.
 *
 * epId             - Endpoint identifier buffer. Used when the endpoint is registered
 * epIdLen          - Length of the endpoint identifier in bytes
 * gkId             - Gatekeeper identifier buffer. Used when the endpoint is registered
 * gkIdLen          - Length of the gatekeeper identifier in bytes
 * termAliasesNode  - Terminal Aliases used for this terminal, as configured by the
 *                    application and updated in RCF messages
 *
 *   The next 2 parameters are used for garbage collection mechanism of
 *   the incoming transactions database
 * firstTx          - First transaction in list of incoming transactions
 * lastTx           - Last transaction in list of incoming transactions
 *
 * evApp            - Application callback functions on RAS events
 * evAutoRas        - Application callback functions on Automatic RAS events
 * evEpRequest      - Callback function for RAS requests not related to calls
 * evEpRequest      - Callback function for RAS requests related to calls
 * evMessages       - Callback function for RAS messages
 * evSendMessage    - Send a message through the network
 *
 * unicastAppHandle     - Application handle of unicast connection
 * multicastAppHandle   - Application handle of multicast connection
 *
 * gatekeeperRASAddress         - Gatekeeper's RAS address node ID
 * gatekeeperCallSignalAddress  - Gatekeeper's Q931 address node ID
 *
 * -- H235 (security) related callbacks:
 * cmiEvRASNewRawMessage            - Called when incoming message needs to be decoded
 * cmiEvRASSendRawMessage           - Called when outgoing message needs to be encoded
 * cmiEvRASReleaseMessageContext    - Called when a message can be discarded
 ************************************************************************/
typedef struct
{
    HAPP                    app;
    RVHLOG                  log;
    RVHLOG                  logChan;
    int                     confNode;
    UINT32                  requestSeqNum;
    rasDecoderStruct        decoder;
    UINT32                  bufferSize;

    HSTIMER                 timers;
    RVHCAT                  hCat;
    HPVT                    hVal;
    HPST                    synMessage;
    HPST                    synProperty;
    HHASH                   inHash;
    HEMA                    inRa;
    HHASH                   outHash;
    HEMA                    outRa;
    HRPOOL                  messages;

    HMEI                    lockInHash;
    HMEI                    lockOutHash;
    HMEI                    lockMessages;
    HMEI                    lockGarbage;

    int                     defaultMessages[cmRASMaxTransaction][4];
                            /* We set the array up to 4 although we only have
                               Request,Confirm,Reject (=3) to allow easier
                               handling and clearer understanding of the code.
                               This doesn't take a lot of memory, as it is
                               allocated once per stack instance. */

    char                    epId[256];
    int                     epIdLen;
    char                    gkId[256];
    int                     gkIdLen;
    int                     termAliasesNode;

    rasInTx*                firstTx;
    rasInTx*                lastTx;

    SCMRASEVENT             evApp;
    SCMAUTORASEVENT         evAutoRas;
    cmiEvRASEPRequestT      evEpRequest;
    cmiEvRASCallRequestT    evCallRequest;
    cmRasMessageEvent       evMessages;
    cmiEvRasMessageToSend   evSendMessage;
    cmiEvRASNewCallT        evRASNewCall;

    HAPPCONN                unicastAppHandle;
    HAPPCONN                multicastAppHandle;

    int                     gatekeeperRASAddress;
    int                     gatekeeperCallSignalAddress;

    cmiEvRASNewRawMessageT          cmiEvRASNewRawMessage;
    cmiEvRASSendRawMessageT         cmiEvRASSendRawMessage;
    cmiEvRASReleaseMessageContextT  cmiEvRASReleaseMessageContext;
	RvRandomGenerator				seed;

} rasModule;



#ifdef __cplusplus
}
#endif

#endif  /* RASDEF_H */

