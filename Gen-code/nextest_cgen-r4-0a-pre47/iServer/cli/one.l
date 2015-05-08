%{
#include "one.tab.h"
%}

TAGALPHA	[A-Za-z_0-9\-]

%option yylineno
%option stack
%x xml_comment
%x xml_directive
%s cdata

%%

\<\!\-\-			{ yy_push_state(xml_comment);/* BEGIN (xml_comment); */}
<xml_comment>\-\-\>		{ yy_pop_state(); /* BEGIN (0); */}
<xml_comment>.		{ }
<xml_comment>\n		{ }
\<\?				{ yy_push_state(xml_directive); }
<xml_directive>\?\>	{ yy_pop_state(); }
<xml_directive>.	{ }
<xml_directive>\n	{ }
\<\!\[CDATA\[			{ yy_push_state(cdata); /* BEGIN (cdata); */}
<cdata>]]\>			{ yy_pop_state(); /* BEGIN (0);*/ }
[ \n\t]				{ }
\<DB\>				{ return _DB; }
\<\/DB\>			{ return _DB_E; }
\<E\>				{ return _E; }
\<V\>				{ return _V; }
\<TRG\>				{ return _TRG; }
\<VG\>				{ return _VG; }
\<CP\>				{ return _CP; }
\<CR\>				{ return _CR; }
\<DCR\>				{ return _DCR; }
\<CPB\>				{ return _CPB; }
\<ET\>				{ return _X_ET; }
\<FW\>				{ return _X_FW; }
\<FP\>				{ return _X_FP; }
\<PXS\>				{ return _X_PXS; }
\<PXC\>				{ return _X_PXC; }
\<DND\>				{ return _X_DND; }
\<DYO\>				{ return _X_DYO; }
\<ITIME\>			{ return _X_ITIME; }
\<MTIME\>			{ return _X_MTIME; }
\<RTIME\>			{ return _X_RTIME; }
\<SFLAGS\>			{ return _X_SFLAGS; }
\<SRNO\>			{ return _X_SRNO; }
\<UPORT\>			{ return _X_UPORT; }
\<PHONE\>			{ return _X_PHONE; }
\<VPN_PHONE\>		{ return _X_VPN_PHONE; }
\<VPN_EXTLEN\>		{ return _X_VPN_EXTLEN; }
\<IPADDR\>			{ return _X_IPADDR; }
\<SUBNETIP\>		{ return _X_SUBNETIP; }
\<SUBNETMASK\>		{ return _X_SUBNETMASK; }
\<ZONE\>			{ return _X_ZONE; }
\<EMAIL\>			{ return _X_EMAIL; }
\<XPHONE\>			{ return _X_XPHONE; }
\<XVPN_PHONE\>		{ return _X_XVPN_PHONE; }
\<NPHONE\>			{ return _X_NPHONE; }
\<NVPN_PHONE\>		{ return _X_NVPN_PHONE; }
\<NVPN_EXTLEN\>		{ return _X_NVPN_EXTLEN; }
\<FNAME\>			{ return _X_FNAME; }
\<LNAME\>			{ return _X_LNAME; }
\<LOCATION\>		{ return _X_LOCATION; }
\<COUNTRY\>			{ return _X_COUNTRY; }
\<COMMENTS\>		{ return _X_COMMENTS; }
\<VPN_ID\>			{ return _X_VPN_ID; }
\<VPN_GROUP\>		{ return _X_VPN_GROUP; }
\<VPN_NAME\>		{ return _X_VPN_NAME; }
\<VPN_LOC\>			{ return _X_VPN_LOC; }
\<VPN_CONTACT\>		{ return _X_VPN_CONTACT; }
\<CP_NAME\>			{ return _X_CP_NAME; }
\<CR_NAME\>			{ return _X_CR_NAME; }
\<CR_SRC\>			{ return _X_CR_SRC; }
\<CR_DEST\>			{ return _X_CR_DEST; }
\<CR_SRCLEN\>		{ return _X_CR_SRCLEN; }
\<CR_DESTLEN\>		{ return _X_CR_DESTLEN; }
\<CR_PREFIX\>		{ return _X_CR_PREFIX; }
\<CR_SRCPREFIX\>	{ return _X_CR_SRCPREFIX; }
\<CR_CPNAME\>		{ return _X_CR_CPNAME; }
\<CR_TRNAME\>		{ return _X_CR_TRNAME; }
\<CR_FLAGS\>		{ return _X_CR_FLAGS; }
\<CR_STIME\>		{ return _X_CR_STIME; }
\<CR_FTIME\>		{ return _X_CR_FTIME; }
\<STATIC\>			{ return _X_STATIC; }
\<DYNOBP\>			{ return _X_DYO; }
\<CR_TM_SEC\>		{ return _X_CR_TM_SEC; }
\<CR_TM_MIN\>		{ return _X_CR_TM_MIN; }
\<CR_TM_HR\>		{ return _X_CR_TM_HR; }
\<CR_TM_MTH\>		{ return _X_CR_TM_MTH; }
\<VPN_PREFIX\>		{ return _X_VPN_PREFIX; }
\<VPN_SIPDOMAIN\>	{ return _X_VPN_SIPDOMAIN; }
\<H323ID\>			{ return _X_H323ID; }
\<PGKID\>			{ return _X_PGKID; }
\<TECHP\>			{ return _X_TECHP; }
\<URI\>				{ return _X_URI; }
\<SIPUSER\>			{ return _X_SIPUSER; }
\<CONTACT\>			{ return _X_CONTACT; }
\<CAP\>				{ return _X_CAP; }
\<CAP2\>			{ return _X_CAP2; }
\<RASIP\>			{ return _X_RASIP; }
\<RASPORT\>			{ return _X_RASPORT; }
\<SIGPORT\>			{ return _X_SIGPORT; }
\<XCALLS\>			{ return _X_XCALLS; }
\<XINCALLS\>		{ return _X_XINCALLS; }
\<INFOTRANSCAP\>	{ return _X_INFOTRANSCAP; }
\<XOUTCALLS\>		{ return _X_XOUTCALLS; }
\<MAXHUNTS\>		{ return _X_MAXHUNTS; }
\<PRIO\>			{ return _X_PRIO; }
\<VENDOR\>			{ return _X_VENDOR; }
\<PASSWORD\>		{ return _X_PASSWORD; }
\<ECAPS1\>			{ return _X_ECAPS1; }
\<CRID\>			{ return _X_CRID; }
\<CUSTID\>			{ return _X_CUSTID; }
\<CDPNTYPE\>		{ return _X_CDPNTYPE; }
\<CGPNTYPE\>		{ return _X_CGPNTYPE; }
\<BCAP\>			{ return _X_BCAP; }
\<TG\>				{ return _X_TG; }
\<SRCITG\>				{ return _X_SRCITG; }
\<DTG\>				{ return _X_DTG; }
\<SRCETG\>				{ return _X_SRCETG; }
\<OGP\>				{ return _X_OGP; }
\<NATIP\>				{ return _X_NATIP; }
\<NATPORT\>				{ return _X_NATPORT; }
\<TRG_NAME\>		{ return _X_TRG_NAME; }
\<TRG_EVT\>			{ return _X_TRG_EVT; }
\<TRG_SVEND\>		{ return _X_TRG_SVEND; }
\<TRG_DVEND\>		{ return _X_TRG_DVEND; }
\<TRG_ACT\>			{ return _X_TRG_ACT; }
\<TRG_ACTD\>		{ return _X_TRG_ACTD; }
\<TRG_ACTFLAGS\>		{ return _X_TRG_ACTFLAGS; }
\<REALM\>			{ return _X_REALM; }
\<REALMID\>			{ return _X_REALMID; }
\<IGROUP\>			{ return _X_IGROUP; }
\<CIDBLK\>			{ return _X_CIDBLK; }
\<RM\>				{ return _RM; }
\<RM_ID\>			{ return _X_RM_ID; }
\<RM_NAME\>			{ return _X_RM_NAME; }
\<RM_RSA\>			{ return _X_RM_RSA; }
\<RM_MASK\>			{ return _X_RM_MASK; }
\<RM_SIGPOOL\>		{ return _X_RM_SIGPOOL; }
\<RM_MEDPOOL\>		{ return _X_RM_MEDPOOL; }
\<RM_AUTHFLAGS\>	{ return _X_RM_AUTHFLAGS; }
\<RM_ASTATUS\>		{ return _X_RM_ASTATUS; }
\<RM_OSTATUS\>		{ return _X_RM_OSTATUS; }
\<RM_ADDR\>			{ return _X_RM_ADDR; }
\<RM_IMR\>			{ return _X_RM_IMR; }
\<RM_EMR\>			{ return _X_RM_EMR; }
\<RM_IFNAME\>		{ return _X_RM_IFNAME; }
\<RM_LIFNAME\>		{ return _X_RM_LIFNAME; }
\<RM_CIDBLK_PREFIX\>    { return _X_RM_CIDBLK_PREFIX;}
\<RM_CIDUNBLK_PREFIX\>  { return _X_RM_CIDUNBLK_PREFIX;}
\<RM_PROXY_REGID\>            { return _X_RM_PROXY_REGID;}
\<RM_PROXY_UPORT\>            { return _X_RM_PROXY_UPORT;}
\<RM_VNETNAME\>			{ return _X_RM_VNETNAME; }
\<IGRP\>			{ return _IGRP; }
\<IGRP_NAME\>		{ return _X_IGRP_NAME; }
\<IGRP_MAXIN\>		{ return _X_IGRP_MAXIN; }
\<IGRP_MAXOUT\>		{ return _X_IGRP_MAXOUT; }
\<IGRP_MAXTOTAL\>	{ return _X_IGRP_MAXTOTAL; }
\<IGRP_IN\>			{ return _X_IGRP_IN; }
\<IGRP_OUT\>		{ return _X_IGRP_OUT; }
\<IGRP_TOTAL\>		{ return _X_IGRP_TOTAL; }
\<IGRP_TIME\>		{ return _X_IGRP_TIME; }
\<VNET\>			{ return _VNET; }
\<VNET_VNETNAME\>		{ return _X_VNET_VNETNAME; }
\<VNET_IFNAME\>			{ return _X_VNET_IFNAME; }
\<VNET_VLANID\>			{ return _X_VNET_VLANID; }
\<VNET_RTGTBLID\>		{ return _X_VNET_RTGTBLID; }
\<VNET_GATEWAY\>		{ return _X_VNET_GATEWAY; }

\<\/.*\>			{ return ENDTAG; }
\<{TAGALPHA}{1,24}\>		{ return UNKNOWNTAG; }
\".*\"				{ 
						yytext[strlen(yytext)-1] = '\0';
						yylval = atoi(&yytext[1]);
						return STRING; 
					}
^#.*			{ }
