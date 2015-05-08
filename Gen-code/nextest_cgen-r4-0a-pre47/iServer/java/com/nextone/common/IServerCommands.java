/*
 * @(#)Commands.java	1.9 05/14/02
 */

package com.nextone.common;
import java.io.*;
import java.util.*;
import com.nextone.util.TM;
import com.nextone.common.CallPlan;
import com.nextone.common.CallPlanRoute;
import com.nextone.common.CallPlanBinding;
import com.nextone.common.IEdgeList;
import com.nextone.common.Trigger;
import com.nextone.common.VpnList;
import com.nextone.common.VpnGroupList;
import com.nextone.util.LinkedHashMap;

/**
 * Implementation of cli commands.It defines the commands insertion and 
 * access of commands in the order they were inserted
 **/

public class IServerCommands {

  public static final String ENABLE   = "enable";
  public static final String DISABLE  = "disable";

  public static final String PUBLIC   = "public";
  public static final String PRIVATE  = "private";


  //  command constants
  public static final String IEDGE = "iedge";
  public static final String VPN   = "vpn";
  public static final String VPNG  = "vpng";
  public static final String CP    = "cp";
  public static final String CR    = "cr";
  public static final String TRIGGER  = "trigger";
  public static final String REALM    = "realm";
  public static final String VNET    = "vnet";
  public static final String IGRP     = "igrp"; 

  //  subcommand constants
  public static final String ADD    = "add";
  public static final String EDIT   = "edit";
  public static final String DELETE = "delete";
  public static final String VPNS   = "vpns";
  public static final String PHONES = "phones";
  public static final String ZONE   = "zone";
  public static final String EMAIL  = "email";
  public static final String UP     = "up";
  public static final String DOWN   = "down";

  //  iedge constants
  public static final String IEDGE_FNAME    = "fname";
  public static final String IEDGE_LNAME    = "lname";
  public static final String IEDGE_LOCATION = "location";
  public static final String IEDGE_COUNTRY  = "country";
  public static final String IEDGE_COMMENTS = "comments";
  public static final String IEDGE_TYPE     = "type";
  public static final String IEDGE_FWDTYPE  = "fwdtype";
  public static final String IEDGE_FORWARD  = "forward";
  public static final String IEDGE_ROLLOVER = "rollover";
  public static final String IEDGE_FWDNO    = "fwdno";
  public static final String IEDGE_STATIC   = "static";
  public static final String IEDGE_H323     = "h323";
  public static final String IEDGE_H323ID   = "h323id";
  public static final String IEDGE_CALLPARTYTYPE   = "cdpntype";
  public static final String IEDGE_SIP      = "sip";
  public static final String IEDGE_URI      = "uri";
  public static final String IEDGE_CONTACT  = "contact";
  public static final String IEDGE_XCALLS   = "xcalls";
  public static final String IEDGE_GATEWAY  = "gateway";
  public static final String IEDGE_GRQ      = "grq";
  public static final String IEDGE_RAI      = "rai";
  public static final String IEDGE_PRIORITY = "priority";
  public static final String IEDGE_RASPORT  = "rasport";
  public static final String IEDGE_Q931PORT = "q931port";
  public static final String IEDGE_TPG      = "tpg";
  public static final String IEDGE_TECHP    = "techp";
  public static final String IEDGE_PGKID    = "pgkid";
  public static final String IEDGE_VENDOR   = "vendor";
  public static final String IEDGE_SUBNETIP = "subnetip";
  public static final String IEDGE_SUBNETMASK     =  "subnetmask";
  public static final String IEDGE_CP             = "cp";
  public static final String IEDGE_PASSWD         = "passwd";
  public static final String IEDGE_MEDIAROUTING   = "mediarouting";
  public static final String IEDGE_HIDEADDRESSCHANGE   = "hideaddresschange";
  public static final String IEDGE_MAXHUNTS       = "maxhunts";
  public static final String IEDGE_H323DISPLAY    = "h323display";
  public static final String IEDGE_MAPALIAS = "mapalias";
  public static final String IEDGE_FORCEH245 = "forceh245";
  public static final String IEDGE_CUSTID    = "custid";
  public static final String IEDGE_LAYER1PROTOCOL    = "bcaplayer1";
  public static final String IEDGE_TG        = "tg";
  public static final String IEDGE_NOMEDIAROUTE = "nmr";
  public static final String IEDGE_XINCALLS = "xincalls";
  public static final String IEDGE_XOUTCALLS = "xoutcalls";
  public static final String IEDGE_CONNH245 = "connh245addr";
  public static final String IEDGE_OGP = "ogp";
  public static final String IEDGE_INFOTRANSCAP = "infotranscap";
  public static final String IEDGE_SRCINGRESSTG = "newsrcitg";
  public static final String IEDGE_SETDESTTG = "setdesttg";
  public static final String IEDGE_REALM = "realm";
  public static final String IEDGE_DELTCS2833 = "deltcs2833";
  public static final String IEDGE_DELTCST38  = "deltcst38";
  public static final String IEDGE_NOTG = "removetg";
  public static final String IEDGE_IGRP = "igrp";
  public static final String IEDGE_DTG       = "dtg";
  public static final String IEDGE_NEWSRCDTG = "newsrcdtg";
  public static final String IEDGE_PIONFASTSTART = "pionfaststart";
  public static final String IEDGE_CAP2833 = "2833capable";
  public static final String IEDGE_PRIVACY    = "privacy";
  public static final String IEDGE_ISDNMAPCC = "mapcc";
  public static final String IEDGE_NATDETECT  = "natdetect";
  public static final String IEDGE_NATIP      = "natip";
  public static final String IEDGE_NATPORT    = "natport";

  //  vpn constants
  public static final String VPN_PREFIX   = "prefix";
  public static final String VPN_LOCATION = "location";
  public static final String VPN_CONTACT  = "contact";

  //  cr constants
  public static final String CR_SRC       = "src";
  public static final String CR_SRCLEN    = "srclen";
  public static final String CR_DEST      = "dest";
  public static final String CR_DESTLEN   = "destlen";
  public static final String CR_PREFIX    = "prefix";
  public static final String CR_CALLTYPE  = "calltype";
  public static final String CR_SRCPREFIX = "srcprefix";
  public static final String CR_DNISDEFAULT  = "dnisdefault";
  public static final String CR_TEMPLATE  = "template";
  public static final String CR_STICKY    = "sticky";

  public static final String CR_TYPE_SOURCE       = "source";
  public static final String CR_TYPE_DESTINATION  = "dest";
  public static final String CR_TYPE_TRANSIT  = "transit";
  public static final String CR_TYPE  = "type";


  //  cpb constants
  public static final String CPB_FORWARD  = "forward";
  public static final String CPB_ROLLOVER = "rollover";
  public static final String NONE         = "none";
  public static final String CPB_PRIORITY = "priority";
  public static final String CPB_FTYPE    = "ftype";
  public static final String CPB_STIME    = "stime";
  public static final String CPB_FTIME    = "ftime";
  public static final String CPB_TYPE    = "type";


  // trigger constants
  public static final String TRIGGER_SRC_VENDOR   = "srcvendor";
  public static final String TRIGGER_DST_VENDOR   = "dstvendor";
  public static final String TRIGGER_SDATA        = "sdata";
  public static final String TRIGGER_OVERRIDE     = "override";


  // realm constants
  public static final String REALM_IFNAME = "ifname";
  public static final String REALM_RSA  = "rsa";
  public static final String REALM_MASK = "mask";
  public static final String REALM_ADDR = "addr";
  public static final String REALM_SIGPOOL  = "sigpool";
  public static final String REALM_MEDPOOL  = "medpool";
  public static final String REALM_IMR     = "imr";
  public static final String REALM_EMR     = "emr";
  public static final String REALM_ADMIN    = "admin";
  public static final String REALM_SIPAUTH    = "sipauth";
  public static final String REALM_CIDBLOCK   = "cidblock";
  public static final String REALM_CIDUNBLOCK = "cidunblock";
  public static final String REALM_PROXY_REGID = "proxy_regid";
  public static final String REALM_PROXY_UPORT = "proxy_uport";
  public static final String REALM_VNETNAME = "vnet";

  // vnet constants
  public static final String VNET_IFNAME = "ifname";
  public static final String VNET_ID  = "id";
  public static final String VNET_GATEWAY = "gw";

  // igrp constants
  public static final String IGRP_MAXIN    = "maxcallsin";
  public static final String IGRP_MAXOUT   = "maxcallsout";
  public static final String IGRP_MAXTOTAL = "maxcallstotal";

  // defined in serverdb.h
  public   static final String [] vendorDescription = {
    "Generic",
    "Clarent",
    "SonusGSX",
    "Broadsoft",
    "Cisco",
    "VocalTec",
    "LucentTnt",
    "Excel",
  };

  // defined in key.h
  public   static final String [] triggerTypes = {
    "H323RequestModeFax",
  };

  public   static final String [] triggerScripts = {
    "Insert Route",
  };


  public static final String INFOTRANSCAP_DEFAULT = "Default";
  public static final String INFOTRANSCAP_PASS = "Pass";

  public static final int ADDRESS_TYPE_PUBLIC  = 0;
  public static final int ADDRESS_TYPE_PRIVATE = 1;


  public static final String[] infoTransCapStr = {
    "Speech",
    "Unrestricted",
    "Restricted",
    "Audio",
    "Unrestrictedtones",
    "Video",
    "Pass",
    "Default"
  };

  public static final String[] deltcsStr = {
    "enable",
    "disable",
    "default"
  };

  public static final String[] cap2833Str = {
    "yes",
    "no",
    "unknown"
  };

  // defined in serverdb.h
  public static final int[] infoTransCapInt = {
    100, //INFO_TRANSCAP_SPEECH
    108, //INFO_TRANSCAP_UNRESTRICTED
    109, //INFO_TRANSCAP_RESTRICTED 
    116, //INFO_TRANSCAP_AUDIO
    117, //INFO_TRANSCAP_UNRESTRICTEDTONES
    124, //INFO_TRANSCAP_VIDEO
    1,   //INFO_TRANSCAP_PASS
    0    //INFO_TRANSCAP_DEFAULT
  };


  // defined in key.h
  public static String[]  mediaRouting  = {
    "xxx",
    "Alwayson",
    "Alwaysoff",
    "On"
  };

  public static String[]  displayMediaRouting  = {
    "Don't care",
    "Always On",
    "Always Off",
    "On"
  };

  public static String getVendorDescription (int ven) {
    if (ven < 0 || ven >= vendorDescription.length)
      return "unknown";
    return vendorDescription[ven];
  }

  public static String getTriggerType(int type) {
    if (type >= triggerTypes.length)
      return "";
    return triggerTypes[type];
  }

  public static String getTriggerScripts(int script) {
    if (script >= triggerScripts.length)
      return "";
    return triggerScripts[script];
  }


  public static String getInfoTransCapString(int cap){

    return infoTransCapStr[getInfoTransCapIntIndex(cap)];
  }

  public static int getInfoTransCapInt(String cap){
    return infoTransCapInt[getInfoTransCapStringIndex(cap)];
  }

  public static int getInfoTransCapIntIndex(int cap){
    for(int i=0; i < infoTransCapInt.length; i++){
      if(cap  ==  infoTransCapInt[i])
        return i;
    }
    //default pass;
    return 6;
  }

  public static int getInfoTransCapStringIndex(String cap){
    for(int i=0; i < infoTransCapStr.length; i++){
      if(cap.equals(infoTransCapStr[i]))
        return i;
    }
    //default pass;
    return 6;
  }

  public static String getDeltcs2833String(int cap) {
    return (((cap < deltcsStr.length) && (cap >= 0))? deltcsStr[cap] : "");
  }

  public static String getDeltcst38String(int cap) {
    return getDeltcs2833String(cap);
  }

  public static String getCap2833String(int cap) {
    return (((cap < cap2833Str.length) && (cap >= 0))? cap2833Str[cap] : "");
  }

  public static Commands createNewRouteCommand(CallPlanRoute cpr, Capabilities cap){
    Commands  crCommand = new Commands(CR);
    crCommand.setPrimaryKey(cpr.getCallPlanRouteName());
    crCommand.addSubCommand(Commands.ADD);

    LinkedHashMap map = new LinkedHashMap();
    String val  = cpr.getSource();
    if( val != null &&  val.length()  !=  0)
      map.put(CR_SRC,val);

    map.put(CR_SRCLEN,String.valueOf(cpr.getSourceLength()));

    val  = cpr.getDestination();
    if( val != null &&  val.length()  !=  0)
      map.put(CR_DEST,val);
    map.put(CR_DESTLEN,String.valueOf(cpr.getDestinationLength()));

    val  = cpr.getPrefix();
    if( val != null &&  val.length()  !=  0)
      map.put(CR_PREFIX,val);

    val  = cpr.getSrcPrefix();
    if( val != null &&  val.length()  !=  0)
      map.put(CR_SRCPREFIX,val);
    val   = cpr.getRouteType();
    if( val.equals(CR_TYPE_SOURCE)  || 
        val.equals(CR_TYPE_DESTINATION ) ||
        val.equals(CR_TYPE_TRANSIT)  
        ) {
      map.put(CR_CALLTYPE,val);
    }

    val  = cpr.getType();
    if( val != null &&  val.length()  !=  0)
      map.put(CR_TYPE,val);

    map.put(CR_DNISDEFAULT, cpr.isDNISDefault()?ENABLE:DISABLE);
    map.put(CR_TEMPLATE, cpr.isTemplateRoute()?ENABLE:DISABLE);
  
    if (cap != null && cap.getDBCapability().isStickyRouteEnabled()) 
      map.put(CR_STICKY, cpr.isStickyRoute()?ENABLE:DISABLE);
    
    if(map.size() > 0)
      crCommand.addSubCommand(Commands.EDIT,map);
    return crCommand;
  }



  public static Commands createRouteEditCommand(CallPlanRoute oldCpr,CallPlanRoute newCpr, 
                             Capabilities cap) {
    Commands  crCommand = new Commands(CR);
    crCommand.setPrimaryKey(newCpr.getCallPlanRouteName());

    LinkedHashMap map = new LinkedHashMap();
    String oldVal  = oldCpr.getSource();
    String newVal  = newCpr.getSource();
    if( !newVal.equals(oldVal))
      map.put(CR_SRC,newVal);

    int i = oldCpr.getSourceLength();
    int j = newCpr.getSourceLength();
    if(j  !=  i)
      map.put(CR_SRCLEN,String.valueOf(j));

    oldVal  = oldCpr.getDestination();
    newVal  = newCpr.getDestination();
    if(!newVal.equals(oldVal))
      map.put(CR_DEST,newVal);

    i = oldCpr.getDestinationLength();
    j = newCpr.getDestinationLength();
    if(j  !=  i)
      map.put(CR_DESTLEN,String.valueOf(j));

    oldVal  = oldCpr.getPrefix();
    newVal  = newCpr.getPrefix();
    if(!newVal.equals(oldVal))
      map.put(CR_PREFIX,newVal);

    oldVal  = oldCpr.getSrcPrefix();
    newVal  = newCpr.getSrcPrefix();
    if(!newVal.equals(oldVal))
      map.put(CR_SRCPREFIX,newVal);


    oldVal   = oldCpr.getRouteType();
    newVal   = newCpr.getRouteType();
    if(!newVal.equals(oldVal)){
      map.put(CR_CALLTYPE,newVal);
    }

    oldVal   = oldCpr.getType();
    newVal   = newCpr.getType();
    if(!newVal.equals(oldVal)){
      map.put(CR_TYPE,newVal);
    }

    if( oldCpr.isDNISDefault()  !=  newCpr.isDNISDefault())
      map.put(CR_DNISDEFAULT, newCpr.isDNISDefault()?ENABLE:DISABLE);

    if( oldCpr.isTemplateRoute()  !=  newCpr.isTemplateRoute())
        map.put(CR_TEMPLATE, newCpr.isTemplateRoute()?ENABLE:DISABLE);

    if (cap != null && cap.getDBCapability().isStickyRouteEnabled()) {
      if( oldCpr.isStickyRoute()  !=  newCpr.isStickyRoute())
        map.put(CR_STICKY, newCpr.isStickyRoute()?ENABLE:DISABLE);
    }

    if(map.size() > 0)
      crCommand.addSubCommand(Commands.EDIT,map);

    if(crCommand.size() > 0)
      return crCommand;
    else
      return null;
  }



  public static Commands createRouteDeleteCommand(CallPlanRoute cpr){
    Commands  crCommand = new Commands(CR);
    crCommand.setPrimaryKey(cpr.getCallPlanRouteName());
    crCommand.addSubCommand(Commands.DELETE);
    return crCommand;
  }

  public static Commands createNewCPCommand(CallPlan cp){
    Commands cpCommand = new Commands(CP);
    cpCommand.setPrimaryKey(cp.getCallPlanName());
    cpCommand.addSubCommand(Commands.ADD);
    return cpCommand;
  }

  public static Commands createCPDeleteCommand(CallPlan cp){
    Commands cpCommand = new Commands(CP);
    cpCommand.setPrimaryKey(cp.getCallPlanName());
    cpCommand.addSubCommand(Commands.DELETE);
    return cpCommand;
  }

  public static Commands createNewCPBCommand(CallPlanBinding cpb){
      Commands cpbCommand = new Commands(CP,cpb.getCallPlanName(),cpb.getCallPlanRouteName());
      cpbCommand.addSubCommand(Commands.ADD);

      LinkedHashMap cpbMap  = new LinkedHashMap();
      String value = String.valueOf(cpb.getPriority());
      cpbMap.put(CPB_PRIORITY,value);

      value = cpb.getRouteType();
      if(value  !=  null  &&  value.length()  !=  0)
        cpbMap.put(CPB_FTYPE,value);

      value = cpb.getType();
      if(value  !=  null  &&  value.length()  !=  0)
        cpbMap.put(CPB_TYPE,value);

      if(cpb.getStartTM() !=  null)
        cpbMap.put(CPB_STIME,cpb.getStartTM().getString());

      if(cpb.getEndTM() !=  null)
        cpbMap.put(CPB_FTIME,cpb.getEndTM().getString());
      if(cpbMap.size() > 0)
        cpbCommand.addSubCommand(EDIT,cpbMap);
      return cpbCommand;
  }


  public static Commands createCPBDeleteCommand(CallPlanBinding cpb){
      Commands cpbCommand = new Commands(CP,cpb.getCallPlanName(),cpb.getCallPlanRouteName());
      cpbCommand.addSubCommand(Commands.DELETE);
      return cpbCommand;
  }

  public static Commands createCPBEditCommand(CallPlanBinding oldCpb,CallPlanBinding newCpb){
      Commands cpbCommand = new Commands(CP,newCpb.getCallPlanName(),newCpb.getCallPlanRouteName());

      LinkedHashMap cpbMap  = new LinkedHashMap();

      int oldPriority = oldCpb.getPriority();
      int newPriority = newCpb.getPriority();      
      String oldValue = "";
      String newValue = "";
      if(oldPriority  !=  newPriority ){
        newValue  = String.valueOf(newPriority);
        cpbMap.put(CPB_PRIORITY,newValue);
      }
      oldValue  = oldCpb.getRouteType();
      newValue  = newCpb.getRouteType();

      if(!newValue.equals(oldValue))
        cpbMap.put(CPB_FTYPE,newValue);

      oldValue  = oldCpb.getType();
      newValue  = newCpb.getType();

      if(!newValue.equals(oldValue))
        cpbMap.put(CPB_TYPE,newValue);


      TM  oldTM = oldCpb.getStartTM();
      TM  newTM = newCpb.getStartTM();

      if(!newTM.equals(oldTM))
        cpbMap.put(CPB_STIME,newTM.getString());

      oldTM = oldCpb.getEndTM();
      newTM = newCpb.getEndTM();

      if(!newTM.equals(oldTM))
        cpbMap.put(CPB_FTIME,newTM.getString());
      if(cpbMap.size() > 0)
        cpbCommand.addSubCommand(Commands.EDIT,cpbMap);
      if(cpbCommand.size() > 0)
        return cpbCommand;
      else
        return null;

  }


  public static Commands createNewGroupCommand(VpnGroupList grp){
    Commands command = new Commands(VPNG);
    command.setPrimaryKey(grp.getVpnGroup());
    command.addSubCommand(Commands.ADD);
    return command;
  }

  public static Commands createGroupDeleteCommand(VpnGroupList grp){
      Commands command  = new Commands(VPNG);
      command.setPrimaryKey(grp.getVpnGroup());
      command.addSubCommand(Commands.DELETE);
      return command;
  }
  public static Commands createNewVpnCommand(VpnList vpn){
    Commands command = new Commands(VPN);
    command.setPrimaryKey(vpn.getVpnName());
    String  properties[];
    int size  = 2;
    String grp  = vpn.getVpnGroup();
    if(grp  !=  null  &&  grp.length()  !=  0)
      size++;
    properties  = new String[size];
    properties[0] = vpn.getVpnId();
    properties[1] = String.valueOf(vpn.getExtLen());
    if(grp  !=  null  &&  grp.length()  !=  0)
      properties[2] = grp;
    command.addSubCommand(Commands.ADD,properties);

    LinkedHashMap map  = new LinkedHashMap();

    String data = vpn.getVpnPrefix();
    if(data !=  null  &&  data.length() !=  0)
      map.put(VPN_PREFIX,vpn.getVpnPrefix());

    data = vpn.getVpnLocation();
    if(data !=  null  &&  data.length() !=  0)
      map.put(VPN_LOCATION,vpn.getVpnLocation());

    data = vpn.getVpnContact();
    if(data !=  null  &&  data.length() !=  0)
      map.put(VPN_CONTACT,vpn.getVpnContact());

    if(map.size() > 0)
      command.addSubCommand(Commands.EDIT,map);

    return command;
  }

  public static Commands createVpnDeleteCommand(VpnList vpn){
      Commands command  = new Commands(VPN);
      command.setPrimaryKey(vpn.getVpnName());
      command.addSubCommand(Commands.DELETE);
      return command;
  }

  public static Commands createVpnEditCommand(VpnList oldVpn, VpnList newVpn){
    Commands command = new Commands(VPN);
    command.setPrimaryKey(newVpn.getVpnName());

    LinkedHashMap map  = new LinkedHashMap();
    if( oldVpn.getVpnPrefix()  !=  null  &&
        newVpn.getVpnPrefix() !=  null  &&
      !oldVpn.getVpnPrefix().equals(newVpn.getVpnPrefix())
      ){
        map.put(VPN_PREFIX,newVpn.getVpnPrefix());
    }

    if( oldVpn.getVpnLocation()  !=  null  &&
        newVpn.getVpnLocation() !=  null  &&
      !oldVpn.getVpnLocation().equals(newVpn.getVpnLocation())
      ){
        map.put(VPN_LOCATION,newVpn.getVpnLocation());
    }

    if( oldVpn.getVpnContact()  !=  null  &&
        newVpn.getVpnContact() !=  null  &&
      !oldVpn.getVpnContact().equals(newVpn.getVpnContact())
      ){
        map.put(VPN_CONTACT,newVpn.getVpnContact());
    }
    if(map.size() > 0)
      command.addSubCommand(Commands.EDIT,map);

    if( oldVpn.getVpnGroup()  !=  null  &&
        newVpn.getVpnGroup() !=  null  &&
      !oldVpn.getVpnGroup().equals(newVpn.getVpnGroup())
      ){
        command.addSubCommand(VPNG,newVpn.getVpnGroup());
    }

    if(command.size() > 0)
      return command;
    else
      return null;

  }


  public static Commands createNewIEdgeCommand(IEdgeList iedge, Capabilities cap){
    Commands command = new Commands(IEDGE);
    command.setKeys(iedge.getSerialNumber(),String.valueOf(iedge.getPort()));

    command.addSubCommand(Commands.ADD);

    String data = iedge.getVpnName();
    if(data !=  null  &&  data.length() !=  0)
      command.addSubCommand(VPNS,data);

    data = iedge.getExtNumber();
    if(data !=  null  &&  data.length() !=  0)
      command.addSubCommand(PHONES,data);

    LinkedHashMap map  = new LinkedHashMap();
    data  = iedge.getFirstName();
    if(data !=  null  &&  data.length() !=  0)
      map.put(IEDGE_FNAME,data);

    data  = iedge.getLastName();
    if(data !=  null  &&  data.length() !=  0)
      map.put(IEDGE_LNAME,data);

    data  = iedge.getLocation();
    if(data !=  null  &&  data.length() !=  0)
      map.put(IEDGE_LOCATION ,data);

    data  = iedge.getCountry();
    if(data !=  null  &&  data.length() !=  0)
      map.put(IEDGE_COUNTRY,data);

    data  = iedge.getComments();
    if(data !=  null  &&  data.length() !=  0)
      map.put(IEDGE_COMMENTS ,data);

    data = iedge.getCustomerId();
    if (data != null && data.length() != 0)
      map.put(IEDGE_CUSTID, data);

    data = iedge.getTrunkGroup();
    if (data != null && data.length() != 0)
      map.put(IEDGE_TG, data);

    data = iedge.getOutgoingPrefix();
    if (data != null && data.length() != 0)
      map.put(IEDGE_OGP, data);

    data  = String.valueOf(iedge.getDeviceType());
    if(data !=  null  &&  data.length() !=  0)
      map.put(IEDGE_TYPE,data);

    boolean calltype  = iedge.isCallRolledOver();
    if(calltype)
      map.put(IEDGE_FWDTYPE ,IEDGE_ROLLOVER);
    else{
      calltype  = iedge.isCallForwarded();
      if(calltype)
        map.put(IEDGE_FWDTYPE ,IEDGE_FORWARD);
    }

    data  = iedge.getForwardedPhone();
    if(data !=  null  &&  data.length() !=  0)
      map.put(IEDGE_FWDNO,data);

    data  = iedge.getAddress();
    if(data !=  null  &&  data.length() !=  0)
      map.put(IEDGE_STATIC,data);

    map.put(IEDGE_H323, iedge.isH323Enable()?ENABLE:DISABLE);

    data  = iedge.getH323Id();
    if(data !=  null  &&  data.length() !=  0)
      map.put(IEDGE_H323ID,data);

    data  = iedge.getCallpartyTypeCommand();
    if(data !=  null  &&  data.length() !=  0)
      map.put(IEDGE_CALLPARTYTYPE,data);

    data = iedge.getLayer1ProtocolCommand();
    if (data != null && data.length() != 0)
      map.put(IEDGE_LAYER1PROTOCOL, data);

    map.put(IEDGE_SIP, iedge.isSipEnable()?ENABLE:DISABLE);


    data  = iedge.getSipUri();
    if(data !=  null  &&  data.length() !=  0)
      map.put(IEDGE_URI,data);

    data  = iedge.getSipContact();
    if(data !=  null  &&  data.length() !=  0)
      map.put(IEDGE_CONTACT,data);

    map.put(IEDGE_XCALLS,String.valueOf(iedge.getMaxCalls()));

    map.put(IEDGE_XINCALLS, String.valueOf(iedge.getMaxInCalls()));

    map.put(IEDGE_XOUTCALLS, String.valueOf(iedge.getMaxOutCalls()));

    map.put(IEDGE_GATEWAY, iedge.isGateway()?ENABLE:DISABLE);

    map.put(IEDGE_GRQ, iedge.isGRQEnable()?ENABLE:DISABLE);

    map.put(IEDGE_RAI, iedge.isRAIEnable()?ENABLE:DISABLE);

    map.put(IEDGE_PRIORITY ,String.valueOf(iedge.getPriority()));

    map.put(IEDGE_RASPORT,String.valueOf(iedge.getRasPort()));

    map.put(IEDGE_Q931PORT ,String.valueOf(iedge.getQ931Port()));

    map.put(IEDGE_TPG, iedge.isTechPrefixEnable()?ENABLE:DISABLE);

    data  = iedge.getTechPrefix();
    if(data !=  null  &&  data.length() !=  0)
      map.put(IEDGE_TECHP,data);

    data  = iedge.getPeerGkId();
    if(data !=  null  &&  data.length() !=  0)
      map.put(IEDGE_PGKID ,data);

    data  = getVendorDescription(iedge.getVendor());
    if(data !=  null  &&  data.length() !=  0)
      map.put(IEDGE_VENDOR,data);

    data  = iedge.getSubnetIp();
    if(data !=  null  &&  data.length() !=  0)
      map.put(IEDGE_SUBNETIP ,data);

    data  = iedge.getSubnetMask();
    if(data !=  null  &&  data.length() !=  0)
      map.put(IEDGE_SUBNETMASK ,data);

    data  = iedge.getCallingPlanName();
    if(data !=  null  &&  data.length() !=  0)
      map.put(IEDGE_CP,data);

    data  = iedge.getH235Password();
    if(data !=  null  &&  data.length() !=  0)
      map.put(IEDGE_PASSWD,data);

    map.put(IEDGE_MEDIAROUTING, iedge.isMediaRoutingEnabled()?ENABLE:DISABLE);

    map.put(IEDGE_NOMEDIAROUTE, iedge.isNeverMediaRouteEnabled()?ENABLE:DISABLE);

    map.put(IEDGE_HIDEADDRESSCHANGE, iedge.isHideAddressChangeEnabled()?ENABLE:DISABLE);

    map.put(IEDGE_MAXHUNTS,String.valueOf(iedge.getMaxHunts()));

    map.put(IEDGE_H323DISPLAY, iedge.isQ931DisplayEnabled()?ENABLE:DISABLE);

    map.put(IEDGE_MAPALIAS, iedge.isMapAliasEnabled()?ENABLE:DISABLE);

    map.put(IEDGE_FORCEH245, iedge.isForceH245Enabled()?ENABLE:DISABLE);

    map.put(IEDGE_CONNH245, iedge.isConnectH245AddressEnabled()?ENABLE:DISABLE);

    data  = IServerCommands.getInfoTransCapString(iedge.getInfoTransCap());
    if(data !=  null  &&  data.length() !=  0)
      map.put(IEDGE_INFOTRANSCAP,data.toLowerCase());

    if (cap != null && cap.getSystemCapability().isTcs2833Enabled()) {
      data  = IServerCommands.getDeltcs2833String(iedge.getDeltcs2833Cap());
      if(data !=  null  &&  data.length() !=  0)
        map.put(IEDGE_DELTCS2833, data.toLowerCase());
    }

    if (cap != null && cap.getSystemCapability().isTcsT38Enabled()) {
      data  = IServerCommands.getDeltcst38String(iedge.getDeltcst38Cap());
      if(data !=  null  &&  data.length() !=  0)
        map.put(IEDGE_DELTCST38, data.toLowerCase());
    }

    if (cap != null && cap.getDBCapability().isCAP2833Enabled()) {
      data  = IServerCommands.getCap2833String(iedge.getCAP2833Cap());
      if(data !=  null  &&  data.length() !=  0)
        map.put(IEDGE_CAP2833, data.toLowerCase());
    }

    if (cap != null && cap.getDBCapability().isRemoveTGEnabled()) 
      map.put(IEDGE_NOTG, iedge.isRemoveTGEnabled()?ENABLE:DISABLE);

    data  = iedge.getSrcIngressTg();
    if(data !=  null  &&  data.length() !=  0)
      map.put(IEDGE_SRCINGRESSTG,data);

    map.put(IEDGE_SETDESTTG, iedge.isDestTg()?ENABLE:DISABLE);

    if (cap != null && cap.getDBCapability().isRealmEnabled()) {
      data  = iedge.getRealmName();
      if(data !=  null  &&  data.length() !=  0)
        map.put(IEDGE_REALM,data);
    }

    data = iedge.getZone();
    if(data !=  null  &&  data.length() !=  0)
      command.addSubCommand(ZONE,data);

    data = iedge.getEmail();
    if(data !=  null  &&  data.length() !=  0)
      command.addSubCommand(EMAIL,data);

    if (cap != null && cap.getDBCapability().isIGRPEnabled()) {
      data = iedge.getIgrpName();
      if(data !=  null  &&  data.length() !=  0)
	  map.put(IEDGE_IGRP, data);
    }

    if (cap != null && cap.getDBCapability().isDtgEnabled()) {
      data = iedge.getDtg();
      if(data !=  null  &&  data.length() !=  0)
        map.put(IEDGE_DTG, data);

      data = iedge.getNewSrcDtg();
      if(data !=  null  &&  data.length() !=  0)
        map.put(IEDGE_NEWSRCDTG, data);
    }
    if (cap != null && cap.getDBCapability().isPIEnabled()) {
      map.put(IEDGE_PIONFASTSTART, iedge.isPiOnFastStartEnabled()?ENABLE:DISABLE);
    }

    if (cap != null && cap.getSipCapability().isSipNatEnabled()) {
        map.put(IEDGE_NATDETECT, iedge.isAutoNatDetectionEnabled()?ENABLE:DISABLE);
        map.put(IEDGE_NATIP, iedge.getNatIp());
        map.put(IEDGE_NATPORT, String.valueOf(iedge.getNatPort()));
    }
    if (cap != null && cap.getSipCapability().isSipPrivacyEnabled()) 
       map.put(IEDGE_PRIVACY, iedge.getSipPrivacy());
 
    if (cap != null && cap.getDBCapability().isISDNMapccEnabled()) {
        map.put(IEDGE_ISDNMAPCC, iedge.isISDNMapccEnabled()?ENABLE:DISABLE);
    }

    if(map.size() > 0)
      command.addSubCommand(Commands.EDIT,map);

    return command;
  }

  public static Commands createIEdgeDeleteCommand(IEdgeList iedge){
      Commands command  = new Commands(IEDGE,iedge.getSerialNumber(),String.valueOf(iedge.getPort()));
      command.addSubCommand(Commands.DELETE);
      return command;
  }


  public static Commands createIEdgeEditCommand(IEdgeList oldIedge,IEdgeList newIedge, Capabilities cap) {
    Commands command = new Commands(IEDGE);
    command.setKeys(newIedge.getSerialNumber(),String.valueOf(newIedge.getPort()));

    String oldData  = oldIedge.getVpnName();
    String newData  = newIedge.getVpnName();
    if( oldData !=  null  &&
        newData !=  null  &&
        !oldData.equals(newData)
        )
      command.addSubCommand(VPNS,newData);

    oldData = oldIedge.getExtNumber();
    newData = newIedge.getExtNumber();
    if( oldData  !=  null  &&  
        newData !=  null  &&
        !oldData.equals(newData)
        )
      command.addSubCommand(PHONES,newData);

    LinkedHashMap map  = new LinkedHashMap();

    oldData = oldIedge.getFirstName();
    newData = newIedge.getFirstName();
    if(oldData  !=  null  &&  
        newData !=  null  &&
        !oldData.equals(newData)
        )
      map.put(IEDGE_FNAME,newData);

    oldData = oldIedge.getLastName();
    newData = newIedge.getLastName();
    if(oldData  !=  null  &&  
        newData !=  null  &&
        !oldData.equals(newData)
        )
      map.put(IEDGE_LNAME,newData);

    oldData = oldIedge.getLocation();
    newData = newIedge.getLocation();
    if(oldData  !=  null  &&  
        newData !=  null  &&
        !oldData.equals(newData)
        )
      map.put(IEDGE_LOCATION ,newData);

    oldData = oldIedge.getCountry();
    newData = newIedge.getCountry();
    if(oldData  !=  null  &&  
        newData !=  null  &&
        !oldData.equals(newData)
        )
      map.put(IEDGE_COUNTRY,newData);

    oldData = oldIedge.getComments();
    newData = newIedge.getComments();
    if(oldData  !=  null  &&  
        newData !=  null  &&
        !oldData.equals(newData)
        )
      map.put(IEDGE_COMMENTS ,newData);

    oldData = oldIedge.getCustomerId();
    newData = newIedge.getCustomerId();
    if(oldData  != null  &&
       newData  != null  &&
       !oldData.equals(newData)
       )
      map.put(IEDGE_CUSTID, newData);

    oldData = oldIedge.getTrunkGroup();
    newData = newIedge.getTrunkGroup();
    if(oldData  != null  &&
       newData  != null  &&
       !oldData.equals(newData)
       )
      map.put(IEDGE_TG, newData);

    oldData = oldIedge.getOutgoingPrefix();
    newData = newIedge.getOutgoingPrefix();
    if(oldData  != null  &&
       newData  != null  &&
       !oldData.equals(newData)
       )
      map.put(IEDGE_OGP, newData);

    oldData   = String.valueOf(oldIedge.getDeviceType());
    newData   = String.valueOf(newIedge.getDeviceType());
    if(oldData  !=  null  &&
      newData !=  null  &&
      !oldData.equals(newData)
      )
      map.put(IEDGE_TYPE,newData);


    if(oldIedge.isCallRolledOver()  !=  newIedge.isCallRolledOver()){
      if(newIedge.isCallRolledOver())
        map.put(IEDGE_FWDTYPE ,IEDGE_ROLLOVER);
      else{
        if(newIedge.isCallForwarded())
          map.put(IEDGE_FWDTYPE ,IEDGE_FORWARD);
      }
    }

    oldData = oldIedge.getForwardedPhone();
    newData = newIedge.getForwardedPhone();
    if(oldData  !=  null  &&  
        newData !=  null  &&
        !oldData.equals(newData)
        )
      map.put(IEDGE_FWDNO,newData);


    oldData = oldIedge.getAddress();
    newData = newIedge.getAddress();
    if(oldData  !=  null  &&  
        newData !=  null  &&
        !oldData.equals(newData)
        )
      map.put(IEDGE_STATIC,newData);

    if(oldIedge.isH323Enable()  !=  newIedge.isH323Enable()){
      map.put(IEDGE_H323, newIedge.isH323Enable()?ENABLE:DISABLE);
    }

    oldData = oldIedge.getH323Id();
    newData = newIedge.getH323Id();
    if(oldData  !=  null  &&  
        newData !=  null  &&
        !oldData.equals(newData)
        )
      map.put(IEDGE_H323ID,newData);

    if(oldIedge.getCallpartyType() !=   newIedge.getCallpartyType()){
      map.put(IEDGE_CALLPARTYTYPE, newIedge.getCallpartyTypeCommand());
    }

    if(oldIedge.getLayer1Protocol() !=   newIedge.getLayer1Protocol()){
      map.put(IEDGE_LAYER1PROTOCOL, newIedge.getLayer1ProtocolCommand());
    }

    if(oldIedge.isSipEnable() !=  newIedge.isSipEnable()){
      map.put(IEDGE_SIP, newIedge.isSipEnable()?ENABLE:DISABLE);
    }

    oldData = oldIedge.getSipUri();
    newData = newIedge.getSipUri();
    if(oldData  !=  null  &&  
        newData !=  null  &&
        !oldData.equals(newData)
        )
      map.put(IEDGE_URI,newData);

    oldData = oldIedge.getSipContact();
    newData = newIedge.getSipContact();
    if(oldData  !=  null  &&  
        newData !=  null  &&
        !oldData.equals(newData)
        )
      map.put(IEDGE_CONTACT,newData);

    if(oldIedge.getMaxCalls() !=  newIedge.getMaxCalls()){
      map.put(IEDGE_XCALLS, String.valueOf(newIedge.getMaxCalls()));
    }

    if(oldIedge.getMaxInCalls() !=  newIedge.getMaxInCalls()){
      map.put(IEDGE_XINCALLS, String.valueOf(newIedge.getMaxInCalls()));
    }

    if(oldIedge.getMaxOutCalls() !=  newIedge.getMaxOutCalls()){
      map.put(IEDGE_XOUTCALLS, String.valueOf(newIedge.getMaxOutCalls()));
    }

    if(oldIedge.isGateway() !=  newIedge.isGateway()){
      map.put(IEDGE_GATEWAY, newIedge.isGateway()?ENABLE:DISABLE);
    }

    if(oldIedge.isGRQEnable() !=  newIedge.isGRQEnable()){
      map.put(IEDGE_GRQ, newIedge.isGRQEnable()?ENABLE:DISABLE);
    }

    if(oldIedge.isRAIEnable() !=  newIedge.isRAIEnable()){
      map.put(IEDGE_RAI, newIedge.isRAIEnable()?ENABLE:DISABLE);
    }

    if(oldIedge.getPriority() !=  newIedge.getPriority()){
      map.put(IEDGE_PRIORITY, String.valueOf(newIedge.getPriority()));
    }

    if(oldIedge.getRasPort()  !=  newIedge.getRasPort()){
      map.put(IEDGE_RASPORT, String.valueOf(newIedge.getRasPort()));
    }

    if(oldIedge.getQ931Port() !=  newIedge.getQ931Port()){
      map.put(IEDGE_Q931PORT, String.valueOf(newIedge.getQ931Port()));
    }

    if(oldIedge.isTechPrefixEnable()  !=  newIedge.isTechPrefixEnable()){
      map.put(IEDGE_TPG, newIedge.isTechPrefixEnable()?ENABLE:DISABLE);
    }

    oldData = oldIedge.getTechPrefix();
    newData = newIedge.getTechPrefix();
    if(oldData  !=  null  &&  
        newData !=  null  &&
        !oldData.equals(newData)
        )
      map.put(IEDGE_TECHP,newData);

    oldData = oldIedge.getPeerGkId();
    newData = newIedge.getPeerGkId();
    if(oldData  !=  null  &&  
        newData !=  null  &&
        !oldData.equals(newData)
        )
      map.put(IEDGE_PGKID ,newData);

    if(oldIedge.getVendor() !=   newIedge.getVendor()){
      map.put(IEDGE_VENDOR, getVendorDescription(newIedge.getVendor()));
    }

    oldData = oldIedge.getSubnetIp();
    newData = newIedge.getSubnetIp();
    if(oldData  !=  null  &&  
        newData !=  null  &&
        !oldData.equals(newData)
        )
      map.put(IEDGE_SUBNETIP ,newData);

    oldData = oldIedge.getSubnetMask();
    newData = newIedge.getSubnetMask();
    if(oldData  !=  null  &&  
        newData !=  null  &&
        !oldData.equals(newData)
        )
      map.put(IEDGE_SUBNETMASK ,newData);

    oldData = oldIedge.getCallingPlanName();
    newData = newIedge.getCallingPlanName();
    if(oldData  !=  null  &&  
        newData !=  null  &&
        !oldData.equals(newData)
        )
      map.put(IEDGE_CP,newData);

    oldData = oldIedge.getH235Password();
    newData = newIedge.getH235Password();
    if(oldData  !=  null  &&  
        newData !=  null  &&
        !oldData.equals(newData)
        )
      map.put(IEDGE_PASSWD,newData);

    if(oldIedge.isMediaRoutingEnabled() !=  newIedge.isMediaRoutingEnabled()){
      map.put(IEDGE_MEDIAROUTING, newIedge.isMediaRoutingEnabled()?ENABLE:DISABLE);
    }

    if(oldIedge.isNeverMediaRouteEnabled() !=  newIedge.isNeverMediaRouteEnabled()){
      map.put(IEDGE_NOMEDIAROUTE, newIedge.isNeverMediaRouteEnabled()?ENABLE:DISABLE);
    }

    if(oldIedge.isHideAddressChangeEnabled()  !=  newIedge.isHideAddressChangeEnabled()){
      map.put(IEDGE_HIDEADDRESSCHANGE, newIedge.isHideAddressChangeEnabled()?ENABLE:DISABLE);
    }

    if(oldIedge.getMaxHunts() !=  newIedge.getMaxHunts()){
        map.put(IEDGE_MAXHUNTS,String.valueOf(newIedge.getMaxHunts()));
    }

    if(oldIedge.isQ931DisplayEnabled() !=  newIedge.isQ931DisplayEnabled()){
      map.put(IEDGE_H323DISPLAY, newIedge.isQ931DisplayEnabled()?ENABLE:DISABLE);
    }

    if (oldIedge.isMapAliasEnabled() != newIedge.isMapAliasEnabled())
      map.put(IEDGE_MAPALIAS, newIedge.isMapAliasEnabled()?ENABLE:DISABLE);

    if (oldIedge.isForceH245Enabled() != newIedge.isForceH245Enabled())
      map.put(IEDGE_FORCEH245, newIedge.isForceH245Enabled()?ENABLE:DISABLE);

    if(oldIedge.isConnectH245AddressEnabled() !=  newIedge.isConnectH245AddressEnabled()){
      map.put(IEDGE_CONNH245, newIedge.isConnectH245AddressEnabled()?ENABLE:DISABLE);
    }

    oldData  = IServerCommands.getInfoTransCapString(oldIedge.getInfoTransCap());
    newData  = IServerCommands.getInfoTransCapString(newIedge.getInfoTransCap());
    if(oldData  !=  null  &&  
        newData !=  null  &&
        !oldData.equals(newData)
        )
      map.put(IEDGE_INFOTRANSCAP,newData.toLowerCase());

    if (cap != null && cap.getSystemCapability().isTcs2833Enabled()) {
      if(oldIedge.getDeltcs2833Cap() !=  newIedge.getDeltcs2833Cap()) { 
        map.put(IEDGE_DELTCS2833, getDeltcs2833String(newIedge.getDeltcs2833Cap()));
      }
    }

    if (cap != null && cap.getSystemCapability().isTcsT38Enabled()) {
      if(oldIedge.getDeltcst38Cap() !=  newIedge.getDeltcst38Cap()) {
        map.put(IEDGE_DELTCST38, getDeltcst38String(newIedge.getDeltcst38Cap()));
      }
    }

    if (cap != null && cap.getDBCapability().isCAP2833Enabled()) {
      if(oldIedge.getCAP2833Cap() !=  newIedge.getCAP2833Cap()) {
        map.put(IEDGE_CAP2833, getCap2833String(newIedge.getCAP2833Cap()));
      }
    }

    if (cap != null && cap.getDBCapability().isRemoveTGEnabled()) { 
      if(oldIedge.isRemoveTGEnabled() !=  newIedge.isRemoveTGEnabled()) {
        map.put(IEDGE_NOTG, newIedge.isRemoveTGEnabled()?ENABLE:DISABLE);
      }
    }



    oldData  = oldIedge.getSrcIngressTg();
    newData  = newIedge.getSrcIngressTg();

    if(oldData  !=  null  &&  
        newData !=  null  &&
        !oldData.equals(newData)
        )
      map.put(IEDGE_SRCINGRESSTG,newData );

    if(oldIedge.isDestTg() !=  newIedge.isDestTg()){
      map.put(IEDGE_SETDESTTG, newIedge.isDestTg()?ENABLE:DISABLE);
    }



    if (cap != null && cap.getDBCapability().isRealmEnabled()) {
      oldData  = oldIedge.getRealmName();
      newData  = newIedge.getRealmName();
      if(oldData  !=  null  && newData !=  null  && !oldData.equals(newData))
        map.put(IEDGE_REALM,newData);
    }

    if (cap != null && cap.getDBCapability().isIGRPEnabled()) {
      oldData  = oldIedge.getIgrpName();
      newData  = newIedge.getIgrpName();
      if(oldData  !=  null  && newData !=  null  && !oldData.equals(newData))
        map.put(IEDGE_IGRP, newData);
    }

    if (cap != null && cap.getDBCapability().isDtgEnabled()) {
      oldData  = oldIedge.getDtg();
      newData  = newIedge.getDtg();
      if(oldData  !=  null  && newData !=  null  && !oldData.equals(newData))
        map.put(IEDGE_DTG, newData);

      oldData  = oldIedge.getNewSrcDtg();
      newData  = newIedge.getNewSrcDtg();
      if(oldData  !=  null  && newData !=  null  && !oldData.equals(newData))
        map.put(IEDGE_NEWSRCDTG, newData);
    }

    if (cap != null && cap.getDBCapability().isPIEnabled()) {
      if(oldIedge.isPiOnFastStartEnabled() !=  newIedge.isPiOnFastStartEnabled()){
        map.put(IEDGE_PIONFASTSTART, newIedge.isPiOnFastStartEnabled()?ENABLE:DISABLE);
      }
    }

    if (cap != null && cap.getSipCapability().isSipNatEnabled()) {

        if(  oldIedge.isAutoNatDetectionEnabled() !=  newIedge.isAutoNatDetectionEnabled())
            map.put(IEDGE_NATDETECT, newIedge.isAutoNatDetectionEnabled()?ENABLE:DISABLE);

        oldData  = oldIedge.getNatIp();
        newData  = newIedge.getNatIp();
        if(oldData  !=  null  && newData !=  null  && !oldData.equals(newData))
            map.put(IEDGE_NATIP, newIedge.getNatIp());
        if(oldIedge.getNatPort() != newIedge.getNatPort())
            map.put(IEDGE_NATPORT, String.valueOf(newIedge.getNatPort()));

    }

    if (cap != null && cap.getSipCapability().isSipPrivacyEnabled()) {
       oldData  = oldIedge.getSipPrivacy();
       newData  = newIedge.getSipPrivacy();
       if(oldData  !=  null  && newData !=  null  && !oldData.equals(newData))
         map.put(IEDGE_PRIVACY, newIedge.getSipPrivacy());
    }

    if (cap != null && cap.getDBCapability().isISDNMapccEnabled()) {
        if(oldIedge.isISDNMapccEnabled() !=  newIedge.isISDNMapccEnabled()){
            map.put(IEDGE_ISDNMAPCC, newIedge.isISDNMapccEnabled()?ENABLE:DISABLE);
        }
    }


    oldData = oldIedge.getZone();
    newData = newIedge.getZone();
    if(oldData  !=  null  &&  
        newData !=  null  &&
        !oldData.equals(newData)
        )
      command.addSubCommand(ZONE,newData);

    oldData = oldIedge.getEmail();
    newData = newIedge.getEmail();
    if(oldData  !=  null  &&  
        newData !=  null  &&
        !oldData.equals(newData)
        )
      command.addSubCommand(EMAIL,newData);



    if(map.size() > 0)
      command.addSubCommand(Commands.EDIT,map);

    if(command.size() > 0)
      return command;
    else
      return null;
  }


  public static Commands createNewTriggerCommand(Trigger tg, Capabilities cap){
    Commands command = new Commands(TRIGGER);
    command.setPrimaryKey(tg.getName());
    command.addSubCommand(Commands.ADD);

    LinkedHashMap map  = new LinkedHashMap();

    map.put(TRIGGER_SRC_VENDOR,getVendorDescription(tg.getSrcVendor()));
    map.put(TRIGGER_DST_VENDOR,getVendorDescription(tg.getDstVendor()));
    

    String data = tg.getData();
    if(data !=  null  &&  data.length() !=  0)
      map.put(TRIGGER_SDATA,data);
   
    if (cap != null && cap.getDBCapability().isOverrideEnabled())
      map.put(TRIGGER_OVERRIDE, (tg.isOverride())?ENABLE:DISABLE);

    if(map.size() > 0)
      command.addSubCommand(Commands.EDIT,map);

    return command;
  }

  public static Commands createTriggerDeleteCommand(Trigger tg){
      Commands command  = new Commands(TRIGGER);
      command.setPrimaryKey(tg.getName());
      command.addSubCommand(Commands.DELETE);
      return command;
  }

  public static Commands createTriggerExtraCommand(Trigger tg){
      Commands command  = new Commands(TRIGGER);
      command.setPrimaryKey(tg.getName());
      command.addSubCommand(Commands.PURGE);
      return command;
  }

  public static Commands createTriggerEditCommand(Trigger oldTg, Trigger newTg, Capabilities cap){
    Commands command = new Commands(TRIGGER);
    command.setPrimaryKey(newTg.getName());

    LinkedHashMap map  = new LinkedHashMap();
    if(oldTg.getSrcVendor() !=  newTg.getSrcVendor())
      map.put(TRIGGER_SRC_VENDOR,getVendorDescription(newTg.getSrcVendor()));

    if(oldTg.getDstVendor() !=  newTg.getDstVendor())
      map.put(TRIGGER_DST_VENDOR,getVendorDescription(newTg.getDstVendor()));


    if( oldTg.getData()  !=  null  &&
        newTg.getData() !=  null  &&
      !oldTg.getData().equals(newTg.getData())
      ){
        map.put(TRIGGER_SDATA,newTg.getData());
    }

    if(newTg.isOverride() != oldTg.isOverride()) {
      map.put(TRIGGER_OVERRIDE, (newTg.isOverride())?ENABLE:DISABLE);
    }
   
    if(map.size() > 0)
      command.addSubCommand(Commands.EDIT,map);

    if(command.size() > 0)
      return command;
    else
      return null;

  }


  public static Commands createNewRealmCommand(Realm realm){
    Commands command = new Commands(REALM);
    command.setPrimaryKey(realm.getName());
    command.addSubCommand(Commands.ADD);

    LinkedHashMap map  = new LinkedHashMap();

    //if(cap.getDBCapability().isVnetEnabled())
	map.put(REALM_VNETNAME,realm.getVnetName());
	//else
	//map.put(REALM_IFNAME,realm.getIfName());
    map.put(REALM_RSA,realm.getRsa());
    map.put(REALM_MASK,realm.getMask());

    if(realm.getAddrType() ==  ADDRESS_TYPE_PUBLIC)
      map.put(REALM_ADDR,PUBLIC);
    else if(realm.getAddrType() ==  ADDRESS_TYPE_PRIVATE)
      map.put(REALM_ADDR,PRIVATE);

    map.put(REALM_SIGPOOL,String.valueOf(realm.getSigPoolId()));
    map.put(REALM_MEDPOOL,String.valueOf(realm.getMedPoolId()));

    map.put(REALM_IMR,mediaRouting[realm.getImr()].toLowerCase());
    map.put(REALM_EMR,mediaRouting[realm.getEmr()].toLowerCase());

    map.put(REALM_ADMIN,(realm.isAdminEnabled())?ENABLE:DISABLE);


    if(map.size() > 0)
      command.addSubCommand(Commands.EDIT,map);

    return command;
  }

  public static Commands createRealmDeleteCommand(Realm realm){
      Commands command  = new Commands(REALM);
      command.setPrimaryKey(realm.getName());
      command.addSubCommand(Commands.DELETE);
      return command;
  }


  public static Commands createRealmEditCommand(Realm oldRealm, Realm newRealm){
    Commands command = new Commands(REALM);
    command.setPrimaryKey(newRealm.getName());

    LinkedHashMap map  = new LinkedHashMap();

//     if(cap.getDBCapability().isVnetEnabled()){
	if( oldRealm.getVnetName()  !=  null  &&
	    newRealm.getVnetName() !=  null  &&
	    !oldRealm.getVnetName().equals(newRealm.getVnetName())
	    ){
	    map.put(REALM_VNETNAME,newRealm.getVnetName());
	}
//     }else{
// 	if( oldRealm.getIfName()  !=  null  &&
// 	    newRealm.getIfName() !=  null  &&
// 	    !oldRealm.getIfName().equals(newRealm.getIfName())
// 	    ){
// 	    map.put(REALM_IFNAME,newRealm.getIfName());
// 	}
//     }

    if( oldRealm.getRsa()  !=  null  &&
        newRealm.getRsa() !=  null  &&
      !oldRealm.getRsa().equals(newRealm.getRsa())
      )
      map.put(REALM_RSA,String.valueOf(newRealm.getRsa()));

    if( oldRealm.getMask()  !=  null  &&
        newRealm.getMask() !=  null  &&
      !oldRealm.getMask().equals(newRealm.getMask())
      )
      map.put(REALM_MASK,String.valueOf(newRealm.getMask()));

    if(oldRealm.getAddrType()  !=  newRealm.getAddrType()){
      if(newRealm.getAddrType() ==  ADDRESS_TYPE_PUBLIC)
        map.put(REALM_ADDR,PUBLIC);
      else if(newRealm.getAddrType() ==  ADDRESS_TYPE_PRIVATE)
        map.put(REALM_ADDR,PRIVATE);
    }


    if(oldRealm.getSigPoolId()  !=  newRealm.getSigPoolId())
      map.put(REALM_SIGPOOL,String.valueOf(newRealm.getSigPoolId()));

    if(oldRealm.getMedPoolId()  !=  newRealm.getMedPoolId())
      map.put(REALM_MEDPOOL,String.valueOf(newRealm.getMedPoolId()));

    if(oldRealm.getImr()  !=  newRealm.getImr())
      map.put(REALM_IMR,mediaRouting[newRealm.getImr()].toLowerCase());
  
    if(oldRealm.getEmr()  !=  newRealm.getEmr())
      map.put(REALM_EMR,mediaRouting[newRealm.getEmr()].toLowerCase());

    if(oldRealm.isAdminEnabled() !=  newRealm.isAdminEnabled()){
      map.put(REALM_ADMIN,(newRealm.isAdminEnabled())?ENABLE:DISABLE);
    }

    if(map.size() > 0)
      command.addSubCommand(Commands.EDIT,map);

    if(command.size() > 0)
      return command;
    else
      return null;

  }


  public static Commands createRealmExtraCommand(Realm realm){
      Commands command  = new Commands(REALM);
      command.setPrimaryKey(realm.getName());
      if(realm.isOpEnabled() )
        command.addSubCommand(UP);
      else
        command.addSubCommand(DOWN);

      return command;
  }


  public static Commands createNewVnetCommand(Vnet vnet, Capabilities cap){
    Commands command = new Commands(VNET);
    command.setPrimaryKey(vnet.getName());
    command.addSubCommand(Commands.ADD);

    LinkedHashMap map  = new LinkedHashMap();

    map.put(VNET_IFNAME,vnet.getIfName());
    if(cap.getDBCapability().isLinux()){
	map.put(VNET_ID,String.valueOf(vnet.getId()));
	map.put(VNET_GATEWAY,vnet.getGateway());
    }

    if(map.size() > 0)
      command.addSubCommand(Commands.EDIT,map);

    return command;
  }

  public static Commands createVnetDeleteCommand(Vnet vnet){
      Commands command  = new Commands(VNET);
      command.setPrimaryKey(vnet.getName());
      command.addSubCommand(Commands.DELETE);
      return command;
  }


  public static Commands createVnetEditCommand(Vnet oldVnet, Vnet newVnet, Capabilities cap){
    Commands command = new Commands(VNET);
    command.setPrimaryKey(newVnet.getName());

    LinkedHashMap map  = new LinkedHashMap();

    if( oldVnet.getIfName()  !=  null  &&
        newVnet.getIfName() !=  null  &&
      !oldVnet.getIfName().equals(newVnet.getIfName())
      ){
        map.put(VNET_IFNAME,newVnet.getIfName());
    }

    if(cap.getDBCapability().isLinux()){
	if( oldVnet.getId()  != newVnet.getId())
	    map.put(VNET_ID,String.valueOf(newVnet.getId()));

	if( oldVnet.getGateway()  !=  null  &&
	    newVnet.getGateway() !=  null  &&
	    !oldVnet.getGateway().equals(newVnet.getGateway())
	    )
	    map.put(VNET_GATEWAY,String.valueOf(newVnet.getGateway()));
    }

    if(map.size() > 0)
      command.addSubCommand(Commands.EDIT,map);

    if(command.size() > 0)
      return command;
    else
      return null;

  }

    
  public static Commands createNewIEdgeGroupCommand(IEdgeGroup igrp){
    Commands command = new Commands(IGRP);
    command.setPrimaryKey(igrp.getName());
    command.addSubCommand(Commands.ADD);

    LinkedHashMap map  = new LinkedHashMap();

    map.put(IGRP_MAXIN,    String.valueOf(igrp.getMaxCallsIn()));
    map.put(IGRP_MAXOUT,   String.valueOf(igrp.getMaxCallsOut()));
    map.put(IGRP_MAXTOTAL, String.valueOf(igrp.getMaxCallsTotal()));

    if(map.size() > 0)
      command.addSubCommand(Commands.EDIT, map);

    return command;
  }

  public static Commands createIEdgeGroupDeleteCommand(IEdgeGroup igrp){
      Commands command  = new Commands(IGRP);
      command.setPrimaryKey(igrp.getName());
      command.addSubCommand(Commands.DELETE);
      return command;
  }

  public static Commands createIEdgeGroupEditCommand(IEdgeGroup oldIgrp, IEdgeGroup newIgrp) {
    Commands command = new Commands(IGRP);
    command.setPrimaryKey(newIgrp.getName());

    LinkedHashMap map  = new LinkedHashMap();

    if (oldIgrp.getMaxCallsIn() != newIgrp.getMaxCallsIn()) {
      map.put(IGRP_MAXIN, String.valueOf(newIgrp.getMaxCallsIn()));
    }

    if (oldIgrp.getMaxCallsOut() != newIgrp.getMaxCallsOut()) {
      map.put(IGRP_MAXOUT, String.valueOf(newIgrp.getMaxCallsOut()));
    }

    if (oldIgrp.getMaxCallsTotal() != newIgrp.getMaxCallsTotal()) {
      map.put(IGRP_MAXTOTAL, String.valueOf(newIgrp.getMaxCallsTotal()));
    }

    if(map.size() > 0)
      command.addSubCommand(Commands.EDIT, map);

    if(command.size() > 0)
      return command;
    else
      return null;
  }
}
