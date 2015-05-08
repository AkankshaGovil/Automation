/*
 * This class is passed serialized between the iServer and iView. The
 * 'serialVersionUID' is used to sync the object between JDK1.2 on linux
 * iServer and JDK1.3 on iView. Any modifications made to this class should
 * be done very carefully to maintain backward compatibility. Changing the
 * 'serialVersionUID' field would result in incompatible serialized objects.
 * In fact, the preferred method to maintain compatibility between iView and
 * iServer code is by using that field.
 *
 * The following is an example of how an addition could be made to this class
 * without affecting compatibility:
 * 
 *    boolean newClassFieldPresent = true;
 *    Object newClassField;
 *    ....
 *    public void newMethod () {
 *        newClassField = ...;
 *    }
 *
 *    Any new code written which uses the newClassField or the newMethod,
 *    should check the newClassFieldPresent variable before accessing those.
 *    Older version of the object, when deserialized, would have the boolean
 *    flag set as false, while the latest objects would have it true.
 *
 * Any other changes that might break the compatibility needs to update the
 * serialVersionUID field, so that the error message shown would be graceful.
 */

package com.nextone.common;

import java.io.*;
import java.text.*;
import java.util.*;
import javax.swing.tree.*;
import com.nextone.util.IPUtil;

/**
 * class which holds all the details about an endpoint (as stored in the
 * iserver)
 */ 
  public class IEdgeList implements Serializable, Comparable, Cloneable, DropStatusSource,DBObject, CommonConstants {

  private volatile RegidPortPair rpp;

  // This field is really the registration id on the iedge. We are not changing the field
  // name here, because it would cause backward incompatibilities
  private  String serialNumber;
  private  String extNumber;//            // Stores extension number
  private  String phone; //
  private  String firstName;
  private  String lastName;
  private  String location;
  private  String country;
  private  String comments;
  private  String customerId;
  private  String trunkGroup;
  private  String zone;
  private  String email; //
  private  String forwardedPhone; //
  private  String forwardedVpnPhone; //
  private  String callingPlanName;
  private  String h323Id;//
  private  String sipUri;//
  private  String sipContact;//
  private  String techPrefix;//
  private  String peerGkId;//
  private  String h235Password;//
  private  String vpnName;
  private  String ogp;
  private  String srcIngressTg;
  private  String realmName;  
  private  String igrpName;
  private  String dtg;
  private  String newSrcDtg;
  private  boolean proxyValid;//
  private  boolean isProxied;//
  private  boolean isProxying;//
  private  boolean callForwarded;//
  private  boolean isCallRollover;//
  private  boolean ad; //                 // accepting drop?
//  private  boolean isDestTg;

  private int devType;
  private int port;//
  private int ipaddr;
  private int forwardedVpnExtLen; 
  private int maxCalls;
  private int maxInCalls;
  private int maxOutCalls;
  private int priority;//
  private int rasPort = CommonConstants.RASPORT;
  private int q931Port = CommonConstants.Q931PORT;
  private int callpartyType;
  private int currentCalls;
  private int vendor;
  private int extLen;
  private int subnetip;//
  private int subnetmask;  //
  private int maxHunts;  
  private int extCaps;
  private int callType;  //
  private short caps;   // capabilities 
  private int stateFlags;  //
  private int layer1Protocol;
  private int infoTransCap;
  private int natIp;
  private int natPort;
  private  long inceptionTime; //
  private  long refreshTime;  //
  private  static final long serialVersionUID = 33369047652010001L;

  public   static final String [] callpartyCommand = {
    "none",
    "unknown",
    "international",
    "national",
    "specific",
    "subscriber",
    "abbreviated"
  };

  public   static final String [] callpartyDescription = {
    "System Default",
    "Unknown",
    "International",
    "National",
    "Specific",
    "Subscriber",
    "Abbreviated"
  };

  //  The following values are copied from  serverdb.h. Do not change them.
  public static final int Q931CDPN_None			= 0;
  public static final int Q931CDPN_Unknown 	=	1; // Default behavior 
  public static final int Q931CDPN_International =	2;
  public static final int Q931CDPN_National		= 3;
  public static final int Q931CDPN_Specific		= 4;
  public static final int Q931CDPN_Subscriber	= 5;
  public static final int Q931CDPN_Abbreviated = 7;

  public static final String[] layer1ProtocolCommand = {
    "default",
    "g711ulaw",
    "g711alaw",
    "h221",
    "pass",
  };

  public static final String[] layer1ProtocolDescription = {
    "System Default",
    "G711ulaw",
    "G711alaw",
    "H221",
    "Pass-Through",
  };

  //  The following values are copied from  serverdb.h. Do not change them.
  public static final int BCAPLAYER1_Default	= 0;
  public static final int BCAPLAYER1_G711ulaw	= 2;
  public static final int BCAPLAYER1_G711alaw	= 3;
  public static final int BCAPLAYER1_H221	= 5;
  public static final int BCAPLAYER1_Pass	= 33;

  //  sip Privacy
  public static String[] sipPrivacy = {
    "rfc3325",
    "draft01",
    "both"
  };

  public static final int SIP_PRIVACY_RFC3325 = 0;
  public static final int SIP_PRIVACY_DRAFT01 = 1;
  public static final int SIP_PRIVACY_BOTH = 2;


  public IEdgeList() {
    resetAll();
    serialNumber= "";
  }

  // do not update this. Not used from iserver_db21
  // this constructor is used in BridgeServerImpl.c
  public IEdgeList(
    int port, 
    int ipaddr,
    int extLen,
    int forwardedVpnExtLen,
    int devType,
    int maxCalls,
    int maxInCalls,
    int maxOutCalls,
    int currentCalls,
    int priority,
    int rasPort,
    int q931Port,
    int callpartyType,
    int vendor,
    int subnetip,
    int subnetmask,
    int maxHunts,
    int extCaps,
    int layer1Protocol,
    long incepTime,
    long refreshTime,
    String serialNumber,
    String vpnName,
    String extNumber,
    String phone,
    String firstName,
    String lastName,
    String location,
    String country,
    String comments,
    String customerId,
    String callingPlanName,
    String zone,
    String email,
    String forwardedPhone,
    String h323Id,
    String sipUri,
    String sipContact,
    String forwardedVpnPhone,
    String techPrefix,
    String peerGkId,
    String h235Password,
    boolean proxyValid,
    boolean isProxied,
    boolean isProxying,
    boolean callForwarded,
    boolean isCallRollover,
    short caps,
    int sflags,
    String tg,
    String ogp,
    int infoTransCap,
    String srcIngressTg,
    String igrpName,
    String dtg,
    String newSrcDtg
    ) {
    this.extLen         = extLen;
    this.vpnName        = vpnName;
    this.serialNumber   = serialNumber;
    this.port           = port;
    this.inceptionTime  = incepTime;
    this.refreshTime    = refreshTime;
    this.ipaddr         = ipaddr;
    this.maxHunts       = maxHunts;
    this.extCaps        = extCaps;
    this.extNumber      = extNumber;
    this.phone          = phone;
    this.firstName      = firstName;
    this.lastName       = lastName;
    this.location       = location;
    this.country        = country;
    this.comments       = comments;
    this.customerId     = customerId;
    this.callingPlanName      = callingPlanName;
    this.zone                 = zone;
    this.email                = email;
    this.proxyValid           = proxyValid;
    this.isProxied            = isProxied;
    this.isProxying           = isProxying;
    this.callForwarded        = callForwarded;
    this.isCallRollover       = isCallRollover;
    this.forwardedPhone       = forwardedPhone;
    this.forwardedVpnPhone    = forwardedVpnPhone;
    this.forwardedVpnExtLen   = forwardedVpnExtLen;
    this.devType      = devType;
    this.h323Id       = h323Id;
    this.sipUri       = sipUri;
    this.sipContact   = sipContact;
    this.maxCalls     = maxCalls;
    this.maxInCalls   = maxInCalls;
    this.maxOutCalls  = maxOutCalls;
    this.currentCalls = currentCalls;
    this.priority     = priority;
    this.rasPort      = rasPort;
    this.q931Port     = q931Port;
    this.callpartyType  = callpartyType;
    this.techPrefix         = techPrefix;
    this.peerGkId           = peerGkId;
    this.vendor             = vendor;
    this.subnetip           = subnetip;
    this.subnetmask         = subnetmask;
    this.h235Password       = h235Password;
    this.caps = caps;
    this.stateFlags = sflags;
    this.trunkGroup = tg;
    this.ogp = ogp;
    this.layer1Protocol = layer1Protocol;
    this.infoTransCap = infoTransCap;
    this.srcIngressTg = srcIngressTg;
    this.igrpName     = igrpName;
    this.dtg          = dtg;
    this.newSrcDtg    = newSrcDtg;
  }

public IEdgeList(
    int port, 
    int ipaddr,
    int extLen,
    int forwardedVpnExtLen,
    int devType,
    int maxCalls,
    int maxInCalls,
    int maxOutCalls,
    int currentCalls,
    int priority,
    int rasPort,
    int q931Port,
    int callpartyType,
    int vendor,
    int subnetip,
    int subnetmask,
    int maxHunts,
    int extCaps,
    int layer1Protocol,
    long incepTime,
    long refreshTime,
    String serialNumber,
    String vpnName,
    String extNumber,
    String phone,
    String firstName,
    String lastName,
    String location,
    String country,
    String comments,
    String customerId,
    String callingPlanName,
    String zone,
    String email,
    String forwardedPhone,
    String h323Id,
    String sipUri,
    String sipContact,
    String forwardedVpnPhone,
    String techPrefix,
    String peerGkId,
    String h235Password,
    boolean proxyValid,
    boolean isProxied,
    boolean isProxying,
    boolean callForwarded,
    boolean isCallRollover,
    short caps,
    int sflags,
    String tg,
    String ogp,
    int infoTransCap,
    String srcIngressTg,
    String realmName,
    String igrpName,
    String dtg,
    String newSrcDtg,
    int natIp,
    int natPort
    ) {
    this.extLen         = extLen;
    this.vpnName        = vpnName;
    this.serialNumber   = serialNumber;
    this.port           = port;
    this.inceptionTime  = incepTime;
    this.refreshTime    = refreshTime;
    this.ipaddr         = ipaddr;
    this.maxHunts       = maxHunts;
    this.extCaps        = extCaps;
    this.extNumber      = extNumber;
    this.phone          = phone;
    this.firstName      = firstName;
    this.lastName       = lastName;
    this.location       = location;
    this.country        = country;
    this.comments       = comments;
    this.customerId     = customerId;
    this.callingPlanName      = callingPlanName;
    this.zone                 = zone;
    this.email                = email;
    this.proxyValid           = proxyValid;
    this.isProxied            = isProxied;
    this.isProxying           = isProxying;
    this.callForwarded        = callForwarded;
    this.isCallRollover       = isCallRollover;
    this.forwardedPhone       = (forwardedPhone == null)?"":forwardedPhone;
    this.forwardedVpnPhone    = (forwardedVpnPhone == null)?"":forwardedVpnPhone;
    this.forwardedVpnExtLen   = forwardedVpnExtLen;
    this.devType      = devType;
    this.h323Id       = h323Id;
    this.sipUri       = sipUri;
    this.sipContact   = sipContact;
    this.maxCalls     = maxCalls;
    this.maxInCalls   = maxInCalls;
    this.maxOutCalls  = maxOutCalls;
    this.currentCalls = currentCalls;
    this.priority     = priority;
    this.rasPort      = rasPort;
    this.q931Port     = q931Port;
    this.callpartyType  = callpartyType;
    this.techPrefix         = techPrefix;
    this.peerGkId           = peerGkId;
    this.vendor             = vendor;
    this.subnetip           = subnetip;
    this.subnetmask         = subnetmask;
    this.h235Password       = h235Password;
    this.caps = caps;
    this.stateFlags = sflags;
    this.trunkGroup = tg;
    this.ogp = ogp;
    this.layer1Protocol = layer1Protocol;
    this.infoTransCap = infoTransCap;
    this.srcIngressTg = srcIngressTg;
    this.realmName    = realmName;
    this.igrpName     = igrpName;
    this.dtg          = dtg;
    this.newSrcDtg    = newSrcDtg;
    this.natIp  = natIp;
    this.natPort  = natPort;
  }

  public IEdgeList(String sno, int dType,String vpnName){
    this();
    serialNumber  = (sno  ==  null)?"":sno;
    devType       = dType;
    vpnName = vpnName;
  }



  public void dump () {
    dump(new PrintWriter(System.out));
  }

  /**
   * writes the output to the given Writer
   * uses a Writer instead of PrintWriter or PrintStream so that this
   * can be used with the JspWriter also
   */
  public void dump (Writer pw) {
    try {
      pw.write("Serial Number: " + serialNumber + "\n");
      pw.write("Port: " + port + "\n");
      SimpleDateFormat df = new SimpleDateFormat("EEE MMM dd hh:mm:ssa yyyy z");
      pw.write("Inception Time: " + df.format(new Date(inceptionTime*1000)) + "\n");
      pw.write("Refresh Time: " + df.format(new Date(refreshTime*1000)) + "\n");
      pw.write("IP Address: " + IPUtil.intToIPString(ipaddr) + "\n");
      pw.write("Name: " + firstName + " " + lastName + "\n");
      pw.write("Location: " + location + "\n");
      pw.write("Country: " + country + "\n");
      pw.write("Comments: " + comments + "\n");
      pw.write("Calling Plan Name: " + callingPlanName + "\n");
      pw.write("Zone: " + zone + "\n");
      pw.write("Phone: " + getPhoneNumber()+ "\n");
      pw.write("Email: " + email + "\n");
      pw.write("Max Hunts: " + maxHunts + "\n");
      pw.write("Q.931 Display: " + isQ931DisplayEnabled() + "\n");
      pw.write("Connect H.245 Addr: " + isConnectH245AddressEnabled() + "\n");
      if (proxyValid) {
	      pw.write("Proxied ");
	      if (isProxied)
	        pw.write("(Present)\n");
	      else
	        pw.write("(Absent)\n");
      }
      if (isProxying)
        pw.write("Proxying\n");
      if (callForwarded) {
	      pw.write("Forwarding " );
	      if (isCallRollover)
	        pw.write("(Rollover) ");
	      pw.write("ON\n");
	      if (!forwardedPhone.equals(""))
	        pw.write("Fwd Phone: " + forwardedPhone + "\n");
	      if (!forwardedVpnPhone.equals(""))
	        pw.write("Fwd VPN Phone: " + forwardedVpnPhone + "/" + forwardedVpnExtLen + "\n");
      }
      if (getDND().booleanValue())
    	  pw.write("DND on\n");
      pw.write("Device Type: " + getDeviceTypeString());
      if (isGateway())
	      pw.write(" (Gateway)");
      pw.write("\n");
      if (isRegistered())
	      pw.write("Currently Registered\n");
      else
	      pw.write("Currently Not Registered\n");
      pw.write("\n");
    } catch (IOException ie) {
      ie.printStackTrace();
    }
  }

  public synchronized String getSerialNumber () {
    return serialNumber;
  }

  public synchronized void setSerialNumber (String s) {
    serialNumber = s;
  }

  public String getSerialNumberPort () {
    return getSerialNumber() + "/" + (getPort()+1);
  }

  public synchronized int getPort () {
    return port;
  }

  public synchronized void setPort (int p) {
    port = p;
  }

  public synchronized int getAddressAsInt () {
    return ipaddr;
  }

  public synchronized String getAddress () {
    if (ipaddr == 0)
      return "";

    return IPUtil.intToIPString(ipaddr);
  }

  public synchronized void setAddress (String addr) {
    if (addr.equals(""))
      ipaddr = 0;
    ipaddr = (int)IPUtil.ipStringToLong(addr);
  }

  public synchronized String getPhoneNumber() {
    return phone;
  }

  public synchronized void setPhoneNumber (String phone) {
    this.phone  = (phone  ==  null)?"":phone;
  }


  public synchronized void  setForwardedVpnExtLen(int len){
      forwardedVpnExtLen  = len;
  }
  public synchronized String getVpnName () {
    return vpnName;
  }

  public synchronized void setVpnName (String s) {
    vpnName = s;
  }

  public synchronized String getExtNumber () {
    return extNumber;
  }

  public synchronized void setExtNumber (String s) {
    extNumber = s;
  }

  public synchronized int getExtLen () {
    return extLen;
  }

  public synchronized void setExtLen (int i) {
    extLen  = i;
  }

  public synchronized String getFirstName () {
    return firstName;
  }

  public synchronized void setFirstName (String s) {
    firstName = s;
  }

  public synchronized String getLastName () {
    return lastName;
  }

  public synchronized void setLastName (String s) {
    lastName = s;
  }

  public synchronized String getFullName (String def) {
    StringBuffer sb = new StringBuffer();
    if (firstName != null && !firstName.equals(""))
      sb.append(firstName);
    if (lastName != null && !lastName.equals(""))
      sb.append(" " + lastName);
    String userName = sb.toString();
    if (userName.equals(""))
      userName = def;

    return userName;
  }

  public synchronized String getLocation () {
    return location;
  }

  public synchronized void setLocation (String s) {
    location = s;
  }

  public synchronized String getCountry () {
    return country;
  }

  public synchronized void setCountry (String s) {
    country = s;
  }

  public synchronized String getCustomerId () {
    return customerId;
  }

  public synchronized void setCustomerId (String s) {
    customerId = s;
  }

  public synchronized String getTrunkGroup () {
    return trunkGroup;
  }

  public synchronized void setTrunkGroup (String s) {
    trunkGroup = s;
  }

  public synchronized String getOutgoingPrefix () {
    return ogp;
  }

  public synchronized void setOutgoingPrefix (String s) {
    ogp = s;
  }

  public synchronized String getZone () {
    return zone;
  }

  public synchronized void setZone (String s) {
    zone = s;
  }

  public synchronized String getEmail () {
    return email;
  }

  public synchronized void setEmail (String s) {
    email = s;
  }

  public synchronized String getComments () {
    return comments;
  }

  public synchronized void setComments (String s) {
    comments = s;
  }

  public synchronized String getCallingPlanName () {
    return (callingPlanName == null)?"":callingPlanName;
  }

  public synchronized void setCallingPlanName (String s) {
    callingPlanName = s;
  }

  public Date getInceptionTime () {
    return new Date(inceptionTime*1000);
  }

  public void setInceptionTime (long time) {
    inceptionTime = time;
  }


  public Date getRefreshTime () {
    return new Date(refreshTime*1000);
  }

  public void setRefreshTime (long time) {
    refreshTime = time;
  }


  public String getProxy () {
    if (proxyValid) {
      if (isProxied)
	return "Proxied (Present)";
      else
	return "Proxied (Absent)";
    }
    if (isProxying)
      return "Proxying";

    return "";
  }

  public void setProxyValid(boolean isValid){
    proxyValid  = isValid;
  }

  public void setProxying(boolean isProxying){
    this.isProxying  = isProxying;
  }

  public String getForwardedPhone () {
    return (forwardedPhone == null)?"": forwardedPhone;
  }

  public void setForwardedPhone(String phone){
      forwardedPhone  = phone;
  }

  public boolean isCallForwarded () {
    return callForwarded;
  }

  public void setCallForwarded(boolean isCallForward){
    callForwarded = isCallForward;
  }


  public boolean isCallRolledOver () {
    return (callForwarded && isCallRollover);
  }

  public void setCallRollOver(boolean isCallRollOver){
    this.isCallRollover  = isCallRollover;
  }

  public void setCallType(int callType){
    this.callType = callType;
  }
  public int getCallType(){
    if(callForwarded){
      if (isCallRollover)
        return CommonConstants.CALLROLLOVER;
      else
        return CommonConstants.CALLFORWARD;
    }
    else
    return CommonConstants.NO_CALLFORWARD;
  }

  public String getCallForwarding () {
    if (callForwarded) {
      StringBuffer s = new StringBuffer("Forwarding ");
      if (isCallRollover)
	s.append("(Rollover) ");
      s.append("ON  ");
      if (!forwardedPhone.equals(""))
	s.append("Ph: " + forwardedPhone + "  ");
      if (!forwardedVpnPhone.equals(""))
	s.append("vpn Ph: " + forwardedVpnPhone);
      return new String(s);
    }

    return "";
  }

  public void setForwardedVpnPhone(String phone){
    forwardedVpnPhone = (phone  ==  null)?"":phone;
  }

  public Boolean getDND () {
    return new Boolean((stateFlags & CommonConstants.CL_DND) != 0);
  }

  public void setDND(boolean isDnd) {
    stateFlags |= CommonConstants.CL_DND;
  }

  public boolean isGateway () {
    return CommonFunctions.BIT_TEST(caps, CommonConstants.CAP_IGATEWAY);
  }

  public void setGateway(boolean isGateway){
    if(isGateway)
      caps  = CommonFunctions.BIT_SET(caps,CommonConstants.CAP_IGATEWAY);
    else
      caps  = CommonFunctions.BIT_RESET(caps,CommonConstants.CAP_IGATEWAY);

  }

  public int getDeviceType () {
    return devType;
  }
  public void setDeviceType(int dType){
    devType = dType;
  }


  /**
   * returns a string for a device type
   * (any change in the following string representations may have adverse
   * effect in a lot of code logics)
   *
   * @param dt the device type
   * @return a string representation of the device type
   */
  public static String getDeviceTypeString (int dt) {
    switch (dt) {
    case CommonConstants.DEVTYPE_I500:
      return "iEdge 500";

    case CommonConstants.DEVTYPE_I510:
      return "iEdge 510";

    case CommonConstants.DEVTYPE_I1000:
      return "iEdge 1000";

    case CommonConstants.DEVTYPE_OSP:
      return "OSP Server";

    case CommonConstants.DEVTYPE_XGW:
      return "H.323 Gateway";

    case CommonConstants.DEVTYPE_ISERVER:
      return "iServer";

    case CommonConstants.DEVTYPE_XGK:
      return "H.323 Gatekeeper";

    case CommonConstants.DEVTYPE_SGK:
      return "Master Gatekeeper";

    case CommonConstants.DEVTYPE_USERACC:
      return "User Account";

    case CommonConstants.DEVTYPE_ENUM:
      return "ENUM Server";

    case CommonConstants.DEVTYPE_IPPHONE:
      return "Generic IP Device";

    case CommonConstants.DEVTYPE_SIPPROXY:
      return "SIP Proxy";

    case CommonConstants.DEVTYPE_SOFTSWITCH:
      return "Softswitch";

    case CommonConstants.DEVTYPE_SIPGW:
      return "SIP Gateway";
    }

    return "unknown (" + dt + ")";
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
  public String getDeviceTypeString () {
    String retval = getDeviceTypeString(devType);
    if (devType == CommonConstants.DEVTYPE_I1000 && isGateway())
      retval += " (Gateway)";

    return retval;
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

  public boolean isRegistered () {
    return ((stateFlags & CommonConstants.CL_ACTIVE) != 0);
  }

  public boolean wasRegistered () {
    return ((stateFlags & CommonConstants.CL_REGISTERED) != 0);
  }

  public String toString () {
    StringBuffer s = new StringBuffer("Port ");
    s.append((port+1));
    s.append(": ");
    if (!getPhoneNumber().equals(""))
      s.append(getPhoneNumber());
    if (!lastName.equals("")) {
      s.append(" (");
      s.append(lastName);
      s.append(")");
    }

    return s.toString();
  }

  public int compareTo (Object o) {
    Object c = o;
    if (o instanceof DefaultMutableTreeNode) {
      c = ((DefaultMutableTreeNode)o).getUserObject();
    }
    if (c instanceof IEdgeList) {
      return new Integer(port).compareTo(new Integer(((IEdgeList)c).getPort()));
    }

    return toString().compareTo(o.toString());
  }

  public Object clone () {
    try{
      return super.clone();
    }catch(Exception e){
      IEdgeList iEdge = new IEdgeList();
      iEdge.update(this);
      return iEdge;
    }
  }

  public synchronized boolean getAcceptingDrop () {
    return ad;
  }

  public synchronized void setAcceptingDrop (boolean b) {
    ad = b;
  }

  public synchronized RegidPortPair getRegidPortPair () {
    rpp = new RegidPortPair(serialNumber, port);
    return rpp;
  }
  public synchronized boolean isH323Enable () {
    return CommonFunctions.BIT_TEST(caps, CommonConstants.CAP_H323);
  }

  public synchronized void setH323Enable (boolean h323Enable) {
    if(h323Enable)
      caps  = CommonFunctions.BIT_SET(caps,CommonConstants.CAP_H323);
    else
      caps  = CommonFunctions.BIT_RESET(caps,CommonConstants.CAP_H323);

  }

  public synchronized boolean isSipEnable () {
    return CommonFunctions.BIT_TEST(caps, CommonConstants.CAP_SIP);
  }

  public synchronized void setSipEnable (boolean sipEnable) {
    if(sipEnable)
      caps  = CommonFunctions.BIT_SET(caps,CommonConstants.CAP_SIP);
    else
      caps  = CommonFunctions.BIT_RESET(caps,CommonConstants.CAP_SIP);
  }

  public synchronized boolean isGRQEnable () {
    return CommonFunctions.BIT_TEST(caps, CommonConstants.CAP_GRQ);

  }

  public synchronized void setGRQEnable (boolean enable) {
    if(enable)
      caps  = CommonFunctions.BIT_SET(caps,CommonConstants.CAP_GRQ);
    else
      caps  = CommonFunctions.BIT_RESET(caps,CommonConstants.CAP_GRQ);

  }

  public synchronized boolean isRAIEnable () {
    return CommonFunctions.BIT_TEST(caps, CommonConstants.CAP_RAI);
  }

  public synchronized void setRAIEnable (boolean enable) {
    
    if(enable)
      caps  = CommonFunctions.BIT_SET(caps,CommonConstants.CAP_RAI);
    else
      caps  = CommonFunctions.BIT_RESET(caps,CommonConstants.CAP_RAI);

  }

  public synchronized int getPriority () {
    return priority;
  }

  public synchronized void setPriority (int val) {
    priority = val;
  }

  public synchronized int getRasPort () {
    return rasPort;
  }

  public synchronized void setRasPort (int val) {
    rasPort = val;
  }

  public synchronized int getQ931Port () {
    return q931Port;
  }

  public synchronized void setQ931Port (int val) {
    q931Port = val;
  }

  public synchronized String getH323Id () {
    return h323Id;
  }

  public synchronized void setH323Id (String h323Id) {
    this.h323Id = h323Id;
  }

  public synchronized int getCallpartyType() {
    return callpartyType;
  }

  public synchronized void setCallpartyType (int callpartyType) {
    this.callpartyType = callpartyType;
  }

  public synchronized void setCallpartyType (String callpartyType) {
    int i = 0;
    for(i=0; i < callpartyDescription.length; i++){
      if(callpartyType.equals(callpartyDescription[i]))
        break;
    }
    //  invalid call party type, set it to none
    if(i > 6)
      i = 1;
    else if(i  ==  6)
      i++;

    this.callpartyType  = i;
  }


  public synchronized String getCallpartyTypeDescription () {
    return getCallpartyTypeDescription(callpartyType);
  }

  public static String getCallpartyTypeDescription (int callparty) {
    if (callparty > Q931CDPN_Abbreviated)
      return callpartyDescription[Q931CDPN_Unknown];
    if (callparty ==  Q931CDPN_Abbreviated)
      return callpartyDescription[callparty -1];
    else
      return callpartyDescription[callparty];
  }


  public String getCallpartyTypeCommand () {
    if (callpartyType > Q931CDPN_Abbreviated)
      return callpartyCommand[Q931CDPN_Unknown];
    if (callpartyType ==  Q931CDPN_Abbreviated)
      return callpartyCommand[callpartyType -1];
    else
      return callpartyCommand[callpartyType];
  }



  public synchronized int getLayer1Protocol() {
    return layer1Protocol;
  }

  public synchronized void setLayer1Protocol(int layer1Protocol) {
    this.layer1Protocol = layer1Protocol;
  }

  public synchronized void setLayer1Protocol(String protocol) {
    if(layer1ProtocolDescription[1].equals(protocol))
        layer1Protocol  = BCAPLAYER1_G711ulaw;
    else if(layer1ProtocolDescription[2].equals(protocol))
        layer1Protocol  = BCAPLAYER1_G711alaw;
    else if(layer1ProtocolDescription[3].equals(protocol))
        layer1Protocol  = BCAPLAYER1_H221;
    else if(layer1ProtocolDescription[4].equals(protocol))
        layer1Protocol  = BCAPLAYER1_Pass;
    else
        layer1Protocol  = BCAPLAYER1_Default;
  }


  public synchronized String getLayer1ProtocolDescription () {
    return getLayer1ProtocolDescription(layer1Protocol);
  }

  public static String getLayer1ProtocolDescription(int layer1Protocol) {

    switch(layer1Protocol){
      case  BCAPLAYER1_G711ulaw:
            return layer1ProtocolDescription[1];
      case  BCAPLAYER1_G711alaw:
            return layer1ProtocolDescription[2];
      case  BCAPLAYER1_H221:
            return layer1ProtocolDescription[3];
      case  BCAPLAYER1_Pass:
            return layer1ProtocolDescription[4];
      default:
        return layer1ProtocolDescription[0];
    }
      
  }

  public String getLayer1ProtocolCommand () {

    switch(layer1Protocol){
      case  BCAPLAYER1_G711ulaw:
            return layer1ProtocolCommand[1];
      case  BCAPLAYER1_G711alaw:
            return layer1ProtocolCommand[2];
      case  BCAPLAYER1_H221:
            return layer1ProtocolCommand[3];
      case  BCAPLAYER1_Pass:
            return layer1ProtocolCommand[4];
      default:
        return layer1ProtocolCommand[0];
    }
      
  }


  public synchronized String getSipUri () {
    return sipUri;
  }

  public synchronized void setSipUri (String sipUri) {
    this.sipUri = sipUri;
  }


  public synchronized String getSipContact () {
    return sipContact;
  }

  public synchronized void setSipContact (String sipContact) {
    this.sipContact = sipContact;
  }

  public synchronized int getMaxCalls () {
    return maxCalls;
  }

  public synchronized void setMaxCalls (int maxCalls) {
    this.maxCalls = maxCalls;
  }

  public synchronized int getMaxInCalls () {
    return maxInCalls;
  }

  public synchronized void setMaxInCalls (int maxInCalls) {
    this.maxInCalls = maxInCalls;
  }

  public synchronized int getMaxOutCalls () {
    return maxOutCalls;
  }

  public synchronized void setMaxOutCalls (int maxOutCalls) {
    this.maxOutCalls = maxOutCalls;
  }

  public synchronized boolean isTechPrefixEnable () {

    return CommonFunctions.BIT_TEST(caps, CommonConstants.CAP_TPG);
  }

  public synchronized void setTechPrefixEnable (boolean enable) {
    if(enable)
      caps  = CommonFunctions.BIT_SET(caps,CommonConstants.CAP_TPG);
    else
      caps  = CommonFunctions.BIT_RESET(caps,CommonConstants.CAP_TPG);

  }

  public synchronized String getTechPrefix () {
    return techPrefix;
  }

  public synchronized void setTechPrefix (String prefix) {
    techPrefix = prefix;
  }

  public synchronized String getPeerGkId () {
    return peerGkId;
  }

  public synchronized void setPeerGkId (String peerGkId) {
    this.peerGkId = peerGkId;
  }

  public synchronized void setVendor (int ven) {
    vendor = (ven < 0 || ven >= IServerCommands.vendorDescription.length)?0:ven;
  }

  public synchronized int getVendor () {
    return vendor;
  }

  public synchronized String getVendorDescription () {
    return IServerCommands.getVendorDescription(vendor);
  }

  public synchronized void setCurrentCalls (int calls) {
    currentCalls = calls;
  }

  public synchronized int getCurrentCalls () {
    return currentCalls;
  }

  public synchronized int getSubnetIpAsInt () {
    return subnetip;
  }

  public synchronized String getSubnetIp () {
    if (subnetip == 0)
      return "";

    return IPUtil.intToIPString(subnetip);
  }

  public synchronized void setSubnetIp (String addr) {
    if (addr == null || addr.equals(""))
      subnetip = 0;
    subnetip = (int)IPUtil.ipStringToLong(addr);
  }

  public synchronized int getSubnetMaskAsInt () {
    return subnetmask;
  }

  public synchronized String getSubnetMask () {
    if (subnetmask == 0)
      return "";

    return IPUtil.intToIPString(subnetmask);
  }

  public synchronized void setSubnetMask (String addr) {
    if (addr == null || addr.equals(""))
      subnetmask = 0;
    subnetmask = (int)IPUtil.ipStringToLong(addr);
  }

  public synchronized void setH235Password (String str) {
    h235Password = str;
  }

  public synchronized String getH235Password () {
    return h235Password;
  }

  public synchronized boolean isMediaRoutingEnabled () {
    return CommonFunctions.BIT_TEST(caps, CommonConstants.CAP_MEDIAROUTE);
  }

  public synchronized void setMediaRoutingEnabled (boolean enable) {
    if(enable)
      caps  = CommonFunctions.BIT_SET(caps,CommonConstants.CAP_MEDIAROUTE);
    else
      caps  = CommonFunctions.BIT_RESET(caps,CommonConstants.CAP_MEDIAROUTE);
  }

  public synchronized boolean isNeverMediaRouteEnabled () {
    return ((extCaps & CommonConstants.ECAPS_NOMEDIAROUTE) != 0);
  }

  public synchronized void setNeverMediaRouteEnabled (boolean enable) {
    if (enable)
      extCaps |= CommonConstants.ECAPS_NOMEDIAROUTE;
    else
      extCaps &= ~CommonConstants.ECAPS_NOMEDIAROUTE;
  }

  public synchronized boolean isHideAddressChangeEnabled () {

    return CommonFunctions.BIT_TEST(caps, CommonConstants.CAP_HIDEADDRESSCHANGE);
  }

  public synchronized void setHideAddressChangeEnabled (boolean enable) {
    if(enable)
      caps  = CommonFunctions.BIT_SET(caps,CommonConstants.CAP_HIDEADDRESSCHANGE);
    else
      caps  = CommonFunctions.BIT_RESET(caps,CommonConstants.CAP_HIDEADDRESSCHANGE);
  }

  public synchronized int getMaxHunts () {
    return maxHunts;
  }

  public synchronized void setMaxHunts (int hunts) {
    maxHunts = hunts;
  }

  public synchronized boolean isQ931DisplayEnabled () {
    return ((extCaps & CommonConstants.ECAPS_NOH323DISPLAY) == 0);
  }

  public synchronized void setQ931DisplayEnabled (boolean enable) {
    if (enable)
      extCaps &= ~CommonConstants.ECAPS_NOH323DISPLAY;
    else
      extCaps |= CommonConstants.ECAPS_NOH323DISPLAY;
  }

  public synchronized boolean isConnectH245AddressEnabled () {
    return ((extCaps & CommonConstants.ECAPS_NOCONNH245) == 0);
  }

  public synchronized void setConnectH245AddressEnabled (boolean enable) {
    if (enable)
      extCaps &= ~CommonConstants.ECAPS_NOCONNH245;
    else
      extCaps |= CommonConstants.ECAPS_NOCONNH245;
  }

  public synchronized boolean isMapAliasEnabled () {
    return ((extCaps & CommonConstants.ECAPS_MAPALIAS) != 0);
  }

  public synchronized void setMapAliasEnabled (boolean enable) {
    if (enable)
      extCaps |= CommonConstants.ECAPS_MAPALIAS;
    else
      extCaps &= ~CommonConstants.ECAPS_MAPALIAS;
  }

  public synchronized boolean isForceH245Enabled () {
    return ((extCaps & CommonConstants.ECAPS_FORCEH245) != 0);
  }

  public synchronized void setForceH245Enabled (boolean enable) {
    if (enable)
      extCaps |= CommonConstants.ECAPS_FORCEH245;
    else
      extCaps &= ~CommonConstants.ECAPS_FORCEH245;
  }

  public synchronized short getCaps () {
    return caps;
  }

  public synchronized void setCaps (short value) {
    caps = value;
  }

  public synchronized int getExtendedCaps () {
    return extCaps;
  }

  public synchronized void setExtendedCaps (int caps) {
    extCaps = caps;
  }

  public synchronized int getStateFlags () {
    return stateFlags;
  }

  public synchronized void setStateFlags (int sflags) {
    stateFlags = sflags;
  }

  public synchronized int getUsagePercentage () {
    if (isGateway() && maxCalls > 0)
      return (currentCalls*100)/maxCalls;

    return 0;
  }

  public synchronized int getInfoTransCap(){
    return infoTransCap;
  }

  public synchronized void setInfoTransCap(int cap){
    this.infoTransCap = cap;
  }

  public synchronized void setDeltcs2833Cap(int cap) {
    if (cap == 2) // system default
    {
      setDeltcs2833dftEnabled(false);
    }
    else if (cap <= 1)
    {
      setDeltcs2833dftEnabled(true);
      if (cap == 0)
        setDeltcs2833Enabled(true);
      else // cap = 1 
        setDeltcs2833Enabled(false);
    }
  } 

  public synchronized int getDeltcs2833Cap() {
    if (!isDeltcs2833dftEnabled())
       return 2;
    else {
      if (isDeltcs2833Enabled())
       return 0;
      else 
       return 1;
    }
  }

  public synchronized boolean isDeltcs2833Enabled() {
    return ((extCaps & CommonConstants.ECAPS_DELTCSRFC2833) == CommonConstants.ECAPS_DELTCSRFC2833);
  }

  public synchronized void setDeltcs2833Enabled (boolean enable) {
    if (enable)
      extCaps |= CommonConstants.ECAPS_DELTCSRFC2833;
    else
      extCaps &= ~CommonConstants.ECAPS_DELTCSRFC2833;
  }

  public synchronized boolean isDeltcs2833dftEnabled() {
    return ((extCaps & CommonConstants.ECAPS_DELTCSRFC2833DFT) == 
                                 CommonConstants.ECAPS_DELTCSRFC2833DFT);
  }

  public synchronized void setDeltcs2833dftEnabled (boolean enable) {
    if (enable)
      extCaps |= CommonConstants.ECAPS_DELTCSRFC2833DFT;
    else
      extCaps &= ~CommonConstants.ECAPS_DELTCSRFC2833DFT;
  }

  public synchronized void setDeltcst38Cap(int cap) {
    if (cap == 2) // system default
    {
      setDeltcst38dftEnabled(false);
    }
    else if (cap <= 1)
    {
      setDeltcst38dftEnabled(true);
      if (cap == 0)
        setDeltcst38Enabled(true);
      else // cap = 1
        setDeltcst38Enabled(false);
    }
  }

  public synchronized int getDeltcst38Cap() {
    if (!isDeltcst38dftEnabled())
      return 2;
    else {
      if (isDeltcst38Enabled())
        return 0;
      else
        return 1;
    }
  }

  public synchronized boolean isDeltcst38Enabled() {
    return ((extCaps & CommonConstants.ECAPS_DELTCST38) == CommonConstants.ECAPS_DELTCST38);
  }

  public synchronized void setDeltcst38Enabled (boolean enable) {
    if (enable)
      extCaps |= CommonConstants.ECAPS_DELTCST38;
    else
      extCaps &= ~CommonConstants.ECAPS_DELTCST38;
  }

  public synchronized boolean isDeltcst38dftEnabled() {
    return ((extCaps & CommonConstants.ECAPS_DELTCST38DFT) ==
                                 CommonConstants.ECAPS_DELTCST38DFT);
  }

  public synchronized void setDeltcst38dftEnabled (boolean enable) {
    if (enable)
      extCaps |= CommonConstants.ECAPS_DELTCST38DFT;
    else
      extCaps &= ~CommonConstants.ECAPS_DELTCST38DFT;
  }

  public synchronized boolean isCAP2833Supported() {
    return ((extCaps & CommonConstants.ECAPS_CAP2833) ==
                                 CommonConstants.ECAPS_CAP2833);
  }

  public synchronized void setCAP2833Supported (boolean enable) {
    if (enable)
      extCaps |= CommonConstants.ECAPS_CAP2833;
    else
      extCaps &= ~CommonConstants.ECAPS_CAP2833;
  }

  public synchronized boolean isCAP2833Known() {
    return ((extCaps & CommonConstants.ECAPS_CAP2833_KNOWN) ==
                                 CommonConstants.ECAPS_CAP2833_KNOWN);
  }

  public synchronized void setCAP2833Known (boolean enable) {
    if (enable)
      extCaps |= CommonConstants.ECAPS_CAP2833_KNOWN;
    else
      extCaps &= ~CommonConstants.ECAPS_CAP2833_KNOWN;
  }

  public synchronized void setCAP2833Cap(int cap) {
    if (cap == 2) {  // unknown
      setCAP2833Known(false);
    } else if (cap <= 1) {
      setCAP2833Known(true);
      if (cap == 0)
        setCAP2833Supported(true);
      else // cap = 1
        setCAP2833Supported(false);
    }
  }

  public synchronized int getCAP2833Cap() {
    if (!isCAP2833Known()) {
      return 2;
    } else {
      if (isCAP2833Supported())
        return 0;
      else
        return 1;
    }
  }

  public synchronized boolean isRemoveTGEnabled() {
    return ((extCaps & CommonConstants.ECAPS_NOTG) == CommonConstants.ECAPS_NOTG);
  }

  public synchronized void setRemoveTGEnabled(boolean enable) {
    if (enable)
      extCaps |= CommonConstants.ECAPS_NOTG;
    else
      extCaps &= ~CommonConstants.ECAPS_NOTG;
  }

  public synchronized String getGatewayUsageString () {
    StringBuffer sb = new StringBuffer();

    if (isGateway()) {
      if (maxCalls < 0)
	      sb.append("out-of-service");
            else if (maxCalls == 0) {
	      sb.append(currentCalls);
	      sb.append(" calls current");
            } else {
	      sb.append(currentCalls);
	      sb.append("/");
	      sb.append(maxCalls);
	      sb.append(" = ");
	      sb.append((currentCalls*100)/maxCalls);
	      sb.append("%");
      }
    }

    return sb.toString();
  }

  public synchronized String getSrcIngressTg(){
    return srcIngressTg;
  }

  public synchronized void setSrcIngressTg(String srcIngressTg){
    this.srcIngressTg = srcIngressTg;
  }

  public synchronized boolean isDestTg(){
    return ( (extCaps & CommonConstants.ECAPS_SETDESTTG )== CommonConstants.ECAPS_SETDESTTG);
  }

  public synchronized void setDestTg(boolean isDestTg){
    if (isDestTg)
      extCaps |= CommonConstants.ECAPS_SETDESTTG;
    else
      extCaps &= ~CommonConstants.ECAPS_SETDESTTG;
  }

  public synchronized void setRealmName(String realmName){
    this.realmName  = realmName;
  }

  public synchronized String getRealmName(){
    return realmName;
  }

  public synchronized String getIgrpName() {
    return igrpName;
  }

  public synchronized void setIgrpName(String igrpName) {
    this.igrpName = igrpName;
  }

  public synchronized String getDtg() {
    return dtg; 
  }

  public synchronized void setDtg(String dtg) {
    this.dtg = dtg;
  }

  public synchronized String getNewSrcDtg() {
    return newSrcDtg; 
  }

  public synchronized void setNewSrcDtg(String newSrcDtg) {
    this.newSrcDtg = newSrcDtg;
  }

  public synchronized void setISDNMapccEnabled (boolean enable) {
	  if (enable)
	    extCaps |= CommonConstants.ECAPS1_MAPISDNCC;
	  else
	    extCaps &= ~CommonConstants.ECAPS1_MAPISDNCC;
  }
    
  public synchronized boolean isISDNMapccEnabled() {
	  return ((extCaps & CommonConstants.ECAPS1_MAPISDNCC) ==
		CommonConstants.ECAPS1_MAPISDNCC);
  }




  public synchronized void setPiOnFastStartEnabled (boolean enable) {
    if (enable)
      extCaps |= CommonConstants.ECAPS_PIONFASTSTART;
    else
      extCaps &= ~CommonConstants.ECAPS_PIONFASTSTART;
  }

  public synchronized boolean isPiOnFastStartEnabled() {
    return ((extCaps & CommonConstants.ECAPS_PIONFASTSTART) ==
                                 CommonConstants.ECAPS_PIONFASTSTART);
  }


  public synchronized void setAutoNatDetection(boolean enable) {
    if (enable)
      extCaps |= CommonConstants.ECAPS_NATDETECT;
    else
      extCaps &= ~CommonConstants.ECAPS_NATDETECT;
  }

  public synchronized boolean isAutoNatDetectionEnabled() {
    return ((extCaps & CommonConstants.ECAPS_NATDETECT) ==
                                 CommonConstants.ECAPS_NATDETECT);
  }


  public synchronized String getNatIp () {
    if (natIp == 0)
      return "";

    return IPUtil.intToIPString(natIp);
  }
  public synchronized int getNatIpAsInt(){
     return natIp;
  }

  public synchronized void setNatIp (String addr) {
    if (addr.equals(""))
      natIp = 0;
    natIp = (int)IPUtil.ipStringToLong(addr);
  }


  public synchronized void setNatPort(int port){
    this.natPort  = port;
  }

  public synchronized int getNatPort(){
    return natPort;
  }



  public synchronized void setSipPrivacy(String privacy){
    extCaps &= ~CommonConstants.ECAPS1_SIP_PRIVACY_RFC3325;
    extCaps &= ~CommonConstants.ECAPS1_SIP_PRIVACY_DRAFT01;

    if(privacy.equals(sipPrivacy[SIP_PRIVACY_BOTH])){
        extCaps |= CommonConstants.ECAPS1_SIP_PRIVACY_RFC3325;
        extCaps |= CommonConstants.ECAPS1_SIP_PRIVACY_DRAFT01;
    }else if(privacy.equals(sipPrivacy[SIP_PRIVACY_RFC3325]))
      extCaps |= CommonConstants.ECAPS1_SIP_PRIVACY_RFC3325;
    else if(privacy.equals(sipPrivacy[SIP_PRIVACY_DRAFT01]))
      extCaps |= CommonConstants.ECAPS1_SIP_PRIVACY_DRAFT01;
  }

  public synchronized String getSipPrivacy(){
    if(((extCaps & CommonConstants.ECAPS1_SIP_PRIVACY_RFC3325) ==
                                 CommonConstants.ECAPS1_SIP_PRIVACY_RFC3325) &&
        ((extCaps & CommonConstants.ECAPS1_SIP_PRIVACY_DRAFT01) ==
                                 CommonConstants.ECAPS1_SIP_PRIVACY_DRAFT01)
      )
      return sipPrivacy[SIP_PRIVACY_BOTH];
    if((extCaps & CommonConstants.ECAPS1_SIP_PRIVACY_RFC3325) ==
                                 CommonConstants.ECAPS1_SIP_PRIVACY_RFC3325) 
      return sipPrivacy[SIP_PRIVACY_RFC3325];
    if((extCaps & CommonConstants.ECAPS1_SIP_PRIVACY_DRAFT01) ==
                                 CommonConstants.ECAPS1_SIP_PRIVACY_DRAFT01) 
      return sipPrivacy[SIP_PRIVACY_DRAFT01];

    return sipPrivacy[SIP_PRIVACY_BOTH];
  }

  public synchronized void update (IEdgeList il) {
    serialNumber = il.getSerialNumber();
    extNumber = il.getExtNumber();
    phone = il.getPhoneNumber();
    firstName = il.getFirstName();
    lastName = il.getLastName();
    location = il.getLocation();
    country = il.getCountry();
    comments = il.getComments();
    customerId = il.getCustomerId();
    trunkGroup = il.getTrunkGroup();
    zone = il.getZone();
    email = il.getEmail();
    forwardedPhone = il.forwardedPhone;
    forwardedVpnPhone = il.forwardedVpnPhone;
    callingPlanName = il.getCallingPlanName();
    h323Id = il.getH323Id();
    sipUri = il.getSipUri();
    sipContact = il.getSipContact();
    techPrefix = il.getTechPrefix();
    peerGkId = il.getPeerGkId();
    h235Password = il.getH235Password();
    vpnName = il.getVpnName();
    ogp = il.getOutgoingPrefix();
    proxyValid = il.proxyValid;
    isProxied = il.isProxied;
    isProxying = il.isProxying;
    callForwarded = il.callForwarded;
    isCallRollover = il.isCallRollover;
    // no ad copy
    // no devType copy
    port = il.getPort();
    ipaddr = il.getAddressAsInt();
    forwardedVpnExtLen = il.forwardedVpnExtLen;
    maxCalls = il.getMaxCalls();
    maxInCalls = il.getMaxInCalls();
    maxOutCalls = il.getMaxOutCalls();
    priority = il.getPriority();
    rasPort = il.getRasPort();
    q931Port = il.getQ931Port();
    callpartyType = il.getCallpartyType();
    currentCalls = il.getCurrentCalls();
    vendor = il.getVendor();
    extLen  = il.getExtLen();
    subnetip = il.getSubnetIpAsInt();
    subnetmask = il.getSubnetMaskAsInt();
    maxHunts = il.getMaxHunts();
    extCaps = il.getExtendedCaps();
    callType = il.getCallType();
    caps = il.getCaps();
    stateFlags = il.getStateFlags();
    layer1Protocol  = il.getLayer1Protocol();
    inceptionTime = il.getInceptionTime().getTime()/1000;
    refreshTime = il.getRefreshTime().getTime()/1000;
    infoTransCap  = il.getInfoTransCap();
    srcIngressTg  = il.getSrcIngressTg();
    realmName = il.getRealmName();
    igrpName  = il.getIgrpName();
    dtg        = il.getDtg();
    newSrcDtg  = il.getNewSrcDtg();
    natIp = il.getNatIpAsInt();
    natPort = il.getNatPort();

  }

  public void resetAll(){
    vpnName = "";
    serialNumber  = "";
    extNumber = "";            // Stores extension number
    firstName = "";
    lastName  = "";
    location  = "";
    country   = "";
    comments  = "";
    customerId = "";
    trunkGroup = "";
    ogp        = "";
    zone      = "";
    email     = "";
    forwardedPhone      = "";
    forwardedVpnPhone   = "";
    callingPlanName = "";
    h323Id      = "";

    sipUri      = "";
    sipContact  = "";
    techPrefix  = "";
    peerGkId    = "";
    h235Password= "";

    proxyValid    = false;
    isProxied     = false;
    isProxying    = false;
    callForwarded = false;
    isCallRollover= false;
    ad            = false;  
    port          = 0;
    inceptionTime = System.currentTimeMillis()/1000;
    refreshTime   = System.currentTimeMillis()/1000;
    ipaddr        = 0;
    currentCalls  = 0;
    vendor    = 0;
    callpartyType = 1;
    subnetip  = 0;
    subnetmask= 0;
    maxCalls  = 0;
    maxInCalls = 0;
    maxOutCalls = 0;
    priority  = 0;
    forwardedVpnExtLen  = 0;
    devType = -1;
    extLen  = 0;
    phone   = "";
    rasPort = CommonConstants.RASPORT;
    q931Port = CommonConstants.Q931PORT;
    maxHunts = 0;
    extCaps = (CommonConstants.ECAPS_NOH323DISPLAY | CommonConstants.ECAPS_NOCONNH245);
    caps  = 0;
    stateFlags = 0;
    layer1Protocol  = BCAPLAYER1_Default;
    infoTransCap  = -1;
    srcIngressTg  = "";
    realmName = "";
    igrpName  = "";
    dtg       = "";
    newSrcDtg = "";
    natIp = 0;
    natPort = 0;

  }


  public void resetIEdge(){
    extNumber = "";
    phone = "";
    email = ""; 
    forwardedPhone    = "";
    forwardedVpnPhone = "";
    h323Id  = "";
    sipUri  = "";
    sipContact  = "";
    trunkGroup = "";
    ogp = "";
    techPrefix  = "";
    peerGkId    = "";
    h235Password  = "";
    vpnName   = "";

    proxyValid  = false;
    isProxied   = false;
    isProxying  = false;
    callForwarded = false;
    isCallRollover  = false;
    callpartyType = 1;
    port  = -1;
    priority  = 0;
    subnetip  = 0;
    subnetmask  = 0;  
    caps  = 0;
    stateFlags  = 0;  
    inceptionTime = System.currentTimeMillis()/1000;
    refreshTime   = System.currentTimeMillis()/1000;
    layer1Protocol  = BCAPLAYER1_Default;
    infoTransCap  = -1;
    srcIngressTg  = "";
    realmName = "";
    igrpName = "";
    dtg       = "";
    newSrcDtg = "";
    natIp = 0;
    natPort = 0;

  }

/*  public Object getPrimaryKey(){
    return serialNumber;
   }

  public Object getSecondaryKey(){
    return new Integer(port);
  }

  public String getPrimaryField(){
    return "serialNumber";
  }
  public String getSecondaryField(){
    return "port";
  }
  public String getFieldValue(String fieldName){
    if(fieldName.equals("vpnName"))
      return vpnName;
    return "";
  }
*/
  public static Object getObject(Object[] row){
    return new IEdgeList(
      ((Integer)row[INDEX_IEDGE_PORT]).intValue(),
      ((Integer)row[INDEX_IEDGE_IPADDR ]).intValue(),
      ((Integer)row[INDEX_IEDGE_EXTLEN ]).intValue(),
      ((Integer)row[INDEX_IEDGE_FORWARDEDVPNEXTLEN ]).intValue(),
      ((Integer)row[INDEX_IEDGE_DEVTYPE ]).intValue(),
      ((Integer)row[INDEX_IEDGE_MAXCALLS ]).intValue(),
      ((Integer)row[INDEX_IEDGE_MAXINCALLS ]).intValue(),
      ((Integer)row[INDEX_IEDGE_MAXOUTCALLS ]).intValue(),
      ((Integer)row[INDEX_IEDGE_CURRENTCALLS ]).intValue(),
      ((Integer)row[INDEX_IEDGE_PRIORITY]).intValue(),
      ((Integer)row[INDEX_IEDGE_RASPORT]).intValue(),
      ((Integer)row[INDEX_IEDGE_Q931PORT]).intValue(),
      ((Integer)row[INDEX_IEDGE_CALLPARTYTYPE]).intValue(),
      ((Integer)row[INDEX_IEDGE_VENDOR]).intValue(),
      ((Integer)row[INDEX_IEDGE_SUBNETIP]).intValue(),
      ((Integer)row[INDEX_IEDGE_SUBNETMASK]).intValue(),
      ((Integer)row[INDEX_IEDGE_MAXHUNTS]).intValue(),
      ((Integer)row[INDEX_IEDGE_EXTCAPS]).intValue(),
      ((Integer)row[INDEX_IEDGE_LAYER1PROTOCOL]).intValue(),
      ((Long)row[INDEX_IEDGE_INCEPTIONTIME]).longValue(),
      ((Long)row[INDEX_IEDGE_REFRESHTIME]).longValue(),
      (String)row[INDEX_IEDGE_SERIALNUMBER],
      (String)row[INDEX_IEDGE_VPNNAME],
      (String)row[INDEX_IEDGE_EXTNUMBER],
      (String)row[INDEX_IEDGE_PHONE],
      (String)row[INDEX_IEDGE_FIRSTNAME],
      (String)row[INDEX_IEDGE_LASTNAME],
      (String)row[INDEX_IEDGE_LOCATION],
      (String)row[INDEX_IEDGE_COUNTRY],
      (String)row[INDEX_IEDGE_COMMENTS],
      (String)row[INDEX_IEDGE_CUSTOMERID],
      (String)row[INDEX_IEDGE_CALLINGPLANNAME],
      (String)row[INDEX_IEDGE_ZONE],
      (String)row[INDEX_IEDGE_EMAIL],
      (String)row[INDEX_IEDGE_FORWARDEDPHONE],
      (String)row[INDEX_IEDGE_H323ID],
      (String)row[INDEX_IEDGE_SIPURI],
      (String)row[INDEX_IEDGE_SIPCONTACT],
      (String)row[INDEX_IEDGE_FORWARDEDVPNPHONE],
      (String)row[INDEX_IEDGE_TECHPREFIX],
      (String)row[INDEX_IEDGE_PEERGKID],
      (String)row[INDEX_IEDGE_H235PASSWORD],
      // boolean type is not supported in Mysql database
      // Instead it is defined as TINYINT
      (((Short)row[INDEX_IEDGE_PROXYVALID]).shortValue() == 1)?true:false,
      (((Short)row[INDEX_IEDGE_ISPROXIED]).shortValue() == 1)?true:false,
      (((Short)row[INDEX_IEDGE_ISPROXYING]).shortValue() == 1)?true:false,
      (((Short)row[INDEX_IEDGE_CALLFORWARDED]).shortValue() == 1)?true:false,
      (((Short)row[INDEX_IEDGE_ISCALLROLLOVER]).shortValue() == 1)?true:false,
      ((Integer)row[INDEX_IEDGE_CAPS]).shortValue(),
      ((Integer)row[INDEX_IEDGE_STATEFLAGS]).intValue(),
      (String)row[INDEX_IEDGE_TRUNKGROUP],
      (String)row[INDEX_IEDGE_OGP],
      ((Integer)row[INDEX_IEDGE_INFOTRANSCAP]).intValue(),
      (String)row[INDEX_IEDGE_SRCINGRESSTG],
      (String)row[INDEX_IEDGE_REALMNAME],
      (String)row[INDEX_IEDGE_IGRPNAME],
      (String)row[INDEX_IEDGE_DTG],
      (String)row[INDEX_IEDGE_NEWSRCDTG],
      ((Integer)row[INDEX_IEDGE_NATIP ]).intValue(),
      ((Integer)row[INDEX_IEDGE_NATPORT]).intValue()
      );
  }

  //  implements DBObjet interface
  // order should be same as in the IViewDB calling plan binding table fields
  public Object[] getObjectArray(){
    Object[]  data= new Object[59];
    data[0] = serialNumber;
    data[1] = extNumber;
    data[2] = phone;
    data[3] = firstName;
    data[4] = lastName;
    data[5] = location;
    data[6] = country;
    data[7] = comments;
    data[8] = customerId;
    data[9] = trunkGroup;
    data[10] = zone;
    data[11] = email;
    data[12] = forwardedPhone;
    data[13] = forwardedVpnPhone;
    data[14] = callingPlanName;
    data[15] = h323Id;
    data[16] = sipUri;
    data[17] = sipContact;
    data[18] = techPrefix;
    data[19] = peerGkId;
    data[20] = h235Password;
    data[21] = vpnName;
    data[22] = ogp;
  // boolean type is not supported in Mysql database
  // Instead it is defined as TINYINT
    data[23] = getShort(proxyValid);
    data[24] = getShort(isProxied);
    data[25] = getShort(isProxying);
    data[26] = getShort(callForwarded);
    data[27] = getShort(isCallRollover);
    data[28] = new Integer(devType);
    data[29] = new Integer(port);
    data[30] = new Integer(ipaddr);
    data[31] = new Integer(forwardedVpnExtLen);
    data[32] = new Integer(maxCalls);
    data[33] = new Integer(maxInCalls);
    data[34] = new Integer(maxOutCalls);
    data[35] = new Integer(priority);
    data[36] = new Integer(rasPort );
    data[37] = new Integer(q931Port );
    data[38] = new Integer(callpartyType);
    data[39] = new Integer(currentCalls);
    data[40] = new Integer(vendor);
    data[41] = new Integer(extLen);
    data[42] = new Integer(subnetip);
    data[43] = new Integer(subnetmask);
    data[44] = new Integer(maxHunts);
    data[45] = new Integer(extCaps);
    data[46] = new Integer((int)caps);
    data[47] = new Integer(stateFlags);
    data[48] = new Integer(layer1Protocol);
    data[49] = new Long(inceptionTime);
    data[50] = new Long(refreshTime);
    data[51] = new Integer(infoTransCap);
    data[52] = srcIngressTg;
    data[53] = realmName;
    data[54] = igrpName;
    data[55] = dtg;
    data[56] = newSrcDtg;
    data[57] = new Integer(natIp);;
    data[58] = new Integer(natPort);

    return data;
  }

  public Short getShort(boolean flag){
      if (flag)
          return new Short((short)1);
      else
          return new Short((short)0);

  }

  public String[] getKeys(){
    String[] ob = new String[2];
    ob[0] = "serialNumber";
    ob[1] = "port";
    return ob;
  }

  public Object[] getValues(){
    Object[] ob = new Object[2];
    ob[0] = serialNumber;
    ob[1] = new Integer(port);
    return ob;
  }

	public boolean equals (Object o) {
	 if (o instanceof IEdgeList &&
        getSerialNumber().equals(((IEdgeList)o).getSerialNumber())&&
        getPort() ==  ((IEdgeList)o).getPort()
      )
      return true;
   return false;
  }
}

