#ifdef USE_RTP
#include <rtp.h>
#endif


/********************************************************************************************
 * RTP_Action - enumeration of possible actions on channels.
 * RTP_None         - Do nothing with channels
 * RTP_Playback     - Playback a file on channel
 * RTP_Record       - Recod a channel on file
 * RTP_Replay       - Replay incoming channels back to sender
 * RTP_RecordReply  - Replay and record the incoming channels
 *
 * Currently, only RTP_Replay and RTP_None are supported
 ********************************************************************************************/
typedef enum
{
    RTP_None,
    RTP_Playback,
    RTP_Record,
    RTP_Replay,
    RTP_RecordReplay
} RTP_Action;


#ifdef USE_RTP

#ifdef WIN32
/* This part is used for RING-0 driver */
#pragma pack (push, CMD_ALINMENT, 1)
#endif


/********************************************************************************************
 * RTPCtrl: RTP control struct
 * allocated    - Indicates if this one is allocated and used or not
 * hRTP         - RTP session handle to use
 * hRTCP        - RTCP session handle to use
 * action       - Action to take on the session (none, replay, etc.)
 * bytes        - Number of bytes received on the session (used to calculate the rate)
 * lastTime     - Last time rate was checked (used to calculate the rate)
 * channels     - Number of channels on the session (we must have 2 channels to allow any
 *                action on the session).
 ********************************************************************************************/

typedef struct
{
    BOOL            allocated;
    HRTPSESSION     hRTP;
    HRTCPSESSION    hRTCP;
    RTP_Action      action;
    UINT32          bytes;
    UINT32          lastTime;
    int             channels;
    void            * pMdl;

} RTPCtrl;


#ifdef WIN32
/* This part is used for RING-0 driver */
#pragma pack (pop, CMD_ALINMENT)
#endif


#endif  /* USE_RTP */

