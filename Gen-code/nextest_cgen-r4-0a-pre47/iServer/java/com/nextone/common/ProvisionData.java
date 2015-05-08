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
import java.util.*;
import com.nextone.util.IPUtil;


public class ProvisionData implements CommonConstants, Serializable {
  private Set indexes = new HashSet(); // Holds index of the parameter names
  private VpnList vpnList;
  private int port = 0;                // Key2
  private String serialNumber;	       // Key1
//  private String vpnName;              // Stores vpn name
//  private String vpnId;
//  private String vpnGroup;             // Stores group name
  private String extNumber;            // Stores extension number
  private String firstName;            // Stores first name
  private String lastName;             // Stores last name
  private String location;             // Stores location of the endpoint
  private String country;
  private String comments;
  private String customerId;
  private String trunkGroup;
  private String zone;
  private String phone;
  private String email;                // Stores email
  private String forwardedPhone;       // Stores forward/rollover number
  private String contact;  
//  private String vpnPrefix;
  private boolean devTypeExists;
  private boolean callForwarded;		
  private boolean isCallRollover;
  private int ipAddress;               // Stores ipaddress
//  private int extLen = -1;             // Stores extension length
  private int devType = -1;            // Stores device type
  private int callType;
  private boolean isGateway;
  private boolean h323Enable;
  private String h323Id;
  private int callpartyType;
  private boolean sipEnable;
  private String sipUri;
  private String sipContact;
  private int maxCalls;
  private boolean grqEnable;
  private boolean raiEnable;
  private int priority = -1;
  private int rasPort = CommonConstants.RASPORT;
  private int q931Port = CommonConstants.Q931PORT;
  private boolean techPrefixEnable;
  private String techPrefix;
  private String peerGkId;
  private int vendor;
  private int subnetmask;
  private int subnetip;
  private String cpname;  // calling plan of this endpoint
  private String h235Password;
  private boolean mediaRouting;
  private boolean hideAddressChange;
  private int maxHunts;
  private boolean q931Display;
  private int layer1Protocol;

  static final long serialVersionUID = 3336904765201986462L;

  public ProvisionData () {
    vpnList = new VpnList("");
//    vpnGroup = "";
//    vpnId = "";
    extNumber = "";
    firstName = "";
    lastName = "";
    location = "";
    country = "";
    comments = "";
    customerId = "";
    trunkGroup = "";
    zone = "";
    phone = "";
    email = "";
    forwardedPhone = "";
//    vpnName = "";
    contact = "";  
//    vpnPrefix = "";
    phone = "";
    serialNumber = "";
    ipAddress = 0;		
//    extLen = -1;
    devType = -1;
    callType = 0;
    port = 0;
    callForwarded = false;
    isCallRollover = false;
    isGateway = false;
    h323Enable = false;
    h323Id = "";
    callpartyType = 0;
    sipEnable = false;
    sipUri = "";
    sipContact = "";
    maxCalls = -1;
    grqEnable = false;
    raiEnable = false;
    priority = -1;
    rasPort = CommonConstants.RASPORT;
    q931Port = CommonConstants.Q931PORT;
    techPrefixEnable = false;
    techPrefix = "";
    peerGkId = "";
    vendor = 0;
    subnetip = 0;
    subnetmask = 0;
    cpname = "";
    h235Password = "";
    mediaRouting = false;
    hideAddressChange = false;
    maxHunts = 0;
    q931Display = false;
    layer1Protocol  = 0;
  }

  // used in IEdgeView.java
  public ProvisionData (String serial,
			int port,
      String vpnName,
//			String vpnname,
//			String vpnid,
//			String vpngrp,
			String extNumber,
			String firstname,
			String lastname,
			String location,
			String cntry,
			String cmnts,
      String custid,
			String zone,
			String email,
			String vpncontact,
//			String vpnprefix,
//			int extlen,
			int devtype,
			String ipaddr,
			int calltype,
			boolean isCallForwarded,
			String fwd,
			String ph,
			boolean isgw,
			boolean h323Enable,
			String h323Id,
      int callpartyType,
			boolean sipEnable,
			String sipUri,
			String sipContact,
			int maxCalls,
			boolean grqEnable,
			boolean raiEnable,
			int priority,
			int rasPort,
			int q931Port,
			boolean techPrefixEnable,
			String techPrefix,
			String peerGkId,
			int vendor,
			int snip,
			int snmask,
			String cpname,
			String password,
			boolean mr,
			boolean hac,
			int maxHunts,
			boolean q931d,
                        int layer1Protocol,
                        String tg) {
    vpnList = new VpnList();
    setSerialNumber(serial);
    setPort(port);
    setVpnName(vpnName);
    setExtNumber(extNumber);
    setFirstName(firstname);
    setLastName(lastname);
    setLocation(location);
    setCountry(cntry);
    setComments(cmnts);
    setCustomerId(custid);
    setTrunkGroup(tg);
    setZone(zone);
    setEmail(email);
    setContact(vpncontact);
    setDevType(devtype);
    setAddress(ipaddr);
    setCallType(calltype);
    setCallForwarded(isCallForwarded);
    setFwdPhone(fwd);
    setPhone(ph);
    setGateway(isgw);
    setH323Enable(h323Enable);
    setH323Id(h323Id);
    setCallpartyType(callpartyType);
    setSipEnable(sipEnable);
    setSipUri(sipUri);
    setSipContact(sipContact);
    setMaxCalls(maxCalls);
    setGRQEnable(grqEnable);
    setRAIEnable(raiEnable);
    setPriority(priority);
    setRasPort(rasPort);
    setQ931Port(q931Port);
    setTechPrefixEnable(techPrefixEnable);
    setTechPrefix(techPrefix);
    setPeerGkId(peerGkId);
    setVendor(vendor);
    setSubnetIp(snip);
    setSubnetMask(snmask);
    setCallingPlan(cpname);
    setH235Password(password);
    setMediaRoutingEnabled(mr);
    setHideAddressChangeEnabled(hac);
    setMaxHunts(maxHunts);
    setQ931DisplayEnabled(q931d);
    setLayer1Protocol(layer1Protocol);
  }

  // used in BridgeServerImpl.c
  public ProvisionData (String vpnname,
			String vpnid,
			String vpngrp,
			String vpnext,
			String firstname,
			String lastname,
			String location,
			String cntry,
			String cmnts,
                        String custid,
			String zone,
			String email,
			int extlen,
			int devtype,
			String vpnlocation,
			String vpncontact,
			String fwd,
			String phone,
			boolean callforward,
			boolean isrollover,
			boolean h323Enable,
			String h323Id,
      int callpartyType,
			boolean sipEnable,
			String sipUri,
			String sipContact,
			int maxCalls,
			boolean isgw,
			boolean grqEnable,
			boolean raiEnable,
			int priority,
			int rasPort,
			int q931Port,
			boolean techPrefixEnable,
			String techPrefix,
			String peerGkId,
			int vendor,
			int snip,
			int snmask,
			String cpname,
			String password,
			boolean mr,
			boolean hac,
			int maxHunts,
                        int layer1Protocol,
                        String tg) {
    setVpnName(vpnname);
    setVpnId(vpnid);
    setVpnGroup(vpngrp);
    setExtNumber(vpnext);
    setFirstName(firstname);
    setLastName(lastname);
    setLocation(location);
    setCountry(cntry);
    setComments(cmnts);
    setCustomerId(custid);
    setTrunkGroup(tg);
    setZone(zone);
    setEmail(email);
    setExtensionLen(extlen);
    setDevType(devtype);
    setContact(vpncontact);
    setFwdPhone(fwd);
    setPhone(phone);
    setVpnPrefix("");
    setGateway(isgw);

    if (callforward) {
      callForwarded = true;
      if (isrollover) {
	callType = CommonConstants.CALLROLLOVER;
	isCallRollover = true;
      } else 
	callType = CommonConstants.CALLFORWARD;
    } else {
      isCallRollover = false;
      callforward = false;
      forwardedPhone = "";
    }
    ipAddress = 0;		

    setH323Enable(h323Enable);
    setH323Id(h323Id);
    setCallpartyType(callpartyType);
    setSipEnable(sipEnable);
    setSipUri(sipUri);
    setSipContact(sipContact);
    setMaxCalls(maxCalls);
    setGRQEnable(grqEnable);
    setRAIEnable(raiEnable);
    setPriority(priority);
    setRasPort(rasPort);
    setQ931Port(q931Port);
    setTechPrefixEnable(techPrefixEnable);
    setTechPrefix(techPrefix);
    setPeerGkId(peerGkId);
    setVendor(vendor);
    setSubnetIp(snip);
    setSubnetMask(snmask);
    setCallingPlan(cpname);
    setH235Password(password);
    setMediaRoutingEnabled(mr);
    setHideAddressChangeEnabled(hac);
    setMaxHunts(maxHunts);
    //    setQ931DisplayEnabled(q931d);
    setLayer1Protocol(layer1Protocol);
  }

  public void dump () {
    System.out.println("VPN Group: " + vpnList.getVpnGroup() );
    System.out.println("Extension Number: " + extNumber );
    System.out.println("First Name: " + firstName );
    System.out.println("Last Name: " + lastName );
    System.out.println("Email: " + email );
    System.out.println("Location: " + location );
    System.out.println("Country: " + country );
    System.out.println("Comments: " + comments );
    System.out.println("Customer Id: " + customerId);
    System.out.println("Trunk Group: " + trunkGroup);
    System.out.println("Zone: " + zone );
    System.out.println("Phone: " + phone );
    System.out.println("ExtensionLen: " + vpnList.getExtLen() );
    System.out.println("Device Type: " + devType );
    System.out.println("VPN Name: " + vpnList.getVpnName());
    System.out.println("VPN Id: " + vpnList.getVpnId());
    System.out.println("VPN Contact: " + contact);
    System.out.println("H323 Id :" +  h323Id);
    System.out.println("Sip Uri: " + sipUri);
    System.out.println("Sip Contact: " + sipContact);
    System.out.println("Max Calls: " + maxCalls);
    System.out.println("GRQ: " + grqEnable);
    System.out.println("RAI: " + raiEnable);
    System.out.println("isGW: " + isGateway);
    System.out.println("priority: " + priority);
    System.out.println("RAS Port: " + rasPort);
    System.out.println("Q.931 Port: " + q931Port);
    System.out.println("Tech Prefix Enabled: " + techPrefixEnable);
    System.out.println("Tech Prefix: " + techPrefix);
    System.out.println("Peer GK Id: " + peerGkId);
    System.out.println("Vendor: " + vendor);
    System.out.println("Subnet: " + IPUtil.intToIPString(subnetip) + "/" + IPUtil.intToIPString(subnetmask));
    System.out.println("Calling plan: " + cpname);
    System.out.println("H.235 Password: " + h235Password);
    System.out.println("Media Routing: " + mediaRouting);
    System.out.println("Hide Address Change: " + hideAddressChange);
    System.out.println("Max Hunts: " + maxHunts);
    System.out.println("Q.931 Display: " + q931Display);
  }

	
  public synchronized void setVpnDetails (VpnList v) {
    if (v != null) {
      vpnList = v;
//      setVpnName(v.getVpnName());
//      setVpnGroup(v.getVpnGroup());
//      setVpnPrefix(v.getVpnPrefix());
//      setVpnId(v.getVpnId());
    }
  }

  public synchronized Object getValue (int cmd){
    //deprecated since 2.0.5. Use the methode directly to get the value

/*
    switch (cmd) {

    case CommonConstants.DEV_TYPE:
      return new Integer(getDevType());

    case CommonConstants.SERIAL_NUMBER:
      return getSerialNumber();

    case CommonConstants.PORT_NUMBER:
      return String.valueOf(getPort()+1);

    case CommonConstants.EXTENSION_NUMBER:
      return getExtNumber();

    case CommonConstants.IP_ADDRESS:
      return getAddress();

    case CommonConstants.VPN_NAME:
      return getVpnName();

    case CommonConstants.VPN_ID:
      return getVpnId();

    case CommonConstants.VPN_PREFIX:
      return getVpnPrefix();

    case CommonConstants.VPN_GROUP:
      return getVpnGroup();

    case CommonConstants.CONTACT:
      return getContact();

    case CommonConstants.FWD_NUMBER:
      return getFwdPhone();

    case CommonConstants.CALLTYPE:
      return new Integer(getCallType());

    case CommonConstants.ISCALLFORWARD:
      return new Boolean(isCallForwarded());

    case CommonConstants.FIRST_NAME:
      return getFirstName();

    case CommonConstants.LAST_NAME:
      return getLastName();

    case CommonConstants.LOCATION:
      return getLocation();

    case CommonConstants.COUNTRY:
      return getCountry();

    case CommonConstants.COMMENTS:
      return getComments();

    case CommonConstants.GW_ZONE:
      return getZone();

    case CommonConstants.EMAIL:
      return getEmail();

    case CommonConstants.VPN_EXTENSION_LEN:
      return new Integer(getExtensionLen());

    case CommonConstants.PHONE_NUMBER:
      return getPhone();

    case CommonConstants.ISGATEWAY:
      return new Boolean(isGateway);

    case CommonConstants.H323_ENABLE:
      return new Boolean(isH323Enable());

    case CommonConstants.H323_ID:
      return getH323Id();

    case CommonConstants.SIP_ENABLE:
      return new Boolean(isSipEnable());

    case CommonConstants.SIP_URI:
      return getSipUri();

    case CommonConstants.SIP_CONTACT:
      return getSipContact();

    case CommonConstants.MAX_CALLS:
      return new Integer(getMaxCalls());

    case CommonConstants.GRQ_ENABLE:
      return new Boolean(isGRQEnable());

    case CommonConstants.RAI_ENABLE:
      return new Boolean(isRAIEnable());

    case CommonConstants.PRIORITY:
      return new Integer(getPriority());

    case CommonConstants.RAS_PORT:
      return new Integer(getRasPort());

    case CommonConstants.Q931_PORT:
      return new Integer(getQ931Port());

    case CommonConstants.TECHPREFIX_ENABLE:
      return new Boolean(isTechPrefixEnable());

    case CommonConstants.TECHPREFIX:
      return getTechPrefix();

    case CommonConstants.PEERGK_ID:
      return getPeerGkId();

    case CommonConstants.VENDOR:
      return new Integer(getVendor());

    case CommonConstants.SUBNETIP:
      return getSubnetIp();

    case CommonConstants.SUBNETMASK:
      return getSubnetMask();

    case CommonConstants.CALLING_PLAN:
      return getCallingPlan();

    case CommonConstants.H235_PASSWORD:
      return getH235Password();

    case CommonConstants.MEDIA_ROUTING:
      return new Boolean(isMediaRoutingEnabled());

    case CommonConstants.HIDE_ADDRESS_CHANGE:
      return new Boolean(isHideAddressChangeEnabled());

    case CommonConstants.MAX_HUNTS:
      return new Integer(getMaxHunts());

    case CommonConstants.Q931_DISPLAY:
      return new Boolean(isQ931DisplayEnabled());
    }
*/
    return "";
  }


  public synchronized void setValue (int cmd, Object newData) {

    //deprecated since 2.0.5. Use the methode directly to set the value
/*
    switch (cmd) {
    case CommonConstants.SERIAL_NUMBER:
      setSerialNumber((String)newData);
      break;

    case CommonConstants.PORT_NUMBER:
      setPort( new Integer((String)newData).intValue()-1);
      break;

    case CommonConstants.VPN_NAME:
      setVpnName((String)newData);
      break;

    case CommonConstants.VPN_ID:
      setVpnId((String)newData);
      break;

    case CommonConstants.VPN_GROUP:
      setVpnGroup((String)newData);
      break;

    case CommonConstants.VPN_NUMBER:
      setExtNumber((String)newData);
      break;

    case CommonConstants.FIRST_NAME:
      setFirstName((String)newData);
      break;

    case CommonConstants.LAST_NAME:
      setLastName((String)newData);
      break;

    case CommonConstants.LOCATION:
      setLocation((String)newData);
      break;

    case CommonConstants.COUNTRY:
      setCountry((String)newData);
      break;

    case CommonConstants.COMMENTS:
      setComments((String)newData);
      break;

    case CommonConstants.GW_ZONE:
      setZone((String)newData);
      break;

    case CommonConstants.VPN_EXTENSION_LEN:
      setExtensionLen(new Integer((String)newData).intValue());
      break;

    case CommonConstants.EMAIL:
      setEmail((String)newData);
      break;

    case CommonConstants.DEV_TYPE:
      setDevType(((Integer)newData).intValue());
      break;

    case CommonConstants.CONTACT:
      setContact((String)newData);
      break;

    case CommonConstants.EXTENSION_NUMBER:
      setExtNumber((String)newData);
      break;

    case CommonConstants.FWD_NUMBER:
      setFwdPhone((String)newData);
      break;

    case CommonConstants.CALLTYPE:
      setCallType(((Integer)newData).intValue());
      break;

    case CommonConstants.ISCALLFORWARD:
      setCallForwarded(((Boolean)newData).booleanValue());
      break;

    case CommonConstants.IP_ADDRESS:
      setAddress((String)newData);
      break;

    case CommonConstants.VPN_PREFIX:
      setVpnPrefix((String)newData);
      break;

    case CommonConstants.PHONE_NUMBER:
      setPhone((String)newData);
      break;

    case CommonConstants.ISGATEWAY:
      setGateway(((Boolean)newData).booleanValue());
      break;

    case CommonConstants.H323_ENABLE:
      setH323Enable(((Boolean)newData).booleanValue());
      break;

    case CommonConstants.H323_ID:
      setH323Id((String)newData);
      break;

    case CommonConstants.SIP_ENABLE:
      setSipEnable(((Boolean)newData).booleanValue());
      break;

    case CommonConstants.SIP_URI:
      setSipUri((String)newData);
      break;

    case CommonConstants.SIP_CONTACT:
      setSipContact((String)newData);
      break;

    case CommonConstants.MAX_CALLS:
      setMaxCalls(((Integer)newData).intValue());
      break;

    case CommonConstants.GRQ_ENABLE:
      setGRQEnable(((Boolean)newData).booleanValue());
      break;

    case CommonConstants.RAI_ENABLE:
      setRAIEnable(((Boolean)newData).booleanValue());
      break;

    case CommonConstants.PRIORITY:
      setPriority(((Integer)newData).intValue());
      break;

    case CommonConstants.RAS_PORT:
      setRasPort(((Integer)newData).intValue());
      break;

    case CommonConstants.Q931_PORT:
      setQ931Port(((Integer)newData).intValue());
      break;

    case CommonConstants.TECHPREFIX_ENABLE:
      setTechPrefixEnable(((Boolean)newData).booleanValue());
      break;

    case CommonConstants.TECHPREFIX:
      setTechPrefix((String)newData);
      break;

    case CommonConstants.PEERGK_ID:
      setPeerGkId((String)newData);
      break;

    case CommonConstants.VENDOR:
      setVendor(((Integer)newData).intValue());
      break;

    case CommonConstants.SUBNETIP:
      setSubnetIp((String)newData);
      break;

    case CommonConstants.SUBNETMASK:
      setSubnetMask((String)newData);
      break;

    case CommonConstants.CALLING_PLAN:
      setCallingPlan((String)newData);
      break;

    case CommonConstants.H235_PASSWORD:
      setH235Password((String)newData);
      break;

    case CommonConstants.MEDIA_ROUTING:
      setMediaRoutingEnabled(((Boolean)newData).booleanValue());
      break;

    case CommonConstants.HIDE_ADDRESS_CHANGE:
      setHideAddressChangeEnabled(((Boolean)newData).booleanValue());
      break;

    case CommonConstants.MAX_HUNTS:
      setMaxHunts(((Integer)newData).intValue());
      break;

    case CommonConstants.Q931_DISPLAY:
      setQ931DisplayEnabled(((Boolean)newData).booleanValue());
      break;
    }
    */
  }


  public synchronized void copyData (ProvisionData pd) {
    setSerialNumber(pd.getSerialNumber());
    setPort(pd.getPort());
    setVpnList(pd.getVpnList());
//    setVpnName(pd.getVpnName());
//    setVpnId(pd.getVpnId());
//    setVpnGroup(pd.getVpnGroup());
    setExtNumber(pd.getExtNumber());
    setFirstName(pd.getFirstName());
    setLastName(pd.getLastName());
    setLocation(pd.getLocation());
    setCountry(pd.getCountry());
    setComments(pd.getComments());
    setCustomerId(pd.getCustomerId());
    setTrunkGroup(pd.getTrunkGroup());
    setZone(pd.getZone());
    setEmail(pd.getEmail());
    setContact(pd.getContact());
//    setVpnPrefix(pd.getVpnPrefix());
    setAddress(pd.getAddress());
    setCallForwarded(pd.isCallForwarded());
    setCallType(pd.getCallType());
//    setExtensionLen(pd.getExtensionLen());
    setDevType(pd.getDevType());
    setFwdPhone(pd.getFwdPhone());
    setPhone(pd.getPhone());
    setGateway(pd.isGateway());
    setH323Enable(pd.isH323Enable());
    setH323Id(pd.getH323Id());
    setCallpartyType(pd.getCallpartyType());
    setSipEnable(pd.isSipEnable());
    setSipUri(pd.getSipUri());
    setSipContact(pd.getSipContact());
    setMaxCalls(pd.getMaxCalls());
    setGRQEnable(pd.isGRQEnable());
    setRAIEnable(pd.isRAIEnable());
    setPriority(pd.getPriority());
    setRasPort(pd.getRasPort());
    setQ931Port(pd.getQ931Port());
    setTechPrefixEnable(pd.isTechPrefixEnable());
    setTechPrefix(pd.getTechPrefix());
    setPeerGkId(pd.getPeerGkId());
    setVendor(pd.getVendor());
    setSubnetIp(pd.getSubnetIp());
    setSubnetMask(pd.getSubnetMask());
    setCallingPlan(pd.getCallingPlan());
    setH235Password(pd.getH235Password());
    setMediaRoutingEnabled(pd.isMediaRoutingEnabled());
    setHideAddressChangeEnabled(pd.isHideAddressChangeEnabled());
    setMaxHunts(pd.getMaxHunts());
    setQ931DisplayEnabled(pd.isQ931DisplayEnabled());
    setLayer1Protocol(pd.getLayer1Protocol());
  }


  public synchronized boolean updateData (ProvisionData pd) {
    boolean flag = false;
    Set oldIndexes = new HashSet(indexes);

    clearIndexes();

    if (!vpnList.getVpnName().equals(pd.getVpnName())) {
      updateVpnName(pd.getVpnName());
      flag = true;
    }
    if (!vpnList.getVpnGroup().equals(pd.getVpnGroup())) {
      updateVpnGroup(pd.getVpnGroup());
      flag = true;
    }
    if (!extNumber.equals(pd.getExtNumber())) {
      updateExtNumber(pd.getExtNumber());
      flag = true;
    }
    if (!firstName.equals(pd.getFirstName())) {
      updateFirstName(pd.getFirstName());
      flag = true;
    }
    if (!lastName.equals(pd.getLastName()) ) {
      updateLastName(pd.getLastName());
      flag = true;
    }
    if (!location.equals(pd.getLocation())) {
      updateLocation(pd.getLocation());
      flag = true;
    }
    if (!country.equals(pd.getCountry())) {
      updateCountry(pd.getCountry());
      flag = true;
    }
    if (!comments.equals(pd.getComments())) {
      updateComments(pd.getComments());
      flag = true;
    }
    if (!customerId.equals(pd.getCustomerId())) {
      updateCustomerId(pd.getCustomerId());
      flag = true;
    }
    if (!trunkGroup.equals(pd.getTrunkGroup())) {
      updateTrunkGroup(pd.getTrunkGroup());
      flag = true;
    }
    if (!zone.equals(pd.getZone())) {
      updateZone(pd.getZone());
      flag = true;
    }
    if (!email.equals(pd.getEmail())) {
      updateEmail(pd.getEmail());
      flag = true;
    }
    if (devType != pd.getDevType() ) {
      updateDevType(pd.getDevType());
      flag = true;
    }
    if(!(getAddress().equals(pd.getAddress()))) {
      updateAddress(pd.getAddress());
      flag = true;
    }
    if (pd.isCallForwarded() &&
	(!forwardedPhone.equals(pd.getFwdPhone()) ||
	 pd.getCallType() != getCallType())) {
      setCallForwarded(true);
      updateCallType(pd.getCallType());
      updateFwdPhone(pd.getFwdPhone());
      flag = true;
    } else if (!pd.isCallForwarded() && isCallForwarded()) {
      updateCallType(pd.getCallType());
      setCallForwarded(false);
      setFwdPhone("");
      flag = true;
    }
    if (isH323Enable() != pd.isH323Enable()) {
      updateH323Enable(pd.isH323Enable());
      flag = true;
    }
    if (isSipEnable() != pd.isSipEnable()) {
      updateSipEnable(pd.isSipEnable());
      flag = true;
    }
    if (isGRQEnable() != pd.isGRQEnable()) {
      updateGRQEnable(pd.isGRQEnable());
      flag = true;
    }
    if (isRAIEnable() != pd.isRAIEnable()) {
      updateRAIEnable(pd.isRAIEnable());
      flag = true;
    }
    if (!(getH323Id().equals(pd.getH323Id()))) {
      updateH323Id(pd.getH323Id());
      flag = true;
    }

    if (getCallpartyType()  !=  pd.getCallpartyType()) {
      updateCallpartyType(pd.getCallpartyType());
      flag = true;
    }

    if (!(getSipUri().equals(pd.getSipUri()))) {
      updateSipUri(pd.getSipUri());
      flag = true;
    }
    if (!getSipContact().equals(pd.getSipContact())) {
      updateSipContact(pd.getSipContact());
      flag = true;
    }
    if (getMaxCalls() != pd.getMaxCalls()) {
      updateMaxCalls(pd.getMaxCalls());
      flag = true;
    }
    if(!extNumber.equals(pd.getExtNumber())) {
      setExtNumber(pd.getExtNumber());
      flag = true;
    }
    if (vpnList.getExtLen() != pd.getExtensionLen()) {
      setExtensionLen(pd.getExtensionLen());
      flag = true;
    }
    if (!contact.equals(pd.getContact())) {
      setContact(pd.getContact());
      flag = true;
    }
    if (!phone.equals(pd.getPhone())) {
      setPhone(pd.getPhone());
    }
    if (!vpnList.getVpnId().equals(pd.getVpnId())) {
      setVpnId(pd.getVpnId());
    }
    if (!serialNumber.equals(pd.getSerialNumber()))
      setSerialNumber(pd.getSerialNumber());

    if (isGateway != pd.isGateway()) {
      updateGateway(pd.isGateway());
      flag = true;
    }

    if (getPriority() != pd.getPriority()) {
      updatePriority(pd.getPriority());
      flag = true;
    }
    if (getRasPort() != pd.getRasPort()) {
      updateRasPort(pd.getRasPort());
      flag = true;
    }
    if (getQ931Port() != pd.getQ931Port()) {
      updateQ931Port(pd.getQ931Port());
      flag = true;
    }
    if (isTechPrefixEnable() != pd.isTechPrefixEnable()) {
      updateTechPrefixEnable(pd.isTechPrefixEnable());
      flag = true;
    }
    if (!(getTechPrefix().equals(pd.getTechPrefix()))) {
      updateTechPrefix(pd.getTechPrefix());
      flag = true;
    }
    if (!(getPeerGkId().equals(pd.getPeerGkId()))) {
      updatePeerGkId(pd.getPeerGkId());
      flag = true;
    }
    if (getVendor() != pd.getVendor()) {
      updateVendor(pd.getVendor());
      flag = true;
    }
    if(!(getSubnetIp().equals(pd.getSubnetIp()))) {
      updateSubnetIp(pd.getSubnetIp());
      flag = true;
    }
    if(!(getSubnetMask().equals(pd.getSubnetMask()))) {
      updateSubnetMask(pd.getSubnetMask());
      flag = true;
    }
    if (!(getCallingPlan().equals(pd.getCallingPlan()))) {
      updateCallingPlan(pd.getCallingPlan());
      flag = true;
    }
    if (!(getH235Password().equals(pd.getH235Password()))) {
      updateH235Password(pd.getH235Password());
      flag = true;
    }
    if (isMediaRoutingEnabled() != pd.isMediaRoutingEnabled()) {
      updateMediaRoutingEnabled(pd.isMediaRoutingEnabled());
      flag = true;
    }
    if (isHideAddressChangeEnabled() != pd.isHideAddressChangeEnabled()) {
      updateHideAddressChangeEnabled(pd.isHideAddressChangeEnabled());
      flag = true;
    }
    if (getMaxHunts() != pd.getMaxHunts()) {
      updateMaxHunts(pd.getMaxHunts());
      flag = true;
    }
    if (isQ931DisplayEnabled() != pd.isQ931DisplayEnabled()) {
      updateQ931DisplayEnabled(pd.isQ931DisplayEnabled());
      flag = true;
    }


    if (getLayer1Protocol()  !=  pd.getLayer1Protocol()) {
      updateLayer1Protocol(pd.getLayer1Protocol());
      flag = true;
    }


    if (!flag){
      indexes = new HashSet(oldIndexes);
    }
		
    return flag;
  }

  public synchronized boolean equals (Object o) {
    ProvisionData p = null;

    if (o == null)
      return false;

    if (o instanceof ProvisionData)
      p = (ProvisionData)o;
    else
      return false;

    if ( (vpnList.getVpnName() != null || p.getVpnName() != null) &&
	 (vpnList.getVpnName() == null || p.getVpnName() == null || !vpnList.getVpnName().equals(p.getVpnName())) )
      return false;

    if ( (forwardedPhone != null || p.getFwdPhone() != null) &&
	 (forwardedPhone == null || p.getFwdPhone() == null || !forwardedPhone.equals(p.getFwdPhone())) )
      return false;

    if (callType != p.getCallType()) 
      return false;


    if ( (vpnList.getVpnGroup() != null || p.getVpnGroup() != null) &&
	 (vpnList.getVpnGroup() == null || p.getVpnGroup() == null || !vpnList.getVpnGroup().equals(p.getVpnGroup())) )
      return false;

    if ( (extNumber != null || p.getExtNumber() != null) &&
	 (extNumber == null || p.getExtNumber() == null || !extNumber.equals(p.getExtNumber())) )
      return false;

    if ( (firstName != null || p.getFirstName() != null) &&
	 (firstName == null || p.getFirstName() == null || !firstName.equals(p.getFirstName())) )
      return false;

    if ( (lastName != null || p.getLastName() != null) &&
	 (lastName == null || p.getLastName() == null || !lastName.equals(p.getLastName())) )
      return false;

    if ( (location != null || p.getLocation() != null) &&
	 (location == null || p.getLocation() == null || !location.equals(p.getLocation())) )
      return false;

    if ( (country != null || p.getCountry() != null) &&
	 (country == null || p.getCountry() == null || !country.equals(p.getCountry())) )
      return false;

    if ( (comments != null || p.getComments() != null) &&
	 (comments == null || p.getComments() == null || !comments.equals(p.getComments())) )
      return false;

    if ( (customerId != null || p.getCustomerId() != null) &&
         (customerId == null || p.getCustomerId() == null || !customerId.equals(p.getCustomerId())) )
      return false;

    if ( (trunkGroup != null || p.getTrunkGroup() != null) &&
         (trunkGroup == null || p.getTrunkGroup() == null || !trunkGroup.equals(p.getTrunkGroup())) )
      return false;

    if ( (zone != null || p.getZone() != null) &&
	 (zone == null || p.getZone() == null || !zone.equals(p.getZone())) )
      return false;

    if ( (email != null || p.getEmail() != null) &&
	 (email == null || p.getEmail() == null || !email.equals(p.getEmail())) )
      return false;

    if ( (vpnList.getExtLen() != -1 || p.getExtensionLen() != -1) &&
	 (vpnList.getExtLen() == -1 || p.getExtensionLen() == -1 || vpnList.getExtLen() != p.getExtensionLen()) )
      return false;

    if ( (devType != -1 || p.getDevType() != -1) &&
	 (devType == -1 || p.getDevType() == -1 || devType != p.getDevType()) )
      return false;

    if (h323Enable != p.isH323Enable()) {
      return false;
    }

    if (sipEnable != p.isSipEnable()) {
      return false;
    }

    if (isGRQEnable() != p.isGRQEnable()) {
      return false;
    }

    if (isRAIEnable() != p.isRAIEnable()) {
      return false;
    }

    if (getRasPort() != p.getRasPort()) {
      return false;
    }

    if (getQ931Port() != p.getQ931Port()) {
      return false;
    }

    if ( (priority != -1 || p.getPriority() != -1) &&
	 (priority == -1 || p.getPriority() == -1 || priority != p.getPriority()) )
      return false;

    if ((h323Id != null || p.getH323Id() != null) &&
	(h323Id == null || p.getH323Id() == null || h323Id.equals(p.getH323Id()))) {
      return false;
    }

    if (callpartyType !=  p.getCallpartyType() ) {
      return false;
    }

    if ((sipUri != null || p.getSipUri() != null) &&
	(sipUri == null || p.getSipUri() == null || sipUri.equals(p.getSipUri()))) {
      return false;
    }

    if ((sipContact != null || p.getSipContact() != null) &&
	(sipContact == null || p.getSipContact() == null || sipContact.equals(p.getSipContact()))) {
      return false;
    }

    if ((maxCalls != -1 || p.getMaxCalls() != -1) &&
	(maxCalls == -1 || p.getMaxCalls() == -1 || maxCalls == p.getMaxCalls())) {
      return false;
    }

    if (isTechPrefixEnable() != p.isTechPrefixEnable()) {
      return false;
    }

    if ((techPrefix != null || p.getTechPrefix() != null) &&
	(techPrefix == null || p.getTechPrefix() == null || techPrefix.equals(p.getTechPrefix()))) {
      return false;
    }
    if ((peerGkId != null || p.getPeerGkId() != null) &&
	(peerGkId == null || p.getPeerGkId() == null || peerGkId.equals(p.getPeerGkId()))) {
      return false;
    }
    if (getVendor() != p.getVendor()) {
      return false;
    }
    if (!getSubnetIp().equals(p.getSubnetIp()))
      return false;
    if (!getSubnetMask().equals(p.getSubnetMask()))
      return false;
    if (!getCallingPlan().equals(p.getCallingPlan()))
      return false;
    if (!getH235Password().equals(p.getH235Password()))
      return false;
    if (isMediaRoutingEnabled() != p.isMediaRoutingEnabled())
      return false;
    if (isHideAddressChangeEnabled() != p.isHideAddressChangeEnabled())
      return false;
    if (getMaxHunts() != p.getMaxHunts())
      return false;
    if (isQ931DisplayEnabled() != p.isQ931DisplayEnabled())
      return false;


    if (layer1Protocol!=  p.getLayer1Protocol() ) {
      return false;
    }


    return true;
  }

  public void setVpnList (VpnList vl) {
    vpnList = vl;
  }

  public VpnList getVpnList () {
    return vpnList;
  }


  /**
   * set the vpn Name 
   * @param vpnName 
   */
  public synchronized void setVpnName (String vpnname) {
    vpnList.setVpnName(vpnname);
  }

  public synchronized void updateVpnName (String vpnname) {
    setVpnName(vpnname);
//    indexes.add(new Integer(CommonConstants.VPN_NAME));
  }

  /**
   * get the vpn Id
   * @return vpnId
   */
  public synchronized String getVpnId() {
    return vpnList.getVpnId();
  }


  /**
   * set the vpn Id 
   * @param vpnId 
   */
  public synchronized void setVpnId(String Id) {
    vpnList.setVpnId(Id);
  }

  /**
   * get the vpn Name
   * @return vpnName
   */
  public synchronized String getVpnName() {
    return vpnList.getVpnName();
  }
	
  public synchronized void setFwdPhone (String s) {
    forwardedPhone = (s == null)?"":s;
  }

  public synchronized void updateFwdPhone (String s) {
    setFwdPhone(s);
//    indexes.add(new Integer(CommonConstants.FWD_NUMBER));
  }

  public synchronized String getFwdPhone () {
    return forwardedPhone;
  }

  public synchronized void setCallType (int i) {
    callType = i;
  }

  public synchronized int getCallType () {
    return callType;	
  }

  public synchronized void updateCallType (int i) {
    setCallType(i);
//    indexes.add(new Integer(CommonConstants.CALLTYPE));
  }
	  
  public synchronized void setVpnGroup (String s) {
    vpnList.setVpnGroup(s);
  }

  public synchronized void updateVpnGroup (String s) {
    setVpnGroup(s);
    // if vpngroup is blank do not add the command
//    if( (vpnList.getVpnGroup().length() != 0 ) && (vpnList.getVpnName().length() != 0))
//      indexes.add(new Integer(CommonConstants.VPN_GROUP));
  }

  public synchronized String getVpnGroup () {
    return vpnList.getVpnGroup();
  }


  /**
   * get the Extension
   * @return extNumber 
   */
  public  synchronized String getExtNumber () {
    return extNumber;
  }

  /**
   * set the Extension
   * @param extNumber Extension number 
   */
  public synchronized void setExtNumber (String ext) {
    extNumber = (ext == null)?"":ext;
  }

  public synchronized void updateExtNumber (String ext) {
    setExtNumber(ext);
//    indexes.add(new Integer(CommonConstants.EXTENSION_NUMBER));
  }
	
  /**
   * get the firstname
   * @return firstname
   */
  public synchronized String getFirstName (){
    return firstName;
  }

  /**
   * set the firstname
   * @param fname Firstname
   */

  public synchronized void setFirstName (String fname) {
    this.firstName = (fname == null)?"":fname;
  }

  public synchronized void updateFirstName (String fname) {
    setFirstName(fname);
//    indexes.add(new Integer(CommonConstants.FIRST_NAME));
  }

  /**
   * get the last name
   * @return Lastname
   */
  public synchronized String getLastName () {
    return lastName ;
  }

  /**
   * set the last name
   * @param fname Lastname
   */

  public synchronized void setLastName (String lname) {
    lastName = (lname == null)?"":lname;
  }

  public synchronized void updateLastName (String lname) {
    setLastName(lname);
//    indexes.add(new Integer(CommonConstants.LAST_NAME));
  }

  /**
   * get the location
   * @return location
   */
  public synchronized String getLocation () {
    return location;
  }

  /**
   * set the location
   * @param country location
   */
  public synchronized void setLocation (String location) {
    this.location = (location == null)?"":location;
  }

  public synchronized void updateLocation (String location) {
    setLocation(location);
//  indexes.add(new Integer(CommonConstants.LOCATION));
  }

  /**
   * get the contact
   * @return contact
   */
  public synchronized String getContact () {
    return contact;
  }

  /**
   * set the Contact
   * @param country location
   */
  public synchronized void setContact (String s) {
    contact = (s == null)?"":s;
  }

  /**
   * get the country
   * @return country
   */
  public synchronized String getCountry () {
    return country;
  }

  /**
   * set the country
   * @param country country
   */
  public synchronized void setCountry (String country) {
    this.country = (country == null)?"":country;
  }

  public synchronized void updateCountry (String country) {
    setCountry(country);
//    indexes.add(new Integer(CommonConstants.COUNTRY));
  }

  /**
   * get the comments
   * @return comments 
   */
  public synchronized String getComments () {
    return comments;
  }

  /**
   * set the comments
   * @param comments comments
   */
  public synchronized void setComments (String comments) {
    this.comments = (comments == null)?"":comments;
  }

  public synchronized void updateComments (String comments) {
    setComments(comments);
//    indexes.add(new Integer(CommonConstants.COMMENTS));
  }

  public synchronized String getCustomerId (){
    return customerId;
  }

  public synchronized void setCustomerId (String cid) {
    this.customerId = (cid == null)?"":cid;
  }

  public synchronized void updateCustomerId (String cid) {
    setCustomerId(cid);
  }

  public synchronized String getTrunkGroup (){
    return trunkGroup;
  }

  public synchronized void setTrunkGroup (String tg) {
    this.trunkGroup = (tg == null)?"":tg;
  }

  public synchronized void updateTrunkGroup (String tg) {
    setTrunkGroup(tg);
  }

  /**
   * get the zone
   * @return zone 
   */
  public synchronized String getZone () {
    return zone;
  }

  /**
   * set the zone
   * @param zone zone
   */
  public synchronized void setZone (String zone) {
    this.zone = (zone == null)?"":zone;
  }
	
  public synchronized void updateZone (String zone) {
    setZone(zone);
//    indexes.add(new Integer(CommonConstants.GW_ZONE));
  }

  /**
   * get the email
   * @return email 
   */
  public synchronized String getEmail () {
    return email;
  }

  /**
   * set the email
   * @param email email
   */
  public synchronized void setEmail (String email) {
    this.email = (email == null)?"":email;
  }

  public synchronized void updateEmail (String email) {
    setEmail(email);
//    indexes.add(new Integer(CommonConstants.EMAIL));
  }

  /**
   * get the extension length
   * @return extension length
   */
  public synchronized int getExtensionLen () {
    return vpnList.getExtLen();
  }

  /**
   * get the Device type
   * @return device type 
   */
  public synchronized int getDevType () {
    return devType;
  }
	
  /**
   * set the extension length
   * @param extlen endpoint extension length
   */
  public synchronized void setExtensionLen (int extlen) {
    vpnList.setExtLen(extlen);
  }

  /**
   * set the Device type
   * @param devtype endpoint type
   */
  public synchronized void setDevType (int devtype) {
    this.devType = devtype;
  }

  public synchronized void updateDevType (int devtype) {
    setDevType(devtype);
//    indexes.add(new Integer(CommonConstants.DEV_TYPE));
  }

  /**
   * set the serial number 
   * @param serialNumber serial number for the end point
   */
  public synchronized void setSerialNumber (String sNo) {
    serialNumber = (sNo == null)?"":sNo;
  }

  public synchronized String getSerialNumber () {
    return serialNumber;
  }

  /**
   * set the phone
   * @param phone
   */
  public synchronized void setPhone (String p) {
    phone = (p == null)?"":p;
  }

  public synchronized String getPhone () {
    return phone;
  }

  public synchronized void setCallForwarded (boolean b) {
    callForwarded = b;
  }

  public synchronized boolean isCallRollover() {
    return isCallRollover;
  }


  public synchronized void setGateway (boolean b) {
    isGateway = b;
  }

  public synchronized void updateGateway (boolean b) {
    setGateway(b);
//    indexes.add(new Integer(CommonConstants.ISGATEWAY));
  }

  /**
   * set the port 
   * @param port 
   */
  public synchronized void setPort (int port) {
    this.port = (port <0)?0:port;
  }

  public synchronized int getPort () {
    return port;
  }

  /**
   * get the Vpn Prefix
   * @return VPN prefix
   */
  public synchronized void setVpnPrefix (String s) {
    vpnList.setVpnPrefix(s);
  }

  /**
   * get the Vpn Prefix
   * @return VPN prefix
   */
  public synchronized String getVpnPrefix () {
    return vpnList.getVpnPrefix();
  }

  public synchronized String getAddress () {
    if (ipAddress == 0) // has to be == 0 because some ips can be < zero
      return new String("");

    return IPUtil.intToIPString(ipAddress);
  }

  public synchronized int getAddressAsInt () {
    return ipAddress;
  }

	
  /**
   * Set the IP Address
   * @param addr IPAddress
   */
  public synchronized void setAddress (String addr) {
    if((addr == null) || addr.equals(""))
      ipAddress = 0;
    ipAddress = (int)IPUtil.ipStringToLong(addr);
  }

  public synchronized void updateAddress (String addr) {
    setAddress(addr);
//    indexes.add(new Integer(CommonConstants.IP_ADDRESS));
  }

  public synchronized boolean isCallForwarded () {
    return callForwarded;
  }

  public synchronized boolean isGateway () {
    return isGateway;
  }

  public synchronized boolean isH323Enable () {
    return h323Enable;
  }

  public synchronized void setH323Enable (boolean h323Enable) {
    this.h323Enable = h323Enable;

  }
  public synchronized void updateH323Enable (boolean h323Enable) {
    setH323Enable(h323Enable);
//    indexes.add(new Integer(CommonConstants.H323_ENABLE));
  }

  public synchronized boolean isSipEnable () {
    return sipEnable;
  }

  public synchronized void setSipEnable (boolean sipEnable) {
    this.sipEnable = sipEnable;
  }

  public synchronized void updateSipEnable (boolean sipEnable) {
    setSipEnable(sipEnable);
//    indexes.add(new Integer(CommonConstants.SIP_ENABLE));
  }

  public synchronized boolean isGRQEnable () {
    return grqEnable;
  }

  public synchronized void setGRQEnable (boolean enable) {
    grqEnable = enable;
  }

  public synchronized void updateGRQEnable (boolean enable) {
    setGRQEnable(enable);
//    indexes.add(new Integer(CommonConstants.GRQ_ENABLE));
  }

  public synchronized boolean isRAIEnable () {
    return raiEnable;
  }

  public synchronized void setRAIEnable (boolean enable) {
    raiEnable = enable;
  }

  public synchronized void updateRAIEnable (boolean enable) {
    setRAIEnable(enable);
//    indexes.add(new Integer(CommonConstants.RAI_ENABLE));
  }

  public synchronized boolean isTechPrefixEnable () {
    return techPrefixEnable;
  }

  public synchronized void setTechPrefixEnable (boolean enable) {
    techPrefixEnable = enable;
  }

  public synchronized void updateTechPrefixEnable (boolean enable) {
    setTechPrefixEnable(enable);
//    indexes.add(new Integer(CommonConstants.TECHPREFIX_ENABLE));
  }

  public synchronized int getPriority () {
    return priority;
  }

  public synchronized void setPriority (int val) {
    priority = val;
  }

  public synchronized void updatePriority (int val) {
    setPriority(val);
//    indexes.add(new Integer(CommonConstants.PRIORITY));
  }

  public synchronized int getRasPort () {
    return rasPort;
  }

  public synchronized void setRasPort (int val) {
    rasPort = val;
  }

  public synchronized void updateRasPort (int val) {
    setRasPort(val);
//    indexes.add(new Integer(CommonConstants.RAS_PORT));
  }

  public synchronized int getQ931Port () {
    return q931Port;
  }

  public synchronized void setQ931Port (int val) {
    q931Port = val;
  }

  public synchronized void updateQ931Port (int val) {
    setQ931Port(val);
//    indexes.add(new Integer(CommonConstants.Q931_PORT));
  }

  public synchronized String getH323Id () {
    return h323Id;
  }

  public synchronized void setH323Id (String h323Id) {
    this.h323Id = (h323Id == null)? "":h323Id;
  }

  public synchronized void updateH323Id (String h323Id) {
    setH323Id(h323Id);
//    indexes.add(new Integer(CommonConstants.H323_ID));
  }

  public synchronized int getCallpartyType() {
    return callpartyType;
  }

  public synchronized void setCallpartyType(int callpartyType) {
    this.callpartyType= callpartyType;
  }

  public synchronized void updateCallpartyType(int callpartyType) {
    setCallpartyType(callpartyType);
//    indexes.add(new Integer(CommonConstants.H323_ID));
  }


  public synchronized int getLayer1Protocol() {
    return layer1Protocol;
  }

  public synchronized void setLayer1Protocol(int layer1Protocol) {
    this.layer1Protocol= layer1Protocol;
  }

  public synchronized void updateLayer1Protocol(int layer1Protocol) {
    setLayer1Protocol(layer1Protocol);
//    indexes.add(new Integer(CommonConstants.H323_ID));
  }



  public synchronized String getTechPrefix () {
    return techPrefix;
  }

  public synchronized void setTechPrefix (String prefix) {
    this.techPrefix = (prefix == null)? "":prefix;
  }

  public synchronized void updateTechPrefix (String prefix) {
    setTechPrefix(prefix);
//    indexes.add(new Integer(CommonConstants.TECHPREFIX));
  }

  public synchronized String getPeerGkId () {
    return peerGkId;
  }

  public synchronized void setPeerGkId (String id) {
    this.peerGkId = (id == null)? "":id;
  }

  public synchronized void updatePeerGkId (String id) {
    setPeerGkId(id);
//    indexes.add(new Integer(CommonConstants.PEERGK_ID));
  }

  public synchronized int getVendor () {
    return vendor;
  }

  public synchronized void setVendor (int ven) {
    vendor = ven;
  }

  public synchronized void updateVendor (int ven) {
    setVendor(ven);
//    indexes.add(new Integer(CommonConstants.VENDOR));
  }

  public synchronized String getSipUri () {
    return sipUri;
  }

  public synchronized void setSipUri (String sipUri) {
    this.sipUri = (sipUri==null)?"":sipUri;
  }

  public synchronized void updateSipUri (String sipUri) {
    setSipUri(sipUri);
//    indexes.add(new Integer(CommonConstants.SIP_URI));
  }

  public synchronized String getSipContact () {
    return sipContact;
  }

  public synchronized void setSipContact (String sipContact) {
    this.sipContact = (sipContact==null)?"":sipContact;
  }

  public synchronized void updateSipContact (String sipContact) {
    setSipContact(sipContact);
//    indexes.add(new Integer(CommonConstants.SIP_CONTACT));
  }

  public synchronized int getMaxCalls () {
    return maxCalls;
  }

  public synchronized void setMaxCalls (int maxCalls) {
    this.maxCalls = maxCalls;
  }

  public synchronized void updateMaxCalls (int maxCalls) {
    setMaxCalls(maxCalls);
//    indexes.add(new Integer(CommonConstants.MAX_CALLS));
  }

  public synchronized String getSubnetIp () {
    if (subnetip == 0) // has to be == 0 because some ips can be < zero
      return new String("");

    return IPUtil.intToIPString(subnetip);
  }

  public synchronized int getSubnetIpAsInt () {
    return subnetip;
  }
  public synchronized void setSubnetIp (int ip) {
    subnetip = ip;
  }

  public synchronized void setSubnetIp (String addr) {
    if((addr == null) || addr.equals(""))
      subnetip = 0;
    subnetip = (int)IPUtil.ipStringToLong(addr);
  }

  public synchronized void updateSubnetIp (String addr) {
    setSubnetIp(addr);
//    indexes.add(new Integer(CommonConstants.SUBNETIP));
  }

  public synchronized String getSubnetMask () {
    if (subnetmask == 0) // has to be == 0 because some ips can be < zero
      return new String("");

    return IPUtil.intToIPString(subnetmask);
  }

  public synchronized int getSubnetMaskAsInt () {
    return subnetmask;
  }


  public synchronized void setSubnetMask (int mask) {
    subnetmask = mask;
  }

  public synchronized void setSubnetMask (String addr) {
    if((addr == null) || addr.equals(""))
      subnetmask = 0;
    subnetmask = (int)IPUtil.ipStringToLong(addr);
  }

  public synchronized void updateSubnetMask (String addr) {
    setSubnetMask(addr);
//    indexes.add(new Integer(CommonConstants.SUBNETMASK));
  }

  public synchronized void setCallingPlan (String plan) {
    cpname = plan;
  }

  public synchronized void updateCallingPlan (String plan) {
    setCallingPlan(plan);
//    indexes.add(new Integer(CommonConstants.CALLING_PLAN));
  }

  public synchronized String getCallingPlan () {
    return cpname;
  }

  public synchronized void setH235Password (String pass) {
    h235Password = pass;
  }

  public synchronized void updateH235Password (String pass) {
    setH235Password(pass);
//    indexes.add(new Integer(CommonConstants.H235_PASSWORD));
  }

  public synchronized String getH235Password () {
    return (h235Password == null)?"":h235Password;
  }

  public synchronized boolean isMediaRoutingEnabled () {
    return mediaRouting;
  }

  public synchronized void setMediaRoutingEnabled (boolean enable) {
    mediaRouting = enable;
  }

  public synchronized void updateMediaRoutingEnabled (boolean enable) {
    setMediaRoutingEnabled(enable);
//    indexes.add(new Integer(CommonConstants.MEDIA_ROUTING));
  }

  public synchronized boolean isHideAddressChangeEnabled () {
    return hideAddressChange;
  }

  public synchronized void setHideAddressChangeEnabled (boolean enable) {
    hideAddressChange = enable;
  }

  public synchronized void updateHideAddressChangeEnabled (boolean enable) {
    setHideAddressChangeEnabled(enable);
//    indexes.add(new Integer(CommonConstants.HIDE_ADDRESS_CHANGE));
  }

  public synchronized int getMaxHunts () {
    return maxHunts;
  }

  public synchronized void setMaxHunts (int hunts) {
    maxHunts = hunts;
  }

  public synchronized void updateMaxHunts (int hunts) {
    setMaxHunts(hunts);
//    indexes.add(new Integer(CommonConstants.MAX_HUNTS));
  }

  public synchronized boolean isQ931DisplayEnabled () {
    return q931Display;
  }

  public synchronized void setQ931DisplayEnabled (boolean enable) {
    q931Display = enable;
  }

  public synchronized void updateQ931DisplayEnabled (boolean enable) {
    setQ931DisplayEnabled(enable);
//    indexes.add(new Integer(CommonConstants.Q931_DISPLAY));
  }

  /**
   * get the parameter list 
   * @return String[] List of parameter values
   */	
  public synchronized String [] getParameterList () {
    //  deprecated since 2.0.5. use Commands.createIEdgexxxCommand instead
/*
    Integer [] index = getIndexList();
    if (index == null)
      return null;

    String [] parameters = new String [index.length];

    for (int i=0; i < index.length; i++) {
      switch (index[i].intValue()) {
      case CommonConstants.VPN_NAME:
	parameters[i] = vpnList.getVpnName();
	break;

      case CommonConstants.VPN_GROUP:
	parameters[i] = vpnList.getVpnGroup();
	break;

      case CommonConstants.EXTENSION_NUMBER:
	parameters[i] = extNumber;
	break;

      case CommonConstants.FIRST_NAME:
	parameters[i] = firstName;
	break;

      case CommonConstants.LAST_NAME:
	parameters[i] = lastName;
	break;

      case CommonConstants.LOCATION:
	parameters[i] = location;
	break;

      case CommonConstants.COUNTRY:
	parameters[i] = country;
	break;

      case CommonConstants.COMMENTS:
	parameters[i] = comments;
	break;

      case CommonConstants.GW_ZONE:
	parameters[i] = zone;
	break;

      case CommonConstants.EMAIL:
	parameters[i] = email;
	break;

      case CommonConstants.CONTACT:
	parameters[i] = contact;
	break;

      case CommonConstants.DEV_TYPE:
	parameters[i] = String.valueOf(devType);
	break;

      case CommonConstants.CALLTYPE:
	if (isCallForwarded())
	  parameters[i]	= String.valueOf(callType);
	else
	  parameters[i]	= String.valueOf(CommonConstants.NO_CALLFORWARD);
      break;

      case CommonConstants.FWD_NUMBER:
	parameters[i] = forwardedPhone;
	break;

      case CommonConstants.IP_ADDRESS:
	parameters[i] = getAddress();
	break;

      case CommonConstants.H323_ENABLE:
	parameters[i] = (h323Enable?"enable":"disable");
	break;

      case CommonConstants.SIP_ENABLE:
	parameters[i] = sipEnable?"enable":"disable";
	break;

      case CommonConstants.H323_ID:
	parameters[i] = h323Id;
	break;

      case CommonConstants.SIP_URI:
	parameters[i] = sipUri;
	break;

      case CommonConstants.SIP_CONTACT:
	parameters[i] = sipContact;
	break;

      case CommonConstants.MAX_CALLS:
	parameters[i] = String.valueOf(maxCalls);
	break;

      case CommonConstants.ISGATEWAY:
	parameters[i] = isGateway()?"enable":"disable";
	break;

      case CommonConstants.GRQ_ENABLE:
	parameters[i] = isGRQEnable()?"enable":"disable";
	break;

      case CommonConstants.RAI_ENABLE:
	parameters[i] = isRAIEnable()?"enable":"disable";
	break;

      case CommonConstants.PRIORITY:
	parameters[i] = String.valueOf(priority);
	break;

      case CommonConstants.RAS_PORT:
	parameters[i] = String.valueOf(rasPort);
	break;

      case CommonConstants.Q931_PORT:
	parameters[i] = String.valueOf(q931Port);
	break;

      case CommonConstants.TECHPREFIX_ENABLE:
	parameters[i] = isTechPrefixEnable()?"enable":"disable";
	break;

      case CommonConstants.TECHPREFIX:
	parameters[i] = techPrefix;
	break;

      case CommonConstants.PEERGK_ID:
	parameters[i] = peerGkId;
	break;

      case CommonConstants.VENDOR:
	parameters[i] = IEdgeList.getVendorDescription(vendor);
	break;

      case CommonConstants.SUBNETIP:
	parameters[i] = (getSubnetIp().length() == 0)?"none":getSubnetIp();
	break;

      case CommonConstants.SUBNETMASK:
	parameters[i] = (getSubnetMask().length() == 0)?"none":getSubnetMask();
	break;

      case CommonConstants.CALLING_PLAN:
	parameters[i] = cpname;
	break;

      case CommonConstants.H235_PASSWORD:
	parameters[i] = getH235Password();
	break;

      case CommonConstants.MEDIA_ROUTING:
	parameters[i] = isMediaRoutingEnabled()?"enable":"disable";
	break;

      case CommonConstants.HIDE_ADDRESS_CHANGE:
	parameters[i] = isHideAddressChangeEnabled()?"enable":"disable";
	break;

      case CommonConstants.MAX_HUNTS:
	parameters[i] = String.valueOf(maxHunts);
	break;

      case CommonConstants.Q931_DISPLAY:
	parameters[i] = isQ931DisplayEnabled()?"enable":"disable";
	break;

      default:
	return null;
      }
    }

    return parameters;
    */
    return null;
  }
	
  /**
   * get the parameter name index list 
   * @return String[] List of parameter names index
   */
  public synchronized Integer [] getIndexList () {
    // deprecated  since 2.0.5. Use Commands.createIEdgexxxCommand instead
    /*
    if (indexes.size() == 0) {
      if (vpnList.getVpnName().length() != 0)
	indexes.add(new Integer(CommonConstants.VPN_NAME));
      if (vpnList.getVpnGroup().length() != 0)
	indexes.add(new Integer(CommonConstants.VPN_GROUP));
      if (extNumber.length() != 0)
	indexes.add(new Integer(CommonConstants.EXTENSION_NUMBER));
      if (firstName.length() != 0)
	indexes.add(new Integer(CommonConstants.FIRST_NAME));
      if (lastName.length() != 0)
	indexes.add(new Integer(CommonConstants.LAST_NAME));
      if (location.length() != 0)
	indexes.add(new Integer(CommonConstants.LOCATION));
      if (country.length() != 0)
	indexes.add(new Integer(CommonConstants.COUNTRY));
      if (comments.length() != 0)
	indexes.add(new Integer(CommonConstants.COMMENTS));
      if (email.length() != 0)
	indexes.add(new Integer(CommonConstants.EMAIL));
      if (zone.length() != 0)
	indexes.add(new Integer(CommonConstants.GW_ZONE));
      if (getAddress().length() != 0)
	indexes.add(new Integer(CommonConstants.IP_ADDRESS));
      if (devType != -1)
	indexes.add(new Integer(CommonConstants.DEV_TYPE));
      if ((forwardedPhone.length() != 0) || callForwarded) {
	indexes.add(new Integer(CommonConstants.CALLTYPE));
	indexes.add(new Integer(CommonConstants.FWD_NUMBER));
      }
      if (h323Id.length() != 0) {
	indexes.add(new Integer(CommonConstants.H323_ID));
      }
      if (sipUri.length() != 0) {
	indexes.add(new Integer(CommonConstants.SIP_URI));
      }
      if (sipContact.length() != 0) {
	indexes.add(new Integer(CommonConstants.SIP_CONTACT));
      }
      if (maxCalls != -1) {
	indexes.add(new Integer(CommonConstants.MAX_CALLS));
      }
      indexes.add(new Integer(CommonConstants.H323_ENABLE));
      indexes.add(new Integer(CommonConstants.SIP_ENABLE));
      indexes.add(new Integer(CommonConstants.ISGATEWAY));
      indexes.add(new Integer(CommonConstants.GRQ_ENABLE));
      indexes.add(new Integer(CommonConstants.RAI_ENABLE));
      if (priority != -1) {
	indexes.add(new Integer(CommonConstants.PRIORITY));
      }
      indexes.add(new Integer(CommonConstants.RAS_PORT));
      indexes.add(new Integer(CommonConstants.Q931_PORT));
      indexes.add(new Integer(CommonConstants.TECHPREFIX_ENABLE));
      if (techPrefix.length() != 0)
	indexes.add(new Integer(CommonConstants.TECHPREFIX));
      if (peerGkId.length() != 0)
	indexes.add(new Integer(CommonConstants.PEERGK_ID));
      indexes.add(new Integer(CommonConstants.VENDOR));
      if (getSubnetIp().length() != 0)
	indexes.add(new Integer(CommonConstants.SUBNETIP));
      if (getSubnetMask().length() != 0)
	indexes.add(new Integer(CommonConstants.SUBNETMASK));
      if (getCallingPlan().length() != 0)
	indexes.add(new Integer(CommonConstants.CALLING_PLAN));
      if (getH235Password().length() != 0)
	indexes.add(new Integer(CommonConstants.H235_PASSWORD));
      indexes.add(new Integer(CommonConstants.MEDIA_ROUTING));
      indexes.add(new Integer(CommonConstants.HIDE_ADDRESS_CHANGE));
      indexes.add(new Integer(CommonConstants.MAX_HUNTS));
      indexes.add(new Integer(CommonConstants.Q931_DISPLAY));
    }

    if (indexes.size() == 0)
      return null;

    Integer [] temp = new Integer[indexes.size()];
    return (Integer [])indexes.toArray(temp);
    */
    return null;
  }



  public String getCallForwarding () {
    if (callForwarded) {
      StringBuffer s = new StringBuffer("Forwarding ");
      if (callType	==	CommonConstants.CALLROLLOVER)
	s.append("(Rollover) ");
      s.append("ON  ");
      if (!forwardedPhone.equals(""))
	s.append("Ph: " + forwardedPhone + "  ");
      return new String(s);
    }

    return "";
  }

  /**
   * remove all the elements in the index list
   */
  public synchronized void clearIndexes () {
    indexes.clear();
  }

}
