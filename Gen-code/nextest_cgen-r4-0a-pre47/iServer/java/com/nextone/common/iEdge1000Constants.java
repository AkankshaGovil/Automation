package com.nextone.common;

public interface iEdge1000Constants {

	public static final short INVALID			= -1;
	  
	//	H323	commands
	public static final short	H323_CODE			= 0x0000;
	public static final short	H323_TYPE			= 0x0001;
	public static final short	H323_PREF			= 0x0002;
	public static final short	H323_GK_ADDR1		= 0x0003;
	public static final short	H323_GK_ADDR2		= 0x0004;
	public static final short	H323_GK_ID1			= 0x0005;
	public static final short	H323_GK_ID2			= 0x0006;
	public static final short	H323_COUNTRY_CODE	= 0x0007;
	public static final short	H323_AREA_CODE		= 0x0008;
	public static final short	H323_EXCHANGE_CODE	= 0x0009;
	public static final short	H323_GW_ADDR		= 0x000A;
    public static final short   H323_DISABLE_FASTSTART   = 0x000B;  

	//H323 constants

	public static String[]	H323_ADDRTYPE=	{
		"discover",
		"static"
	};


	//Iserver commands
	public static final short	ISERVER_CODE			= 0x0000;
	public static final short	ISERVER_ADDR1			= 0x0001;
	public static final short	ISERVER_ADDR2			= 0x0002;
	public static final short	ISERVER_REGID			= 0x0003;

	//	Portdata commands

	public static final short PORTDATA_CODE		=	0x0000;
	public static final short PORTDATA_CARDS	=	0x0001;

	//	Card commands
	public static final short CARD_TYPE		=	0x0000;
	public static final short CARD_ID		=	0x0001;
	public static final short CARD_CLOCK	=	0x0002;
	public static final short CARD_ISMASTER	=	0x0003;
	public static final short CARD_SPANS	=	0x0004;

	//Card Constants
	public static final String CARD_T1_STRING = "digital_t1";
	public static final String CARD_E1_STRING = "digital_e1";

	public static final short CARD_INVALID	= -1;
	public static final short CARD_T1		= 1;
	public static final short CARD_E1		= 2;
	public static final short CARD_ANALOG	= 3;

    public static final String CARD_CLOCK_SOURCESTRING[] = {"internal",
														"recovered_span_a",
														"recovered_span_b"};


	//Span commands
	public static final short SPAN_TYPE			= 0x0000;
	public static final short SPAN_ID			= 0x0001;
	public static final short SPAN_COUNTRYCODE	= 0x0002;
	public static final short SPAN_DIDPREFIX	= 0x0003;
	public static final short SPAN_PORTS		= 0x0004;

	//	Span constants
	public static final short MAX_CAS_PORTS		= 24;	
	public static final short MAX_T1_PRI_PORTS	= 23;	
	public static final short MAX_E1_PRI_PORTS	= 30;	

	public static final short SPAN_INVALID		= -1;
	public static final short SPAN_CAS			= 1;
	public static final short SPAN_PRI_USER	= 2;
	public static final short SPAN_PRI_NETWORK = 3;

	//	PriSpan commands
	public static final short PRISPAN_MODE			= 0x0005;
	public static final short PRISPAN_SWITCH		= 0x0006;
	public static final short PRISPAN_Q931VARIANT	= 0x0007;
	public static final short PRISPAN_LOOPBACKMODE	= 0x0008;
	public static final short PRISPAN_FRAMINGMODE	= 0x0009;
	public static final short PRISPAN_LINECODE		= 0x000A;

	//Prispan constants

	public static final String []SPAN_MODE_STRING= {"cas",
                                                    "pri_user",
                                                    "pri_network"};

	public static final String []SPAN_VARIANT_STRING= {"at&t_custom",
                                                          "nortel_custom",
                                                          "national_isdn1",
                                                          "national_isdn2",
                                                          "jate",
                                                          "net3",
                                                          "net5",
                                                          "1tr6_bri",
                                                          "1tr6_isdn",
                                                          "vn3",
                                                          "ccitt",
                                                          "q933",
                                                          "q933_t123",
                                                          "ts014",
                                                          "ts013",
                                                          };

	public static final String []SPAN_SWITCH_STRING = {"at&t_4ess",
                                                         "at&t_5ess",
                                                         "nortel_dms100",
                                                         "nortel_dms250",
                                                         "ericsson_md110_t1",
                                                         "ericsson_md110_e1",
                                                         "siemens",
                                                         "ntt",
                                                         "itu_conformant"};


	public static final String []SPAN_FRAMING_STRING= {"Extended Superframe",
                                                       "Superframe SLC96",
                                                       "Superframe",
                                                       "CRC4",
                                                       "No CRC4"};

	public static final String []SPAN_LOOPBACK_STRING = {"no_loopback",
                                                           "local_loopback",
                                                           "remote_loopback"};


	public static final String []SPAN_LINECODE_STRING = {"b8zs",
                                                       "b7zs",
                                                       "ami",
                                                       "hdb3"};

	public static final int		T1_NOSTATUS = 0x40;

	//	T1PriSpan commands
	public static final short T1PRISPAN_LINE_LENGTH		= 0x000B;

	//T1PriSpan constants

	public static final String []SPAN_LINELENGTH_STRING = {"0-133ft",
                                                         "133-266ft",
                                                         "266-399ft",
                                                         "399-533ft",
                                                         "533-655ft"};


	//	E1PriSpan commands
	public static final short E1PRISPAN_TERM_MODE		= 0x000C;

	//	E1PriSpan	commands
	public static final String []SPAN_TERMMODE_STRING	 = {"75ohm",
                                                              "120ohm"};
	//	Port commands
	public static final short PORT_ID				=	0x0000;
	public static final short PORT_TIMESLOT			=	0x0001;
	public static final short PORT_ROLLOVERTYPE		=	0x0002;
	public static final short PORT_STATUS			=	0x0003;
	public static final short PORT_SIGNALLINGMODE	=	0x0004;
	public static final short PORT_EXTERNALGKNUMBER	=	0x0005;
	public static final short PORT_AUTONUMBER		=	0x0006;
	public static final short PORT_ROLLOVERNUMBER	=	0x0007;
	public static final short PORT_H323ID			=	0x0008;
	public static final short PORT_EMAIL			=	0x0009;
	public static final short PORT_ISROLLOVER		=	0x000A;

	public static final short PORT_UNUSED=0;
	public static final short PORT_ACTIVE=1;
	public static final short PORT_DEBUG=2;


	//port constants

	public static final short INVALID_MODE	=	-1;
	public static final short SIGMODE_MIN_PRI_MODE	=	1;
	public static final short SIGMODE_MIN_CAS_MODE	=	3;
	public static final short SIGMODE_PRI_NETWORK	=	2;
	public static final short SIGMODE_MAX_PRI_MODE		=	SIGMODE_PRI_NETWORK;
	public static final short SIGMODE_PRI_USER			=	SIGMODE_MIN_PRI_MODE;
	public static final short SIGMODE_FXO_GROUND_START	=	SIGMODE_MIN_CAS_MODE;
	public static final short SIGMODE_FXS_GROUND_START	=	4;
	public static final short SIGMODE_FXO_LOOP_START	=	5;
	public static final short SIGMODE_FXS_LOOP_START	=	6;
	public static final short SIGMODE_WINK_START		=	7;
	public static final short SIGMODE_WINK_START_WITH_B_AND_D	=	8;
	public static final short SIGMODE_DELAY_DIAL		=	9;
	public static final short SIGMODE_IMMEDIATE_START	=	10;
	public static final short SIGMODE_MAX_CAS_MODE=SIGMODE_IMMEDIATE_START;


	public static final String []PORT_ROLLTYPE_STRING = {"lus","vpn"};


	public static final String []PORT_STATUS_STRING = {"inactive","active","debug"};
	public static final String []PORT_SIGNALLING_STRING= 
  											{"pri_user","pri_network",
  											"fxo_ground_start","fxs_ground_start",
											"fxo_loop_start","fxs_loop_start",
											"wink_start","wink_start_with_b&d",
											"delay_dial","immediate_start"};



	//Prefix commands
	public static final short PREFIX_ID		=	0x0000;
	public static final short PREFIX_VPN	=	0x0001;
	public static final short PREFIX_LUS	=	0x0002;

	//Download commands
	public static final short DOWNLOAD_CODE		=	0x0000;
	public static final short DOWNLOAD_ADDR		=	0x0001;
	public static final short DOWNLOAD_USER		=	0x0002;
	public static final short DOWNLOAD_PASSWORD	=	0x0003;
	public static final short DOWNLOAD_DIR		=	0x0004;
	public static final short DOWNLOAD_FILE		=	0x0005;

	//NAT commands

	public static final short NAT_CODE				=	0x0000;
	public static final short NAT_ISENABLED			=	0x0001;
	public static final short NAT_IPADDR			=	0x0002;
	public static final short NAT_MASK				=	0x0003;
	public static final short NAT_DATA				=	0x0004;
	public static final short NAT_MAP				=	0x0005;
	public static final short NATPROXY_PROTOCOL		=	0x0006;
	public static final short NATPROXY_LOCALIP		=	0x0007;
	public static final short NATPROXY_LOCALMASK	=	0x0008;
	public static final short NATPROXY_PUBLICIP		=	0x0009;
	public static final short NATPROXY_EXTERNALPORT	=	0x000A;
	public static final short NATPROXY_INTERNALPORT	=	0x000B;
	public static final short NATMAP_LOCALIP		=	0x000C;
	public static final short NATMAP_LOCALMASK		=	0x000D;
	public static final short NATMAP_PUBLICIP		=	0x000E;
	public static final short NATMAP_PUBLICMASK		=	0x000F;
    public static final short NAT_IS_PROXYENABLED	=	0x0010;

	//SIP commands
	public static final short SIP_OUT_PROTOCOL			=	0x0000;
	public static final short SIP_REGISTRATION			=	0x0001;
	public static final short SIP_PROXYSERVERPORT		=	0x0002;
	public static final short SIP_REGURI				=	0x0003;
	public static final short SIP_PROTOCOL				=	0x0004;
	public static final short SIP_SERVERADDRESS			=	0x0005;
	public static final short SIP_PROXYSERVERADDRESS	=	0x0006;
	public static final short SIP_DOMAIN				=	0x0007;
        public static final short SIP_ALWAYSROUTETOPROXY                =       0x0008;


	//IPFilter commands

	public static final short IPFILTER_CODE			=	0x0000;
	public static final short IPFILTER_ISENABLED	=	0x0001;
	public static final short IPFILTER_DATA			=	0x0002;
	public static final short IPFILTER_RULE			=	0x0003;
	public static final short IPFILTER_DIRECTION	=	0x0004;
	public static final short IPFILTER_LOG			=	0x0005;
	public static final short IPFILTER_QUICK		=	0x0006;
	public static final short IPFILTER_ONINTERFACE	=	0x0007;
	public static final short IPFILTER_PROTOCOL		=	0x0008;
	public static final short IPFILTER_SOURCEIP		=	0x0009;
	public static final short IPFILTER_SOURCEMASK	=	0x000A;
	public static final short IPFILTER_DESTIP		=	0x000B;
	public static final short IPFILTER_DESTMASK		=	0x000C;
	public static final short IPFILTER_IPOPTIONS	=	0x000D;
	public static final short IPFILTER_KEEP			=	0x000E;
	public static final short IPFILTER_GROUPOPT		=	0x000F;
	public static final short IPFILTER_GROUPNUM		=	0x0010;
	public static final short IPFILTER_SOURCEPORT	=	0x0011;
	public static final short IPFILTER_SOURCEPORTEND=	0x0012;
	public static final short IPFILTER_DESTPORT		=	0x0013;
	public static final short IPFILTER_DESTPORTEND	=	0x0014;
	public static final short IPFILTER_SOURCERANGE	=	0x0015;
	public static final short IPFILTER_SOURCEOP		=	0x0016;
	public static final short IPFILTER_DESTRANGE	=	0x0017;
	public static final short IPFILTER_DESTOP		=	0x0018;


	//	DHCP Server commands
	public static final short DHCPSERVER_CODE			=	0x0000;
	public static final short DHCPSERVER_ISDHCPENABLED	=	0x0001;
	public static final short DHCPSERVER_DOMAINNAME		=	0x0002;
	public static final short DHCPSERVER_ZNB0			=	0x0003;
	public static final short DHCPSERVER_ZNB1			=	0x0004;
	public static final short DHCPSERVER_ZNB0MASK		=	0x0005;
	public static final short DHCPSERVER_ZNB1MASK		=	0x0006;
	public static final short DHCPSERVER_NAMESERVERS	=	0x0007;
	public static final short DHCPSERVER_NETWORKS		=	0x0008;
	public static final short DHCPSERVER_NETMASKS		=	0x0009;
	public static final short DHCPSERVER_SUBNETS		=	0x000A;
	
	public static final short DHCPSUBNET_SUBNET			=	0x000B;
	public static final short DHCPSUBNET_SUBNETMASK		=	0x000C;	
	public static final short DHCPSUBNET_BROADCAST		=	0x000D;
	public static final short DHCPSUBNET_MTU			=	0x000E;
	public static final short DHCPSUBNET_LEASETIME		=	0x000F;
	public static final short DHCPSUBNET_ROUTER			=	0x0010;
	public static final short DHCPSUBNET_DHCPRANGES		=	0x0011;

	public static final short SUBNET_CLIENTID		=	0x0012;
	public static final short SUBNET_FLAG			=	0x0013;
	public static final short SUBNET_CLIENTADDR		=	0x0014;
	public static final short SUBNET_SERVERADDR		=	0x0015;
	public static final short SUBNET_LEASETIME		=	0x0016;

	public static final short DHCPRANGE_SERVERADDR	=	0x0017;
	public static final short DHCPRANGE_STARTADDR	=	0x0018;
	public static final short DHCPRANGE_ENDADDR		=	0x0019;


    //  Codec commands
	public static final short CODECS	            =	0x0000;
	public static final short PREFORDER	            =	0x0001;
	public static final short G7231MAXFRAMES	    =	0x0002;
	public static final short G729MAXFRAMES	        =	0x0003;
	public static final short G711ALAWMAXINTERVAL	=	0x0004;
	public static final short G711ULAWMAXINTERVAL	=	0x0005;

}

