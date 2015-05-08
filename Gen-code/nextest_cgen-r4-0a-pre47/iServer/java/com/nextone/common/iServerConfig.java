package com.nextone.common;
   
import java.net.*;
import java.util.*;
import java.io.*;
import javax.xml.parsers.*;
import org.w3c.dom.*;
import org.xml.sax.*;
import javax.xml.transform.stream.StreamResult; 
import com.nextone.util.WeakEncryptionInputStream;
import com.nextone.util.WeakEncryptionOutputStream;
import com.nextone.util.IPMask;
import com.nextone.util.IPUtil;
import com.nextone.util.SysUtil;
import org.apache.crimson.tree.XmlDocument;

/**
 * this class represents the iserver configuration
 */
public class iServerConfig {

  //  process modules (same as in lsconfig.h)
  public final static int PROCESS_NONE = 0;
  public final static int PROCESS_LUS = 1;
  public final static int PROCESS_VPNS = 2;
  public final static int PROCESS_BCS = 3;
  public final static int PROCESS_JSERVER = 4;
  public final static int PROCESS_FAXS = 5;
  public final static int PROCESS_GIS = 7;
  public final static int PROCESS_PM = 8;

  //  debug modules for each process
  public final static int DEF      = 0;
  public final static int REGISTER = 1;
  public final static int FIND     = 2;
  public final static int AGE      = 3;
  public final static int CACHE    = 4;
  public final static int INIT     = 5;
  public final static int SEL      = 6;
  public final static int PKT      = 7;
  public final static int DB       = 8;
  public final static int SHM      = 9;
  public final static int CDR      = 10;
  public final static int FAXP     = 11;
  public final static int CONN     = 12;
  public final static int TMR      = 13;
  public final static int RED      = 14;
  public final static int XML      = 15;
  public final static int CLI      = 16;
  public final static int LMGR     = 17;
  public final static int PMGR     = 18;
  public final static int H323     = 19;
  public final static int LRQ      = 20;
  public final static int RRQ      = 21;
  public final static int ARQ      = 22;
  public final static int SIP      = 23;
  public final static int Q931     = 24;
  public final static int SCC      = 25;
  public final static int IWF      = 26;
  public final static int BRIDGE   = 27;
  public final static int FCE      = 28;
  public final static int RADC     = 29;  
  public final static int ISPD     = 30;  
  public final static int RSD      = 31;  
  public final static int DLIC     = 32;  
  public final static int IRQ      = 33;  
  public final static int ICMP     = 34;  
  public final static int EXECD    = 35;  
  public final static int SCM      = 36;  
  public final static int SCMRPC   = 37;  

  public final static int MAX_MODULES = 38;

  public final String[] moduleNames = {
    "def",
    "register",
    "find",
    "age",
    "cache",
    "init",
    "sel",
    "pkt",
    "db",
    "shm",
    "cdr",
    "faxp",
    "conn",
    "tmr",
    "red",
    "xml",
    "cli",
    "lmgr",
    "pmgr",
    "h323",
    "lrq",
    "rrq",
    "arq",
    "sip",
    "q931",
    "scc",
    "iwf",
    "bridge",
    "fce",
    "radc",
    "ispd",
    "rsd",
    "dlic",
    "irq",
    "icmp",
    "execd",
     "scm",
    "scmrpc"
  };



  public static final int SERVER_PROXY = 0;
  public static final int SERVER_REDIRECT = 1;
  public static final int SERVER_PROXYSTATEFULL = 2;
  public static final String [] sipServerTypes = {
    "Stateless Proxy",
    "Redirect",
    "Stateful Proxy",
  };

  public static final int CDRMINDCTIFIXED = 1;
  public static final int CDRMINDCTIDAILY = 2;
  public static final int CDRMINDCTISEQ = 3;
  public static final int CDRNEXTONETIME = 4;
  public static final String [] cdrTypes = {
    "Fixed",
    "Daily",
    "Sequential",
    "Timer",
  };

  public static final int CDRFORMAT_XML = 0;
  public static final int CDRFORMAT_MIND = 1;
  public static final int CDRFORMAT_SYSLOG = 2;
  public static final int CDRFORMAT_CISCORADIUS = 3;
  public static final String [] cdrFormats = {
    "XML",
    "MIND",
    "SYSLOG",
    "CISCORADIUS",
  };

  public static final int BILLING_NONE = 0;
  public static final int BILLING_POSTPAID = 1;
  public static final int BILLING_PREPAID = 2;
  public static final String [] billingTypes = {
    "None",
    "Post-paid",
    "Pre-paid",
  };

  public  static  final String  PROTOCOL_SIP  = "SIP";
  public  static  final String  PROTOCOL_H323 = "H323";

  // from codecs.h

  public static final String[] defaultCodecString = {
    "CodecGPCMU",
    "CodecGPCMA",
    "CodecG7231",
    "CodecG729"
  };

  public static final String[] sipAuth= {
    "none",
    "local",
    "radius"
  };

  public static final int[] defaultCodecInt = {
    0,
    8,
    4,
    18
  };

  /* from cdr.h */
  public static final int CDRSTART1 = 0x1;
  public static final int CDRSTART2 = 0x2;
  public static final int CDREND1   = 0x4;
  public static final int CDREND2   = 0x8;
  public static final int CDRHUNT   = 0x10;

  public static final short IFTYPE_UNDEFINED = 0;  // must always be zero
  public static final short IFTYPE_PUBLIC = 1;
  public static final short IFTYPE_PRIVATE = 2;
  public static final short IFTYPE_MEDIA_ROUTE = 3;

  // from lsconfig.h
  public static final short MAX_NUM_RAD_ENTRIES = 2;

  private SipConfig sipConfig;
  private H323Config h323Config;
  private FceConfig fceConfig;
  private FceConfigNew fceConfigNew;
  private BillingConfig billingConfig;
  private AdvancedConfig advancedConfig;
  private SystemConfig systemConfig;
  private RedundsConfig redundsConfig;
  private LoggingConfig loggingConfig;
  private Triggers  triggers;
  private RadiusConfig radiusConfig;

  private static DocumentBuilder db = null;
  private static Document doc = null;
  static {
    try {
      db = DocumentBuilderFactory.newInstance().newDocumentBuilder();
      doc = db.newDocument();
    } catch (Exception e) {
      e.printStackTrace();
    }
  }

  public static String getBillingTypeString (int st) {
    return billingTypes[st];
  }

  public static int getBillingType (String st) {
    for (int i = 0; i < billingTypes.length; i++)
      if (billingTypes[i].equals(st))
      	return i;
    return BILLING_NONE;  // default
  }

  public static String getCdrFormatString (int st) {
    return cdrFormats[st];
  }

  public static int getCdrFormat (String st) {
    for (int i = 0; i < cdrFormats.length; i++)
      if (cdrFormats[i].equals(st))
      	return i;

    return CDRFORMAT_MIND;  // default
  }

  public static String getCdrTypeString (int st) {
    return cdrTypes[st-1];
  }

  public static int getCdrType (String st) {
    for (int i = 0; i < cdrTypes.length; i++)
      if (cdrTypes[i].equals(st))
	return i+1;

    return CDRMINDCTIFIXED;  // default
  }

  public static String getSipServerTypeString (int st) {
    return sipServerTypes[st];
  }

  public static int getSipServerType (String st) {
    for (int i = 0; i < sipServerTypes.length; i++)
      if (sipServerTypes[i].equals(st))
	return i;

    return SERVER_PROXY;  // default
  }

  public iServerConfig (String xmlString) throws IOException, SAXException, DOMException {
    this(db.parse(new InputSource(new StringReader(xmlString))).getDocumentElement());
  }

  public iServerConfig (Element element) throws DOMException {
    String version = element.getAttribute("version");
    if (version.equals("1.0"))
      create10(element);
    else
      throw new DOMException(DOMException.NOT_FOUND_ERR, "Do not recognize config version " + version);
  }

  private void create10 (Element element) throws DOMException {
/*
    NodeList list = element.getElementsByTagName("Triggers");
    if (list == null || list.getLength() != 1) {
      triggers = new Triggers();
      triggers.setValid(false);
    } else
      triggers = new Triggers((Element)list.item(0));
*/

    NodeList list = element.getElementsByTagName("sipConfig");
    if (list == null || list.getLength() != 1){
      sipConfig = new SipConfig();
      sipConfig.setValid(false);
    }else
      sipConfig = new SipConfig((Element)list.item(0));

    list = element.getElementsByTagName("h323Config");
    if (list == null || list.getLength() != 1){
      h323Config = new H323Config();
      h323Config.setValid(false);
    }else
      h323Config = new H323Config((Element)list.item(0));

    list = element.getElementsByTagName("fceConfig");
    if (list == null || list.getLength() != 1){
      fceConfig = new FceConfig();
      fceConfig.setValid(false);
    }else
      fceConfig = new FceConfig((Element)list.item(0));

    list = element.getElementsByTagName("fceConfigNew");
    if (list == null || list.getLength() != 1){
      fceConfigNew = new FceConfigNew();
      fceConfigNew.setValid(false);
    }else
      fceConfigNew = new FceConfigNew((Element)list.item(0));
   

    list = element.getElementsByTagName("billingConfig");
    if (list == null || list.getLength() != 1){
      billingConfig = new BillingConfig();
      billingConfig.setValid(false);
    }else
      billingConfig = new BillingConfig((Element)list.item(0));

    list = element.getElementsByTagName("advancedConfig");
    if (list == null || list.getLength() != 1){
      advancedConfig = new AdvancedConfig();
      advancedConfig.setValid(false);
    }else
      advancedConfig = new AdvancedConfig((Element)list.item(0));

    list = element.getElementsByTagName("systemConfig");
    if (list == null || list.getLength() != 1){
      systemConfig = new SystemConfig();
      systemConfig.setValid(false);
    }else
      systemConfig = new SystemConfig((Element)list.item(0));

    list = element.getElementsByTagName("redundsConfig");
    if (list == null || list.getLength() != 1){
      redundsConfig = new RedundsConfig();
      redundsConfig.setValid(false);
    } else
      redundsConfig = new RedundsConfig((Element)list.item(0));

    list = element.getElementsByTagName("loggingConfig");
    if (list == null || list.getLength() != 1){
      loggingConfig = new LoggingConfig();
      loggingConfig.setValid(false);
    } else
      loggingConfig = new LoggingConfig((Element)list.item(0));

    list = element.getElementsByTagName("radiusConfig");
    if (list == null || list.getLength() != 1){
      radiusConfig = new RadiusConfig();
      radiusConfig.setValid(false);
    } else
      radiusConfig= new RadiusConfig((Element)list.item(0));


    
  }

  public iServerConfig () {
    triggers  = new Triggers();
    sipConfig = new SipConfig();
    h323Config = new H323Config();
    fceConfig = new FceConfig();
    fceConfigNew =  new FceConfigNew();
    billingConfig = new BillingConfig();
    advancedConfig = new AdvancedConfig();
    systemConfig = new SystemConfig();
    redundsConfig = new RedundsConfig();
    loggingConfig = new LoggingConfig();
    radiusConfig  = new RadiusConfig();
  }

 
  public String toString () {
    try {
//      return getXMLRepresentation().toString();
      return getXMLString();
    } catch (Exception e) {
      e.printStackTrace();
    }
    return "";
  }

  public String getXMLString() {
    StringWriter  sw= new StringWriter();
    try{
      Element e = doc.getDocumentElement();
      if(e  != null){
        doc.removeChild((Node)e);
      }
      doc.appendChild(getXMLRepresentation());
      XmlDocument xdoc = (XmlDocument) doc;

      xdoc.write (sw); 
    }catch(Exception ie){
      ie.printStackTrace();
    }
    Element e = doc.getDocumentElement();
    if(e  != null){
      doc.removeChild((Node)e);
    }
    return sw.toString();
  }
  
  public Node getXMLRepresentation () throws DOMException {
    Element root = doc.createElement("iServerConfig");
    root.setAttribute("version", "1.0");
//    root.appendChild(triggers.getXMLRepresentation());
    root.appendChild(sipConfig.getXMLRepresentation());
    root.appendChild(h323Config.getXMLRepresentation());
    root.appendChild(fceConfig.getXMLRepresentation());
    root.appendChild(fceConfigNew.getXMLRepresentation());
    root.appendChild(billingConfig.getXMLRepresentation());
    root.appendChild(advancedConfig.getXMLRepresentation());
    root.appendChild(systemConfig.getXMLRepresentation());
    root.appendChild(redundsConfig.getXMLRepresentation());
    root.appendChild(loggingConfig.getXMLRepresentation());
    root.appendChild(radiusConfig.getXMLRepresentation());

    return root;
  }

  public boolean equals (Object o) {
    if (o.getClass().equals(iServerConfig.class)) {
      iServerConfig given = (iServerConfig)o;
      if (sipConfig.equals(given.getSipConfig()) &&
	  h323Config.equals(given.getH323Config()) &&
	  fceConfig.equals(given.getFceConfig()) &&
    fceConfigNew.equals(given.getFceConfigNew()) &&
	  billingConfig.equals(given.getBillingConfig()) &&
	  advancedConfig.equals(given.getAdvancedConfig()) &&
	  systemConfig.equals(given.getSystemConfig()) &&
	  redundsConfig.equals(given.getRedundsConfig()) &&
	  loggingConfig.equals(given.getLoggingConfig())  &&
    radiusConfig.equals(given.getRadiusConfig())  )
	return true;
    }

    return false;
  }

  public String [] getAvailableInterfaces () {
    return fceConfig.getIpFilterConfig().getAvailableInterfaces();
  }

  public Triggers getTriggers(){
    return triggers;
  }

  public SipConfig getSipConfig () {
    return sipConfig;
  }

  public H323Config getH323Config () {
    return h323Config;
  }

  public FceConfig getFceConfig () {
    return fceConfig;
  }

  public FceConfigNew getFceConfigNew () {
    return fceConfigNew;
  }

  public BillingConfig getBillingConfig () {
    return billingConfig;
  }

  public AdvancedConfig getAdvancedConfig () {
    return advancedConfig;
  }

  public SystemConfig getSystemConfig () {
    return systemConfig;
  }

  public RedundsConfig getRedundsConfig () {
    return redundsConfig;
  }

  public LoggingConfig getLoggingConfig () {
    return loggingConfig;
  }

  public RadiusConfig getRadiusConfig() {
    return radiusConfig;
  }

  public class SipConfig {
    private boolean configValid;

    private String serverName;
    private String sipDomain;
    private boolean authEnabled;
    private String authPassword;
    private boolean recordRouteEnabled;
    private int serverType;
    private int maxForwards;
    private int sipTimerC;
    private int sipPort;
    private int auth;
    private boolean obpEnabled;
    private boolean internalCallAllowed;
    private boolean dynamicEPAllowed;

    public boolean equals (SipConfig sc) {
      if (((serverName == null && sc.getServerName() == null) ||
	   serverName.equals(sc.getServerName())) &&
	  ((sipDomain == null && sc.getSipDomain() == null) ||
	   sipDomain.equals(sc.getSipDomain())) &&
          auth == sc.getAuthentication() &&
          //authEnabled deprecated
	  //authEnabled == sc.isAuthenticationEnabled() &&
	  ((authPassword == null && sc.getAuthenticationPassword() == null) ||
	   authPassword.equals(sc.getAuthenticationPassword())) &&
	  recordRouteEnabled == sc.isRecordRouteEnabled() &&
          maxForwards == sc.getMaxForwards() &&
          sipTimerC == sc.getSipTimerC() &&
	  serverType == sc.getServerType() &&
	  sipPort == sc.getSipPort()
	&& obpEnabled == sc.isOBPEnabled()
	&& internalCallAllowed == sc.isInternalCallAllowed()
	&& dynamicEPAllowed == sc.isDynamicEPAllowed()
)
	return true;

      return false;
    }

    SipConfig () {
      setValid(true);
    }

    SipConfig (Element element) throws DOMException {
      this();

      NodeList list = element.getElementsByTagName("sipAuthorization");
      if (list != null && list.getLength() == 1) {
	Element authElem = (Element)list.item(0);
	String status = authElem.getAttribute("status");
	authEnabled = "enabled".equals(status);
        //authEnabled deprecated
        if(authEnabled)
          auth = 1;
        else
          auth = 0;

	list = authElem.getElementsByTagName("password");
	if (list != null && list.getLength() == 1) {
	  Node childNode = list.item(0).getLastChild();
	  if (childNode != null)
	    authPassword = childNode.getNodeValue();
	}
        list = authElem.getElementsByTagName("auth");
        if (list != null && list.getLength() == 1) {
          Node childNode = list.item(0).getLastChild();
          if (childNode != null){
            try{
                  auth = Integer.parseInt(childNode.getNodeValue());
            }catch(Exception e){}
          }
        }
      }

      list = element.getElementsByTagName("sipServerType");
      if (list != null && list.getLength() == 1) {
	Node childNode = list.item(0).getLastChild();
	if (childNode != null && childNode.getNodeValue() != null)
	  serverType = iServerConfig.getSipServerType(childNode.getNodeValue());
      }

      list = element.getElementsByTagName("serverName");
      if (list != null && list.getLength() == 1) {
	Node childNode = list.item(0).getLastChild();
	if (childNode != null)
	  serverName = childNode.getNodeValue();
      }

      list = element.getElementsByTagName("sipDomain");
      if (list != null && list.getLength() == 1) {
	Node childNode = list.item(0).getLastChild();
	if (childNode != null)
	  sipDomain = childNode.getNodeValue();
      }

      list = element.getElementsByTagName("recordRoute");
      if (list != null && list.getLength() == 1) {
	String status = ((Element)list.item(0)).getAttribute("status");
	recordRouteEnabled = "enabled".equals(status);
      }
	list = element.getElementsByTagName("internalCall");
	if (list != null && list.getLength() == 1) {
		String status = ((Element) list.item(0)).getAttribute("status");
		internalCallAllowed = "enabled".equals(status);
	}
	list = element.getElementsByTagName("obp");
	if (list != null && list.getLength() == 1) {
		String status = ((Element) list.item(0)).getAttribute("status");
		obpEnabled = "enabled".equals(status);
	}			
	list = element.getElementsByTagName("dynamicEndpoint");
	if (list != null && list.getLength() == 1) {
		String status = ((Element) list.item(0)).getAttribute("status");
		dynamicEPAllowed = "enabled".equals(status);
	}
      list = element.getElementsByTagName("sipMaxForwards");
      if (list != null && list.getLength() == 1) {
	Node childNode = list.item(0).getLastChild();
	if (childNode != null && childNode.getNodeValue() != null)
	  maxForwards = Integer.parseInt(childNode.getNodeValue());
      }

      list = element.getElementsByTagName("sipTimerC");
      if (list != null && list.getLength() == 1) {
        Node childNode = list.item(0).getLastChild();
        if (childNode != null && childNode.getNodeValue() != null)
          sipTimerC = Integer.parseInt(childNode.getNodeValue());
      }
      list = element.getElementsByTagName("sipPort");
      if (list != null && list.getLength() == 1) {
        Node childNode = list.item(0).getLastChild();
        if (childNode != null && childNode.getNodeValue() != null)
          sipPort = Integer.parseInt(childNode.getNodeValue());
      }
    }

    public void setValid (boolean val) {
      configValid = val;
    }

    public boolean isValid () {
      return configValid;
    }

    public synchronized String getServerName () {
      return serverName;
    }

    public synchronized void setServerName (String newName) {
      serverName = newName;
    }

    public synchronized String getSipDomain () {
      return sipDomain;
    }

    public synchronized void setSipDomain (String newDomain) {
      sipDomain = newDomain;
    }
//  authEnabled deprecated
/*
    public synchronized boolean isAuthenticationEnabled () {
      return authEnabled;
    }

    public synchronized void setAuthenticationEnabled (boolean flag) {
      authEnabled = flag;
    }
*/
    public synchronized int getAuthentication() {
      return auth;
    }

    public synchronized void setAuthentication(int auth) {
      this.auth = auth;
    }
    public synchronized String getAuthenticationPassword () {
      return authPassword;
    }

    public synchronized void setAuthenticationPassword (String passwd) {
      authPassword = passwd;
    }

    public synchronized boolean isRecordRouteEnabled () {
      return recordRouteEnabled;
    }

    public synchronized void setRecordRouteEnabled (boolean flag) {
      recordRouteEnabled = flag;
    }
	public synchronized boolean isOBPEnabled() {
		return obpEnabled;
	}

	public synchronized void setOBPEnabled(boolean flag) {
		obpEnabled = flag;
	}
			
	public synchronized boolean isInternalCallAllowed() {
		return internalCallAllowed;
	}

	public synchronized void setInternalCallAllowed(boolean flag) {
		internalCallAllowed = flag;
	}
	public synchronized boolean isDynamicEPAllowed() {
		return dynamicEPAllowed;
	}

	public synchronized void setDynamicEPAllowed(boolean flag) {
		dynamicEPAllowed = flag;
	}
	
    public synchronized int getServerType () {
      return serverType;
    }

    public synchronized void setServerType (int stype) {
      serverType = stype;
    }

    public synchronized int getMaxForwards () {
      return maxForwards;
    }

    public synchronized void setMaxForwards (int fw) {
      maxForwards = fw;
    }

    public synchronized int getSipTimerC() {
      return sipTimerC;
    }

    public synchronized void setSipTimerC (int tmc) {
      sipTimerC = tmc;
    }
      public synchronized int getSipPort() {
      return sipPort;
    }

    public synchronized void setSipPort (int p) {
      sipPort = p;
    }
      
    public synchronized Node getXMLRepresentation () throws DOMException {
      Element root = doc.createElement("sipConfig");
      if (!isValid())
        return root;

      Element tag = doc.createElement("sipAuthorization");
      Text data;
      //authEnabled deprecated
      tag.setAttribute("status",(auth > 0) ?"enabled":"disabled");
      if (authPassword != null && authPassword.length() > 0) {
	Element tag2 = doc.createElement("password");
	data = doc.createTextNode(authPassword);
	tag2.appendChild(data);
	tag.appendChild(tag2);
      }

      Element tag2 = doc.createElement("auth");
      data = doc.createTextNode(String.valueOf(auth));
      tag2.appendChild(data);
      tag.appendChild(tag2);

      root.appendChild(tag);

      if (serverName != null && serverName.length() > 0) {
	tag = doc.createElement("serverName");
	data = doc.createTextNode(serverName);
	tag.appendChild(data);
	root.appendChild(tag);
      }

      if (sipDomain != null && sipDomain.length() > 0) {
	tag = doc.createElement("sipDomain");
	data = doc.createTextNode(sipDomain);
	tag.appendChild(data);
	root.appendChild(tag);
      }

      tag = doc.createElement("sipServerType");
      data = doc.createTextNode(iServerConfig.getSipServerTypeString(serverType));
      tag.appendChild(data);
      root.appendChild(tag);

      tag = doc.createElement("recordRoute");
      tag.setAttribute("status", recordRouteEnabled?"enabled":"disabled");
      root.appendChild(tag);

tag = doc.createElement("obp");
tag.setAttribute(
	"status",
	obpEnabled ? "enabled" : "disabled");
root.appendChild(tag);

tag = doc.createElement("internalCall");
tag.setAttribute(
	"status",
	internalCallAllowed ? "enabled" : "disabled");
root.appendChild(tag);

tag = doc.createElement("dynamicEndpoint");
tag.setAttribute(
	"status",
	dynamicEPAllowed ? "enabled" : "disabled");
root.appendChild(tag);
			
      tag = doc.createElement("sipMaxForwards");
      data = doc.createTextNode(String.valueOf(maxForwards));
      tag.appendChild(data);
      root.appendChild(tag);

      tag = doc.createElement("sipTimerC");
      data = doc.createTextNode(String.valueOf(sipTimerC));
      tag.appendChild(data);
      root.appendChild(tag);

      tag = doc.createElement("sipPort");
      data = doc.createTextNode(String.valueOf(sipPort));
      tag.appendChild(data);
      root.appendChild(tag);

      return root;
    }
  }

  public class H323Config {
    private boolean configValid;

    private String sgkRegPrefix;
    private String gkId;
    private int rrqTimer;
    private int rrqTtl;
    private int cps;
    private int instances;
    private int sgkMaxCalls;
    private int maxCalls;
    private int infoTransCap;
    private boolean h245RoutingEnabled;
    private boolean forceH245;
    private boolean callRoutingEnabled;
    private boolean fastStartEnabled;
    private boolean localProceeding;
    private boolean allowDestArqEnabled;
    private boolean allowAuthArqEnabled;
    private boolean h245TunnelEnabled;

    public boolean equals (H323Config hc) {
      if (((sgkRegPrefix == null && hc.getSgkRegistrationPrefix() == null) ||
	   sgkRegPrefix.equals(hc.getSgkRegistrationPrefix())) &&
	  ((gkId == null && hc.getGkId() == null) ||
	   gkId.equals(hc.getGkId())) &&
	  rrqTimer == hc.getRrqTimer() &&
	  rrqTtl == hc.getRrqTtl() &&
	  h245RoutingEnabled == hc.isH245RoutingEnabled() &&
	  forceH245 == hc.isH245Forced() &&
	  callRoutingEnabled == hc.isCallRoutingEnabled() &&
	  fastStartEnabled == hc.isFastStartEnabled() &&
          localProceeding == hc.isLocalProceedingEnabled() &&
    cps ==  hc.getCps() &&
    instances == hc.getInstances() &&
    sgkMaxCalls ==  hc.getSgkMaxCalls() &&
    maxCalls    ==  hc.getMaxCalls() &&
    infoTransCap  ==  hc.getInfoTransCap()  &&
    allowDestArqEnabled  ==  hc.isAllowDestArqEnabled() &&
    allowAuthArqEnabled  ==  hc.isAllowAuthArqEnabled() &&
    h245TunnelEnabled  ==  hc.isH245TunnelEnabled())
	    return true;

      return false;
    }

    H323Config () {
      setValid(true);
    }

    H323Config (Element element) throws DOMException {
      this();

      NodeList list = element.getElementsByTagName("sgkRegPrefix");
      if (list != null && list.getLength() == 1) {
	      Node childNode = list.item(0).getLastChild();
	      if (childNode != null)
	        sgkRegPrefix = childNode.getNodeValue();
      }

      list = element.getElementsByTagName("gkId");
      if (list != null && list.getLength() == 1) {
	      Node childNode = list.item(0).getLastChild();
	      if (childNode != null)
	        gkId = childNode.getNodeValue();
      }

      list = element.getElementsByTagName("rrqTimer");
      if (list != null && list.getLength() == 1) {
	      Node childNode = list.item(0).getLastChild();
	      if (childNode != null && childNode.getNodeValue() != null)
	        rrqTimer = Integer.parseInt(childNode.getNodeValue());
      }

      list = element.getElementsByTagName("rrqTtl");
      if (list != null && list.getLength() == 1) {
	      Node childNode = list.item(0).getLastChild();
	      if (childNode != null && childNode.getNodeValue() != null)
	        rrqTtl = Integer.parseInt(childNode.getNodeValue());
      }

      list = element.getElementsByTagName("h245Routing");
      if (list != null && list.getLength() == 1) {
	      String status = ((Element)list.item(0)).getAttribute("status");
	      h245RoutingEnabled = "enabled".equals(status);
      }

      list = element.getElementsByTagName("forceH245");
      if (list != null && list.getLength() == 1) {
	      String status = ((Element)list.item(0)).getAttribute("status");
	      forceH245 = "enabled".equals(status);
      }

      list = element.getElementsByTagName("callRouting");
      if (list != null && list.getLength() == 1) {
	      String status = ((Element)list.item(0)).getAttribute("status");
	      callRoutingEnabled = "enabled".equals(status);
      }

      list = element.getElementsByTagName("fastStart");
      if (list != null && list.getLength() == 1) {
	      String status = ((Element)list.item(0)).getAttribute("status");
	      fastStartEnabled = "enabled".equals(status);
      }

      list = element.getElementsByTagName("callsPerSecond");
      if (list != null && list.getLength() == 1) {
	      Node childNode = list.item(0).getLastChild();
	      if (childNode != null && childNode.getNodeValue() != null)
	        cps = Integer.parseInt(childNode.getNodeValue());
      }

      list = element.getElementsByTagName("instances");
      if (list != null && list.getLength() == 1) {
	      Node childNode = list.item(0).getLastChild();
	      if (childNode != null && childNode.getNodeValue() != null)
	        instances = Integer.parseInt(childNode.getNodeValue());
      }

      list = element.getElementsByTagName("sgkMaxCalls");
      if (list != null && list.getLength() == 1) {
	      Node childNode = list.item(0).getLastChild();
	      if (childNode != null && childNode.getNodeValue() != null)
	        sgkMaxCalls = Integer.parseInt(childNode.getNodeValue());
      }

      list = element.getElementsByTagName("maxCalls");
      if (list != null && list.getLength() == 1) {
	      Node childNode = list.item(0).getLastChild();
	      if (childNode != null && childNode.getNodeValue() != null)
	        maxCalls = Integer.parseInt(childNode.getNodeValue());
      }

      list = element.getElementsByTagName("localProceeding");
      if (list != null && list.getLength() == 1) {
	      String status = ((Element)list.item(0)).getAttribute("status");
	      localProceeding = "enabled".equals(status);
      }

      list = element.getElementsByTagName("infoTransCap");
      if (list != null && list.getLength() == 1) {
	      Node childNode = list.item(0).getLastChild();
	      if (childNode != null && childNode.getNodeValue() != null)
	        infoTransCap = Integer.parseInt(childNode.getNodeValue());
      }

      list = element.getElementsByTagName("allowDestArq");
      if (list != null && list.getLength() == 1) {
	      String status = ((Element)list.item(0)).getAttribute("status");
	      allowDestArqEnabled = "enabled".equals(status);
      }

      list = element.getElementsByTagName("allowAuthArq");
      if (list != null && list.getLength() == 1) {
	      String status = ((Element)list.item(0)).getAttribute("status");
	      allowAuthArqEnabled = "enabled".equals(status);
      }

      list = element.getElementsByTagName("h245Tunnel");
      if (list != null && list.getLength() == 1) {
         String status = ((Element)list.item(0)).getAttribute("status");
         h245TunnelEnabled = "enabled".equals(status);
      }
    }

    public void setValid (boolean val) {
      configValid = val;
    }

    public boolean isValid () {
      return configValid;
    }

    public synchronized String getSgkRegistrationPrefix () {
      return sgkRegPrefix;
    }

    public synchronized void setSgkRegistrationPrefix (String prefix) {
      sgkRegPrefix = prefix;
    }

    public synchronized String getGkId () {
      return gkId;
    }

    public synchronized void setGkId (String id) {
      gkId = id;
    }

    public synchronized int getRrqTimer () {
      return rrqTimer;
    }

    public synchronized void setRrqTimer (int timer) {
      rrqTimer = timer;
    }

    public synchronized int getRrqTtl () {
      return rrqTtl;
    }

    public synchronized void setRrqTtl (int ttl) {
      rrqTtl = ttl;
    }

    public synchronized boolean isH245RoutingEnabled () {
      return h245RoutingEnabled;
    }

    public synchronized void setH245RoutingEnabled (boolean flag) {
      h245RoutingEnabled = flag;
    }

    public synchronized boolean isH245Forced () {
      return forceH245;
    }

    public synchronized void setH245Forced (boolean flag) {
      forceH245 = flag;
    }

    public synchronized boolean isCallRoutingEnabled () {
      return callRoutingEnabled;
    }

    public synchronized void setCallRoutingEnabled (boolean flag) {
      callRoutingEnabled = flag;
    }

    public synchronized boolean isFastStartEnabled () {
      return fastStartEnabled;
    }

    public synchronized void setFastStartEnabled (boolean flag) {
      fastStartEnabled = flag;
    }

    public synchronized void setCps(int calls) {
      cps = calls;
    }

    public synchronized int getCps() {
      return cps;
    }

    public synchronized void setInstances(int instances) {
      this.instances = instances;
    }

    public synchronized int getInstances() {
      return instances;
    }

    public synchronized void setSgkMaxCalls(int calls) {
      this.sgkMaxCalls = calls;
    }

    public synchronized int getSgkMaxCalls() {
      return sgkMaxCalls;
    }

    public synchronized void setMaxCalls(int calls) {
      this.maxCalls = calls;
    }

    public synchronized int getMaxCalls() {
      return maxCalls;
    }

    public synchronized boolean isLocalProceedingEnabled () {
      return localProceeding;
    }

    public synchronized void setLocalProceedingEnabled (boolean flag) {
      localProceeding = flag;
    }

    public synchronized void setInfoTransCap(int cap) {
      this.infoTransCap = cap;
    }

    public synchronized int getInfoTransCap() {
      return infoTransCap;
    }

    
    public synchronized boolean isAllowDestArqEnabled () {
      return allowDestArqEnabled;
    }

    public synchronized void setAllowDestArqEnabled(boolean flag) {
      allowDestArqEnabled = flag;
    }

    public synchronized boolean isAllowAuthArqEnabled () {
      return allowAuthArqEnabled;
    }

    public synchronized void setAllowAuthArqEnabled(boolean flag) {
      allowAuthArqEnabled = flag;
    }

    public synchronized boolean isH245TunnelEnabled () {
      return h245TunnelEnabled;
    }

    public synchronized void setH245TunnelEnabled(boolean flag) {
      h245TunnelEnabled = flag;
    }

    public synchronized Node getXMLRepresentation () throws DOMException {
      Element root = doc.createElement("h323Config");
      if (!isValid())
        return root;

      Element tag;
      Text data;
      if (sgkRegPrefix != null && sgkRegPrefix.length() > 0) {
	      tag = doc.createElement("sgkRegPrefix");
	      data = doc.createTextNode(sgkRegPrefix);
	      tag.appendChild(data);
	      root.appendChild(tag);
      }
      if (gkId != null && gkId.length() > 0) {
	    tag = doc.createElement("gkId");
	    data = doc.createTextNode(gkId);
	    tag.appendChild(data);
	    root.appendChild(tag);
      }

      tag = doc.createElement("rrqTimer");
      data = doc.createTextNode(String.valueOf(rrqTimer));
      tag.appendChild(data);
      root.appendChild(tag);

      tag = doc.createElement("rrqTtl");
      data = doc.createTextNode(String.valueOf(rrqTtl));
      tag.appendChild(data);
      root.appendChild(tag);

      tag = doc.createElement("h245Routing");
      tag.setAttribute("status", h245RoutingEnabled?"enabled":"disabled");
      root.appendChild(tag);
      tag = doc.createElement("forceH245");
      tag.setAttribute("status", forceH245?"enabled":"disabled");
      root.appendChild(tag);
      tag = doc.createElement("callRouting");
      tag.setAttribute("status", callRoutingEnabled?"enabled":"disabled");
      root.appendChild(tag);
      tag = doc.createElement("fastStart");
      tag.setAttribute("status", fastStartEnabled?"enabled":"disabled");
      root.appendChild(tag);

      tag = doc.createElement("callsPerSecond");
      data = doc.createTextNode(String.valueOf(cps));
      tag.appendChild(data);
      root.appendChild(tag);

      tag = doc.createElement("instances");
      data = doc.createTextNode(String.valueOf(instances));
      tag.appendChild(data);
      root.appendChild(tag);

      tag = doc.createElement("sgkMaxCalls");
      data = doc.createTextNode(String.valueOf(sgkMaxCalls));
      tag.appendChild(data);
      root.appendChild(tag);

      tag = doc.createElement("maxCalls");
      data = doc.createTextNode(String.valueOf(maxCalls));
      tag.appendChild(data);
      root.appendChild(tag);

      tag = doc.createElement("localProceeding");
      tag.setAttribute("status", localProceeding?"enabled":"disabled");
      root.appendChild(tag);

      tag = doc.createElement("infoTransCap");
      data = doc.createTextNode(String.valueOf(infoTransCap));
      tag.appendChild(data);
      root.appendChild(tag);

      tag = doc.createElement("allowDestArq");
      tag.setAttribute("status", allowDestArqEnabled?"enabled":"disabled");
      root.appendChild(tag);

      tag = doc.createElement("allowAuthArq");
      tag.setAttribute("status", allowAuthArqEnabled?"enabled":"disabled");
      root.appendChild(tag);

      tag = doc.createElement("h245Tunnel");
      tag.setAttribute("status", h245TunnelEnabled?"enabled":"disabled");
      root.appendChild(tag);

      return root;
    }
  }

  public class FceConfig {
    private boolean configValid;

    private String fwName;
    private InetAddress fwConnectAddr;
    private boolean h245Enabled;
    private boolean isPublic;
    private List networkList;

    public static final String AUTH_TYPE_MD5 = "MD5";
    public static final String AUTH_TYPE_SHA1 = "SHA1";

    private AravoxConfig aravoxConfig;
    private IpFilterConfig ipFilterConfig;

    public boolean equals (FceConfig fc) {
      if (((fwName == null && fc.getFirewallName() == null) ||
	   fwName.equals(fc.getFirewallName())) &&
	  ((fwConnectAddr == null && fc.getFirewallConnectAddr() == null) ||
	   fwConnectAddr.equals(fc.getFirewallConnectAddr())) &&
	  h245Enabled == fc.isH245Enabled() &&
	  isPublic == fc.isDefaultPublic() &&
	  networkList.equals(fc.getNetworkList()) &&
	  ipFilterConfig.equals(fc.getIpFilterConfig()) &&
	  aravoxConfig.equals(fc.getAravoxConfig()))
	return true;

      return false;
    }

    FceConfig () {
      setValid(true);
      aravoxConfig = new AravoxConfig();
      ipFilterConfig = new IpFilterConfig();
      networkList = Collections.synchronizedList(new LinkedList());
      fwName  = "none";
    }

    FceConfig (Element element) throws DOMException {
      this();
      NodeList list = element.getElementsByTagName("firewallName");
      if (list != null && list.getLength() == 1) {
	Element fwElem = (Element)list.item(0);
	h245Enabled = Boolean.valueOf(fwElem.getAttribute("h245")).booleanValue();
	Node childNode = fwElem.getLastChild();
	if (childNode != null)
	  fwName = childNode.getNodeValue();
      }

      list = element.getElementsByTagName("firewallConnectAddr");
      if (list != null && list.getLength() == 1) {
	Node childNode = list.item(0).getLastChild();
	if (childNode != null) {
	  try {
	    fwConnectAddr = InetAddress.getByName(childNode.getNodeValue());
	  } catch (IOException ie) {
	    throw new DOMException(DOMException.NOT_FOUND_ERR, "Unable to parse firewallConnectAddr");
	  }
      	}
      }

      list = element.getElementsByTagName("networkDefault");
      if (list != null && list.getLength() == 1) {
	isPublic = Boolean.valueOf(((Element)list.item(0)).getAttribute("public")).booleanValue();
      } else
	isPublic = true;  // true by default for older iServers

      list = element.getElementsByTagName("privateNet");
      int len = list.getLength();
      for (int i = 0; i < len; i++) {
	Element childElem = (Element)list.item(i);
	String ip = childElem.getAttribute("ip");
	String mask = childElem.getAttribute("mask");
	try {
	  addNetworkList(ip, mask, false);
	} catch (IOException ie) {
	  throw new DOMException(DOMException.NOT_FOUND_ERR, "Unable to parse privateNet for " + ip + "/" + mask);
	}
      }

      list = element.getElementsByTagName("publicNet");
      len = list.getLength();
      for (int i = 0; i < len; i++) {
	Element childElem = (Element)list.item(i);
	String ip = childElem.getAttribute("ip");
	String mask = childElem.getAttribute("mask");
	try {
	  addNetworkList(ip, mask, true);
	} catch (IOException ie) {
	  throw new DOMException(DOMException.NOT_FOUND_ERR, "Unable to parse publicNet for " + ip + "/" + mask);
	}
      }

      list = element.getElementsByTagName("AravoxCfg");
      if (list != null && list.getLength() == 1)
        try{
      	  aravoxConfig = new AravoxConfig((Element)list.item(0));
        }catch(SAXException se){
	  throw new DOMException(DOMException.NOT_FOUND_ERR, "Unable to parse Aravox Configuration");
        }catch(IOException ie){
	  throw new DOMException(DOMException.NOT_FOUND_ERR, "Unable to parse Aravox Configuration");
        }

      list = element.getElementsByTagName("IpFilterCfg");
      if (list != null && list.getLength() == 1) {
	try {
	  ipFilterConfig = new IpFilterConfig((Element)list.item(0));
	} catch (SAXException se) {
	  throw new DOMException(DOMException.NOT_FOUND_ERR, "Unable to parse IP filter Configuration");
	} catch (IOException ie) {
	  throw new DOMException(DOMException.NOT_FOUND_ERR, "Unable to parse IP filter Configuration");
	}
      }
        
    }

    public void setValid (boolean val) {
      configValid = val;
    }

    public boolean isValid () {
      return configValid;
    }

    public synchronized String getFirewallName () {
      return fwName;
    }

    public synchronized void setFirewallName (String name) {
      fwName = name;
    }

    public synchronized boolean isH245Enabled () {
      return h245Enabled;
    }

    public synchronized void setH245Enabled (boolean flag) {
      h245Enabled = flag;
    }

    public synchronized int getFirewallConnectAddrAsInt () {
      if (fwConnectAddr == null)
	return -1;

      return (int)(IPUtil.ipStringToLong(fwConnectAddr.getHostAddress()) & 0xffffffff);
    }

    public synchronized InetAddress getFirewallConnectAddr () {
      return fwConnectAddr;
    }

    public synchronized void setFirewallConnectAddr (int ipaddr) throws IOException {
      setFirewallConnectAddr(IPUtil.intToIPString(ipaddr));
    }

    public synchronized void setFirewallConnectAddr (String ipaddr) throws IOException {
      if (ipaddr == null || ipaddr.length() == 0)
	fwConnectAddr = InetAddress.getByName("0.0.0.0");
      else
	fwConnectAddr = InetAddress.getByName(ipaddr);
    }

    public synchronized boolean isDefaultPublic () {
      return isPublic;
    }

    public synchronized void setDefaultPublic (boolean val) {
      isPublic = val;
    }

    public synchronized void addNetworkList (int addr, int mask, boolean isPublic) throws IOException {
      addNetworkList(IPUtil.intToIPString(addr), IPUtil.intToIPString(mask), isPublic);
    }

    public synchronized void addNetworkList (String addr, String mask, boolean isPublic) throws IOException {
      addNetworkList(new IPMask(addr, mask), isPublic);
    }

    public synchronized void addNetworkList (IPMask ipm, boolean isPublic) {
      networkList.add(new NetworkList(ipm, isPublic));
    }

    public synchronized void addNetworkList (Collection c) {
      networkList.addAll(c);
    }

    public synchronized NetworkList[] getNetworkListAsArray () {
      int len = networkList.size();

      if (len == 0)
      	return null;

      NetworkList [] ipma = new NetworkList [len];
      ipma = (NetworkList [])networkList.toArray(ipma);

      // sort the listing in a proper order
      NetworkList [] result = new NetworkList [ipma.length];
      int index = 0;
      for (int i = 0; i < ipma.length; i++) {
	if (ipma[i].isNetworkPublic() == isPublic)
	  result[index++] = ipma[i];
      }
      for (int i = 0; i < ipma.length; i++) {
	if (ipma[i].isNetworkPublic() != isPublic)
	  result[index++] = ipma[i];
      }

      return result;
    }
		  
    public synchronized List getNetworkList () {
      return networkList;
    }

    public synchronized boolean removeNetList (int addr, int mask, boolean ispub) {
      boolean result = false;

      if (networkList.size() > 0) {
      	NetworkList given = null;
	try {
	  given = new NetworkList(new IPMask(IPUtil.intToIPString(addr), IPUtil.intToIPString(mask)), ispub);
	} catch (IOException ie) {
	  return result;
	}

	Iterator it = networkList.iterator();
	while (it.hasNext()) {
	  NetworkList nl = (NetworkList)it.next();
	  if (given.equals(nl)) {
	    result = true;
	    it.remove();
	  }
	}
      }

      return result;
    }

    public synchronized void clearNetworkList () {
      networkList.clear();
    }

    public synchronized IpFilterConfig getIpFilterConfig () {
      return ipFilterConfig;
    }

    public synchronized AravoxConfig getAravoxConfig () {
      return aravoxConfig;
    }

    public void setAravoxConfig (String xmlString) throws IOException, SAXException, DOMException{
      aravoxConfig = new AravoxConfig(db.parse(new InputSource(new StringReader(xmlString))).getDocumentElement());
    }

    public synchronized Node getXMLRepresentation () throws DOMException {
      Element root = doc.createElement("fceConfig");
      if (!isValid())
        return root;

      if (fwName != null) {
	Element tag = doc.createElement("firewallName");
	Text data = doc.createTextNode(fwName);
	tag.appendChild(data);
	tag.setAttribute("h245", h245Enabled?"TRUE":"FALSE");
	root.appendChild(tag);
      }

      if (fwConnectAddr != null) {
	Element tag = doc.createElement("firewallConnectAddr");
	Text data = doc.createTextNode(fwConnectAddr.getHostAddress());
	tag.appendChild(data);
	root.appendChild(tag);
      }

      Element tag = doc.createElement("networkDefault");
      tag.setAttribute("public", isPublic?"TRUE":"FALSE");
      root.appendChild(tag);

      Iterator it = getNetworkList().iterator();
      while (it.hasNext()) {
	NetworkList nl = (NetworkList)it.next();
	if (nl.isNetworkPublic())
	  tag = doc.createElement("publicNet");
	else
	  tag = doc.createElement("privateNet");
	tag.setAttribute("ip", nl.getIPMask().getIPAsString());
	tag.setAttribute("mask", nl.getIPMask().getMaskAsString());
	root.appendChild(tag);
      }

      root.appendChild(aravoxConfig.getXMLRepresentation());

      root.appendChild(ipFilterConfig.getXMLRepresentation());

      return root;
    }

    public class IpFilterConfig {
      private Interface [] interfaces;
      private boolean natEnabled;
      private boolean packetSteeringEnabled;

      IpFilterConfig () {
	interfaces= new Interface [0];
      }

      IpFilterConfig (Element element) throws IOException, SAXException, DOMException {
	this();
	NodeList list = element.getElementsByTagName("Interface");
	if (list != null) {
	  int len = list.getLength();
	  for (int i = 0; i < len; i++) {
	    Element elem = (Element)list.item(i);

	    String ifname = elem.getLastChild().getNodeValue();
	    Interface intf = getInterface(ifname);
	    if (intf == null) {
	      intf = new Interface(ifname, IFTYPE_UNDEFINED);
	      interfaces = (Interface [])SysUtil.createObjectArray(interfaces, intf);
	    }

	    short type = getInterfaceTypeAsShort(elem.getAttribute("type"));
	    if (type == IFTYPE_UNDEFINED)
	      intf.interfaceType = 0;
	    else {
	      intf.interfaceType = CommonFunctions.BIT_SET(intf.interfaceType, type);
              intf.interfaceType = CommonFunctions.BIT_RESET(intf.interfaceType, IFTYPE_UNDEFINED);
            }
	  }
	}

	list  = element.getElementsByTagName("NAT");
	if (list != null && list.getLength() == 1) {
	  Element subElement = (Element)list.item(0);
	  natEnabled  = new Boolean(subElement.getAttribute("enable")).booleanValue();
	}

	list  = element.getElementsByTagName("PacketSteering");
	if (list != null && list.getLength() == 1) {
	  Element subElement = (Element)list.item(0);
	  packetSteeringEnabled  = new Boolean(subElement.getAttribute("enable")).booleanValue();
	}
      }

      public String toString () {
	try {
	  return getXMLRepresentation().toString();
	} catch (Exception e) {
	  e.printStackTrace();
	}
	return "";
      }

      // from the given list of Interfaces, return the type bitarray for the given interface name
      private short getIfType (Interface [] ifs, String ifname) {
	for (int i = 0; i < ifs.length; i++)
	  if (ifs[i].interfaceName.equals(ifname))
	    return ifs[i].interfaceType;

	return IFTYPE_UNDEFINED; // must be zero
      }

      public synchronized void setAvailableInterfaces (String [] ifs) {
	Interface [] old = interfaces;
	interfaces = new Interface [0];
	for (int i = 0; i < ifs.length; i++)
	  interfaces = (Interface [])SysUtil.createObjectArray(interfaces, new Interface(ifs[i], getIfType(old, ifs[i])));
      }

      public synchronized String [] getAvailableInterfaces () {
	String [] ifs = new String [interfaces.length];
	for (int i = 0; i < interfaces.length; i++)
	  ifs[i] = new String(interfaces[i].interfaceName);
	return ifs;
      }

      private short getInterfaceTypeAsShort (String type) {
	return type.equals("private")?IFTYPE_PRIVATE:type.equals("public")?IFTYPE_PUBLIC:type.equals("mediaroute")?IFTYPE_MEDIA_ROUTE:IFTYPE_UNDEFINED;
      }

      private String getInterfaceTypeAsString (int type) {
	switch (type) {
	case IFTYPE_PRIVATE:
	  return "private";
	case IFTYPE_PUBLIC:
	  return "public";
	case IFTYPE_MEDIA_ROUTE:
	  return "mediaroute";
	}

	return "undefined";
      }

      // return the Interface object for the given interface name
      private Interface getInterface (String ifname) {
	for (int i = 0; i < interfaces.length; i++)
	  if (interfaces[i].interfaceName.equals(ifname))
	    return interfaces[i];

	return null;
      }

      // return the first available interface name for the given interface type
      // this method is only useful when there is atmost one interface for the given type
      private String getInterface (short type) {
	for (int i = 0; i < interfaces.length; i++)
	  if (CommonFunctions.BIT_TEST(interfaces[i].interfaceType, type))
	    return interfaces[i].interfaceName;

	return "";
      }

      // set all the interfaces to given type
      private void setInterface (String [] ifnames, short type) throws BridgeException {
	for (int i = 0; ifnames != null && i < ifnames.length; i++)
	  setInterface(ifnames[i], type);
      }

      // set the interface to the given type
      // if the interface name is not given, reset the given interface type from all interfaces
      private void setInterface (String ifname, short type) throws BridgeException {
	//	System.out.println("setting interface " + ifname + " to " + getInterfaceTypeAsString(type));
	if (ifname != null)
	  ifname = ifname.trim();
	if (ifname == null || ifname.length() == 0) {
	  for (int i = 0; i < interfaces.length; i++) {
            interfaces[i].interfaceType = CommonFunctions.BIT_RESET(interfaces[i].interfaceType, type);
	  }
	} else {
	  boolean found = false;
	  for (int j = 0; j < interfaces.length; j++) {
	    if (ifname.equals(interfaces[j].interfaceName)) {
	      if (type == IFTYPE_UNDEFINED)
		interfaces[j].interfaceType = 0;
	      else
		interfaces[j].interfaceType = CommonFunctions.BIT_SET(interfaces[j].interfaceType, type);
	      found = true;
	      break;
	    }
	  }

	  if (!found)
	    throw new BridgeException("Invalid interface name '" + ifname + "'");
	}
      }

      public synchronized String getPublicInterface () {
	return getInterface(IFTYPE_PUBLIC);
      }

      public synchronized void setPublicInterface (String ifname) throws BridgeException {
	setInterface(ifname, IFTYPE_PUBLIC);
      }

      public synchronized String getPrivateInterface () {
	return getInterface(IFTYPE_PRIVATE);
      }

      public synchronized void setPrivateInterface (String ifname) throws BridgeException {
	setInterface(ifname, IFTYPE_PRIVATE);
      }

      public synchronized String [] getMediaRouteInterface () {
	String [] ifs = new String [0];
	for (int i = 0; i < interfaces.length; i++)
	  if (CommonFunctions.BIT_TEST(interfaces[i].interfaceType, IFTYPE_MEDIA_ROUTE))
	    ifs = SysUtil.createStringArray(ifs, interfaces[i].interfaceName);

	return ifs;
      }

      public synchronized void setMediaRouteInterface (String [] ifnames) throws BridgeException {
	for (int i = 0; i < interfaces.length; i++)
	  interfaces[i].interfaceType = CommonFunctions.BIT_RESET(interfaces[i].interfaceType, IFTYPE_MEDIA_ROUTE);
	setInterface(ifnames, IFTYPE_MEDIA_ROUTE);
      }

      public  void setNatEnabled(boolean enabled){
	natEnabled = enabled;
      }
        
      public boolean isNatEnabled(){
	return natEnabled;
      }

      public  void setPacketSteeringEnabled(boolean enabled){
	packetSteeringEnabled = enabled;
      }
        
      public boolean isPacketSteeringEnabled(){
	return packetSteeringEnabled;
      }

      private class Interface {
	String interfaceName;
	short interfaceType;

	Interface (String name, short type) {
	  interfaceName = name;
	  interfaceType = type;
	}
      }

      public synchronized Node getXMLRepresentation () throws DOMException {
	Element root = doc.createElement("IpFilterCfg");
	for (int i = 0; i < interfaces.length; i++) {
	  int [] types = new int [0];
	  if (interfaces[i].interfaceType == IFTYPE_UNDEFINED) {
	    types = new int [1];
	    types[0] = IFTYPE_UNDEFINED;
	  } else {
	    if (CommonFunctions.BIT_TEST(interfaces[i].interfaceType, IFTYPE_PRIVATE))
	      types = SysUtil.createIntArray(types, IFTYPE_PRIVATE);
	    if (CommonFunctions.BIT_TEST(interfaces[i].interfaceType, IFTYPE_PUBLIC))
	      types = SysUtil.createIntArray(types, IFTYPE_PUBLIC);
	    if (CommonFunctions.BIT_TEST(interfaces[i].interfaceType, IFTYPE_MEDIA_ROUTE))
	      types = SysUtil.createIntArray(types, IFTYPE_MEDIA_ROUTE);
	  }

	  for (int j = 0; j < types.length; j++) {
	    Element tag = doc.createElement("Interface");
	    tag.setAttribute("type", getInterfaceTypeAsString(types[j]));
	    tag.appendChild(doc.createTextNode(interfaces[i].interfaceName));
	    root.appendChild(tag);
	  }
	}

	Element tag = doc.createElement("NAT");
	tag.setAttribute("enable", natEnabled?"TRUE":"FALSE");
	root.appendChild(tag);

	tag = doc.createElement("PacketSteering");
	tag.setAttribute("enable", packetSteeringEnabled?"TRUE":"FALSE");
	root.appendChild(tag);

	return root;
      }

    }


    public class AravoxConfig {

      private Hashtable aravoxList;

      AravoxConfig () {
        aravoxList  = new Hashtable();
      }

      AravoxConfig (Element element) throws IOException, SAXException, DOMException{
        this();
        NodeList list = element.getElementsByTagName("Firewall");
        if(list !=  null){
          aravoxList  = new Hashtable();
	  for (int i=0; i<list.getLength(); i++) {
	    Aravox  aravox  = new Aravox((Element)list.item(i));
	    aravoxList.put(aravox.getKey(),aravox);
	  }
        }
      }


      public synchronized Node getXMLRepresentation () throws DOMException {
	return getXMLRepresentation(true);
      }

      public synchronized Node getXMLRepresentation (boolean addStats) throws DOMException {
	Element root = doc.createElement("AravoxCfg");

        Iterator  ie  = aravoxList.values().iterator();
        while(ie.hasNext()){
          Aravox aravox = (Aravox)ie.next();
          root.appendChild(aravox.getXMLRepresentation(addStats));
        }
        return root;
      }

      public String getXMLStringWithNoStats () {
	return getXMLStringCommon(false);
      }

      public String getXMLString() {
	return getXMLStringCommon(true);
      }

      private String getXMLStringCommon (boolean addStats) {
        StringWriter  sw= new StringWriter();
        try{
          Element e = doc.getDocumentElement();
          if(e  != null){
            doc.removeChild((Node)e);
          }
          e = (Element)getXMLRepresentation(addStats);
          e.setAttribute("version", "1.0");
          doc.appendChild(e);
	  XmlDocument xdoc = (XmlDocument) doc;
	  xdoc.write (sw); 
        }catch(Exception ie){
        }

	Element e = doc.getDocumentElement();
        if(e  != null){
          doc.removeChild((Node)e);
        }

        return sw.toString();
      }

      public Hashtable getAravoxList(){
        return aravoxList;
      }
      public void setAravoxList(Hashtable fwList){
        aravoxList  = fwList;
      }

      public Aravox createNewAravox(){
        return new Aravox();
      }


      public class Aravox {
        private int firewallId;
        private InetAddress firewallAddr;
        private int userId;
        private String authType;
        private String authData;
        private boolean natEnabled;
        private boolean packetSteeringEnabled;
        private Hashtable ppcList;

        Aravox () {
          ppcList = new Hashtable();
          try{
            firewallAddr  = InetAddress.getByName("0.0.0.0");
          }catch(UnknownHostException ue){
          }
          authType  = "MD5";
          authData  = "";
          firewallId = 0;
          userId  = 0;

        }
    
        Aravox (Element element) throws DOMException {
	  this();

          if(element  !=  null){
            firewallId  = Integer.parseInt(element.getAttribute("id"));

            NodeList  list  = element.getElementsByTagName("IpAddress");
	    if (list != null && list.getLength() == 1) {
	      Node childNode = list.item(0).getLastChild();
              if (childNode != null){
		try {
		  firewallAddr = InetAddress.getByName(childNode.getNodeValue());
		} catch (IOException ie) {
		  throw new DOMException(DOMException.NOT_FOUND_ERR, "Unable to parse firewallConnectAddr");
		}

              }
	    }
            //  read the auth details
            list  = element.getElementsByTagName("Auth");
	    if (list != null && list.getLength() == 1) {
	      Element authElem = (Element)list.item(0);
              authType  = authElem.getAttribute("type");
            
              list  = authElem.getElementsByTagName("UserId");
              if (list != null && list.getLength() == 1) {
                userId  =  Integer.parseInt(list.item(0).getLastChild().getNodeValue());
              }

              list  = authElem.getElementsByTagName("AuthData");
              if (list != null && list.getLength() == 1) {
                authData  =  list.item(0).getLastChild().getNodeValue();    
              }

            }

            list  = element.getElementsByTagName("NAT");
	    if (list != null && list.getLength() == 1) {
              Element subElement = (Element)list.item(0);
              natEnabled  = (new Boolean( subElement.getAttribute("enable"))).booleanValue();
            }

            list  = element.getElementsByTagName("PacketSteering");
	    if (list != null && list.getLength() == 1) {
              Element subElement = (Element)list.item(0);
              packetSteeringEnabled  = new Boolean( subElement.getAttribute("enable")).booleanValue();
            }

            list = element.getElementsByTagName("PPC");
            if(list !=  null){
	      for (int i=0; i<list.getLength(); i++) {
		PPCConfig ppc = new PPCConfig((Element)list.item(i));
		ppcList.put(ppc.getKey(), ppc);
              }
            }

          }

        }

        public synchronized Node getXMLRepresentation () throws DOMException {
	  return getXMLRepresentation(true);
	}

	public synchronized Node getXMLRepresentation (boolean addStats) throws DOMException {
	  Element root = doc.createElement("Firewall");
          root.setAttribute("id", String.valueOf(firewallId));

	  Element tag = doc.createElement("IpAddress");
	  Text data = doc.createTextNode(firewallAddr.getHostAddress());
	  tag.appendChild(data);
	  root.appendChild(tag);

	  tag = doc.createElement("Auth");
	  tag.setAttribute("type", authType);

	  Element tag2;
          if(authType !=  null  && authType.length() >0){
            tag2  = doc.createElement("UserId");
	    data = doc.createTextNode(String.valueOf(userId));
	    tag2.appendChild(data);
	    tag.appendChild(tag2);
          }

	  if (authData!= null && authData.length() > 0) {
            tag2 = doc.createElement("AuthData");
	    data = doc.createTextNode(authData);
	    tag2.appendChild(data);
	    tag.appendChild(tag2);
	  }
	  root.appendChild(tag);

	  tag = doc.createElement("NAT");
          tag.setAttribute("enable", natEnabled?"TRUE":"FALSE");
	  root.appendChild(tag);

	  tag = doc.createElement("PacketSteering");
          tag.setAttribute("enable", packetSteeringEnabled?"TRUE":"FALSE");
	  root.appendChild(tag);

          Iterator ie = ppcList.values().iterator();
          while(ie.hasNext()) {
            PPCConfig ppc = (PPCConfig)ie.next();
            root.appendChild(ppc.getXMLRepresentation(addStats));
          }

	  return root;
        }

	public boolean equals (Object o) {
	  if (o.getClass().equals(Aravox.class)) {
	    Aravox given = (Aravox)o;
	    if (firewallId == given.getFirewallId() &&
		((firewallAddr == null && given.getFirewallAddr() == null) ||
		 firewallAddr.equals(given.getFirewallAddr())) &&
		userId == given.getUserId() &&
		((authType == null && given.getAuthType() == null) ||
		 authType.equals(given.getAuthType())) &&
		((authData == null && given.getAuthData() == null) ||
		 authData.equals(given.getAuthData())) &&
		natEnabled == given.isNatEnabled() &&
		packetSteeringEnabled == given.isPacketSteeringEnabled() &&
		((ppcList == null && given.getPPCList() == null) ||
		 ppcList.equals(given.getPPCList())))
	      return true;
	  }
	  return false;
	}

        public String getKey(){
          return String.valueOf(firewallId);
        }

        public void setFirewallId(int id){
          firewallId  = id;
        }

        public String toString() {
	  StringBuffer sb = new StringBuffer();
	  sb.append(firewallId);
	  sb.append(" (");
	  sb.append(getFirewallAddr().getHostAddress());
	  sb.append("/");
	  sb.append(getAuthType());
	  if (isNatEnabled()) {
	    sb.append("/");
	    sb.append("NAT");
	  }
	  if (isPacketSteeringEnabled()) {
	    sb.append("/");
	    sb.append("PS");
	  }
	  sb.append(")");

	  return sb.toString();
        }

        public int getFirewallId(){
          return firewallId;
        }
      
        public void setFirewallAddr(InetAddress addr){
          firewallAddr  = addr;

        }
        public InetAddress getFirewallAddr(){
          return firewallAddr;
        }

        public  void setAuthType(String type){
          authType  = type;

        }
        public String getAuthType(){
          return authType;
        }

        public void setUserId(int id){
          userId  = id;
        }

        public int getUserId(){
          return userId;
        }

        public  void setAuthData(String data){
          authData  = data;

        }
        public String getAuthData(){
          return authData;
        }

        public  void setNatEnabled(boolean enabled){
          natEnabled  = enabled;
	}
        
        public boolean isNatEnabled(){
          return natEnabled;
        }

        public  void setPacketSteeringEnabled(boolean enabled){
          packetSteeringEnabled  = enabled;

        }
        public boolean isPacketSteeringEnabled(){
          return packetSteeringEnabled;
        }

        public  void setPPCList (Hashtable ppc) {
          ppcList  = ppc;

        }
        public Hashtable getPPCList () {
          return ppcList;
        }

        public PPCConfig createNewPPC () {
          return new PPCConfig();
        }

  	public class PPCConfig {
      private String role;
      private int id;
    	private boolean adminStatus;
      private boolean signallingEnabled;
      private InetAddress publicAddr;
	    private PPCStats ppcStats;
	    private String privateIpAddr;
	    private int privateLowPort, privateHighPort;

      PPCConfig() {
	      id  = 0;
	      role  = "";
	      signallingEnabled = false;

	      try{
	        publicAddr  = InetAddress.getByName("0.0.0.0");
	      }catch(UnknownHostException ue){
	      }
	      ppcStats = new PPCStats();

	      privateLowPort = privateHighPort = 0;
	      privateIpAddr = null;
      }

          PPCConfig (Element element) throws DOMException {
	    this();
            if (element  !=  null) {
              role  = element.getAttribute("role");
	      String as = element.getAttribute("adminStatus");
	      if (as == null || as.length() == 0)
		adminStatus = true;  // older .xml files don't have this attr
	      else
		adminStatus = as.toUpperCase().equals("UP");

              NodeList  list  = element.getElementsByTagName("id");
              if (list != null && list.getLength() == 1) {
                id  =  Integer.parseInt(list.item(0).getLastChild().getNodeValue());
              }
              list  = element.getElementsByTagName("Signaling");
              if (list != null && list.getLength() == 1) {
                Element subElement  = (Element)list.item(0);
                signallingEnabled  =  new Boolean(subElement.getAttribute("enable")).booleanValue();
              }
              list  = element.getElementsByTagName("PublicAddress");
              if (list != null && list.getLength() == 1) {
		try {
		  publicAddr = InetAddress.getByName(list.item(0).getLastChild().getNodeValue());
		} catch (IOException ie) {
		  throw new DOMException(DOMException.NOT_FOUND_ERR, "Error parsing public address: " + ie.toString());
		}

              }

	      list = element.getElementsByTagName("PortAlloc");
	      if (list != null && list.getLength() == 1) {
		Element subElement = (Element)list.item(0);
		privateIpAddr = subElement.getAttribute("IpAddress");
		try {
		  privateLowPort = Integer.parseInt(subElement.getAttribute("low"));
		} catch (NumberFormatException nfe) {}
		try {
		  privateHighPort = Integer.parseInt(subElement.getAttribute("high"));
		} catch (NumberFormatException nfe) {}
	      }

	      // create the PPC Stats
	      list = element.getElementsByTagName("PPCStats");
	      if (list != null && list.getLength() == 1)
		ppcStats = new PPCStats((Element)list.item(0));
            }

          }

          public synchronized Node getXMLRepresentation () throws DOMException {
	    return getXMLRepresentation(true);
	  }

	  public synchronized Node getXMLRepresentation (boolean addStats) throws DOMException {
	    Element root = doc.createElement("PPC");
            root.setAttribute("role", role);
	    root.setAttribute("adminStatus", adminStatus?"UP":"DOWN");

	    Element tag = doc.createElement("id");
	    Text data = doc.createTextNode(String.valueOf(id));
	    tag.appendChild(data);      
	    root.appendChild(tag);

            tag = doc.createElement("Signaling");
            tag.setAttribute("enable", signallingEnabled?"TRUE":"FALSE");
            root.appendChild(tag);

	    tag = doc.createElement("PublicAddress");
	    data = doc.createTextNode(publicAddr.getHostAddress());
	    tag.appendChild(data);
	    root.appendChild(tag);

	    tag = doc.createElement("PortAlloc");
	    tag.setAttribute("IpAddress", getPrivateIpAddr());
	    tag.setAttribute("low", String.valueOf(privateLowPort));
	    tag.setAttribute("high", String.valueOf(privateHighPort));
	    root.appendChild(tag);

	    if (addStats)
	      root.appendChild(ppcStats.getXMLRepresentation());

            return root;
          }

          public String toString(){
	    StringBuffer sb = new StringBuffer();
	    sb.append(id);
	    sb.append(" (");
	    sb.append(getRole());
	    if (isSignallingEnabled())
	      sb.append("/Signal");
	    sb.append("/");
	    sb.append(getPublicAddr().getHostAddress());
	    sb.append("/admin:");
	    sb.append(adminStatus?"UP":"DOWN");
	    if (getPrivateIpAddr() != null && getPrivateIpAddr().length() > 0) {
	      sb.append("/");
	      sb.append(getPrivateIpAddr());
	      sb.append(":");
	      sb.append(privateLowPort);
	      sb.append("-");
	      sb.append(privateHighPort);
	    }
	    sb.append(")");

	    return sb.toString();
          }

          public String getRole(){
            return role;
          }
          public void setRole(String role){
            this.role = role;
          }
    
	  public boolean isAdminUp () {
	    return adminStatus;
	  }

	  public void setAdminUp (boolean status) {
	    adminStatus = status;
	  }

          public int getId(){
            return id;
          }
          public void setId(int id){
            this.id = id;
          }

          public boolean isSignallingEnabled(){
            return signallingEnabled;
          }

          public void setSignallingEnabled(boolean enable){
            signallingEnabled = enable;
          }
    
          public InetAddress getPublicAddr(){
            return publicAddr;
          }

          public void setPublicAddr(InetAddress addr){
            publicAddr  = addr;
          }

	  public String getPrivateIpAddr () {
	    return (privateIpAddr == null)?"":privateIpAddr;
	  }

	  public void setPrivateIpAddr (String addr) {
	    privateIpAddr = addr;
	  }

	  public int getPrivateLowPort () {
	    return privateLowPort;
	  }

	  public void setPrivateLowPort (int port) {
	    privateLowPort = port;
	  }

	  public int getPrivateHighPort () {
	    return privateHighPort;
	  }

	  public void setPrivateHighPort (int port) {
	    privateHighPort = port;
	  }

          public String getKey(){
            return String.valueOf(id);
          }

	  public PPCStats getPPCStats () {
	    return ppcStats;
	  }
        }


	public class PPCStats {
	  private int pinholes;
	  private int numTranslationFailed;
	  private String lastTranslationFailCause;
	  private int lastTranslationFailCauseCode;
	  private int numPermissionFailed;
	  private String lastPermissionFailCause;
	  private int lastPermissionFailCauseCode;

	  PPCStats () {
	  }

	  PPCStats (Element element) throws DOMException {
	    try {
	      pinholes = Integer.parseInt(element.getAttribute("pinholes"));
	    } catch (Exception e) {}

	    NodeList list = element.getElementsByTagName("translationFailed");
	    if (list != null && list.getLength() == 1) {
	      Element childElem = (Element)list.item(0);
	      try {
		numTranslationFailed = Integer.parseInt(childElem.getAttribute("total"));
		lastTranslationFailCauseCode = Integer.parseInt(childElem.getAttribute("code"));
	      } catch (Exception e) {}
	      if (childElem.getLastChild() != null)
		lastTranslationFailCause = childElem.getLastChild().getNodeValue();
	    }

	    list = element.getElementsByTagName("permissionFailed");
	    if (list != null && list.getLength() == 1) {
	      Element childElem = (Element)list.item(0);
	      try {
		numPermissionFailed = Integer.parseInt(childElem.getAttribute("total"));
		lastPermissionFailCauseCode = Integer.parseInt(childElem.getAttribute("code"));
	      } catch (Exception e) {}
	      if (childElem.getLastChild() != null)
		lastPermissionFailCause = childElem.getLastChild().getNodeValue();
	    }
	  }

	  public synchronized Node getXMLRepresentation () throws DOMException {
	    Element root = doc.createElement("PPCStats");
            root.setAttribute("pinholes", String.valueOf(pinholes));

	    Element tag = doc.createElement("translationFailed");
	    tag.setAttribute("total", String.valueOf(numTranslationFailed));
	    tag.setAttribute("code", String.valueOf(lastTranslationFailCauseCode));
	    Text data = doc.createTextNode(getLastTranslationFailCause());
	    tag.appendChild(data);      
	    root.appendChild(tag);

	    tag = doc.createElement("permissionFailed");
	    tag.setAttribute("total", String.valueOf(numPermissionFailed));
	    tag.setAttribute("code", String.valueOf(lastPermissionFailCauseCode));
	    data = doc.createTextNode(getLastPermissionFailCause());
	    tag.appendChild(data);      
	    root.appendChild(tag);

	    return root;
	  }

	  public synchronized int getNumPinholes () {
	    return pinholes;
	  }

	  public synchronized int getNumTranslationFailed () {
	    return numTranslationFailed;
	  }

	  public synchronized int getLastTranslationFailCauseCode () {
	    return lastTranslationFailCauseCode;
	  }

	  public synchronized String getLastTranslationFailCause () {
	    return (lastTranslationFailCause == null)?"":lastTranslationFailCause;
	  }

	  public synchronized int getNumPermissionFailed () {
	    return numPermissionFailed;
	  }

	  public synchronized int getLastPermissionFailCauseCode () {
	    return lastPermissionFailCauseCode;
	  }

	  public synchronized String getLastPermissionFailCause () {
	    return (lastPermissionFailCause == null)?"":lastPermissionFailCause;
	  }
	}
      }
    }
  }


 public class FceConfigNew {
    private boolean configValid;
    private String fwName;
    private InetAddress fwAddress;
    private PoolConfig poolConfig;


    public static final String NAME_NONE  = "none";
    public static final String NAME_NSF   = "NSF";
    public static final String NAME_MFCP  = "MFCP";
    public static final String NAME_HOTKNIFE = "HKNIFE";

    public boolean equals (FceConfigNew fcn) {
      if (((fwName == null && fcn.getFirewallName() == null) ||
	      fwName.equals(fcn.getFirewallName())) )
	      return true;
      return false;
    }

    FceConfigNew () {
      setValid(true);
      fwName = "none";
      poolConfig  = new PoolConfig();
    }

    FceConfigNew (Element element) throws DOMException {
      this();
      NodeList list = element.getElementsByTagName("firewallName");
      if (list != null && list.getLength() == 1) {
	      Element elem = (Element)list.item(0);
	      Node childNode = elem.getLastChild();
	      if (childNode != null)
	        fwName = childNode.getNodeValue();
      }

      list = element.getElementsByTagName("firewallAddress");
      if (list != null && list.getLength() == 1) {
        Element elem = (Element)list.item(0);
        Node childNode = elem.getLastChild();
        if (childNode != null) {
          try {
            fwAddress = InetAddress.getByName(childNode.getNodeValue());
          } catch (IOException ie) {
            throw new DOMException(DOMException.NOT_FOUND_ERR, "Unable to parse firewall address");
          }
        }
      }

      list = element.getElementsByTagName("PoolCfg");
      if (list != null && list.getLength() == 1)
        try{
      	  poolConfig = new PoolConfig((Element)list.item(0));
        }catch(SAXException se){
      	  throw new DOMException(DOMException.NOT_FOUND_ERR, "Unable to parse Pool Configuration");
        }catch(IOException ie){
	        throw new DOMException(DOMException.NOT_FOUND_ERR, "Unable to parse Pool Configuration");
        }

    }

    public void setValid (boolean val) {
      configValid = val;
    }

    public boolean isValid () {
      return configValid;
    }

    public synchronized String getFirewallName () {
      return fwName;
    }

    public synchronized void setFirewallName(String name) {
      this.fwName = name;
    }

    public synchronized InetAddress getFirewallAddress () {
      return fwAddress; 
    } 
    
    public synchronized int getFirewallAddressAsInt () {
      if (fwAddress == null)
        return -1;
        
      return (int)(IPUtil.ipStringToLong(fwAddress.getHostAddress()) & 0xffffffff);
    } 

    public synchronized void setFirewallAddress (int ipaddr) throws IOException {
      setFirewallAddress(IPUtil.intToIPString(ipaddr));
    }

    public synchronized void setFirewallAddress (String ipaddr) throws IOException {
      if (ipaddr == null || ipaddr.length() == 0)
        fwAddress = InetAddress.getByName("0.0.0.0");
      else
        fwAddress = InetAddress.getByName(ipaddr);
    }

    public synchronized PoolConfig getPoolConfig () {
      return poolConfig;
    }

    public void setPoolConfig (String xmlString) throws IOException, SAXException, DOMException{
      poolConfig = new PoolConfig(db.parse(new InputSource(new StringReader(xmlString))).getDocumentElement());
    }

    public void setPoolConfig(PoolConfig cfg)
    {
      poolConfig = cfg;
    }

    public synchronized Node getXMLRepresentation () throws DOMException {
      Element root = doc.createElement("fceConfigNew");
      if (!isValid())
        return root;

      if (fwName != null) {
	      Element tag = doc.createElement("firewallName");
	      Text data = doc.createTextNode(fwName);
	      tag.appendChild(data);
	      root.appendChild(tag);
      }

      if (fwAddress != null) {
	Element tag = doc.createElement("firewallAddress");
	Text data = doc.createTextNode(fwAddress.getHostAddress());
	tag.appendChild(data);
	root.appendChild(tag);
      }

      root.appendChild(poolConfig.getXMLRepresentation());
      return root;
    }

    public PoolConfig createNewPoolConfig() {
      poolConfig = new PoolConfig();
      return poolConfig;
    }

    public class PoolConfig {
	private String version;
      private LinkedHashMap poolList;
	private LinkedHashMap netIfList;

      PoolConfig() {
        poolList  = new LinkedHashMap();
	netIfList = new LinkedHashMap();
      }

      PoolConfig (Element element) throws IOException, SAXException, DOMException{
        this();
	version = element.getAttribute("version");
        NodeList list = element.getElementsByTagName("Pool");
        if(list !=  null){
          poolList  = new LinkedHashMap();
        //Logger.debug("[PoolConfig]: node list len = " + list.getLength());
	        for (int i=0; i<list.getLength(); i++) {
	          Pool  pool  = new Pool((Element)list.item(i));
        //Logger.debug("[PoolConfig]: get a pool = " + pool);
        //Logger.debug("[PoolConfig]: pool name = " + pool.getName());
	          poolList.put(pool.getKey(), pool);
	        }
        }

        //Logger.debug("[PoolConfig]: node list= " + list);
        //Logger.debug("[PoolConfig]: pool list= " + poolList);

	list = element.getElementsByTagName("Net-If");
	if (list != null) {
		netIfList = new LinkedHashMap();
		for (int i = 0; i < list.getLength(); i++) {
			NetIf netIf = new NetIf((Element) list.item(i), false);
			netIfList.put(netIf.getKey(), netIf);
		}
	}
	
		list = element.getElementsByTagName("Vnet");
		if (list != null) {
		    netIfList = new LinkedHashMap();
		    for (int i = 0; i < list.getLength(); i++) {
			NetIf netIf = new NetIf((Element) list.item(i), true);
			netIfList.put(netIf.getKey(), netIf);
		    }
		}

      }


      public synchronized Node getXMLRepresentation () throws DOMException {
	      Element root = doc.createElement("PoolCfg");
	if(version != null && version.length() > 0)
		root.setAttribute("version", version);
        Iterator  ie  = poolList.values().iterator();
        while(ie.hasNext()){
          Pool pool = (Pool)ie.next();
          root.appendChild(pool.getXMLRepresentation());
        }
	ie = netIfList.values().iterator();
	while (ie.hasNext()) {
		NetIf netIf = (NetIf) ie.next();
		root.appendChild(netIf.getXMLRepresentation());
	}
        return root;
      }

      public String getXMLString () {
        StringWriter  sw= new StringWriter();
        try{
          Element e = doc.getDocumentElement();
          if(e  != null){
            doc.removeChild((Node)e);
          }
          e = (Element)getXMLRepresentation();
          doc.appendChild(e);
	          XmlDocument xdoc = (XmlDocument) doc;
	          xdoc.write (sw); 
        }catch(Exception ie){
          ie.printStackTrace();
        }

      	Element e = doc.getDocumentElement();
        if(e  != null){
          doc.removeChild((Node)e);
        }

        return sw.toString();
      }
       public LinkedHashMap getDataList() {
	LinkedHashMap data = new LinkedHashMap();
	data.putAll(poolList);
	data.putAll(netIfList);
	return data;
	}

      public LinkedHashMap getPoolList(){
        return poolList;
      }
      public void setPoolList(LinkedHashMap list){
        poolList  = list;
      }
			public LinkedHashMap getNetIfList() {
				return netIfList;
			}

			public void setNetIfList(LinkedHashMap list) {
				netIfList = list;
			}
	public ArrayList getPoolIdList() {
				ArrayList idList = new ArrayList();

				LinkedHashMap map = getPoolList();
				Collection collec = map.values();

				Iterator it = collec.iterator();
				if (it != null) {
					while (it.hasNext()) {
						Object o = it.next();
						Pool pool = (Pool) o;
						//Pool pool = (Pool)it.next();
						if (pool != null)
							idList.add(new Integer(pool.getId()));
					}
				}
				return idList;
			}
	public ArrayList getNetIfNameList() {
				ArrayList nameList = new ArrayList();

				LinkedHashMap map = getNetIfList();
				Collection collec = map.values();

				Iterator it = collec.iterator();
				if (it != null) {
					while (it.hasNext()) {
						Object o = it.next();
						NetIf netIf = (NetIf) o;

						if (netIf != null)
							nameList.add(netIf.getName());
					}
				}
				return nameList;
			}

      public Pool createNewPool(){
        return new Pool();
      }
	public NetIf createNewNetIf(boolean isVnet) {
				return new NetIf(isVnet);
			}
     
      public Pool createNewPool(int id, String name) {
        return new Pool(id, name);
      }
	public NetIf createNewNetIf(
				String name,
				int vlanId,
				String pintf, boolean isVnet) {
				return new NetIf(name, vlanId, pintf, isVnet);
			}
      public void addNewPool(Pool pool)
      {
        poolList.put(pool.getKey(), pool);
      }
	public void addNewNetIf(NetIf netIf) {
				netIfList.put(netIf.getKey(), netIf);
			}
	public Pool getPool(String id) {
				return (Pool) (getPoolList().get(id));
			}
			public class NetIf {
				private String pInterface;
				private String name;
				private int vlanId;
				private LinkedHashMap routeList;
			    private boolean isVnet;

				NetIf(boolean isVnet) {
					this("", 0, "0.0.0.0", isVnet);
				}

				NetIf(String name, int vlanId, String intf, boolean isVnet) {
					this.name = name;
					this.vlanId = vlanId;
					this.pInterface = intf;
					this.isVnet = isVnet;
					routeList = new LinkedHashMap();
				}

				NetIf(Element element, boolean isVnet) throws DOMException {
					this(isVnet);
					if (element != null) {
						name = element.getAttribute("name");
						pInterface = element.getAttribute("interface");
						vlanId =
							Integer.parseInt(element.getAttribute("vlanid"));
						NodeList list = element.getElementsByTagName("Route");
						if (list != null) {
							for (int i = 0; i < list.getLength(); i++) {
								Route route = new Route((Element) list.item(i));
								routeList.put(route.getKey(), route);
							}
						}

					}
				}
				/* add a route to the NetIf */
				public void addRoute(Route route) {
					routeList.put(route.getKey(), route);
				}

				public void removeRoute(Route route) {
					routeList.remove(route.getKey());
				}
				public void setRouteList(LinkedHashMap list) {
					routeList = list;

				}
				public LinkedHashMap getRouteList() {
					return routeList;
				}

				public Route createNewRoute() {
					return new Route();
				}

				public Route createNewRoute(
					String destIp,
					String mask,
					String gw) {
					return new Route(destIp, mask, gw);
				}

				public synchronized Node getXMLRepresentation()
					throws DOMException {
					Element root = doc.createElement(isVnet?"Vnet":"Net-If");
					root.setAttribute("name", name);
					root.setAttribute("vlanid", String.valueOf(vlanId));
					root.setAttribute("interface", pInterface);
					Iterator ie = routeList.values().iterator();
					while (ie.hasNext()) {
						Route route = (Route) ie.next();
						root.appendChild(route.getXMLRepresentation());
					}
					return root;
				}

				public synchronized boolean equals(Object o) {
					if (o.getClass().equals(NetIf.class)) {
						NetIf given = (NetIf) o;
						if (((pInterface == null
							&& given.getInterface() == null)
							|| pInterface.equals(given.getInterface()))
							&& ((name == null && given.getName() == null)
								|| name.equals(given.getName()))
							&& vlanId == given.getVlanId())
							return true;
					}
					return false;

				}

				public synchronized String getKey() {
					return pInterface + ":" + name + ":" + vlanId;
				}

				public synchronized String getInterface() {
					return pInterface;
				}

				public void setInterface(String pInterface) {
					this.pInterface = pInterface;
				}

				public int getVlanId() {
					return vlanId;
				}

				public void setVlanId(int vlanId) {
					this.vlanId = vlanId;
				}

				public String getName() {
					return name;
				}

				public void setName(String name) {
					this.name = name;
				}

				public class Route {
					private String gw;
					private String destIp;
					private String mask;

					Route() {
						this("0.0.0.0", "255.255.255.0", "0.0.0.0");
					}

					Route(String destIp, String mask, String gw) {
						this.gw = gw;
						this.destIp = destIp;
						this.mask = mask;
					}

					Route(Element element) throws DOMException {
						this();
						if (element != null) {
							gw = element.getAttribute("gw");
							destIp = element.getAttribute("dest_ip");
							mask = element.getAttribute("mask");
						}
					}

					public synchronized Node getXMLRepresentation()
						throws DOMException {
						Element root = doc.createElement("Route");
						root.setAttribute("dest_ip", destIp);
						root.setAttribute("mask", mask);
						root.setAttribute("gw", gw);

						return root;
					}

					public synchronized boolean equals(Object o) {
						if (o.getClass().equals(Route.class)) {
							Route given = (Route) o;
							if (((gw == null && given.getGw() == null)
								|| gw.equals(given.getGw()))
								&& ((destIp == null && given.getDestIp() == null)
									|| destIp.equals(given.getDestIp())))
								return true;
						}
						return false;

					}

					public synchronized String getKey() {
						return destIp + ":" + mask + ":" + gw;
					}

					public synchronized String getDestIp() {
						return destIp;
					}

					public void setDestIp(String destIp) {
						this.destIp = destIp;
					}

					public String getGw() {
						return gw;
					}

					public void setGw(String gw) {
						this.gw = gw;
					}

					public String getMask() {
						return mask;
					}

					public void setMask(String mask) {
						this.mask = mask;
					}
				}

			}
      public class Pool{
        private int id;
        private String name;
        private LinkedHashMap portList;

        Pool () {
          this(0, "");
        }
    
        Pool(Element element) throws DOMException {
      	  this();

          if(element  !=  null){
            id = Integer.parseInt(element.getAttribute("id"));
            name  = element.getAttribute("name");
            NodeList list = element.getElementsByTagName("PortAlloc");
            if(list !=  null){
	            for (int i=0; i<list.getLength(); i++) {
		            PortAlloc portAlloc = new PortAlloc((Element)list.item(i));
		            portList.put(portAlloc.getKey(), portAlloc);
              }
            }
          }
        }

        /* create an empty pool */
        Pool (int id, String name)
        {
           this.id = id; 
           this.name = name;
           portList = new LinkedHashMap();
        }

        /* add a port to this pool */
        public void addPortAlloc(PortAlloc palloc)
        {
          portList.put(palloc.getKey(), palloc); 
        }

        public void removePortAlloc(PortAlloc palloc)
        {
          portList.remove(palloc.getKey());
        }
 
        public synchronized Node getXMLRepresentation () throws DOMException {
	        Element root = doc.createElement("Pool");
          root.setAttribute("id", String.valueOf(id));
          root.setAttribute("name", name);

          Iterator ie = portList.values().iterator();
          while(ie.hasNext()) {
            PortAlloc portAlloc = (PortAlloc)ie.next();
            root.appendChild(portAlloc.getXMLRepresentation());
          }

	        return root;
	      }

	      public boolean equals (Object o) {
	        if (o.getClass().equals(Pool.class)) {
	          Pool given = (Pool)o;
	          if(id== given.getId() &&
		          ((name != null && given.getName() != null) ||
		            name.equals(given.getName())) &&
		          ((portList != null && given.getPortAllocList() != null) ||
		            portList.equals(given.getPortAllocList())))
	            return true;
	        }
	        return false;
	      }

        public String getKey(){
          return String.valueOf(id);
        }

        public void setId(int id){
          this.id  = id;
        }

        public int getId(){
          return id;
        }

        public void setName(String name){
          this.name  = name;
        }

        public String getName(){
          return name;
        }


        public  void setPortAllocList (LinkedHashMap list) {
          portList  = list;

        }
        public LinkedHashMap getPortAllocList () {
          return portList;
        }

        public PortAlloc createNewPortAlloc (boolean isVnet) {
          return new PortAlloc(isVnet);
        }

//        public PortAlloc createNewPortAlloc (String intef, String sig, String addr, String mask, 
        public PortAlloc createNewPortAlloc (String intef, boolean sig, String addr, String mask, 
                     int low, int high, boolean isVnet) 
        {
          return new PortAlloc(intef, sig, addr, mask, low, high, isVnet);
        }

  	  public class PortAlloc {
            private String pInterface;
            //private String signaling;
            private boolean signaling;
            //private InetAddress address;
            private String address;
            private String mask;
            private int low;
            private int high;
	      private boolean isVnet;

          PortAlloc(boolean isVnet) {
            this("", false, "0.0.0.0", "255.255.255.0", 0, 0, isVnet); 
          }
         
//          PortAlloc(String intf, String sig, String addr, String mask, int low, int high) 
          PortAlloc(String intf, boolean sig, String addr, String mask, int low, int high, boolean isVnet) 
          {
	    this.low   = low;
            this.high  = high;
            pInterface = intf;
            signaling  = sig;
            this.mask = mask;
	    address  = addr;
	    this.isVnet = isVnet;
/*
	    try{
	       address  = InetAddress.getByName(addr);
	    }catch(UnknownHostException ue)
            {
              Logger.error("[PortAlloc]:error" + ue); 
	    }
*/
          }
  
          PortAlloc (Element element) throws DOMException {
	    this(false);
            if (element  !=  null) {

		isVnet = false;
		pInterface =  element.getAttribute(isVnet?"vnet":"net-if");
		isVnet = true;
		pInterface =  element.getAttribute(isVnet?"vnet":"net-if");

		if(pInterface.length() == 0)
		    pInterface = element.getAttribute("interface");
              
               signaling  =  new Boolean(element.getAttribute("signaling")).booleanValue();
//               signaling  = element.getAttribute("signaling");

/*
	       try {
	          address = InetAddress.getByName(element.getAttribute("address"));
	       } catch (IOException ie) {
	          throw new DOMException(DOMException.NOT_FOUND_ERR, "Error parsing address: " 
                  + ie.toString());
               }
*/

	       address = element.getAttribute("address");
	       mask = element.getAttribute("mask");
               if (!signaling) {
                 low  = Integer.parseInt(element.getAttribute("low"));
                 high = Integer.parseInt(element.getAttribute("high"));
               }
               //Logger.debug("[PortAlloc]: elements:");
               //Logger.debug("[PortAlloc]: interface=" + pInterface + ",signaling=" + 
                   //signaling + ", address=" + address + ", mask=" + mask + 
                   //", low=" + low + ", high=" + high);
            }
          }

		private boolean isHotknifeInterface(String s) {
			boolean val = false;
						
			for(int i = 0; i < CommonConstants.HOTKNIFE_INTERFACES.length; i++) {
				if(s.equals(CommonConstants.HOTKNIFE_INTERFACES[i])) {
					val = true;
					break;
				}
			}
			return val;
		}
		
          public synchronized Node getXMLRepresentation () throws DOMException {
      	    Element root = doc.createElement("PortAlloc");
			if(fwName.equals(NAME_HOTKNIFE) &&
				(!signaling) &&
				(!isHotknifeInterface(pInterface)))								
				root.setAttribute(isVnet?"vnet":"net-if",pInterface);
			else
				root.setAttribute("interface", pInterface);
           
            root.setAttribute("signaling", signaling?"true":"false");
            root.setAttribute("address", address);
            root.setAttribute("mask", mask);
            if (!signaling) {
              root.setAttribute("low", String.valueOf(low));
              root.setAttribute("high", String.valueOf(high));
            }
            return root;
          }

    	    public synchronized boolean equals (Object o) {
	          if (o.getClass().equals(PortAlloc.class)) {
	            PortAlloc given = (PortAlloc)o;
              if( ((pInterface == null && given.getInterface() == null) ||
		              pInterface.equals(given.getInterface())) &&
                  ((address == null && given.getAddress() == null) ||
		              address.equals(given.getAddress())) &&
                  low   ==  given.getLow()  &&
                  high  ==  given.getHigh() 
                  )
                  return true;
            }
            return false;

          }
  
          public synchronized String getKey(){
            return pInterface + ":" + address +":" + mask;
          }

          public synchronized String getInterface(){
            return pInterface;
          }

          public void setInterface(String pInterface){
            this.pInterface  = pInterface;
          }

          public int getLow(){
            return low;
          }

          public void setLow(int low){
            this.low  = low;
          }

          public int getHigh(){
            return high;
          }

          public void setHigh(int high){
            this.high  = high;
          }

          //public InetAddress getAddress(){
          public String getAddress(){
            return address;
          }

//          public void setAddress(InetAddress address){
          public void setAddress(String address){
            this.address  = address;
          }

          public String getMask(){
            return mask;
          }

          public void setMask(String mask){
            this.mask  = mask;
          }

/*          public String getSignaling(){
            return signaling;
          }
*/
          public boolean isSignaling(){
            return signaling;
          }

//          public void setSignaling(String signaling){
          public void setSignaling(boolean signaling){
            this.signaling  = signaling;
          }
        }
      }
    }
  }

  public class BillingConfig {
    private boolean configValid;

    private int cdrType;
    private int cdrFormat;
    private String dir;
    private int cdrTimer;
    private int billingType;

    private boolean cdrLogStart1, cdrLogStart2, cdrLogEnd2, cdrLogHunt;

    public boolean equals (BillingConfig bc) {
      if (cdrType == bc.getCdrType() &&
	        cdrFormat == bc.getCdrFormat() &&
	        ((dir == null && bc.getDirectory() == null) ||
	         dir.equals(bc.getDirectory())) &&
	        (cdrType != CDRNEXTONETIME ||
	         cdrTimer == bc.getCdrTimer()) &&
	        billingType == bc.getBillingType() &&
          cdrLogStart1 == bc.getCdrLogStart1() &&
          cdrLogStart2 == bc.getCdrLogStart2() &&
          cdrLogEnd2 == bc.getCdrLogEnd2() &&
          cdrLogHunt == bc.getCdrLogHunt())
      	return true;

      return false;
    }

    BillingConfig () {
      setValid(true);
    }

    BillingConfig (Element element) throws DOMException {
      this();

      NodeList list = element.getElementsByTagName("billingType");
      if (list != null && list.getLength() == 1) {
	      Node childNode = list.item(0).getLastChild();
	      if (childNode != null)
	        billingType = iServerConfig.getBillingType(childNode.getNodeValue());
      }

      list = element.getElementsByTagName("cdr");
      if (list != null && list.getLength() == 1) {
	      Element cdrElem = (Element)list.item(0);
	      cdrType = iServerConfig.getCdrType(cdrElem.getAttribute("type"));
	      cdrFormat = iServerConfig.getCdrFormat(cdrElem.getAttribute("format"));
	      list = cdrElem.getElementsByTagName("dir");
	      if (list != null && list.getLength() == 1) {
	        Node childNode = list.item(0).getLastChild();
	        if (childNode != null)
	          dir = childNode.getNodeValue();
	      }
	      list = cdrElem.getElementsByTagName("cdrTimer");
	      if (list != null && list.getLength() == 1) {
	        Node childNode = list.item(0).getLastChild();
	        if (childNode != null)
	          cdrTimer = Integer.parseInt(childNode.getNodeValue());
	      }
        cdrLogStart1  = new Boolean(cdrElem.getAttribute("start1")).booleanValue();
        cdrLogStart2  = new Boolean(cdrElem.getAttribute("start2")).booleanValue();
        cdrLogEnd2  = new Boolean(cdrElem.getAttribute("end2")).booleanValue();
        cdrLogHunt  = new Boolean(cdrElem.getAttribute("hunt")).booleanValue();
      }
    }

    public void setValid (boolean val) {
      configValid = val;
    }

    public boolean isValid () {
      return configValid;
    }

    public synchronized int getCdrType () {
      return cdrType;
    }

    public synchronized void setCdrType (int type) {
      cdrType = type;
    }

    public synchronized int getCdrFormat () {
      return cdrFormat;
    }

    public synchronized void setCdrFormat (int type) {
      cdrFormat = type;
    }

    public synchronized String getDirectory () {
      return dir;
    }

    public synchronized void setDirectory (String dir) {
      this.dir = dir;
    }

    public synchronized int getCdrTimer () {
      return cdrTimer;
    }

    public synchronized void setCdrTimer (int time) {
      cdrTimer = time;
    }

    public synchronized int getBillingType () {
      return billingType;
    }

    public synchronized void setBillingType (int type) {
      billingType = type;
    }

    public synchronized boolean getCdrLogStart1 () {
      return cdrLogStart1;
    }

    public synchronized void setCdrLogStart1 (boolean val) {
      cdrLogStart1 = val;
    }

    public synchronized boolean getCdrLogStart2 () {
      return cdrLogStart2;
    }

    public synchronized void setCdrLogStart2 (boolean val) {
      cdrLogStart2 = val;
    }

    public synchronized boolean getCdrLogEnd2 () {
      return cdrLogEnd2;
    }

    public synchronized void setCdrLogEnd2 (boolean val) {
      cdrLogEnd2 = val;
    }

    public synchronized boolean getCdrLogHunt () {
      return cdrLogHunt;
    }

    public synchronized void setCdrLogHunt (boolean val) {
      cdrLogHunt = val;
    }

    public synchronized Node getXMLRepresentation () throws DOMException {
      Element root = doc.createElement("billingConfig");
      if (!isValid())
        return root;

      Element tag = doc.createElement("billingType");
      Text data = doc.createTextNode(iServerConfig.getBillingTypeString(billingType));
      tag.appendChild(data);
      root.appendChild(tag);

      tag = doc.createElement("cdr");
      tag.setAttribute("type", iServerConfig.getCdrTypeString(cdrType));
      tag.setAttribute("format", iServerConfig.getCdrFormatString(cdrFormat));
      tag.setAttribute("start1", cdrLogStart1?"TRUE":"FALSE");
      tag.setAttribute("start2", cdrLogStart2?"TRUE":"FALSE");
      tag.setAttribute("end2", cdrLogEnd2?"TRUE":"FALSE");
      tag.setAttribute("hunt", cdrLogHunt?"TRUE":"FALSE");
      if (dir != null && dir.length() > 0) {
	      Element tag2 = doc.createElement("dir");
	      data = doc.createTextNode(dir);
	      tag2.appendChild(data);
	      tag.appendChild(tag2);
      }
      Element tag2 = doc.createElement("cdrTimer");
      data = doc.createTextNode(String.valueOf(cdrTimer));
      tag2.appendChild(data);
      tag.appendChild(tag2);
      root.appendChild(tag);

      return root;
    }


  }

  public class RadiusConfig{
    private boolean configValid;
    private String[] servers;
    private String[] secrets;
    private String dirName;
    private int timeout;
    private int retry;
    private int deadTime;
    private boolean sendMsg;
    private boolean useOSIF;

    public boolean equals (RadiusConfig rc) {
      boolean ret = true;

      String[] rServers = rc.getServers();
      String[] rSecrets = rc.getSecrets();

      for(int i=0; i < MAX_NUM_RAD_ENTRIES; i++){
        if( !((servers[i] ==  null  &&  rServers[i] ==  null )  ||
             servers[i].equals(rServers[i])
             )){
          ret = false;
          break;
        }
        if( !((secrets[i] ==  null  &&  rSecrets[i] ==  null )  ||
             secrets[i].equals(rSecrets[i])
             )){
          ret = false;
          break;
        }
      }

      if ( ret &&
	        timeout == rc.getTimeout() &&
          retry   == rc.getRetry() &&
          deadTime == rc.getDeadTime() && 
          dirName   ==  rc.getDirName() &&
          sendMsg == rc.getSendAccountingMsg() &&
          useOSIF == rc.getUseOSIF()

          )
      	return true;

      return false;
    }

    RadiusConfig () {
      setValid(true);
      servers = new String[MAX_NUM_RAD_ENTRIES];
      secrets = new String[MAX_NUM_RAD_ENTRIES];

      for(int i=0; i < MAX_NUM_RAD_ENTRIES; i++) {
        servers[i]  = "";
        secrets[i]  = "";
      }
      timeout = 0;
      retry = 0;
      deadTime  = 0;
      dirName = "";
      sendMsg = false;
      useOSIF = false;
    }

    RadiusConfig (Element element) throws DOMException {
      this();

      NodeList list = element.getElementsByTagName("server");
      if (list != null){
        for(int i=0; i < list.getLength(); i++) {
	        Node childNode = list.item(i).getLastChild();
	        if (childNode != null)
	          servers[i] = childNode.getNodeValue();
        }
      }
      list = element.getElementsByTagName("secret");
      if (list != null){
        for(int i=0; i < list.getLength(); i++) {
	        Node childNode = list.item(i).getLastChild();
	        if (childNode != null)
	          secrets[i] = childNode.getNodeValue();
        }
      }

	    list = element.getElementsByTagName("timeout");
	    if (list != null && list.getLength() == 1) {
	      Node childNode = list.item(0).getLastChild();
        if (childNode != null){
	        try {
		        timeout = Integer.parseInt(childNode.getNodeValue());
	        } catch (Exception e) {}
        }
      }
	    list = element.getElementsByTagName("retry");
	    if (list != null && list.getLength() == 1) {
	      Node childNode = list.item(0).getLastChild();
        if (childNode != null){
	        try {
		        retry = Integer.parseInt(childNode.getNodeValue());
	        } catch (Exception e) {}
        }
      }
	    list = element.getElementsByTagName("deadTime");
	    if (list != null && list.getLength() == 1) {
	      Node childNode = list.item(0).getLastChild();
        if (childNode != null){
	        try {
		        deadTime = Integer.parseInt(childNode.getNodeValue());
	        } catch (Exception e) {}
        }
      }

	    list = element.getElementsByTagName("dirName");
	    if (list != null && list.getLength() == 1) {
	      Node childNode = list.item(0).getLastChild();
        if (childNode != null){
	        try {
		        dirName = childNode.getNodeValue();
	        } catch (Exception e) {}
        }
      }

      list = element.getElementsByTagName("sendMsg");
      if (list != null && list.getLength() == 1) {
        Node childNode = list.item(0).getLastChild();
        if (childNode != null) {
          try {
            sendMsg = new Boolean(childNode.getNodeValue()).booleanValue();
          } catch (Exception e) {
            System.out.println("Error parse send message: " + e);
          }
        }
      }

      list = element.getElementsByTagName("useOSIF");
      if (list != null && list.getLength() == 1) {
        Node childNode = list.item(0).getLastChild();
        if (childNode != null) {
          try {
            useOSIF = new Boolean(childNode.getNodeValue()).booleanValue();
          } catch (Exception e) {
            System.out.println("Error parse use OSIF: " + e);
          }
        }
      }

    }

    public void setSendAccountingMsg(boolean smsg)
    {
      sendMsg = smsg;
    }

    public boolean getSendAccountingMsg()
    {
      return sendMsg;
    }

    public void setUseOSIF(boolean uosif)
    {
      useOSIF = uosif;
    }

    public boolean getUseOSIF()
    {
      return useOSIF;
    }

    public void setValid (boolean val) {
      configValid = val;
    }

    public boolean isValid () {
      return configValid;
    }

    public String[] getServers(){
      return servers;
    }

    public void setServers(String[] servers){
      this.servers  = servers;
    }

    public String[] getSecrets(){
      return secrets;
    }

    public void setSecrets(String[] secrets){
      this.secrets  = secrets;
    }

    public void setTimeout(int timeout){
      this.timeout  = timeout;
    }

    public int getTimeout(){
      return timeout;
    }

    public void setRetry(int retry){
      this.retry  = retry;
    }

    public int getRetry(){
      return retry;
    }

    public void setDeadTime(int deadTime){
      this.deadTime = deadTime;
    }

    public int getDeadTime(){
      return deadTime;
    }

    public void setDirName(String dirName){
      this.dirName = dirName;
    }

    public String getDirName(){
      return dirName;
    }

    

    public synchronized Node getXMLRepresentation () throws DOMException {
      Element root = doc.createElement("radiusConfig");
      if (!isValid())
        return root;

      for(int i=0; i < MAX_NUM_RAD_ENTRIES; i++) {
        Element tag = doc.createElement("server");
        Text data = doc.createTextNode(servers[i]);
        tag.appendChild(data);
        root.appendChild(tag);
        tag = doc.createElement("secret");
        data = doc.createTextNode(secrets[i]);
        tag.appendChild(data);
        root.appendChild(tag);
      }

      Element tag = doc.createElement("timeout");
      Text data = doc.createTextNode(String.valueOf(timeout));
      tag.appendChild(data);
      root.appendChild(tag);

      tag = doc.createElement("retry");
      data = doc.createTextNode(String.valueOf(retry));
      tag.appendChild(data);
      root.appendChild(tag);

      tag = doc.createElement("deadTime");
      data = doc.createTextNode(String.valueOf(deadTime));
      tag.appendChild(data);
      root.appendChild(tag);

      tag = doc.createElement("dirName");
      data = doc.createTextNode(dirName);
      tag.appendChild(data);
      root.appendChild(tag);

      tag = doc.createElement("sendMsg");
      data = doc.createTextNode(String.valueOf(sendMsg));
      tag.appendChild(data);
      root.appendChild(tag);
      
      tag = doc.createElement("useOSIF");
      data = doc.createTextNode(String.valueOf(useOSIF));
      tag.appendChild(data);
      root.appendChild(tag);

      return root;
    }
  }

  public class AdvancedConfig {
    private boolean configValid;

    private int numSegments;
    private int segSize;
    private int priority;
    private int numThreads;
    private int threadStackSize;

    public boolean equals (AdvancedConfig ac) {
      if (numSegments == ac.getNumSegments() &&
	  segSize == ac.getSegmentSize() &&
	  priority == ac.getPriority() &&
	  numThreads == ac.getNumThreads() &&
	  threadStackSize == ac.getThreadStackSize())
	return true;

      return false;
    }

    AdvancedConfig () {
      setValid(true);
    }

    AdvancedConfig (Element element) throws DOMException {
      this();

      NodeList list = element.getElementsByTagName("numSegments");
      if (list != null && list.getLength() == 1) {
	Node childNode = list.item(0).getLastChild();
	if (childNode != null && childNode.getNodeValue() != null)
	  numSegments = Integer.parseInt(childNode.getNodeValue());
      }

      list = element.getElementsByTagName("segSize");
      if (list != null && list.getLength() == 1) {
	Node childNode = list.item(0).getLastChild();
	if (childNode != null && childNode.getNodeValue() != null)
	  segSize = Integer.parseInt(childNode.getNodeValue());
      }

      list = element.getElementsByTagName("priority");
      if (list != null && list.getLength() == 1) {
	Node childNode = list.item(0).getLastChild();
	if (childNode != null && childNode.getNodeValue() != null)
	  priority = Integer.parseInt(childNode.getNodeValue());
      }

      list = element.getElementsByTagName("threads");
      if (list != null && list.getLength() == 1) {
	Element threadElem = (Element)list.item(0);
	String stack = threadElem.getAttribute("stack");
	if (stack != null)
	  threadStackSize = Integer.parseInt(stack);

	Node childNode = threadElem.getLastChild();
	if (childNode != null && childNode.getNodeValue() != null)
	  numThreads = Integer.parseInt(childNode.getNodeValue());
      }
    }

    public void setValid (boolean val) {
      configValid = val;
    }

    public boolean isValid () {
      return configValid;
    }

    public synchronized int getNumSegments () {
      return numSegments;
    }

    public synchronized void setNumSegments (int num) {
      numSegments = num;
    }

    public synchronized int getSegmentSize () {
      return segSize;
    }

    public synchronized void setSegmentSize (int size) {
      segSize = size;
    }

    public synchronized int getPriority () {
      return priority;
    }

    public synchronized void setPriority (int prio) {
      priority = prio;
    }

    public synchronized int getNumThreads () {
      return numThreads;
    }

    public synchronized void setNumThreads (int num) {
      numThreads = num;
    }

    public synchronized int getThreadStackSize () {
      return threadStackSize;
    }

    public synchronized void setThreadStackSize (int num) {
      threadStackSize = num;
    }


    public synchronized Node getXMLRepresentation () throws DOMException {
      Element root = doc.createElement("advancedConfig");
      if (!isValid())
        return root;

      Element tag;
      Text data;
      if (numSegments > 0) {
	tag = doc.createElement("numSegments");
	data = doc.createTextNode(String.valueOf(numSegments));
	tag.appendChild(data);
	root.appendChild(tag);
      }
      if (segSize > 0) {
	tag = doc.createElement("segSize");
	data = doc.createTextNode(String.valueOf(segSize));
	tag.appendChild(data);
	root.appendChild(tag);
      }
      tag = doc.createElement("priority");
      data = doc.createTextNode(String.valueOf(priority));
      tag.appendChild(data);
      root.appendChild(tag);
      tag = doc.createElement("threads");
      if (threadStackSize != 0)
	tag.setAttribute("stack", String.valueOf(threadStackSize));
      data = doc.createTextNode(String.valueOf(numThreads));
      tag.appendChild(data);
      root.appendChild(tag);

      return root;
    }
  }

  public class SystemConfig {
    private boolean configValid;

    private int g711Ulaw64Duration;
    private int g711Alaw64Duration;
    private int g729Frames;
    private int g7231Frames;
    private int defaultCodec;
    private String enumDomain;
    private int rolloverTime;
    private String radiusServer;
    private String radiusSecret;
    private boolean allowAllSrc;
    private boolean allowAllDst;
    private boolean allowHairpinCalls = true;
    private boolean defaultMediaRouting = true;
    private boolean defaultMidCallMediaChange;
    private int maxHunts, maxHuntsLimit, maxHuntsAllowableDuration;
    private boolean allowAllRtp;
    private boolean forwardSrcAddr;
    private boolean removeRFC2833;
    private boolean removeT38;
    private int cacheTimeout;
    private int maxCallDuration;
    private String mswName;
    private String mgmtIp;
	private boolean mapIsdncc;
	private boolean mapLrjReason;
	private String useCodeMap;

    public boolean equals (SystemConfig sc) {
      if (g711Ulaw64Duration == sc.getG711Ulaw64Duration() &&
	  g711Alaw64Duration == sc.getG711Alaw64Duration() &&
	  g729Frames == sc.getG729Frames() &&
	  g7231Frames == sc.getG7231Frames() &&
    defaultCodec  ==  sc.getDefaultCodec()  &&
	  ((enumDomain == null && sc.getEnumDomain() == null) ||
	   enumDomain.equals(sc.getEnumDomain())) &&
	  rolloverTime == sc.getRolloverTime() &&
	  ((radiusServer == null && sc.getRadiusServer() == null) ||
	   radiusServer.equals(sc.getRadiusServer())) &&
	  ((radiusSecret == null && sc.getRadiusSecret() == null) ||
	   radiusSecret.equals(sc.getRadiusSecret())) &&
	  allowAllSrc == sc.isAllSrcAllowed() &&
	  allowAllDst == sc.isAllDstAllowed() &&
	  allowHairpinCalls == sc.isHairpinCallsAllowed() &&
	  defaultMediaRouting == sc.isMediaRoutingEnabled() &&
	  defaultMidCallMediaChange == sc.isMidCallMediaChangeEnabled() &&
	  maxHunts == sc.getMaxHunts() &&
          allowAllRtp == sc.isAllRtpAllowed() &&
          forwardSrcAddr == sc.isSrcAddrForwarded() &&
          removeRFC2833 == sc.isRemoveRFC2833() &&
          removeT38 == sc.isRemoveT38() &&
          cacheTimeout == sc.getCacheTimeout() &&
          maxCallDuration == sc.getMaxCallDuration() &&
	  maxHuntsLimit == sc.getMaxHuntsLimit()  &&
          maxHuntsAllowableDuration == sc.getMaxHuntsAllowableDuration() &&
	      ((mswName == null && sc.getMswName() == null) || mswName.equals(sc.getMswName())) &&
	      ((mgmtIp == null && sc.getMgmtIp() == null) || mgmtIp.equals(sc.getMgmtIp()))

	  && mapIsdncc == sc.isMapIsdncc()
	  && mapLrjReason == sc.isMapLrjReason()
	  && ((useCodeMap == null && sc.getUseCodeMap() == null)
	      || useCodeMap.equals(sc.getUseCodeMap()))
	  
	  )
	  return true;

      return false;
    }

    SystemConfig () {
      setValid(true);
      defaultCodec  = -1;
    }

    SystemConfig (Element element) throws DOMException {
      this();

      NodeList list = element.getElementsByTagName("codec");
      if (list != null && list.getLength() == 1) {
	Element codecElem = (Element)list.item(0);
	try {
	  String num = codecElem.getAttribute("g711Ulaw64Duration");
	  if (num != null && num.length() > 0)
	    g711Ulaw64Duration = Integer.parseInt(num);
	  num = codecElem.getAttribute("g711Alaw64Duration");
	  if (num != null && num.length() > 0)
	    g711Alaw64Duration = Integer.parseInt(num);
	  num = codecElem.getAttribute("g729Frames");
	  if (num != null && num.length() > 0)
	    g729Frames = Integer.parseInt(num);
	  num = codecElem.getAttribute("g7231Frames");
	  if (num != null && num.length() > 0)
	    g7231Frames = Integer.parseInt(num);
    num = codecElem.getAttribute("default");
	  if (num != null && num.length() > 0)
	    defaultCodec = Integer.parseInt(num);

	} catch (NumberFormatException ne) {
	  throw new DOMException(DOMException.NOT_FOUND_ERR, ne.toString());
	}
      }

      list = element.getElementsByTagName("enumDomain");
      if (list != null && list.getLength() == 1) {
	Node childNode = list.item(0).getLastChild();
	if (childNode != null)
	  enumDomain = childNode.getNodeValue();
      }

      list = element.getElementsByTagName("rolloverTime");
      if (list != null && list.getLength() == 1) {
	Node childNode = list.item(0).getLastChild();
	if (childNode != null && childNode.getNodeValue() != null)
	  rolloverTime = Integer.parseInt(childNode.getNodeValue());
      }

      list = element.getElementsByTagName("cacheTimeout");
      if (list != null && list.getLength() == 1) {
        Node childNode = list.item(0).getLastChild();
        if (childNode != null && childNode.getNodeValue() != null) {
          try {
            cacheTimeout = Integer.parseInt(childNode.getNodeValue());
          } catch (NumberFormatException ne) {}
        }
      }

      list = element.getElementsByTagName("maxCallDuration");
      if (list != null && list.getLength() == 1) {
        Node childNode = list.item(0).getLastChild();
        if (childNode != null && childNode.getNodeValue() != null) {
          try {
            maxCallDuration = Integer.parseInt(childNode.getNodeValue());
          } catch (NumberFormatException ne) {}
        }
      }

      list = element.getElementsByTagName("radius");
      if (list != null && list.getLength() == 1) {
 	Element radiusElem = (Element)list.item(0);
	list = radiusElem.getElementsByTagName("server");
 	if (list != null && list.getLength() == 1) {
 	  Node childNode = list.item(0).getLastChild();
 	  if (childNode != null)
 	    radiusServer = childNode.getNodeValue();
 	}
 
 	list = radiusElem.getElementsByTagName("secret");
 	if (list != null && list.getLength() == 1) {
 	  Node childNode = list.item(0).getLastChild();
 	  if (childNode != null && childNode.getNodeValue() != null) {
 	    try {
 	      radiusSecret = WeakEncryptionInputStream.getDecryptedString(childNode.getNodeValue());
 	    } catch (IOException ie) {
 	      throw new DOMException(DOMException.NOT_FOUND_ERR, "Unable to read RADIUS secret: " + ie.toString());
 	    }
 	  }
 	}
      }      
      
      list = element.getElementsByTagName("accessControl");
      if (list != null && list.getLength() == 1) {
	Element acElem = (Element)list.item(0);
	allowAllSrc = Boolean.valueOf(acElem.getAttribute("allowAllSrc")).booleanValue();
	allowAllDst = Boolean.valueOf(acElem.getAttribute("allowAllDst")).booleanValue();
	allowHairpinCalls = Boolean.valueOf(acElem.getAttribute("allowHairpinCalls")).booleanValue();
	defaultMediaRouting = Boolean.valueOf(acElem.getAttribute("defaultMediaRouting")).booleanValue();
	defaultMidCallMediaChange = Boolean.valueOf(acElem.getAttribute("defaultMidCallMediaChange")).booleanValue();
        allowAllRtp = Boolean.valueOf(acElem.getAttribute("allowAllRtp")).booleanValue();
        forwardSrcAddr = Boolean.valueOf(acElem.getAttribute("forwardSrcAddr")).booleanValue();
        removeRFC2833 = Boolean.valueOf(acElem.getAttribute("removeRFC2833")).booleanValue();
        removeT38 = Boolean.valueOf(acElem.getAttribute("removeT38")).booleanValue();

	list = acElem.getElementsByTagName("maxHunts");
	if (list != null && list.getLength() == 1) {
	  Element mhElem = (Element)list.item(0);
	  maxHuntsLimit = Integer.parseInt(mhElem.getAttribute("limit"));
          String duration = mhElem.getAttribute("allowableDuration");
          if (duration != null && duration.length() != 0)
            maxHuntsAllowableDuration = Integer.parseInt(duration);
	  Node childNode = mhElem.getLastChild();
	  if (childNode != null && childNode.getNodeValue() != null)
	    maxHunts = Integer.parseInt(childNode.getNodeValue());
	}
      }

      list = element.getElementsByTagName("mswName");
      if (list != null && list.getLength() == 1) {
        Node childNode = list.item(0).getLastChild();
        if (childNode != null && childNode.getNodeValue() != null) {
            mswName = childNode.getNodeValue();
        }
      }

      list = element.getElementsByTagName("mgmtIp");
      if (list != null && list.getLength() == 1) {
        Node childNode = list.item(0).getLastChild();
        if (childNode != null && childNode.getNodeValue() != null) {
            mgmtIp = childNode.getNodeValue();
        }
      }

	    list = element.getElementsByTagName("reasonCode");
	    if (list != null && list.getLength() == 1) {
		Element childNode = (Element)list.item(0);
		if (childNode != null){
		    useCodeMap = childNode.getAttribute("useCodeMap");
		    mapIsdncc = Boolean.valueOf(childNode.getAttribute("mapIsdncc")).booleanValue();
		    mapLrjReason = Boolean.valueOf(childNode.getAttribute("mapLrjReason")).booleanValue();
		}
		//Logger.debug("============reasonCode: useCodeMap="+useCodeMap+", mapIsdncc="+mapIsdncc+", mapLrjReason="+mapLrjReason);
	    }

    }

    
    public void setValid (boolean val) {
      configValid = val;
    }

    public boolean isValid () {
      return configValid;
    }

    public synchronized int getG711Ulaw64Duration () {
      return g711Ulaw64Duration;
    }

    public synchronized void setG711Ulaw64Duration (int dur) {
      g711Ulaw64Duration = dur;
    }

    public synchronized int getG711Alaw64Duration () {
      return g711Alaw64Duration;
    }

    public synchronized void setG711Alaw64Duration (int dur) {
      g711Alaw64Duration = dur;
    }

    public synchronized int getG729Frames () {
      return g729Frames;
    }

    public synchronized void setG729Frames (int frames) {
      g729Frames = frames;
    }

    public synchronized int getG7231Frames () {
      return g7231Frames;
    }

    public synchronized void setG7231Frames (int frames) {
      g7231Frames = frames;
    }

    public synchronized int getDefaultCodec() {
      return defaultCodec;
    }

    public synchronized void setDefaultCodec(int defaultVal) {
      this.defaultCodec = defaultVal;
    }

    public synchronized void setDefaultCodec(String defaultVal) {
      defaultCodec  = defaultCodecInt[getDefaultCodecStringIndex(defaultVal)];
    }

    public synchronized int getDefaultCodecStringIndex(String defaultVal) {
      for(int i=0; i < defaultCodecString.length; i++){
        if(defaultCodecString[i].equals(defaultVal))
          return i;
      }
      return 0;
    }

    public synchronized int getDefaultCodecIntIndex(){
      return getDefaultCodecIntIndex(defaultCodec);
    }

    public synchronized int getDefaultCodecIntIndex(int defaultVal){
      for(int i=0; i < defaultCodecInt.length; i++){
        if(defaultVal==  defaultCodecInt[i])
          return i;
      }
      return 0;
    }

    public synchronized String getEnumDomain () {
      return enumDomain;
    }

    public synchronized void setEnumDomain (String domain) {
      enumDomain = domain;
    }

    public synchronized int getRolloverTime () {
      return rolloverTime;
    }

    public synchronized void setRolloverTime (int time) {
      rolloverTime = time;
    }
    public synchronized String getRadiusServer () {
      return radiusServer;
    }
 
    public synchronized void setRadiusServer (String server) {
      radiusServer = server;
    }
 
    public synchronized String getRadiusSecret () {
      return radiusSecret;
    }
    public synchronized void setRadiusSecret (String secret) {
      radiusSecret = secret;
    }

    public synchronized boolean isAllSrcAllowed () {
      return allowAllSrc;
    }

    public synchronized void setAllSrcAllowed (boolean flag) {
      allowAllSrc = flag;
    }

    public synchronized boolean isAllDstAllowed () {
      return allowAllDst;
    }

    public synchronized void setAllDstAllowed (boolean flag) {
      allowAllDst = flag;
    }

    public synchronized boolean isHairpinCallsAllowed () {
      return allowHairpinCalls;
    }

    public synchronized void setHairpinCallsAllowed (boolean flag) {
      allowHairpinCalls = flag;
    }

    public synchronized boolean isMediaRoutingEnabled () {
      return defaultMediaRouting;
    }

    public synchronized void setMediaRouting (boolean flag) {
      defaultMediaRouting = flag;
    }

    public synchronized boolean isMidCallMediaChangeEnabled () {
      return defaultMidCallMediaChange;
    }

    public synchronized void setMidCallMediaChange (boolean flag) {
      defaultMidCallMediaChange = flag;
    }

    public synchronized int getMaxHunts () {
      return maxHunts;
    }

    public synchronized void setMaxHunts (int hunts) {
      maxHunts = hunts;
    }

    public synchronized int getMaxHuntsLimit () {
      return maxHuntsLimit;
    }

    public synchronized void setMaxHuntsLimit (int huntsLimit) {
      maxHuntsLimit = huntsLimit;
    }

    public synchronized int getMaxHuntsAllowableDuration () {
      return maxHuntsAllowableDuration;
    }

    public synchronized void setMaxHuntsAllowableDuration (int duration) {
      maxHuntsAllowableDuration = duration;
    }

    public synchronized boolean isAllRtpAllowed () {
      return allowAllRtp;
    }

    public synchronized void setAllRtpAllowed (boolean flag) {
      allowAllRtp = flag;
    }

    public synchronized boolean isSrcAddrForwarded () {
      return forwardSrcAddr;
    }

    public synchronized void setSrcAddrForwarded (boolean flag) {
      forwardSrcAddr = flag;
    }

    public synchronized void setRemoveRFC2833 (boolean flag) {
      removeRFC2833 = flag;
    }

    public synchronized boolean isRemoveRFC2833 () {
      return removeRFC2833;
    }

    public synchronized void setRemoveT38 (boolean flag) {
      removeT38 = flag;
    }

    public synchronized boolean isRemoveT38 () {
      return removeT38;
    }

    public synchronized void setCacheTimeout(int timeout) {
      cacheTimeout = timeout;
    }

    public synchronized int getCacheTimeout() {
      return cacheTimeout;
    }

    public synchronized void setMaxCallDuration (int timeout) {
      maxCallDuration = timeout;
    }

    public synchronized int getMaxCallDuration () {
      return maxCallDuration;
    }

    public synchronized void setMswName(String mswName) {
      this.mswName = mswName;
    }

    public synchronized String getMswName() {
      return mswName;
    }


    public synchronized void setMgmtIp(String ip) {
      this.mgmtIp = ip;
    }

    public synchronized String getMgmtIp() {
      return mgmtIp;
    }

	public synchronized void setMapIsdncc(boolean flag) {
	    mapIsdncc = flag;
	}

	public synchronized boolean isMapIsdncc() {
	    return mapIsdncc;
	}

	public synchronized void setMapLrjReason(boolean flag) {
	    mapLrjReason = flag;
	}

	public synchronized boolean isMapLrjReason() {
	    return mapLrjReason;
	}

	public synchronized void setUseCodeMap(String id){
	    this.useCodeMap = id;
	}

	public synchronized String getUseCodeMap(){
	    return useCodeMap;
	}


    public synchronized Node getXMLRepresentation () throws DOMException {
      Element root = doc.createElement("systemConfig");
      if (!isValid())
        return root;

      Element tag = doc.createElement("codec");
      if (g711Ulaw64Duration != 0)
	tag.setAttribute("g711Ulaw64Duration", String.valueOf(g711Ulaw64Duration));
      if (g711Alaw64Duration != 0)
	tag.setAttribute("g711Alaw64Duration", String.valueOf(g711Alaw64Duration));
      if (g729Frames != 0)
	tag.setAttribute("g729Frames", String.valueOf(g729Frames));
      if (g7231Frames != 0)
	tag.setAttribute("g7231Frames", String.valueOf(g7231Frames));
      if (defaultCodec != -1)
	tag.setAttribute("default", String.valueOf(defaultCodec));

      root.appendChild(tag);

      if (enumDomain != null && enumDomain.length() > 0) {
	tag = doc.createElement("enumDomain");
	Text data = doc.createTextNode(enumDomain);
	tag.appendChild(data);
	root.appendChild(tag);
      }

      tag = doc.createElement("radius");
      if (radiusServer != null && radiusServer.length() > 0) {
 	Element tag1 = doc.createElement("server");
 	Text data = doc.createTextNode(radiusServer);
 	tag1.appendChild(data);
 	tag.appendChild(tag1);
      }
      if (radiusSecret != null && radiusSecret.length() > 0) {
	Element tag1 = doc.createElement("secret");
	try {
	  String estr = WeakEncryptionOutputStream.getEncryptedString(radiusSecret);
	  CDATASection cdata = doc.createCDATASection(estr);
	  tag1.appendChild(cdata);
	} catch (IOException ie) {
	  throw new DOMException(DOMException.NOT_FOUND_ERR, "Unable to write RADIUS secret:" + ie.toString());
	}
	tag.appendChild(tag1);
      }   

      tag = doc.createElement("rolloverTime");
      Text data = doc.createTextNode(String.valueOf(rolloverTime));
      tag.appendChild(data);
      root.appendChild(tag);

      tag = doc.createElement("cacheTimeout");
      data = doc.createTextNode(String.valueOf(cacheTimeout));
      tag.appendChild(data);
      root.appendChild(tag);

      tag  = doc.createElement("maxCallDuration");
      data = doc.createTextNode(String.valueOf(maxCallDuration));
      tag.appendChild(data);
      root.appendChild(tag);

      tag = doc.createElement("accessControl");
      tag.setAttribute("allowAllSrc", String.valueOf(allowAllSrc));
      tag.setAttribute("allowAllDst", String.valueOf(allowAllDst));
      tag.setAttribute("allowHairpinCalls", String.valueOf(allowHairpinCalls));
      tag.setAttribute("defaultMediaRouting", String.valueOf(defaultMediaRouting));
      tag.setAttribute("defaultMidCallMediaChange", String.valueOf(defaultMidCallMediaChange));
      tag.setAttribute("allowAllRtp", String.valueOf(allowAllRtp));
      tag.setAttribute("forwardSrcAddr", String.valueOf(forwardSrcAddr));
      tag.setAttribute("removeRFC2833", String.valueOf(removeRFC2833));
      tag.setAttribute("removeT38", String.valueOf(removeT38));
      Element child = doc.createElement("maxHunts");
      child.setAttribute("limit", String.valueOf(maxHuntsLimit));
      child.setAttribute("allowableDuration", String.valueOf(maxHuntsAllowableDuration));
      child.appendChild(doc.createTextNode(String.valueOf(maxHunts)));
      tag.appendChild(child);
      root.appendChild(tag);

      tag  = doc.createElement("mswName");
      data = doc.createTextNode(mswName);
      tag.appendChild(data);
      root.appendChild(tag);

      tag  = doc.createElement("mgmtIp");
      data = doc.createTextNode(mgmtIp);
      tag.appendChild(data);
      root.appendChild(tag);

	    tag = doc.createElement("reasonCode");
	    tag.setAttribute("mapIsdncc", Boolean.toString(mapIsdncc));
	    tag.setAttribute("mapLrjReason", Boolean.toString(mapLrjReason));
	    tag.setAttribute("useCodeMap", useCodeMap);
	    root.appendChild(tag);

      
      return root;
    }
  }

  public class RedundsConfig {
    private boolean configValid;

    private NetworkConfig networkConfig;
    private DatabaseConfig databaseConfig;

    public boolean equals (RedundsConfig rc) {
      if (networkConfig.equals(rc.getNetworkConfig()) &&
	  databaseConfig.equals(rc.getDatabaseConfig()))
	return true;

      return false;
    }

    RedundsConfig () {
      setValid(true);
      networkConfig = new NetworkConfig();
      databaseConfig = new DatabaseConfig();
    }

    RedundsConfig (Element element) throws DOMException {
      this();

      NodeList list = element.getElementsByTagName("networkConfig");
      if (list != null && list.getLength() == 1) {
	networkConfig = new NetworkConfig((Element)list.item(0));
      }

      list = element.getElementsByTagName("databaseConfig");
      if (list != null && list.getLength() == 1) {
	databaseConfig = new DatabaseConfig((Element)list.item(0));
      }
    }

    public void setValid (boolean val) {
      configValid = val;
    }

    public boolean isValid () {
      return configValid;
    }

    public synchronized NetworkConfig getNetworkConfig () {
      return networkConfig;
    }

    public synchronized DatabaseConfig getDatabaseConfig () {
      return databaseConfig;
    }

    public synchronized Node getXMLRepresentation () throws DOMException {
      Element root = doc.createElement("redundsConfig");
      if (!isValid())
        return root;

      root.appendChild(networkConfig.getXMLRepresentation());

      root.appendChild(databaseConfig.getXMLRepresentation());

      return root; 
    }

    public class NetworkConfig {
      private String serverType;
	private String primaryInterface, secondaryInterface,controlInterface;
//      private String primaryInterfaceVip;
      private String [] primaryInterfaceVips;
      private String [] secondaryInterfaceVips;
      private String primaryInterfaceRouter;
      private String secondaryInterfaceRouter;
      private String [] peers;
      private boolean scmEnabled;

      public boolean equals (NetworkConfig nc) {
	if (serverType.equals(nc.getServerType()) &&
	    primaryInterface.equals(nc.getPrimaryInterface()) &&
	    secondaryInterface.equals(nc.getSecondaryInterface()) &&
      //primaryInterfaceVip.equals(nc.getPrimaryInterfaceVip()) &&
      primaryInterfaceVips.length ==  nc.getPrimaryInterfaceVips().length &&
      secondaryInterfaceVips.length ==  nc.getSecondaryInterfaceVips().length &&
      objEquals(primaryInterfaceVips,nc.getPrimaryInterfaceVips()) &&
	    objEquals(secondaryInterfaceVips,nc.getSecondaryInterfaceVips()) &&
	    primaryInterfaceRouter.equals(nc.getPrimaryInterfaceRouter()) &&
	    secondaryInterfaceRouter.equals(nc.getSecondaryInterfaceRouter()) &&
	    controlInterface.equals(nc.getControlInterface()) &&
	    peers.length == nc.getPeers().length &&
      objEquals(peers,nc.getPeers()) &&
      scmEnabled ==  nc.isScmEnabled()

)        {
	      return true;
    	}

	      return false;
      }

      NetworkConfig () {
	      serverType = "disabled";
	      primaryInterface = "";
	      secondaryInterface = "";
	      controlInterface = "";
//	    primaryInterfaceVip = "";
        primaryInterfaceVips  = new String[0];
	secondaryInterfaceVips = new String[0];
	      primaryInterfaceRouter = "";
	      secondaryInterfaceRouter = "";
	      peers = new String [0];
        scmEnabled  = false;
      }

      NetworkConfig (Element element) throws DOMException {
	      this();

	      serverType = element.getAttribute("serverType");

	      NodeList list = element.getElementsByTagName("primaryInterface");
	      if (list != null && list.getLength() == 1) {
	        Element childElem = (Element)list.item(0);
	        if (childElem != null) {
      //	    primaryInterfaceVip = childElem.getAttribute("vip");
	          primaryInterfaceRouter = childElem.getAttribute("router");
//	          if (childElem.getLastChild() != null)
//	            primaryInterface = childElem.getLastChild().getNodeValue();
	        }
	        list = childElem.getElementsByTagName("name");
	        if (list != null && list.getLength() == 1) {
            Node childNode = list.item(0).getLastChild();
	          if (childNode != null)
	            primaryInterface = childNode.getNodeValue();

          }

	        list = childElem.getElementsByTagName("vip");
	        if (list != null) {
	          int len = list.getLength();
	          for (int i = 0; i < len; i++) {
	            Node childNode = list.item(i).getLastChild();
	            if (childNode != null)
	              primaryInterfaceVips = SysUtil.createStringArray(primaryInterfaceVips, childNode.getNodeValue());
	          }
	        }

	      }

	      list = element.getElementsByTagName("secondaryInterface");
	      if (list != null && list.getLength() == 1) {
		  Element childElem = (Element)list.item(0);
		  if (childElem != null) 
		      secondaryInterfaceRouter = childElem.getAttribute("router");
		  list = childElem.getElementsByTagName("name");
		  if (list != null && list.getLength() == 1) {
		      Node childNode = list.item(0).getLastChild();
		      if (childNode != null)
			  secondaryInterface = childNode.getNodeValue();
		  }

	        list = childElem.getElementsByTagName("vip");
	        if (list != null) {
	          int len = list.getLength();
	          for (int i = 0; i < len; i++) {
	            Node childNode = list.item(i).getLastChild();
	            if (childNode != null)
	              secondaryInterfaceVips = SysUtil.createStringArray(secondaryInterfaceVips, childNode.getNodeValue());
	          }
	        }

	      }

	      list = element.getElementsByTagName("controlInterface");
	      if (list != null && list.getLength() == 1) {
	        Node childNode = list.item(0).getLastChild();
	        if (childNode != null)
	          controlInterface = childNode.getNodeValue();
	      }

	      list = element.getElementsByTagName("peer");
	      if (list != null) {
	        int len = list.getLength();
	        for (int i = 0; i < len; i++) {
	          Node childNode = list.item(i).getLastChild();
	          if (childNode != null)
	            peers = SysUtil.createStringArray(peers, childNode.getNodeValue());
	        }
	      }
        list = element.getElementsByTagName("scm");
        if (list != null && list.getLength() == 1) {
      	  String status = ((Element)list.item(0)).getAttribute("status");
	        scmEnabled = "enabled".equals(status);
        }

      }

      public boolean objEquals(String[] arr1, String[] arr2){

	      for (int i = 0; i < arr1.length; i++) {
	        int j = 0;
	        for (;j < arr2.length; j++) {
	          if (arr1[i].equals(arr2[j]))
		          break;
	        } 
	        if (j == arr2.length)
	          return false;  // could not find that ip
	      }
        return false;
      }


      public synchronized void setServerType (String type) {
	serverType = (type == null)?"":type;
      }

      public synchronized String getServerType () {
	return serverType;
      }

      public synchronized boolean isServerEnabled () {
	return !serverType.equals("disabled");
      }

      public synchronized void setPrimaryInterface (String name) {
	primaryInterface = (name == null)?"":name;
      }

      public synchronized String getPrimaryInterface () {
	return primaryInterface;
      }
      public synchronized void setSecondaryInterface (String name) {
	secondaryInterface = (name == null)?"":name;
      }

      public synchronized String getSecondaryInterface () {
	return secondaryInterface;
      }
/*
      public synchronized void setPrimaryInterfaceVip (String vip) {
	primaryInterfaceVip = (vip == null)?"":vip;
      }

      public synchronized String getPrimaryInterfaceVip () {
	return primaryInterfaceVip;
      }
*/
      public synchronized void setPrimaryInterfaceVips (String[] vs) {
        primaryInterfaceVips = (vs  == null)?new String [0]:vs;
      }


      public synchronized String[] getPrimaryInterfaceVips() {
	      return primaryInterfaceVips;
      }

      public synchronized void clearPrimaryInterfaceVips () {
	      primaryInterfaceVips = new String [0];
      }

      public synchronized void addPrimaryInterfaceVip (String vip) {
	      primaryInterfaceVips = SysUtil.createStringArray(primaryInterfaceVips, vip);
      }

      public synchronized void setSecondaryInterfaceVips (String[] vs) {
        secondaryInterfaceVips = (vs  == null)?new String [0]:vs;
      }


      public synchronized String[] getSecondaryInterfaceVips() {
	      return secondaryInterfaceVips;
      }

      public synchronized void clearSecondaryInterfaceVips () {
	      secondaryInterfaceVips = new String [0];
      }

      public synchronized void addSecondaryInterfaceVip (String vip) {
	      secondaryInterfaceVips = SysUtil.createStringArray(secondaryInterfaceVips, vip);
      }

      public synchronized void setPrimaryInterfaceRouter (String router) {
	primaryInterfaceRouter = (router == null)?"":router;
      }

      public synchronized String getPrimaryInterfaceRouter () {
	return primaryInterfaceRouter;
      }
      public synchronized void setSecondaryInterfaceRouter (String router) {
	secondaryInterfaceRouter = (router == null)?"":router;
      }

      public synchronized String getSecondaryInterfaceRouter () {
	return secondaryInterfaceRouter;
      }

      public synchronized void setControlInterface (String name) {
	controlInterface = (name == null)?"":name;
      }

      public synchronized String getControlInterface () {
	return controlInterface;
      }

      public synchronized void setPeers (String [] ps) {
	peers = (ps == null)?new String [0]:ps;
      }

      public synchronized String [] getPeers () {
	return peers;
      }

      public synchronized void clearPeers () {
	peers = new String [0];
      }

      public synchronized void addPeer (String peer) {
	peers = SysUtil.createStringArray(peers, peer);
      }

      public synchronized void setScmEnabled(boolean enable){
        scmEnabled  = enable;
      }

      public synchronized boolean isScmEnabled(){
        return scmEnabled;
      }


      public synchronized Node getXMLRepresentation () throws DOMException {
	      Element root = doc.createElement("networkConfig");

	      root.setAttribute("serverType", getServerType());

	      Element tag = doc.createElement("primaryInterface");
//	      tag.setAttribute("vip", getPrimaryInterfaceVip());
	      tag.setAttribute("router", getPrimaryInterfaceRouter());
//	      Text data = doc.createTextNode(getPrimaryInterface());
//	      tag.appendChild(data);

        Element ptag = doc.createElement("name");
        Text data = doc.createTextNode(getPrimaryInterface());
        ptag.appendChild(data);
        tag.appendChild(ptag);

	      for (int i = 0; i < primaryInterfaceVips.length; i++) {
	        ptag = doc.createElement("vip");
	        data = doc.createTextNode(primaryInterfaceVips[i]);
	        ptag.appendChild(data);
          tag.appendChild(ptag);
	      }

	      root.appendChild(tag);

	      if( getSecondaryInterface() != null && getSecondaryInterface().length() > 0 ) {
		  tag = doc.createElement("secondaryInterface");
		  tag.setAttribute("router", getSecondaryInterfaceRouter());
		  ptag = doc.createElement("name");
		  data = doc.createTextNode(getSecondaryInterface());
		  ptag.appendChild(data);
		  tag.appendChild(ptag);

		  for (int i = 0; i < secondaryInterfaceVips.length; i++) {
		      ptag = doc.createElement("vip");
		      data = doc.createTextNode(secondaryInterfaceVips[i]);
		      ptag.appendChild(data);
		      tag.appendChild(ptag);
		  }

		  root.appendChild(tag);
	      }

	      tag = doc.createElement("controlInterface");
	      data = doc.createTextNode(getControlInterface());
	      tag.appendChild(data);
	      root.appendChild(tag);

	      for (int i = 0; i < peers.length; i++) {
	        tag = doc.createElement("peer");
	        data = doc.createTextNode(peers[i]);
	        tag.appendChild(data);
	        root.appendChild(tag);
	      }

         tag = doc.createElement("scm");
         tag.setAttribute("status", scmEnabled? "enabled":"disabled");
         root.appendChild(tag);
	 return root;
      }
    }

    public class DatabaseConfig {
      private boolean serverStatus;
      private String ifName, mcastAddr;
      private int port, priority;

      public boolean equals (DatabaseConfig dc) {
	if (serverStatus == dc.isServerEnabled() &&
	    ifName.equals(dc.getInterfaceName()) &&
	    mcastAddr.equals(dc.getMcastAddr()) &&
	    port == dc.getPort() &&
	    priority == dc.getPriority())
	  return true;

	return false;
      }

      DatabaseConfig () {
	ifName = "";
	mcastAddr = "";
      }

      DatabaseConfig (Element element) throws DOMException {
	this();

	String status = element.getAttribute("status");
	serverStatus = "enabled".equals(status);

	NodeList list = element.getElementsByTagName("interface");
	if (list != null && list.getLength() == 1) {
	  Node childNode = list.item(0).getLastChild();
	  if (childNode != null)
	    ifName = childNode.getNodeValue();
	}

	list = element.getElementsByTagName("mcastAddr");
	if (list != null && list.getLength() == 1) {
	  Node childNode = list.item(0).getLastChild();
	  if (childNode != null)
	    mcastAddr = childNode.getNodeValue();
	}

	list = element.getElementsByTagName("port");
	if (list != null && list.getLength() == 1) {
	  Node childNode = list.item(0).getLastChild();
	  if (childNode != null) {
	    try {
	      port = Integer.parseInt(childNode.getNodeValue());
	    } catch (NumberFormatException nfe) {}
	  }
	}

	list = element.getElementsByTagName("priority");
	if (list != null && list.getLength() == 1) {
	  Node childNode = list.item(0).getLastChild();
	  if (childNode != null) {
	    try {
	      priority = Integer.parseInt(childNode.getNodeValue());
	    } catch (NumberFormatException nfe) {}
	  }
	}
      }

      public synchronized void setServerEnabled (boolean status) {
	serverStatus = status;
      }

      public synchronized boolean isServerEnabled () {
	return serverStatus;
      }

      public synchronized void setInterfaceName (String name) {
	ifName = (name == null)?"":name;
      }

      public synchronized String getInterfaceName () {
	return ifName;
      }

      public synchronized void setMcastAddr (String addr) {
	mcastAddr = addr;
      }

      public synchronized String getMcastAddr () {
	return mcastAddr;
      }

      public synchronized void setPort (int port) {
	this.port = port;
      }

      public synchronized int getPort () {
	return port;
      }

      public synchronized void setPriority (int prio) {
	priority = prio;
      }

      public synchronized int getPriority () {
	return priority;
      }

      public synchronized Node getXMLRepresentation () throws DOMException {
	Element root = doc.createElement("databaseConfig");

	root.setAttribute("status", isServerEnabled()?"enabled":"disabled");

	Element tag = doc.createElement("interface");
	Text data = doc.createTextNode(getInterfaceName());
	tag.appendChild(data);
	root.appendChild(tag);

	tag = doc.createElement("mcastAddr");
	data = doc.createTextNode(getMcastAddr());
	tag.appendChild(data);
	root.appendChild(tag);

	tag = doc.createElement("port");
	data = doc.createTextNode(String.valueOf(getPort()));
	tag.appendChild(data);
	root.appendChild(tag);

	tag = doc.createElement("priority");
	data = doc.createTextNode(String.valueOf(getPriority()));
	tag.appendChild(data);
	root.appendChild(tag);

	return root;
      }
    }

  }

  public class LoggingConfig {
    private boolean configValid;

    boolean []lusModules;
    boolean []vpnsModules;
    boolean []bcsModules;
    boolean []jserverModules;
    boolean []faxModules;
    boolean []gisModules;
    boolean []pmModules;
    String slogDebugPath;
    String slogErrPath;
    String h323Path;

    //  custom logs
    int debugLevel;
    boolean updateAllocation;
    boolean tpktchan;
    boolean udpchan;
    boolean pererr;
    boolean cm;
    boolean cmapicb;
    boolean cmapi;
    boolean cmerr;
    boolean li;
    boolean liinfo;

    //sdbug level
    int sdebugLevel;

    LoggingConfig () {
      setValid(true);

      lusModules   =  new boolean[MAX_MODULES];
      vpnsModules  =  new boolean[MAX_MODULES];
      bcsModules   =  new boolean[MAX_MODULES];
      jserverModules =  new boolean[MAX_MODULES];
      faxModules     =  new boolean[MAX_MODULES];
      gisModules     =  new boolean[MAX_MODULES];
      pmModules      =  new boolean[MAX_MODULES];
      slogDebugPath  =  "";
      slogErrPath    =  "";
      h323Path       =  "";
      debugLevel      = -1;
      updateAllocation  = false;
      tpktchan    = false;  
      udpchan     = false;
      pererr      = false;
      cm          = false;
      cmapicb     = false;
      cmapi       = false;      
      cmerr       = false;      
      li          = false;      
      liinfo      = false;      
      sdebugLevel = -1;
    }

    LoggingConfig (Element element) throws DOMException {

      this();

      NodeList list = element.getElementsByTagName("lus");
      if (list != null && list.getLength() == 1) {
	Element childElem = (Element)list.item(0);
        for(int j=0; j < MAX_MODULES; j++){
	  String attribute  = moduleNames[j]+"_log";
	  boolean enabled = "enabled".equals(childElem.getAttribute(attribute));
	  setLusModuleEnabled(j,enabled);
        }
      }

      list = element.getElementsByTagName("vpns");
      if (list != null && list.getLength() == 1) {
	Element childElem = (Element)list.item(0);
        for(int j=0; j < MAX_MODULES; j++){
	  String attribute  = moduleNames[j]+"_log";
	  boolean enabled = "enabled".equals(childElem.getAttribute(attribute));
	  setVpnsModuleEnabled(j,enabled);
        }
      }

      list = element.getElementsByTagName("bcs");
      if (list != null && list.getLength() == 1) {
	Element childElem = (Element)list.item(0);
        for(int j=0; j < MAX_MODULES; j++){
	  String attribute  = moduleNames[j]+"_log";
	  boolean enabled = "enabled".equals(childElem.getAttribute(attribute));
	  setBcsModuleEnabled(j,enabled);
        }
      }


      list = element.getElementsByTagName("jserver");
      if (list != null && list.getLength() == 1) {
	Element childElem = (Element)list.item(0);
        for(int j=0; j < MAX_MODULES; j++){
	  String attribute  = moduleNames[j]+"_log";
	  boolean enabled = "enabled".equals(childElem.getAttribute(attribute));
	  setJServerModuleEnabled(j,enabled);
        }
      }

      list = element.getElementsByTagName("fax");
      if (list != null && list.getLength() == 1) {
	Element childElem = (Element)list.item(0);
        for(int j=0; j < MAX_MODULES; j++){
	  String attribute  = moduleNames[j]+"_log";
	  boolean enabled = "enabled".equals(childElem.getAttribute(attribute));
	  setFaxModuleEnabled(j,enabled);
        }
      }

      list = element.getElementsByTagName("gis");
      if (list != null && list.getLength() == 1) {
	Element childElem = (Element)list.item(0);
        for(int j=0; j < MAX_MODULES; j++){
	  String attribute  = moduleNames[j]+"_log";
	  boolean enabled = "enabled".equals(childElem.getAttribute(attribute));
	  setGisModuleEnabled(j,enabled);
        }
      }

      list = element.getElementsByTagName("pm");
      if (list != null && list.getLength() == 1) {
	Element childElem = (Element)list.item(0);
        for(int j=0; j < MAX_MODULES; j++){
	  String attribute  = moduleNames[j]+"_log";
	  boolean enabled = "enabled".equals(childElem.getAttribute(attribute));
	  setPmModuleEnabled(j,enabled);
        }
      }

      list = element.getElementsByTagName("syslog_err");
      if (list != null && list.getLength() == 1) {
        Node childNode  = list.item(0).getLastChild();
	if (childNode != null)
	  slogErrPath = childNode.getNodeValue();
      }

      list = element.getElementsByTagName("syslog_debug");
      if (list != null && list.getLength() == 1) {
        Node childNode  = list.item(0).getLastChild();
	if (childNode != null)
	  slogDebugPath = childNode.getNodeValue();
      }

      list = element.getElementsByTagName("h323_log");
      if (list != null && list.getLength() == 1) {
        Node childNode = list.item(0).getLastChild();
        if (childNode != null)
          h323Path = childNode.getNodeValue();
      }

      list = element.getElementsByTagName("customLogConfig");
      if (list != null && list.getLength() == 1){

          Element customElement = (Element)list.item(0);
          NodeList childList = customElement.getElementsByTagName("debugLevel");
          debugLevel  = -1;
          if (childList != null && childList.getLength() == 1) {
	          Node childNode = childList.item(0).getLastChild();
            if (childNode != null && childNode.getNodeValue() != null)
	            debugLevel = Integer.parseInt(childNode.getNodeValue());
          }

          childList = customElement.getElementsByTagName("updateAllocation");
          if (childList != null && childList.getLength() == 1) {
	          Element childElem = (Element)childList.item(0);
	          updateAllocation = "enabled".equals(childElem.getAttribute("status"));
          }

          childList= customElement.getElementsByTagName("tpktchan");
          if (childList!= null && childList.getLength() == 1) {
	          Element childElem = (Element)childList.item(0);
	          tpktchan = "enabled".equals(childElem.getAttribute("status"));
          }

          childList= customElement.getElementsByTagName("udpchan");
          if (childList!= null && childList.getLength() == 1) {
	          Element childElem = (Element)childList.item(0);
	          udpchan = "enabled".equals(childElem.getAttribute("status"));
          }

      
          childList= customElement.getElementsByTagName("pererr");
          if (childList!= null && childList.getLength() == 1) {
	          Element childElem = (Element)childList.item(0);
	          pererr = "enabled".equals(childElem.getAttribute("status"));
          }
          childList= customElement.getElementsByTagName("cm");
          if (childList!= null && childList.getLength() == 1) {
	          Element childElem = (Element)childList.item(0);
	          cm = "enabled".equals(childElem.getAttribute("status"));
          }
      
          childList= customElement.getElementsByTagName("cmapicb");
          if (childList!= null && childList.getLength() == 1) {
	          Element childElem = (Element)childList.item(0);
	          cmapicb = "enabled".equals(childElem.getAttribute("status"));
          }

          childList= customElement.getElementsByTagName("cmapi");
          if (childList!= null && childList.getLength() == 1) {
	          Element childElem = (Element)childList.item(0);
	          cmapi = "enabled".equals(childElem.getAttribute("status"));
          }

          childList= customElement.getElementsByTagName("cmerr");
          if (childList!= null && childList.getLength() == 1) {
	          Element childElem = (Element)childList.item(0);
	          cmerr = "enabled".equals(childElem.getAttribute("status"));
          }

          childList= customElement.getElementsByTagName("li");
          if (childList!= null && childList.getLength() == 1) {
	          Element childElem = (Element)childList.item(0);
	          li = "enabled".equals(childElem.getAttribute("status"));
          }

          childList= customElement.getElementsByTagName("liinfo");
          if (childList!= null && childList.getLength() == 1) {
	          Element childElem = (Element)childList.item(0);
	          liinfo = "enabled".equals(childElem.getAttribute("status"));
          }

      }

      list = element.getElementsByTagName("sdebugLevel");
      if (list != null && list.getLength() == 1) {
        Node childNode = list.item(0).getLastChild();
        if (childNode != null && childNode.getNodeValue() != null)
          sdebugLevel = Integer.parseInt(childNode.getNodeValue());
      }
    }

    public void setValid (boolean val) {
      configValid = val;
    }

    public boolean isValid () {
      return configValid;
    }

    public synchronized Node getXMLRepresentation () throws DOMException {
      Element root = doc.createElement("loggingConfig");


      // lus process modules
      Element tag = doc.createElement("lus"); for(int i=0; i < MAX_MODULES; i++){
        String  attribute = moduleNames[i]+"_log";
        tag.setAttribute(attribute, (isLusModuleEnabled(i))?"enabled":"disabled");
      }
      root.appendChild(tag);

      tag = doc.createElement("vpns");
      for(int i=0; i < MAX_MODULES; i++){
        String  attribute = moduleNames[i]+"_log";
        tag.setAttribute(attribute, (isVpnsModuleEnabled(i))?"enabled":"disabled");
      }
      root.appendChild(tag);

      tag = doc.createElement("bcs");
      for(int i=0; i < MAX_MODULES; i++){
        String  attribute = moduleNames[i]+"_log";
        tag.setAttribute(attribute, (isBcsModuleEnabled(i))?"enabled":"disabled");
      }
      root.appendChild(tag);

      tag = doc.createElement("jserver");
      for(int i=0; i < MAX_MODULES; i++){
        String  attribute = moduleNames[i]+"_log";
        tag.setAttribute(attribute, (isJServerModuleEnabled(i))?"enabled":"disabled");
      }
      root.appendChild(tag);

      tag = doc.createElement("fax");
      for(int i=0; i < MAX_MODULES; i++){
        String  attribute = moduleNames[i]+"_log";
        tag.setAttribute(attribute, (isFaxModuleEnabled(i))?"enabled":"disabled");
      }
      root.appendChild(tag);



      tag = doc.createElement("gis");
      for(int i=0; i < MAX_MODULES; i++){
        String  attribute = moduleNames[i]+"_log";
        tag.setAttribute(attribute, (isGisModuleEnabled(i))?"enabled":"disabled");
      }
      root.appendChild(tag);

      tag = doc.createElement("pm");
      for(int i=0; i < MAX_MODULES; i++){
        String  attribute = moduleNames[i]+"_log";
        tag.setAttribute(attribute, (isPmModuleEnabled(i))?"enabled":"disabled");
      }
      root.appendChild(tag);
      tag = doc.createElement("syslog_debug");
      Text data = doc.createTextNode(slogDebugPath);
      tag.appendChild(data);
      root.appendChild(tag);

      tag = doc.createElement("syslog_err");
      data = doc.createTextNode(slogErrPath);
      tag.appendChild(data);
      root.appendChild(tag);

      tag = doc.createElement("h323_log");
      data = doc.createTextNode(h323Path);
      tag.appendChild(data);
      root.appendChild(tag);

      Element customTag = doc.createElement("customLogConfig");

      tag = doc.createElement("debugLevel");
      data = doc.createTextNode(String.valueOf(debugLevel));
      tag.appendChild(data);
      customTag.appendChild(tag);

      tag = doc.createElement("updateAllocation");
      tag.setAttribute("status", (updateAllocation)?"enabled":"disabled");
      customTag.appendChild(tag);

      tag = doc.createElement("tpktchan");
      tag.setAttribute("status", (tpktchan)?"enabled":"disabled");
      customTag.appendChild(tag);

      tag = doc.createElement("udpchan");
      tag.setAttribute("status", (udpchan)?"enabled":"disabled");
      customTag.appendChild(tag);


      tag = doc.createElement("pererr");
      tag.setAttribute("status", (pererr)?"enabled":"disabled");
      customTag.appendChild(tag);

      tag = doc.createElement("cm");
      tag.setAttribute("status", (cm)?"enabled":"disabled");
      customTag.appendChild(tag);

      tag = doc.createElement("cmapicb");
      tag.setAttribute("status", (cmapicb)?"enabled":"disabled");
      customTag.appendChild(tag);

      tag = doc.createElement("cmapi");
      tag.setAttribute("status", (cmapi)?"enabled":"disabled");
      customTag.appendChild(tag);

      tag = doc.createElement("cmerr");
      tag.setAttribute("status", (cmerr)?"enabled":"disabled");
      customTag.appendChild(tag);

      tag = doc.createElement("li");
      tag.setAttribute("status", (li)?"enabled":"disabled");
      customTag.appendChild(tag);

      tag = doc.createElement("liinfo");
      tag.setAttribute("status", (liinfo)?"enabled":"disabled");
      customTag.appendChild(tag);

      root.appendChild(customTag);

      tag = doc.createElement("sdebugLevel");
      data = doc.createTextNode(String.valueOf(sdebugLevel));
      tag.appendChild(data);
      root.appendChild(tag);

      return root;
    }

    public String getModuleLogs(int process){

      boolean []logs  = new boolean[MAX_MODULES];

      switch(process){
      case  PROCESS_LUS:
	      logs  = lusModules;
	      break;
            case  PROCESS_VPNS:
	      logs  = vpnsModules;
	      break;
            case  PROCESS_BCS:
	      logs  = bcsModules;
	      break;
            case  PROCESS_JSERVER:
	      logs  = jserverModules;
	      break;
            case  PROCESS_FAXS:
	      logs  = faxModules;
	      break;
            case  PROCESS_GIS:
	      logs  = gisModules;
	      break;
            case  PROCESS_PM:
	      logs  = pmModules;
	      break;
            default:
	      break;
      }
      StringBuffer  moduleLogs  = new StringBuffer();
      for(int i=0; i < logs.length; i++){
        moduleLogs.append(logs[i]?"1":"0");
      }

      return moduleLogs.toString();        

    }

    public boolean setModuleLogs (int process, String logs) {
      StringTokenizer st = new StringTokenizer(logs,";");
      // check for < MAX_MODULES (not !=), if new modules are added in the C code, we would get
      // a longer string than we handle here
      if (st ==  null || (st.countTokens() < MAX_MODULES))
        return false;

      boolean [] tempLogs = new boolean [MAX_MODULES];
      for (int index = 0; index < MAX_MODULES; tempLogs[index++] = st.nextToken().equals("1"));

      switch (process) {
        case  PROCESS_LUS:
	        lusModules  = tempLogs;
	        break;
        case  PROCESS_VPNS:
	        vpnsModules  = tempLogs;
	        break;
        case  PROCESS_BCS:
	        bcsModules  = tempLogs;
	        break;
        case  PROCESS_JSERVER:
	        jserverModules  = tempLogs;
	        break;
        case  PROCESS_FAXS:
	        faxModules  = tempLogs;
	        break;
        case  PROCESS_GIS:
	        gisModules  = tempLogs;
	        break;
        case  PROCESS_PM:
	        pmModules  = tempLogs;
	        break;
        default:
      	  return false;
      }

      return true;
    }

    public void  setsLogDebugPath(String path){
      slogDebugPath = path;
    }

    public String  getsLogDebugPath(){
      return slogDebugPath;
    }

    public void  setsLogErrPath(String path){
      slogErrPath = path;
    }

    public String  getsLogErrPath(){
      return slogErrPath;
    }

    public void setH323LogPath (String path) {
      h323Path = path;
    }

    public String getH323LogPath () {
      return h323Path;
    }

    public void setLusModuleEnabled(int index, boolean enabled){
      lusModules[index]  = enabled;
    }
    public boolean isLusModuleEnabled(int index){
      return lusModules[index];
    }

    public void setVpnsModuleEnabled(int index, boolean enabled){
      vpnsModules[index]  = enabled;
    }
    public boolean isVpnsModuleEnabled(int index){
      return vpnsModules[index];
    }

    public void setBcsModuleEnabled(int index, boolean enabled){
      bcsModules[index]  = enabled;
    }

    public boolean isBcsModuleEnabled(int index){
      return bcsModules[index];
    }

    public void setJServerModuleEnabled(int index, boolean enabled){
      jserverModules[index]  = enabled;
    }

    public boolean isJServerModuleEnabled(int index){
      return jserverModules[index];
    }


    public void setFaxModuleEnabled(int index, boolean enabled){
      faxModules[index]  = enabled;
    }
    public boolean isFaxModuleEnabled(int index){
      return faxModules[index];
    }


    public void setGisModuleEnabled(int index, boolean enabled){
      gisModules[index]  = enabled;

    }

    public boolean isGisModuleEnabled(int index){     
      return gisModules[index];
    }


    public void setPmModuleEnabled(int index, boolean enabled){
      pmModules[index]  = enabled;
    }

    public boolean isPmModuleEnabled(int index){
      return pmModules[index];
    }


    public void setDebugLevel(int level){
      debugLevel  = level;
    }

    public int getDebugLevel(){
      return debugLevel;
    }

    public boolean isUpdateAllocation(){
      return updateAllocation;
    }
    
    public void setUpdateAllocation(boolean updateAllocation){
      this.updateAllocation = updateAllocation;
    }

    public boolean isTpktchan(){
      return tpktchan;
    }
    
    public void setTpktchan(boolean tpktchan){
      this.tpktchan = tpktchan;
    }

    public boolean isUdpchan(){
      return udpchan;
    }
    
    public void setUdpchan(boolean udpchan){
      this.udpchan = udpchan;
    }

    public boolean isPererr(){
      return pererr;
    }
    
    public void setPererr(boolean pererr){
      this.pererr = pererr;
    }

    public boolean isCm(){
      return cm;
    }
    
    public void setCm(boolean cm){
      this.cm = cm;
    }

    public boolean isCmapicb(){
      return cmapicb;
    }
    
    public void setCmapicb(boolean cmapicb){
      this.cmapicb  = cmapicb;
    }

    public boolean isCmapi(){
      return cmapi;
    }
    
    public void setCmapi(boolean cmapi){
      this.cmapi  = cmapi;
    }

    public boolean isCmerr(){
      return cmerr;
    }
    
    public void setCmerr(boolean cmerr){
      this.cmerr  = cmerr;
    }

    public boolean isLi(){
      return li;
    }
    
    public void setLi(boolean li){
      this.li  = li;
    }

    public boolean isLiinfo(){
      return liinfo;
    }
    
    public void setLiinfo(boolean liinfo){
      this.liinfo  = liinfo;
    }

    public void setSDebugLevel(int level){
      sdebugLevel  = level;
    }

    public int getSDebugLevel(){
      return sdebugLevel;
    }
  }
  public class Triggers{
    private RouteTrigger sip;
    private RouteTrigger h323;
    boolean configValid;
    
    Triggers() {
      sip = new RouteTrigger(PROTOCOL_SIP);
      h323 = new RouteTrigger(PROTOCOL_H323);
    }
  

    Triggers(Element element) throws DOMException {
      this();
      parseXML(element);
    }

    public synchronized void  parseXML(Element element) throws DOMException{

      if(element  !=  null){
        NodeList  list  = element.getElementsByTagName(PROTOCOL_SIP);
	      if (list != null && list.getLength() == 1)  {
          sip.parseXML(list);
        }
        list  = element.getElementsByTagName(PROTOCOL_H323);
	      if (list != null && list.getLength() == 1)  {
          h323.parseXML(list);
        }
      }
    }

    public String getXMLFile() {
      StringWriter  sw= new StringWriter();
      try{
        Element e = doc.getDocumentElement();
        if(e  != null){
          doc.removeChild((Node)e);
        }
        e = (Element)getXMLRepresentation();
        e.setAttribute("version", "1.0");
        doc.appendChild(e);
	      XmlDocument xdoc = (XmlDocument) doc;
	      xdoc.write (sw); 
      }catch(Exception ie){
      }

	    Element e = doc.getDocumentElement();
      if(e  != null){
        doc.removeChild((Node)e);
      }

      return sw.toString();
    }


    public synchronized Node getXMLRepresentation () throws DOMException {
	    Element root = doc.createElement("Triggers");

      root.appendChild((Node)sip.getXMLRepresentation());
      root.appendChild((Node)h323.getXMLRepresentation());
	    return root;
    }



    public void setTriggers (String xmlString) throws IOException, SAXException, DOMException{
      parseXML(db.parse(new InputSource(new StringReader(xmlString))).getDocumentElement());
    }

    public boolean equals (Object o) {
	    if (o.getClass().equals(Triggers.class)) {
	      Triggers given = (Triggers)o;
	      if(sip   == given.getSip() &&
           h323  ==  given.getH323()
          )
	        return true;
	    }
	    return false;
    }

    public RouteTrigger getSip(){
      return sip;
    }
    public void setSip(RouteTrigger sip){
      this.sip  = sip;
    }

    public RouteTrigger getH323(){
      return h323;
    }
    public void setH323(RouteTrigger h323){
      this.h323  = h323;
    }

    public String toString() {
	    StringBuffer sb = new StringBuffer();
	    sb.append("SIP Triggers =");
	    sb.append(sip.toString());
	    sb.append("\n H323 Triggers=");
	    sb.append(h323.toString());
	    return sb.toString();
    }


    public void setValid (boolean val) {
      configValid = val;
    }

    public boolean isValid () {
      return configValid;
    }

    public class RouteTrigger{
      private String protocol;
      private int   huntDefault;
      private int[] hunt;
      private int[] noHunt;
      private int huntMax;
      private int noHuntMax;


      public RouteTrigger(String protocol){
        this.protocol = protocol;
        init();
      }

      public void init(){
        huntMax    = 0;
        noHuntMax  = 0;
        huntDefault= 0;
        hunt     = null;
        noHunt   = null;
      }


      public void parseXML(NodeList list){
	      if (list != null && list.getLength() == 1)  {
          Element e  = (Element)list.item(0);
          huntDefault = Integer.parseInt(e.getAttribute("default"));
          huntMax   = Integer.parseInt(e.getAttribute("hunt_max"));
          noHuntMax = Integer.parseInt(e.getAttribute("no_hunt_max"));

          list  = e.getElementsByTagName("Hunt");
          if (list != null ){
            hunt  = new int[huntMax];
            for(int i = 0; i < list.getLength(); i++) {
              hunt[i] = Integer.parseInt(list.item(i).getLastChild().getNodeValue());
            }
          }

          list  = e.getElementsByTagName("NoHunt");
          if (list != null ){
            noHunt  = new int[noHuntMax];
            for(int i = 0; i < list.getLength(); i++)  {
              noHunt[i] = Integer.parseInt(list.item(i).getLastChild().getNodeValue());
            }
          }
	      }
      }



      public synchronized Element getXMLRepresentation () throws DOMException {
	        Element root= doc.createElement(protocol);
          root.setAttribute("default",String.valueOf(huntDefault));
          root.setAttribute("hunt_max",String.valueOf(hunt.length));
          root.setAttribute("no_hunt_max",String.valueOf(noHunt.length));

          if(hunt !=  null){
            Arrays.sort(hunt);
            for(int i=0; i < hunt.length; i++){
              Element huntTag = doc.createElement("Hunt");
	            Text data = doc.createTextNode(String.valueOf(hunt[i]));
	            huntTag.appendChild(data);
              root.appendChild(huntTag);
            }
          }

          if(noHunt !=  null){
            Arrays.sort(noHunt);
            for(int i=0; i < noHunt.length; i++){
              Element huntTag = doc.createElement("NoHunt");
	            Text data = doc.createTextNode(String.valueOf(noHunt[i]));
	            huntTag.appendChild(data);
              root.appendChild(huntTag);
            }
          }
          return root;
      }

      public void setHuntDefault(int huntDefault){
        this.huntDefault = huntDefault;
      }

      public int getHuntDefault(){
        return huntDefault;
      }

       public int getHuntMax(){
         return (hunt  ==  null)?0: hunt.length ;
      }

      public int getNoHuntMax(){
        return (noHunt  ==  null)?0: noHunt.length ;
      }

      public void setNoHunt(int[] hunt){
        noHunt = hunt;
      }

      public int[] getNoHunt(){
        return noHunt;
      }

      public void setHunt(int[] hunt){
        this.hunt = hunt;
      }

      public int[] getHunt(){
        return hunt;
      }

      public String toString() {
	      StringBuffer sb = new StringBuffer();
	      sb.append("HuntDefault =");
	      sb.append(huntDefault);
	      sb.append("\n huntMax =");
	      sb.append(getHuntMax());
	      sb.append("\n NoHuntMax =");
	      sb.append(getNoHuntMax());
	      sb.append("\n hunt =");
	      sb.append(hunt);
	      sb.append("\n noHunt =");
	      sb.append(noHunt);
	      return sb.toString();
      }
    }
  }
}
