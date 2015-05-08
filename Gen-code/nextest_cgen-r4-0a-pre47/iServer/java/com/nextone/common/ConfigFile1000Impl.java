package com.nextone.common;

import java.io.*;
import java.net.*;
import java.util.*;

import java.text.SimpleDateFormat;
import com.nextone.util.IPAddress;
import com.nextone.util.IPUtil;
import com.nextone.util.SysUtil;





/***********************************************************************
 * This class implements methods to get an iEdge 1000's configuration
 * from the device. 
 * Please do not change the order of the tabs(H323, Iserver etc.). This will affect 
 * the partial configuration. If you change the order please update the
 * partial configuration keys and command parser in PartialConfig.java
 *
 ************************************************************************/

public class ConfigFile1000Impl implements ConfigProvider,iEdge1000Constants{
	private Registration	reg;
	private ConfigRetriever cr;
	private i1000ConfigData configData;


	/**
	* @param reg the registration method received from the iedge we
	* are interested to get the configuration from
	*/
	ConfigFile1000Impl (Registration reg, ConfigRetriever cr) {
		this.reg	=	reg;
		this.cr		=	cr;
		configData	=	new i1000ConfigData(cr);
	}

	
	public String getConfig () throws IOException {

		try {
			configData.ReadFromDevice();
		}catch (IOException ie) {
			throw new IOException(ie.getMessage());
		}


		StringBuffer sb = new StringBuffer();
		String cfg;

		// put some header info
		sb.append(getHeaderInfo());

		// erase exsisting information
		sb.append(getStaleInfo());

		// get h323 set commands
		cfg = getH323Config();

		if (cfg != null)
			sb.append(cfg);
		else
			throw new IOException("H323 Data fetch failed");

		// get iserver set commands
		cfg = getIserverConfig();
		if (cfg != null)
			sb.append(cfg);
		else
			throw new IOException("IServer Data fetch failed");
		// get port set commands
		cfg	=	getPortConfig();
		if(cfg	!=	null)
			sb.append(cfg);
		else
			throw new IOException("Port Data fetch failed");
		// get prefix set commands
		cfg	=	getPrefixConfig();
		if(cfg	!=	null)
			sb.append(cfg);
		else
			throw new IOException("Dialing Prefix Data fetch failed");

		cfg	=	getDhcpServerConfig();
		if(cfg	!=	null)
			sb.append(cfg);
		else
			throw new IOException("DhcpServer Data fetch failed");


		cfg	=	getIPFilterConfig();
		if(cfg	!=	null)
			sb.append(cfg);
		else
			throw new IOException("IP Filter Data fetch failed");

		cfg	=	getNATConfig();
		if(cfg	!=	null)
			sb.append(cfg);
		else
			throw new IOException("NAT Data fetch failed");

		cfg	=	getSIPConfig();
		if(cfg	!=	null)
			sb.append(cfg);
		else
			throw new IOException("SIP Data fetch failed");

		// get software download set commands
		cfg	=	getDownloadConfig();
		if(cfg	!=	null)
			sb.append(cfg);
		else
			throw new IOException("Software download Data fetch failed");

        //  get codec set commands
		cfg	=	getCodecConfig();
		if(cfg	!=	null)
			sb.append(cfg);
		else
			throw new IOException("Codec Data fetch failed");


		return sb.toString();
	}

	/**
	 * Get H323 related set commands
	 */
	private String getH323Config(){

		DataProvider h323	=	configData.getH323Data();

		StringBuffer sb = new StringBuffer();
		Object	ob;


		//	Set command for preferance
		if ( (ob	=	h323.getData(iEdge1000Constants.H323_PREF))	==	null)
			return null;

		if(((Boolean)ob).booleanValue())
			sb.append("set iedge h323_prefer enable "+ "\n");
		else
			sb.append("set iedge h323_prefer disable "+"\n");

		//  Set command for gateway address
		if ( (ob	=	h323.getData(iEdge1000Constants.H323_GW_ADDR))	==	null)
			return null;

		if( ((String)ob).length()	==	0)
			sb.append("set h323_gateway address "+ "\n");
		else
			sb.append("set h323_gateway address "+ (String)ob+ "\n");

		//	Set command for address type
		if ( (ob	=	h323.getData(iEdge1000Constants.H323_TYPE))	==	null)
			return null;

		sb.append("set h323_gatekeeper address_type " + iEdge1000Constants.H323_ADDRTYPE[((Short)ob).shortValue() - 1] + "\n");

		//	Set the gatekeeper primary address
		if ( (ob	=	h323.getData(iEdge1000Constants.H323_GK_ADDR1))	==	null)
			return null;

		if( ((String)ob).length()	==	0)
			sb.append("set h323_gatekeeper primary_address "+ "\n");
		else
			sb.append("set h323_gatekeeper primary_address "+ (String)ob+ "\n");

		//	Set the gatekeeper secondary address
		if ( (ob	=	h323.getData(iEdge1000Constants.H323_GK_ADDR2))	==	null)
			return null;

		if( ((String)ob).length()	==	0)
			sb.append("set h323_gatekeeper secondary_address "+ "\n");
		else
			sb.append("set h323_gatekeeper secondary_address "+ (String)ob+ "\n");


		//	Set the country code
		if ( (ob	=	h323.getData(iEdge1000Constants.H323_COUNTRY_CODE))	==	null)
			return null;

		if(((String)ob).length()	==	0)
			sb.append("set h323_gateway country_code"+ "\n");
		else
			sb.append("set h323_gateway country_code "+ (String)ob+ "\n");

		//	Set the area code
		if ( (ob	=	h323.getData(iEdge1000Constants.H323_AREA_CODE))	==	null)
			return null;

		if(((String)ob).length()	==	0)
			sb.append("set h323_gateway area_code "+ "\n");
		else
			sb.append("set h323_gateway area_code "+ (String)ob+ "\n");

		//	Set the exchange code
		if ( (ob	=	h323.getData(iEdge1000Constants.H323_EXCHANGE_CODE))	==	null)
			return null;

		if(((String)ob).length()	==	0)
			sb.append("set h323_gateway exchange_code "+ "\n");
		else
			sb.append("set h323_gateway exchange_code "+ (String)ob+ "\n");


		//	Set command for gate keeper id
		if ( (ob	=	h323.getData(iEdge1000Constants.H323_GK_ID1))	==	null)
			return null;

		if( ((String)ob).length()	==	0 || ((String)ob).length() >= 128)
			sb.append("set h323_gatekeeper gatekeeper_id "+ "\n");
		else
			sb.append("set h323_gatekeeper gatekeeper_id "+ (String)ob+ "\n");


        //  Set command for disable fast start
        if((ob	=	h323.getData(iEdge1000Constants.H323_DISABLE_FASTSTART))	!=	null){
            boolean fastStart   =   ((Boolean)ob).booleanValue();
            sb.append("set h323_stack disable_fast_start "+fastStart+"\n");
        }

		return sb.toString();
	}


	/**
	 * Get Iserver related set commands
	 */

	private String getIserverConfig(){
		DataProvider	iServer	=	configData.getIServerData();

		StringBuffer sb = new StringBuffer();
		Object	ob;

		//	Set command for primary address
		if( (ob	=	iServer.getData(iEdge1000Constants.ISERVER_ADDR1)) == null)
			return null;

		if( ( (String)ob).length()	==	0)
			sb.append("set primary_iserver_address \n");
		else
			sb.append("set primary_iserver_address "+ (String)ob+ "\n");

		//	Set command for secondary address
		if( (ob	=	iServer.getData(iEdge1000Constants.ISERVER_ADDR2)) == null)
			return null;

		if(( (String)ob).length()	==	0)
			sb.append("set secondary_iserver_address \n");
		else
			sb.append("set secondary_iserver_address "+ (String)ob+ "\n");

		//	Set command for registration id
		if( (ob	=	iServer.getData(iEdge1000Constants.ISERVER_REGID)) == null)
			return null;
		if( ( (String)ob).length()	==	0)
			sb.append("set iedge registration_id \n");
		else
			sb.append("set iedge registration_id "+ (String)ob+ "\n");
		return sb.toString();
	}

	/**
	 * Get port related set commands
	 */

	private String getPortConfig(){
		DataProvider	portData	=	configData.getPortData();

		StringBuffer sb = new StringBuffer();
		Object	ob;

		// Traverse the tree getting the changes.
		if( (ob = portData.getData(iEdge1000Constants.PORTDATA_CARDS))	==	null)
			return null;

		Iterator cards = ((HashMap)ob).values().iterator();
		while (cards.hasNext()) {
			DataProvider card = (DataProvider)cards.next();
			sb.append(getCardCommands(card ));
		}
		return sb.toString();
	}

	private String getCardCommands(DataProvider card){

		StringBuffer commands = new StringBuffer(400);
		Object	ob;
		short	id;
		short	type;

		if((ob	= card.getData(CARD_ID))	==	null)
			return null;
		id	=	((Short)ob).shortValue();

		if((ob	= card.getData(CARD_TYPE))	==	null)
			return null;
		type	=	((Short)ob).shortValue();

		switch(type){
			case 1:	// Digital

			case 2:// Digital	
			{
				if((ob	= card.getData(iEdge1000Constants.CARD_SPANS))	==	null)
					return null;
				String cardtype	=	"empty_";
				if(type	==	1)
					cardtype	=	"digital_t1_";
				else
					cardtype	=	"digital_e1_";

		        commands.append("set line_card "+id+ " type "+cardtype +((HashMap)ob).size()+"_span"+"\n");

				if((ob	= card.getData(iEdge1000Constants.CARD_ISMASTER))	==	null)
					return null;
				commands.append("set line_card "+id+" mvip_master "+((Boolean)ob).booleanValue()+"\n");
				if((ob	= card.getData(iEdge1000Constants.CARD_CLOCK))	==	null)
					return null;
				commands.append("set line_card "+id+" clock_source "+ iEdge1000Constants.CARD_CLOCK_SOURCESTRING[((Short)ob).shortValue() -1]+"\n");
				break;
			}
			case 3: //Analog
				commands.append("set line_card "+id+" type analog\n");
				commands.append("set line_card "+id+" clock_source internal\n");
				break;
		}


		// Append commands for all the Spans

		if((ob	= card.getData(CARD_SPANS))	==	null)
			return null;


		Iterator iter = ( (HashMap)ob).values().iterator();
		while (iter.hasNext()) {
			DataProvider span = (DataProvider)iter.next();
			String cmd	=	getSpanCommands(span, id,type);
			if(cmd	!=	null)
				commands.append(cmd);
		}
		return commands.toString();
	}


	private String getSpanCommands(DataProvider span, short cardId,short cardType){

		short spanId;
		short spanType;
		Object ob;
		StringBuffer commands = new StringBuffer(400);

		if( (ob	=	span.getData(iEdge1000Constants.SPAN_ID)) == null)
			return null;
		spanId	=	((Short)ob).shortValue();

		if( (ob	=	span.getData(iEdge1000Constants.SPAN_TYPE)) == null)
			return null;
		spanType	=	((Short)ob).shortValue();

		switch(spanType){
			case 1: //Casspan
				commands.append("set digital_span "+cardId+"/"+spanId+" mode cas\n");
				break;

			case 2:
			case 3:{
				switch(cardType){
					case 1:{	// T1Prispan
						if( (ob	=	span.getData(iEdge1000Constants.T1PRISPAN_LINE_LENGTH) ) == null)
							return null;
						short lineLength	=	((Short)ob).shortValue();
						if(lineLength	==	iEdge1000Constants.INVALID)
							commands.append("set digital_span "+cardId+"/"+spanId+ " t1_line_length invalid"+"\n");
						else
							commands.append("set digital_span "+cardId+"/"+spanId+ " t1_line_length "+ iEdge1000Constants.SPAN_LINELENGTH_STRING[lineLength-1]+"\n");
						break;
					}
					case 2:{	//E1PriSpan
						if( (ob	=	span.getData(iEdge1000Constants.E1PRISPAN_TERM_MODE) ) == null)
							return null;
						short termMode	=	((Short)ob).shortValue();

						if( termMode	==	iEdge1000Constants.INVALID)
							commands.append("set digital_span "+cardId+"/"+spanId+" e1_line_termination invalid"+"\n");
						else
							commands.append("set digital_span "+cardId+"/"+spanId+ " e1_line_termination "+ iEdge1000Constants.SPAN_TERMMODE_STRING[termMode-1]+"\n");
						break;
				   }
					default:
						break;

				}
				if( (ob	=	span.getData(iEdge1000Constants.PRISPAN_MODE) ) == null)
					return null;
				short	mode	=	((Short)ob).shortValue();

				if(mode	==	iEdge1000Constants.INVALID)
					commands.append("set digital_span "+cardId+"/"+spanId+" mode invalid"+"\n");
				else
					commands.append("set digital_span "+cardId+"/"+spanId+" mode "+iEdge1000Constants.SPAN_MODE_STRING[mode-1]+"\n");

				if( (ob	=	span.getData(iEdge1000Constants.PRISPAN_Q931VARIANT) ) == null)
					return null;
				short	variant	=	((Short)ob).shortValue();

				if(variant	==	iEdge1000Constants.INVALID)
					commands.append("set digital_span "+cardId+"/"+spanId+" q931_variant invalid"+"\n");
				else
					commands.append("set digital_span "+cardId+"/"+spanId+ " q931_variant "+ iEdge1000Constants.SPAN_VARIANT_STRING[variant-1]+"\n");

				if( (ob	=	span.getData(iEdge1000Constants.PRISPAN_SWITCH) ) == null)
					return null;
				short	switchtype	=	((Short)ob).shortValue();

				if(switchtype	==	iEdge1000Constants.INVALID)
					commands.append("set digital_span "+cardId+"/"+spanId+" switch_type invalid"+"\n");
				else
					commands.append("set digital_span "+cardId+"/"+spanId+ " switch_type " + iEdge1000Constants.SPAN_SWITCH_STRING[switchtype-1]+"\n");

				if( (ob	=	span.getData(iEdge1000Constants.PRISPAN_FRAMINGMODE) ) == null)
					return null;
				short	framingMode	=	((Short)ob).shortValue();

				if(framingMode	==	iEdge1000Constants.INVALID)
					commands.append("set digital_span "+cardId+"/"+spanId+" framing invalid"+"\n");
				else
					commands.append("set digital_span "+cardId+"/"+spanId+" framing "+iEdge1000Constants.SPAN_FRAMING_STRING[framingMode-1]+"\n");

				if( (ob	=	span.getData(iEdge1000Constants.PRISPAN_LOOPBACKMODE) ) == null)
					return null;
				short	loopbackMode	=	((Short)ob).shortValue();

				if(loopbackMode	==	iEdge1000Constants.INVALID)
					commands.append("set digital_span "+cardId+"/"+spanId+" line_loopback invalid"+"\n");
				else
					commands.append("set digital_span "+cardId+"/"+spanId+ " line_loopback "+ iEdge1000Constants.SPAN_LOOPBACK_STRING[loopbackMode-1]+"\n");

				if( (ob	=	span.getData(iEdge1000Constants.PRISPAN_LINECODE) ) == null)
					return null;
				short	lineCode	=	((Short)ob).shortValue();

				if(lineCode	==	iEdge1000Constants.INVALID)
					commands.append("set digital_span "+cardId+"/"+spanId+" line_code invalid"+"\n");
				else
					commands.append("set digital_span "+cardId+"/"+spanId+" line_code "+ iEdge1000Constants.SPAN_LINECODE_STRING[lineCode-1]+"\n");
			}
		}

		if( (ob	=	span.getData(iEdge1000Constants.SPAN_COUNTRYCODE) ) == null)
			return null;
		commands.append("set digital_span "+cardId+"/"+spanId+" country_code "+((Integer)ob).intValue()+"\n");

		if( (ob	=	span.getData(iEdge1000Constants.SPAN_DIDPREFIX) ) == null)
			return null;
		commands.append("set digital_span "+cardId+"/"+spanId+" did_prefix "+(String)ob+"\n");

		// Append commands for all the Ports
		if( (ob	=	span.getData(iEdge1000Constants.SPAN_PORTS) ) == null)
			return null;

		Iterator ports = ((TreeMap)ob).values().iterator();
		while (ports.hasNext()) {
			DataProvider port = (DataProvider)ports.next();
			String cmd	=	getPortCommands(port,cardId, spanId );
			if(cmd	!=	null && cmd.length()	!=	0)
				commands.append(cmd);
		}
		return commands.toString();
	}

	private String getPortCommands(DataProvider port, short cardId, short spanId){

		StringBuffer commands = new StringBuffer(400);
		Object	ob;
		if( (ob	=	port.getData(iEdge1000Constants.PORT_TIMESLOT) ) == null)
			return null;
		short	timeSlot	=	((Short)ob).shortValue();

		String uniqueId = cardId+"/"+spanId+"/"+ timeSlot;

		// The set comand for a port is of the form
		//  set [digital_port|analog_port] <slot>/<span>/<timeslot> ........
		if( (ob	=	port.getData(iEdge1000Constants.PORT_STATUS) ) == null)
			return null;
		short status	=	((Short)ob).shortValue();

		if( status	!=	iEdge1000Constants.PORT_UNUSED){

			commands.append("set digital_port "+uniqueId+" status "+iEdge1000Constants.PORT_STATUS_STRING[status]+"\n");

			// there is no need to send the signaling mode for the PRI span
			// as changing the span type will cause the port types to
			// be modified.
			if( (ob	=	port.getData(iEdge1000Constants.PORT_SIGNALLINGMODE) ) == null)
				return null;

			short signalMode	=	((Short)ob).shortValue();

			if (signalMode>= SIGMODE_MIN_CAS_MODE && signalMode <= SIGMODE_MAX_CAS_MODE) {
				commands.append("set digital_port "+uniqueId+" signaling_mode "+iEdge1000Constants.PORT_SIGNALLING_STRING[signalMode-1]+"\n");
			}

			if( (ob	=	port.getData(iEdge1000Constants.PORT_EXTERNALGKNUMBER) ) == null)
				return null;
			String extGKNumber	=	(String)ob;
			if(extGKNumber.length()	!=	0)
				commands.append("set digital_port "+uniqueId+" phone_number "+extGKNumber+"\n");

			if( (ob	=	port.getData(iEdge1000Constants.PORT_H323ID) ) == null)
				return null;
			String h323	=	(String)ob;

			if(h323.length()	!=	0)
				commands.append("set digital_port "+uniqueId+" h323_id "+h323+"\n");

			if( (ob	=	port.getData(iEdge1000Constants.PORT_EMAIL) ) == null)
				return null;

			String email	=	(String)ob;
			if(email.length()	!=	0)
			commands.append("set digital_port "+uniqueId+" email_id "+email+"\n");


			if( (ob	=	port.getData(iEdge1000Constants.PORT_ROLLOVERTYPE) ) == null)
				return null;
			short	type	=	((Short)ob).shortValue();

			if( (ob	=	port.getData(iEdge1000Constants.PORT_ISROLLOVER) ) == null)
				return null;

			if( ((Boolean)ob).booleanValue()){
				if( (ob	=	port.getData(iEdge1000Constants.PORT_ROLLOVERNUMBER) ) == null)
					return null;
				String rollNumber	=	(String)ob;
				if(rollNumber.length()	!=	0){
					commands.append("set digital_port "+uniqueId+" rollover_number "+rollNumber+"\n");
					commands.append("set digital_port "+uniqueId+" rollover_number_type "+PORT_ROLLTYPE_STRING[type]+"\n");
				}
			}
			else{
				if( (ob	=	port.getData(iEdge1000Constants.PORT_AUTONUMBER) ) == null)
					return null;
				String autoNumber	=	(String)ob;
				if(autoNumber.length()	!=	0){
					commands.append("set digital_port "+uniqueId+" autoattendant_number "+autoNumber+"\n");
					commands.append("set digital_port "+uniqueId+" autoattendant_number_type "+PORT_ROLLTYPE_STRING[type]+"\n");
				}
			}
			return commands.toString();
		}
		else
			return "";
	}



	/**
	 * Get Dialing prefix related set commands
	 */

	private String getPrefixConfig(){
		DataProvider	prefix=	configData.getPrefixData();

		StringBuffer sb = new StringBuffer();
		Object ob;

		//	Set command for vpnprefix
		if( (ob	=	prefix.getData(iEdge1000Constants.PREFIX_VPN)) == null)
			return null;
		if(((String)ob).length()	==	0)
			sb.append("set iedge vpn_dial_prefix \n");
		else
			sb.append("set iedge vpn_dial_prefix "+ (String)ob+ "\n");

		//	Set command for vpnprefix
		if( (ob	=	prefix.getData(iEdge1000Constants.PREFIX_LUS)) == null)
			return null;
		if(((String)ob).length()	==	0)
			sb.append("set iedge lus_dial_prefix \n");
		else
			sb.append("set iedge lus_dial_prefix "+(String)ob+ "\n");
		return sb.toString();

	}

	
	/**
	 * Get software download related set commands
	 */

	private String getDownloadConfig(){
		DataProvider	download=	configData.getDownloadData();

		StringBuffer sb = new StringBuffer();
		Object ob;

		if( (ob	=	download.getData(iEdge1000Constants.DOWNLOAD_ADDR)	)	==	null)
			return null;

		if (((String)ob).length()	==	0) {
			sb.append("erase download_server_address\n");
		} else if (IPUtil.isValidIP((String)ob)) {
			sb.append("set download_server_address " + (String)ob + "\n");
		} else {
			System.err.println( "Invalid download server IP adddress - " + (String)ob);
			return null;
		}

		if( (ob	=	download.getData(iEdge1000Constants.DOWNLOAD_USER)	)	==	null)
			return null;

		if (( (String)ob).length()	==	0)
			sb.append("erase download_user\n");
		else
			sb.append("set download_user " + (String)ob+ "\n");

		if( (ob	=	download.getData(iEdge1000Constants.DOWNLOAD_PASSWORD)	)	==	null)
			return null;

		if (( (String)ob).length()	==	0)
			sb.append("erase download_password\n");
		else
			sb.append("set download_password " + (String)ob + "\n");

		if( (ob	=	download.getData(iEdge1000Constants.DOWNLOAD_DIR)	)	==	null)
			return null;

		if (((String)ob).length()	==	0)
			sb.append("erase download_directory\n");
		else
			sb.append("set download_directory " + (String)ob+ "\n");

		if( (ob	=	download.getData(iEdge1000Constants.DOWNLOAD_FILE)	)	==	null)
			return null;

		if (((String)ob).length()	==	0)
			sb.append("erase download_file\n");
		else
			sb.append("set download_file " + (String)ob + "\n");
		return sb.toString();
	}	
	
	/**
	 *	Get NAT related set commands
	 */	

	public String getNATConfig() throws IOException {

		DataProvider	nat=	configData.getNATData();

		StringBuffer sb = new StringBuffer();
		Object ob;

		
		if( (ob	=	nat.getData(iEdge1000Constants.NAT_DATA)	)	==	null)
			return null;

		Enumeration e = ( (Vector)ob).elements();
		int index	=	0;
		while (e.hasMoreElements()) {
			DataProvider natData = (DataProvider)e.nextElement();

			if( (ob	=	natData.getData(iEdge1000Constants.NATPROXY_INTERNALPORT)	)	==	null)
				return null;
			int internalPort	=	((Integer)ob).intValue();

			if( (ob	=	natData.getData(iEdge1000Constants.NATPROXY_EXTERNALPORT)	)	==	null)
				return null;
			int	extPort	=	((Integer)ob).intValue();

			if( (ob	=	natData.getData(iEdge1000Constants.NATPROXY_LOCALIP)	)	==	null)
				return null;
			String localIp	=	(String)ob;

			if( (ob	=	natData.getData(iEdge1000Constants.NATPROXY_LOCALMASK)	)	==	null)
				return null;
			String localMask	=	(String)ob;

			if( (ob	=	natData.getData(iEdge1000Constants.NATPROXY_PUBLICIP)	)	==	null)
				return null;
			String publicIp	=	(String)ob;

			if( (ob	=	natData.getData(iEdge1000Constants.NATPROXY_PROTOCOL)	)	==	null)
				return null;
			String protocol	=	(String)ob;

			String s = "rdr znb0 "+ localIp+ "/"+ localMask+ " port "+ internalPort + " -> " + publicIp + " port " + extPort+ " " + protocol +"\n";
			sb.append("set nat_proxy add " +index + " " + s);
			index++;
		}

		if( (ob	=	nat.getData(iEdge1000Constants.NAT_MAP)	)	==	null)
			return null;

		DataProvider map	=	(DataProvider)ob;

		if( (ob	=	map.getData(iEdge1000Constants.NATMAP_PUBLICMASK)	)	==	null)
			return null;

		String publicMask	=	(String)ob;
		if(publicMask.length()	!=	0){

			if( (ob	=	map.getData(iEdge1000Constants.NATMAP_LOCALIP)	)	==	null)
				return null;
			String localIp	=	(String)ob;

			if( (ob	=	map.getData(iEdge1000Constants.NATMAP_LOCALMASK)	)	==	null)
				return null;
			String localMask	=	(String)ob;

			if( (ob	=	map.getData(iEdge1000Constants.NATMAP_PUBLICIP)	)	==	null)
				return null;
			String publicIp	=	(String)ob;
			sb.append("set nat_proxy add 0 map znb0 "+ localIp+ "/"+ localMask + " -> " +publicIp+ "/"+ publicMask+ " \n");
		}

		sb.append("set nat_proxy save\n");		

		if( (ob	=	nat.getData(iEdge1000Constants.NAT_ISENABLED)	)	==	null)
			return null;

		if (((Boolean)ob).booleanValue())
               sb.append("set nat_proxy on\n");
		else
               sb.append("set nat_proxy off\n");

		if( (ob	=	nat.getData(iEdge1000Constants.NAT_IS_PROXYENABLED)	)	==	null)
			return null;

		if (((Boolean)ob).booleanValue())
               sb.append("set proxy on\n");
		else
               sb.append("set proxy off\n");

		return(sb.toString());
	}


	/**
	 *	Get SIP related set commands
	 */	

	private String getSIPConfig(){
		DataProvider	sip=	configData.getSIPData();
		Object ob;

		if( (ob	=	sip.getData(iEdge1000Constants.SIP_OUT_PROTOCOL)	)	==	null)
			return null;
		String  outProtocol	=	(String)ob;

		if( (ob	=	sip.getData(iEdge1000Constants.SIP_REGISTRATION)	)	==	null)
			return null;
		boolean registration=	((Boolean)ob).booleanValue();

		if( (ob	=	sip.getData(iEdge1000Constants.SIP_PROXYSERVERPORT)	)	==	null)
			return null;
		int port=	((Integer)ob).intValue();

		if( (ob	=	sip.getData(iEdge1000Constants.SIP_REGURI)	)	==	null)
			return null;
		String regUri=	(String)ob;

		if( (ob	=	sip.getData(iEdge1000Constants.SIP_PROTOCOL)	)	==	null)
			return null;
		String protocol=	(String)ob;

		if( (ob	=	sip.getData(iEdge1000Constants.SIP_SERVERADDRESS)	)	==	null)
			return null;
		String serverAddr=	(String)ob;

		if( (ob	=	sip.getData(iEdge1000Constants.SIP_PROXYSERVERADDRESS)	)	==	null)
			return null;
		String proxyServerAddr=	(String)ob;

		if( (ob	=	sip.getData(iEdge1000Constants.SIP_DOMAIN)	)	==	null)
			return null;
		String domain=	(String)ob;

		if ((ob =   sip.getData(iEdge1000Constants.SIP_ALWAYSROUTETOPROXY)) == null)
			return null;
         boolean alwaysRouteToProxy = ((Boolean)ob).booleanValue();
 

		StringBuffer sb = new StringBuffer();

		if (outProtocol.length() > 0)
        	sb.append("set iedge voip_protocol_prefer "+  outProtocol+" \n");
		else
        	sb.append("set iedge voip_protocol_prefer dynamic \n");


		if(domain.length()	!=	0	)
			sb.append("set sip domain " + domain + "\n");

		if(serverAddr.length()	!=	0)
			sb.append("set sip registrar_address " + serverAddr + "\n");

		if(proxyServerAddr.length()	!=	0)
            sb.append("set sip proxy_server_address " + proxyServerAddr + "\n");

		if(port >= 0)
			sb.append("set sip proxy_server_port " + port + "\n");

		if(registration)
			sb.append("set sip registration enable\n");
		else
            sb.append("set sip registration\n");

		if(regUri.length()	!=	0	)
			sb.append("set sip registration_uri "+ regUri +"\n");
		if(protocol.length()	!=0)
			sb.append("set sip transport_protocol "+ protocol  +"\n");

		 sb.append("set sip always_routeto_proxy "+ alwaysRouteToProxy +"\n");

		return sb.toString();

	}


	/**
	 *	Get IPFilter related set commands
	 */	

	public String getIPFilterConfig() throws IOException {

		DataProvider	ipFilter=	configData.getIPFilterData();

		StringBuffer sb = new StringBuffer();
		Object ob;


		if( (ob	=	ipFilter.getData(iEdge1000Constants.IPFILTER_DATA)	)	==	null)
			return null;

		Enumeration e = ( (Vector)ob).elements();
		int index	=	0;
		while (e.hasMoreElements()) {

			DataProvider ipFilterData = (DataProvider)e.nextElement();

			if( (ob	=	ipFilterData.getData(iEdge1000Constants.IPFILTER_RULE)	)	==	null)
				return null;
			String rule	=	(String)ob;

			if( (ob	=	ipFilterData.getData(iEdge1000Constants.IPFILTER_DIRECTION)	)	==	null)
				return null;
			String direction	=	(String)ob;

			if( (ob	=	ipFilterData.getData(iEdge1000Constants.IPFILTER_LOG)	)	==	null)
				return null;
			String log	=	(String)ob;

			if( (ob	=	ipFilterData.getData(iEdge1000Constants.IPFILTER_QUICK)	)	==	null)
				return null;
			String quick	=	(String)ob;

			if( (ob	=	ipFilterData.getData(iEdge1000Constants.IPFILTER_ONINTERFACE)	)	==	null)
				return null;
			String onInterface	=	(String)ob;

			if( (ob	=	ipFilterData.getData(iEdge1000Constants.IPFILTER_PROTOCOL)	)	==	null)
				return null;
			String protocol	=	(String)ob;

			if( (ob	=	ipFilterData.getData(iEdge1000Constants.IPFILTER_SOURCEIP)	)	==	null)
				return null;
			String sourceIp	=	(String)ob;

			if( (ob	=	ipFilterData.getData(iEdge1000Constants.IPFILTER_SOURCEMASK)	)	==	null)
				return null;
			String sourceMask	=	(String)ob;
			
			if( (ob	=	ipFilterData.getData(iEdge1000Constants.IPFILTER_DESTIP)	)	==	null)
				return null;
			String destIp	=	(String)ob;
			
			if( (ob	=	ipFilterData.getData(iEdge1000Constants.IPFILTER_DESTMASK)	)	==	null)
				return null;
			String destMask	=	(String)ob;
			
			if( (ob	=	ipFilterData.getData(iEdge1000Constants.IPFILTER_IPOPTIONS)	)	==	null)
				return null;
			String ipOptions	=	(String)ob;
			
			if( (ob	=	ipFilterData.getData(iEdge1000Constants.IPFILTER_KEEP)	)	==	null)
				return null;
			String keep	=	(String)ob;
			
			if( (ob	=	ipFilterData.getData(iEdge1000Constants.IPFILTER_GROUPOPT)	)	==	null)
				return null;
			String groupOpt	=	(String)ob;
			
			if( (ob	=	ipFilterData.getData(iEdge1000Constants.IPFILTER_GROUPNUM)	)	==	null)
				return null;
			String groupNum	=	(String)ob;
			
			if( (ob	=	ipFilterData.getData(iEdge1000Constants.IPFILTER_SOURCERANGE)	)	==	null)
				return null;
			String sourceRange	=	(String)ob;
			
			if( (ob	=	ipFilterData.getData(iEdge1000Constants.IPFILTER_SOURCEOP)	)	==	null)
				return null;
			String sourceOp	=	(String)ob;
			
			if( (ob	=	ipFilterData.getData(iEdge1000Constants.IPFILTER_DESTRANGE)	)	==	null)
				return null;
			String destRange	=	(String)ob;
			
			if( (ob	=	ipFilterData.getData(iEdge1000Constants.IPFILTER_DESTOP)	)	==	null)
				return null;
			String destOp	=	(String)ob;
			

			if( (ob	=	ipFilterData.getData(iEdge1000Constants.IPFILTER_SOURCEPORT)	)	==	null)
				return null;
			int sourcePort	=	((Integer)ob).intValue();
			
			if( (ob	=	ipFilterData.getData(iEdge1000Constants.IPFILTER_SOURCEPORTEND)	)	==	null)
				return null;
			int sourcePortEnd	=	((Integer)ob).intValue();

			if( (ob	=	ipFilterData.getData(iEdge1000Constants.IPFILTER_DESTPORT)	)	==	null)
				return null;
			int destPort	=	((Integer)ob).intValue();

			if( (ob	=	ipFilterData.getData(iEdge1000Constants.IPFILTER_DESTPORTEND)	)	==	null)
				return null;
			int destPortEnd	=	((Integer)ob).intValue();

			sb.append("set ip_filter add ");
			sb.append(rule);
			sb.append(" "+direction);
			if(log.length()	!=	0)
				sb.append(" "+log);
			if(quick.length()	!=	0){
				sb.append(" "+quick);
			}
			if(onInterface.length()	!=	0)
				sb.append(" on "+onInterface);

			if(protocol.length() != 0 && !protocol.equals("any"))
				sb.append(" proto "+ protocol);
			sb.append(" from ");
			if(sourceIp.equals("any"))
				sb.append(sourceIp);
			else
				sb.append(sourceIp + " mask "+sourceMask);

			if(sourceOp.equals("="))
				sb.append(" port = "+sourcePort);
			else if(sourceRange.length()	!=	0){
				if (sourceRange.equals("in_range"))
					sb.append(" port "+sourcePort + " <> "+ sourcePortEnd);
				else
					sb.append(" port "+sourcePort + " >< "+ sourcePortEnd);
			}
			sb.append(" to ");
			if(destIp.equals("any"))
				sb.append(destIp);
			else
				sb.append(destIp + " mask "+destMask);

			if(destOp.equals("="))
				sb.append(" port = "+destPort);
			else if(destRange.length()	!=	0){
				if (destRange.equals("in_range"))
					sb.append(" port "+destPort + " <> "+ destPortEnd);
				else
					sb.append(" port "+destPort + " >< "+ destPortEnd);
			}

			if(ipOptions.length()	!=	0)
				sb.append(" with " + ipOptions);

			if (keep.length()	!=	0)
				sb.append(" keep " + keep);
			if (groupOpt.length()	!=	0)
				sb.append(" " + groupOpt+ " " + groupNum);
			sb.append("\n");
		}


		if( (ob	=	ipFilter.getData(iEdge1000Constants.IPFILTER_ISENABLED)	)	==	null)
			return null;


		sb.append("set ip_filter install\n");
		if (((Boolean)ob).booleanValue())		
               sb.append("set ip_filter on\n");
		else
               sb.append("set ip_filter off\n");


		return sb.toString();

	}

	public String getDhcpServerConfig() throws IOException {
		DataProvider	dhcpServer=	configData.getDhcpServerData();

		StringBuffer sb = new StringBuffer();
		Object ob;

		//	get domain name
		if( (ob	=	dhcpServer.getData(iEdge1000Constants.DHCPSERVER_DOMAINNAME)	)	==	null)
			return null;
		String domainName	=	(String)ob;

		//	get name servers
		if( (ob	=	dhcpServer.getData(iEdge1000Constants.DHCPSERVER_NAMESERVERS)	)	==	null)
			return null;
		Vector nameServers	=	(Vector)ob;

		//get sub nets
		if( (ob	=	dhcpServer.getData(iEdge1000Constants.DHCPSERVER_SUBNETS)	)	==	null)
			return null;
		HashMap subnets=	(HashMap)ob;

		if( (ob	=	dhcpServer.getData(iEdge1000Constants.DHCPSERVER_ISDHCPENABLED)	)	==	null)
			return null;
		boolean isDhcpEnabled =	((Boolean)ob).booleanValue();


		if(domainName.length()	==	0)
			sb.append("set dhcp_server param domain DNSdmain  "+" \n");
		else
			sb.append("set dhcp_server param domain DNSdmain  "+ domainName+" \n");

		StringBuffer servers	=	new StringBuffer();	
		for (Enumeration e = nameServers.elements() ; e.hasMoreElements() ;) {
				servers.append(e.nextElement());
				servers.append(",");
		}

		if( servers.toString().length()	==	0)
			sb.append("set dhcp_server param domain DNSserv "+" \n");
		else
			sb.append("set dhcp_server param domain DNSserv "+ servers.substring(0,servers.length()-1)+" \n");

		//	Set commands for subnets
		Set subnetKeys	=	subnets.keySet();
		for (Iterator ie = subnetKeys.iterator() ; ie.hasNext();) {
			String subnetIp			=	(String)ie.next();
			DataProvider	subnet	=	(DataProvider)subnets.get(subnetIp);
			if(subnet	!=	null){
				if( (ob	=	subnet.getData(iEdge1000Constants.DHCPSUBNET_SUBNETMASK)	)	==	null)
					return null;
				String mask	=	(String)ob;
				if( (ob	=	subnet.getData(iEdge1000Constants.DHCPSUBNET_LEASETIME)	)	==	null)
					return null;
				Integer leaseTime	=	(Integer)ob;
				if( (ob	=	subnet.getData(iEdge1000Constants.DHCPSUBNET_ROUTER)	)	==	null)
					return null;
				String router	=	(String)ob;

				sb.append("set dhcp_server add subnet "+subnetIp+" "+mask+" \n");
				if(leaseTime.intValue()	!=	-1)
					sb.append("set dhcp_server add LeaseTim "+subnetIp+ " "+leaseTime+" \n");
				if(router.length() !=	0)
					sb.append("set dhcp_server add Router "+subnetIp+ " "+router+" \n");

				if( (ob	=	subnet.getData(iEdge1000Constants.DHCPSUBNET_DHCPRANGES)	)	==	null)
					return null;

				Vector	dhcpRanges	=	(Vector)ob;
				if(dhcpRanges.size()	==	0){
					sb.append("set dhcp_server range clean "+subnetIp +" \n");
					sb.append("set dhcp_server install \n");

				}else{
					for (Enumeration e = dhcpRanges.elements() ; e.hasMoreElements() ;) {
						DataProvider	dhcpRange	=	(DataProvider)e.nextElement();

						if( (ob	=	dhcpRange.getData(iEdge1000Constants.DHCPRANGE_SERVERADDR)	)	==	null)
							return null;
						String serverAddr	=	(String)ob;
						if( (ob	=	dhcpRange.getData(iEdge1000Constants.DHCPRANGE_STARTADDR)	)	==	null)
							return null;
						String startAddr	=	(String)ob;

						if( (ob	=	dhcpRange.getData(iEdge1000Constants.DHCPRANGE_ENDADDR)	)	==	null)
							return null;
						String	endAddr		=	(String)ob;

						sb.append("set dhcp_server range "+ subnetIp+ " "+ serverAddr+" "+startAddr+ " "+ endAddr+" domain\n");
					}
                    sb.append("set dhcp_server install\n");

				}				 
			}

		}

		

		if(isDhcpEnabled)
			sb.append("set dhcp enable\n");
		else
			sb.append("set dhcp disable\n");

		return sb.toString();
	}



	/**
	 * Get Codec related set commands
	 */

	private String getCodecConfig(){
		DataProvider	codec=	configData.getCodecData();

		StringBuffer sb = new StringBuffer();
		Object ob;

		if( (ob	=	codec.getData(iEdge1000Constants.PREFORDER	)	)	==	null)
			return null;

        Vector  prefOrder  =   ((Vector)ob);
        Enumeration e = prefOrder.elements();
        if (e.hasMoreElements()) {
            String pref = "set codec preference_order "+e.nextElement();
            while (e.hasMoreElements()) {
                pref += ", "+e.nextElement();
            }
            pref += "\n";
            sb.append(pref);
        }else{
            sb.append("set codec preference_order\n");
        }

		if( (ob	=	codec.getData(iEdge1000Constants.G7231MAXFRAMES	)	)	==	null)
			return null;
        String value    =   (String)ob;
        sb.append("set codec g723.1_max_frames "+value+"\n");

		if( (ob	=	codec.getData(iEdge1000Constants.G729MAXFRAMES	)	)	==	null)
			return null;
        value    =   (String)ob;
        sb.append("set codec g729_max_frames "+value+"\n");

		if( (ob	=	codec.getData(iEdge1000Constants.G711ALAWMAXINTERVAL	)	)	==	null)
			return null;
        value    =   (String)ob;

        sb.append("set codec g711_a_law_max_interval "+value+"\n");

		if( (ob	=	codec.getData(iEdge1000Constants.G711ULAWMAXINTERVAL	)	)	==	null)
			return null;
        value    =   (String)ob;

        sb.append("set codec g711_u_law_max_interval "+value+"\n");
		return sb.toString();

    }
	// some header in the file
	private String getHeaderInfo () {
		StringBuffer sb = new StringBuffer();
		//		 sb.append("#\n# Generated by " + Constants.VERSION + "\n");
		sb.append("# ");
		sb.append(new SimpleDateFormat("EEE MMM dd hh:mm:ssa yyyy z").format(new Date()));
		sb.append("\n# ");
		sb.append(CommonConstants.SIGNATURE);
		sb.append(reg.getDeviceId());
		sb.append(CommonConstants.SIGNATURE);
		sb.append("\n#\n");
//		sb.append("set registration_id ");
//		sb.append(reg.getRegId());

		return sb.toString();
	}

	//  erase all the information
	private String getStaleInfo () {
		return "set iedge factory_defaults\n";
	}



}

