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

public class VpnList implements Serializable, Comparable ,Cloneable {
    String  vpngrp;
	  String vpnid;
	  int extlen;
	  String vpnnam;
	  String vpnprefix;
	  String vpnloc;
	  String vpncon;

	  static final long serialVersionUID = 33369047652010001L;

	  public VpnList () {
		 this("");
	  }

	  public VpnList (String s) {
		 this(s, -1);
	  }

	  public VpnList (String s, int len) {
		 vpnid	= "";
		 vpnnam = s;
		 extlen = len;
		 vpngrp	= "";
		 vpnprefix	=	"";
		 vpnloc		=	"";
		 vpncon		=	"";
	  }

    public VpnList(String id, String name, int len, String group, String prefix, String loc, String con){
      vpnid = id;
      vpnnam  = name;
      extlen  = len;
      vpngrp =  group;
      vpnprefix = prefix;
      vpnloc  = loc;
      vpncon  = con;
    }



	  public String getVpnGroup () {
		 return vpngrp;
	  }

	  public String getVpnId () {
		 return vpnid;
	  }

	  public void setVpnId (String s) {
		  vpnid	=	(s	==	null)?"":s;
	  }

	  public int getExtLen () {
		 return extlen;
	  }

	  public void setExtLen (int ext) {
		 extlen	=	ext;
	  }

	  public String getVpnName () {
		 return vpnnam;
	  }

	  public String getVpnPrefix() {
		  return vpnprefix;
	  }


	  public String getVpnLocation () {
		 return vpnloc;
	  }

	  public void setVpnName (String s) {
		  vpnnam	=	(s	==	null)?"":s;
	  }


	  public void setVpnGroup (String s) {
		  vpngrp  = s;
	  }


	  public void setVpnPrefix (String s) {
		vpnprefix	=	(s	==	null)?"":s;
	  }


	  public void setVpnLocation (String s) {
		 vpnloc = (s	==	null)?"":s;
	  }

	  public String getVpnContact () {
		 return vpncon;
	  }

	  public void setVpnContact (String s) {
		 vpncon = (s	==	null)?"":s;
	  }

	  public void dump () {
		 System.out.println("VPN Id: " + vpnid);
		 System.out.println("Ext Len: " + extlen);
		 System.out.println("VPN Group: " + vpngrp);
		 System.out.println("VPN Name: " + vpnnam);
		 System.out.println("VPN Contact: " + vpncon);
		 System.out.println("VPN Location: " + vpnloc);
		 System.out.println("");
	  }

	  private String getVpnIdString () {
		 StringBuffer sb = new StringBuffer();
		 if (extlen > 0) {
			sb.append(vpnid);
			sb.append("/");
			sb.append(String.valueOf(extlen));
		 } else
			sb.append(vpnid);

		 return sb.toString();
	  }

	  public String toString () {
		 StringBuffer sb = new StringBuffer();
		 if (vpnnam != null && !vpnnam.equals("")) {
			sb.append(vpnnam);
			String s = getVpnIdString();
			if (!s.equals("")) {
			   sb.append(" (");
			   sb.append(s);
			   sb.append(")");
			}
		 } else {
			sb.append(getVpnIdString());
		 }

		 return sb.toString();
	  }

	  public boolean equals (Object o) {
		 if (o instanceof VpnList &&
			 ((VpnList)o).getVpnId().equals(getVpnId()) &&
			 ((VpnList)o).getExtLen() == getExtLen() &&
			 ((VpnList)o).getVpnGroup().equals(getVpnGroup()) &&
			 ((VpnList)o).getVpnName().equals(getVpnName()) &&
			 ((VpnList)o).getVpnLocation().equals(getVpnLocation()) &&
			 ((VpnList)o).getVpnContact().equals(getVpnContact()))
			return true;

		 return false;
	  }

	  public int compareTo (Object o) {
		 Object c = o;
		 if (o instanceof DefaultMutableTreeNode) {
			c = ((DefaultMutableTreeNode)o).getUserObject();
		 }
		 if (c instanceof VpnList)
			return vpnnam.compareTo(((VpnList)c).getVpnName());

		 return vpnnam.compareTo(o.toString());
	  }

	  public HashMap getParams () {

		HashMap hm = new HashMap();
		if((vpnprefix	!=	null) && (vpnprefix.length()	!=	0))
			hm.put("prefix", vpnprefix); // store vpn prefix 

		if((vpncon	!=	null) && (vpncon.length()	!=	0))
			hm.put("contact", vpncon); // store vpn prefix 

		if((vpnloc	!=	null) && (vpnloc.length()	!=	0))
			hm.put("location", vpnloc); // store vpn prefix 

		return hm;
	  }


	  public synchronized void update (VpnList vl) {

		setVpnId(vl.getVpnId());
		setExtLen(vl.getExtLen());
		setVpnGroup(vl.getVpnGroup());
		setVpnName(vl.getVpnName());
		setVpnPrefix(vl.getVpnPrefix());
		setVpnLocation(vl.getVpnLocation());
		setVpnContact(vl.getVpnContact());
	  }

    public  void  resetAll(){
      vpngrp  = "";
      vpnid = "";  
	    extlen  = 0;
	    vpnnam  = "";
	    vpnprefix = "";
	    vpnloc  = "";
	    vpncon  = "";
    }

    public Object getPrimaryKey(){
      return vpnnam;
    }

    public Object getSecondaryKey(){
      return null;
    }
    public String getPrimaryField(){
      return "vpnnam";
    }
    public String getSecondaryField(){
      return "null";
    }
    public String getFieldValue(String fieldName){
      if(fieldName.equals("vpngrp"))
        return vpngrp;
      return "";

    }

    public Object clone () {
    try{
      return super.clone();
    }catch(Exception e){
       VpnList vpn = new VpnList(
        getVpnId(),
        getVpnName(),
        getExtLen(),
        getVpnGroup(),
        getVpnPrefix(),
        getVpnLocation(),
        getVpnContact()
        );
      return vpn;
    }
  }
}

