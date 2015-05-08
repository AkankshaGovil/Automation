%{
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <strings.h>
#include <string.h>

#include "ipc.h"
#include "srvrlog.h"
#include "lsconfig.h"
#include "age.h"
#include "server.h"
#include "cdr.h"
#include "codecs.h"

#include "config.tab.h"
%}

HEXVAL		0x[0-9a-fA-F]+
INTEGER		[\+\-]?[0-9]+
CAUSESTRING	[a-zA-Z\-]+

%option yylineno
%option noyywrap
sconfiglval.val
%%
^#.*                    { }
[ \n\t]                 { }
{INTEGER}		{ sconfiglval.val = atoi(sconfigtext); return INTEGER; }
{HEXVAL}		{ sconfiglval.val = strtoul(sconfigtext, 0, 0); return INTEGER; }
cp			{ return CP; }
all			{ return ALL; }
allow			{ return ALLOW; }
dest			{ return DEST; }
destarq			{ return DESTARQ; }
autharq			{ return AUTHARQ; }
src			{ return SRC; }
xml			{ return XML;	}
lru			{ return LRU; }
utilz		{ return UTILZ; }
e911loc			{ return E911LOC; }
tapcall			{ return TAPCALL; }
untapcall		{ return UNTAPCALL; }
allocations		{ return ALLOCATIONS; }
update			{ return UPDATE; }
enable			{ return ENABLE; }
disable			{ return DISABLE; }
enumdomain		{ return ENUMDOMAIN; }
record-route	{ return RECORDROUTE; }
route			{ return ROUTE; }
force			{ return FORCE; }
share			{ return SHARE; }
servers			{ return SERVERS; }
endpoints		{ return ENDPTS; }
gateways		{ return GWS; }
segowner		{ return SEGOWNER; }
segaddr			{ return SEGADDR; }
segaddrtype             { return SEGADDRTYPE;}
segs			{ return SEGS; }
segsize			{ return SEGSIZE; }
dynamic			{ return DYNAMIC; }
fixed			{ return FIXED; }
daemonize		{ return DAEMONIZE; }
threads			{ return THREADS; }
procs			{ return PROCS; }
lwps			{ return LWPS; }
pm				{ return PM; }
gis				{ return GIS; }
lus				{ return LUS; }
vpns			{ return VPNS; }
bcs				{ return BCS; }
jserver			{ return JSERVER; }
cli				{ return CLI; }
faxserver		{ return FAXS; }
none			{ return NONE; }
no				{ return NO; }
local			{ return LOCAL; }
db				{ return DBTOKEN; }
age				{ return AGE; }
fce				{ return FCE; }
mgmt_interface			{ return MGMT_INTERFACE; }
mgmt_ip				{ return MGMT_IP; }
peering_config	{ return ISPD; }
primary_interface		{ return PRIMARY_INTERFACE; }
secondary_interface		{ return SECONDARY_INTERFACE; }
control_interface		{ return CONTROL_INTERFACE; }
server_type		{ return ISPD_TYPE; }
interface_name		{ return ISPD_INTERFACE_NAME; }
interface_router	{ return ISPD_ROUTER; }
interface_vip		{ return ISPD_VIP; }
peer_iserver		{ return ISPD_PEER_ISERVER; }
scm				{ return SCM; }
rsd				{ return RSD; }
rs_ifname		{ return RS_IFNAME; }
rs_mcast_addr	{ return RS_MCAST_ADDR; }
rs_port			{ return RS_PORT; }
rs_cp_cmd_str	{ return RS_CP_CMD_STR; }
rs_tmp_dir		{ return RS_TMP_DIR; }
rs_host_prio	{ return RS_HOST_PRIO; }
rs_ssn_int		{ return RS_SSN_INT; }
histdb_size		{ return HISTDB_SIZE; }
execd			{ return EXECD; }
timeout			{ return TMOUT; }
maxretransmitcount	{ return MAXRETRANSCOUNT; }
port			{ return PORT; }
debug			{ return DDEBUG; }
hdebug			{ return HDEBUG; }
sdebug			{ return SDEBUG; }
level			{ return LEVEL; }
uccproto		{ return UCC; }
xml				{ return XML; }
udp				{ sconfiglval.val = IPPROTO_UDP; return IPPROTO; }
tcp				{ sconfiglval.val = IPPROTO_TCP; return IPPROTO; }
sipserver		{ sconfiglval.val = SIPSERVER; return SIPSERVER; }
sipport			{ return SIPPORT; }
mswname			{ return MSWNAME; }
proxy			{ sconfiglval.val = SERVER_PROXY; return SERVERTYPE; }
proxystatefull	{ sconfiglval.val = SERVER_PROXYSTATEFULL; return SERVERTYPE; }
proxystateful	{ sconfiglval.val = SERVER_PROXYSTATEFULL; return SERVERTYPE; }
redirect		{ sconfiglval.val = SERVER_REDIRECT; return SERVERTYPE; }
sip				{ sconfiglval.val = CPROTO_SIP; return UCC_PROTO; }
h323			{ sconfiglval.val = CPROTO_H323; return UCC_PROTO; }
instance		{ return INSTANCE; }
stack			{ return STACK; }
ua				{ return UA; }
tsm				{ return TSM; }
iwf				{ return IWF; }
bridge			{ return BRIDGE; }
sipmaxforwards	{ return SIPMAXFORWARDS; }
siptimer		{ return SIPTIMER; }
siptrans		{ return SIPTRANS; }
T1				{ return T1; }
T2				{ return T2; }
C				{ return C; }
invitec			{ return INVITEC; }
modreg			{ sconfiglval.val = MREGISTER; return MODULE; }
modfind			{ sconfiglval.val = MFIND; return MODULE; }
modage			{ sconfiglval.val = MAGE; return MODULE; }
modcache		{ sconfiglval.val = MCACHE; return MODULE; }
modinit			{ sconfiglval.val = MINIT; return MODULE; }
modsel			{ sconfiglval.val = MSEL; return MODULE; }
modpkt			{ sconfiglval.val = MPKT; return MODULE; }
moddb			{ sconfiglval.val = MDB; return MODULE; }
modshm			{ sconfiglval.val = MSHM; return MODULE; }
modcdr			{ sconfiglval.val = MCDR; return MODULE; }
modfaxp			{ sconfiglval.val = MFAXP; return MODULE; }
modconn			{ sconfiglval.val = MCONN; return MODULE; }
modtmr			{ sconfiglval.val = MTMR; return MODULE; }
modredund		{ sconfiglval.val = MRED; return MODULE; }
modxml			{ sconfiglval.val = MXML; return MODULE; }
modcli			{ sconfiglval.val = MCLI; return MODULE; }
modlmgr			{ sconfiglval.val = MLMGR; return MODULE; }
modpmgr			{ sconfiglval.val = MPMGR; return MODULE; }
modh323         { sconfiglval.val = MH323; return MODULE; }
modlrq         	{ sconfiglval.val = MLRQ; return MODULE; }
modrrq         	{ sconfiglval.val = MRRQ; return MODULE; }
modarq         	{ sconfiglval.val = MARQ; return MODULE; }
modsip         	{ sconfiglval.val = MSIP; return MODULE; }
modq931        	{ sconfiglval.val = MQ931; return MODULE; }
modscc         	{ sconfiglval.val = MSCC; return MODULE; }
modiwf         	{ sconfiglval.val = MIWF; return MODULE; }
modbridge		{ sconfiglval.val = MBRIDGE; return MODULE; }
modfce         	{ sconfiglval.val = MFCE; return MODULE; }
modradc        	{ sconfiglval.val = MRADC; return MODULE; }
modispd        	{ sconfiglval.val = MISPD; return MODULE; }
modrsd			{ sconfiglval.val = MRSD; return MODULE; }
moddlic			{ sconfiglval.val = MDLIC; return MODULE; }
modirq        	{ sconfiglval.val = MIRQ; return MODULE; }
modicmp        	{ sconfiglval.val = MICMPD; return MODULE; }
modexecd		{ sconfiglval.val = MEXECD; return MODULE; }
modscm		{ sconfiglval.val = MSCM; return MODULE; }
modscmrpc		{ sconfiglval.val = MSCMRPC; return MODULE; }
prefix			{ return PREFIX; }
log				{ return DLOG; }
slog			{ return SLOG; }
policy			{ return POLICY; }
callstate		{ return CALLSTATE; }
jserverlogfile	{ return DLOGFILE; }
jserverloglevel { return DLOGLEVEL; }
faxdirectory	{ return FAXDIR; }
faxuser			{ return FAXUSER; }
faxpassword		{ return FAXPASSWORD; }
terminal		{ return DTERMINAL; }
syslog			{ return DSYSLOG; }
priority		{ return PRIO; }
read_permission	{ return PERMISSION_READ; }
write_permission	{ return PERMISSION_WRITE; }
gkid			{ return GKID; }
rrqttl			{ return RRQTTL; }
rrqtimer		{ return RRQTIMER; }
callttl			{ return CALLTTL; }
h245			{ return H245; }
proceeding 		{ return PROCEEDING; }
tunneling 		{ return TUNNELING; }
call			{ return CALL; }
sipdomain		{ return SIPDOMAIN; }
sipauthpassword	{ return SIPAUTHPASSWORD; }
sipauth			{ return SIPAUTH; }
cdr				{ return CDR; }
mindcti			{ return MINDCTI; }
fixed			{ return FIXED; }
seq				{ return SEQ; }
time            { return TIME; }
daily			{ return DAILY; }
dir				{ return DIR; }
cdrtimer		{ return CDRTIMER;}
start1			{ sconfiglval.val = CDRSTART1; return CDREVENT; }
start2			{ sconfiglval.val = CDRSTART2; return CDREVENT; }
end1			{ sconfiglval.val = CDREND1; return CDREVENT; }
end2			{ sconfiglval.val = CDREND2; return CDREVENT; }
hunt			{ sconfiglval.val = CDRHUNT; return CDREVENT; }
sleep			{ return SLEEP; }
faststart		{ return FASTSTART; }
always2833		{ return ALWAYS2833; }
mapisdncc		{ return MAPISDNCC; }
maplrjreason	{ return MAPLRJREASON; }
fsinconnect		{ return FSINCONNECT; }
getanifromacf	{ return GETANIFROMACF; }
siphold3264		{ return SIPHOLD3264; }
reinvite-no-sdp { return REINVITENOSDP; }
on				{ return ON; }
off				{ return OFF; }
billing			{ return BILLING; }
postpaid		{ sconfiglval.val = BILLING_POSTPAID; return BILLTYPE; }
prepaid			{ sconfiglval.val = BILLING_PREPAID; return BILLTYPE; }
ciscoprepaid	{ sconfiglval.val = BILLING_CISCOPREPAID; return BILLTYPE; }
firstauth		{ return FIRST_AUTH; }
secondauth		{ return SECOND_AUTH; }
rollovertime	{ return ROLLOVERTIME;}
codec			{ return CODEC;	}
xconnid			{ return XCONNID; }
g711-u-law-64k	{ return G711ULAW64; }
g711ulaw64k		{ return G711ULAW64; }
g711Ulaw64k		{ return G711ULAW64; }
g711-a-law-64k	{ return G711ALAW64; }
g711alaw64k		{ return G711ALAW64; }
g711Alaw64k		{ return G711ALAW64; }
g729			{ return G729; }
g723.1			{ return G7231; }
fwname			{ return FWNAME; }
fwaddress	       	{ return FWIPADDR; }
private			{ return FWPRIVATE; }
public			{ return FWPUBLIC; }
default			{ return DEFAULT; }
fwconnectaddr	{ return FWCONNECTADDR; }
altgk			{ return ALTGK; }
gk				{ return GK; }
h323qsize		{ return H323QSIZE; }
routedebug		{ routeDebug = 1; }
realtime		{ return REALTIME; }
radius			{ return RADIUS; }
radserver		{ return RADIUS_SERVER; }
radsecret		{ return SHARED_SECRET; }
radtimeout		{ return RADIUS_TIMEOUT; }
radretries		{ return RADIUS_RETRIES; }
raddeadtime		{ return RADIUS_DEADTIME; }
radacct			{ return RADIUS_ACCT; }
acctsessionid	{ return ACCT_SESSION_ID; }
overloaded		{ return OVERLOADED; }
hairpin			{ return HAIRPIN; }
hairPin			{ return HAIRPIN; }
mediaRouting		{ return MEDIAROUTING; }
mediarouting		{ return MEDIAROUTING; }
midCallMediaChange	{ return MIDCALLMEDIACHANGE; }
midcallmediachange	{ return MIDCALLMEDIACHANGE; }
obp				{ return OBP; }
internalcalling	{ return INTERNAL; }
h323cps 		{ return H323CPS;}
h323qlen 		{ return H323QLEN;}
maxcalls		{ return MAXCALLS;}
sgk				{ return SGK; }
maxrasbuffsize	{ return MAXRASBUFFSIZE; }
infoTransCap	{ return INFOTRANSCAP;}
dynamicendpoints	{ return DYNAMIC_ENDPOINTS; }
natdetection		{ return NAT_DETECTION; }
h323Cps 		{ return H323CPS;}
h323QLen 		{ return H323QLEN;}
maxhunts		{ return MAXHUNTS; }
maxcallduration	{ return MAXCALLDUR; }
maxhuntallowdur	{ return MAXHUNTALLOWDUR; }
crid			{ return CRID; }
mask			{ return HIDE; }
hide			{ return HIDE; }
forward			{ return FORWARD; }
addr			{ return ADDR; }
rtp				{ return RTP; }
shutdown		{ return SHUTDOWN; }
graceful		{ return GRACEFUL; }
compression		{ return COMPRESSION; }
pad				{ return PAD; }
dnsrecoverytimeout { return DNS_REC_TIMEOUT; }
removetcs2833	{ return REMOVETCS2833; }
removetcst38	{ return REMOVETCST38; }
disable			{ return DISABLE; }
callidlen		{ return CALLIDLEN; }
sipminse                { return SIP_SESSION_TIMER_MINSE;}
sipsess           { return SIP_SESSION_TIMER_EXPIRY;}
realm			{ return REALM; }
local-isdn-cc 	{ return LOCAL_ISDN_CC; }
ani				{ return ANI; }
dnis			{ return DNIS; }
portsel			{ return PORTSEL; }
huntAllCC		{ return HUNTALLCC; }
usecodemap		{ return USECODEMAP; }
tcpUnreachCC	{ return TIMEREXPIRYCC; }
\"[^\"]*\"		{
					sconfigtext[strlen(sconfigtext)-1] = '\0';
					sconfiglval.str = strdup(&sconfigtext[1]);
					return STRING;
				}
{CAUSESTRING}		{
				sconfiglval.str = strdup(sconfigtext);
				return CAUSESTR;
			}
.				{ return (int)sconfigtext[0]; }
