#ifndef _xml_tags_h_
#define _xml_tags_h_

#include "tags.h"

#define MAX_XML_DEPTH	20

typedef int (*XMLStartElemHandler)(void *userData, int tag, int depth, const char **atts);
typedef int (*XMLEndElemHandler)(void *userData, int tag, int depth);

typedef struct
{
	XMLStartElemHandler startElem;
	XMLEndElemHandler endElem;

	/* Maintain a history of what all tags were found */
	unsigned char 	tagh[TAGH_LEN];

	/* Maintain some context sensitive information  while parsing */
	int 		depth;
	int 		inFieldType[MAX_XML_DEPTH]; 

	InfoEntry 	infoEntry;
	VpnEntry 	vpnEntry;		
	VpnGroupEntry 	vpnGroupEntry;
	CallPlanEntry	cpEntry;
	VpnRouteEntry	routeEntry;

	int 		pktType;	/* Old Packets, PKT_* */
	unsigned long 	aloidIpAddress, vpnsIpAddress;

	/* Pointer to the whole buffer, which came in */
	char 		*buf;
	int 		buflen;

} XMLCacheCb;

typedef struct
{
	char name[TAG_NAME_LEN];
	int type;
	char cname[TAG_NAME_LEN];
} XMLTagTypes;

#endif /* _xml_tags_h_ */
