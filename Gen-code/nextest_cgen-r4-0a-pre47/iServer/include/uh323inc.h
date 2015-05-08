#ifndef _UH323INC_H_
#define _UH323INC_H_
#include "sipcall.h"

union _CLIRData
{
#define FLAG_NONE_SET           0x0000
#define PR_INDICATOR_SET		0x0001
#define SCR_INDICATOR_SET		0x0002
    struct
    {
        unsigned char screeningIndicator:2;
        unsigned char presentationIndicator:2;
        unsigned char flags:4;
    }data;
    unsigned char char_data;
};
typedef union _CLIRData   CLIRData;

typedef struct {
	char			callID[CALL_ID_LEN];

	/* Calling party number */
	char 			callingpn[PHONE_NUM_LEN];

	/* Called party number */
	char 			calledpn[PHONE_NUM_LEN];

    /* Determined destination of the call */

    /* RTP Params */
	RTPSet			*localSet;
	DTMFParams              *dtmf;
	int				nlocalset;
	unsigned long 	destip;
	short			destport;
	unsigned long 	h245ip;
	short			h245port;
	char 			*display;
	int				displaylen;

	/* Used only used for setup to invite */
	header_url		*requri; 

	header_url_list	*remotecontact_list;

	/* NodeId used for messages between H323 to H323 calls */
	int				nodeId; 
	unsigned long	mediaChanged;
	char				h323Id[H323ID_LEN];

	/* Flag to indicate if calling pn is POTS replaced */
	unsigned int	flags;
	int				stateMode;
	int				modeStatus;
	char			modeName[256];
	int				msgFlags; /* General flags*/
	#define 		MSGFLAG_PI   0x1
	int				q931NodeId; /* forward q931 elements */
	int				sid; /* sessionId */

	/* src signaling addr, used to communicate src information 
	 * to dest 
	 */
	unsigned int 		srcsigip;
	unsigned short	srcsigport;
	int				progress; //progress descriptor
	int				notify; // notify descriptor
	int				pi_IE; // progress indicator Information element
	int				cause; // cause in the progress message

	char 			*confID;	// Conf ID of the leg1 (ONLY for incoming Setups)
								// will always be CONF_ID_LEN, and binary data.
	CLIRData        clirData;
} H323EventData;


#endif
