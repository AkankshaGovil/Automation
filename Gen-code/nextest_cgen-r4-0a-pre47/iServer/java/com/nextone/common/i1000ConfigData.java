package com.nextone.common;

import java.util.*;
import java.io.*;
import java.net.*;
import com.nextone.util.LimitedDataInputStream;

import com.nextone.JUCon.Constants;
import com.nextone.util.IPUtil;


// XML specifics
import org.w3c.dom.*;
import javax.xml.parsers.*;  
import org.xml.sax.SAXException;  
import org.xml.sax.SAXParseException;

/**
 *	Class which stores the IEdge1000 Configuration details
 **/

public class i1000ConfigData {

	ConfigRetriever		cr;
	H323		h323;
	IServer		iserver;
	Prefix		prefix;
	PortData	portData;
	DhcpServer	dhcpServer;
	NAT			nat;
	SIP			sip;
	IPFilter	ipFilter;
	Download	download;
    Codec       codec;

	i1000ConfigData(ConfigRetriever cr){
		this.cr		=	cr;
		h323		=	new	H323();
		iserver		=	new IServer();
		portData	=	new PortData();
		prefix		=	new Prefix();
		dhcpServer	=	new	DhcpServer();
		nat			=	new	NAT();
		sip			=	new	SIP();
		ipFilter	=	new IPFilter();
		download	=	new Download();
        codec       =   new Codec();
	}


	public boolean ReadFromDevice() throws IOException {
		boolean bRet	=	false;

		try{
			bRet	=		h323.ReadFromDevice(cr);
			if(bRet)
				bRet	=	iserver.ReadFromDevice(cr);
			if(bRet)
				bRet	=	portData.ReadFromDevice(cr);
			if(bRet)
				bRet	=	prefix.ReadFromDevice(cr);
			if(bRet)
				bRet	=	dhcpServer.ReadFromDevice(cr);
			if(bRet)
				bRet	=	nat.ReadFromDevice(cr);
			if(bRet)
				bRet	=	sip.ReadFromDevice(cr);
			if(bRet)
				bRet	=	ipFilter.ReadFromDevice(cr);
			if(bRet)
				bRet	=	download.ReadFromDevice(cr);
			if(bRet)
				bRet	=	codec.ReadFromDevice(cr);


		}catch(Exception ioe){
			throw new IOException(ioe.getMessage());
		}

		return(bRet);
	}



	/**
	 *	Get the h323 configuration details
	 */
	public DataProvider getH323Data(){
		return(h323);
	}

	/**
	 *	Get the IServer configuration details
	 */
	public DataProvider getIServerData(){
		return(iserver);
	}

	/**
	 *	Get the Cards configuration details
	 */
	public DataProvider getPortData(){
		return(portData);
	}
	
	/**
	 *	Get the Prefix configuration details
	 */
	public DataProvider getPrefixData(){
		return(prefix);
	}
	/**
	 *	Get the dhcp server configuration details
	 */
	public DataProvider getDhcpServerData(){
		return(dhcpServer);
	}


	/**
	 *	Get the NAT configuration details
	 */
	public DataProvider getNATData(){
		return(nat);
	}

	/**
	 *	Get the SIP configuration details
	 */
	public DataProvider getSIPData(){
		return(sip);
	}

	/**
	 *	Get the IPFilter configuration details
	 */
	public DataProvider getIPFilterData(){
		return(ipFilter);
	}
	/**
	 *	Get the software download configuration details
	 */
	public DataProvider getDownloadData(){
		return(download);
	}


	/**
	 *	Get the codec configuration details
	 */
	public DataProvider getCodecData(){
		return(codec);
	}


	public void extractGetReply (LimitedDataInputStream dis) throws IOException {	
	}


	/**
	 * class which stores h323 configuration details
	 */
	public class H323 implements DataConsumer, DataProvider,iEdge1000Constants{

		short	code;
		short	type;
		boolean	isPref;
        boolean disableFastStart;
		String	gk_primaryAddr;
		String	gk_secondaryAddr;
		String	countryCode;
		String	areaCode;
		String	exchangeCode;
		String	gw_address;
		String	gk_id1;
		String	gk_id2;

		/**
		 * Creates an empty h323 object
		 */
		H323(){
			code	=	-1;
			type	=	0;
			isPref	=	false;
            disableFastStart    =   false;
			gk_primaryAddr	=	"";
			gk_secondaryAddr=	"";
			countryCode	=	"";
			areaCode	=	"";
			exchangeCode=	"";
			gw_address	=	"";
			gk_id1		=	"";
			gk_id2		=	"";
		}

		/**
		 * Get the h323 configuration details
		 */
		public Object getData(short cmd){

			switch(cmd){
				case	iEdge1000Constants.H323_CODE:
						return new Short(code);

				case	iEdge1000Constants.H323_TYPE:
						return new Short(type);

				case	iEdge1000Constants.H323_PREF:
						return new Boolean(isPref);

				case	iEdge1000Constants.H323_GK_ADDR1:
						return gk_primaryAddr;

				case	iEdge1000Constants.H323_GK_ADDR2:
						return gk_secondaryAddr;

				case	iEdge1000Constants.H323_GK_ID1	:
						return gk_id1;

				case	iEdge1000Constants.H323_GK_ID2	:
						return gk_id2;

				case	iEdge1000Constants.H323_COUNTRY_CODE:
						return countryCode;

				case	iEdge1000Constants.H323_AREA_CODE:
						return areaCode;

				case	iEdge1000Constants.H323_EXCHANGE_CODE:
						return exchangeCode;

				case	iEdge1000Constants.H323_GW_ADDR:
						return gw_address;

                case    iEdge1000Constants.H323_DISABLE_FASTSTART:
                        return new Boolean(disableFastStart);
				default:
						return null;
			}
		}

		/**
		 * Reads the h323 configuration details from the 1000 device and stores them
		 */
		public boolean ReadFromDevice(ConfigRetriever cr)  throws IOException {

			boolean	bRet	=	false;
			try{
				bRet	=	cr.retrieve(Constants.H323, this);
			}
			catch(Exception ioe){
				throw new IOException(ioe.getMessage());
			}
			return bRet;
		}
		/**
		 * Reads the h323 configuration details from the 1000 device and stores them
		 */
		public void extractGetReply (LimitedDataInputStream dis) throws IOException {

			code	=	dis.readShort();

			if (code == Constants.H323) {

				try {

					DocumentBuilderFactory dbf = DocumentBuilderFactory.newInstance();
			        DocumentBuilder db = dbf.newDocumentBuilder();
			        Document document = db.parse(dis);
	
					NodeList list = document.getElementsByTagName("h323Data");

					if (list.getLength()>0) {
						Node node = list.item(0);
						NamedNodeMap map = node.getAttributes();
						Node preferred = map.getNamedItem("pref");
						if (preferred != null)
							isPref	=	Boolean.valueOf(preferred.getNodeValue()).booleanValue();
					}

					// Gatekeeper address type
					list = document.getElementsByTagName("GAT");
					if (list.getLength()>0) {
						Node node = list.item(0).getFirstChild();
						if (node != null)
							type	=	Short.parseShort(node.getNodeValue());
					}

					// Primary Gatekeeper Address
					list = document.getElementsByTagName("PGA");
					if (list.getLength()>0) {
						Node node = list.item(0).getFirstChild();
						if (node != null)
							gk_primaryAddr	=	node.getNodeValue();
					}

					// Gateway Country Code
					list = document.getElementsByTagName("GCC");
					if (list.getLength()>0) {
						Node node = list.item(0).getFirstChild();
						if (node != null)
							countryCode	=	node.getNodeValue();
					}

					// Gateway Area Code
					list = document.getElementsByTagName("GAC");
					if (list.getLength()>0) {
						Node node = list.item(0).getFirstChild();
						if (node != null)
							areaCode	=	""+node.getNodeValue();
					}

					// Gateway Exchange Code
					list = document.getElementsByTagName("GEC");
					if (list.getLength()>0) {
						Node node = list.item(0).getFirstChild();
						if (node != null)
							exchangeCode	=	node.getNodeValue();
					}

					// Gatekeeper ID
					list = document.getElementsByTagName("GID");
					if (list.getLength()>0) {
						Node node = list.item(0).getFirstChild();
						if (node != null)
							gk_id1	=	node.getNodeValue();
					}

					// VOIP Gateway
					list = document.getElementsByTagName("VG");
					if (list.getLength()>0) {
						Node node = list.item(0).getFirstChild();
						if (node != null)
							gw_address	=	node.getNodeValue();
					}


                    // disable fast start
                    disableFastStart    =   false;
                    list    =   document.getElementsByTagName("FastStart");

                    if (list!= null && list.getLength()>0) {
                        Node node   =   list.item(0).getFirstChild();
                        if(node     !=  null){
                            NamedNodeMap attribs = node.getAttributes();
                            Node valueNode       = attribs.getNamedItem("disable");

                            if (valueNode != null) {
                                String disable = valueNode.getNodeValue();
                                 disableFastStart    =   (new Boolean(disable)).booleanValue();
                            } 
                        }
                    } else {
                            System.err.println("Received an invalid disable fast start value.");
                    }

				}catch (SAXException sxe) {
					throw new IOException("H323 config get request error" + "\n" + sxe.getMessage());
				}catch (ParserConfigurationException pce) {
					throw new IOException("H323 config get request error" + "\n" + pce.getMessage());
				}catch (DOMException de){
					throw new IOException("H323 config get request error" + "\n" + de.getMessage());
				} catch(Exception ioe) {
					// This will catch all parser and IO errors.
					throw new IOException("H323 config get request error" + "\n" + ioe.getMessage());
		
				}

			}

		}//	Function end

	}

	/**
	 * class which stores Iserver configuration details
	 */

	public class IServer implements DataConsumer, DataProvider,iEdge1000Constants{

		short	code;
		String	primaryAddr;
		String	secondaryAddr;
		String	regId;

		IServer(){
			code			=	-1;
			primaryAddr		=	"";
			secondaryAddr	=	"";
			regId			=	"";	
		}

		public Object getData(short cmd){

			switch(cmd){
				case	iEdge1000Constants.ISERVER_CODE:
					return new Short(code);
				case	iEdge1000Constants.ISERVER_ADDR1:
					return primaryAddr;
				case	iEdge1000Constants.ISERVER_ADDR2:
					return secondaryAddr;
				case	iEdge1000Constants.ISERVER_REGID:
					return regId;
				default:
					return null;

			}
		
		}


		public boolean ReadFromDevice(ConfigRetriever cr)  throws IOException {

			boolean	bRet	=	false;
			try{
				bRet	=	cr.retrieve(Constants.ISERVER, this);
			}
			catch(Exception ioe){
				throw new IOException(ioe.getMessage());
			}
			return bRet;
		}

		public void extractGetReply (LimitedDataInputStream dis) throws IOException {

			code	= dis.readShort();

			if (code == Constants.ISERVER) {
			//////////////////////////////////////
			// Read the XML DOM
			//////////////////////////////////////
				try {

					DocumentBuilderFactory dbf = DocumentBuilderFactory.newInstance();
			        DocumentBuilder db = dbf.newDocumentBuilder();
			        Document document = db.parse(dis);

					// Get the iServer addresses
					NodeList list = document.getElementsByTagName("Primary");
					if (list.getLength()>0) {
						Node node = list.item(0).getFirstChild();
						if (node != null)
							primaryAddr	=	node.getNodeValue();
					}

					list = document.getElementsByTagName("Secondary");
					if (list.getLength()>0) {
						Node node = list.item(0).getFirstChild();
						if (node != null)
							secondaryAddr	=	node.getNodeValue();
					}

					// Get the registration ID
					list = document.getElementsByTagName("RegistrationID");
					if (list.getLength()>0) {
						Node node = list.item(0).getFirstChild();
						if (node != null) //##### This should never be null
							regId	=	node.getNodeValue();
					}
				}catch (SAXParseException spe){
					throw new IOException("Server config get request error" + "\n" + spe.getMessage());
				}catch (SAXException sxe) {
					throw new IOException("Server config get request error" + "\n" + sxe.getMessage());
				}catch (ParserConfigurationException pce) {
					throw new IOException("Server config get request error" + "\n" + pce.getMessage());
				}catch (DOMException de){
					throw new IOException("Server config get request error" + "\n" + de.getMessage());
				} catch(Exception ioe) {
					// This will catch all parser and IO errors.
					throw new IOException("Server config get request error" + "\n" + ioe.getMessage());

				}

			}//	Function end
		}

	}

	/**
	 * class which stores cards configuration details
	 */

	public class PortData implements DataConsumer, DataProvider,iEdge1000Constants{
		short code;

		HashMap	cards;

		PortData(){
			code	=	-1;
			cards	=	new HashMap();
		}

		/**
		 * Get the Card configuration details
		 */

		public Object getData(short cmd){
			switch(cmd){
				case	iEdge1000Constants.PORTDATA_CODE:	
					return new Short(code);
				case	iEdge1000Constants.PORTDATA_CARDS:
					return cards;
				default:
					return null;

			}
		}

		public boolean ReadFromDevice(ConfigRetriever cr)  throws IOException {

			boolean	bRet	=	false;
			try{
				bRet	=	cr.retrieve(Constants.PHONE_PORT, this);
			}
			catch(Exception ioe){
				throw new IOException(ioe.getMessage());
			}
			return bRet;
		}

		public void extractGetReply (LimitedDataInputStream dis) throws IOException {

			code	=	dis.readShort();
			HashMap spans;


			if (code == Constants.PHONE_PORT) {
			// Read the data from the input stream and call
			// the constructor on the PortConfigData. The PortConfigData
			// will extract it's related data from the DOM.
				try {
					DocumentBuilderFactory dbf = DocumentBuilderFactory.newInstance();
			        DocumentBuilder db = dbf.newDocumentBuilder();
			        Document document = db.parse(dis);

					cards	=	new HashMap();
					NodeList list = document.getElementsByTagName("Card");

					for (int i=0; i<list.getLength(); i++) {
						Node node = list.item(i);

						// For each card type create the correct instance.
						Card card;

						NamedNodeMap map = node.getAttributes();
						Node cardType = map.getNamedItem("type");
						Node cardID = map.getNamedItem("id");
						Node clkSource = map.getNamedItem("clkSource");
						Node mvipMaster = map.getNamedItem("mvip");

						if (cardType == null || cardID == null) {
							//Invalid entry
							continue;
						}
						short id = Short.parseShort(cardID.getNodeValue());
						short type = Short.parseShort(cardType.getNodeValue());
						short src = Short.parseShort(clkSource.getNodeValue());
						short master = Short.parseShort(mvipMaster.getNodeValue());

						switch (type) {
							case 1:
								// Digital
								spans	=	getSpanList(node,type);
								card = new DigitalT1Card(id, src,(master!=0),spans);

								break;
							case 2:
								// Digital
								spans	=	getSpanList(node, type);
								card = new DigitalE1Card(id, src,(master!=0),spans);

								break;
							case 3:
								// Analog
								card = new AnalogCard(id);

								break;
							case 0:
								continue;

							default:
								throw new Exception("Invalid Card type received");
						}
						cards.put(new Short(id),card);
					}

				}catch (SAXParseException spe){
					throw new IOException("Port config get request error" + "\n" + spe.getMessage());
				}catch (SAXException sxe) {
					throw new IOException("Port config get request error" + "\n" + sxe.getMessage());
				}catch (ParserConfigurationException pce) {
					throw new IOException("Port config get request error" + "\n" + pce.getMessage());
				}catch (DOMException de){
					throw new IOException("Port config get request error" + "\n" + de.getMessage());
				} catch(Exception ioe) {
				// This will catch all parser and IO errors.
					throw new IOException("Port config get request error" + "\n" + ioe.getMessage());

				} 
			}

		}//	Function end

		
		/**
		 * Get the span list for the given card
		 * @param aCard parent
		 */
		public HashMap getSpanList(Node aCard, short theCardType){

			HashMap spanList	=	new HashMap();
			TreeMap	portList;

			// Read the Spans from the DOM   
			NodeList list = aCard.getChildNodes();
			for (short i=0; i<list.getLength(); i++) {
				Node node = list.item(i);

				// For each card type create the correct instance.
				Span span;
				NamedNodeMap map = node.getAttributes();
				Node spanModeNode = map.getNamedItem("mode");
				if (spanModeNode == null) {
					System.err.println("Received null span mode");
					continue;
				}
      
				Node spanIDNode = map.getNamedItem("id");
				if (spanIDNode == null) {
					System.err.println("Received null span id");
					continue;
				}

				Node countryCodeNode = map.getNamedItem("cc");
				if (countryCodeNode == null) {
					System.err.println("Received null country code");
					continue;
				}

				Node switchTypeNode = map.getNamedItem("type");
				if (switchTypeNode == null) {
					System.err.println("Received null switch type");
					continue;
				}

				Node q931VariantNode = map.getNamedItem("q931");
				if (q931VariantNode == null) {
					System.err.println("Received null q931Variant");
					continue;
				}
      
				Node lineCode = map.getNamedItem("lc");
				if (lineCode == null)  {
					System.err.println("Received null line code for span");
					continue;
				}

				Node loopbackNode = map.getNamedItem("lb");
				if (loopbackNode == null) {
					System.err.println("Received null loopback node");
					continue;
				}

				Node framingNode = map.getNamedItem("fr");
				if (framingNode == null) {
					System.err.println("Received null framing node for span");
					continue;
				}

				Node didNode = map.getNamedItem("did");
				if (didNode == null) {
					System.err.println("Received null DID node for span");
					continue;
				}

				Node lineLengthNode = map.getNamedItem("ll"); // T1 Specific
				Node lineTermNode = map.getNamedItem("lt"); // E1 Specific
				if (lineLengthNode == null && lineTermNode == null) {
					System.err.println("Received null line length and termination");
					continue;
				}

				short id = Short.parseShort(spanIDNode.getNodeValue());
				short mode = Short.parseShort(spanModeNode.getNodeValue());
				int countryCode = Integer.parseInt(countryCodeNode.getNodeValue());
				String did;
				if (didNode == null) {
					did = "";
				} else {
					did = didNode.getNodeValue();
				}

				switch (mode) {
					case 1:
						// CAS
						portList	=	getPortList(node, iEdge1000Constants.MAX_CAS_PORTS);
						span = new CasSpan(id,did,countryCode,portList);
					break;
					case 2:
					case 3: {
						// PRI User = 2
						// PRI Network = 3
						short switchType = Short.parseShort(switchTypeNode.getNodeValue());
                        if( switchType <= 0 || switchType > iEdge1000Constants.SPAN_SWITCH_STRING.length)
                            switchType  =   iEdge1000Constants.INVALID;
                        
						short q931 = Short.parseShort(q931VariantNode.getNodeValue());


                        if( q931 <= 0 || q931 > iEdge1000Constants.SPAN_VARIANT_STRING.length)
                            q931  =   iEdge1000Constants.INVALID;

						short loop = Short.parseShort(loopbackNode.getNodeValue());


                        if( loop <= 0 || loop > iEdge1000Constants.SPAN_LOOPBACK_STRING.length)
                            loop  =   iEdge1000Constants.INVALID;

						short code = Short.parseShort(lineCode.getNodeValue());


                        if( code <= 0 || code > iEdge1000Constants.SPAN_LINECODE_STRING.length)
                            code  =   iEdge1000Constants.INVALID;


						short frame = Short.parseShort(framingNode.getNodeValue());


                        if( frame <= 0 || frame > iEdge1000Constants.SPAN_FRAMING_STRING.length)
                            frame  =   iEdge1000Constants.INVALID;


						// Create the correct span mode.
						// We have already ascertained that it is a digital card
						switch (theCardType) {
							case iEdge1000Constants.CARD_T1:
								short len = Short.parseShort(lineLengthNode.getNodeValue());
                                if( len <= 0 || len > iEdge1000Constants.SPAN_LINELENGTH_STRING.length)
                                    len  =   iEdge1000Constants.INVALID;

								portList	=	getPortList(node, iEdge1000Constants.MAX_T1_PRI_PORTS);
								span = new T1PriSpan(mode,id,did,mode,switchType,q931,len,loop,frame,code,countryCode,portList);
								break;
							case iEdge1000Constants.CARD_E1:
								short termination = Short.parseShort(lineTermNode.getNodeValue());

                                if( termination <= 0 || termination > iEdge1000Constants.SPAN_TERMMODE_STRING.length)
                                    termination  =   iEdge1000Constants.INVALID;

								portList	=	getPortList(node, iEdge1000Constants.MAX_E1_PRI_PORTS);
								span = new E1PriSpan(mode,id,did,mode,switchType,q931,termination,loop,frame,code,countryCode,portList);
								break;
							default:
								System.err.println("Unsupported Card Type "+theCardType);
								continue;
						}
						break;
					}
					default:
						// Incorrect span configuration received.
						// Ignore this span.
						System.err.println("Received invalid span mode for card");
						continue;
				}

				spanList.put(new Short(id),span);
			}

			return spanList;

		}//	Function end



		/**
		 * Get the port list for the given span
		 * @param	id port id
		 * @param	aport parent
		 */

		public TreeMap getPortList(Node aSpan, int maxPorts){

			TreeMap portList	=	new TreeMap();

			// Read the Spans from the DOM   
			NodeList list = aSpan.getChildNodes(); // Get the ports
			for (int i=0; i<list.getLength(); i++) {
				Node portNode = list.item(i);

				NamedNodeMap map = portNode.getAttributes();
				Node portID = map.getNamedItem("id");
				if (portID == null) {
					System.err.println("Received PortID of null from iEdge");
					continue;
				}
				short id = Short.parseShort(portID.getNodeValue());

				Port port = getPort(id, portNode);

				if (portList.size() < maxPorts) {
					portList.put(new Short(id), port);
				} else {
					// This is allowable when switching from CAS to PRI. Just do not add the final port.
					System.err.println("Invalid port value found. Size:"+portList.size()+" ID :"+id+" Max:"+maxPorts);
					break;
				}
			}
			return portList;
		}//Function end


		/**
		 * Get the port details
		 * @param	id port id
		 * @param	aport parent
		 */

		public Port getPort(short id, Node aPort){

			short	timeSlot		=	-1;
			short	rollOverType	=	-1;
			short	status			=	-1;
			short	signallingMode		=	-1;
			String	externalGKNumber	=	"";
			String	autoNumber			=	"";
			String	rollOverNumber		=	"";
			String	h323Id				=	"";
			String	email				=	"";

			// Read the Port data from the DOM   
			NodeList list = aPort.getChildNodes(); // Get the ports
			for (int i=0; i<list.getLength(); i++) {
				Node portNode = list.item(i);
				if (portNode.getNodeName().equals("PN")) {
					// Phone Number
					if (portNode.getFirstChild() != null) {
						externalGKNumber	=	portNode.getFirstChild().getNodeValue();
					}
				} else if (portNode.getNodeName().equals("TS")) {
				// Timeslot
					if (portNode.getFirstChild() != null) {
						 timeSlot	=	Short.parseShort(portNode.getFirstChild().getNodeValue()) ;
					}	
				} else if (portNode.getNodeName().equals("AA")) {
					// Auto Attendant Number
			        if (portNode.getFirstChild() != null) {
						autoNumber	=	portNode.getFirstChild().getNodeValue();
					}
				} else if (portNode.getNodeName().equals("RO")) {
					// Rollover Number
					if (portNode.getFirstChild() != null) {
						rollOverNumber	=	portNode.getFirstChild().getNodeValue();
					}
				} else if (portNode.getNodeName().equals("RT")) {
					// Rollover Type
					if (portNode.getFirstChild() != null) {
						// Edge starts at 1.
						rollOverType	=	Short.parseShort(portNode.getFirstChild().getNodeValue());
						rollOverType	-= 1;
					}
				} else if (portNode.getNodeName().equals("SM")) {
					if (portNode.getFirstChild() != null) {
						// Signaling Mode.
						signallingMode	=	Short.parseShort(portNode.getFirstChild().getNodeValue());
					}
				} else if (portNode.getNodeName().equals("St")) {
					if (portNode.getFirstChild() != null) {
						// Status
						status	=	Short.parseShort(portNode.getFirstChild().getNodeValue());
					}
				} else if (portNode.getNodeName().equals("Em")) {
					// Email address
					if (portNode.getFirstChild() != null) {
						email	=	portNode.getFirstChild().getNodeValue();
					}
				} else if (portNode.getNodeName().equals("H3")) {
					// H323ID address
					if (portNode.getFirstChild() != null) {
						h323Id	=	portNode.getFirstChild().getNodeValue();
					}
				}
			}

			return new Port(id,timeSlot,rollOverType,status,signallingMode,externalGKNumber,autoNumber,rollOverNumber,h323Id,email);

		}//	Function end

		public class Card implements DataProvider,iEdge1000Constants{
			short	type;
			short	id;
			short	clock;
			boolean	isMaster;
			HashMap	spans;


			public Card(){
				type	=	CARD_INVALID;
				id		=	-1;
				clock	=	0;
				isMaster	=	false;
				spans	=	null;
			}

			public Card(short type, short id, short clock, boolean isMaster, HashMap spans){
				this.type	=	type;
				this.id		=	id;
				this.clock	=	clock;
				this.isMaster	=	isMaster;
				this.spans	=	spans;

			}

			public Object getData(short cmd){
				switch(cmd){
				case	iEdge1000Constants.CARD_TYPE:
					return	new	Short(type);
				case	iEdge1000Constants.CARD_ID:
					return	new	Short(id);
				case	iEdge1000Constants.CARD_CLOCK:
					return new Short(clock);
				case	iEdge1000Constants.CARD_ISMASTER:
					return	new Boolean(isMaster);
				case	iEdge1000Constants.CARD_SPANS:
					return	spans;
				default:
					return	null;
				}
			}

		}

		public class AnalogCard extends Card{

			public AnalogCard(short id){
				super(iEdge1000Constants.CARD_ANALOG,id, (short)-1, false, null);
			}
		}

		public class DigitalCard extends Card{
			String cardTypeinString;

			public DigitalCard(short type, short id, short clock, boolean isMaster, HashMap spans) {
				super(type, id, clock, isMaster, spans);
			}


		}


		public class DigitalT1Card extends DigitalCard{

			public DigitalT1Card(short id, short clock, boolean isMaster, HashMap spans) {
				super(iEdge1000Constants.CARD_T1,id,clock,isMaster,spans);
				cardTypeinString = iEdge1000Constants.CARD_T1_STRING;
			}

		}

		public class DigitalE1Card extends DigitalCard{

			public DigitalE1Card( short id, short clock, boolean isMaster, HashMap spans) {
				super(iEdge1000Constants.CARD_E1,id,clock,isMaster, spans);
				cardTypeinString = iEdge1000Constants.CARD_E1_STRING;
			}
		}

		public class Span implements DataProvider,iEdge1000Constants{
			short	type;
			short	id;
			int		countryCode;
			String	didPrefix;
			TreeMap	ports;


			public Span(){
				type	=	SPAN_INVALID;
				id		=	-1;
				countryCode	=	-1;
				didPrefix	=	"";
				ports	=	null;
			}

			public Span(short type,short id, String aDID, int countryCode, TreeMap ports) {
				this.type	=	type;
				this.id		=	id;
				this.didPrefix	= (aDID == null)?"":aDID;
				this.ports			=	ports;
				this.countryCode	=	countryCode;
			}


			public Object	getData(short cmd){

				switch(cmd){
					case	iEdge1000Constants.SPAN_TYPE:		
						return new Short(type);
					case	iEdge1000Constants.SPAN_ID:
						return new Short(id);
					case	iEdge1000Constants.SPAN_COUNTRYCODE:
						return new Integer(countryCode);
					case	iEdge1000Constants.SPAN_DIDPREFIX:
						return didPrefix;
					case	iEdge1000Constants.SPAN_PORTS:
						return	ports;
					default:
						return null;
				}
			}
		}

		public class CasSpan extends Span{

			public CasSpan(short id, String aDID, int countryCode, TreeMap ports) {
				super(iEdge1000Constants.SPAN_CAS, id, aDID, countryCode, ports );
			}

		}


		public class PriSpan extends Span{

			short	mode;
			short	switchtype;
			short	q931Variant;
			short	loopbackMode;
			short	framingMode;
			short	lineCode;


			public PriSpan(short type, short id, String aDID, short mode, short switchtype, short q931Variant, short loopbackMode, short framingMode, short lineCode, int countryCode, TreeMap ports) {
				super(type,id, aDID, countryCode, ports);
				this.mode	=	mode;
				this.switchtype	=	switchtype;
				this.q931Variant	=	q931Variant;
				this.loopbackMode	=	loopbackMode;
				this.framingMode	=	framingMode;
				this.lineCode	=	lineCode;
			}

			public Object getData(short cmd){

				switch(cmd){
					case	iEdge1000Constants.PRISPAN_MODE:
						return new Short(mode);
					case	iEdge1000Constants.PRISPAN_SWITCH:
						return new Short(switchtype);
					case	iEdge1000Constants.PRISPAN_Q931VARIANT:
						return new Short(q931Variant);
					case	iEdge1000Constants.PRISPAN_LOOPBACKMODE:
						return new Short(loopbackMode);
					case	iEdge1000Constants.PRISPAN_FRAMINGMODE:
						return new Short(framingMode);
					case	iEdge1000Constants.PRISPAN_LINECODE:
						return new Short(lineCode);
					default:
						return (super.getData(cmd));
				}

			}

		}


		public class T1PriSpan extends PriSpan{

			short lineLength;

			public T1PriSpan(short type, short id, String aDID, short mode, short switchtype, short q931Variant, short lineLength, short loopbackMode, short framingMode, short lineCode, int countryCode, TreeMap ports) {
				super(type,id,aDID,mode,switchtype,q931Variant,loopbackMode,framingMode,lineCode,countryCode,ports);
				this.lineLength	=	lineLength;
			}

			public Object getData(short cmd){

				switch(cmd){
					case	iEdge1000Constants.T1PRISPAN_LINE_LENGTH:
						return new Short(lineLength);
					default:
						return(super.getData(cmd));
				}
			}

		}

		public class E1PriSpan extends PriSpan{

			short termMode;

			public E1PriSpan(short type, short id, String aDID, short mode, short switchtype, short q931Variant, short termMode, short loopbackMode, short framingMode, short lineCode, int countryCode, TreeMap ports) {
				super(type,id,aDID,mode,switchtype,q931Variant,loopbackMode,framingMode,lineCode,countryCode,ports);
				this.termMode=	termMode;
			}

			public Object getData(short cmd){

				switch(cmd){
					case	iEdge1000Constants.E1PRISPAN_TERM_MODE:
						return new Short(termMode);
					default:
						return(super.getData(cmd));
				}
			}

		}


		public class Port implements DataProvider,iEdge1000Constants{

			short	id;
			short	timeSlot;
			short	rollOverType;
			short	status;
			short	signallingMode;
			String	externalGKNumber;
			String	autoNumber;
			String	rollOverNumber;
			String	h323Id;
			String	email;



			public Port() {
				id			=	-1;
				timeSlot	=	iEdge1000Constants.INVALID;
				rollOverType=	0;
				status		=	0;
				signallingMode	=	0;
				externalGKNumber=	"";
				autoNumber		=	"";
				rollOverNumber	=	"";
				h323Id			=	"";
				email			=	"";

			}

			public Port(short	id,short timeSlot,short	rollOverType,short status,short	signallingMode,String externalGKNumber,String autoNumber,String	rollOverNumber,String h323Id,String	email) {
				this.id				=	id	;
				this.timeSlot		=	timeSlot;
				this.rollOverType	=	rollOverType;
				this.status			=	status;
				this.signallingMode	=	signallingMode;
				this.externalGKNumber=	(externalGKNumber == null)?"":externalGKNumber;
				this.autoNumber		=	(autoNumber == null)?"":autoNumber;
				this.rollOverNumber	=	(rollOverNumber == null)?"":rollOverNumber;
				this.h323Id			=	(h323Id == null)?"":h323Id;
				this.email			=	(email == null)?"":email;

			}

			public Object getData(short cmd){
				switch(cmd){
					case	iEdge1000Constants.PORT_ID:
						return	new Short(id);
					case	iEdge1000Constants.PORT_TIMESLOT:
						return	new	Short(timeSlot);
					case	iEdge1000Constants.PORT_ROLLOVERTYPE:
						return	new	Short(rollOverType);
					case	iEdge1000Constants.PORT_STATUS:
						return	new Short(status);
					case	iEdge1000Constants.PORT_SIGNALLINGMODE	:
						return	new	Short(signallingMode);
					case	iEdge1000Constants.PORT_EXTERNALGKNUMBER:
						return	externalGKNumber;
					case	iEdge1000Constants.PORT_AUTONUMBER		:
						return	autoNumber;
					case	iEdge1000Constants.PORT_ROLLOVERNUMBER	:
						return	rollOverNumber;
					case	iEdge1000Constants.PORT_ISROLLOVER:
							return	new Boolean(isRolloverNumber());
					case	iEdge1000Constants.PORT_H323ID:
						return	h323Id;
					case	iEdge1000Constants.PORT_EMAIL:
						return	email;
					default:
						return null;
				}

			}

			public boolean isRolloverNumber() {
				switch (signallingMode) {
					case iEdge1000Constants.SIGMODE_PRI_USER:
					case iEdge1000Constants.SIGMODE_FXS_GROUND_START:
					case iEdge1000Constants.SIGMODE_FXS_LOOP_START:
					return true;

				default:
					// All other modes.
					return false;
				}
			}
		}

	}
	
	/**
	 * class which stores dialing prefix configuration details
	 */

	public class Prefix implements DataConsumer, DataProvider,iEdge1000Constants{

		short  code;
		String vpnPrefix;
		String lusPrefix;


		Prefix(){
			code	=	-1;
			vpnPrefix	=	"";
			lusPrefix	=	"";

		}

		/**
		 * Get the prefix configuration details
		 */

		public Object getData(short cmd){

			switch(cmd){
				case	iEdge1000Constants.PREFIX_ID:
					return new Short(code);
				case	iEdge1000Constants.PREFIX_VPN:
					return vpnPrefix;
				case	iEdge1000Constants.PREFIX_LUS:
					return lusPrefix;
				default:
					return null;
			}
			
		}

		public boolean ReadFromDevice(ConfigRetriever cr)  throws IOException {

			boolean	bRet	=	false;
			try{
				bRet	=	cr.retrieve(Constants.DIAL_PREFIX, this);
			}
			catch(Exception ioe){
				throw new IOException(ioe.getMessage());
			}
			return bRet;
		}

		public void extractGetReply (LimitedDataInputStream dis) throws IOException {

			code = dis.readShort();
			if (code == Constants.DIAL_PREFIX) {
				//////////////////////////////////////
				// Read the XML DOM
				//////////////////////////////////////
				try {

					DocumentBuilderFactory dbf = DocumentBuilderFactory.newInstance();
			        DocumentBuilder db = dbf.newDocumentBuilder();
			        Document document = db.parse(dis);
					// Get the VPN (internal) dial prefix
					NodeList list = document.getElementsByTagName("VPN");
					if (list.getLength()>0) {
						Node node = list.item(0).getFirstChild();
						if (node != null) {
							vpnPrefix	=	node.getNodeValue();
						}
					}
					// Get the LUS (external) dial prefix
					list = document.getElementsByTagName("LUS");
					if (list.getLength()>0) {
						Node node = list.item(0).getFirstChild();
						if (node != null) {
							lusPrefix	=	node.getNodeValue();
						}
					}
				}catch (SAXParseException spe){
					throw new IOException("Prefix config get request error" + "\n" + spe.getMessage());
				}catch (SAXException sxe) {
					throw new IOException("Prefix config get request error" + "\n" + sxe.getMessage());
				}catch (ParserConfigurationException pce) {
					throw new IOException("Prefix config get request error" + "\n" + pce.getMessage());
				}catch (DOMException de){
					throw new IOException("Prefix config get request error" + "\n" + de.getMessage());
				} catch(Exception ioe) {
					// This will catch all parser and IO errors.
					throw new IOException("Prefix config get request error" + "\n" + ioe.getMessage());

				}

			}
		}//	Function end

	}

	/**
	 * class which stores software download configuration details
	 */


	public class Download implements DataConsumer, DataProvider,iEdge1000Constants{
		short	code;
		String	serverAddress;
		String  userName;
		String	password;
		String	directory;
		String	filename;


		Download(){
			code	=	-1;
			serverAddress	=	"";
			userName	=	"";
			password	=	"";
			directory	=	"";
			filename	=	"";
		}

		/**
		 * Get the software download configuration details
		 */

		public Object getData(short cmd){

			switch(cmd){
			case  iEdge1000Constants.DOWNLOAD_CODE	:
				return  new Short(code);
			case iEdge1000Constants.DOWNLOAD_ADDR	:
				return serverAddress;
			case iEdge1000Constants.DOWNLOAD_USER	:
				return userName;
			case iEdge1000Constants.DOWNLOAD_PASSWORD:
				return password;
			case iEdge1000Constants.DOWNLOAD_DIR	:
				return directory;
			case iEdge1000Constants.DOWNLOAD_FILE	:
				return filename;
			default:
				return null;
			}
			
		}
		public boolean ReadFromDevice(ConfigRetriever cr)  throws IOException {

			boolean	bRet	=	false;
			try{
				bRet	=	cr.retrieve(Constants.DOWNLOAD_CODE, this);
			}
			catch(Exception ioe){
				throw new IOException(ioe.getMessage());
			}
			return bRet;
		}

		public void extractGetReply (LimitedDataInputStream dis) throws IOException {

			code = dis.readShort();

			if (code == Constants.DOWNLOAD_CODE) {
				//////////////////////////////////////
				// Read the XML DOM
				//////////////////////////////////////
				try {
					DocumentBuilderFactory dbf = DocumentBuilderFactory.newInstance();
			        DocumentBuilder db = dbf.newDocumentBuilder();
			        Document document = db.parse(dis);

					// Get the iServer addresses
					NodeList list = document.getElementsByTagName("SN");
					if (list.getLength()>0) {
						Node node = list.item(0).getFirstChild();
						if (node != null)
							serverAddress	=	node.getNodeValue();
					}

					// Username
					list = document.getElementsByTagName("UN");
					if (list.getLength()>0) {
						Node node = list.item(0).getFirstChild();
						if (node != null) 
							userName	=	node.getNodeValue();
					}

					// Password
					list = document.getElementsByTagName("PD");
					if (list.getLength()>0) {
						Node node = list.item(0).getFirstChild();
						if (node != null)
							password	=	""+node.getNodeValue();
					}

					// Directory name
					list = document.getElementsByTagName("DD");
					if (list.getLength()>0) {
						Node node = list.item(0).getFirstChild();
						if (node != null)
							directory	=	""+node.getNodeValue();
					}

					// Filename
					list = document.getElementsByTagName("FN");
					if (list.getLength()>0) {
						Node node = list.item(0).getFirstChild();
						if (node != null)
							filename	=	""+node.getNodeValue();
					}
				}catch (SAXParseException spe){
					throw new IOException("Download config get request error" + "\n" + spe.getMessage());
				}catch (SAXException sxe) {
					throw new IOException("Download config get request error" + "\n" + sxe.getMessage());
				}catch (ParserConfigurationException pce) {
					throw new IOException("Download config get request error" + "\n" + pce.getMessage());
				}catch (DOMException de){
					throw new IOException("Download config get request error" + "\n" + de.getMessage());
				} catch(Exception ioe) {
					// This will catch all parser and IO errors.
					throw new IOException("Download config get request error" + "\n" + ioe.getMessage());

				}


			}
		}//	Function end

	}



	/**
	 * class which stores NAT configuration details
	 */


	public class NAT implements DataConsumer, DataProvider,iEdge1000Constants{
		short	code;
		boolean isNATEnabled;
        boolean isNATProxyEnabled;
		String  natIPAddr;
		String  natMask;
		NatMapData	natMap;
		Vector natData;


		NAT(){
			code	=	-1;
			isNATEnabled	=	false;
            isNATProxyEnabled   =   false;
			natIPAddr		=	"";
			natMask	=	"";
			natData			=	new Vector();
			natMap			=	new NatMapData();
		}

		/**
		 * Get the NAT configuration details
		 */

		public Object getData(short cmd){

			switch(cmd){
			case  iEdge1000Constants.NAT_CODE:
				return  new Short(code);

			case iEdge1000Constants.NAT_ISENABLED:
				return new Boolean(isNATEnabled);

			case iEdge1000Constants.NAT_IS_PROXYENABLED:
				return new Boolean(isNATProxyEnabled);

			case iEdge1000Constants.NAT_IPADDR:
				return natIPAddr;

			case iEdge1000Constants.NAT_MASK:
				return natMask;

			case iEdge1000Constants.NAT_DATA:
				return natData;

			case iEdge1000Constants.NAT_MAP:
				return natMap;

			default:
				return null;
			}
			
		}
		public boolean ReadFromDevice(ConfigRetriever cr)  throws IOException {

			boolean	bRet	=	false;
			try{
				bRet	=	cr.retrieve(Constants.NAT, this);
			}
			catch(Exception ioe){
				throw new IOException(ioe.getMessage());
			}
			return bRet;
		}

		public void extractGetReply (LimitedDataInputStream dis) throws IOException {


			short code = dis.readShort();
			if (code == Constants.NAT) {

     			try  {
					DocumentBuilderFactory dbf = DocumentBuilderFactory.newInstance();
					DocumentBuilder db = dbf.newDocumentBuilder();
					Document document = db.parse(dis);

					//	get NAT enabled flag
					NodeList list = document.getElementsByTagName("ipNatEnable");
					if (list != null && list.getLength() > 0) {
						Node node = list.item(0).getFirstChild();
						if (node != null ) {
							isNATEnabled	=	Boolean.valueOf(node.getNodeValue()).booleanValue();
						}
					}
                    //  get NAT proxy Enabled flag
					list = document.getElementsByTagName("ipProxyEnable");
					if (list != null && list.getLength() > 0) {
						Node node = list.item(0).getFirstChild();
						if (node != null ) {
							isNATProxyEnabled	=	Boolean.valueOf(node.getNodeValue()).booleanValue();
						}
					}

					//	get NAT ipaddresses and SubnetMask addresses
					list = document.getElementsByTagName("znb1");

					if (list != null && list.getLength() > 0) {
						Node node = list.item(0).getFirstChild();
						if (node	!=	null) {
							String natString = node.getNodeValue();
							int index = natString.indexOf('/');	
							natIPAddr = natString.substring(0,index);
							natMask	= natString.substring(index+1);
						}
					}

					list = document.getElementsByTagName("ipNat");

					if (list != null && list.getLength() > 0) {

						Node node = list.item(0).getFirstChild();
						if(node	!=	null){
							String data	=	node.getNodeValue();
							StringTokenizer ltk = new StringTokenizer(data,"\n");
							while(ltk.hasMoreTokens()){
								String	line	=	ltk.nextToken();

								if (line.indexOf("proxy") >= 0)
									continue;

								String token,key	=	"";
								String localIp	=	"";
								String localMask	=	"";
								String publicIp	=	"";
								String publicMask	=	"";
								int ePort	=	0;
								int iPort	=	0;
								String protocol	=	"";

								boolean	nat	=	true;

								StringTokenizer tk = new StringTokenizer(line);

								key	=	token	=	tk.nextToken();
								if (!token.equals("map") && !token.equals("rdr"))
									continue;

								token = tk.nextToken();
								if (!token.equals("znb0") && !token.equals("znb1")) 
									continue;

								token = tk.nextToken();
								int index   = token.indexOf("/");

								localIp = token.substring(0,index);
								localMask = token.substring(index+1);



								token	= tk.nextToken();

								if (key.equals("map")) {

									if (!token.equals("->")) 
										continue;

									token = tk.nextToken();
									index = token.indexOf("/");

									if(nat){
										publicIp = token.substring(0,index);
										publicMask = token.substring(index+1);
										nat		=	false;
										natMap	=	new NatMapData(localIp,localMask,publicIp,publicMask);
									}
								}
								else {
									// Nat proxy
									if (!token.equals("port")) 
										continue;
									// internal port is well known port
									iPort = Integer.parseInt(tk.nextToken());
									token = tk.nextToken();
									if (!token.equals("->")) 
										continue;

									publicIp = tk.nextToken();

									token = tk.nextToken();
									if (!token.equals("port")) 
										continue;

									// external port can be any port 
									ePort = Integer.parseInt(tk.nextToken());
									protocol = tk.nextToken();
									natData.add(new  NatProxyData(protocol,localIp,localMask,iPort,ePort,publicIp));
								}


							}
					
						}
					}

				}catch (SAXParseException spe){
					throw new IOException("NAT config get request error" + "\n" + spe.getMessage());
				}catch (SAXException sxe) {
					throw new IOException("NAT config get request error" + "\n" + sxe.getMessage());
				}catch (ParserConfigurationException pce) {
					throw new IOException("NAT config get request error" + "\n" + pce.getMessage());
				}catch (DOMException de){
					throw new IOException("NAT config get request error" + "\n" + de.getMessage());
				} catch(Exception ioe) {
					// This will catch all parser and IO errors.
					throw new IOException("NAT config get request error" + "\n" + ioe.getMessage());

				}
			}		


		}
	}

	public class NatMapData implements DataProvider,iEdge1000Constants{
		String	localIp;
		String	localMask;
		String	publicIp;
		String	publicMask;

		public NatMapData(){
			localIp		=	"";
			localMask	=	"";
			publicIp	=	"";
			publicMask	=	"";
		}
		public NatMapData(String localIp,String localMask,String publicIp,String publicMask){
			this.localIp	= (localIp	== null)?"":localIp	;
			this.localMask= (localMask== null)?"":localMask;
			this.publicIp= (publicIp== null)?"":publicIp;
			this.publicMask= (publicMask== null)?"":publicMask;
		}

		public Object getData(short cmd){

			switch(cmd){
				case iEdge1000Constants.NATMAP_LOCALIP:
					return localIp;

				case iEdge1000Constants.NATMAP_LOCALMASK:
					return localMask;

				case iEdge1000Constants.NATMAP_PUBLICIP:
					return publicIp;

				case iEdge1000Constants.NATMAP_PUBLICMASK:
					return publicMask;
				default:
					return null;

			}
		}

	}

	public class NatProxyData implements DataProvider,iEdge1000Constants{
		String protocol;
		String localIp;
		String localMask;
		String publicIp;
		int externalPort;
		int internalPort;

		public NatProxyData(){
			protocol	=	"";
			localIp		=	"";
			localMask	=	"";
			publicIp	=	"";
			externalPort=	0;
			internalPort=	0;
		}


		public NatProxyData(String protocol,String localIp,String localMask, int internalPort,int externalPort,String publicIp){
			this.protocol	=	(protocol	==	null)?"":protocol;
			this.localIp	=	(localIp	==	null)?"":localIp;
			this.localMask	=	(localMask	==	null)?"":localMask;
			this.publicIp	=	(publicIp	==	null)?"":publicIp;
			this.externalPort	=	externalPort;
			this.internalPort	=	internalPort;
		}

		public Object getData(short cmd){

			switch(cmd){
				case iEdge1000Constants.NATPROXY_PROTOCOL:
					return protocol;
				case iEdge1000Constants.NATPROXY_LOCALIP:
					return localIp;
				case iEdge1000Constants.NATPROXY_LOCALMASK:
					return localMask;
				case iEdge1000Constants.NATPROXY_PUBLICIP:
					return publicIp;
				case iEdge1000Constants.NATPROXY_EXTERNALPORT:
					return new Integer(externalPort);
				case iEdge1000Constants.NATPROXY_INTERNALPORT:
					return new Integer(internalPort);
				default:
					return null;
			}
		}

	}



	public class IPFilter implements DataConsumer, DataProvider,iEdge1000Constants{
		short	code;
		boolean	isIPFilterEnabled;
		Vector	ipDataList;

		IPFilter(){
			code				=	-1;
			isIPFilterEnabled	=	false;
			ipDataList			=	new Vector();
		}


		/**
		 * Get the IPFilter configuration details
		 */

		public Object getData(short cmd){

			switch(cmd){
				case  iEdge1000Constants.IPFILTER_CODE:
					return  new Short(code);

				case  iEdge1000Constants.IPFILTER_ISENABLED:
					return  new Boolean(isIPFilterEnabled);

				case  iEdge1000Constants.IPFILTER_DATA:{
					return  ipDataList;
													   }
				default:
					return null;
			}
			
		}
		public boolean ReadFromDevice(ConfigRetriever cr)  throws IOException {

			boolean	bRet	=	false;
			try{
				bRet	=	cr.retrieve(Constants.IP_FILTER, this);
			}
			catch(Exception ioe){
				throw new IOException(ioe.getMessage());
			}
			return bRet;
		}

		private boolean splitIP(String iIP, StringBuffer ip, StringBuffer maskin) {
		String emptyString = new String("");
		if (iIP == null || iIP.length() == 0)
			return false;
		int i = iIP.indexOf('/');
		if (i==-1) {
			if (iIP.equals("any")) {
				ip.append(iIP);
				return true;
			}
		        if (IPUtil.ipStringToIntArray(iIP) == null) 
				return false;
		}
		
		String ipv;
		String maskStr;
		if (i == -1) {
		   ipv = iIP;
		   maskStr = emptyString;
		}  else {
		   ipv = iIP.substring(0,i);
		   maskStr = iIP.substring(i+1);
		}
			
		if (IPUtil.ipStringToIntArray(ipv) == null) 
		   return false;
		else 
		   ip.append(ipv);
		if (maskStr.length() == 0) {
			return true;
		}
		if (maskStr.indexOf('.') != -1) {
			maskin.append(maskStr);
			return true;
		} else if (maskStr.length() > 2 && maskStr.charAt(0) == '0' &&
			maskStr.charAt(1) == 'x') {
			long lm;
			try {
				lm = Long.parseLong(maskStr.substring(2),16);
			} catch (NumberFormatException nfe) {
				return false;
			}
		        maskin.append(IPUtil.intToIPString((int)lm));
			return true;
		} else {
		  try {
		   int masknum = Integer.parseInt(maskStr);
		   if (masknum <0 || masknum > 32) 
			return false;
		   int mask  = 0xffffffff;
		   int s = 32-masknum; 	
		   mask = mask >> s;
		   mask = mask << s;
		   maskin.append(IPUtil.intToIPString(mask));
		   return true;	
		  } catch (NumberFormatException ne) {
			return false;
		  }
		}
	  }

		public void extractGetReply (LimitedDataInputStream dis) throws IOException {


			short code = dis.readShort();
			if (code == Constants.IP_FILTER) {
      			try  {
					DocumentBuilderFactory dbf = DocumentBuilderFactory.newInstance();
					DocumentBuilder db = dbf.newDocumentBuilder();
					Document document = db.parse(dis);

					//	get IP filter enabled flag
					NodeList list = document.getElementsByTagName("ipFilterEnable");
					if (list != null && list.getLength() > 0) {
						Node node = list.item(0).getFirstChild();
						if (node != null ) {
							isIPFilterEnabled	=	Boolean.valueOf(node.getNodeValue()).booleanValue();
						}
					}



					list = document.getElementsByTagName("ipfilter");

					if (list != null && list.getLength() > 0) {

						Node node = list.item(0).getFirstChild();
						if(node	!=	null){
							String data	=	node.getNodeValue();
							StringTokenizer ltk = new StringTokenizer(data,"\n");
							if(ipDataList	==	null)
								ipDataList	=	new Vector();
							while(ltk.hasMoreTokens()){

								String	token		=	"";
								String	errString	=	"";
								String	rule		=	"";
								String	direction	=	"";
								String	log			=	"";
								String	quick		=	"";
								String	onInterface	=	"";
								String	protocol	=	"";
								String	sourceIp	=	"";
								String	sourceMask	=	"";
								String	destIp		=	"";
								String	destMask	=	"";
								String	sourceRange	=	"";
								String	destRange	=	"";
								String	sourceOp	=	"";
								String	destOp		=	"";
								String	ipOptions	=	"";
								String	keep		=	"";
								String	groupOpt	=	"";
								String	groupNum	=	"";

								StringBuffer tempIP;
								StringBuffer tempMask;
								int	sourcePort		=	-1;
								int	sourcePortEnd	=	-1;
								int	destPort		=	-1;
								int	destPortEnd		=	-1;

								String	line	=	ltk.nextToken();
								StringTokenizer tk = new StringTokenizer(line);


								while(tk.hasMoreTokens()){

									//Get rule
									token	=	tk.nextToken();
									if (token.equals("block") || token.equals("pass")) 
										rule	=	token;
									//Get direction
									else if (token.equals("in") || token.equals("out")) 
										direction	=	token;
									//Get log
									else if (token.equals("log")) 
										log = token ;
									//Get quick
									else if (token.equals("quick")) 
										quick = token;
									//Get oninterface
									else if (token.equals("on")) 
										onInterface = tk.nextToken();
									//	Get protocol
									else if (token.equals("proto")) 
										protocol = tk.nextToken();
									//	Get sourceip/mask
									else if (token.equals("from")) {
										token = tk.nextToken();
										if(token.equals("any")){
											sourceIp	=	token;
										}else{
											/*
											int index	=	token.indexOf('/');
											sourceIp	=	token.substring(0,index);
											sourceMask	=	token.substring(index+1);
											*/
											tempIP = new StringBuffer("");
											tempMask = new StringBuffer("");
											splitIP(token,tempIP,tempMask);
											sourceIp = tempIP.toString();
											sourceMask = tempMask.toString();
										}
									}

									//	Get source port
									else if (token.equals("port")) {
										sourceOp	=	tk.nextToken();
										if(sourceOp.equals("=")){
											token = tk.nextToken();
											if(token	!=	null)
												sourcePort	=	Integer.parseInt(token);
										}else{
											sourcePort	=	Integer.parseInt(sourceOp);
											sourceRange =	tk.nextToken();
											if(sourceRange.equals("in_range") || sourceRange.equals("out_range")){
												token = tk.nextToken();
												if(token	!=	null)
													sourcePortEnd	=	Integer.parseInt(token);
											}
										}

									}
									//	Get destination ip/mask
									else if (token.equals("to")) {
										token = tk.nextToken();
										if(token.equals("any")){
											destIp	=	token;
										}else{
											/*
											int index	=	token.indexOf('/');
											destIp	=	token.substring(0,index);
											destMask	=	token.substring(index+1);
											*/
											tempIP = new StringBuffer("");
											tempMask = new StringBuffer("");
											splitIP(token,tempIP,tempMask);
											destIp = tempIP.toString();
											destMask = tempMask.toString();
										}
									}

									//	Get destination port
									else if (token.equals("port")) {
										token = tk.nextToken();
										destOp		=	token;
										if(token.equals("=")){
											token = tk.nextToken();
											if(token	!=	null)
												destPort	=	Integer.parseInt(token);
										}else{
											destPort	=	Integer.parseInt(destOp);
											destRange =	tk.nextToken();

											if(destRange.equals("in_range") || destRange.equals("out_range")){
												token = tk.nextToken();
												if(token	!=	null)
													destPortEnd	=	Integer.parseInt(token);
											}
										}
									}
									else if (token.equals("with")) {
										token = tk.nextToken();
										if (token.equals("short") || token.equals("ipopt") ||
											token.equals("ipopts") ) {
											if (token.equals("ipopts"))
												token	=	"ipopt";
											ipOptions = token;
										}
									}
									else if (token.equals("keep")) {
										token = tk.nextToken();
										if (token.equals("state") || token.equals("frags")) 
											keep	=	token;
									}
									else if (token.equals("head")||token.equals("group")) {
										groupOpt	=	token;
										groupNum	=	tk.nextToken();
									}

								}

								ipDataList.add(new IPFilterData(rule,direction,log,quick,onInterface,protocol,sourceIp,sourceMask,
											destIp,destMask,ipOptions,keep,groupOpt,groupNum,sourceRange, sourceOp, destRange,destOp,sourcePort,sourcePortEnd,destPort,destPortEnd));


							} //End while
						}//End if
					}//End If


				}catch (SAXParseException spe){
					throw new IOException("IP Filter config get request error" + "\n" + spe.getMessage());
				}catch (SAXException sxe) {
					throw new IOException("IP Filter config get request error" + "\n" + sxe.getMessage());
				}catch (ParserConfigurationException pce) {
					throw new IOException("IP Filter config get request error" + "\n" + pce.getMessage());
				}catch (DOMException de){
					throw new IOException("IP Filter config get request error" + "\n" + de.getMessage());
				} catch(Exception ioe) {
					// This will catch all parser and IO errors.
					throw new IOException("IP Filter config get request error" + "\n" + ioe.getMessage());

				}


			}// End If
		}
	}

	class IPFilterData implements DataProvider,iEdge1000Constants{

		private String	rule;		
		private String	direction;	
		private String	log	;		
		private String	quick;		
		private String	onInterface;	
		private String	protocol;	
		private String	sourceIp;	
		private String	sourceMask;	
		private String	destIp;		
		private String	destMask;	
		private String	ipOptions;	
		private String	keep;		
		private String	groupOpt;	
		private String	groupNum;	
		private String	sourceRange;	
		private String	sourceOp;	
		private String	destRange;	
		private String	destOp;	
		private int	sourcePort;		
		private int	sourcePortEnd;	
		private int	destPort;		
		private int	destPortEnd;		

		public IPFilterData(){
			rule		=	"";
			direction	=	"";
			log			=	"";
			quick		=	"";
			onInterface	=	"";
			protocol	=	"";
			sourceIp	=	"";
			sourceMask	=	"";
			destIp		=	"";
			destMask	=	"";
			ipOptions	=	"";
			keep		=	"";
			groupOpt	=	"";
			groupNum	=	"";
			sourceRange	=	"";
			sourceOp	=	"";
			destRange	=	"";
			destOp		=	"";
			sourcePort		=	0;
			sourcePortEnd	=	0;
			destPort		=	0;
			destPortEnd		=	0;

		}

		public IPFilterData(String rule,String direction,	String log,	String quick,String onInterface,String protocol,
							String	sourceIp,String	sourceMask,	String	destIp,String destMask,String ipOptions,String	keep,String	groupOpt,String	groupNum,
							String	sourceRange, String	sourceOp, String destRange,String destOp,int sourcePort,int	sourcePortEnd,int destPort,int destPortEnd){

			this.rule		=	(rule	==	null)?"":rule;
			this.direction	=	(direction	==	null)?"":direction;
			this.log		=	(log	==	null)?"":log;
			this.quick		=	(quick	==	null)?"":quick;
			this.onInterface=	(onInterface	==	null)?"":onInterface	;
			this.protocol	=	(protocol	==	null)?"":protocol;
			this.sourceIp	=	(sourceIp	==	null)?"":sourceIp;
			this.sourceMask	=	(sourceMask	==	null)?"":sourceMask;
			this.destIp		=	(destIp	==	null)?"":destIp;
			this.destMask	=	(destMask	==	null)?"":destMask;
			this.ipOptions	=	(ipOptions	==	null)?"":ipOptions;
			this.keep		=	(keep	==	null)?"":keep;
			this.groupOpt	=	(groupOpt	==	null)?"":groupOpt;
			this.groupNum	=	(groupNum	==	null)?"":groupNum;
			this.sourceRange	=	(sourceRange	==	null)?"":sourceRange;
			this.sourceOp	=	(sourceOp	==	null)?"":sourceOp;
			this.destRange	=	(destRange	==	null)?"":destRange;
			this.destOp		=	(destOp	==	null)?"":destOp;
			this.sourcePort	=	sourcePort;
			this.sourcePortEnd=	sourcePortEnd;
			this.destPort	=	destPort;
			this.destPortEnd=	destPortEnd;
		}

		public Object getData(short cmd){
			switch(cmd){
				case	iEdge1000Constants.IPFILTER_RULE:
					return rule;
				case	iEdge1000Constants.IPFILTER_DIRECTION:
					return direction;
				case	iEdge1000Constants.IPFILTER_LOG:
					return log;
				case	iEdge1000Constants.IPFILTER_QUICK:
					return quick;
				case	iEdge1000Constants.IPFILTER_ONINTERFACE	:
					return onInterface	;
				case	iEdge1000Constants.IPFILTER_PROTOCOL:
					return protocol;
				case	iEdge1000Constants.IPFILTER_SOURCEIP:
					return sourceIp;
				case	iEdge1000Constants.IPFILTER_SOURCEMASK:
					return sourceMask;
				case	iEdge1000Constants.IPFILTER_DESTIP:
					return destIp;
				case	iEdge1000Constants.IPFILTER_DESTMASK:
					return destMask;
				case	iEdge1000Constants.IPFILTER_IPOPTIONS:
					return ipOptions;
				case	iEdge1000Constants.IPFILTER_KEEP:
					return keep;
				case	iEdge1000Constants.IPFILTER_GROUPOPT:
					return groupOpt;
				case	iEdge1000Constants.IPFILTER_GROUPNUM:
					return groupNum;
				case	iEdge1000Constants.IPFILTER_SOURCERANGE:
					return sourceRange;
				case	iEdge1000Constants.IPFILTER_SOURCEOP:
					return sourceOp;
				case	iEdge1000Constants.IPFILTER_DESTRANGE:
					return destRange;
				case	iEdge1000Constants.IPFILTER_DESTOP:
					return destOp;
				case	iEdge1000Constants.IPFILTER_SOURCEPORT:
					return new Integer( sourcePort);
				case	iEdge1000Constants.IPFILTER_SOURCEPORTEND:
					return new Integer( sourcePortEnd);
				case	iEdge1000Constants.IPFILTER_DESTPORT:
					return new Integer( destPort);
				case	iEdge1000Constants.IPFILTER_DESTPORTEND:
					return new Integer( destPortEnd);
				default:
					return null;
			}

		}

	}

	/*
	 * Get DhcpServer data
	 */


	public class DhcpServer implements DataConsumer, DataProvider,iEdge1000Constants{
		short	code;
		boolean	isDhcpEnabled;
		String	domainName;
		String	znb0;
		String	znb1;
		String	znb0Mask;
		String	znb1Mask;
		String  leaseNeg;
		String  timeServ;
		String	utcoffst;

		Vector	nameServers;
		Vector	networks;
		Vector	netMasks;
		HashMap	subnets;


		DhcpServer(){
			code	=	-1;
			isDhcpEnabled	=	false;		
			domainName	=	"";		
			znb0	=	"";		
			znb1	=	"";		
			znb0Mask	=	"";		
			znb1Mask	=	"";		
			nameServers	=	new Vector();		
			networks	=	new Vector();		
			netMasks	=	new Vector();		
			subnets		=	new HashMap();		
		}


		/**
		 * Get the IPFilter configuration details
		 */

		public Object getData(short cmd){

			switch(cmd){
				case	iEdge1000Constants.DHCPSERVER_CODE:
					return new Integer(code);
				case	iEdge1000Constants.DHCPSERVER_ISDHCPENABLED:
					return new Boolean(isDhcpEnabled);
				case	iEdge1000Constants.DHCPSERVER_DOMAINNAME:
					return domainName;
				case	iEdge1000Constants.DHCPSERVER_ZNB0:
					return znb0;
				case	iEdge1000Constants.DHCPSERVER_ZNB1:
					return znb1;
				case	iEdge1000Constants.DHCPSERVER_ZNB0MASK:
					return znb0Mask;
				case	iEdge1000Constants.DHCPSERVER_ZNB1MASK:
					return znb1Mask;
				case	iEdge1000Constants.DHCPSERVER_NAMESERVERS:
					return nameServers;
				case	iEdge1000Constants.DHCPSERVER_NETWORKS:
					return networks;
				case	iEdge1000Constants.DHCPSERVER_NETMASKS:
					return netMasks;
				case	iEdge1000Constants.DHCPSERVER_SUBNETS:
					return subnets;
				default:
					return null;
			}
			
		}
		public boolean ReadFromDevice(ConfigRetriever cr)  throws IOException {

			boolean	bRet	=	false;
			try{
				bRet	=	cr.retrieve(Constants.DHCP_SERVER, this);
			}
			catch(Exception ioe){
				throw new IOException(ioe.getMessage());
			}
			return bRet;
		}

		public void extractGetReply (LimitedDataInputStream dis) throws IOException {


			short code = dis.readShort();
			if (code == Constants.DHCP_SERVER) {
      			try  {
					String temp;
					DocumentBuilderFactory dbf = DocumentBuilderFactory.newInstance();
					DocumentBuilder db = dbf.newDocumentBuilder();
					Document document = db.parse(dis);

					//	get Dhcp server enabled flag
					NodeList list = document.getElementsByTagName("enable");
					if (list != null && list.getLength() > 0) {
						Node node = list.item(0).getFirstChild();
						if (node != null ) {
							isDhcpEnabled	=	Boolean.valueOf(node.getNodeValue()).booleanValue();
						}
					}
					//get znb0 znb1 
			      	list	= document.getElementsByTagName("znb0");
					if(list != null && list.getLength() > 0) {
						Node node = list.item(0).getFirstChild();

						if (node != null ) {
							temp		=	node.getNodeValue();
							int index	=	temp.indexOf('/');
							znb0		=	temp.substring(0,index);
							znb0Mask	=	temp.substring(index+1);
						}
					}

			      	list	= document.getElementsByTagName("znb1");
					if(list != null && list.getLength() > 0) {
						Node node = list.item(0).getFirstChild();

						if (node != null ) {
							temp		=	node.getNodeValue();
							int index	=	temp.indexOf('/');
							znb1		=	temp.substring(0,index);
							znb1Mask	=	temp.substring(index+1);
						}
					}
					//	get networks/netmasks
			      	list	= document.getElementsByTagName("networks");
					if(list != null && list.getLength() > 0) {
						Node node = list.item(0).getFirstChild();

						if (node != null ) {
							temp	=	node.getNodeValue();
							StringTokenizer tk = new StringTokenizer(temp,"\n");
							if(netMasks	==	null)
								netMasks	=	new Vector();
							if(networks	==	null)
								networks	=	new Vector();

							while (tk.hasMoreTokens()) {
								StringTokenizer ntk = new StringTokenizer(tk.nextToken());
								networks.add(ntk.nextToken());
								netMasks.add(ntk.nextToken());
							}

						}
					}
					//	get subnets/domain
					list = document.getElementsByTagName("dhcptab");
					if (list != null && list.getLength() > 0)  { 
						Node node = list.item(0).getFirstChild();
						String data	=	node.getNodeValue();

						if(subnets	==	null)
							subnets	=	new HashMap();
						StringTokenizer ltk = new StringTokenizer(data,"\n");
						while(ltk.hasMoreTokens()){

							String	line	=	ltk.nextToken();
							StringTokenizer tk = new StringTokenizer(line, ":");

							String	subnet	=	"";
							String	subnetMask	=	"";
							String	broadCast	=	"";
							String	mtu	=	"";
							int		leaseTime	=	0;
							String	router	=	"";
							String	token	=	tk.nextToken();
							String	str1;
							String	str2;
							int	index;

							if (token.equals("#"))
								continue;
							//	get	subsets
							if(token.indexOf("domain")	==	-1){
								index	=	token.indexOf("m");
								subnet	=	token.substring(0,index).trim();

								while(tk.hasMoreTokens()){
									token	=	tk.nextToken();
									if(token.equals("LeaseNeg"))
										continue;

									if( (index	=	token.indexOf("=")) ==  -1)
                                        continue;
									str1	=	token.substring(0,index);
									str2	=	token.substring(index+1);

									if(str1.equals("Broadcst"))
										broadCast	=	str2;
									if(str1.equals("Subnet"))
										subnetMask	=	str2;
									if(str1.equals("MTU"))
										mtu	=	str2;
									if(str1.equals("LeaseTim"))
										leaseTime =	Integer.parseInt(str2);
									if(str1.equals("Router"))
										router	=	str2;
								}
								subnets.put(subnet, new DhcpSubNet(broadCast,subnetMask,mtu,leaseTime,router));
							}else{	//get domain related values

								while(tk.hasMoreTokens()){
									token	=	tk.nextToken();
									if(token.equals("LeaseNeg"))
										continue;
									if((index	=	token.indexOf("=")) ==  -1)
                                        continue;
									str1	=	token.substring(0,index);
									str2	=	token.substring(index+1);
									if(str1.equals("UTCoffst"))
										utcoffst	=	str2;
									
									if(str1.equals("Timeserv"))
										timeServ	=	str2;
									if(str1.equals("LeaseNeg"))
										leaseNeg	=	str2;
									if(str1.equals("DNSdmain"))
										domainName	=	str2;
									if(str1.equals("DNSserv")){
										StringTokenizer nametk = new StringTokenizer(str2," ");
										while(nametk.hasMoreTokens()){
											token	=	nametk.nextToken();
											nameServers.add(token);
										}

									}

								}

							}//End else

						}// End while
						
					}// End if
					// get dhcp range properties
					list = document.getElementsByTagName("dhcp_net");
					if (list != null && list.getLength() > 0)  { 

						for(int i =0 ; i < list.getLength(); i++){
							Node node	=	list.item(i);

							NamedNodeMap map = node.getAttributes();
							Node subNode	 = map.getNamedItem("id"); 
							String subnetIp	=	subNode.getNodeValue();

							subNode	=	node.getFirstChild();
							if(subNode	!=	null){
								String data	=	subNode.getNodeValue();

								StringTokenizer ltk = new StringTokenizer(data,"\n");
								String	clientId;
								String	flag;
								String	clientAddr;
								String	serverAddr;
								int		leaseTime;
								DhcpSubNet	dhcpSubNet	=	(DhcpSubNet)subnets.get(subnetIp);
								Vector	subAddresses	=	new Vector();
								while (ltk.hasMoreTokens()) {
									String line	=	ltk.nextToken();
									StringTokenizer tk = new StringTokenizer(line.trim());

									clientId	=	tk.nextToken();
									flag		=	tk.nextToken();
									clientAddr	=	tk.nextToken();
									serverAddr	=	tk.nextToken();
									leaseTime	=	Integer.parseInt(tk.nextToken());
									subAddresses.add(new SubNet(clientId,flag,clientAddr,serverAddr,leaseTime));
								}

								if(subAddresses.size() >0){
									SubNet	[]tempArr	=	new SubNet[subAddresses.size()];
									SubNet	[]arrSubNets	=	(SubNet[])subAddresses.toArray(tempArr);

									Arrays.sort(arrSubNets);

									long startAddr	=	arrSubNets[0].getClientIpinLong();;
									long nextAddr	=	startAddr;	
									int startIndex	=	0;
									int	j	=	0;
									boolean startflag	=	true;
									String startServerAddr	=	arrSubNets[0].getServerAddr();
									String nextServerAddr	=	startServerAddr;

									for (j=0; j<arrSubNets.length; j++) {
										nextAddr			=	arrSubNets[j].getClientIpinLong();
										nextServerAddr	=	arrSubNets[j].getServerAddr();

										if(( (nextAddr -  startAddr) >1) ||  !startServerAddr.equals(nextServerAddr)){
											dhcpSubNet.addRange(new DhcpRange(startServerAddr,arrSubNets[startIndex].getClientAddr(),arrSubNets[j-1].getClientAddr()));
											startAddr	=	arrSubNets[j].getClientIpinLong();;
											startServerAddr	=	arrSubNets[j].getServerAddr();
											startIndex	=	j;
										}
										else{
											startAddr	=	nextAddr;
											
										}
									}

									dhcpSubNet.addRange(new DhcpRange(nextServerAddr,arrSubNets[startIndex].getClientAddr(),arrSubNets[j-1].getClientAddr()));
								}
							}
						}
					}

				}catch (SAXParseException spe){
					throw new IOException("Dhcp Server config get request error" + "\n" + spe.getMessage());
				}catch (SAXException sxe) {
					throw new IOException("Dhcp Server config get request error" + "\n" + sxe.getMessage());
				}catch (ParserConfigurationException pce) {
					throw new IOException("Dhcp Server config get request error" + "\n" + pce.getMessage());
				}catch (DOMException de){
					throw new IOException("Dhcp Server config get request error" + "\n" + de.getMessage());
				} catch(Exception ioe) {
					// This will catch all parser and IO errors.
					throw new IOException("Dhcp Server config get request error" + "\n" + ioe.getMessage());

				}

			}
		}

	}

	public class DhcpSubNet implements DataProvider,iEdge1000Constants{
		String	subnetMask;
		String	broadCast;
		String	mtu;
		int		leaseTime;
		String	router;
		Vector	dhcpRanges;


		DhcpSubNet(){
			subnetMask	=	"";
			broadCast	=	"";
			mtu			=	"";
			leaseTime	=	-1;
			router		=	"";
			dhcpRanges	=	new Vector();
		}

		public DhcpSubNet(String broadCast,String	subnetMask,String mtu,int leaseTime,String	router){
			this.subnetMask	=	(subnetMask == null)?"":subnetMask;
			this.broadCast	=	(broadCast == null)?"":broadCast;
			this.mtu		=	(mtu		== null)?"":mtu;
			this.leaseTime	=	leaseTime;
			this.router		=	(router	 == null)?"":router	;
			dhcpRanges	=	new Vector();
		}

		public void addRange(DhcpRange ob){
			if(dhcpRanges	==	null)
				dhcpRanges	=	new Vector();
			dhcpRanges.add(ob);

		}
		/**
		 * Get the Dhcp subnet configuration details
		 */

		public Object getData(short cmd){

			switch(cmd){

				case	iEdge1000Constants.DHCPSUBNET_SUBNETMASK:
					return subnetMask;
				case	iEdge1000Constants.DHCPSUBNET_BROADCAST:
					return broadCast;
				case	iEdge1000Constants.DHCPSUBNET_MTU:
					return mtu;
				case	iEdge1000Constants.DHCPSUBNET_LEASETIME:
					return new Integer(leaseTime);
				case	iEdge1000Constants.DHCPSUBNET_ROUTER:
					return router;
				case	iEdge1000Constants.DHCPSUBNET_DHCPRANGES:
					return dhcpRanges;

				default:
					return null;
			}
		}			
	}

	public class DhcpRange implements DataProvider,iEdge1000Constants{
		String	serverAddr;
		String	startAddr;
		String	endAddr;

		public DhcpRange(){
			serverAddr	=	"";
			startAddr	=	"";
			endAddr		=	"";
		}

		public DhcpRange(String serverAddr, String startAddr,String endAddr	){
			this.serverAddr	=	(serverAddr == null)?"":serverAddr;
			this.startAddr	=	(startAddr == null)?"":startAddr;
			this.endAddr	=	(endAddr	 == null)?"":endAddr;
		}

		public Object getData(short cmd){

			switch(cmd){
				case	iEdge1000Constants.DHCPRANGE_SERVERADDR:
					return serverAddr;
				case	iEdge1000Constants.DHCPRANGE_STARTADDR:
					return startAddr;
				case	iEdge1000Constants.DHCPRANGE_ENDADDR:
					return endAddr;

				default:
					return null;
			}
		}

	}

	public class SubNet implements DataProvider,iEdge1000Constants,Comparable{
		String	clientId;
		String	flag;
		String	clientAddr;
		String	serverAddr;
		int		leaseTime;


		public SubNet(){
			clientId	=	"";
			flag		=	"";
			clientAddr	=	"";
			serverAddr	=	"";
			leaseTime	=	0;
		}

		public SubNet(String clientId,String flag,String	clientAddr,String serverAddr,int leaseTime){
			this.clientId	=	(clientId == null)?"":clientId;
			this.flag		=	(flag	 == null)?"":flag	;
			this.clientAddr	=	(clientAddr == null)?"":clientAddr;
			this.serverAddr	=	(serverAddr == null)?"":serverAddr;
			this.leaseTime	=	leaseTime;
		}


		public Object getData(short cmd){

			switch(cmd){
				case	iEdge1000Constants.SUBNET_CLIENTID:
					return clientId;
				case	iEdge1000Constants.SUBNET_FLAG:
					return flag;
				case	iEdge1000Constants.SUBNET_CLIENTADDR:
					return clientAddr;
				case	iEdge1000Constants.SUBNET_SERVERADDR:
					return serverAddr;
				case	iEdge1000Constants.SUBNET_LEASETIME:
					return new Integer(leaseTime);
				default:
					return null;
			}
		}	
		public long getClientIpinLong(){
			return IPUtil.ipStringToLong(clientAddr);
		}

		public String getClientAddr(){
			return clientAddr;
		}

		public String getServerAddr(){
			return serverAddr;
		}


		public int compareTo(Object o) {
			SubNet ob = (SubNet) o;
			long l1	=	getClientIpinLong();
			long l2	=	ob.getClientIpinLong();
			return ((int)(l1-l2));
		}


	}

	/**
	 * class which stores Sip configuration details
	 */
	public class SIP implements DataConsumer, DataProvider,iEdge1000Constants{

		short	code;
		boolean	registration;
		int		proxyServerPort;
        String  outgoingProtocol;
		String	regUri;
		String	protocol;
		String	serverAddress;
		String	proxyServerAddress;
		String	domain;
		boolean alwaysRouteToProxy;

		/**
		 * Creates an empty sip object
		 */
		SIP(){
			code	=	-1;
			registration	=	false;
			proxyServerPort	=	-1;
            outgoingProtocol    =   "";
			regUri			=	"";
			protocol		=	"";
			serverAddress	=	"";
			proxyServerAddress	=	"";
			domain	=	"";
			alwaysRouteToProxy = false;
		}

		/**
		 * Get the SIP configuration details
		 */
		public Object getData(short cmd){

			switch(cmd){

				case	iEdge1000Constants.SIP_OUT_PROTOCOL:
					return outgoingProtocol;
				case	iEdge1000Constants.SIP_REGISTRATION:
					return new Boolean(registration);
				case	iEdge1000Constants.SIP_PROXYSERVERPORT:
					return new Integer(proxyServerPort);
				case	iEdge1000Constants.SIP_REGURI:
					return regUri;
				case	iEdge1000Constants.SIP_PROTOCOL:
					return protocol;
				case	iEdge1000Constants.SIP_SERVERADDRESS:
					return serverAddress;
				case	iEdge1000Constants.SIP_PROXYSERVERADDRESS:
					return proxyServerAddress;
				case	iEdge1000Constants.SIP_DOMAIN:
					return domain;
				case    iEdge1000Constants.SIP_ALWAYSROUTETOPROXY:
                    return new Boolean(alwaysRouteToProxy);

				default:
						return null;
			}
		}

		/**
		 * Reads the sip configuration details from the 1000 device and stores them
		 */
		public boolean ReadFromDevice(ConfigRetriever cr)  throws IOException {

			boolean	bRet	=	false;
			try{
				bRet	=	cr.retrieve(Constants.SIP, this);
			}
			catch(Exception ioe){
				throw new IOException(ioe.getMessage());
			}
			return bRet;
		}
		/**
		 * Reads the sip configuration details from the 1000 device and stores them
		 */
		public void extractGetReply (LimitedDataInputStream dis) throws IOException {

			code	=	dis.readShort();

			if (code == Constants.SIP) {

				try {

					DocumentBuilderFactory dbf = DocumentBuilderFactory.newInstance();
			        DocumentBuilder db = dbf.newDocumentBuilder();
			        Document document = db.parse(dis);
	
					NodeList list = document.getElementsByTagName("sip");
					Node node;
					if (list.getLength()>0) {
						node = list.item(0);
						NamedNodeMap map = node.getAttributes();
						Node status = map.getNamedItem("status");
                        outgoingProtocol    =   status.getNodeValue();


						list = node.getChildNodes();

						NodeList childList;
						for(int i=0; i < list.getLength(); i++){
							node		=	list.item(i);
							String name	=	node.getNodeName();
							if (name.equals("sipRegUri")) {
								childList = node.getChildNodes();
								if (childList != null && childList.getLength() > 0) 
								   regUri = childList.item(0).getNodeValue();
							} else if (name.equals("sipProto")) {
								childList = node.getChildNodes();
								if (childList != null && childList.getLength() > 0)
								   protocol = childList.item(0).getNodeValue();
							} else if (name.equals("sipServerAddr")) {
								childList = node.getChildNodes();
								if (childList != null && childList.getLength() > 0)
								   serverAddress = childList.item(0).getNodeValue();
							} else if (name.equals("sipProxyServerAddr")) {
								childList = node.getChildNodes();
								if (childList != null && childList.getLength() > 0)
									proxyServerAddress = childList.item(0).getNodeValue();
							} else if (name.equals("sipDomain")) {
								childList = node.getChildNodes();
								if (childList != null && childList.getLength() > 0)
									domain = childList.item(0).getNodeValue();
							} else if (name.equals("sipRegistration")) {
								childList = node.getChildNodes();
								registration = false;
								if (childList != null && childList.getLength() > 0){
								   if (childList.item(0).getNodeValue().equals("true"))
									  registration = true;
								}

							} else if (name.equals("sipProxyServerPort")) {
								childList = node.getChildNodes();
								if (childList != null && childList.getLength() > 0) {
									String val = childList.item(0).getNodeValue();
								   try {
									  proxyServerPort = Integer.parseInt(val);
								   } catch (NumberFormatException nfe) {
										 throw new IOException(nfe.getMessage());
								   }
								}
							} else if (name.equals("sipAlwaysRouteToProxy")) {
                                childList = node.getChildNodes();
                                alwaysRouteToProxy = false;
                                if (childList != null && childList.getLength() > 0){
                                   if (childList.item(0).getNodeValue().equals("enable"))
                                      alwaysRouteToProxy = true;
                                }
							}
						}
					}
				}catch (SAXException sxe) {
					throw new IOException("SIP config get request error" + "\n" + sxe.getMessage());
				}catch (ParserConfigurationException pce) {
					throw new IOException("SIP config get request error" + "\n" + pce.getMessage());
				}catch (DOMException de){
					throw new IOException("SIP config get request error" + "\n" + de.getMessage());
				} catch(Exception ioe) {
					// This will catch all parser and IO errors.
					throw new IOException("SIP config get request error" + "\n" + ioe.getMessage());
		
				}

			}

		}//	Function end

	}

	/**
	 * class which stores Codec configuration details
	 */
	public class Codec implements DataConsumer, DataProvider,iEdge1000Constants{

		short	code;
        Vector  codecObjects;
        Vector  prefOrder;
        String  g7231MaxFrames;
        String  g729MaxFrames;
        String  g711ALawMaxInterval;
        String  g711ULawMaxInterval;

		/**
		 * Creates an empty Codec object
		 */
		Codec(){
			code	=	-1;
            codecObjects      =   new Vector();
            prefOrder   =   new Vector();
            g7231MaxFrames  =   "";
            g729MaxFrames   =   "";
            g711ALawMaxInterval =   "";        
            g711ULawMaxInterval =   "";        
        }

		/**
		 * Get the Codec configuration details
		 */
		public Object getData(short cmd){
			switch(cmd){
                case    iEdge1000Constants.CODECS:
                        return codecObjects;
                case    iEdge1000Constants.PREFORDER:
                        return  prefOrder;
                case    iEdge1000Constants.G7231MAXFRAMES:
                        return  g7231MaxFrames;
                case    iEdge1000Constants.G729MAXFRAMES:
                        return  g729MaxFrames;
                case    iEdge1000Constants.G711ALAWMAXINTERVAL:
                        return  g711ALawMaxInterval;
                case    iEdge1000Constants.G711ULAWMAXINTERVAL:
                        return  g711ULawMaxInterval;
				default:
						return null;
			}
		}

		/**
		 * Reads the codec configuration details from the 1000 device and stores them
		 */
		public boolean ReadFromDevice(ConfigRetriever cr)  throws IOException {

			boolean	bRet	=	false;
			try{
				bRet	=	cr.retrieve(Constants.CODEC, this);
			}
			catch(Exception ioe){
				throw new IOException(ioe.getMessage());
			}
			return bRet;
		}
		/**
		 * Reads the Codec configuration details from the 1000 device and stores them
		 */
		public void extractGetReply (LimitedDataInputStream dis) throws IOException {

			code	=	dis.readShort();

			if (code == Constants.CODEC) {

				try {

					DocumentBuilderFactory dbf = DocumentBuilderFactory.newInstance();
			        DocumentBuilder db = dbf.newDocumentBuilder();
			        Document document = db.parse(dis);

                    // The CODECs are below the CODECS tag. There should only be
                    // one of these.
                    NodeList list = document.getElementsByTagName("Codecs");
                    if (list.getLength()>0) {
                      Node codecsList = list.item(0);
                      // Cycle through all of the children
                      NodeList codecs = codecsList.getChildNodes();
                      int len = codecs.getLength();
                      for (int i=0; i<len; i++) {
                        Node codec = codecs.item(i);
                        NamedNodeMap attribs = codec.getAttributes();
                        Node value = attribs.getNamedItem("value");
                        if (value != null) {
                          codecObjects.add(value.getNodeValue());
                        } 
                      }
                    }

                    list = document.getElementsByTagName("PreferenceOrder");
                    if (list.getLength()>0) {
                      Node codecsList = list.item(0);
                      // Cycle through all of the children
                      NodeList codecs = codecsList.getChildNodes();
                      int len = codecs.getLength();
                      for (int i=0; i<len; i++) {
                        Node codec = codecs.item(i);
                        NamedNodeMap attribs = codec.getAttributes();
                        Node value = attribs.getNamedItem("value");
                        if (value != null) {
                          prefOrder.add(value.getNodeValue());
                        } 
                      }
                    }

                    list = document.getElementsByTagName("G7231MaxFrames");
                    if (list.getLength()>0) {
                      Node node = list.item(0);
                      // Asumes only 1
                      Node leaf = node.getFirstChild();
                      if (leaf != null) {
                        g7231MaxFrames = (String)leaf.getNodeValue();
                      } 
                    }

                    list = document.getElementsByTagName("G729MaxFrames");
                    if (list.getLength()>0) {
                      Node node = list.item(0);
                      // Asumes only 1
                      Node leaf = node.getFirstChild();
                      if (leaf != null) {
                        g729MaxFrames = (String)leaf.getNodeValue();
                      } 
                    }
                    list = document.getElementsByTagName("G711ALawMaxInterval");
                    if (list.getLength()>0) {
                      Node node = list.item(0);
                      // Asumes only 1
                      Node leaf = node.getFirstChild();
                      if (leaf != null) {
                        g711ALawMaxInterval = (String)leaf.getNodeValue();
                      } 
                    }
                    list = document.getElementsByTagName("G711ULawMaxInterval");
                    if (list.getLength()>0) {
                      Node node = list.item(0);
                      // Asumes only 1
                      Node leaf = node.getFirstChild();
                      if (leaf != null) {
                        g711ULawMaxInterval = (String)leaf.getNodeValue();
                      } 
                    }

				}catch (SAXException sxe) {
					throw new IOException("Codec config get request error" + "\n" + sxe.getMessage());
				}catch (ParserConfigurationException pce) {
					throw new IOException("Codec config get request error" + "\n" + pce.getMessage());
				}catch (DOMException de){
					throw new IOException("Codec config get request error" + "\n" + de.getMessage());
				} catch(Exception ioe) {
					// This will catch all parser and IO errors.
					throw new IOException("Codec config get request error" + "\n" + ioe.getMessage());
		
				}

			}

		}//	Function end

	}

}

