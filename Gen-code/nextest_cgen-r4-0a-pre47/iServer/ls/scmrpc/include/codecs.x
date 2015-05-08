/* moved from include/codecs.h */
enum _CodecType
{
	CodecGPCMU = 0,
	CodecGPCMA = 8,
	CodecG7231 = 4,
	CodecG728 = 15,
	CodecG729 = 18,
	CodecRFC2833 = 101,
	T38Fax = 128,
	CodecG729A = 129,
	CodecG729B = 130,
	CodecG729AwB = 131,
	CodecG723AR = 200,
	CodecNonStandard = 255
};

typedef enum _CodecType CodecType;

#ifdef RPC_HDR
%#define MSW_IS_NONSTANDARD_CODEC(codecType)  ( (codecType) >= CodecG723AR)
#endif

enum _MediaDirection
{
	SendRecv = 0,
	SendOnly,
	RecvOnly
};

typedef enum _MediaDirection MediaDirection;

enum _MediaType
{
	MediaAudio = 0,
	MediaImage = 1,	/* Fax goes here */
	MediaVideo = 2

};

typedef enum _MediaType MediaType;

#ifdef RPC_HDR
%
%typedef enum
%{
%	TransportRTPAVP = 0,	// AVP Group
%	TransportUDPTL = 1,		// T.38 UDP Transport App Layer
%
%} TransportType;
%
%#define MAX_CODECS  12
%
#endif /* RPC_HDR */

struct _RTPSet
{
	MediaType	mediaType;
	CodecType	codecType;

	unsigned long	rtpaddr; /* Stored in host byte order */
	unsigned short 	rtpport; /* Stored in host byte order */
	int				param;

	int			dataTypeHandle;
	MediaDirection	direction;
	int				flags;
#ifdef RPC_HDR
%#define	RTP_FLAGS_SS 0x1
#endif
	
	int			mLineNo;
};

typedef _RTPSet RTPSet;

struct _SDPAttr
{
	char type;
	string name<>;
	string value<>;
	int	mLineNo;
};

typedef struct _SDPAttr SDPAttr;

#ifdef RPC_HDR

%#define G711ULAW64_MS   200
%extern int g711Ulaw64Duration;
%
%#define G711ALAW64_MS   200
%extern int g711Alaw64Duration;
%
%#define G729_FRAMES     20
%extern int g729Frames;
%
%#define G7231_FRAMES    6
%extern int g7231Frames;

%#define MAXPARAM	256

%extern CodecType defaultCodec;

#endif /* RPC_HDR */
