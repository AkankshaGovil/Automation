package com.nextone.common;

import java.io.*;
import java.util.*;
import java.util.zip.*;
import java.net.InetAddress;


/**
 * This class contains utility methods to operate on a configuration file
 *
 */
public class ConfigFile {
  private static Map configCommands;
  private static Map i1000ConfigCommands;


  static {

    i1000ConfigCommands = Collections.synchronizedMap(new HashMap());
    // list of unique commands
    i1000ConfigCommands.put("set factory_defaults:",
			    new CommandProperty(false, new int [] {CommonConstants.DEVTYPE_I1000}, null));

    i1000ConfigCommands.put("h323_prefer:",
			    new CommandProperty(false, new int [] {CommonConstants.DEVTYPE_I1000}, null));
    i1000ConfigCommands.put("h323_gateway address:",
			    new CommandProperty(false, new int [] {CommonConstants.DEVTYPE_I1000}, null));
    i1000ConfigCommands.put("h323_gateway country_code:",
			    new CommandProperty(false, new int [] {CommonConstants.DEVTYPE_I1000}, null));
    i1000ConfigCommands.put("h323_gateway area_code:",
			    new CommandProperty(false, new int [] {CommonConstants.DEVTYPE_I1000}, null));
    i1000ConfigCommands.put("h323_gateway exchange_code:",
			    new CommandProperty(false, new int [] {CommonConstants.DEVTYPE_I1000}, null));
    i1000ConfigCommands.put("h323_gatekeeper address_type:",
			    new CommandProperty(false, new int [] {CommonConstants.DEVTYPE_I1000}, null));
    i1000ConfigCommands.put("h323_gatekeeper primary_address:",
			    new CommandProperty(false, new int [] {CommonConstants.DEVTYPE_I1000}, null));
    i1000ConfigCommands.put("h323_gatekeeper secondary_address:",
			    new CommandProperty(false, new int [] {CommonConstants.DEVTYPE_I1000}, null));
    i1000ConfigCommands.put("h323_gatekeeper gatekeeper_id:",
			    new CommandProperty(false, new int [] {CommonConstants.DEVTYPE_I1000}, null));
    i1000ConfigCommands.put("h323_stack disable_fast_start:",
			    new CommandProperty(false, new int [] {CommonConstants.DEVTYPE_I1000}, null));

    i1000ConfigCommands.put("primary_iserver_address:",
			    new CommandProperty(false, new int [] {CommonConstants.DEVTYPE_I1000}, null));
    i1000ConfigCommands.put("secondary_iserver_address:",
			    new CommandProperty(false, new int [] {CommonConstants.DEVTYPE_I1000}, null));
    i1000ConfigCommands.put("iedge registration_id:",
			    new CommandProperty(true, new int [] {CommonConstants.DEVTYPE_I1000}, null));
    i1000ConfigCommands.put("line_card:type:",
			    new CommandProperty(false, new int [] {CommonConstants.DEVTYPE_I1000}, null));
    i1000ConfigCommands.put("line_card:mvip_master:",
			    new CommandProperty(false, new int [] {CommonConstants.DEVTYPE_I1000}, null));
    i1000ConfigCommands.put("line_card:clock_source:",
			    new CommandProperty(false, new int [] {CommonConstants.DEVTYPE_I1000}, null));
    i1000ConfigCommands.put("digital_span:mode:",
			    new CommandProperty(false, new int [] {CommonConstants.DEVTYPE_I1000}, null));
    i1000ConfigCommands.put("digital_span:t1_line_length:",
			    new CommandProperty(false, new int [] {CommonConstants.DEVTYPE_I1000}, null));
    i1000ConfigCommands.put("digital_span:e1_line_termination:",
			    new CommandProperty(false, new int [] {CommonConstants.DEVTYPE_I1000}, null));
    i1000ConfigCommands.put("digital_span:q931_variant:",
			    new CommandProperty(false, new int [] {CommonConstants.DEVTYPE_I1000}, null));
    i1000ConfigCommands.put("digital_span:switch_type:",
			    new CommandProperty(false, new int [] {CommonConstants.DEVTYPE_I1000}, null));
    i1000ConfigCommands.put("digital_span:framing:",
			    new CommandProperty(false, new int [] {CommonConstants.DEVTYPE_I1000}, null));
    i1000ConfigCommands.put("digital_span:line_loopback:",
			    new CommandProperty(false, new int [] {CommonConstants.DEVTYPE_I1000}, null));
    i1000ConfigCommands.put("digital_span:line_code:",
			    new CommandProperty(false, new int [] {CommonConstants.DEVTYPE_I1000}, null));
    i1000ConfigCommands.put("digital_span:country_code:",
			    new CommandProperty(false, new int [] {CommonConstants.DEVTYPE_I1000}, null));
    i1000ConfigCommands.put("digital_span:did_prefix:",
			    new CommandProperty(false, new int [] {CommonConstants.DEVTYPE_I1000}, null));
    i1000ConfigCommands.put("digital_port:status:",
			    new CommandProperty(false, new int [] {CommonConstants.DEVTYPE_I1000}, null));
    i1000ConfigCommands.put("digital_port:signaling_mode:",
			    new CommandProperty(false, new int [] {CommonConstants.DEVTYPE_I1000}, null));
    i1000ConfigCommands.put("digital_port:phone_number:",
			    new CommandProperty(false, new int [] {CommonConstants.DEVTYPE_I1000}, null));
    i1000ConfigCommands.put("digital_port:h323_id:",
			    new CommandProperty(false, new int [] {CommonConstants.DEVTYPE_I1000}, null));
    i1000ConfigCommands.put("digital_port:email_id:",
			    new CommandProperty(false, new int [] {CommonConstants.DEVTYPE_I1000}, null));
    i1000ConfigCommands.put("digital_port:rollover_number:",
			    new CommandProperty(false, new int [] {CommonConstants.DEVTYPE_I1000}, null));
    i1000ConfigCommands.put("digital_port:rollover_number_type:",
			    new CommandProperty(false, new int [] {CommonConstants.DEVTYPE_I1000}, null));
    i1000ConfigCommands.put("digital_port:autoattendant_number:",
			    new CommandProperty(false, new int [] {CommonConstants.DEVTYPE_I1000}, null));
    i1000ConfigCommands.put("digital_port:autoattendant_number_type:",
			    new CommandProperty(false, new int [] {CommonConstants.DEVTYPE_I1000}, null));
    i1000ConfigCommands.put("vpn_dial_prefix:",	
			    new CommandProperty(false, new int [] {CommonConstants.DEVTYPE_I1000}, null));
    i1000ConfigCommands.put("lus_dial_prefix:",
			    new CommandProperty(false, new int [] {CommonConstants.DEVTYPE_I1000}, null));
    i1000ConfigCommands.put("dhcp enable:",
			    new CommandProperty(false, new int [] {CommonConstants.DEVTYPE_I1000}, null));
    i1000ConfigCommands.put("dhcp disable:",
			    new CommandProperty(false, new int [] {CommonConstants.DEVTYPE_I1000}, null));
    i1000ConfigCommands.put("dhcp_server:DNSdmain:",
			    new CommandProperty(false, new int [] {CommonConstants.DEVTYPE_I1000}, null));
    i1000ConfigCommands.put("dhcp_server:DNSserv:",
			    new CommandProperty(false, new int [] {CommonConstants.DEVTYPE_I1000}, null));
    i1000ConfigCommands.put("dhcp_server:subnet:",
			    new CommandProperty(false, new int [] {CommonConstants.DEVTYPE_I1000}, null));
    i1000ConfigCommands.put("dhcp_server:LeaseTim:",
			    new CommandProperty(false, new int [] {CommonConstants.DEVTYPE_I1000}, null));
    i1000ConfigCommands.put("dhcp_server:Router:",
			    new CommandProperty(false, new int [] {CommonConstants.DEVTYPE_I1000}, null));
    i1000ConfigCommands.put("dhcp_server range:",
			    new CommandProperty(false, new int [] {CommonConstants.DEVTYPE_I1000}, null));
    i1000ConfigCommands.put("dhcp_server install:",
			    new CommandProperty(false, new int [] {CommonConstants.DEVTYPE_I1000}, null));
    i1000ConfigCommands.put("ip_filter add:",
			    new CommandProperty(false, new int [] {CommonConstants.DEVTYPE_I1000}, null));
    i1000ConfigCommands.put("ip_filter install:",
			    new CommandProperty(false, new int [] {CommonConstants.DEVTYPE_I1000}, null));
    i1000ConfigCommands.put("ip_filter on:",
			    new CommandProperty(false, new int [] {CommonConstants.DEVTYPE_I1000}, null));
    i1000ConfigCommands.put("ip_filter off:",
			    new CommandProperty(false, new int [] {CommonConstants.DEVTYPE_I1000}, null));
    i1000ConfigCommands.put("nat_proxy:rdr:",
			    new CommandProperty(true, new int [] {CommonConstants.DEVTYPE_I1000}, null));
    i1000ConfigCommands.put("nat_proxy:map:",
			    new CommandProperty(true, new int [] {CommonConstants.DEVTYPE_I1000}, null));
    i1000ConfigCommands.put("nat_proxy on:",
			    new CommandProperty(true, new int [] {CommonConstants.DEVTYPE_I1000}, null));
    i1000ConfigCommands.put("nat_proxy off:",
			    new CommandProperty(true, new int [] {CommonConstants.DEVTYPE_I1000}, null));
    i1000ConfigCommands.put("nat_proxy save:",
			    new CommandProperty(false, new int [] {CommonConstants.DEVTYPE_I1000}, null));

    i1000ConfigCommands.put("voip_protocol_prefer:",
			    new CommandProperty(false, new int [] {CommonConstants.DEVTYPE_I1000}, null));
    i1000ConfigCommands.put("sip domain:",
			    new CommandProperty(false, new int [] {CommonConstants.DEVTYPE_I1000}, null));
    i1000ConfigCommands.put("sip transport_protocol:",
			    new CommandProperty(false, new int [] {CommonConstants.DEVTYPE_I1000}, null));
    i1000ConfigCommands.put("sip server_address:",
			    new CommandProperty(false, new int [] {CommonConstants.DEVTYPE_I1000}, null));
    i1000ConfigCommands.put("sip proxy_server_address:",
			    new CommandProperty(false, new int [] {CommonConstants.DEVTYPE_I1000}, null));
    i1000ConfigCommands.put("sip proxy_server_port:",
			    new CommandProperty(false, new int [] {CommonConstants.DEVTYPE_I1000}, null));
    i1000ConfigCommands.put("sip registration:",
			    new CommandProperty(false, new int [] {CommonConstants.DEVTYPE_I1000}, null));
    i1000ConfigCommands.put("sip registration_uri:",
			    new CommandProperty(false, new int [] {CommonConstants.DEVTYPE_I1000}, null));

		 
    i1000ConfigCommands.put("download_server_address:",
			    new CommandProperty(false, new int [] {CommonConstants.DEVTYPE_I1000}, null));
    i1000ConfigCommands.put("download_user:",
			    new CommandProperty(false, new int [] {CommonConstants.DEVTYPE_I1000}, null));
    i1000ConfigCommands.put("download_password:",
			    new CommandProperty(false, new int [] {CommonConstants.DEVTYPE_I1000}, null));
    i1000ConfigCommands.put("download_directory:",
			    new CommandProperty(false, new int [] {CommonConstants.DEVTYPE_I1000}, null));
    i1000ConfigCommands.put("download_file:",
			    new CommandProperty(false, new int [] {CommonConstants.DEVTYPE_I1000}, null));


    configCommands = Collections.synchronizedMap(new HashMap());
    // list of unique commands
    configCommands.put("registration_id",
		       new CommandProperty(true, new int [] {CommonConstants.DEVTYPE_ANY}, "erase registration_id"));
    configCommands.put("fqdn",
		       new CommandProperty(true, new int [] {CommonConstants.DEVTYPE_I500, CommonConstants.DEVTYPE_I510}, "erase fqdn"));
    configCommands.put("ip_address",
		       new CommandProperty(true, new int [] {CommonConstants.DEVTYPE_I500, CommonConstants.DEVTYPE_I510}, "set ip_address 10.0.0.1"));
    configCommands.put("nat_ip",
		       new CommandProperty(true, new int [] {CommonConstants.DEVTYPE_ANY}, "erase nat_ip"));
    configCommands.put("nat_proxy",
		       new CommandProperty(true, new int [] {CommonConstants.DEVTYPE_ANY}, null));
    configCommands.put("phone_number",
		       new CommandProperty(true, new int [] {CommonConstants.DEVTYPE_ANY}, null));


    // list of other commands
    configCommands.put("config_version",
		       new CommandProperty(false, new int [] {CommonConstants.DEVTYPE_ANY}, "erase config_version"));
    configCommands.put("admin_password",
		       new CommandProperty(false, new int [] {CommonConstants.DEVTYPE_I500, CommonConstants.DEVTYPE_I510}, "erase admin_password"));
    configCommands.put("user_name",
		       new CommandProperty(false, new int [] {CommonConstants.DEVTYPE_I500, CommonConstants.DEVTYPE_I510}, "erase user_name"));
    configCommands.put("user_password",
		       new CommandProperty(false, new int [] {CommonConstants.DEVTYPE_I500, CommonConstants.DEVTYPE_I510}, "erase user_password"));
    configCommands.put("primary_dhcp_server",
		       new CommandProperty(false, new int [] {CommonConstants.DEVTYPE_I500, CommonConstants.DEVTYPE_I510}, "erase primary_dhcp_server"));
    configCommands.put("secondary_dhcp_server",
		       new CommandProperty(false, new int [] {CommonConstants.DEVTYPE_I500, CommonConstants.DEVTYPE_I510}, "erase secondary_dhcp_server"));
    configCommands.put("primary_dns_ip_address",
		       new CommandProperty(false, new int [] {CommonConstants.DEVTYPE_I500, CommonConstants.DEVTYPE_I510}, "erase primary_dns_ip_address"));
    configCommands.put("secondary_dns_ip_address",
		       new CommandProperty(false, new int [] {CommonConstants.DEVTYPE_I500, CommonConstants.DEVTYPE_I510}, "erase secondary_dns_ip_address"));
    configCommands.put("subnet_mask",
		       new CommandProperty(false, new int [] {CommonConstants.DEVTYPE_I500, CommonConstants.DEVTYPE_I510}, "set subnet_mask 255.0.0.0"));
    configCommands.put("ip_gateway_address",
		       new CommandProperty(false, new int [] {CommonConstants.DEVTYPE_I500, CommonConstants.DEVTYPE_I510}, "set ip_gateway_address 10.0.0.254"));
    configCommands.put("dhcp_client",
		       new CommandProperty(false, new int [] {CommonConstants.DEVTYPE_ANY}, "set dhcp_client enable"));
    configCommands.put("dhcp_hostname",
		       new CommandProperty(false, new int [] {CommonConstants.DEVTYPE_I500, CommonConstants.DEVTYPE_I510}, "set dhcp_hostname"));
    configCommands.put("prefer_h323",
		       new CommandProperty(false, new int [] {CommonConstants.DEVTYPE_ANY}, "set prefer_h323 disable"));
    configCommands.put("h323_gatekeeper_addrtype",
		       new CommandProperty(false, new int [] {CommonConstants.DEVTYPE_ANY}, "set h323_gatekeeper_addrtype discover"));
    configCommands.put("h323_primary_gatekeeper_address",
		       new CommandProperty(false, new int [] {CommonConstants.DEVTYPE_ANY}, "erase h323_primary_gatekeeper_address"));
    configCommands.put("h323_secondary_gatekeeper_address",
		       new CommandProperty(false, new int [] {CommonConstants.DEVTYPE_ANY}, "erase h323_secondary_gatekeeper_address"));
    configCommands.put("h323_gateway_countrycode",
		       new CommandProperty(false, new int [] {CommonConstants.DEVTYPE_I510, CommonConstants.DEVTYPE_I1000}, "erase h323_gateway_countrycode"));
    configCommands.put("h323_gateway_areacode",
		       new CommandProperty(false, new int [] {CommonConstants.DEVTYPE_I510, CommonConstants.DEVTYPE_I1000}, "erase h323_gateway_areacode"));
    configCommands.put("h323_gateway_exchangecode",
		       new CommandProperty(false, new int [] {CommonConstants.DEVTYPE_I510, CommonConstants.DEVTYPE_I1000}, "erase h323_gateway_exchangecode"));
    configCommands.put("default_voip_router_ip_address",
		       new CommandProperty(false, new int [] {CommonConstants.DEVTYPE_I500}, "erase default_voip_router_ip_address"));
    configCommands.put("h323_gatekeeper_id",
		       new CommandProperty(false, new int [] {CommonConstants.DEVTYPE_ANY}, "erase h323_gatekeeper_id"));
    configCommands.put("internet_access_port",
		       new CommandProperty(false, new int [] {CommonConstants.DEVTYPE_I500, CommonConstants.DEVTYPE_I510}, "set internet_access_port ethernet"));
    configCommands.put("dialup_name",
		       new CommandProperty(false, new int [] {CommonConstants.DEVTYPE_I500, CommonConstants.DEVTYPE_I510}, "erase dialup_name"));
    configCommands.put("dialup_password",
		       new CommandProperty(false, new int [] {CommonConstants.DEVTYPE_I500, CommonConstants.DEVTYPE_I510}, "erase dialup_password"));
    configCommands.put("primary_dialup_number",
		       new CommandProperty(false, new int [] {CommonConstants.DEVTYPE_I500, CommonConstants.DEVTYPE_I510}, "erase primary_dialup_number"));
    configCommands.put("secondary_dialup_number",
		       new CommandProperty(false, new int [] {CommonConstants.DEVTYPE_I500, CommonConstants.DEVTYPE_I510}, "erase secondary_dialup_number"));
    configCommands.put("modem_baud",
		       new CommandProperty(false, new int [] {CommonConstants.DEVTYPE_I500, CommonConstants.DEVTYPE_I510}, "set modem_baud 19200"));
    configCommands.put("dialup_script",
		       new CommandProperty(false, new int [] {CommonConstants.DEVTYPE_I500, CommonConstants.DEVTYPE_I510}, "erase dialup_script"));
    configCommands.put("modem_setup_script",
		       new CommandProperty(false, new int [] {CommonConstants.DEVTYPE_I500, CommonConstants.DEVTYPE_I510}, "erase modem_setup_script"));
    configCommands.put("iserver_address",
		       new CommandProperty(false, new int [] {CommonConstants.DEVTYPE_ANY}, "erase iserver_address"));
    configCommands.put("secondary_iserver_address",
		       new CommandProperty(false, new int [] {CommonConstants.DEVTYPE_ANY}, "erase secondary_iserver_address"));
    configCommands.put("nat_mask",
		       new CommandProperty(false, new int [] {CommonConstants.DEVTYPE_ANY}, "erase nat_mask"));
    configCommands.put("nat_tcpto",
		       new CommandProperty(false, new int [] {CommonConstants.DEVTYPE_ANY}, "erase nat_tcpto"));
    configCommands.put("nat_udpto",
		       new CommandProperty(false, new int [] {CommonConstants.DEVTYPE_ANY}, "erase nat_udpto"));
    configCommands.put("nat",
		       new CommandProperty(false, new int [] {CommonConstants.DEVTYPE_ANY}, "set nat disable"));
    configCommands.put("lus_dial_prefix",
		       new CommandProperty(false, new int [] {CommonConstants.DEVTYPE_I500, CommonConstants.DEVTYPE_I510}, "set lus_dial_prefix 7"));
    configCommands.put("vpn_dial_prefix",
		       new CommandProperty(false, new int [] {CommonConstants.DEVTYPE_I500, CommonConstants.DEVTYPE_I510}, "set vpn_dial_prefix $"));
    configCommands.put("itsp_dial_prefix",
		       new CommandProperty(false, new int [] {CommonConstants.DEVTYPE_I500, CommonConstants.DEVTYPE_I510}, "set itsp_dial_prefix 8"));
    configCommands.put("pstn_dial_prefix",
		       new CommandProperty(false, new int [] {CommonConstants.DEVTYPE_I500, CommonConstants.DEVTYPE_I510}, "set pstn_dial_prefix 9"));
    configCommands.put("gateway_dial_prefix",
		       new CommandProperty(false, new int [] {CommonConstants.DEVTYPE_I510}, "erase gateway_dial_prefix"));
    configCommands.put("download_server_address",
		       new CommandProperty(false, new int [] {CommonConstants.DEVTYPE_ANY}, "erase download_server_address"));
    configCommands.put("download_user",
		       new CommandProperty(false, new int [] {CommonConstants.DEVTYPE_ANY}, "erase download_user"));
    configCommands.put("download_password",
		       new CommandProperty(false, new int [] {CommonConstants.DEVTYPE_ANY}, "erase download_password"));
    configCommands.put("download_directory",
		       new CommandProperty(false, new int [] {CommonConstants.DEVTYPE_ANY}, "erase download_directory"));
    configCommands.put("download_file",
		       new CommandProperty(false, new int [] {CommonConstants.DEVTYPE_ANY}, "erase download_file"));
    configCommands.put("softrom_download_file",
		       new CommandProperty(false, new int [] {CommonConstants.DEVTYPE_I500, CommonConstants.DEVTYPE_I510}, "erase softrom_download_file"));
    configCommands.put("external_fax_server",
		       new CommandProperty(false, new int [] {CommonConstants.DEVTYPE_ANY}, "erase external_fax_server"));
    configCommands.put("jitter",
		       new CommandProperty(false, new int [] {CommonConstants.DEVTYPE_I500, CommonConstants.DEVTYPE_I510}, "set jitter 0"));
    configCommands.put("read_permission",
		       new CommandProperty(false, new int [] {CommonConstants.DEVTYPE_ANY}, "erase read_permission"));
    configCommands.put("write_permission",
		       new CommandProperty(false, new int [] {CommonConstants.DEVTYPE_ANY}, "erase write_permission"));
    configCommands.put("ivr_password",
		       new CommandProperty(false, new int [] {CommonConstants.DEVTYPE_I510}, "erase ivr_password"));
    configCommands.put("data_vpn",
		       new CommandProperty(false, new int [] {CommonConstants.DEVTYPE_ANY}, "set data_vpn off"));
    configCommands.put("ip_filter",
		       new CommandProperty(false, new int [] {CommonConstants.DEVTYPE_ANY}, "set ip_filter off"));
    configCommands.put("dhcp_server",
		       new CommandProperty(false, new int [] {CommonConstants.DEVTYPE_ANY}, "set dhcp_server disable"));
    configCommands.put("itsp_dial_entry",
		       new CommandProperty(false, new int [] {CommonConstants.DEVTYPE_I500}, null));
    configCommands.put("itsp_dial_timeout",
		       new CommandProperty(false, new int [] {CommonConstants.DEVTYPE_I500}, null));
    configCommands.put("port_status",
		       new CommandProperty(false, new int [] {CommonConstants.DEVTYPE_I500}, null));
    configCommands.put("port_type",
		       new CommandProperty(false, new int [] {}, null));
    configCommands.put("email_id",
		       new CommandProperty(false, new int [] {CommonConstants.DEVTYPE_I500, CommonConstants.DEVTYPE_I510}, null));
    configCommands.put("h323_id",
		       new CommandProperty(false, new int [] {CommonConstants.DEVTYPE_I500, CommonConstants.DEVTYPE_I510}, null));
    configCommands.put("auto_attendant_num",
		       new CommandProperty(false, new int [] {CommonConstants.DEVTYPE_I510}, null));
    configCommands.put("auto_attendant_num_type",
		       new CommandProperty(false, new int [] {CommonConstants.DEVTYPE_I510}, null));
    configCommands.put("rollover_num",
		       new CommandProperty(false, new int [] {CommonConstants.DEVTYPE_I500}, null));
    configCommands.put("rollover_num_type",
		       new CommandProperty(false, new int [] {CommonConstants.DEVTYPE_I500}, null));
    configCommands.put("hybridtype",
		       new CommandProperty(false, new int [] {CommonConstants.DEVTYPE_I510}, null));
    configCommands.put("line_port_status",
		       new CommandProperty(false, new int [] {CommonConstants.DEVTYPE_I500}, null));
    configCommands.put("emergency_number",
		       new CommandProperty(false, new int [] {CommonConstants.DEVTYPE_I500}, null));
    configCommands.put("busy_lineport_foripcall",
		       new CommandProperty(false, new int [] {CommonConstants.DEVTYPE_I500}, null));
  }

  private ConfigFile () {}

  /**
   * returns the config command list
   */
  public static Map getConfigCommands () {
    return configCommands;
  }

  /**
   * extracts from the file what device type this config is meant for
   *
   * @param file the config file
   * @return one of DEVTYPE_ANY, DEVTYPE_I500, DEVTYPE_I510,
   * DEVTYPE_I1000, DEVTYPE_UNKNOWN
   */
  public static int getConfigDeviceType (String file) {
    // look for the signature
    int first = file.indexOf(CommonConstants.SIGNATURE);
    if (first == -1)
      // does not have the signature stored in the file
      return CommonConstants.DEVTYPE_UNKNOWN;
    first += CommonConstants.SIGNATURE.length();
    int second = file.indexOf(CommonConstants.SIGNATURE, first);
    if (second < first)
      // signature not stored in proper format
      return CommonConstants.DEVTYPE_UNKNOWN;

    String idstr = file.substring(first, second);
    int id = -1;
    try {
      id = Integer.parseInt(idstr);
    } catch (Exception e) {}

    switch (id) {
    case CommonConstants.DEVICE_ID_500:
    case CommonConstants.DEVTYPE_I500:
      return CommonConstants.DEVTYPE_I500;

    case CommonConstants.DEVICE_ID_510:
    case CommonConstants.DEVTYPE_I510:
      return CommonConstants.DEVTYPE_I510;

    case CommonConstants.DEVICE_ID_1000:
      return CommonConstants.DEVTYPE_I1000;

    case CommonConstants.DEVTYPE_ANY:
      return CommonConstants.DEVTYPE_ANY;
    }

    return CommonConstants.DEVTYPE_UNKNOWN;
  }

  /**
   * return the device type of an id
   *
   * @return one of DEVTYPE_I500, DEVTYPE_I510, DEVTYPE_I1000, 
   * DEVTYPE_UNKNOWN
   */
  public static int getDeviceTypeForId (int id) {
    switch (id) {
    case CommonConstants.DEVICE_ID_500:
      return CommonConstants.DEVTYPE_I500;

    case CommonConstants.DEVICE_ID_510:
      return CommonConstants.DEVTYPE_I510;

    case CommonConstants.DEVICE_ID_1000:
      return CommonConstants.DEVTYPE_I1000;
    }

    return CommonConstants.DEVTYPE_UNKNOWN;
  }

  /**
   * return the device id of a type
   *
   * @return one of DEVICE_ID_500, DEVICE_ID_510, DEVICE_ID_1000
   * if type is DEVTYPE_ANY or DEVTYPE_UNKNOWN, it returns -1
   */
  public static int getDeviceIdForType (int type) {
    switch (type) {
    case CommonConstants.DEVTYPE_I500:
      return CommonConstants.DEVICE_ID_500;

    case CommonConstants.DEVTYPE_I510:
      return CommonConstants.DEVICE_ID_510;

    case CommonConstants.DEVTYPE_I1000:
      return CommonConstants.DEVICE_ID_1000;
    }

    return -1;
  }

  /**
   * returns if this command is unique to a given iedge (such as ip address
   * registration id, etc)
   *
   * @return true if the command should be a unique command, false if
   * several iedges can be applied with this command
   * @exception IllegalArgumentException if we don't recognize the
   * command
   */
  public static boolean isUniqueCommand (String commandline, int devType) {
    if(devType	==	CommonConstants.DEVTYPE_I1000 && 
       (commandline.startsWith("set") || commandline.startsWith("erase"))){
      boolean flag;

      Set		keys	=	i1000ConfigCommands.keySet();
      String	temp[]	=	new String[keys.size()];
      String	[]commandKeys		=	(String[])keys.toArray(temp);
      for(int i=0; i <  commandKeys.length; i++){
	StringTokenizer stk = new StringTokenizer(commandKeys[i],":");
	flag	=	true;
	while (stk.hasMoreTokens()&& flag) {
	  if(commandline.indexOf(stk.nextToken())	==	-1)
	    flag	=	false;

	}
	if(flag)
	  return ((CommandProperty)i1000ConfigCommands.get(commandKeys[i])).isUniqueCommand();
      }
      return false;
    }
    else {

      StringTokenizer st = new StringTokenizer(commandline);
      st.nextToken();
      String comamnd = st.nextToken();

      if (!configCommands.containsKey(comamnd))
	throw new IllegalArgumentException("Command not recognized");

      return ((CommandProperty)configCommands.get(comamnd)).isUniqueCommand();
    }
  }

  /**
   * returns if this command is valid for the given device type
   *
   * @return true if it is valid, false otherwise
   * @exception IllegalArgumentException if we don't recognize the
   * command
   */
  public static boolean isCommandValidForDeviceType (String commandLine, int devType) {
    if(devType	==	CommonConstants.DEVTYPE_I1000){
      boolean	flag;
      Set		keys	=	i1000ConfigCommands.keySet();
      String	temp[]	=	new String[keys.size()];
      String	[]commandKeys		=	(String[])keys.toArray(temp);
      if ( commandLine.startsWith("set") || commandLine.startsWith("erase")) {

	for(int i=0; i <  commandKeys.length; i++){
	  StringTokenizer stk = new StringTokenizer(commandKeys[i],":");
	  flag	=	true;
	  while (stk.hasMoreTokens() && flag) {
	    if(commandLine.indexOf(stk.nextToken())	==	-1)
	      flag	=	false;
	  }
	  if(flag)
	    return true;
	}
      }
      return false;
    }
    else{ 
      StringTokenizer st = new StringTokenizer(commandLine);
      st.nextToken();
      String cmd = st.nextToken();

      if (!configCommands.containsKey(cmd))
	throw new IllegalArgumentException("Command not recognized");
      return ((CommandProperty)configCommands.get(cmd)).isValidForDeviceType(devType);

    }
		  
			
  }


  public static class CommandProperty {
    private String defaultStr;
    private boolean isUnique; // should this be unique on a device?
    private int [] devTypes;  // valid for these device types, any of
    // DEVTYPE_I500, DEVTYPE_I510, DEVTYPE_I1000, DEVTYPE_ANY
    // DEVTYPE_ANY500

    public CommandProperty (boolean u, int [] dt, String def) {
      isUnique = u;
      devTypes = dt;
      defaultStr = def;
    }

    public boolean isUniqueCommand () {
      return isUnique;
    }

    public boolean isValidForDeviceType (int dt) {
      for (int i = 0; devTypes != null && i < devTypes.length; i++) {
	if (devTypes[i] == CommonConstants.DEVTYPE_ANY ||
	    devTypes[i] == dt)
	  return true;
      }

      return false;
    }

    public int [] getDeviceTypes () {
      return devTypes;
    }

    public String getDefault () {
      return defaultStr;
    }
  }


  /**
   * creates and returns an instance of a ConfigProvider which talks to
   * the given device
   */
  public static ConfigProvider createConfigProvider (ConfigRetriever cr, Registration reg) throws IOException {
    switch (reg.getDeviceId()) {
    case CommonConstants.DEVICE_ID_500:
    case CommonConstants.DEVICE_ID_510:
      return new ConfigFile500Impl(reg, cr);
    case CommonConstants.DEVICE_ID_1000:
      return new ConfigFile1000Impl(reg, cr);

    }

    throw new IOException("Device id " + reg.getDeviceId() + " not supported yet");
  }


}

