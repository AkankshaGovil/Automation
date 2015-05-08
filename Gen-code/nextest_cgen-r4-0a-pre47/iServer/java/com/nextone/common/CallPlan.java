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

public class CallPlan implements Serializable, Comparable, Cloneable, DropStatusSource {
	  String cpname;
	  String vpng;
	  long refreshTime;
	  transient boolean ad;

	  static final long serialVersionUID = 5803789377086540602L;
	  static final SimpleDateFormat df = new SimpleDateFormat("EEE MMM dd hh:mm:ssa yyyy z");

	  public CallPlan (String c, String v, long t) {
		 cpname = c;
		 vpng = v;
		 refreshTime = t;
	  }

  public synchronized void reset (String cp, String vg, long rt) {
    cpname = cp;
    vpng = vg;
    refreshTime = rt;
  }

	  public void dump () {
		 System.out.println("Call Plan Name: " + cpname);
		 System.out.println("VPN Group: " + vpng);
		 System.out.println("Refresh Time: " + df.format(new Date(refreshTime*1000)));
	  }

	  public synchronized String getCallPlanName () {
		 return cpname;
	  }

	  public synchronized void setCallPlanName (String s) {
		 cpname = s;
	  }

	  public synchronized String getVpnGroup () {
		 return vpng;
	  }

	  public synchronized void setVpnGroup (String s) {
		 vpng = s;
	  }

	  public synchronized long getRefreshTimeinLong() {
      return refreshTime;
	  }

    public synchronized void setRefreshTime(long time){
      refreshTime = time;
    }

	  public Date getRefreshTime () {
		 return new Date(refreshTime*1000);
//		 return df.format(new Date(refreshTime*1000));
	  }

	  public synchronized boolean getAcceptingDrop () {
		 return ad;
	  }

	  public synchronized void setAcceptingDrop (boolean b) {
		 ad = b;
	  }

	  public String toString () {
		 StringBuffer s = new StringBuffer();
		 s.append(cpname);

		 if (!vpng.equals(""))
			s.append(" (" + vpng + ")");

		 return s.toString();
	  }

	  public int compareTo (Object o) {
		 if (o instanceof CallPlan)
			return getCallPlanName().compareTo(((CallPlan)o).getCallPlanName());
		 return toString().compareTo(o.toString());
	  }

	  public Object clone () throws CloneNotSupportedException {
		 return super.clone();
	  }

    public void resetAll(){
		 cpname = "";
		 vpng   = "";
		 refreshTime = 0;
    }

    public Object getPrimaryKey(){
      return cpname;
    }

    public Object getSecondaryKey(){
      return null;
    }

    public String getPrimaryField(){
      return "cpname";
    }
    public String getSecondaryField(){
      return "";
    }

    public String getFieldValue(String fieldName){
      return "";
    }

    public boolean equals (Object o) {
		 if (o instanceof CallPlan)
			return ((CallPlan)o).getCallPlanName().equals(getCallPlanName());
     if(o !=  null)
		  return toString().equals(o.toString());
     return false;
    }


}
