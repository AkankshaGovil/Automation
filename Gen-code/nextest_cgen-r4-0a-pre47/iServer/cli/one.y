%{
#include "alerror.h"
#include "bits.h"
#include "ipc.h"
#include "key.h"
#include "serverdb.h"
#include "db.h"
#include "cli.h"
#include "log.h"

//prototype for the lexer function
int yylex(void);

NetoidInfoEntry netInfo; 
ClientAttribs clAttribs, clAttribsDef = { 0 };
VpnEntry vpnEntry;
VpnGroupEntry vpnGroupEntry;
CallPlanEntry cpEntry;
VpnRouteEntry crEntry;
CallPlanBindEntry cpbEntry;
TriggerEntry triggerEntry;
RealmEntry rmEntry;
IgrpInfo	igrpEntry;
VnetEntry vnetEntry;

#define ProperNesting()	(!parse_iedge && !parse_vpn && !parse_vpng && !parse_cp && !parse_cr && !parse_cpb && !parse_trg && !parse_igrp)

int parse_iedge = 0, parse_vpn = 0, parse_vpng = 0, parse_cp = 0, parse_cr = 0, parse_cpb = 0, parse_realm = 0, parse_igrp = 0, parse_trg = 0, parse_vnet=0;
extern char *yytext;

char netdbfilename[256], vpnsdbfilename[256], vpngdbfilename[256],
			cpdbfilename[256], crdbfilename[256], cpbdbfilename[256],
			attrdbfilename[256], trgdbfilename[256], realmdbfilename[256], igrpdbfilename[256],
			vnetdbfilename[256];

DB_tDb netdbstruct, vpnsdbstruct, vpngdbstruct, cpdbstruct, crdbstruct,
			cpbdbstruct, attrdbstruct, trgdbstruct, realmdbstruct, igrpdbstruct, vnetdbstruct;

DB netdb = 0, vpnsdb = 0, vpngdb = 0, cpdb = 0, crdb = 0, cpbdb = 0, attrdb = 0, trgdb = 0,
		realmdb = 0, igrpdb = 0, vnetdb = 0;

static char oldVpnGroup[VPN_GROUP_LEN], newVpnGroup[VPN_GROUP_LEN];
static int ncroutes = 0;
int nroutes = 0, nbindings = 0, niedges = 0, nplans = 0, nvpns = 0, nvpngs = 0, ntriggers = 0,
	nrealms = 0, nigrps = 0, nvnets = 0;
int nerrors = 0;

%}

%token	STRING
%token	_E _V _VG _DB _DB_E
%token	_X_ET _X_FW _X_FP _X_PXS _X_PXC _X_DND _X_XPHONE _X_XVPN_PHONE
%token 	_X_SRNO _X_UPORT _X_PHONE _X_VPN_PHONE 
%token	_X_VPN_EXTLEN _X_NPHONE _X_NVPN_PHONE _X_NVPN_EXTLEN
%token	_X_IPADDR _X_ZONE _X_EMAIL _X_STATIC _X_DYO
%token	_X_FNAME _X_LNAME _X_LOCATION _X_COUNTRY _X_COMMENTS
%token	_X_ITIME _X_MTIME _X_SUBNETIP _X_SUBNETMASK _X_RTIME _X_SFLAGS

%token 	_X_VPN_ID _X_VPN_GROUP _X_VPN_NAME _X_VPN_LOC _X_VPN_CONTACT
%token 	_CP _CR _DCR _CPB _X_CP_NAME _X_CR_DEST _X_CR_PREFIX _X_CR_NAME
%token 	_X_CR_SRC _X_CR_SRCLEN _X_CR_DESTLEN _X_CR_FLAGS _X_CR_STIME _X_CR_FTIME
%token 	_X_CR_SRCPREFIX _X_CR_CPNAME _X_CR_TRNAME

%token	E_ V_ VG_
%token	X_ET_ X_FW_ X_FP_ X_PXS_ X_PXC_ X_DND_ X_XPHONE_ X_XVPN_PHONE_
%token 	X_SRNO_ X_UPORT_ X_PHONE_ X_VPN_PHONE_ 
%token	X_VPN_EXTLEN_ X_NPHONE_ X_NVPN_PHONE_ X_NVPN_EXTLEN_
%token	X_IPADDR_ X_ZONE_ X_EMAIL_
%token	X_FNAME_ X_LNAME_ X_LOCATION_ X_COUNTRY_ X_COMMENTS_
%token	X_ITIME_ X_MTIME_ X_STATIC_

%token 	X_VPN_ID_ X_VPN_GROUP_ X_VPN_NAME_ X_VPN_LOC_ X_VPN_CONTACT_
%token	CP_ CR_ CPB_ X_CP_NAME_ X_CR_DEST_ X_CR_PREFIX_ X_CR_NAME_
%token 	X_CR_SRC_ X_CR_SRCLEN_ X_CR_DESTLEN_ X_CR_FLAGS_
%token _X_CR_TM_SEC X_CR_TM_SEC_ _X_CR_TM_MIN X_CR_TM_MIN_
%token _X_CR_TM_HR X_CR_TM_HR_ _X_CR_TM_MTH X_CR_TM_MTH_
%token _X_VPN_PREFIX X_VPN_PREFIX_ _X_VPN_SIPDOMAIN X_VPN_SIPDOMAIN_
%token _X_H323ID X_H323ID_ _X_URI X_URI_ _X_SIPUSER _X_PGKID
%token X_SIPUSER_ _X_CONTACT X_CONTACT_ _X_PASSWORD 

%token _X_CAP _X_CAP2 _X_RASIP _X_RASPORT _X_SIGPORT _X_XCALLS _X_INFOTRANSCAP _X_PRIO _X_TECHP _X_VENDOR 
%token _X_MAXHUNTS _X_ECAPS1 _X_CRID _X_CUSTID _X_CDPNTYPE _X_BCAP _X_TG _X_SRCITG  _X_DTG _X_SRCETG _X_CGPNTYPE
%token _X_XINCALLS _X_XOUTCALLS _X_OGP _X_REALM _X_REALMID _X_IGROUP _X_NATIP _X_NATPORT

%token	_TRG _X_TRG_EVT _X_TRG_NAME _X_TRG_SVEND _X_TRG_DVEND _X_TRG_ACT _X_TRG_ACTD  _X_TRG_ACTFLAGS

%token _RM _X_RM_ID _X_RM_NAME _X_RM_RSA _X_RM_MASK _X_RM_SIGPOOL _X_RM_MEDPOOL _X_RM_AUTHFLAGS _X_RM_VNETNAME
%token _X_RM_ASTATUS _X_RM_OSTATUS
%token _X_RM_ADDR _X_RM_IMR _X_RM_EMR _X_RM_IFNAME _X_RM_LIFNAME
%token _IGRP _X_IGRP_NAME _X_IGRP_MAXIN _X_IGRP_MAXOUT _X_IGRP_MAXTOTAL _X_IGRP_IN  
%token _X_IGRP_OUT _X_IGRP_TOTAL _X_IGRP_TIME 
%token _X_CIDBLK
%token _X_RM_CIDBLK_PREFIX _X_RM_CIDUNBLK_PREFIX _X_RM_PROXY_REGID _X_RM_PROXY_UPORT
%token _VNET _X_VNET_VNETNAME _X_VNET_IFNAME _X_VNET_VLANID _X_VNET_RTGTBLID _X_VNET_GATEWAY

%token ENDTAG
%token UNKNOWNTAG

%start iserver_db

%%

iserver_db :	_DB db_one_list _DB_E
			{
				print_progress();

				// Done
  	  			CLIPRINTF((stdout, "\n"));

				DbPrintStats();

				ResetDatabaseParsing();
			}
		| _DB _DB_E
			{
				print_progress();

				// Done
  	  			CLIPRINTF((stdout, "\n"));

				DbPrintStats();

				ResetDatabaseParsing();
			}
		| error 
			{
				print_progress();

				ResetDatabaseParsing();

				//yyerrok; - just quit and display the error
			}
		;

db_one_list : 	db_one_list db_one
		| db_one
		;

db_one : 	db_one_iedge 
		| db_one_vpn 
		| db_one_vpng 
		| db_one_cp 
		| db_one_cr 
		| db_one_cpb
		| db_one_trg
		| db_one_realm
		| db_one_igrp
		| db_one_vnet
		| db_one_unknown
		;

db_one_iedge	: iedge
			{
				print_progress();
			}

db_one_vpn	: vpn
			{
				print_progress();
			}

db_one_vpng	: vpng
			{
				print_progress();
			}

db_one_cp	: cp
			{
				print_progress();
			}

db_one_cr	: cr
			{
				print_progress();
			}

db_one_cpb	: cpb
			{
				print_progress();
			}

db_one_trg	: trg
			{
				print_progress();
			}

db_one_realm : realm
			{
				print_progress();
			}
db_one_igrp : igrp
			{
				print_progress();
			}

db_one_vnet : vnet
			{
				print_progress();
			}

db_one_unknown : UNKNOWNTAG 
			{
  	  			CLIPRINTF((stdout, "unknown db tag %s\n", yytext));
			}
	unknown_attr_list ENDTAG

unknown_attr_list : unknown_attr unknown_attr_list
		| unknown_attr
		;

unknown_attr : UNKNOWNTAG 
			{
  	  			CLIPRINTF((stdout, "unknown field tag %s\n", yytext));
			}
	STRING ENDTAG

iedge	: _E 
		{
			if (!ProperNesting())
			{
				fprintf(stdout, "db not nested properly at iedge #%d\n",
					niedges);
			}

			if (!netdb || !attrdb)
			{

					// Opening a new database ?
					// Check if others need to be closed
					ResetDatabaseParsing();
	
     				netdbstruct.read_write = GDBM_WRCREAT;

  	  				//CLIPRINTF((stdout, "opening %s\n", netdbfilename));
     				if (!(netdb = DbOpenByID(netdbfilename, DB_eNetoids, &netdbstruct)))
     				{
  	  				CLIPRINTF((stdout, "Unable to open %s\n", netdbfilename));
	    				return -1;
     				}

				attrdbstruct.read_write = GDBM_WRCREAT;
  	  				//CLIPRINTF((stdout, "opening %s\n", attrdbfilename));
     				if (!(attrdb = DbOpenByID(attrdbfilename, DB_eAttribs, &attrdbstruct)))
     				{
        				CLIPRINTF((stdout, "Unable to open %s\n", attrdbfilename));
	      				return -1;
     				}
			}

			parse_iedge = 1;
			InitNetoidInfoEntry(&netInfo);
			memset(&clAttribs, 0, sizeof(ClientAttribs));

				       
			/* Initialization value for iedge time */
			netInfo.rTime = time(0);
			netInfo.iTime = time(0);
			netInfo.cTime = time(0);
			netInfo.mTime = time(0);

		}
	iedge_attr_list 
	ENDTAG
		{
			/* Initialization value for iedge time */
			netInfo.rTime = time(0);
			netInfo.iTime = time(0);
			netInfo.cTime = time(0);
			netInfo.mTime = time(0);

			/* Backward compatibility */
			/* If the vpn name is un-assigned, extract it from the 
			 * vpn phone number
			 */
			if (strlen(netInfo.vpnName) == 0)
			{
				strncpy(netInfo.vpnName, netInfo.vpnPhone,
					strlen(netInfo.vpnPhone) - netInfo.vpnExtLen);
			}
			
			// Set up the reams for upgrade
			if (!strcmp(netInfo.realmName, REALM_UNASSIGNED))
			{
				if (netInfo.realmId != REALM_ID_UNASSIGNED)
				{
					strcpy(netInfo.realmName, REALM_ANY);
					netInfo.realmId = REALM_ID_ANY;
				}
			}
			
			if ( (((int)netInfo.ecaps1 & ECAPS1_SIP_PRIVACY_DRAFT01) == 0 ) &&
			     (((int)netInfo.ecaps1 & ECAPS1_SIP_PRIVACY_RFC3325) == 0 ))
			{
				netInfo.ecaps1 = netInfo.ecaps1 | ECAPS1_SIP_PRIVACY_DRAFT01;
				netInfo.ecaps1 |= ECAPS1_SIP_PRIVACY_RFC3325;
			}

			if (IsSGatekeeper(&netInfo) && !netInfo.crId)
			{
				// generate a random crId
				netInfo.crId = lrand48();
			}
			else if (!netInfo.crId)
			{
				netInfo.crId = 1;
			}

			/* We have the iedge entry now */
			if (BIT_TEST(netInfo.sflags, ISSET_REGID) &&
				BIT_TEST(netInfo.sflags, ISSET_UPORT))
			{
				if (CacheHandleIedge(&netInfo, clicmdop) < 0)
				{
					fprintf(stdout,
						"Unable to %s iedge %s %lu in cache\n",
						DbOperToStr(clicmdop),
						netInfo.regid,
						netInfo.uport);
				}

				if (DbOperate(netdb, (char*)&netInfo, sizeof(NetoidInfoEntry), 
					(char*)&netInfo, sizeof(NetoidSNKey), clicmdop, 1))
				{
					fprintf(stdout,
						"Unable to %s iedge %s %lu in db\n",
						DbOperToStr(clicmdop),
						netInfo.regid,
						netInfo.uport);
					nerrors ++;
				}

				print_progress();
				
				if (memcmp(&clAttribs, &clAttribsDef, sizeof(ClientAttribs)))
				{
					/* something was written */
					if (DbOperate(attrdb, (char*)&clAttribs, 
						sizeof(ClientAttribs), 
						(char*)&netInfo, sizeof(NetoidSNKey), clicmdop, 1))
					{
						fprintf(stdout,
							"Unable to %s attrs of %s %lu in db\n",
							DbOperToStr(clicmdop),
							netInfo.regid,
							netInfo.uport);
						nerrors ++;
					}
					print_progress();
				}
			}

			parse_iedge = 0;
			niedges ++;
		}

vpn	: _V 
		{
			if (!vpnsdb)
			{
					// Opening a new database ?
					// Check if others need to be closed
					ResetDatabaseParsing();
	
     				vpnsdbstruct.read_write = GDBM_WRCREAT;
  	  				//CLIPRINTF((stdout, "opening %s\n", vpnsdbfilename));
     				if (!(vpnsdb = DbOpenByID(vpnsdbfilename, DB_eVpns, &vpnsdbstruct)))
     				{
    					CLIPRINTF((stdout, "Unable to open %s\n", vpnsdbfilename));
    					return -1;
     				}
			}
			parse_vpn = 1;
			memset(&vpnEntry, 0, sizeof(VpnEntry));
			vpnEntry.mTime = time(0);
		}
	vpn_attr_list 
	ENDTAG
		{
			vpnEntry.mTime = time(0);
			if (strlen(vpnEntry.vpnName) == 0)
			{
				strncpy(vpnEntry.vpnName, vpnEntry.vpnId,
						VPNS_ATTR_LEN);
			}

			if (strlen(vpnEntry.vpnName) != 0)
			{
				if (CacheHandleVpn(&vpnEntry, clicmdop) < 0)
				{
					fprintf(stdout,
						"Unable to store vpn %s in cache\n",
						vpnEntry.vpnName);
				}

				if (DbOperate(vpnsdb, (char*)&vpnEntry, sizeof(vpnEntry), 
					(char*)&vpnEntry, sizeof(VpnKey), clicmdop, 1))
				{
					fprintf(stdout,
						"Unable to %s vpn %s in db\n",
						DbOperToStr(clicmdop),
						vpnEntry.vpnName);
					nerrors ++;
				}
				print_progress();
			}
			parse_vpn = 0;
			nvpns ++;
		}

vpng	: _VG 
		{
			if (!vpngdb)
			{
					// Opening a new database ?
					// Check if others need to be closed
					ResetDatabaseParsing();
	
     				vpngdbstruct.read_write = GDBM_WRCREAT;
  	  				//CLIPRINTF((stdout, "opening %s\n", vpngdbfilename));
     				if (!(vpngdb = DbOpenByID(vpngdbfilename, DB_eVpnG, &vpngdbstruct)))
     				{
    					CLIPRINTF((stdout, "Unable to open %s\n", vpngdbfilename));
    					return -1;
     				}
			}
			parse_vpng = 1;
			memset(&vpnGroupEntry, 0, sizeof(VpnGroupEntry));
			vpnGroupEntry.mTime = time(0);
		}
	vpng_attr_list 
	ENDTAG
		{
			vpnGroupEntry.mTime = time(0);
			if (strlen(vpnGroupEntry.vpnGroup) != 0)
			{
				if (CacheHandleVpnG(&vpnGroupEntry, clicmdop) < 0)
				{
					fprintf(stdout,
						"Unable to store vpng %s in cache\n",
						vpnGroupEntry.vpnGroup);
				}

				if (DbOperate(vpngdb, (char*)&vpnGroupEntry, sizeof(vpnGroupEntry), 
					(char*)&vpnGroupEntry, sizeof(VpnGroupKey), clicmdop, 1))
				{
					fprintf(stdout,
						"Unable to %s vpng %s in db\n",
						DbOperToStr(clicmdop),
						vpnGroupEntry.vpnGroup);
					nerrors ++;
				}
				print_progress();
			}
			parse_vpng = 0;
			nvpngs ++;
		}

cp	: _CP
		{
			if (!ProperNesting())
			{
				fprintf(stdout, "db not nested properly at plan #%d\n",
					nplans);
			}

			if (!cpdb)
			{
					// Opening a new database ?
					// Check if others need to be closed
					ResetDatabaseParsing();
	
     				cpdbstruct.read_write = GDBM_WRCREAT;
  	  				//CLIPRINTF((stdout, "opening %s\n", cpdbfilename));
     				if (!(cpdb = DbOpenByID(cpdbfilename, DB_eCallPlan, &cpdbstruct)))
     				{

      				CLIPRINTF((stdout, "Unable to open %s\n", cpdbfilename));
	    				return -1;
     				}
			}
			parse_cp = 1;
			memset(&cpEntry, 0, sizeof(cpEntry));
			cpEntry.mTime = time(0);
		}
	cp_attr_list
	ENDTAG
		{
			cpEntry.mTime = time(0);
			if (strlen(cpEntry.cpname) != 0)
			{
				/* Store in the database */
				if (DbOperate(cpdb, (char*)&cpEntry, sizeof(cpEntry), 
					(char*)&cpEntry, sizeof(CallPlanKey), clicmdop, 1))
				{
					fprintf(stdout,
						"Unable to %s cp %s in db\n",
						DbOperToStr(clicmdop),
						cpEntry.cpname);
					nerrors ++;
				}
				print_progress();
			}
			parse_cp = 0;
			nplans ++;
		}

cr_tag	: _CR
	| _DCR
	;

cr	: cr_tag
		{
			if (!ProperNesting())
			{
				fprintf(stdout, "db not nested properly at route #%d\n",
					nroutes);
			}

			if (!crdb)
			{
					// Opening a new database ?
					// Check if others need to be closed
					ResetDatabaseParsing();
	
     				crdbstruct.read_write = GDBM_WRCREAT;
  	  				//CLIPRINTF((stdout, "opening %s\n", crdbfilename));
     				if (!(crdb = DbOpenByID(crdbfilename, DB_eCallRoute, &crdbstruct)))
     				{
    					CLIPRINTF((stdout, "Unable to open %s\n", crdbfilename));
	    				return -1;
     				}
			}

			parse_cr = 1;
			memset(&crEntry, 0, sizeof(crEntry));
			crEntry.mTime = time(0);
		}
	cr_attr_list
	ENDTAG
		{
			crEntry.mTime = time(0);
			if (!strlen(crEntry.crname) && strlen(cpbEntry.cpname))
			{
				fprintf(stdout,
					"Route %d had no name\n", nroutes);
			}

			{
				if ((crEntry.crflags&(CRF_CALLORIGIN|CRF_CALLDEST|CRF_TRANSIT)) == 0)	
				{
					crEntry.crflags |= CRF_CALLDEST;
				}
				/* Correct for bug which was turning on CALLDEST if TRANSIT was set */
				if ((crEntry.crflags & CRF_TRANSIT) != 0)	
				{
					crEntry.crflags &= ~CRF_CALLDEST;
				}

				if (CacheHandleCR(&crEntry, clicmdop) < 0)
				{
					fprintf(stdout,
						"Unable to store cr %s in cache\n",
						crEntry.crname);
				}

				/* Store in the database */
				if (DbOperate(crdb, (char*)&crEntry, sizeof(crEntry), 
					(char*)&crEntry, sizeof(VpnRouteKey), clicmdop, 1))
				{
					fprintf(stdout,
						"Unable to %s cr %s in db\n",
						DbOperToStr(clicmdop),
						crEntry.crname);
					nerrors ++;
				}
				print_progress();
			}

			parse_cr = 0;
			nroutes ++;
		}

cpb	: _CPB
		{
			if (!ProperNesting())
			{
				fprintf(stdout, "db not nested properly at binding #%d\n",
					nbindings);
			}

			if (!cpbdb)
			{
					// Opening a new database ?
					// Check if others need to be closed
					ResetDatabaseParsing();
	
     				cpbdbstruct.read_write = GDBM_WRCREAT;
  	  				//CLIPRINTF((stdout, "opening %s\n", cpbdbfilename));
     				if (!(cpbdb = DbOpenByID(cpbdbfilename, DB_eCallPlanBind, &cpbdbstruct)))
     				{
    					CLIPRINTF((stdout, "Unable to open %s\n", cpbdbfilename));
	    				return -1;
     				}
			}
			parse_cpb = 1;
			memset(&cpbEntry, 0, sizeof(cpbEntry));

			cpbEntry.sTime.tm_year = TM_ANY;
			cpbEntry.sTime.tm_yday = TM_ANY;
			cpbEntry.sTime.tm_wday = TM_ANY;
            cpbEntry.sTime.tm_mon = TM_ANY;
            cpbEntry.sTime.tm_mday = TM_ANY;
            cpbEntry.sTime.tm_hour = TM_ANY;
            cpbEntry.sTime.tm_min = TM_ANY;
            cpbEntry.sTime.tm_sec = TM_ANY;

			cpbEntry.fTime.tm_year = TM_ANY;
			cpbEntry.fTime.tm_yday = TM_ANY;
			cpbEntry.fTime.tm_wday = TM_ANY;
            cpbEntry.fTime.tm_mon = TM_ANY;
            cpbEntry.fTime.tm_mday = TM_ANY;
            cpbEntry.fTime.tm_hour = TM_ANY;
            cpbEntry.fTime.tm_min = TM_ANY;
            cpbEntry.fTime.tm_sec = TM_ANY;

			cpbEntry.mTime = time(0);
		}
	cpb_attr_list
	ENDTAG
		{
			cpbEntry.mTime = time(0);
			if ((strlen(cpbEntry.cpname) != 0) &&
				(strlen(cpbEntry.crname) != 0))
			{
				// Test if we need to set the time flag
				CPBTestTime(&cpbEntry);

				if (CacheHandleCPB(&cpbEntry, clicmdop) < 0)
				{
					fprintf(stdout,
						"Unable to store cpb %s %s in cache\n",
						cpbEntry.cpname, cpbEntry.crname);
				}

				/* Store in the database */
				if (DbOperate(cpbdb, (char*)&cpbEntry, sizeof(cpbEntry), 
					(char*)&cpbEntry, sizeof(CallPlanBindKey), clicmdop, 1))
				{
					fprintf(stdout,
						"Unable to %s cpb %s %s in db\n",
						DbOperToStr(clicmdop),
						cpbEntry.cpname, cpbEntry.crname);
					nerrors ++;
				}
				print_progress();
			}
			parse_cpb = 0;
			nbindings ++;
		}

trg	: _TRG
		{
			if (!ProperNesting())
			{
				fprintf(stdout, "db not nested properly at trigger #%d\n",
					ntriggers);
			}

			if (!trgdb)
			{
					// Opening a new database ?
					// Check if others need to be closed
					ResetDatabaseParsing();
	
     				trgdbstruct.read_write = GDBM_WRCREAT;
  	  				//CLIPRINTF((stdout, "opening %s\n", cpbdbfilename));
     				if (!(trgdb = DbOpenByID(trgdbfilename, DB_eTrigger, &trgdbstruct)))
     				{
    					CLIPRINTF((stdout, "Unable to open %s\n", trgdbfilename));
	    				return -1;
     				}
			}
			parse_trg = 1;
			memset(&triggerEntry, 0, sizeof(triggerEntry));

		}
	trg_attr_list
	ENDTAG
		{
			if (triggerEntry.name[0] != '\0')
			{
				if (CacheHandleTrigger(&triggerEntry, clicmdop) < 0)
				{
					fprintf(stdout,
						"Unable to store trigger %s in cache\n",
						triggerEntry.name);
				}

				/* Store in the database */
				if (DbOperate(trgdb, (char*)&triggerEntry, sizeof(triggerEntry), 
					(char*)&triggerEntry, sizeof(TriggerKey), clicmdop, 1))
				{
					fprintf(stdout,
						"Unable to %s trigger %s in db\n",
						DbOperToStr(clicmdop),
						triggerEntry.name);
					nerrors ++;
				}
				print_progress();
			}
			parse_trg = 0;
			ntriggers ++;
		}

realm	: _RM
		{
			if (!realmdb)
			{
					// Opening a new database ?
					// Check if others need to be closed
					ResetDatabaseParsing();
	
     				realmdbstruct.read_write = GDBM_WRCREAT;
  	  				//CLIPRINTF((stdout, "opening %s\n", vpnsdbfilename));
     				if (!(realmdb = DbOpenByID(realmdbfilename, DB_eRealm, &realmdbstruct)))
     				{
    					CLIPRINTF((stdout, "Unable to open %s\n", realmdbfilename));
    					return -1;
     				}
			}
			parse_realm = 1;
			memset(&rmEntry, 0, sizeof(RealmEntry));
		}
	realm_attr_list 
	ENDTAG
		{
			if (rmEntry.realmId != 0)
			{
				if (strcmp(rmEntry.ifName, REALM_UNASSIGNED) && !strcmp(rmEntry.vnetName, VNET_UNASSIGNED))
				{
					if (!vnetdb)
					{
     						vnetdbstruct.read_write = GDBM_WRCREAT;
     						if (!(vnetdb = DbOpenByID(vnetdbfilename, DB_eVnet, &vnetdbstruct)))
     						{
    							CLIPRINTF((stdout, "Unable to open %s\n", vnetdbfilename));
    							return -1;
     						}
					}

					memset(&vnetEntry, 0, sizeof(VnetEntry));
					sprintf(vnetEntry.vnetName, "%s-novlan", rmEntry.ifName);
					strcpy(vnetEntry.ifName, rmEntry.ifName);
					strcpy(rmEntry.vnetName, vnetEntry.vnetName);

					if (!DbFindEntry (vnetdb, vnetEntry.vnetName, sizeof(VnetKey)))
					{
						if (DbOperate(vnetdb, (char*)&vnetEntry, sizeof(vnetEntry), 
							vnetEntry.vnetName, sizeof(VnetKey), clicmdop, 1))
						{
							fprintf(stdout, "Unable to %s Vnet %s in db\n", DbOperToStr(clicmdop),
								vnetEntry.vnetName);
							nerrors ++;
						}
						nvnets ++;

						if (CacheHandleVnet(&vnetEntry, clicmdop) < 0)
						{
							fprintf(stdout, "Unable to store vnet %s in cache\n", vnetEntry.vnetName);
						}
					}
				}

				if (CacheHandleRealm(&rmEntry, clicmdop) < 0)
				{
					fprintf(stdout,
						"Unable to store realm %s in cache\n",
						rmEntry.realmName);
				}

				if (DbOperate(realmdb, (char*)&rmEntry, sizeof(rmEntry), 
					rmEntry.realmName, sizeof(RealmKey), clicmdop, 1))
				{
					fprintf(stdout,
						"Unable to %s realm %s in db\n",
						DbOperToStr(clicmdop),
						rmEntry.realmName);
					nerrors ++;
				}

				print_progress();
			}
			parse_realm = 0;
			nrealms ++;
		}

igrp : _IGRP
		{
			if (!igrpdb)
			{
					// Opening a new database ?
					// Check if others need to be closed
					ResetDatabaseParsing();
	
     				igrpdbstruct.read_write = GDBM_WRCREAT;
  	  				//CLIPRINTF((stdout, "opening %s\n", vpnsdbfilename));
     				if (!(igrpdb = DbOpenByID(igrpdbfilename, DB_eIgrp, &igrpdbstruct)))
     				{
    					CLIPRINTF((stdout, "Unable to open %s\n", igrpdbfilename));
    					return -1;
     				}
			}
			parse_igrp = 1;
			memset(&igrpEntry, 0, sizeof(IgrpInfo));
		}
	igrp_attr_list 
	ENDTAG
		{
			if (strlen(igrpEntry.igrpName))
			{
				if (CacheHandleIgrp(&igrpEntry, clicmdop) < 0)
				{
					fprintf(stdout, "Unable to store igrp %s in cache\n", igrpEntry.igrpName);
				}

				if (DbOperate(igrpdb, (char*)&igrpEntry, sizeof(igrpEntry), 
					igrpEntry.igrpName, sizeof(IgrpKey), clicmdop, 1))
				{
					fprintf(stdout, "Unable to %s igrp %s in db\n", DbOperToStr(clicmdop),
						igrpEntry.igrpName);
					nerrors ++;
				}
				print_progress();
			}
			parse_igrp= 0;
			nigrps ++;
		}

vnet : _VNET
		{
			if (!vnetdb)
			{
					// Opening a new database ?
					// Check if others need to be closed
					ResetDatabaseParsing();
	
     				vnetdbstruct.read_write = GDBM_WRCREAT;
  	  				//CLIPRINTF((stdout, "opening %s\n", vnetdbfilename));
     				if (!(vnetdb = DbOpenByID(vnetdbfilename, DB_eVnet, &vnetdbstruct)))
     				{
    					CLIPRINTF((stdout, "Unable to open %s\n", vnetdbfilename));
    					return -1;
     				}
			}
			parse_vnet = 1;
			memset(&vnetEntry, 0, sizeof(VnetEntry));
		}
	vnet_attr_list 
	ENDTAG
		{
			if (strlen(vnetEntry.vnetName))
			{
				if (CacheHandleVnet(&vnetEntry, clicmdop) < 0)
				{
					fprintf(stdout, "Unable to store vnet %s in cache\n", vnetEntry.vnetName);
				}

				if (DbOperate(vnetdb, (char*)&vnetEntry, sizeof(vnetEntry), 
					vnetEntry.vnetName, sizeof(VnetKey), clicmdop, 1))
				{
					fprintf(stdout, "Unable to %s Vnet %s in db\n", DbOperToStr(clicmdop),
						vnetEntry.vnetName);
					nerrors ++;
				}
				print_progress();
			}
			parse_vnet= 0;
			nvnets ++;
		}

iedge_attr_list	: iedge_attr iedge_attr_list
		| iedge_attr
		| unknown_attr	
		;

vpn_attr_list	: vpn_attr vpn_attr_list
		| vpn_attr
		| unknown_attr	
		;

vpng_attr_list	: vpng_attr vpng_attr_list
		| vpng_attr
		| unknown_attr	
		;

cp_attr_list	: cp_attr cp_attr_list
		| cp_attr
		| unknown_attr	
		;

cr_attr_list	: cr_attr cr_attr_list
		| cr_attr
		| unknown_attr	
		;

cpb_attr_list	: cpb_attr cpb_attr_list
		| cpb_attr
		| unknown_attr	
		;

trg_attr_list	: trg_attr trg_attr_list
		| trg_attr
		| unknown_attr	
		;

realm_attr_list	: realm_attr realm_attr_list
		| realm_attr
		;

igrp_attr_list	: igrp_attr igrp_attr_list
		| igrp_attr
		| unknown_attr	
		;

vnet_attr_list	: vnet_attr vnet_attr_list
		| vnet_attr
		| unknown_attr	
		;


iedge_attr	: e_reg_id
		| e_uport
		| e_itime
		| e_rtime
		| e_sflags
		| e_mtime
		| e_et
		| e_state
		| e_phone
		| e_vpn_phone
		| e_vpn_ext_len	
		| v_vpn_name
		| e_ipaddr
		| e_subnetip
		| e_subnetmask
		| e_zone
		| e_email
		| e_nphone
		| e_xphone
		| e_nvpn_phone
		| e_nvpn_ext_len
		| e_xvpn_phone
		| e_fname
		| e_lname
		| e_location
		| e_country
		| e_comments
		| e_h323id
		| e_pgkid
		| e_techp
		| e_contact
		| e_sipuser
		| e_uri
		| cp_name
		| e_cap
		| e_cap2
		| e_ecaps1
		| e_password
		| e_ras 
		| e_rasport
		| e_sigport
		| e_xcalls
		| e_infotranscap
		| e_xincalls
		| e_xoutcalls
		| e_prio
		| e_vendor
		| e_xhunts
		| e_crid
		| e_custid
		| e_cdpntype
		| e_cgpntype
		| e_bcap
		| e_tg
		| e_srcitg
		| e_dtg
		| e_srcetg
		| e_ogp
		| e_realm
		| e_realmid
		| e_igroup
		| e_natip
		| e_natport
                | e_cidblk
		;

vpn_attr	: v_vpn_id
		| v_vpn_name
		| v_vpn_loc
		| v_vpn_contact
		| v_vpn_extlen
		| v_vpn_prefix
		| v_vpn_sipdomain
		| e_mtime
		| vg_vpn_group
		;

vpng_attr	: vg_vpn_group
		| e_mtime
		;

cp_attr		: cp_name
		| vg_vpn_group
		| e_mtime
		;

cr_attr		: cp_name
		| cr_name
		| cr_src
		| cr_srclen
		| cr_dest
		| cr_destlen
		| cr_prefix
		| cr_srcprefix
		| cr_cpname
		| cr_trname
		| cr_flags
		| e_mtime
		;

cpb_attr	: cp_name
		| cr_name
		| cr_flags
		| cr_stime
		| cr_ftime
		| e_prio
		| e_mtime
		;

trg_attr	: trg_name
		| trg_event
		| trg_svend
		| trg_dvend
		| trg_act
		| trg_actd
		| trg_actflags

realm_attr	: realm_name
		| realm_id
		| realm_rsa
		| realm_mask
		| realm_sigpoolid
		| realm_medpoolid
		| realm_authflags
		| realm_astatus
		| realm_ostatus
		| realm_addr
		| realm_imr
		| realm_emr
		| realm_ifname
		| realm_lifname
                | realm_cidblk_prefix
                | realm_cidunblk_prefix
                | realm_proxy_regid
                | realm_proxy_uport
                | realm_vnetname
		;

igrp_attr	: igrp_name
		| igrp_maxin
		| igrp_maxout
		| igrp_maxtotal
		| igrp_in
		| igrp_out
		| igrp_total
		| igrp_time
		;

vnet_attr	: vnet_vnetname
		| vnet_ifname
		| vnet_vlanid
		| vnet_rtgtblid
		| vnet_gateway
		;

e_reg_id	: _X_SRNO STRING 
			{
				strncpy(netInfo.regid, &yytext[1], 
					REG_ID_LEN);
				netInfo.regid[REG_ID_LEN-1] = '\0';

				BIT_SET(netInfo.sflags, ISSET_REGID);
			}
		ENDTAG

e_uport	: _X_UPORT STRING
			{
				netInfo.uport = $2;
				BIT_SET(netInfo.sflags, ISSET_UPORT);
			}
		ENDTAG

e_et	: _X_ET STRING 
			{
				int type = FindIedgeType(&yytext[1]);

				/* Take care of backward compatibility */
				if (type >= 0)
				{
					SetIedgeType(&netInfo, type);
				}
				else if (!strcmp(&yytext[1], "PC"))
				{
					netInfo.ispcorgw = 1;
				} 
				else if (!strcmp(&yytext[1], "GW"))
				{
					netInfo.ispcorgw = 2;
				}
			}
		ENDTAG

e_state	: _X_FW ENDTAG
		{
			netInfo.stateFlags |= CL_FORWARD;
		}
	| _X_FP STRING 
		{
			if (!strcmp(&yytext[1], "ROLLOVER"))
			{
				netInfo.protocol = NEXTONE_REDIRECT_ROLLOVER;
			}
		}
	ENDTAG
	| _X_PXS ENDTAG
		{
			netInfo.stateFlags |= CL_PROXY;
		}
	| _X_PXC ENDTAG
		{
			netInfo.stateFlags |= CL_PROXYING;
		}
	| _X_DND ENDTAG
		{
			netInfo.stateFlags |= CL_DND;
		}
	| _X_STATIC ENDTAG
		{
			netInfo.stateFlags |= CL_STATIC;
		}
	| _X_DYO ENDTAG
		{
			netInfo.stateFlags |= CL_DYNAMIC;
		}
	;

e_itime	: _X_ITIME STRING 
			{
				netInfo.iTime = $2;
				netInfo.rTime = netInfo.iTime;
			}
	ENDTAG

e_rtime	: _X_RTIME STRING 
			{
				netInfo.rTime = $2;
			}
	ENDTAG

e_sflags : _X_SFLAGS STRING 
			{
				unsigned short  sflags = $2;
				if (sflags & CL_ACTIVE)
				{
					netInfo.stateFlags |= CL_ACTIVE;
				}
			}
	ENDTAG

e_mtime	: _X_MTIME STRING 
			{
				netInfo.mTime = $2;
				vpnEntry.mTime = $2;
				vpnGroupEntry.mTime = $2;
				cpEntry.mTime = $2;
				crEntry.mTime = $2;
			}
	ENDTAG

e_phone	: _X_PHONE STRING
			{
				strncpy(netInfo.phone, &yytext[1], 
					PHONE_NUM_LEN);
				netInfo.phone[PHONE_NUM_LEN-1] = '\0';
				BIT_SET(netInfo.sflags, ISSET_PHONE);
			}
	ENDTAG

e_vpn_phone	: _X_VPN_PHONE STRING
			{
				strncpy(netInfo.vpnPhone, &yytext[1],
					VPN_LEN);
				netInfo.vpnPhone[VPN_LEN-1] = '\0';
				BIT_SET(netInfo.sflags, ISSET_VPNPHONE);
			}
	ENDTAG

e_vpn_ext_len	: _X_VPN_EXTLEN STRING ENDTAG
			{
				netInfo.vpnExtLen = $2;
			}

e_ipaddr	: _X_IPADDR STRING 
			{
				netInfo.ipaddress.l = ntohl(inet_addr(&yytext[1]));
				BIT_SET(netInfo.sflags, ISSET_IPADDRESS);
			}
		ENDTAG

e_subnetip	: _X_SUBNETIP STRING 
			{
				netInfo.subnetip = ntohl(inet_addr(&yytext[1]));
			}
		ENDTAG

e_subnetmask	: _X_SUBNETMASK STRING 
			{
				netInfo.subnetmask = ntohl(inet_addr(&yytext[1]));
			}
		ENDTAG

e_zone	: _X_ZONE STRING
			{
				strncpy(netInfo.zone, &yytext[1], 
					ZONE_LEN);
				netInfo.zone[ZONE_LEN-1] = '\0';
			}
	ENDTAG

e_email	: _X_EMAIL STRING
			{
				strncpy(netInfo.email, &yytext[1], 
					EMAIL_LEN);
				netInfo.email[EMAIL_LEN-1] = '\0';
				BIT_SET(netInfo.sflags, ISSET_EMAIL);
			}
	ENDTAG

e_nphone	: _X_NPHONE STRING
			{
				strncpy(netInfo.nphone, &yytext[1], 
					PHONE_NUM_LEN);
				netInfo.nphone[PHONE_NUM_LEN-1] = '\0';
				BIT_SET(netInfo.nsflags, ISSET_PHONE);
			}
		ENDTAG

e_nvpn_phone	: _X_NVPN_PHONE STRING
			{
#if 0
				strncpy(netInfo.nvpnPhone, &yytext[1],
					VPN_LEN);
				netInfo.nvpnPhone[VPN_LEN-1] = '\0';
				BIT_SET(netInfo.nsflags, ISSET_VPNPHONE);
#endif
			}
		ENDTAG

e_xphone	: _X_XPHONE STRING
			{
#if 0
				strncpy(netInfo.xphone, &yytext[1], 
					PHONE_NUM_LEN);
				netInfo.xphone[PHONE_NUM_LEN-1] = '\0';
#endif
			}
		ENDTAG

e_xvpn_phone	: _X_XVPN_PHONE STRING
			{
#if 0
				strncpy(netInfo.xvpnPhone, &yytext[1],
					VPN_LEN);
				netInfo.xvpnPhone[VPN_LEN-1] = '\0';
#endif
			}
		ENDTAG

e_nvpn_ext_len	: _X_NVPN_EXTLEN STRING ENDTAG
			{
#if 0
				netInfo.nvpnExtLen = $2;
#endif
			}

e_h323id	: _X_H323ID STRING 
			{
				strncpy(netInfo.h323id, &yytext[1], 
					H323ID_LEN);
				netInfo.h323id[H323ID_LEN-1] = '\0';
			}
		ENDTAG

e_pgkid	: _X_PGKID STRING 
			{
				strncpy(netInfo.pgkid, &yytext[1], 
					GKID_LEN);
				netInfo.pgkid[GKID_LEN-1] = '\0';
			}
		ENDTAG

e_techp	: _X_TECHP STRING 
			{
				strncpy(netInfo.techprefix, &yytext[1], 
					PHONE_NUM_LEN);
				netInfo.techprefix[PHONE_NUM_LEN-1] = '\0';
			}
		ENDTAG

e_uri		: _X_URI STRING 
			{
				strncpy(netInfo.uri, &yytext[1], 
					SIPURL_LEN);
				netInfo.uri[SIPURL_LEN-1] = '\0';
			}
		ENDTAG

e_sipuser	: _X_SIPUSER STRING 
			{
				// This field no longer used
				// strncpy(netInfo.sipuser, &yytext[1], 
				// 	PHONE_NUM_LEN);
				// netInfo.sipuser[PHONE_NUM_LEN-1] = '\0';
			}
		ENDTAG

e_contact	: _X_CONTACT STRING 
			{
				strncpy(netInfo.contact, &yytext[1], 
					SIPURL_LEN);
				netInfo.contact[SIPURL_LEN-1] = '\0';
			}
		ENDTAG

e_fname	: _X_FNAME STRING
			{
				strncpy(clAttribs.clFname, &yytext[1], 
					CLIENT_ATTR_LEN);
				clAttribs.clFname[CLIENT_ATTR_LEN-1] = '\0';
			}
		ENDTAG

e_lname	: _X_LNAME STRING
			{
				strncpy(clAttribs.clLname, &yytext[1], 
					CLIENT_ATTR_LEN);
				clAttribs.clLname[CLIENT_ATTR_LEN-1] = '\0';
			}
		ENDTAG

e_location	: _X_LOCATION STRING
			{
				strncpy(clAttribs.clLoc, &yytext[1], 
					CLIENT_ATTR_LEN);
				clAttribs.clLoc[CLIENT_ATTR_LEN-1] = '\0';
			}
		ENDTAG

e_country	: _X_COUNTRY STRING
			{
				strncpy(clAttribs.clCountry, &yytext[1], 
					CLIENT_ATTR_LEN);
				clAttribs.clCountry[CLIENT_ATTR_LEN-1] = '\0';
			}
		ENDTAG

e_comments	: _X_COMMENTS STRING
			{
				strncpy(clAttribs.clComments, &yytext[1], 
					CLIENT_ATTR_LEN);
				clAttribs.clComments[CLIENT_ATTR_LEN-1] = '\0';
			}
		ENDTAG

e_vendor	: _X_VENDOR STRING ENDTAG
			{
				netInfo.vendor = $2;
			}

e_password	: _X_PASSWORD STRING 
			{
				nx_strlcpy(netInfo.passwd, &yytext[1], 
					PASSWD_LEN);
			}
		ENDTAG

e_cap	: _X_CAP STRING ENDTAG
			{
				netInfo.cap = $2;
			}

e_cap2	: _X_CAP2 STRING ENDTAG
			{
				netInfo.cap = ntohs($2); 
			}

e_ecaps1	: _X_ECAPS1 STRING ENDTAG
			{
				netInfo.ecaps1 = $2;
			}

e_ras	: _X_RASIP STRING ENDTAG
			{
				netInfo.rasip = $2;
			}
e_rasport	: _X_RASPORT STRING ENDTAG
			{
				netInfo.rasport = $2;
			}

e_sigport	: _X_SIGPORT STRING ENDTAG
			{
				netInfo.callsigport = $2;
			}

e_xcalls	: _X_XCALLS STRING ENDTAG
			{
				IedgeXCalls(&netInfo) = $2;
			}

e_infotranscap	: _X_INFOTRANSCAP STRING ENDTAG
			{
				IedgeInfoTransCap(&netInfo) = $2;
			}

e_xincalls	: _X_XINCALLS STRING ENDTAG
			{
				IedgeXInCalls(&netInfo) = $2;
			}

e_xoutcalls	: _X_XOUTCALLS STRING ENDTAG
			{
				IedgeXOutCalls(&netInfo) = $2;
			}

e_xhunts	: _X_MAXHUNTS STRING ENDTAG
			{
				netInfo.maxHunts = $2;
			}

e_crid		: _X_CRID STRING ENDTAG
			{
				netInfo.crId = $2;
			}

e_custid	: _X_CUSTID STRING 
			{
				nx_strlcpy(netInfo.custID, &yytext[1],
					CLIENT_ATTR_LEN);
			}
		ENDTAG

e_cdpntype	: _X_CDPNTYPE STRING ENDTAG
			{
				netInfo.q931IE[Q931IE_CDPN] = $2;
			}

e_cgpntype	: _X_CGPNTYPE STRING ENDTAG
			{
				netInfo.q931IE[Q931IE_CGPN] = $2;
			}

e_bcap	: _X_BCAP STRING ENDTAG
			{
				*(unsigned int *)(netInfo.bcap) = $2;
			}

e_tg	: _X_TG STRING 
			{
				nx_strlcpy(netInfo.tg, &yytext[1],
					PHONE_NUM_LEN);
			}
		ENDTAG

e_srcitg	: _X_SRCITG STRING 
			{
				nx_strlcpy(netInfo.srcIngressTG, &yytext[1],
					PHONE_NUM_LEN);
			}

		ENDTAG

e_dtg	: _X_DTG STRING 
			{
				nx_strlcpy(netInfo.dtg, &yytext[1],
					PHONE_NUM_LEN);
			}
		ENDTAG

e_srcetg	: _X_SRCETG STRING 
			{
				nx_strlcpy(netInfo.srcEgressTG, &yytext[1],
					PHONE_NUM_LEN);
			}

		ENDTAG

e_ogp	: _X_OGP STRING 
			{
				nx_strlcpy(netInfo.ogprefix, &yytext[1],
					PHONE_NUM_LEN);
			}
		ENDTAG

e_realm	: _X_REALM STRING 
			{
				nx_strlcpy(netInfo.realmName, &yytext[1],
					REALM_NAME_LEN);
			}
		ENDTAG

e_realmid	: _X_REALMID STRING 
			{
				netInfo.realmId = $2;
			}
		ENDTAG

e_igroup	: _X_IGROUP STRING 
			{
				nx_strlcpy(netInfo.igrpName, &yytext[1],
					IGRP_NAME_LEN);
			}
		ENDTAG

e_natip		: _X_NATIP STRING
			{
				netInfo.natIp = ntohl(inet_addr(&yytext[1]));
				BIT_SET(netInfo.sflags, ISSET_NATIP);
			}
		ENDTAG

e_natport	: _X_NATPORT STRING
			{
				netInfo.natPort = $2;
				BIT_SET(netInfo.sflags, ISSET_NATPORT);
			}
		ENDTAG

e_prio	: _X_PRIO STRING ENDTAG
			{
				netInfo.priority = $2;
				cpbEntry.priority = $2;
			}

e_igroup : _X_IGROUP STRING
			{
				nx_strlcpy(netInfo.igrpName, &yytext[1], 
						IGRP_NAME_LEN);
			}
			ENDTAG
e_cidblk        : _X_CIDBLK STRING
                        {
				netInfo.cidblock = $2;
                        }
                   ENDTAG

v_vpn_id	: _X_VPN_ID STRING
			{
				strncpy(vpnEntry.vpnId, &yytext[1], VPN_LEN);
				vpnEntry.vpnId[VPN_LEN-1] = '\0';
			}
		ENDTAG

v_vpn_name	: _X_VPN_NAME STRING
			{
				strncpy(vpnEntry.vpnName, &yytext[1], VPNS_ATTR_LEN);
				vpnEntry.vpnName[VPNS_ATTR_LEN-1] = '\0';

				strncpy(netInfo.vpnName, &yytext[1], VPNS_ATTR_LEN);
				netInfo.vpnName[VPNS_ATTR_LEN-1] = '\0';
			}
		ENDTAG

v_vpn_prefix: _X_VPN_PREFIX STRING 
			{
				strncpy(vpnEntry.prefix, &yytext[1], PHONE_NUM_LEN);
				vpnEntry.prefix[PHONE_NUM_LEN-1] = '\0';
			}
		ENDTAG

v_vpn_sipdomain : _X_VPN_SIPDOMAIN STRING 
			{
				strncpy(vpnEntry.sipdomain, &yytext[1], SIPURL_LEN);
				vpnEntry.sipdomain[SIPURL_LEN-1] = '\0';
			}
		ENDTAG

v_vpn_loc	: _X_VPN_LOC STRING
			{
				strncpy(vpnEntry.vpnLoc, &yytext[1], VPNS_ATTR_LEN);
				vpnEntry.vpnLoc[VPNS_ATTR_LEN-1] = '\0';
			}
		ENDTAG

v_vpn_contact	: _X_VPN_CONTACT STRING
			{
				strncpy(vpnEntry.vpnContact, &yytext[1], VPNS_ATTR_LEN);
				vpnEntry.vpnContact[VPNS_ATTR_LEN-1] = '\0';
			}
		ENDTAG

v_vpn_extlen	: _X_VPN_EXTLEN STRING ENDTAG
			{
				vpnEntry.vpnExtLen = $2;
			}

vg_vpn_group	: _X_VPN_GROUP STRING
			{
				strncpy(vpnEntry.vpnGroup, &yytext[1], 
					VPN_GROUP_LEN-1);
				strncpy(vpnGroupEntry.vpnGroup, &yytext[1], 
					VPN_GROUP_LEN-1);
				strncpy(cpEntry.vpnGroup, &yytext[1],
					VPN_GROUP_LEN-1);
			}
		ENDTAG

cp_name		: _X_CP_NAME STRING
			{
				strncpy(cpEntry.cpname, &yytext[1],
					CALLPLAN_ATTR_LEN-1);
				strncpy(cpbEntry.cpname, &yytext[1],
					CALLPLAN_ATTR_LEN-1);
				strncpy(netInfo.cpname, &yytext[1],
					CALLPLAN_ATTR_LEN-1);
			}
		ENDTAG

cr_name		: _X_CR_NAME STRING
			{
				strncpy(crEntry.crname, &yytext[1],
					CALLPLAN_ATTR_LEN-1);
				strncpy(cpbEntry.crname, &yytext[1],
					CALLPLAN_ATTR_LEN-1);
			}
		ENDTAG

cr_dest		: _X_CR_DEST STRING
			{
				strncpy(crEntry.dest, &yytext[1],
					VPN_LEN-1);
			}
		ENDTAG

cr_destlen		: _X_CR_DESTLEN STRING
			{
				crEntry.destlen = $2;
			}
		ENDTAG

cr_src		: _X_CR_SRC STRING
			{
				strncpy(crEntry.src, &yytext[1],
					VPN_LEN-1);
			}
		ENDTAG

cr_srclen		: _X_CR_SRCLEN STRING
			{
				crEntry.srclen = $2;
			}
		ENDTAG

cr_prefix	: _X_CR_PREFIX STRING
			{
				strncpy(crEntry.prefix, &yytext[1],
					PHONE_NUM_LEN-1);
			}
		ENDTAG

cr_srcprefix: _X_CR_SRCPREFIX STRING
			{
				strncpy(crEntry.srcprefix, &yytext[1],
					PHONE_NUM_LEN-1);
			}
		ENDTAG

cr_cpname: _X_CR_CPNAME STRING
			{
				strncpy(crEntry.cpname, &yytext[1],
					CALLPLAN_ATTR_LEN-1);
			}
		ENDTAG

cr_trname: _X_CR_TRNAME STRING
			{
				strncpy(crEntry.trname, &yytext[1],
					TRIGGER_ATTR_LEN-1);
			}
		ENDTAG

cr_flags	: _X_CR_FLAGS STRING
			{
				crEntry.crflags = $2;
				cpbEntry.crflags = $2;
			}
		ENDTAG

cr_stime	: _X_CR_STIME STRING
			{
				char storec;

				sscanf(&yytext[1], "%d%c%d%c%d%c%d%c%d%c%d%c%d%c%d",
					&cpbEntry.sTime.tm_year,
					&storec,
					&cpbEntry.sTime.tm_yday,
					&storec,
					&cpbEntry.sTime.tm_wday,
            		&storec,
            		&cpbEntry.sTime.tm_mon,
            		&storec,
            		&cpbEntry.sTime.tm_mday,
            		&storec,
            		&cpbEntry.sTime.tm_hour,
            		&storec,
            		&cpbEntry.sTime.tm_min,
            		&storec,
            		&cpbEntry.sTime.tm_sec);
			}
		ENDTAG

cr_ftime	: _X_CR_FTIME STRING
			{
				char storec;

        		sscanf(&yytext[1], "%d%c%d%c%d%c%d%c%d%c%d%c%d%c%d",
            		&cpbEntry.fTime.tm_year,
            		&storec,
            		&cpbEntry.fTime.tm_yday,
            		&storec,
            		&cpbEntry.fTime.tm_wday,
            		&storec,
            		&cpbEntry.fTime.tm_mon,
            		&storec,
            		&cpbEntry.fTime.tm_mday,
            		&storec,
            		&cpbEntry.fTime.tm_hour,
            		&storec,
            		&cpbEntry.fTime.tm_min,
            		&storec,
            		&cpbEntry.fTime.tm_sec);
			}
		ENDTAG

trg_name	: _X_TRG_NAME STRING
			{
				nx_strlcpy(triggerEntry.name, &yytext[1], TRIGGER_ATTR_LEN);
			}
		ENDTAG
		
trg_event	: _X_TRG_EVT STRING
			{
				triggerEntry.event = atoi(&yytext[1]);
			}
		ENDTAG

trg_svend	: _X_TRG_SVEND STRING
			{
				triggerEntry.srcvendor = atoi(&yytext[1]);
			}
		ENDTAG

trg_dvend	: _X_TRG_DVEND STRING
			{
				triggerEntry.dstvendor = atoi(&yytext[1]);
			}
		ENDTAG

trg_act	: _X_TRG_ACT STRING
			{
				triggerEntry.action = atoi(&yytext[1]);
			}
		ENDTAG

trg_actd	: _X_TRG_ACTD STRING
			{
				nx_strlcpy(triggerEntry.actiondata, &yytext[1], 
					TRIGGER_ATTR_LEN);
			}
		ENDTAG

trg_actflags	: _X_TRG_ACTFLAGS STRING
			{
				triggerEntry.actionflags = atoi(&yytext[1]);
			}
		ENDTAG

realm_name : _X_RM_NAME STRING
			{
				strncpy(rmEntry.realmName, &yytext[1], REALM_NAME_LEN);
			}
			ENDTAG

realm_id	: _X_RM_ID STRING ENDTAG
			{
				rmEntry.realmId = $2;
			}

realm_rsa	: _X_RM_RSA STRING 
			{
				rmEntry.rsa = ntohl(inet_addr(&yytext[1]));
			}
			ENDTAG

realm_mask	: _X_RM_MASK STRING 
			{
				rmEntry.mask = ntohl(inet_addr(&yytext[1]));
			}
			ENDTAG

realm_sigpoolid : _X_RM_SIGPOOL STRING ENDTAG
			{
				rmEntry.sigPoolId = $2;
			}

realm_medpoolid : _X_RM_MEDPOOL STRING ENDTAG
			{
				rmEntry.medPoolId = $2;
			}

realm_authflags : _X_RM_AUTHFLAGS STRING ENDTAG
			{
				rmEntry.authFlags = $2;
			}

realm_astatus : _X_RM_ASTATUS STRING ENDTAG
			{
				rmEntry.adminStatus = $2;
			}

realm_ostatus : _X_RM_OSTATUS STRING ENDTAG
			{
				rmEntry.operStatus = $2;
			}

realm_addr : _X_RM_ADDR STRING ENDTAG
			{
				rmEntry.addrType = $2;
			}

realm_imr : _X_RM_IMR STRING ENDTAG
			{
				rmEntry.interRealm_mr = $2;
			}

realm_emr : _X_RM_EMR STRING ENDTAG
			{
				rmEntry.intraRealm_mr = $2;
			}

realm_ifname : _X_RM_IFNAME STRING
			{
				strncpy(rmEntry.ifName, &yytext[1], IFI_NAME);
			}
		ENDTAG
realm_lifname : _X_RM_LIFNAME STRING
			{
				strncpy(rmEntry.vipName, &yytext[1], IFI_NAME);
			}
		ENDTAG
realm_cidblk_prefix : _X_RM_CIDBLK_PREFIX STRING
			{
				strncpy(rmEntry.cidblk, &yytext[1], CID_BLK_UNBLK_LEN);
			}
		ENDTAG
realm_cidunblk_prefix : _X_RM_CIDUNBLK_PREFIX STRING
			{
				strncpy(rmEntry.cidunblk, &yytext[1], CID_BLK_UNBLK_LEN);
			}
		ENDTAG
realm_proxy_regid : _X_RM_PROXY_REGID STRING 
			{
				strncpy(rmEntry.mp.regid, &yytext[1],REG_ID_LEN );
				rmEntry.mp.regid[REG_ID_LEN-1]='\0';
			}
		ENDTAG
realm_proxy_uport : _X_RM_PROXY_UPORT STRING 
			{
				rmEntry.mp.uport = $2;
			}
		ENDTAG

realm_vnetname : _X_RM_VNETNAME STRING
			{
				strncpy(rmEntry.vnetName, &yytext[1], VNET_NAME_LEN);
			}
		ENDTAG

igrp_name : _X_IGRP_NAME STRING
			{
				strncpy(igrpEntry.igrpName, &yytext[1], IGRP_NAME_LEN);
			}
		ENDTAG

igrp_maxin : _X_IGRP_MAXIN STRING ENDTAG
			{
				igrpEntry.nMaxCallsIn = $2;
			}

igrp_maxout : _X_IGRP_MAXOUT STRING ENDTAG
			{
				igrpEntry.nMaxCallsOut = $2;
			}

igrp_maxtotal : _X_IGRP_MAXTOTAL STRING ENDTAG
			{
				igrpEntry.nMaxCallsTotal = $2;
			}

igrp_in : _X_IGRP_IN STRING ENDTAG
			{
				igrpEntry.nCallsIn = $2;
			}

igrp_out : _X_IGRP_OUT STRING ENDTAG
			{
				igrpEntry.nCallsOut = $2;
			}

igrp_total : _X_IGRP_TOTAL STRING ENDTAG
			{
				igrpEntry.nCallsTotal = $2;
			}

igrp_time : _X_IGRP_TIME STRING ENDTAG
			{
				igrpEntry.dndTime = $2;
			}

vnet_vnetname : _X_VNET_VNETNAME STRING
			{
				strncpy(vnetEntry.vnetName, &yytext[1], VNET_NAME_LEN);
			}
		ENDTAG

vnet_ifname : _X_VNET_IFNAME STRING
			{
				strncpy(vnetEntry.ifName, &yytext[1], IFI_NAME);
			}
		ENDTAG

vnet_vlanid : _X_VNET_VLANID STRING ENDTAG
			{
				vnetEntry.vlanid = $2;
			}

vnet_rtgtblid : _X_VNET_RTGTBLID STRING ENDTAG
			{
				vnetEntry.rtgTblId = $2;
			}

vnet_gateway : _X_VNET_GATEWAY STRING 
			{
				vnetEntry.gateway = ntohl(inet_addr(&yytext[1]));
			}
			ENDTAG

%%

int
ResetDatabaseParsing(void)
{
	if (netdb) DbClose(&netdbstruct);
   	if (attrdb) DbClose(&attrdbstruct);
   	if (vpnsdb) DbClose(&vpnsdbstruct);
   	if (vpngdb) DbClose(&vpngdbstruct);
   	if (cpdb) DbClose(&cpdbstruct);
   	if (crdb) DbClose(&crdbstruct);
   	if (cpbdb) DbClose(&cpbdbstruct);
   	if (trgdb) DbClose(&trgdbstruct);
   	if (realmdb) DbClose(&realmdbstruct);
   	if (igrpdb) DbClose(&igrpdbstruct);
   	if (vnetdb) DbClose(&vnetdbstruct);

	cpbdb = 0;
	crdb = 0;
	cpdb = 0;
	vpngdb = 0;
	vpnsdb = 0;
	netdb = 0;
	attrdb = 0;
	trgdb = 0;
	realmdb = 0;
	igrpdb = 0;
	vnetdb = 0;

	return 0;
}
