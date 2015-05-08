#ifndef _xmltags_x_
#define _xmltags_x_

#include "xmllang.h"
/* include in a C file, and compile */

XMLTagTypes tags[] = 
{
	{ "ADV",	TAG_ADV,		"TAG_ADV" },
	{ "CANS",	TAG_CALLANSWER,		"TAG_CALLANSWER" },	
	{ "AX",		TAG_ACTIVE,		"TAG_ACTIVE" },
	{ "AZ",		TAG_AGED,		"TAG_AGED" },
	{ "CDR",	TAG_CDR,		"TAG_CDR" },
	{ "CE", 	TAG_CACHEENTRY, 	"TAG_CACHEENTRY" },
	{ "CM", 	TAG_COMMENTS, 		"TAG_COMMENTS" },
	{ "CU", 	TAG_CACHEUPDATE, 	"TAG_CACHEUPDATE" },
	{ "CN",		TAG_COUNTRY, 		"TAG_COUNTRY" },
	{ "DE",		TAG_DBENTRY, 		"TAG_DBENTRY" },
	{ "DD",		TAG_DELETE,		"TAG_DELETE" },
	{ "DU",		TAG_DBUPDATE, 		"TAG_DBUPDATE" },
	{ "DND",	TAG_DND, 		"TAG_DND" },
	{ "DSYN",	TAG_DSYN, 		"TAG_DSYN" },
	{ "EM",		TAG_EMAIL, 		"TAG_EMAIL" },
	{ "ET",		TAG_ENDPTTYPE, 		"TAG_ENDPTTYPE" },
	{ "FN",		TAG_FNAME, 		"TAG_FNAME" },
	{ "FW",		TAG_FWDINFO, 		"TAG_FWDINFO" },
	{ "FP",		TAG_FWDPROTO, 		"TAG_FWDPROTO" },
	{ "HRT",	TAG_HRTBT,		"TAG_HRTBT" },
	{ "IE",		TAG_IEDGE, 		"TAG_IEDGE" },
	{ "IP", 	TAG_IPADDRESS, 		"TAG_IPADDRESS" },
	{ "IT",		TAG_ITIME, 		"TAG_ITIME" },
	{ "LC",		TAG_LOCATION, 		"TAG_LOCATION" },
	{ "LCL",	TAG_LOCALNODE,		"TAG_LOCALNODE" },
	{ "LN",		TAG_LNAME, 		"TAG_LNAME" },
	{ "LUS",	TAG_LUS,		"TAG_LUS" },
	{ "MSG",	TAG_MESSAGE, 		"TAG_MESSAGE" },
	{ "MT", 	TAG_MTIME,		"TAG_MTIME" },
	{ "NEXTXML",	TAG_NEXTXML, 		"TAG_NEXTXML" },
	{ "PH", 	TAG_PHONE, 		"TAG_PHONE" },
	{ "PXC",	TAG_PROXYC, 		"TAG_PROXYC" },
	{ "PXI",	TAG_PROXYINFO, 		"TAG_PROXYINFO" },
	{ "PXS",	TAG_PROXYS, 		"TAG_PROXYS" },
	{ "QY",		TAG_QUERY,		"TAG_QUERY" },
	{ "RT",		TAG_RTIME,		"TAG_RTIME" },
	{ "SN", 	TAG_SERIALNO, 		"TAG_SERIALNO" },
	{ "SYN",	TAG_SYN, 		"TAG_SYN" },
	{ "UP", 	TAG_UPORT, 		"TAG_UPORT" },
	{ "VPH", 	TAG_VPNPHONE, 		"TAG_VPNPHONE" },
	{ "VPN",	TAG_VPN,		"TAG_VPN" },
	{ "VPNG",	TAG_VPNG,		"TAG_VPNG" },
	{ "VPNS",	TAG_VPNS,		"TAG_VPNS" },
	{ "VPX", 	TAG_VPNEXTLEN, 		"TAG_VPNEXTLEN" },
	{ "ZN",         TAG_ZONE,               "TAG_ZONE" },

	{ "*N*O*N*E*", 	TAG_NONE, "TAG_NONE" },	/* Keep In End */
};

int
XML_GetTagFieldType(const char *name)
{
	int i;

	for (i=0; tags[i].type != TAG_NONE; i++)
	{
		if (!strcmp(tags[i].name, name))
		{
			return tags[i].type;
		}
	}

	return TAG_NONE;
}

#endif /* _xmltags_x_ */
