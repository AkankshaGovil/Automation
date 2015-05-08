package com.nextone.common;

public interface CommonConstants {
	
 // hot knife interface lists
 public static final String[] HOTKNIFE_INTERFACES = new String[] {
 	"hk0,0",
 	"hk0,1",
 	"hk0,2"  
 };
 
  // port numbers/ip addresses  used
  public static final int MCAST_SEND_PORT     = 10101;
  public static final int MCAST_LISTEN_PORT   = 10102;
  public static final String MCAST_ADDRESS    = "224.0.1.20";

  // this is the port which ensures only one iView instance runs 
  // on a given machine
  public static final int INSTANCE_PORT = 10103;
  public static final int JSERVER_INSTANCE_PORT = 10104;
  public static final int JSERVER_LOG_PORT = 10105;

  public static final int ISERVER_BUTTERFLY_PORT = 12979;

  public static final int RASPORT = 1719;
  public static final int Q931PORT = 1720;

  // number of receive threads
  public static final int NUM_RECEIVERS = 16;

  // device ids
  public static final short DEVICE_ID_510 = 510;
  public static final short DEVICE_ID_500 = 500;
  public static final short DEVICE_ID_1000 = 1000;
  public static final short DEVICE_ID_PINGTEL = 100;
  public static final short DEVICE_ID_VEGA_50 = 101;
  public static final short DEVICE_ID_VEGA_100 = 102;
  public static final short DEVICE_ID_ISERVER = 0;

  // this signature is written in every configuration file saved
  public final String SIGNATURE = "%%--%%";

  // some protocol definitions
  public static final short TCP = 0x6;
  public static final short UDP = 0x11;
  public static final short ANY = 0x0;

  // command codes used in the hello protocol

  // commands used internally by the jserver
  public static final short STOP = 0;
  public static final short STATUS = 1;
  public static final short DEBUG = 2;
  public static final short UPTIME = 3;
  public static final short RECONFIG = 4;
  public static final short ALIVE = 5;
  public static final short RESTART = 6;

  // protocol versions
  public static final short HELLO_VERSION_NORMAL = 0;
  public static final short HELLO_VERSION_NO_AUTH = 1;

  // commands to the ucon
  public static final int HELLO = 100;
  public static final int UNIQUE_ID = 101;
  public static final int RESET_UNIQUE_ID = 102;
  public static final int COMMAND = 150;
  public static final short REMOTE = 160;

  // codes from the ucon
  public static final int REGISTRATION = 200;
  public static final int COMMAND_REPLY = 201;
  public static final int NEED_UNIQUE_ID = 202;
  public static final int NEED_SERIAL_NUM = 203;

  // command codes to ucon (or jserver)
  public static final int GET_COMMAND = 400;
  public static final int SET_COMMAND = 401;
  public static final int MCAST_GET_COMMAND = 402;
  public static final int MCAST_SET_COMMAND = 403;
  public static final int REBOOT_COMMAND = 404;
  public static final int MCAST_REBOOT_COMMAND = 405;
  public static final int BLINK_COMMAND = 406;
  public static final int MCAST_BLINK_COMMAND = 407;
  public static final int DOWNLOAD_COMMAND = 408;
  public static final int MCAST_DOWNLOAD_COMMAND = 409;
  public static final int DOWNLOAD_REBOOT_COMMAND = 410;
  public static final int MCAST_DOWNLOAD_REBOOT_COMMAND = 411;
  public static final int REBOOT_OS_COMMAND = 412;

  // command codes to jserver
  public static final int LIST_COMMAND = 425;
  public static final int IEDGE_COMMAND = 426;  // related to iedge operations
  public static final int VERSION_COMMAND = 429;
  public static final int STATUS_COMMAND = 430;
  public static final int MAINTENANCE_COMMAND = 432;
  public static final int AUTO_DOWNLOAD_COMMAND = 433;
  public static final int LOOKUP_COMMAND = 434;
  public static final int MISC_COMMAND = 436;
  public static final int CFG_COMMAND = 437;
  public static final int DB_COMMAND = 438;
  public static final int LS_ALARM_COMMAND = 439;
  public static final int CAP_COMMAND = 440;

  // sub-codes in jserver command codes
  public static final int MAX_RECORDS = 600;
  public static final int IEDGE_GET = 604;


  public static final int MAINTENANCE_GET_GROUP_NAMES = 660;
  public static final int MAINTENANCE_GET_GROUP = 661;
  public static final int MAINTENANCE_PUT_GROUP = 662;
  public static final int MAINTENANCE_DELETE_GROUP = 663;
  public static final int MAINTENANCE_GET_GROUP_NAMES_COMMENTS = 664;
  public static final int MAINTENANCE_GET_REQUEST_NAMES = 665;
  public static final int MAINTENANCE_GET_REQUEST = 666;
  public static final int MAINTENANCE_PUT_REQUEST = 667;
  public static final int MAINTENANCE_DELETE_REQUEST = 668;
  public static final int MAINTENANCE_REQUEST_ACTIVE = 669;
  public static final int MAINTENANCE_REQUEST_ABORT = 670;
  public static final int MAINTENANCE_GET_REQUEST_NAMES_COMMENTS = 671;
  public static final int MAINTENANCE_GET_LOG_NAMES = 672;
  public static final int MAINTENANCE_DELETE_LOG = 673;
  public static final int AUTO_DOWNLOAD_GET_NAMES = 680;
  public static final int AUTO_DOWNLOAD_GET_NAMES_COMMENTS = 681;
  public static final int AUTO_DOWNLOAD_GET_CONFIG = 682;
  public static final int AUTO_DOWNLOAD_PUT_CONFIG = 683;
  public static final int AUTO_DOWNLOAD_DELETE = 684;
  public static final int AUTO_DOWNLOAD_ACTIVE = 685;
  public static final int AUTO_DOWNLOAD_ABORT = 686;
  public static final int AUTO_DOWNLOAD_DELETE_LOG = 687;
  public static final int LOOKUP_BY_PHONE = 700;
  public static final int LOOKUP_BY_VPNPHONE = 701;
  public static final int LOOKUP_BY_CALLINGPLAN_ROUTE_NAME = 702;
  public static final int PRESENCE_NUMBER = 801;
  public static final int ISERVER_MAX_CALLS = 802;
  public static final int ISERVER_COMPRESSION = 803;
  public static final int ISERVER_MAX_MR_CALLS = 804;
  public static final int ISERVER_GET_CFG = 850;
  public static final int ISERVER_SET_CFG = 851;
  public static final int ISERVER_DB_IMPORT  = 852;
  public static final int ISERVER_DB_EXPORT  = 853;
  public static final int ISERVER_DB_GET_FILENAMES  = 854;
  public static final int LS_ALARM_STATUS   = 855;
  public static final int LS_ALARM_CLEAR    = 856;
  public static final int CLEAR_LOG_FILE = 857;
  public static final int PROCESS_COMMAND = 858;
  public static final int PROCESS_BULK_COMMANDS = 859;
  public static final int ISERVER_GET_CAP = 860;
  public static final int ETHERNET = 450;
  public static final int NAT = 451;
  public static final int NAT_PROXY = 452;
  public static final int ITSP = 453;
  public static final int INTERNET = 454;
  public static final int CFG = 455;
  public static final int H323 = 456;
  public static final int DHCP_RELAY = 457;
  public static final int DIAL_PREFIX = 458;
  public static final int ROLLOVER_NUMBER = 459;
  public static final int DATA_VPN = 460;
  public static final int IP_FILTER = 461;
  public static final int DOWNLOAD_CODE = 462;
  public static final int LINE_PORT = 463;
  public static final int ACCESS_CONTROL = 464;
  public static final int ISERVER = 465;
  public static final int DNS = 466;
  public static final int PHONE_PORT = 467;
  public static final int PERMISSION = 468;
  public static final int DHCP_SERVER = 469;
  public static final int IVR = 470;
  public static final int SIP = 471;
  public static final int CODEC = 472;

  // code from iserver

  public static final String VPN_NONE = "<None>";

  //  public static final short PERMISSION = 468;  // same as PERMISSION defined above

  public final short ACCESS_ETHERNET = 1;
  public final short ACCESS_MODEM = 2;
  public final short ACCESS_SERIAL = 3;


  // reboot codes
  public static final int REBOOT_ROM = 550;
  public static final int REBOOT_RAM = 551;
  public static final int REBOOT_CFG = 552;

  // device modes
  public static final int RAM_MODE = 300;
  public static final int ROM_MODE = 301;
  public static final int OFFLINE_MODE = 302;

  // value types
  public static final short INTEGER_TYPE = 0;
  public static final short STRING_TYPE = 1;
  public static final short UNKNOWN_TYPE = -1;

  // some table max.s
  public static final int MAX_ITSP_ENTRIES = 6;
  public static final int MAX_NAT_PROXY_ENTRIES = 5;
  public static final int MAX_LINE_PORTS = 3;
  public static final int MAX_1000_LINE_PORTS = 240;
  public static final int MAX_PHONE_ENTRIES = 3;
  public static final int MAX_1000_PHONE_ENTRIES = 240;

  // device types used in the iserver (these are also
  // defined in serverdb.h for the iserver
  public static final int DEVTYPE_UNKNOWN = -1;
  public static final int DEVTYPE_I500 = 0;
  public static final int DEVTYPE_OSP = 1;
  public static final int DEVTYPE_I510 = 2;
  public static final int DEVTYPE_I1000 = 3;
  public static final int DEVTYPE_XGW = 4;
  public static final int DEVTYPE_ISERVER = 5;
  public static final int DEVTYPE_XGK = 6;
  public static final int DEVTYPE_SGK = 7;
  public static final int DEVTYPE_USERACC = 8;
  public static final int DEVTYPE_ENUM = 9;
  public static final int DEVTYPE_IPPHONE = 10;
  public static final int DEVTYPE_SIPPROXY = 11;
  public static final int DEVTYPE_SOFTSWITCH = 12;
  public static final int DEVTYPE_SIPGW = 13;
  public static final int DEVTYPE_ANY500 = Integer.MAX_VALUE-1;
  public static final int DEVTYPE_ANY = Integer.MAX_VALUE;

  // lists which the iserver can serve
  public static final int LOG_FILE = 2;
  public static final int MISC_REQ = 3;

  //  do not change the values for the following constants.
  public static final int IEDGE_ONLY          = 0x0001;
  public static final int IEDGE_VPN_ONLY      = 0x0002;
  public static final int IEDGE_GROUP_ONLY    = 0x0004;
  public static final int IEDGE_LIST          = 0x0007;
  public static final int CALLING_PLAN_ONLY       = 0x0010;
  public static final int CALLING_PLAN_BIND_ONLY  = 0x0020;
  public static final int CALLING_PLAN_ROUTE_ONLY = 0x0040;
  public static final int CALLING_PLAN_LIST       = 0x0070;
  public static final int TRIGGER_ONLY      = 0x0100;
  public static final int REALM_ONLY        = 0x0200;
  public static final int IGRP_ONLY         = 0x0400;
  public static final int DYNAMIC_CPR_ONLY = 0x0800;
  public static final int TRIGGER_DCR_ONLY = 0x0900;
  public static final int VNET_ONLY        = 0x1000;

  public static final int LIST_ALL          = 0xFFFF;

    // db name
    public static final String IEDGE = "iedge";
    public static final String VPN = "vpn";
    public static final String VPNG = "vpng";
    public static final String CP = "cp";
    public static final String CPB = "cpb";
    public static final String CR = "cr";
    public static final String DCR = "dcr";
    public static final String TRIGGER = "trigger";
    public static final String IGRP = "igrp";


  // The value of each of these items depend on iserver key.h file
  public static final int CRF_FORWARD   = 0x0100;
  public static final int CRF_ROLLOVER  = 0x0200;
  public static final int CRF_REJECT		= 0x2000;
  public static final int CRF_TRANSIT		= 0x4000;
  public static final int CRF_STICKY		= 0x20000;


  public static final short CALLFORWARD     = 0x0030;
  public static final short CALLROLLOVER    = 0x0031;
  public static final short NO_CALLFORWARD  = 0x0032;
  public static final short ISGATEWAY = 0x0033;
  //	common requests the iserver can serve
  public static final int CALL_ROUTE	 = 0x000F;
  public static final int CALL_ROUTE_WITH_REALM	 = 0x0010;

  public static final String TYPE_REJECT = "reject";
  public static final String TYPE_NORMAL = "normal";
  public static final String ROUTE  = "route";
  public static final String HUNT   = "hunt";




  // The value of each of these items depend on iserver ipc.h file
  // used for sflags, to indicate what all is set etc 
  public static final short ISSET_REGID = 1;
  public static final short ISSET_IPADDRESS = 2;
  public static final short ISSET_UPORT = 3;
  public static final short ISSET_PHONE = 4;

  // capabilities
  public static final short CAP_UCC                     = 0;
  public static final short CAP_OSP                     = 1;
  public static final short CAP_IGATEWAY                = 2;
  public static final short CAP_I1000                   = 3;
  public static final short CAP_LRQ                     = 4;
  public static final short CAP_IRQ                     = 5;
  public static final short CAP_ARQ                     = 6;
  public static final short CAP_SIP			= 7;	/* SIP Capable device */
  public static final short CAP_H323                    = 8;	/* H.323 Capable device */
  public static final short CAP_ENUMS                   = 9;
  public static final short CAP_GRQ                     = 10; /* GRQ capable */
  public static final short CAP_TPG			= 11;	/* Tech Prefix Gateway */
  public static final short CAP_RAI			= 12;	/* RAI Capable device */
  public static final short CAP_MEDIAROUTE              = 13;	/* Media routing */
  public static final short CAP_HIDEADDRESSCHANGE       = 14;	/* hide mid-call address change */
  public static final short CAP_ROUTEDIRECT             = 15;

  // Extended capabilities flags (from ipc.h)
  public static final short ECAPS_NOH323DISPLAY = 0x1;
  public static final short ECAPS_NODIRECTCALLS = 0x2;
  public static final short ECAPS_MAPALIAS = 0x4;
  public static final short ECAPS_FORCEH245 = 0x8;
  public static final short ECAPS_NOCONNH245 = 0x10;
  public static final short ECAPS_NOMEDIAROUTE = 0x20;
  public static final short ECAPS_SETDESTTG    =  0x40;
  public static final short ECAPS_PIONFASTSTART  =  0x80;
  public static final short ECAPS_DELTCST38        =  0x100;
  public static final short ECAPS_DELTCST38DFT     =  0x200;
  public static final short ECAPS_DELTCSRFC2833    =  0x400;
  public static final short ECAPS_DELTCSRFC2833DFT =  0x800;
  public static final short ECAPS_NOTG =  0x1000;
  public static final int ECAPS_CAP2833_KNOWN = 0x8000;
  public static final short ECAPS_CAP2833 = 0x4000;
  public static final int ECAPS_NATDETECT = 0x02000;
  public static final int ECAPS1_MAPISDNCC = 0x10000;
  public static final int ECAPS1_SIP_PRIVACY_RFC3325  = 0x00020000;   // SIP PRIVACY RFC 3325 support
  public static final int ECAPS1_SIP_PRIVACY_DRAFT01  = 0x00040000;   // SIP PRIVACY draft-ietf-sip-privacy-draft-01.txt

  // log file types
  public static final int JSERVER_LOGFILE = 0;
  public static final int MAINTENANCE_LOGFILE = 1;
  public static final int AUTODOWNLOAD_LOGFILE = 2;
  public static final int ISERVER_LOGFILE = 3;

  // levels of logging
  public static final int LOG_LEVEL_ERROR = 0;
  public static final int LOG_LEVEL_WARNING = 1;
  public static final int LOG_LEVEL_MEDIUM = 2;
  public static final int LOG_LEVEL_VERBOSE = 3;
  public static final String LOG_LINE_START = "%"; // each line of log starts with this

  // names of some thread...
  public static final String updateDeviceQPName = "UpdateDeviceQueueProcessor";
  public static final String receiveRegQPName = "RegistrationQueueProcessor";
  public static final String cleanupThreadName = "CleanupDevices";

  // some limits for the number of simultaneous operations...
  public static final int MAX_REGISTRATIONS = 500;
  public static final int MAX_AUTODOWNLOAD_REQUESTS = 500;

  // the retry interval
  public static final int RETRY_INTERVAL = 3;

  // the max udp packet size between iview and iserver
  public static final int MAX_IVIEW_ISERVER_PACKET_SIZE = 4096;

  /* Max number of emergncy numbers */
  public final int MAX_EMERGENCY_NUMBERS = 5;

  // warning:   This value should be same as the value specified in iserver/ ipc.h
  public final int CALLINGPLAN_ATTR_LENGTH  =   28;

  public static final String [] portStatus = {"unused", "active", "debug", "unknown"};
  public static final String [] portType = {"voice", "data", "fax", "unknown"};
  public static final int LUS_TYPE = 0;
  public static final int VPN_TYPE = 1;
  public static final String [] rollType = {"lus", "vpn"};

  public static final int LS_LIMIT_OK = 0;
  public static final int LS_LIMIT_EXCEEDED   = 1;
  public static final int LS_LIMIT_THRESHOLD  = 2;

  // database commands
  public static final int IMPORT_ALL  =  0x0000;
  public static final int IMPORT_ADD  =  0x0001;
  public static final int IMPORT_REPLACE  =  0x0002;
  public static final int IMPORT_DELETE   =  0x0003;




  //public static final String DATA_DIR  = "data";

  public static final String ROOT_START_TAG   = "<DB>";
  public static final String ROOT_END_TAG     = "</DB>";
  public static final String IEDGE_START_TAG  = "<E>";
  public static final String IEDGE_END_TAG    = "</E>";
  public static final String VPN_START_TAG    = "<V>";
  public static final String VPN_END_TAG      = "</V>";
  public static final String GROUP_START_TAG  = "<VG>";
  public static final String GROUP_END_TAG    = "</VG>";
  public static final String CP_START_TAG     = "<CP>";
  public static final String CP_END_TAG       = "</CP>";
  public static final String CPB_START_TAG    = "<CPB>";
  public static final String CPB_END_TAG      = "</CPB>";
  public static final String CR_START_TAG     = "<CR>";
  public static final String CR_END_TAG       = "</CR>";
  public static final String TG_START_TAG     = "<TRG>";
  public static final String TG_END_TAG       = "</TRG>";
  public static final String RM_START_TAG     = "<RM>";
  public static final String RM_END_TAG       = "</RM>";
  public static final String IGRP_START_TAG   = "<IGRP>";
  public static final String IGRP_END_TAG     = "</IGRP>";


  // the value of this should be the same as in lsconfig.h
  public static final int SYSTEM_MAX_HUNTS = 50;
  public static final int SYSTEM_MAX_CALL_DURATION = 31536000;
  public static final int SIP_TIMERC       = 3600;
  public static final int SIP_MAX_FORWARDS = 100;
  public static final int SIP_MAX_PORT = 65535;
  public static final int SYSTEM_MAX_HUNTS_ALLOWABLE_DURATION = 65535;

  //  minimum records size
  public static final int MIN_RECORDS = 100;

  // endpoint state flags (defined in ipc.h)
  public static final int CL_ACTIVE     = 0x1;     /* active, breathing endpoint */
  public static final int CL_IDLE       = 0x2;
  public static final int CL_STATIC     = 0x4;     /* statically defined endpoint */
  public static final int CL_ALIVE      = 0x8;     /* alive, but not active */
  public static final int CL_BUSY       = 0x10;    /* other busy -- dialong etc. */
  public static final int CL_INCONF     = 0x20;    /* in conference */
  public static final int CL_DND        = 0x40;    /* DND */
  public static final int CL_FORWARD    = 0x80;    /* forward all incoming calls */
  public static final int CL_REGISTERED = 0x100;   /* registered once at least */
  public static final int CL_INCALL     = 0x200;   /* in call */
  public static final int CL_FORWARDDEST= 0x400;   /* represents a forwarded number */
  public static final int CL_PROXY      = 0x800;   /* someone will proxy me */
  public static final int CL_PROXYING   = 0x1000;  /* I am proxying someone */
  public static final int CL_HOLDORIG   = 0x2000;
  public static final int CL_FORWARDSTATIC=0x4000; /* statically configured forwarding */
  public static final int CL_TAP        = 0x80000;
  public static final int CL_DYNAMIC    = 0x100000;/* a synamically created entry */


  //  DB constants
  public static final int DB_LOCAL  = 0;
  public static final int DB_MYSQL  = 1;

  public static final String WORKING_DIR = System.getProperty( "user.home" ) + java.io.File.separator + ".iview" +  java.io.File.separator;
  public static final String DATA_DIR  = WORKING_DIR + "data";
  public static final String LOCAL_DB_CONF = WORKING_DIR + "db.conf";
  public static final String LOG4J_CONF = WORKING_DIR + "log4j.properties";
  public static final String DEBUG_LOG_FILE = WORKING_DIR + "iView.log";

  public static final String MYSQL_DRIVER = "com.mysql.jdbc.Driver";
  public static final String MYSQL_URL  = "jdbc:mysql://localhost/iview";

  public static final String LOCAL_DRIVER = "com.mckoi.JDBCDriver";
  public static final String LOCAL_URL  = ("jdbc:mckoi:local://" + LOCAL_DB_CONF).replace( '\\', '/');
  public static final String DB_USER    = "root";
  public static final String DB_PASSWORD  = "system";


  public static final int BATCH_LIMIT = 200;

  // The following values are copied from ipc.h. do not change it

  public static final int PHONE_NUM_LEN   = 64-1;
  public static final int VPN_LEN  = PHONE_NUM_LEN;
  public static final int VPN_GROUP_LEN	 = 80-1;
  public static final int ZONE_LEN	= 30-1;
  public static final int EMAIL_LEN	= 80-1;

  public static final int ENDPOINTID_LEN	= 128-1;
  public static final int GKID_LEN 	= 256-1;
  public static final int SIPDOMAIN_LEN		= 256-1;
  public static final int SIPURL_LEN	= 256-1;
  public static final int SIPAUTHPASSWORD_LEN = 32-1;

  public static final int RADSERVERADDR_LEN	= 256-1;
  public static final int SECRET_LEN	  = 256-1;

  public static final int ENUMDOMAIN_LEN	= 256-1;

  public static final int CALLID_LEN	= 256-1;
  public static final int H323ID_LEN	= 128-1;



  public static final int CLIENT_ATTR_LEN 	= 28-1;
  public static final int VPNS_ATTR_LEN   	= 28-1;
  public static final int REG_ID_LEN        = 68-1;
  public static final int PASSWORD_LEN		  = 16;
  public static final int TAG_NAME_LEN		  = 32;
  public static final int CALLPLAN_ATTR_LEN	= 96-1;
  public static final int PASSWD_LEN       = 64;
  public static final int TRIGGER_ATTR_LEN	= 28-1;
  public static final int REALM_NAME_LEN    = 32-1;
  public static final int VNET_NAME_LEN    = 32-1;
  public static final int IGRP_NAME_LEN     = 32-1;
  public static final int IFI_NAME	        = 16;
  public static final int MSWNAME_LEN       = 256;
  public static final int CID_BLOCK_UNBLOCK_LEN = 16-1;

  public static final int CALLPLAN_ATTR_DISPLAY_LEN = 28;

  //  do the db operations only in the local database
  public static final int DB_LOCAL_ONLY   = 0x0001;
  //  do the db operations only in the iserver
  public static final int DB_SERVER_ONLY  = 0x0002;
  //  do the db operations on both
  public static final int DB_ALL          = 0x0003;

  // table names
  public final String TABLE_CP      = "callingplan";
  public final String TABLE_CPB     = "cpbinding";
  public final String TABLE_CPR     = "callingroute";
  public final String TABLE_EP      = "endpoint";
  public final String TABLE_VPN     = "vpn";
  public final String TABLE_GRP     = "group";
  public final String TABLE_LOCK    = "locktable";
  public final String TABLE_MSWINFO = "mswinfo";
  public final String TABLE_TRIGGER = "trigger";
  public final String TABLE_REALM   = "realm";
  public final String TABLE_IGRP    = "igrp";
  public final String TABLE_DCR = "dcr";
  public final String TABLE_VNET   = "vnet";

  // mswinfo table field names
  public static final String INFO_ID        = "id";
  public static final String INFO_DETAILS   = "details";
  
  // do not change the following. The order must follow the entries in the 
  //  create statement (IViewDB.java)

  // calling plan table field names;
  public static final String CP_NAME     = "cpname";
  public static final String CP_VPNG     = "vpng";
  public static final String CP_REFRESH  = "refreshTime";

  //  calling plan constants
  public final int INDEX_CP_NAME    = 0;
  public final int INDEX_CP_GROUP   = 1;
  public final int INDEX_CP_REFRESH = 2;

  // calling plan binding table field names;
  public static final String CPB_CPNAME       = "cpname";
  public static final String CPB_CRNAME       = "crname";
  public static final String CPB_REFRESH      = "refreshTime";
  public static final String CPB_STIME_SEC    = "s_sec";
  public static final String CPB_STIME_MIN    = "s_min";
  public static final String CPB_STIME_HOUR   = "s_hour";
  public static final String CPB_STIME_MDAY   = "s_mday";
  public static final String CPB_STIME_MON    = "s_mon";
  public static final String CPB_STIME_YEAR   = "s_year";
  public static final String CPB_STIME_WDAY   = "s_wday";
  public static final String CPB_STIME_YDAY   = "s_yday";
  public static final String CPB_STIME_ISDST  = "s_isdst";
  public static final String CPB_ETIME_SEC    = "e_sec";
  public static final String CPB_ETIME_MIN    = "e_min";
  public static final String CPB_ETIME_HOUR   = "e_hour";
  public static final String CPB_ETIME_MDAY   = "e_mday";
  public static final String CPB_ETIME_MON    = "e_mon";
  public static final String CPB_ETIME_YEAR   = "e_year";
  public static final String CPB_ETIME_WDAY   = "e_wday";
  public static final String CPB_ETIME_YDAY   = "e_yday";
  public static final String CPB_ETIME_ISDST  = "e_isdst";
  public static final String CPB_PRIORITY     = "priority";
  public static final String CPB_FLAG         = "flag";
  
  //  calling plan binding constants
  public final int INDEX_CPB_CPNAME       = 0;
  public final int INDEX_CPB_CRNAME       = 1;
  public final int INDEX_CPB_REFRESH      = 2;
  public final int INDEX_CPB_STIME_SEC    = 3;
  public final int INDEX_CPB_STIME_MIN    = 4;
  public final int INDEX_CPB_STIME_HOUR   = 5;
  public final int INDEX_CPB_STIME_MDAY   = 6;
  public final int INDEX_CPB_STIME_MON    = 7;
  public final int INDEX_CPB_STIME_YEAR   = 8;
  public final int INDEX_CPB_STIME_WDAY   = 9;
  public final int INDEX_CPB_STIME_YDAY   = 10;
  public final int INDEX_CPB_STIME_ISDST  = 11;
  public final int INDEX_CPB_ETIME_SEC    = 12;
  public final int INDEX_CPB_ETIME_MIN    = 13;
  public final int INDEX_CPB_ETIME_HOUR   = 14;
  public final int INDEX_CPB_ETIME_MDAY   = 15;
  public final int INDEX_CPB_ETIME_MON    = 16;
  public final int INDEX_CPB_ETIME_YEAR   = 17;
  public final int INDEX_CPB_ETIME_WDAY   = 18;
  public final int INDEX_CPB_ETIME_YDAY   = 19;
  public final int INDEX_CPB_ETIME_ISDST  = 20;
  public final int INDEX_CPB_PRIORITY     = 21;
  public final int INDEX_CPB_FLAG         = 22;


  // calling plan route table field names;
  public static final String CPR_NAME       = "crname";
  public static final String CPR_DEST       = "dest";
  public static final String CPR_PREFIX     = "prefix";
  public static final String CPR_SRCPREFIX  = "srcPrefix";
  public static final String CPR_REFRESH    = "refreshTime";
  public static final String CPR_SRCPRESENT = "srcPresent";
  public static final String CPR_SRC        = "src";
  public static final String CPR_SRCLEN     = "srcLen";
  public static final String CPR_DESTLEN    = "destLen";
  public static final String CPR_ROUTEFLAGS = "routeFlags";
  public static final String DCR_TRNAME = "trname";
  public static final String DCR_CPNAME = "cpname";


  // calling plan route constants
  public final int INDEX_CPR_NAME         = 0;
  public final int INDEX_CPR_DEST         = 1;
  public final int INDEX_CPR_PREFIX       = 2;
  public final int INDEX_CPR_SRCPREFIX    = 3;
  public final int INDEX_CPR_REFRESHTIME  = 4;
  public final int INDEX_CPR_SRCPRESENT   = 5;
  public final int INDEX_CPR_SRC          = 6;
  public final int INDEX_CPR_SRCLEN       = 7;
  public final int INDEX_CPR_DESTLEN      = 8;
  public final int INDEX_CPR_ROUTEFLAGS   = 9;
  public final int INDEX_DCR_TRNAME = 10;
  public final int INDEX_DCR_CPNAME = 11;

  // group field names
  public static final String GRP_NAME     = "vpngrp";

  // vpn group constants
  public final int INDEX_GRP_NAME         = 0;

  // vpn field names;
  public static final String VPN_NAME     = "vpnnam";
  public static final String VPN_ID       = "vpnid";
  public static final String VPN_GRP      = "vpngrp";
  public static final String VPN_EXTLEN   = "extlen";
  public static final String VPN_PREFIX   = "vpnprefix";
  public static final String VPN_LOCATION = "vpnloc";
  public static final String VPN_CONTACT  = "vpncon";

  // vpn constants
  public final int INDEX_VPN_NAME         = 0;
  public final int INDEX_VPN_ID           = 1;
  public final int INDEX_VPN_GROUP        = 2;
  public final int INDEX_VPN_EXTLEN       = 3;
  public final int INDEX_VPN_PREFIX       = 4;
  public final int INDEX_VPN_LOCATION     = 5;
  public final int INDEX_VPN_CONTACT      = 6;

  // end point table field names;
  public static final String EP_SERIALNUMBER  = "serialNumber";
  public static final String EP_EXTNUMBER     = "extNumber";
  public static final String EP_PHONE         = "phone";
  public static final String EP_FIRSTNAME     = "firstName";
  public static final String EP_LASTNAME      = "lastName";
  public static final String EP_LOCATION      = "location";
  public static final String EP_COUNTRY       = "country";
  public static final String EP_COMMENTS      = "comments";
  public static final String EP_CUSTOMERID    = "customerId";
  public static final String EP_TRUNKGROUP    = "trunkGroup";
  public static final String EP_ZONE          = "zone";
  public static final String EP_EMAIL           = "email";
  public static final String EP_FORWARDEDPHONE    = "forwardedPhone";
  public static final String EP_FORWARDEDVPNPHONE = "forwardedVpnPhone";
  public static final String EP_CALLINGPLANNAME   = "callingPlanName";
  public static final String EP_H323ID            = "h323Id";
  public static final String EP_SIPURI            = "sipUri";
  public static final String EP_SIPCONTACT        = "sipContact";
  public static final String EP_TECHPREFIX        = "techPrefix";
  public static final String EP_PEERGKID          = "peerGkId";
  public static final String EP_H235PASSWORD      = "h235Password";
  public static final String EP_VPNNAME           = "vpnName";
  public static final String EP_OGP               = "ogp";
  public static final String EP_PROXYVALID      = "proxyValid";
  public static final String EP_ISPROXIED       = "isProxied";
  public static final String EP_ISPROXYING      = "isProxying";
  public static final String EP_CALLFORWARDED   = "callForwarded";
  public static final String EP_ISCALLROLLOVER  = "isCallRollover";
  public static final String EP_DEVTYPE         = "devType";
  public static final String EP_PORT            = "port";
  public static final String EP_IPADDR          = "ipaddr";
  public static final String EP_FORWARDEDVPNEXTLEN  = "forwardedVpnExtLen";
  public static final String EP_MAXCALLS            = "maxCalls";
  public static final String EP_MAXINCALLS          = "maxInCalls";
  public static final String EP_MAXOUTCALLS         = "maxOutCalls";
  public static final String EP_PRIORITY            = "priority";
  public static final String EP_RASPORT             = "rasPort";
  public static final String EP_Q931PORT            = "q931Port";
  public static final String EP_CALLPARTYTYPE       = "callpartyType";
  public static final String EP_CURRENTCALLS        = "currentCalls";
  public static final String EP_VENDOR          = "vendor";
  public static final String EP_EXTLEN          = "extLen";
  public static final String EP_SUBNETIP        = "subnetip";
  public static final String EP_SUBNETMASK      = "subnetmask";
  public static final String EP_MAXHUNTS        = "maxHunts";
  public static final String EP_EXTCAPS         = "extCaps";
  public static final String EP_CAPS            =  "caps";
  public static final String EP_STATEFLAGS      = "stateFlags";
  public static final String EP_LAYER1PROTOCOL  = "layer1Protocol";
  public static final String EP_INCEPTIONTIME   = "inceptionTime";
  public static final String EP_REFRESHTIME     = "refreshTime";
  public static final String EP_INFOTRANSCAP    = "infotranscap";
  public static final String EP_SRCINGRESSTG    = "srcingresstg";
  public static final String EP_REALMNAME       = "realmname";
  public static final String EP_IGRPNAME        = "igrpname";
  public static final String EP_DTG             = "dtg";
  public static final String EP_NEWSRCDTG       = "newsrcdtg";
  public static final String EP_NATIP           = "natip";
  public static final String EP_NATPORT         = "natport";
  public static final String EP_CALL_ID_BLOCK      = "callIdBlock";

  // endpoint constants
  public final int INDEX_IEDGE_SERIALNUMBER       = 0;
  public final int INDEX_IEDGE_EXTNUMBER          = 1;
  public final int INDEX_IEDGE_PHONE              = 2;
  public final int INDEX_IEDGE_FIRSTNAME          = 3;
  public final int INDEX_IEDGE_LASTNAME           = 4;
  public final int INDEX_IEDGE_LOCATION           = 5;
  public final int INDEX_IEDGE_COUNTRY            = 6;
  public final int INDEX_IEDGE_COMMENTS           = 7;
  public final int INDEX_IEDGE_CUSTOMERID         = 8;
  public final int INDEX_IEDGE_TRUNKGROUP         = 9;
  public final int INDEX_IEDGE_ZONE               = 10;
  public final int INDEX_IEDGE_EMAIL              = 11;
  public final int INDEX_IEDGE_FORWARDEDPHONE     = 12;
  public final int INDEX_IEDGE_FORWARDEDVPNPHONE  = 13;
  public final int INDEX_IEDGE_CALLINGPLANNAME    = 14;
  public final int INDEX_IEDGE_H323ID             = 15;
  public final int INDEX_IEDGE_SIPURI             = 16;
  public final int INDEX_IEDGE_SIPCONTACT         = 17;
  public final int INDEX_IEDGE_TECHPREFIX         = 18;
  public final int INDEX_IEDGE_PEERGKID           = 19;
  public final int INDEX_IEDGE_H235PASSWORD       = 20;
  public final int INDEX_IEDGE_VPNNAME            = 21;
  public final int INDEX_IEDGE_OGP                = 22;
  public final int INDEX_IEDGE_PROXYVALID         = 23;
  public final int INDEX_IEDGE_ISPROXIED          = 24;
  public final int INDEX_IEDGE_ISPROXYING         = 25;
  public final int INDEX_IEDGE_CALLFORWARDED      = 26;
  public final int INDEX_IEDGE_ISCALLROLLOVER     = 27;
  public final int INDEX_IEDGE_DEVTYPE            = 28;
  public final int INDEX_IEDGE_PORT               = 29;
  public final int INDEX_IEDGE_IPADDR             = 30;
  public final int INDEX_IEDGE_FORWARDEDVPNEXTLEN = 31;
  public final int INDEX_IEDGE_MAXCALLS           = 32;
  public final int INDEX_IEDGE_MAXINCALLS         = 33;
  public final int INDEX_IEDGE_MAXOUTCALLS        = 34;
  public final int INDEX_IEDGE_PRIORITY           = 35;
  public final int INDEX_IEDGE_RASPORT            = 36;
  public final int INDEX_IEDGE_Q931PORT           = 37;
  public final int INDEX_IEDGE_CALLPARTYTYPE      = 38;
  public final int INDEX_IEDGE_CURRENTCALLS       = 39;
  public final int INDEX_IEDGE_VENDOR             = 40;
  public final int INDEX_IEDGE_EXTLEN             = 41;
  public final int INDEX_IEDGE_SUBNETIP           = 42;
  public final int INDEX_IEDGE_SUBNETMASK         = 43;
  public final int INDEX_IEDGE_MAXHUNTS           = 44;
  public final int INDEX_IEDGE_EXTCAPS            = 45;
  public final int INDEX_IEDGE_CAPS               = 46;
  public final int INDEX_IEDGE_STATEFLAGS         = 47;
  public final int INDEX_IEDGE_LAYER1PROTOCOL     = 48;
  public final int INDEX_IEDGE_INCEPTIONTIME      = 49;
  public final int INDEX_IEDGE_REFRESHTIME        = 50;
  public final int INDEX_IEDGE_INFOTRANSCAP       = 51;
  public final int INDEX_IEDGE_SRCINGRESSTG       = 52;
  public final int INDEX_IEDGE_REALMNAME          = 53;
  public final int INDEX_IEDGE_IGRPNAME           = 54;
  public final int INDEX_IEDGE_DTG                = 55;
  public final int INDEX_IEDGE_NEWSRCDTG          = 56;
  public final int INDEX_IEDGE_NATIP              = 57;
  public final int INDEX_IEDGE_NATPORT            = 58;
  public final int INDEX_IEDGE_CALLIDBLOCK        = 59;

  // trigger constants
 
  public static final String TG_NAME  = "name";
  public static final String TG_EVENT = "event";
  public static final String TG_SCRIPT  = "script";
  public static final String TG_DATA    = "data";
  public static final String TG_SRCVENDOR = "srcVendor";
  public static final String TG_DSTVENDOR = "dstVendor";
  public static final String TG_OVERRIDE  = "override";

  public static final int INDEX_TG_NAME  = 0;
  public static final int INDEX_TG_EVENT = 1;
  public static final int INDEX_TG_SCRIPT  = 2;
  public static final int INDEX_TG_DATA    = 3;
  public static final int INDEX_TG_SRCVENDOR = 4;
  public static final int INDEX_TG_DSTVENDOR = 5;
  public static final int INDEX_TG_OVERRIDE  = 6;


  // Realm constants

  public static final String REALM_NAME     = "name";
  public static final String REALM_IFNAME   = "ifName";
  public static final String REALM_VNETNAME   = "vnetName";
  public static final String REALM_VIPNAME  = "vipName";
  public static final String REALM_ID   = "id";
  public static final String REALM_RSA  = "rsa";
  public static final String REALM_MASK = "mask";
  public static final String REALM_SIGPOOLID  = "sigPoolId";
  public static final String REALM_MEDPOOLID  = "medPoolId";
  public static final String REALM_ADDRTYPE   = "addrType";
  public static final String REALM_ASTATUS   = "adminStatus";
  public static final String REALM_OSTATUS   = "opStatus";
  public static final String REALM_IMR  = "imr";
  public static final String REALM_EMR  = "emr";
  public static final String REALM_SIPAUTH    = "sipAuth";
  public static final String REALM_CIDBLOCK   = "cidBlock";
  public static final String REALM_CIDUNBLOCK = "cidUnBlock";
  public static final String REALM_PROXY_REGID = "proxyRegId";
  public static final String REALM_PROXY_PORT = "proxyPort";

  public static final int INDEX_REALM_NAME     = 0;
  public static final int INDEX_REALM_IFNAME   = 1;
  public static final int INDEX_REALM_VIPNAME  = 2;
  public static final int INDEX_REALM_ID   = 3;
  public static final int INDEX_REALM_RSA  = 4;
  public static final int INDEX_REALM_MASK = 5;
  public static final int INDEX_REALM_SIGPOOLID  = 6;
  public static final int INDEX_REALM_MEDPOOLID  = 7;
  public static final int INDEX_REALM_ADDRTYPE   = 8;
  public static final int INDEX_REALM_ASTATUS   = 9;
  public static final int INDEX_REALM_OSTATUS   = 10;
  public static final int INDEX_REALM_IMR  = 11;
  public static final int INDEX_REALM_EMR  = 12;
  public static final int INDEX_REALM_SIPAUTH     = 13;
  public static final int INDEX_REALM_CIDBLOCK    = 14;
  public static final int INDEX_REALM_CIDUNBLOCK  = 15;
  public static final int INDEX_REALM_PROXY_REGID = 16;
  public static final int INDEX_REALM_PROXY_PORT  = 17;
  public static final int INDEX_REALM_VNETNAME   = 18;


  // Vnet constants

  public static final String VNET_NAME     = "name";
  public static final String VNET_IFNAME   = "ifName";
  public static final String VNET_ID   = "id";
  public static final String VNET_RTGTBLID  = "rtgTblId";
  public static final String VNET_GATEWAY = "gateway";
    
    public static final int INDEX_VNET_NAME     = 0;
    public static final int INDEX_VNET_IFNAME   = 1;
    public static final int INDEX_VNET_ID   = 2;
    public static final int INDEX_VNET_RTGTBLID  = 3;
    public static final int INDEX_VNET_GATEWAY = 4;
    

  // IEdge Group constants
  public static final String IGRP_NAME     = "name";
  public static final String IGRP_MAXIN    = "maxIn";
  public static final String IGRP_MAXOUT   = "maxOut";
  public static final String IGRP_MAXTOTAL = "maxTotal";
  public static final String IGRP_IN       = "callsIn";
  public static final String IGRP_OUT      = "callsOut";
  public static final String IGRP_TOTAL    = "callsTotal";
  public static final String IGRP_TIME     = "dndTime";

  public static final int INDEX_IGRP_NAME     = 0;
  public static final int INDEX_IGRP_MAXIN    = 1;
  public static final int INDEX_IGRP_MAXOUT   = 2;
  public static final int INDEX_IGRP_MAXTOTAL = 3;
  public static final int INDEX_IGRP_IN       = 4;
  public static final int INDEX_IGRP_OUT      = 5;
  public static final int INDEX_IGRP_TOTAL    = 6;
  public static final int INDEX_IGRP_TIME     = 7;

  public static final int IGRP_MAX_CALLS      = 65535;

  //Error codes and strings
  public static final String[] errorMsg = new String[] { "Operation Successful",
                                                         "Undefined Error",
                                                         "No Such Entry",
                                                         "Operation Not Permitted",
                                                         "Access Denied",
                                                         "System I/O failed",
                                                         "Duplicate Entry Exists",
                                                         "Invalid Arguments",
                                                         "Insufficient Arguments",
                                                         "Incompatible VPN Parameters",
                                                         "License Limit Reached",
                                                         "Unclassified error" };

   public static final int OK         = 0;   /* No Error */
   public static final int UNDEFINED  = 1;   /* Undefined Error */
        /* general errors */
   public static final int NOENTRY    = 2;   /* No Such Entry */
   public static final int OPNOPERM   = 3;   /* Operation not permitted  */
   public static final int NOACCESS   = 4;   /* Access Denied */
   public static final int IOERROR    = 5;   /* System I/O failed */
   public static final int EXISTS     = 6;   /* Entry exists */
   public static final int INVALARGS  = 7;   /* Invalid Arguments */
   public static final int INSUFFARGS = 8;   /* Insufficient Arguments */
        /* specific ones */
   public static final int BADVPN     = 9;
   public static final int NOLICENSE  = 10;  /* No more licenses for add ports */
   
   public static final int ERROR_MAX  = 11;

   public static final int MAX_POOL_IDS = 255; /* maximum pool id from fce/include/poolalloc.h */
   
   public static final int MAX_MEDIA_PORT   =   65535;
   public static final int MIN_MEDIA_PORT   =   1024;

   public final static int SET_TIMEOUT  = 20;
   public final static int MAX_SET_TIMEOUT  = 120;

   /* seconds to wait before giving up on a get request */
   public final int GET_TIMEOUT = 5;
   public final int MAX_GET_TIMEOUT = 60;

  /* interval in seconds between two multicast packets */
  public final int MCAST_INTERVAL = 30;
  public final int MIN_MCAST_INTERVAL = 30;
  public final int MAX_MCAST_INTERVAL = 300;
  
  /* cleanup thread wakeup time in seconds */
  public final int CLEANUP_SLEEP = 15;
}
