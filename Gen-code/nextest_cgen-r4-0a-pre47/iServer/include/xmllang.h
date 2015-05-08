#ifndef _xml_lang_h_
#define _xml_lang_h_

#define TAG_NONE		0
#define	TAG_SERIALNO		1
#define TAG_UPORT		2
#define TAG_IPADDRESS		3
#define TAG_PHONE		4
#define TAG_VPNPHONE		5
#define TAG_IEDGE		6
#define TAG_COMMENTS		7
#define TAG_COUNTRY		8
#define TAG_DND			9
#define TAG_EMAIL		10
#define TAG_ENDPTTYPE		11
#define TAG_FNAME		12
#define TAG_FWDINFO		13
#define TAG_FWDPROTO		14
#define TAG_ITIME		15
#define TAG_LOCATION		16
#define TAG_LNAME		17
#define TAG_PROXYS		18
#define TAG_PROXYC		19
#define TAG_VPNEXTLEN		20
#define TAG_CACHEUPDATE		21
#define TAG_DBUPDATE		22
#define TAG_CACHEENTRY		23
#define TAG_DBENTRY		24
#define TAG_MESSAGE		25
#define TAG_NEXTXML		26
#define TAG_SYN			27
#define TAG_ADV			28
#define TAG_RTIME		29
#define TAG_QUERY		30
#define TAG_ACTIVE		31
#define TAG_DELETE		32
#define TAG_DSYN		33
#define TAG_VPN			34
#define TAG_VPNG		35
#define TAG_AGED		36
#define TAG_MTIME		37
#define TAG_PROXYINFO		38
#define TAG_HRTBT		39
#define TAG_LUS			40
#define TAG_VPNS		41
#define TAG_LOCALNODE		42
#define TAG_CDR			43
#define TAG_CALLANSWER		44
#define TAG_ZONE		45

#define TAG_MAX			46

typedef int (*XMLStartElemHandler)(void *userData, int tag, int depth, const char **atts);
typedef int (*XMLEndElemHandler)(void *userData, int tag, int depth);

typedef struct
{
	char name[TAG_NAME_LEN];
	int type;
	char cname[TAG_NAME_LEN];
} XMLTagTypes;

#endif /* _xml_lang_h_ */
