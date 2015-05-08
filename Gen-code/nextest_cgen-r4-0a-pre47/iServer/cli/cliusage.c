#include "cli.h"
#include "log.h"

void
HandleCommandUsage(Command *comm, int argc, char **argv)
{
	short mflags = comm->flags & cliLibFlags;

	  if (!(mflags & COMMANDF_USER))
	  {
		return;
	  }
	
	  format(stdout, comm->level);
	  PrintUsage(comm, argc, argv, comm->flags);
	  fprintf(stdout, "\n");
}

void
NetoidAddUsage(Command *comm, int argc, char **argv)
{
     fprintf(stdout, "%s ", comm->name);
     fprintf(stdout, "\tregid low[-high] [vpn-name] [vpn-name] ...");
}

void
NetoidFindUsage(Command *comm, int argc, char **argv)
{
     fprintf(stdout, "%s ", comm->name);
     fprintf(stdout, "\tregid uport");
}

void
NetoidLkupUsage(Command *comm, int argc, char **argv)
{
     fprintf(stdout, "%s ", comm->name);
     fprintf(stdout, "\tphone|regid [uport]|ip-address|url|ipaddr [realm]");
}

void
NetoidDeleteUsage(Command *comm, int argc, char **argv)
{
     fprintf(stdout, "%s ", comm->name);
     fprintf(stdout, "\tregid low[-high]");
}

void
NetoidListUsage(Command *comm, int argc, char **argv)
{
     fprintf(stdout, "%s ", comm->name);
}

void
NetoidCacheUsage(Command *comm, int argc, char **argv)
{
     fprintf(stdout, "%s ", comm->name);
     fprintf(stdout, "\t[phones|vpnphones|ip|gw|email]");
}

void
NetoidRouteUsage(Command *comm, int argc, char **argv)
{
     fprintf(stdout, "%s ", comm->name);
     fprintf(stdout, "\t{regid}|{cid:caller-id}|{realm:realm-name source-ip} [uport] [ani:ani-no] [dnis:dnis-no] [called-no] [dest-tg]\n");
     fprintf(stdout, "\t\t\tregid uport called-no\n");
     fprintf(stdout, "\t\t\tregid uport called-no dest-tg\n");

     fprintf(stdout, "\t\t\tregid ani:ani-no\n");
     fprintf(stdout, "\t\t\tregid dnis:dnis-no\n"); 

     fprintf(stdout, "\t\t\tcid:caller-id called-no\n");
     fprintf(stdout, "\t\t\tcid:caller-id called-no dest-tg\n");

     fprintf(stdout, "\t\t\trealm:realm-name source-ip called-no\n");
     fprintf(stdout, "\t\t\trealm:realm-name source-ip called-no dest-tg\n");
}

void
NetoidHuntUsage(Command *comm, int argc, char **argv)
{
     NetoidRouteUsage(comm, argc, argv);
}

void
VpnAddUsage(Command *comm, int argc, char **argv)
{
     fprintf(stdout, "%s ", comm->name);
     fprintf(stdout, "\tvpn-name vpn-id extn-len [vpn-group]");
}

void
VpnDeleteUsage(Command *comm, int argc, char **argv)
{
     fprintf(stdout, "%s ", comm->name);
     fprintf(stdout, "\tvpn-name");
}

void
VpnCacheUsage(Command *comm, int argc, char **argv)
{
     fprintf(stdout, "%s ", comm->name);
}

void
VpnEditUsage(Command *comm, int argc, char **argv)
{
     fprintf(stdout, "%s\tvpn-name", comm->name);
}

void
VpnListUsage(Command *comm, int argc, char **argv)
{
     fprintf(stdout, "%s ", comm->name);
}

void
VpnVpnGUsage(Command *comm, int argc, char **argv)
{
     fprintf(stdout, "%s ", comm->name);
     fprintf(stdout, "\tvpn-name vpn-group");
}

void
VpnGAddUsage(Command *comm, int argc, char **argv)
{
     fprintf(stdout, "%s ", comm->name);
     fprintf(stdout, "\tvpn-group");
}

void
VpnGDeleteUsage(Command *comm, int argc, char **argv)
{
     fprintf(stdout, "%s ", comm->name);
     fprintf(stdout, "\tvpn-group");
}

void
VpnGListUsage(Command *comm, int argc, char **argv)
{
     fprintf(stdout, "%s ", comm->name);
}

void
VpnGCacheUsage(Command *comm, int argc, char **argv)
{
     fprintf(stdout, "%s ", comm->name);
}

void
VpnGEditUsage(Command *comm, int argc, char **argv)
{
     fprintf(stdout, "%s\tvpn-group", comm->name);
}

void
NetoidVpnsUsage(Command *comm, int argc, char **argv)
{
	fprintf(stdout, "%s ", comm->name);
     	fprintf(stdout, "\tregid low[-high] [vpn-name] [vpn-name] ...");
}

void
NetoidVpngUsage(Command *comm, int argc, char **argv)
{
	fprintf(stdout, "%s ", comm->name);
     	fprintf(stdout, "\tregid low[-high] [vpn-id extn] [vpn-id extn] ...");
}

void
NetoidPhonesUsage(Command *comm, int argc, char **argv)
{
	fprintf(stdout, "%s ", comm->name);
     	fprintf(stdout, "\tregid low[-high] [phone] [phone] ...");
}

void
NetoidEmailUsage(Command *comm, int argc, char **argv)
{
	fprintf(stdout, "%s ", comm->name);
     	fprintf(stdout, "\tregid low[-high] [email] [email] ...");
}

void
NetoidZoneUsage(Command *comm, int argc, char **argv)
{
	fprintf(stdout, "%s ", comm->name);
     	fprintf(stdout, "\tregid low[-high] [zone] [zone] ...");
}

void
NetoidIpUsage(Command *comm, int argc, char **argv)
{
	fprintf(stdout, "%s ", comm->name);
     	fprintf(stdout, "\tregid low[-high] [ipaddr]");
}

void
NetoidProxyUsage(Command *comm, int argc, char **argv)
{
	fprintf(stdout, "%s ", comm->name);
     	fprintf(stdout, "\tregid low[-high] ");
}

void
NetoidEditHelp(Command *comm, int argc, char **argv)
{
	fprintf(stdout, "\t\t\t%s ", comm->name);
   	fprintf(stdout, "\tregid low[-high] <field> <value>\n\n");
	fprintf(stdout, "\t\t\tbcaplayer1 <default|pass|g711ulaw|g711alaw|h221> \n");
	fprintf(stdout, "\t\t\tcdpntype <default|unknown|national|international|specific|subscriber|abbreviated|pass>\n");
	fprintf(stdout, "\t\t\tcgpntype <pass|unknown|national|international|specific|subscriber|abbreviated>\n");
	fprintf(stdout, "\t\t\tcomments <string> - comments\n");
	fprintf(stdout, "\t\t\tconnh245addr <enable|disable> - h.245 addr in connect\n");
	fprintf(stdout, "\t\t\tsetdesttg <enable|disable> - set destination trunk group\n");
	fprintf(stdout, "\t\t\tcontact <URI>  - sip contact\n");
	fprintf(stdout, "\t\t\tcountry <string> - country\n");
	fprintf(stdout, "\t\t\tcp <string> - calling plan\n");
	fprintf(stdout, "\t\t\tdeltcs2833 <enable|disable|default>  - remove 2833 cap from TCS\n");
	fprintf(stdout, "\t\t\tdeltcst38 <enable|disable|default>  - remove t38Fax from TCS\n");
	fprintf(stdout, "\t\t\tdnd <enable|disable> - DND\n");
	fprintf(stdout, "\t\t\tdtg <string>  - destination trunk group\n");
	fprintf(stdout, "\t\t\tfname <string> - first name\n");
	fprintf(stdout, "\t\t\tforceh245 <enable|disable> - H.323 Force H.245\n");
	fprintf(stdout, "\t\t\tfwdno <phone> \n");
	fprintf(stdout, "\t\t\tfwdtype <forward|rollover|none> \n");
	fprintf(stdout, "\t\t\tgrq <enable|disable> - H.323 GRQ\n");
	fprintf(stdout, "\t\t\tgw <enable|disable> \n");
	fprintf(stdout, "\t\t\th323 <enable|disable> \n");
	fprintf(stdout, "\t\t\th323display <enable|disable> \n");
	fprintf(stdout, "\t\t\th323id <string> - h.323 id\n");
	fprintf(stdout, "\t\t\tigrp <igrp-name> - Iedge Group of endpoint\n");
	fprintf(stdout, "\t\t\tinfotranscap <speech | unrestricted | restricted | audio | unrestrictedtones | video | pass> \n");
	fprintf(stdout, "\t\t\tip <ipaddr> - reg ip address\n");
	fprintf(stdout, "\t\t\tlname <string> - last name\n");
	fprintf(stdout, "\t\t\tlocation <string> - location\n");
	fprintf(stdout, "\t\t\tmapalias <enable|disable> - interpret/support mapAlias for H.323\n");
	fprintf(stdout, "\t\t\tmaxhunts <integer> - Max Hunts for this source\n");
	fprintf(stdout, "\t\t\tmr <enable|disable> - media route\n");
	fprintf(stdout, "\t\t\tncalls <integer>  - current calls\n");
	fprintf(stdout, "\t\t\tnewsrcitg <string>  - source ingress trunk group\n");
	fprintf(stdout, "\t\t\tnmr <string>  - never media route\n");
	fprintf(stdout, "\t\t\togp <string>  - outgoing prefix\n");
	fprintf(stdout, "\t\t\tpasswd <string> - H.235 password\n");
	fprintf(stdout, "\t\t\tpgkid <string> - Parent Gk Id\n");
	fprintf(stdout, "\t\t\tpionfaststart <enable|disable> - H.323 PI on Fast Start\n");
	fprintf(stdout, "\t\t\tpriority <integer>  - priority\n");
	fprintf(stdout, "\t\t\tq931port <integer> - H.323 H.225 Q.931 port\n");
	fprintf(stdout, "\t\t\tqval <A.B> - q-value to be registered\n");
	fprintf(stdout, "\t\t\trai <enable|disable> - H.323 RAI\n");
	fprintf(stdout, "\t\t\trealm <realm-name> - realm of endpoint\n");
	fprintf(stdout, "\t\t\trasport <integer> - H.323 H.225 RAS port\n");
	fprintf(stdout, "\t\t\treg <active|inactive> - reg status\n");
	fprintf(stdout, "\t\t\tremovetg <enable|disable> - remove trunk group ID\n");
	fprintf(stdout, "\t\t\tsip <enable|disable> \n");
	fprintf(stdout, "\t\t\tstatic <ipaddr> - static ip address\n");
	fprintf(stdout, "\t\t\tsticky <enable|disable> \n");
	fprintf(stdout, "\t\t\tsubnetip <subnet addr> - subnet address\n");
	fprintf(stdout, "\t\t\tsubnetmask <subnet mask> - subnet mask\n");
	fprintf(stdout, "\t\t\ttechp <string> - Registration Tech Prefix\n");
	fprintf(stdout, "\t\t\ttg <string>  - trunk group\n");
	fprintf(stdout, "\t\t\ttpg <enable|disable> - Tech Prefix Plan\n");
	fprintf(stdout, "\t\t\ttype <string> - endpoint type\n");
	fprintf(stdout, "\t\t\tuareg <enable|disable>  - MSW registration\n");
	fprintf(stdout, "\t\t\turi <URI> - sip uri\n");
	fprintf(stdout, "\t\t\tvendor <clarent|sonusGSX|broadsoft|avaya|nortel|generic> \n");
	fprintf(stdout, "\t\t\txcalls <integer>  - max calls\n");
	fprintf(stdout, "\t\t\txincalls <integer>  - max ingress calls\n");
	fprintf(stdout, "\t\t\txoutcalls <integer>  - max egress calls\n");
	fprintf(stdout, "\t\t\t2833capable <unknown|yes|no>  - endpoint's 2833 capability \n");
	fprintf(stdout, "\t\t\ttg <string>  - trunk group\n");
	fprintf(stdout, "\t\t\tnewsrcitg <string>  - source ingress trunk group\n");
	fprintf(stdout, "\t\t\tnewsrcdtg <string>  - source egress trunk group\n");
	fprintf(stdout, "\t\t\tinfotranscap <speech | unrestricted | restricted | audio | unrestrictedtones | video | pass> \n");
	fprintf(stdout, "\t\t\tdeltcs2833 <enable|disable|default>  - remove 2833 cap from TCS\n");
	fprintf(stdout, "\t\t\tdeltcst38 <enable|disable|default>  - remove t38Fax from TCS\n");
	fprintf(stdout, "\t\t\tremovetg <enable|disable> - remove trunk group ID\n");
	fprintf(stdout, "\t\t\tuareg <enable|disable>  - MSW registration\n");
	fprintf(stdout, "\t\t\tnatdetect <enable|disable>  - detect nat\n");
	fprintf(stdout, "\t\t\tnatip <ipaddr>  - NAT/OBP IP address\n");
	fprintf(stdout, "\t\t\tnatport <integer>  - NAT/OBP port\n");
        fprintf(stdout, "\t\t\tprivacy  <rfc3325|draft01|both> - sip privacy\n");
        fprintf(stdout, "\t\t\tcidblock <enable|disable> - Call ID Prsentation\n");        
}

void
NetoidEditUsage(Command *comm, int argc, char **argv)
{
	fprintf(stdout, "%s ", comm->name);
     	fprintf(stdout, "\tregid low[-high] [data]\n");
#ifdef actual_usage
     	fprintf(stdout, "\tregid low[-high] ['/' n1.attr1 val1 n1.attr2 val2 ... '/'] ['/' n2.attr1 val1 n2.attr2 val2 ... '/'] ...");
#endif
}

void
NetoidRegUsage(Command *comm, int argc, char **argv)
{
	fprintf(stdout, "%s ", comm->name);
     	fprintf(stdout, "\tregid low[-high] attrib-list");
#ifdef actual_usage
     	fprintf(stdout, "\tregid low[-high] ['/' n1.attr1 val1 n1.attr2 val2 ... '/'] ['/' n2.attr1 val1 n2.attr2 val2 ... '/'] ...");
#endif
}

void
DbExportUsage(Command *comm, int argc, char **argv)
{
	fprintf(stdout, "%s ", comm->name);
     	fprintf(stdout, "\t <db-name>");
}

void
DbSaveUsage(Command *comm, int argc, char **argv)
{
	fprintf(stdout, "%s ", comm->name);
     	fprintf(stdout, "\t <dirname>");
}

void
DbCreateUsage(Command *comm, int argc, char **argv)
{
	fprintf(stdout, "%s ", comm->name);
     	fprintf(stdout, "\t <db-name>");
}

void
DbCopyUsage(Command *comm, int argc, char **argv)
{
	fprintf(stdout, "%s ", comm->name);
     	fprintf(stdout, "\t <db-name>");
}

void
DbCleanUsage(Command *comm, int argc, char **argv)
{
	fprintf(stdout, "%s ", comm->name);
     	fprintf(stdout, "\t [all|iedge|vpn|vpng|cp|realm|igrp|vnet]");
}

void
DbStaleUsage(Command *comm, int argc, char **argv)
{
	fprintf(stdout, "%s ", comm->name);
}

void
DbAddUsage(Command *comm, int argc, char **argv)
{
	fprintf(stdout, "%s ", comm->name);
     	fprintf(stdout, "\t <db-name>");
}

void
DbReplaceUsage(Command *comm, int argc, char **argv)
{
	fprintf(stdout, "%s ", comm->name);
     	fprintf(stdout, " <db-name>");
}

void
DbDeleteUsage(Command *comm, int argc, char **argv)
{
	fprintf(stdout, "%s ", comm->name);
     	fprintf(stdout, "\t <db-name>");
}

void
DbInitUsage(Command *comm, int argc, char **argv)
{
	fprintf(stdout, "%s ", comm->name);
}

void
DbInfoUsage(Command *comm, int argc, char **argv)
{
	fprintf(stdout, "%s ", comm->name);
}

void
DbOrgUsage(Command *comm, int argc, char **argv)
{
	fprintf(stdout, "%s ", comm->name);
}

void
DbReplUsage(Command *comm, int argc, char **argv)
{
	fprintf(stdout, "%s ", comm->name);
	fprintf(stdout, "\t <ip-address>");
}

void
DbRevUsage(Command *comm, int argc, char **argv)
{
	fprintf(stdout, "%s ", comm->name);
}

void
DbRevShowUsage(Command *comm, int argc, char **argv)
{
	fprintf(stdout, "%s ", comm->name);
}

void
DbRevIncrUsage(Command *comm, int argc, char **argv)
{
	fprintf(stdout, "%s ", comm->name);
}

void
DbRevModUsage(Command *comm, int argc, char **argv)
{
	fprintf(stdout, "%s ", comm->name);
	fprintf(stdout, "\t<newdbrev>");
}

void
DbSwitchUsage(Command *comm, int argc, char **argv)
{
	fprintf(stdout, "%s ", comm->name);
	fprintf(stdout, "\t<db-name>");
}

void
DbHistUsage(Command *comm, int argc, char **argv)
{
	fprintf(stdout, "%s ", comm->name);
}

void
FaxsLkupUsage(Command *comm, int argc, char **argv)
{
     fprintf(stdout, "%s ", comm->name);
     fprintf(stdout, "\t<phone>");
}

void
FaxsDeleteUsage(Command *comm, int argc, char **argv)
{
     fprintf(stdout, "%s ", comm->name);
     fprintf(stdout, "\t<file>");
}

void
FaxsAddUsage(Command *comm, int argc, char **argv)
{
     fprintf(stdout, "%s ", comm->name);
     fprintf(stdout, "\t<LUS|VPNS> <phone> <file>");
}

void
FaxsListUsage(Command *comm, int argc, char **argv)
{
     fprintf(stdout, "%s ", comm->name);
}

void
CPAddUsage(Command *comm, int argc, char **argv)
{
     fprintf(stdout, "%s ", comm->name);
     fprintf(stdout, "\tcp-name [route-name]");
}

void
CPLkupUsage(Command *comm, int argc, char **argv)
{
     fprintf(stdout, "%s ", comm->name);
     fprintf(stdout, "\tcp-name [dest]");
}

void
CPDeleteUsage(Command *comm, int argc, char **argv)
{
     fprintf(stdout, "%s ", comm->name);
     fprintf(stdout, "\tcp-name [route-name]");
}

void
CPListUsage(Command *comm, int argc, char **argv)
{
     fprintf(stdout, "%s\t[bindings]", comm->name);
}

void
CPCacheUsage(Command *comm, int argc, char **argv)
{
     fprintf(stdout, "%s ", comm->name);
}

void
CPEditUsage(Command *comm, int argc, char **argv)
{
     fprintf(stdout, "%s ", comm->name);
     fprintf(stdout, "\tcp-name");
}

void
CPBEditHelp(Command *comm, int argc, char **argv)
{
	fprintf(stdout, "\t\t\t%s ", comm->name);
    fprintf(stdout, "\tcp-name cr-name <field> <value>\n\n");
	fprintf(stdout, "\t\t\tpriority <integer> - route binding priority\n");
	fprintf(stdout, "\t\t\ttype <normal|reject> - route binding type\n");
	fprintf(stdout, "\t\t\tftype <forward|rollover|none> - route forwarding type\n");
	fprintf(stdout, "\t\t\tstime <YYYY/DOY/DOW/MM/DOM/HH/MM/SS> - service start time\n");
	fprintf(stdout, "\t\t\tftime <YYYY/DOY/DOW/MM/DOM/HH/MM/SS> - service end time\n");
}

void
CRAddUsage(Command *comm, int argc, char **argv)
{
     fprintf(stdout, "%s ", comm->name);
     fprintf(stdout, "\tcr-name ");
}

void
CRDeleteUsage(Command *comm, int argc, char **argv)
{
     fprintf(stdout, "%s ", comm->name);
     fprintf(stdout, "\tcr-name");
}

void
CRListUsage(Command *comm, int argc, char **argv)
{
     fprintf(stdout, "%s ", comm->name);
}

void
CRCacheUsage(Command *comm, int argc, char **argv)
{
     fprintf(stdout, "%s ", comm->name);
}

void
CRLkupUsage(Command *comm, int argc, char **argv)
{
     fprintf(stdout, "%s ", comm->name);
     fprintf(stdout, "\t{cr-name|phone}");
}

void
CREditUsage(Command *comm, int argc, char **argv)
{
     fprintf(stdout, "%s ", comm->name);
     fprintf(stdout, "\tcr-name");
}

void
CREditHelp(Command *comm, int argc, char **argv)
{
    fprintf(stdout, "\t\t\t%s ", comm->name);
    fprintf(stdout, "\tcr-name <field> <value>\n\n");
	fprintf(stdout, "\t\t\tprefix <string> - route destprefix\n");
	fprintf(stdout, "\t\t\tdest <string> - route dest\n");
	fprintf(stdout, "\t\t\tdestlen <integer> - route destlen\n");
	fprintf(stdout, "\t\t\tsrcprefix <string> - route srcprefix or @filename\n");
	fprintf(stdout, "\t\t\tsrc <string> - route source\n");
	fprintf(stdout, "\t\t\tsrclen <integer> - route srclen\n");
	fprintf(stdout, "\t\t\tcalltype <origin|dest|transit> - route type\n");
	fprintf(stdout, "\t\t\tdnisdefault <enable|disable> - dnis default\n");
	fprintf(stdout, "\t\t\tsticky <enable|disable> - route sticky property\n");
	fprintf(stdout, "\t\t\ttype <normal|reject> - route type\n");
	fprintf(stdout, "\t\t\ttemplate <enable|disable> - for template routes\n");
}

void
GkRegUsage(Command *comm, int argc, char **argv)
{
	fprintf(stdout, "\t\t\t%s ", comm->name);
   	fprintf(stdout, "\tregid uport <field> <value>\n\n");
	fprintf(stdout, "\t\t\tepid <epid-hex> \n");
	fprintf(stdout, "\t\t\tttl <integer> \n");
}

void
CallCacheUsage(Command *comm, int argc, char **argv)
{
     fprintf(stdout, "%s <filename>", comm->name);
}

void
CallLkupUsage(Command *comm, int argc, char **argv)
{
     fprintf(stdout, "%s <callid> <fname>", comm->name);
}

void
CallDeleteUsage(Command *comm, int argc, char **argv)
{
     fprintf(stdout, "%s <callid>", comm->name);
}

void
CacheCreateUsage(Command *comm, int argc, char **argv)
{
     fprintf(stdout, "%s ", comm->name);
}

void
CacheCleanUsage(Command *comm, int argc, char **argv)
{
     fprintf(stdout, "%s ", comm->name);
}

void
RsdClearUsage(Command *comm, int argc, char **argv)
{
     fprintf(stdout, "%s ", comm->name);
}

void
RsdAddUsage(Command *comm, int argc, char **argv)
{
     fprintf(stdout, "%s <port> <ip addr> <master|slave> ", comm->name);
}

void
RsdDeleteUsage(Command *comm, int argc, char **argv)
{
     fprintf(stdout, "%s <port> <ip addr> <master|slave> ", comm->name);
}

void
RsdListUsage(Command *comm, int argc, char **argv)
{
     fprintf(stdout, "%s ", comm->name);
}

void
TriggerAddUsage(Command *comm, int argc, char **argv)
{
     fprintf(stdout, "%s name", comm->name);
}

void
TriggerDeleteUsage(Command *comm, int argc, char **argv)
{
     fprintf(stdout, "%s name", comm->name);
}

void
TriggerListUsage(Command *comm, int argc, char **argv)
{
     fprintf(stdout, "%s ", comm->name);
}

void
TriggerEditUsage(Command *comm, int argc, char **argv)
{
     fprintf(stdout, "%s name", comm->name);
}

void
TriggerPurgeUsage(Command *comm, int argc, char **argv)
{
     fprintf(stdout, "%s name", comm->name);
}

void
RealmAddUsage(Command *comm, int argc, char **argv)
{
     fprintf(stdout, "%s ", comm->name);
     fprintf(stdout, "\trealm-name");
}

void
RealmDeleteUsage(Command *comm, int argc, char **argv)
{
     fprintf(stdout, "%s\trealm-name", comm->name);
}

void 
RealmEditUsage(Command *comm, int argc, char **argv)
{
	fprintf(stdout, "%s ", comm->name);
	fprintf(stdout, "\trealm-name [rsa] [mask] ...");
}

void 
RealmEditHelp(Command *comm, int argc, char **argv)
{
    fprintf(stdout, "%s realm-name\n\n", comm->name);
    fprintf(stdout, "\t addr - public/private\n");
    fprintf(stdout, "\t admin - enable/disable\n");
    fprintf(stdout, "\t cidblock   <string> - prefix to block call id \n");
    fprintf(stdout, "\t cidunblock <string> - prefix to unblock call id\n");
    fprintf(stdout, "\t default - enable/disable\n");
    fprintf(stdout, "\t emr - Between Realms xxx/alwayson/alwaysoff/on\n");
    fprintf(stdout, "\t imr - Within Realm xxx/alwayson/alwaysoff/on\n");
    fprintf(stdout, "\t mask\n");
    fprintf(stdout, "\t medpool\n");
    fprintf(stdout, "\t proxy_regid <regid>\n");    
    fprintf(stdout, "\t proxy_uport <uport>\n");
    fprintf(stdout, "\t rsa\n");
    fprintf(stdout, "\t sigpool\n");
    fprintf(stdout, "\t sipauth - inv/reg/bye/all/none\n");
    fprintf(stdout, "\t vnetname\n");
}

void
RealmListUsage(Command *comm, int argc, char **argv)
{
     fprintf(stdout, "%s ", comm->name);
}

void
RealmCacheUsage(Command *comm, int argc, char **argv)
{
     fprintf(stdout, "%s ", comm->name);
}

void
RealmLkupUsage(Command *comm, int argc, char **argv)
{
     fprintf(stdout, "%s ", comm->name);
}

void
RealmUpUsage(Command *comm, int argc, char **argv)
{
     fprintf(stdout, "%s\trealm-name|all", comm->name);
}

void
RealmDownUsage(Command *comm, int argc, char **argv)
{
     fprintf(stdout, "%s\trealm-name|all", comm->name);
}

void
RealmOpenUsage(Command *comm, int argc, char **argv)
{
     fprintf(stdout, "%s\trealm-name|all", comm->name);
}

void
RealmCloseUsage(Command *comm, int argc, char **argv)
{
     fprintf(stdout, "%s\trealm-name|all", comm->name);
}

void
IgrpAddUsage(Command *comm, int argc, char **argv)
{
     fprintf(stdout, "%s ", comm->name);
     fprintf(stdout, "\tigrp-name");
}

void
IgrpDeleteUsage(Command *comm, int argc, char **argv)
{
     fprintf(stdout, "%s\tigrp-name", comm->name);
}

void 
IgrpEditUsage(Command *comm, int argc, char **argv)
{
    fprintf(stdout, "%s igrp-name\n", comm->name);
    fprintf(stdout, "\t\t\t maxcallsin - Max ingress calls\n");
    fprintf(stdout, "\t\t\t maxcallsout - Max egress calls\n");
    fprintf(stdout, "\t\t\t maxcallstotal - Max total calls");
}

void
IgrpListUsage(Command *comm, int argc, char **argv)
{
     fprintf(stdout, "%s ", comm->name);
}

void
IgrpCacheUsage(Command *comm, int argc, char **argv)
{
     fprintf(stdout, "%s ", comm->name);
}

void
IgrpLkupUsage(Command *comm, int argc, char **argv)
{
     fprintf(stdout, "%s\tigrp-name", comm->name);
}

int
VnetAddUsage(Command *comm, int argc, char **argv)
{
     fprintf(stdout, "%s ", comm->name);
     fprintf(stdout, "\tvnet-name");
}

int
VnetDeleteUsage(Command *comm, int argc, char **argv)
{
     fprintf(stdout, "%s\tvnet-name", comm->name);
}

int 
VnetEditUsage(Command *comm, int argc, char **argv)
{
    fprintf(stdout, "%s vnet-name\n", comm->name);
    fprintf(stdout, "\t\t\t ifname  - Interface Name\n");
#ifdef NETOID_LINUX
    fprintf(stdout, "\t\t\t vlanid  - VLAN Id\n");
    fprintf(stdout, "\t\t\t gateway - Gateway\n");
#endif
}

int
VnetListUsage(Command *comm, int argc, char **argv)
{
     fprintf(stdout, "%s ", comm->name);
}

int
VnetCacheUsage(Command *comm, int argc, char **argv)
{
     fprintf(stdout, "%s ", comm->name);
}

int
VnetLkupUsage(Command *comm, int argc, char **argv)
{
     fprintf(stdout, "%s\tvnet-name", comm->name);
}
