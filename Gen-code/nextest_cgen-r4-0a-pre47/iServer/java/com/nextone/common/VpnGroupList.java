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

public class VpnGroupList implements Serializable, Comparable {
	  String vpngrp;

	  static final long serialVersionUID = 33369047652010002L;

	  public VpnGroupList () {
		 this("");
	  }

	  public VpnGroupList (String s) {
		 setVpnGroup(s);
	  }

	  public void dump () {
		 System.out.println("VPN Group: " + vpngrp);
	  }

	  public String getVpnGroup () {
		 return vpngrp;
	  }
	  public void setVpnGroup (String s) {
      vpngrp = (s  ==  null)?"":s;
	  }


	  public String toString () {
		 return vpngrp;
	  }

	  public boolean equals (Object o) {
		 if (o instanceof VpnGroupList &&
			 ((VpnGroupList)o).getVpnGroup().equals(getVpnGroup())) {
			return true;
		 }

		 return false;
	  }

	  public int compareTo (Object o) {
		 Object c = o;
		 if (o instanceof DefaultMutableTreeNode) {
			c = ((DefaultMutableTreeNode)o).getUserObject();
		 }
		 if (c instanceof VpnGroupList)
			return vpngrp.compareTo(((VpnGroupList)c).getVpnGroup());

		 return vpngrp.compareTo(o.toString());
	  }

    public void resetAll(){
      vpngrp  = "";
    }

    public Object getPrimaryKey(){
      return vpngrp;
    }

    public Object getSecondaryKey(){
      return null;
    }

    public String getPrimaryField(){
      return "vpngrp";
    }
    public String getSecondaryField(){
      return "";
    }
    public String getFieldValue(String fieldName){
      return "";
    }
}

