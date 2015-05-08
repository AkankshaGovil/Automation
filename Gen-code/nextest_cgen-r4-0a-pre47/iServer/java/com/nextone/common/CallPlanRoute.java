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
import java.sql.Timestamp;

import com.nextone.util.SysUtil;

public class CallPlanRoute implements Serializable, Comparable, Cloneable ,DBObject, CommonConstants{
  /** @deprecated in favor of the new calling plan scheme */
  String cpname = "";

  String dest;
  String prefix;    // destination prefix
  String srcPrefix; // source prefix
  long refreshTime;

  boolean srcPresent = false;
  String src;
  int srcLen;
  int destLen;
  String crname;
  int routeFlags;  // source or destination route
  Date  startTime; // start time applicable to the route. Received from CallingplanBind object
  Date  endTime;   // end time applicable to the route. Received from CallingplanBind object

  public static final String ROUTE_TYPE_SOURCE      = "source";
  public static final String ROUTE_TYPE_DESTINATION = "dest";
  public static final String ROUTE_TYPE_TRANSIT     = "transit";
  public static final String ROUTE_TYPE_UNKNOWN     = "unknown";
  
  public static final int CRF_CALLORIGIN  = 0x0010;
  public static final int CRF_CALLDEST    = 0x0020;
  public static final int CRF_DNISDEFAULT = 0x1000;
  public static final int CRF_TRANSIT		  = 0x4000;
  public static final int CRF_TEMPLATE	  = 0x10000;

  public static final String ANY = "[any]";

  static final long serialVersionUID = -3074397631807357287L;

  static final SimpleDateFormat df = new SimpleDateFormat("EEE MMM dd hh:mm:ssa yyyy z");

  public CallPlanRoute(){
    this("");
  }

  public CallPlanRoute(String crname){
    this(crname,"",0,"",0,"","",0,0);
  
  }

  public CallPlanRoute (String cr, String s, int sl, String d, int dl, String dp, String sp, int f, long t) {
    resetAll();
    crname = (cr == null)?"":cr;
    src = (s == null)?"":s;
    srcLen = sl;
    dest = (d == null)?"":d;
    destLen = dl;
    prefix = (dp == null)?"":dp;
    srcPrefix = (sp == null)?"":sp;
    routeFlags = f;
    refreshTime = t;
    srcPresent = true;
  }

  public CallPlanRoute (String cr, String s, int sl, String d, int dl, String dp, String sp, String fType, long t) {
    resetAll();
    crname = (cr == null)?"":cr;
    src = (s == null)?"":s;
    srcLen = sl;
    dest = (d == null)?"":d;
    destLen = dl;
    prefix = (dp == null)?"":dp;
    srcPrefix = (sp == null)?"":sp;
    setRouteType(fType);
    srcPresent = true;
  }



  public void reset (String dst, String pre, long rt, boolean sp, String src, String srcPre, int sl, int dl, String cr, int rf) {
    dest = (dst == null)?"":dst;
    prefix = (pre == null)?"":pre;
    refreshTime = rt;
    srcPresent = sp;
    this.src = (src == null)?"":src;
    srcPrefix = (srcPre == null)?"":srcPre;
    srcLen = sl;
    destLen = dl;
    crname = (cr == null)?"":cr;
    routeFlags = rf;
  }

  public void dump () {
    System.out.println("Call Plan Route Name: " + crname);
    System.out.println("Source: " + src + " Len: " + srcLen);
    System.out.println("Source Prefix: " + srcPrefix);
    System.out.println("Destination: " + dest + " Len: " + destLen);
    System.out.println("Destination Prefix: " + prefix);
    System.out.println("Route Flags: " + routeFlags);
    System.out.println("Refresh Time: " + df.format(new Date(refreshTime*1000)));
  }

  /** these calls should not be used when dealing with post 1.2 iserver */
  public synchronized String getCallPlanName () {
    return cpname;
  }

  /** these calls should not be used when dealing with post 1.2 iserver */
  public synchronized void setCallPlanName (String s) {
    cpname = s;
  }

  public synchronized String getCallPlanRouteName () {
    return crname;
  }

  public synchronized void setCallPlanRouteName (String s) {
    crname = s;
  }

  public synchronized String getDestination () {
    return dest;
  }

  public synchronized String getDestinationString () {
    if (dest.equals("") || dest.equals("-"))
      return new String(ANY);
    else if (dest.startsWith("-"))
      return new String(dest.substring(1));

    return new String(dest);
  }

  public String getRejectionString () {
    if (dest.startsWith("-") ||
	(srcPresent && src.startsWith("-")))
      return "[REJECT]";

    return "";
  }

  public synchronized void setDestination (String s) {
    dest = s;
  }

  public synchronized boolean isSourcePresent () {
    return srcPresent;
  }

  public synchronized void setSourcePresent (boolean flag) {
    srcPresent  = flag;
  }


  public synchronized int getDestinationLength () {
    return destLen;
  }

  public synchronized void setDestinationLength (int dl) {
    destLen = dl;
  }

  public synchronized String getSource () {
    return src;
  }

  public synchronized String getSourceString () {
    if (src.equals("") || src.equals("-"))
      return new String(ANY);
    else if (src.startsWith("-"))
      return new String(src.substring(1));

    return new String(src);
  }

  public synchronized void setSource (String s) {
    src = s;
  }

  public synchronized int getSourceLength () {
    return srcLen;
  }

  public synchronized void setSourceLength (int sl) {
    srcLen = sl;
  }

  public synchronized String getPrefix () {
    return prefix;
  }

  public synchronized void setPrefix (String s) {
    prefix = s;
  }

  public synchronized int getRouteFlags () {
    return routeFlags;
  }

  public static int getFlagForType (String type) {
    if (type != null) {
      if (type.equals(ROUTE_TYPE_SOURCE))
	      return CRF_CALLORIGIN;
      else if (type.equals(ROUTE_TYPE_DESTINATION))
	      return CRF_CALLDEST;
      else if(type.equals(ROUTE_TYPE_TRANSIT))
        return CRF_TRANSIT;
    }

    return 0;
  }

  public synchronized void setRouteType (String type) {
    if (type != null) {
      if (type.equals(ROUTE_TYPE_SOURCE)) {
	      routeFlags &= ~CRF_CALLDEST;
        routeFlags &= ~CRF_TRANSIT;
	      routeFlags |= CRF_CALLORIGIN;
      } else if (type.equals(ROUTE_TYPE_DESTINATION)) {
	      routeFlags &= ~CRF_CALLORIGIN;
        routeFlags &= ~CRF_TRANSIT;
	      routeFlags |= CRF_CALLDEST;
      } else if(type.equals(ROUTE_TYPE_TRANSIT)){
	      routeFlags &= ~CRF_CALLORIGIN;
        routeFlags &= ~CRF_CALLDEST;
	      routeFlags |= CRF_TRANSIT;
      }
      else {
	      routeFlags &= ~CRF_CALLORIGIN;
	      routeFlags &= ~CRF_CALLDEST;
        routeFlags &= ~CRF_TRANSIT;
      } 
    } else {
	      routeFlags &= ~CRF_CALLORIGIN;
	      routeFlags &= ~CRF_CALLDEST;
        routeFlags &= ~CRF_TRANSIT;
    } 
  }


  public synchronized  void setDNISDefault(boolean isDefault){
    if( isDefault)
      routeFlags |= CRF_DNISDEFAULT;
    else
      routeFlags &= ~CRF_DNISDEFAULT;
  }

  public synchronized String getRouteType () {
    if (isRouteTypeSource())
      return ROUTE_TYPE_SOURCE;
    else if (isRouteTypeDestination())
      return ROUTE_TYPE_DESTINATION;
    else if (isRouteTypeTransit())
      return ROUTE_TYPE_TRANSIT;

    return ROUTE_TYPE_UNKNOWN;
  }

  public synchronized  void setTemplateRoute(boolean isTemplate){
    if( isTemplate)
      routeFlags |= CRF_TEMPLATE;
    else
      routeFlags &= ~CRF_TEMPLATE;
  }

  public synchronized void setStickyRoute(boolean isSticky) {
    if(isSticky)
      routeFlags |= CRF_STICKY;
    else
      routeFlags &= ~CRF_STICKY;
  }

  public boolean isStickyRoute() {
    return ((routeFlags & CRF_STICKY) == CRF_STICKY);
  }

  public synchronized void setRouteFlags (int flag) {
    routeFlags = flag;
  }

  public synchronized void addRouteFlags (int flag) {
    routeFlags |= flag;
  }

  public synchronized void deleteRouteFlags (int flag) {
    routeFlags &= ~flag;
  }

  public synchronized String getType(){
    if( (routeFlags & CRF_REJECT)  ==  CRF_REJECT)
      return TYPE_REJECT;
    return TYPE_NORMAL;
  }

  public boolean isRouteTypeSource () {
    return ((routeFlags & CRF_CALLORIGIN) == CRF_CALLORIGIN);
  }

  public boolean isRouteTypeDestination () {
    return ((routeFlags & CRF_CALLDEST) == CRF_CALLDEST);
  }

  public boolean isRouteTypeTransit () {
    return ((routeFlags & CRF_TRANSIT) == CRF_TRANSIT);
  }

  public boolean isTemplateRoute(){
    return ((routeFlags & CRF_TEMPLATE) == CRF_TEMPLATE);
  }



  public boolean isDNISDefault(){
    return ((routeFlags & CRF_DNISDEFAULT) == CRF_DNISDEFAULT);
  }
  public long getRefreshTime () {
    return refreshTime;
  }
  public void setRefreshTime(long time){
    refreshTime = time;
  }

  public Date getStartTime() {
    return startTime;
  }

  public void setStartTime(Date sTime) {
    startTime =   sTime;
  }

  public Date getEndTime() {
    return endTime;
  }

  public void setEndTime(Date eTime) {
    endTime =   eTime;
  }

  public void setSrcPrefix(String prefix){
    srcPrefix = (prefix == null)?"":prefix;
  }

  public String getSrcPrefix(){
    return (srcPrefix == null)?"":srcPrefix;
  }

  public String getRouteDetails () {
    StringBuffer sb = new StringBuffer();
    if (srcPresent) {
      sb.append("Calling #: ");
      sb.append(SysUtil.getSizedString(getSourceString(), 15));
      if (srcLen != 0) {
        sb.append("/");
        sb.append(SysUtil.getSizedString(String.valueOf(srcLen), 3));
      } else
        sb.append("    "); // 4 characters
      sb.append("    Calling # Prefix: ");
      if (getSrcPrefix().equals(""))
	sb.append(SysUtil.getSizedString("<none>", 15));
      else
	sb.append(SysUtil.getSizedString(srcPrefix, 15));
      sb.append("  ");
    }

    sb.append("Called #: ");
    sb.append(SysUtil.getSizedString(getDestinationString(), 15));
    if (srcPresent && destLen != 0) {
      sb.append("/");
      sb.append(SysUtil.getSizedString(String.valueOf(destLen), 3));
    } else
      sb.append("    "); // 4 characters
    sb.append("    Called # Prefix: ");
    if (prefix.equals(""))
      sb.append(SysUtil.getSizedString("<none>", 15));
    else
      sb.append(SysUtil.getSizedString(prefix, 15));

    sb.append("    Applied at: ");
    if (isRouteTypeSource())
      sb.append("Ingress");
    else if (isRouteTypeDestination())
      sb.append("Egress ");
    else
      sb.append("not set");

    sb.append(" ");
    sb.append(getRejectionString());

    return sb.toString();
  }

  public String toString () {
    StringBuffer sb = new StringBuffer();
    if (srcPresent) {
      sb.append(SysUtil.getSizedString(crname, 15));
      sb.append(": ");
    }
    sb.append(getRouteDetails());

    return sb.toString();
  }

  public int compareTo (Object o) {
    return toString().compareTo(o.toString());
  }

  public Object clone () {
    try{
      return super.clone();
    }catch(Exception e){
      CallPlanRoute cpr = new CallPlanRoute();
      cpr.reset(getDestination(),
        getPrefix(),
        getRefreshTime(),
        isSourcePresent(),
        getSource(),
        getSrcPrefix(), 
        getSourceLength(),
        getDestinationLength(),
        getCallPlanRouteName(),
        getRouteFlags()
        );
      return cpr;
    }
  }

  // equals does not check for the route name to be equals, what we really check here to see
  // if the given route is "equal" to the current route in terms of the route properties
  public boolean equals (Object o) {
    if (o instanceof CallPlanRoute) {
      CallPlanRoute cpr = (CallPlanRoute)o;

      if (srcPresent) {
	if (isSourcePresent() == cpr.isSourcePresent() &&
	    getDestinationString().equals(cpr.getDestinationString()) &&
	    getPrefix().equals(cpr.getPrefix()) &&
	    getSrcPrefix().equals(cpr.getSrcPrefix()) &&
	    getSourceString().equals(cpr.getSourceString()) &&
	    getRejectionString().equals(cpr.getRejectionString()) &&
	    getDestinationLength() == cpr.getDestinationLength() &&
	    getSourceLength() == cpr.getSourceLength() &&
	    getRouteFlags() == cpr.getRouteFlags())
	  return true;
      } else {
	if (isSourcePresent() == cpr.isSourcePresent() &&
	    getDestination().equals(cpr.getDestination()) &&
	    getPrefix().equals(cpr.getPrefix()) &&
	    getRejectionString().equals(cpr.getRejectionString()))
	  return true;
      }
    }

    return false;
  }
		

  public void resetAll(){
    crname  = "";
    src     = "";
    srcLen  = 0;
    dest    = "";
    destLen = 0;
    prefix  = "";
    routeFlags  = 0;
    refreshTime = 0;
    srcPresent  = true;
    srcPrefix = "";
  }

/*
  public Object getPrimaryKey(){
    return crname;
  }
 
  public Object getSecondaryKey(){
    return null;
  }

  public String getPrimaryField(){
    return "crname";
  }
  public String getSecondaryField(){
    return "";
  }

  public String getFieldValue(String fieldName){
    if(fieldName.equals("src"))
      return getSource();
    if(fieldName.equals("dest"))
      return getDestination();
    return "";
  }
*/
  public static Object getObject(Object[] row){
    CallPlanRoute cpr  = new CallPlanRoute((String)row[INDEX_CPR_NAME],
      (String)row[INDEX_CPR_SRC],
      ((Integer)row[INDEX_CPR_SRCLEN]).intValue(),
      (String)row[INDEX_CPR_DEST],
      ((Integer)row[INDEX_CPR_DESTLEN]).intValue(),
      (String)row[INDEX_CPR_PREFIX],
      (String)row[INDEX_CPR_SRCPREFIX],
      ((Integer)row[INDEX_CPR_ROUTEFLAGS]).intValue(),
      ((Long)row[INDEX_CPR_REFRESHTIME]).longValue()
      );
      cpr.setSourcePresent(((Boolean)row[INDEX_CPR_SRCPRESENT]).booleanValue());
     return cpr;
  } 
  
  //  implements DBObjet interface
  // order should be same as in the IViewDB calling plan binding table fields
  public Object[] getObjectArray(){
    Object[]  data= new Object[10];
    data[0] = crname;
    data[1] = dest;
    data[2] = prefix;
    data[3] = srcPrefix;
    data[4] = new Long(refreshTime);
    data[5] = new Boolean(srcPresent);
    data[6] = src;
    data[7] = new Integer(srcLen);
    data[8] = new Integer(destLen);
    data[9] = new Integer(routeFlags);
    return data;
  }
    public String[] getKeys(){
      String[] ob = new String[1];
      ob[0] = "crname";
      return ob;
    }

    public Object[] getValues(){
      Object[] ob = new Object[1];
      ob[0] = crname;
      return ob;
    }
}
