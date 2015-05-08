package com.nextone.common;

import java.io.StringReader;
import java.io.StringWriter;

import javax.xml.parsers.DocumentBuilder;
import javax.xml.parsers.DocumentBuilderFactory;

import org.apache.crimson.tree.XmlDocument;
import org.w3c.dom.DOMException;
import org.w3c.dom.Document;
import org.w3c.dom.Element;
import org.w3c.dom.Node;
import org.w3c.dom.NodeList;
import org.xml.sax.InputSource;



/**
 *  This class represents iserver capabilities
 **/

public class Capabilities{
  public static final String BILLING  = "Billing"; 
  public static final String FCE      = "FCE"; 
  public static final String SYSTEM   = "System";
  public static final String SIP      = "Sip"; 
  public static final String DB       = "Database";
  public static final String REDUND = "Redundancy";
  public static final String LOGGING  = "Logging";
  public static final String H323     = "H323";

  private Billing billing;
  private FCE fce;
  private SystemCapability system;
  private SipCapability    sip;
  private LoggingCapability logging;
  private DBCapability     database;
  private RedundCapability redund;
  private H323Capability   h323;

  private String[]  interfaceNames;

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

  public Capabilities() {
    setDefault();
  }


  public Capabilities(String xmlString) throws Exception {
    this(db.parse(new InputSource(new StringReader(xmlString))).getDocumentElement());
  }


  public Capabilities (Element element)  {
    this();
    try{
      String version = element.getAttribute("version");
      if (version.equals("1.0"))
        create10(element);
      else
        throw new DOMException(DOMException.NOT_FOUND_ERR, "Do not recognize cap version " + version);
    }catch(Exception e){
      setDefault();
    }

  }


  private void create10 (Element element) {
    NodeList list;
    // get the interface names

    try{
	    list = element.getElementsByTagName("Interface");
	    if (list != null) {
        interfaceNames  = new String[list.getLength()];
	      for (int i = 0; i < list.getLength(); i++) {
	        Element elem = (Element)list.item(i);
  	      interfaceNames[i] = elem.getLastChild().getNodeValue();
        }
      }
    }catch(Exception e){
    }
    try{
      list = element.getElementsByTagName(DB);
      if (list != null && list.getLength() == 1) {
        database = new DBCapability((Element)list.item(0));
      }
    }catch(Exception e){
    }

    try{
      list = element.getElementsByTagName("Realm");
      if (list != null && list.getLength() == 1){
	String status = ((Element)list.item(0)).getAttribute("status");
        setRealmEnabled("enabled".equals(status));
      }
    }catch(Exception e){
      setRealmEnabled(false);
    }

    try{
      list = element.getElementsByTagName(BILLING);
      if (list != null && list.getLength() == 1) {
        billing = new Billing((Element)list.item(0));
      }
    }catch(Exception e){
    }

    try{
      list = element.getElementsByTagName(FCE);
      if (list != null && list.getLength() == 1) {
        fce = new FCE((Element)list.item(0));
      }
    }catch(Exception e){
    }

    try{
      list = element.getElementsByTagName(SYSTEM);
      if (list != null && list.getLength() == 1) {
        system = new SystemCapability((Element)list.item(0));
      }
    }catch(Exception e){
    }

    try{
      list = element.getElementsByTagName(SIP);
      if (list != null && list.getLength() == 1) {
        sip = new SipCapability((Element)list.item(0));
      }
    }catch(Exception e){
    }

    try{
      list = element.getElementsByTagName(LOGGING);
      if (list != null && list.getLength() == 1) {
        logging = new LoggingCapability((Element)list.item(0));
      }
    }catch(Exception e){
    }

    try{
      list = element.getElementsByTagName(H323);
      if (list != null && list.getLength() == 1) {
        h323 = new H323Capability((Element)list.item(0));
      }
    }catch(Exception e){
    }

    try {
      list = element.getElementsByTagName("tcs2833");
      if (list != null && list.getLength() == 1) {
        String status = ((Element)list.item(0)).getAttribute("status");
        setTcs2833Enabled("enabled".equals(status));
      }
    } catch (Exception e) {
      setTcs2833Enabled(false);
    }

    try {
      list = element.getElementsByTagName("cacheTimeout");
      if (list != null && list.getLength() == 1) {
        String status = ((Element)list.item(0)).getAttribute("status");
        setCacheTimeoutEnabled("enabled".equals(status));
      }
    } catch (Exception e) {
      setCacheTimeoutEnabled(false);
    }

    try {
      list = element.getElementsByTagName("removetg");
      if (list != null && list.getLength() == 1) {
        String status = ((Element)list.item(0)).getAttribute("status");
        setRemoveTGEnabled("enabled".equals(status));
      }
    } catch (Exception e) {
      setRemoveTGEnabled(false);
    }

    try{
      list = element.getElementsByTagName(REDUND);
      if (list != null && list.getLength() == 1) {
        redund = new RedundCapability((Element)list.item(0));
      }
    }catch(Exception e){
    }
  }



  public String toString () {
    try {
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
    }
    Element e = doc.getDocumentElement();
    if(e  != null){
      doc.removeChild((Node)e);
    }
    return sw.toString();
  }
  
  public Node getXMLRepresentation () throws DOMException {
    Element root = doc.createElement("Capabilities");
    root.setAttribute("version", "1.0");

    //interfaces
    if(interfaceNames !=  null){
	    for (int j = 0; j < interfaceNames.length; j++) {
	      Element tag = doc.createElement("Interface");
	      tag.appendChild(doc.createTextNode(interfaceNames[j]));
	      root.appendChild(tag);
	    }
    }
  
    //database related
    root.appendChild(database.getXMLRepresentation(doc));

    // realm enabled
    Element tag = doc.createElement("Realm");
    tag.setAttribute("status", isRealmEnabled()?"enabled":"disabled");
    root.appendChild(tag);

    //remove tg
    tag = doc.createElement("removetg");
    tag.setAttribute("status", isRemoveTGEnabled()?"enabled":"disabled");
    root.appendChild(tag);

    root.appendChild(billing.getXMLRepresentation(doc));
    root.appendChild(fce.getXMLRepresentation(doc));

    // tcs2833 and cache timeout are here for backward capatible with msw
    //tcs2833
    tag = doc.createElement("tcs2833");
    tag.setAttribute("status", isTcs2833Enabled()?"enabled":"disabled");
    root.appendChild(tag);

    //cache timeout
    tag = doc.createElement("cacheTimeout");
    tag.setAttribute("status", isCacheTimeoutEnabled()?"enabled":"disabled");
    root.appendChild(tag);

    root.appendChild(system.getXMLRepresentation(doc));
    root.appendChild(sip.getXMLRepresentation(doc));
    root.appendChild(logging.getXMLRepresentation(doc));
    root.appendChild(redund.getXMLRepresentation(doc));
    root.appendChild(h323.getXMLRepresentation(doc));

    return root;
  }

  private void setDefault(){
    billing = new Billing();    
    fce     = new FCE();
    system = new SystemCapability();
    sip    = new SipCapability();
    logging = new LoggingCapability();
    database = new DBCapability();
    redund = new RedundCapability();
    h323 = new H323Capability();

    interfaceNames  = null;
  }

  public Billing getBilling(){
    return billing;
  }

  public FCE getFCE(){
    return fce;
  }

  public SystemCapability getSystemCapability() {
    return system;
  }
 
  public SipCapability getSipCapability() {
    return sip;
  }

  public LoggingCapability getLoggingCapability() {
    return logging;
  }

  public DBCapability getDBCapability() {
    return database;
  }
    
  public RedundCapability getRedundCapability() {
    return redund;
  }

  public H323Capability getH323Capability() {
    return h323;
  }

  public String[] getInterfaceNames(){
    return interfaceNames;
  }

  public void setInterfaceNames(String[] interfaceNames){
    this.interfaceNames = interfaceNames;
  }

  public boolean isRealmEnabled(){
    return database.isRealmEnabled();
  }

  public void setRealmEnabled(boolean enabled){
    database.setRealmEnabled(enabled);
  }

  public boolean isVnetEnabled(){
    return database.isVnetEnabled();
  }

  public void setVnetEnabled(boolean enabled){
    database.setVnetEnabled(enabled);
  }

  public boolean isLinux(){
    return database.isLinux();
  }

  public void setLinux(boolean enabled){
    database.setLinux(enabled);
  }

  // tcs2833 and cache timeout are here for backward capatible with msw
  public boolean isTcs2833Enabled() {
    return system.isTcs2833Enabled();
  }

  public void setTcs2833Enabled(boolean enable) {
    system.setTcs2833Enabled(enable);
  }

  public boolean isCacheTimeoutEnabled() {
    return system.isCacheTimeoutEnabled();
  }

  public void setCacheTimeoutEnabled(boolean enable) {
    system.setCacheTimeoutEnabled(enable);
  }

  public boolean isRemoveTGEnabled() {
    return database.isRemoveTGEnabled();
  }

  public void setRemoveTGEnabled(boolean enable) {
    database.setRemoveTGEnabled(enable);
  }

  public void setCapabilitiesEnabled(boolean enable) {
    getFCE().setCapabilitiesEnabled(enable);
    getSystemCapability().setCapabilitiesEnabled(enable);
    getSipCapability().setCapabilitiesEnabled(enable);
    getDBCapability().setCapabilitiesEnabled(enable);
    getRedundCapability().setCapabilitiesEnabled(enable);
    getLoggingCapability().setCapabilitiesEnabled(enable);
    //set radius enable
    if (getBilling().radiusEnabled()) {
      getBilling().setCapabilitiesEnabled(enable);
    }
    getH323Capability().setCapabilitiesEnabled(enable);
  }

  public class SystemCapability extends AbstractCapability {
     private static final int MAX_CALL_DURATION = 0;
     private static final int TCS2833           = 1;
     private static final int CACHE_TIMEOUT     = 2;
     private static final int TCST38            = 3;
     private static final int MSW_NAME          = 4;
     private static final int DEFAULT_CODEC     = 5;
     private static final int MAX_HUNT_DURATION = 6;
     private static final int CODE_MAP = 7;
      

     SystemCapability() {
       super();
     }

     SystemCapability(Element element) {
       super(element);
     }

     protected Object[][] getCapabilityConfig() {
       if (config == null) {
          config = new Object[][] {
                     { "maxCallDuration", new Boolean(false) },
                     { "tcs2833",         new Boolean(false) },
                     { "cacheTimeout",    new Boolean(false) },
                     { "tcst38",          new Boolean(false) },
                     { "mswName",         new Boolean(false) },
                     { "defaultCodec",    new Boolean(false) },
                     { "maxHuntAllowableDuration", new Boolean(false) },
                     { "codeMap",         new Boolean(false) }
                   };
       }

       return config;
     }

     protected Element createRootElement(Document doc) {
       return doc.createElement(SYSTEM);
     }

     public boolean isMaxCallDurationEnabled() {
        return ((Boolean)config[MAX_CALL_DURATION][STATUS]).booleanValue();
     }

     public void setMaxCallDurationEnabled(boolean enable) {
        this.config[MAX_CALL_DURATION][STATUS] = new Boolean(enable);
     }
  
     public boolean isTcs2833Enabled() {
       return ((Boolean)config[TCS2833][STATUS]).booleanValue();
     }

     public void setTcs2833Enabled(boolean enable) {
       this.config[TCS2833][STATUS] = new Boolean(enable);
     }

     public boolean isTcsT38Enabled() {
       return ((Boolean)config[TCST38][STATUS]).booleanValue();
     }

     public void setTcsT38Enabled(boolean enable) {
       this.config[TCST38][STATUS] = new Boolean(enable);
     }

     public boolean isCacheTimeoutEnabled() {
       return ((Boolean)config[CACHE_TIMEOUT][STATUS]).booleanValue();
     }

     public void setCacheTimeoutEnabled(boolean enable) {
       this.config[CACHE_TIMEOUT][STATUS] = new Boolean(enable);
     }

     public boolean isMswNameEnabled() {
       return ((Boolean)config[MSW_NAME][STATUS]).booleanValue();
     }

     public void setMswNameEnabled(boolean enable) {
       this.config[MSW_NAME][STATUS] = new Boolean(enable);
     }

     public boolean isDefaultCodecEnabled() {
       return ((Boolean)config[DEFAULT_CODEC][STATUS]).booleanValue();
     }

     public void setDefaultCodecEnabled(boolean enable) {
       this.config[DEFAULT_CODEC][STATUS] = new Boolean(enable);
     }

     public boolean isMaxHuntAllowableDurationEnabled() {
       return ((Boolean)config[MAX_HUNT_DURATION][STATUS]).booleanValue();
     }

     public void setMaxHuntAllowableDurationEnabled(boolean enable) {
       this.config[MAX_HUNT_DURATION][STATUS] = new Boolean(enable);
     }
     public boolean isCodeMapEnabled() {
       return ((Boolean)config[CODE_MAP][STATUS]).booleanValue();
     }

     public void setCodeMapEnabled(boolean enable) {
       this.config[CODE_MAP][STATUS] = new Boolean(enable);
     }

  }

  public class SipCapability extends AbstractCapability {
     private static final int SIP_TIMERC = 0;
     private static final int SIP_PORT  = 1;
     private static final int SIP_NAT   = 2;
     private static final int SIP_PRIVACY   = 3;
     private static final int SIP_AUTH_RADIUS   = 4;
     private static final int SIP_DOMAIN   = 5;
     private static final int SIP_OBP = 6;

     SipCapability() { super(); }

     SipCapability(Element element) { super(element); }

     protected Object[][] getCapabilityConfig() {
       if (config == null) {
          config = new Object[][]
                   {
		       {"sipTimerC", new Boolean(false) },
		       {"sipPort", new Boolean(false) },
           {"sipNat", new Boolean(false) },
           {"sipPrivacy", new Boolean(false) },
           {"sipAuthRadius", new Boolean(false) },
           {"sipDomain", new Boolean(true) },
           {"sipOBP", new Boolean(false)}
                   };
       }

       return config;
     }

     protected Element createRootElement(Document doc) {
       return doc.createElement(SIP);
     }

     public boolean isSipTimerCEnabled() {
       return ((Boolean)config[SIP_TIMERC][STATUS]).booleanValue();
     }

     public void setSipTimerCEnabled(boolean enable) {
       this.config[SIP_TIMERC][STATUS] = new Boolean(enable);
     }
     
    public boolean isSipPortEnabled() {
	    return ((Boolean)config[SIP_PORT][STATUS]).booleanValue();
    }
    public void setSipPortEnabled(boolean enable) {
	    this.config[SIP_PORT][STATUS] = new Boolean(enable);
    }  

    public boolean isSipNatEnabled() {
	    return ((Boolean)config[SIP_NAT][STATUS]).booleanValue();
    }
    public void setSipNatEnabled(boolean enable) {
	    this.config[SIP_NAT][STATUS] = new Boolean(enable);
    }  
	public boolean isSipOBPEnabled() {
		 return ((Boolean)config[SIP_OBP][STATUS]).booleanValue();
	 }
	 public void setSipOBPEnabled(boolean enable) {
		 this.config[SIP_OBP][STATUS] = new Boolean(enable);
	 }  
    public boolean isSipPrivacyEnabled() {
	    return ((Boolean)config[SIP_PRIVACY][STATUS]).booleanValue();
    }
    public void setSipPrivacyEnabled(boolean enable) {
	    this.config[SIP_PRIVACY][STATUS] = new Boolean(enable);
    }  

    public boolean isSipAuthRadiusEnabled() {
	    return ((Boolean)config[SIP_AUTH_RADIUS][STATUS]).booleanValue();
    }
    public void setSipAuthRadiusEnabled(boolean enable) {
	    this.config[SIP_AUTH_RADIUS][STATUS] = new Boolean(enable);
    }  
    public boolean isSipDomainEnabled() {
	    return ((Boolean)config[SIP_DOMAIN][STATUS]).booleanValue();
    }
    public void setSipDomainEnabled(boolean enable) {
	    this.config[SIP_DOMAIN][STATUS] = new Boolean(enable);
    }  


  }
  
  public class LoggingCapability extends AbstractCapability {
     private static final int SDEBUG = 0;
     private static final int ISPD   = 1; 
     private static final int SCM   = 2; 

     LoggingCapability() { super(); }

     LoggingCapability(Element element) { super(element); }

     protected Object[][] getCapabilityConfig() {
       if (config == null) {
          config = new Object[][]
                   {
                     {"sdebugLevel", new Boolean(false) }, 
                     {"ispd", new Boolean(false) },
                     {"scm", new Boolean(false) }
                   };
       }

       return config;
     }

     protected Element createRootElement(Document doc) {
       return doc.createElement(LOGGING);
     }

     public boolean isSDebugEnabled() {
       return ((Boolean)config[SDEBUG][STATUS]).booleanValue();
     }

     public void setSDebugEnabled(boolean enable) {
      this.config[SDEBUG][STATUS] = new Boolean(enable);
     }

     public boolean isISPDEnabled() {
       return ((Boolean)config[ISPD][STATUS]).booleanValue();
     }

     public void setISPDEnabled(boolean enable) {
      this.config[ISPD][STATUS] = new Boolean(enable);
     }

     public boolean isSCMEnabled() {
       return ((Boolean)config[SCM][STATUS]).booleanValue();
     }

     public void setSCMEnabled(boolean enable) {
      this.config[SCM][STATUS] = new Boolean(enable);
     }

  }

  public class Billing extends AbstractCapability {
    private static final int RADIUS  = 0;
    private static final int SENDMSG = 1;
    private static final int USEOSIF = 2;

    Billing(){
      super();
    }

    Billing(Element element){
      super(element);
    }

    protected Object[][] getCapabilityConfig() {
     if (config == null) {
        config = new Object[][]
                 {
                   {"Radius",  new Boolean(false) },
                   {"sendMsg", new Boolean(false) },
                   {"useOSIF", new Boolean(false) }
                 };
     }

      return config;
    }

    protected Element createRootElement(Document doc) {
      return doc.createElement(BILLING);
    }

    public void setRadiusEnabled(boolean enabled){
      this.config[RADIUS][STATUS] = new Boolean(enabled);
    }

    public boolean radiusEnabled(){
      return ((Boolean)config[RADIUS][STATUS]).booleanValue();
    }

    public boolean sendMsgEnabled() {
      return ((Boolean)config[SENDMSG][STATUS]).booleanValue();
    }

    public void setSendMsgEnabled(boolean enable) {
      this.config[SENDMSG][STATUS] = new Boolean(enable);
    }

    public boolean useOSIFEnabled() {
      return ((Boolean)config[USEOSIF][STATUS]).booleanValue();
    }

    public void setUseOSIFEnabled(boolean enable) {
      this.config[USEOSIF][STATUS] = new Boolean(enable);
    }

  }

  public class DBCapability extends AbstractCapability {
      private static final int REFRESH_IEDGE = 0; 
      private static final int REMOVETG      = 1;
      private static final int REALM         = 2;
      private static final int IGRP          = 3;
      private static final int STICKY_ROUTE  = 4;
      private static final int DTG           = 5;
      private static final int OVERRIDE      = 6;
      private static final int PI            = 7;
      private static final int CAP2833       = 8;
      private static final int ISDN_MAPCC = 9;
      private static final int DCR = 10;
      private static final int REALM_SIP_CID = 11;
      private static final int VIP_NOT_USED      = 12;
      private static final int REALM_MIRROR_PROXY  = 13;
      private static final int TRACE_ROUTE_BY_REALM_ANI = 14;
      private static final int VNET         = 15;
      private static final int LINUX         = 16;
      
      DBCapability() { super(); }

      DBCapability(Element element) { super(element); }

     protected Object[][] getCapabilityConfig() {
       if (config == null) {
          config = new Object[][]
                   {
                     {"refreshIEdge", new Boolean(false) },
                     {"removetg", new Boolean(false) },
                     {"Realm", new Boolean(false) },
                     {"Igrp", new Boolean(false) },
                     {"sticky", new Boolean(false) },
                     {"dtg", new Boolean(false) },
                     {"override", new Boolean(false) },
                     {"pi", new Boolean(false) },
                     {"capable2833", new Boolean(false)},
		     {"ISDNMapcc", new Boolean( false )}, 
		     {"dcr", new Boolean(false)},
		     {"realm_sip_cid", new Boolean(false)},
		     {"vip_not_used", new Boolean(false)},
                     {"realm_mirror_proxy",new Boolean(false)},
                     {"trace_route_by_realm_ani",new Boolean(false)},
                     {"Vnet", new Boolean(false) },
                     {"Linux", new Boolean(false) }
                   };
       }
       return config;
     }

     protected Element createRootElement(Document doc) {
       return doc.createElement(DB);
     }

     public boolean isRealmEnabled() {
       return ((Boolean)config[REALM][STATUS]).booleanValue();
     }

     public void setRealmEnabled(boolean enable) {
       this.config[REALM][STATUS] = new Boolean(enable);
     }
 
     public boolean isVnetEnabled() {
       return ((Boolean)config[VNET][STATUS]).booleanValue();
     }

     public void setVnetEnabled(boolean enable) {
       this.config[VNET][STATUS] = new Boolean(enable);
     }
 
     public boolean isLinux() {
       return ((Boolean)config[LINUX][STATUS]).booleanValue();
     }

     public void setLinux(boolean enable) {
       this.config[LINUX][STATUS] = new Boolean(enable);
     }
 
     public boolean isRemoveTGEnabled() {
       return ((Boolean)config[REMOVETG][STATUS]).booleanValue();
     }

     public void setRemoveTGEnabled(boolean enable) {
       this.config[REMOVETG][STATUS] = new Boolean(enable);
     }

     public boolean isRefreshIEdgeEnabled() {
       return ((Boolean)config[REFRESH_IEDGE][STATUS]).booleanValue();
     }

     public void setRefreshIEdgeEnabled(boolean enable) {
       this.config[REFRESH_IEDGE][STATUS] = new Boolean(enable);
     }

     public boolean isIGRPEnabled() {
       return ((Boolean)config[IGRP][STATUS]).booleanValue();
     }

     public void setIGRPEnabled(boolean enable) {
       this.config[IGRP][STATUS] = new Boolean(enable);
     }

     public boolean isStickyRouteEnabled() {
       return ((Boolean)config[STICKY_ROUTE][STATUS]).booleanValue();
     }

     public void setStickyRouteEnabled(boolean enable) {
       this.config[STICKY_ROUTE][STATUS] = new Boolean(enable);
     }

     public boolean isDtgEnabled() {
       return ((Boolean)config[DTG][STATUS]).booleanValue();
     }

     public void setDtgEnabled(boolean enable) {
       this.config[DTG][STATUS] = new Boolean(enable);
     }

     public void setOverrideEnabled(boolean enable) {
       this.config[OVERRIDE][STATUS] = new Boolean(enable);
     }

     public boolean isOverrideEnabled() {
       return ((Boolean)config[OVERRIDE][STATUS]).booleanValue(); 
     }

     public boolean isPIEnabled() {
       return ((Boolean)config[PI][STATUS]).booleanValue();
     }

     public void setPIEnabled(boolean enable) {
       this.config[PI][STATUS] = new Boolean(enable);
     }

     public boolean isISDNMapccEnabled() {
       return ((Boolean)config[ISDN_MAPCC][STATUS]).booleanValue();
     }

     public void setISDNMapccEnabled(boolean enable) {
       this.config[ISDN_MAPCC][STATUS] = new Boolean(enable);
     }

     public boolean isCAP2833Enabled() {
       return ((Boolean)config[CAP2833][STATUS]).booleanValue();
     }

     public void setCAP2833Enabled(boolean enable) {
       this.config[CAP2833][STATUS] = new Boolean(enable);
     }
     public boolean isDCREnabled(){
	  return ((Boolean)config[DCR][STATUS]).booleanValue();
      }
      public void setDCREnabled(boolean enable) {
       this.config[DCR][STATUS] = new Boolean(enable);
     }

     public boolean isRealmSip_CidEnabled(){
        return ((Boolean)config[REALM_SIP_CID][STATUS]).booleanValue();
     }

     public void setRealmSip_CidEnabled(boolean enable) {
       this.config[REALM_SIP_CID][STATUS] = new Boolean(enable);
     }

     public boolean isVipNotUsed(){
        return ((Boolean)config[VIP_NOT_USED][STATUS]).booleanValue();
     }

     public void setVipNotUsed(boolean enable) {
       this.config[VIP_NOT_USED][STATUS] = new Boolean(enable);
     }

     public boolean isMirrorProxyEnabled(){
       return ((Boolean)config[REALM_MIRROR_PROXY][STATUS]).booleanValue();
     }

     public void setMirrorProxyEnabled(boolean enable) {
       this.config[REALM_MIRROR_PROXY][STATUS] = new Boolean(enable);
     }

     public boolean isTraceRouteByRealmAni(){
        return ((Boolean)config[TRACE_ROUTE_BY_REALM_ANI][STATUS]).booleanValue();
     }

     public void setTraceRouteByRealmAni(boolean enable) {
       this.config[TRACE_ROUTE_BY_REALM_ANI][STATUS] = new Boolean(enable);
     }


  }

  public class H323Capability extends AbstractCapability {
     private static final int H245_TUNNEL    = 0;
     private static final int ALLOW_AUTH_ARQ = 1;

     H323Capability() { super(); }

     H323Capability(Element element) { super(element); }

     protected Object[][] getCapabilityConfig() {
       if (config == null) {
          config = new Object[][]
                   {
                     {"h245Tunnel", new Boolean(false) },
                     {"allowAuthArq", new Boolean(false) }
                   };
       }

       return config;
     }

     protected Element createRootElement(Document doc) {
       return doc.createElement(H323);
     }

     public boolean isH245TunnelEnabled() {
       return ((Boolean)config[H245_TUNNEL][STATUS]).booleanValue();
     }

     public void set245TunnelEnabled(boolean enable) {
       this.config[H245_TUNNEL][STATUS] = new Boolean(enable);
     }

     public boolean isAllowAuthArqEnabled() {
       return ((Boolean)config[ALLOW_AUTH_ARQ][STATUS]).booleanValue();
     }

     public void setAllowAuthArqEnabled(boolean enable) {
       this.config[ALLOW_AUTH_ARQ][STATUS] = new Boolean(enable);
     }
  }

  public class FCE extends AbstractCapability {
    private static final int POOL = 0;
    private static final int MFCP = 1;
    private static final int HOTKNIFE = 2;

    FCE() { super(); }

    FCE(Element element) { super(element); }

    protected Object[][] getCapabilityConfig() {
     if (config == null) {
        config = new Object[][]
                 {
                   {"PoolConfig",  new Boolean(false) },
                   {"mfcp", new Boolean(false) },
                   {"HotKnife", new Boolean(false) }
                 };
     }

      return config;
    }

    protected Element createRootElement(Document doc) {
      return doc.createElement(FCE);
    }

    public void setPoolConfig(boolean enabled){
      this.config[POOL][STATUS] = new Boolean(enabled);
    }

    public boolean isPoolConfig(){
      return ((Boolean)config[POOL][STATUS]).booleanValue();
    }

    public void setMFCPEnabled(boolean enabled){
      this.config[MFCP][STATUS] = new Boolean(enabled);
    }

    public boolean isMFCPEnabled(){
      return ((Boolean)config[MFCP][STATUS]).booleanValue();
    }
    
	public void setHotKnifeEnabled(boolean enabled){
	   this.config[HOTKNIFE][STATUS] = new Boolean(enabled);
	 }

	 public boolean isHotKnifeEnabled(){
	   return ((Boolean)config[HOTKNIFE][STATUS]).booleanValue();
	 }
  }

 public class RedundCapability extends AbstractCapability {
     private static final int SECONDARY_VIP = 0;
     private static final int SCM = 1;

     RedundCapability() { super(); }

     RedundCapability(Element element) { super(element); }

     protected Object[][] getCapabilityConfig() {
       if (config == null) {
          config = new Object[][]
                   {
                     {"secondaryVip", new Boolean(false) },
                     {"scm", new Boolean(false)}
                   };
       }
       return config;
     }

     protected Element createRootElement(Document doc) {
       return doc.createElement(REDUND);
     }

     public boolean isSecondaryVipEnabled() {
       return ((Boolean)config[SECONDARY_VIP][STATUS]).booleanValue();
     }

     public void setSecondaryVipEnabled(boolean enable) {
    	 this.config[SECONDARY_VIP][STATUS] = new Boolean(enable);
    }

     public boolean isScmEnabled() {
       return ((Boolean)config[SCM][STATUS]).booleanValue();
     }

     public void setScmEnabled(boolean enable) {
    	 this.config[SCM][STATUS] = new Boolean(enable);
    }

  }

  public abstract class AbstractCapability {
    public static final int NAME   = 0;
    public static final int STATUS = 1;

    protected Object[][] config;

    public AbstractCapability() {
      getCapabilityConfig();
    }

    public AbstractCapability (Element element)  {
      this();
      init(element);
    }

    protected void init(Element element) {
      for (int i = 0; i < config.length; i++) {
        try {
          NodeList list = element.getElementsByTagName((String)config[i][NAME]);
          if (list != null && list.getLength() == 1) {
             String status = ((Element)list.item(0)).getAttribute("status");
             config[i][STATUS] = new Boolean("enabled".equals(status));
          }
        } catch (Exception e) {
           config[i][STATUS] = new Boolean(false);
        }
      }
    }

    public synchronized Node getXMLRepresentation (Document doc) throws DOMException {
      Element root = createRootElement(doc);

      for (int i = 0; i < config.length; i++) {
        Element tag = doc.createElement((String)config[i][NAME]);
        tag.setAttribute("status", ((Boolean)config[i][STATUS]).booleanValue()?"enabled":"disabled");
        root.appendChild(tag);
      }

      return root;
    }

    protected abstract Element createRootElement(Document doc);

    public void setCapabilitiesEnabled(boolean enable) {
       for (int i = 0; i < config.length; i++) {
         config[i][STATUS] = new Boolean(enable);
       }
    }

    protected Object[][] getCapabilityConfig() {
      config = new Object[0][0];
      return config;
    }
  }
}
