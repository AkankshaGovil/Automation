


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
import com.nextone.util.TM;

public class CallPlanBinding implements Serializable, Comparable, Cloneable,DBObject, CommonConstants {
  String cpname;
  String crname;
  long refreshTime;
  TM tmStart, tmEnd;
  int priority;

//  String routeType;
//  String  type;

  int flag;
  private boolean dummyPlan = false;

  public static final String ROUTE_TYPE_FORWARD = "forward";
  public static final String ROUTE_TYPE_ROLLOVER = "rollover";
  public static final String ROUTE_TYPE_NONE = "none";

  static final long serialVersionUID = 7891301741944476265L;

  public CallPlanBinding (String cp, String cr, int t, TM stm, TM etm, int prio, String rt) {
    this(cp,cr,(long)t,stm,etm,prio,rt);
  }

  public CallPlanBinding (String cp, String cr, long t, TM stm, TM etm, int prio, String rt) {
    cpname = cp;
    crname = cr;
    refreshTime = t;
    tmStart = stm;
    tmEnd = etm;
    priority = prio;
    
    flag  = getFlag(rt);
/*
    if(isValidRouteType(rt))
      routeType = rt;
    else
      routeType = ROUTE_TYPE_NONE;
*/
  }

  public CallPlanBinding (String cp, String cr, long t, TM stm, TM etm, int prio, int flag) {
    cpname = cp;
    crname = cr;
    refreshTime = t;
    tmStart = stm;
    tmEnd = etm;
    priority = prio;
    this.flag  = flag;
  }


  public CallPlanBinding (String cp, String cr, int t, TM st, TM et) {
    cpname = cp;
    crname = cr;
    refreshTime = (long)t;
    tmStart = st;
    tmEnd = et;
    priority = 0;
    flag  = 0;

  }


  public CallPlanBinding (String cp, CallPlanBinding cpb) {
    cpname = cp;
    crname = cpb.getCallPlanRouteName();
    refreshTime = cpb.getRefreshTime().getTime()/1000;
    try {
      tmStart = (TM)cpb.getStartTM().clone();
    } catch (CloneNotSupportedException ce) {
      tmStart = new TM();
    }
    try {
      tmEnd = (TM)cpb.getEndTM().clone();
    } catch (CloneNotSupportedException ce) {
      tmEnd = new TM();
    }
    priority = cpb.getPriority();
//    routeType = cpb.getRouteType();
    flag  = cpb.getFlag();

  }

  public void reset (String cp, String cr, long rt, TM ts, TM te, int pr, int flag, boolean dummy) {
    cpname = cp;
    crname = cr;
    refreshTime = rt;
    tmStart = ts;
    tmEnd = te;
    priority = pr;
    this.flag = flag;
    dummyPlan = dummy;
  }

  public void dump () {
    SimpleDateFormat df = new SimpleDateFormat("EEE MMM dd hh:mm:ssa yyyy z");
    System.out.println("Call Plan Name: " + cpname);
    System.out.println("Call Plan Route Name: " + crname);
    System.out.println("Start: " + tmStart);
    System.out.println("End: " + tmEnd);
    System.out.println("Priority: " + priority);
//    System.out.println("Route Type: " + routeType);
    System.out.println("Refresh Time: " + df.format(new Date(refreshTime*1000)));
  }

  public synchronized String getCallPlanName () {
    return cpname;
  }

  public synchronized void setCallPlanName (String s) {
    cpname = s;
  }

  public synchronized String getCallPlanRouteName () {
    return crname;
  }

  public synchronized void setCallPlanRouteName (String s) {
    crname = s;
  }

  public synchronized void setDummyPlan (boolean value) {
    dummyPlan = value;
  }

  public synchronized boolean isDummyPlan () {
    return dummyPlan;
  }

  public synchronized Date getRefreshTime () {
    return new Date(refreshTime*1000);
  }

  public synchronized long getRefreshTimeInLong () {
    return refreshTime;
  }

  public synchronized void setRefreshTime(long time) {
    refreshTime = time;
  }

  public synchronized TM getStartTM () {
    return tmStart;
  }

  public synchronized void setStartTM (TM newtm) {
    tmStart.setTM(newtm);
  }

  public synchronized TM getEndTM () {
    return tmEnd;
  }

  public synchronized void setEndTM (TM newtm) {
    tmEnd.setTM(newtm);
  }

  public synchronized int getPriority () {
    return priority;
  }

  public synchronized void setPriority (int value) {
    priority = value;
  }

  public synchronized String getType () {
    if(( flag  & CRF_REJECT) ==  CRF_REJECT)
      return TYPE_REJECT;
    return TYPE_NORMAL;
  }


  public synchronized String getRouteType () {
    if( (flag  & CRF_FORWARD) ==  CRF_FORWARD)
      return ROUTE_TYPE_FORWARD;
    if( (flag  & CRF_ROLLOVER)==  CRF_ROLLOVER)
      return ROUTE_TYPE_ROLLOVER;
    return ROUTE_TYPE_NONE;
  }


  public synchronized void setRouteType (String rt) {
    flag  |=  getFlag(rt);
/*    if (isValidRouteType(rt))
      routeType = rt;
    else
      throw new IllegalArgumentException("Route type (" + rt + ") is invalid");
*/
  }

/*  public boolean isValidRouteType (String rt) {
    if (rt != null &&
	(rt.equals(ROUTE_TYPE_FORWARD) ||
	 rt.equals(ROUTE_TYPE_ROLLOVER) ||
	 rt.equals(ROUTE_TYPE_NONE)))
      return true;

    return false;
  }
*/


  public int getFlag(String rt){
    if(rt !=  null){
      if(rt.equals(ROUTE_TYPE_FORWARD))
        return  CRF_FORWARD;
      if(rt.equals(ROUTE_TYPE_ROLLOVER))
        return  CRF_ROLLOVER;
      if(rt.equals(TYPE_REJECT))
        return  CRF_REJECT;
    }
    return 0;
  }

  public synchronized void addFlag (int f) {
    flag |= f;
  }

  public synchronized void deleteFlag (int f) {
    flag &= ~f;
  }

      
  public String toString () {
    return cpname + "/" + crname;
  }

  public int compareTo (Object o) {
    if (o.getClass().equals(CallPlanBinding.class)) {
      CallPlanBinding cpb = (CallPlanBinding)o;
      int rs = cpb.getPriority() - priority;
      if (rs == 0)
        return crname.compareTo(cpb.getCallPlanRouteName());

      return rs;
    }

    return toString().compareTo(o.toString());
  }

  public Object clone () throws CloneNotSupportedException {
    return super.clone();
  }


  public synchronized void setFlag(int flag){
    this.flag = flag;
  }

  public synchronized int getFlag(){
    return flag;
  }




  public CallPlanBinding copy () throws Exception{
 
    // serialize calling plan binding into byte array

    ByteArrayOutputStream baos = new ByteArrayOutputStream();
    ObjectOutputStream oos = new ObjectOutputStream(baos);
    oos.writeObject(this);
    byte buf[] = baos.toByteArray();
    oos.close();

    // deserialize byte array into Calling plan binding

    ByteArrayInputStream bais =   new ByteArrayInputStream(buf);
    ObjectInputStream ois = new ObjectInputStream(bais);
    CallPlanBinding cpb  = (CallPlanBinding)ois.readObject();
    ois.close();

    return cpb;
      
  }


  public void resetAll(){
    cpname = "";
    crname = "";
    refreshTime = 0;
    tmStart = new TM();
    tmEnd   = new TM();
    priority = 0;
    flag  = 0;
  }
/*
  public Object getPrimaryKey(){
    return cpname;
  }

  public Object getSecondaryKey(){
    return crname;

  }

  public String getPrimaryField(){
    return "cpname";
  }
  public String getSecondaryField(){
    return "crname";
  }

  public String getFieldValue(String fieldName){
    return "";
  }
*/
  
  public static Object getObject(Object[] row){
 /*   Calendar sCal = Calendar.getInstance();
    sCal.setTime( new Date ( ((Long)row[INDEX_CPB_STIME]).longValue()));

    Calendar eCal = Calendar.getInstance();
    eCal.setTime( new Date ( ((Long)row[INDEX_CPB_ETIME]).longValue()));
*/
    return new CallPlanBinding((String)row[INDEX_CPB_CPNAME],
    (String)row[INDEX_CPB_CRNAME],
    ((Long)row[INDEX_CPB_REFRESH]).longValue(),
    new TM(
    ((Integer)row[INDEX_CPB_STIME_SEC]).intValue(),
    ((Integer)row[INDEX_CPB_STIME_MIN]).intValue(),
    ((Integer)row[INDEX_CPB_STIME_HOUR]).intValue(),
    ((Integer)row[INDEX_CPB_STIME_MDAY]).intValue(),
    ((Integer)row[INDEX_CPB_STIME_MON]).intValue(),
    ((Integer)row[INDEX_CPB_STIME_YEAR]).intValue(),
    ((Integer)row[INDEX_CPB_STIME_WDAY]).intValue(),
    ((Integer)row[INDEX_CPB_STIME_YDAY]).intValue(),
    ((Integer)row[INDEX_CPB_STIME_ISDST]).intValue()
    ),
    new TM(
    ((Integer)row[INDEX_CPB_ETIME_SEC]).intValue(),
    ((Integer)row[INDEX_CPB_ETIME_MIN]).intValue(),
    ((Integer)row[INDEX_CPB_ETIME_HOUR]).intValue(),
    ((Integer)row[INDEX_CPB_ETIME_MDAY]).intValue(),
    ((Integer)row[INDEX_CPB_ETIME_MON]).intValue(),
    ((Integer)row[INDEX_CPB_ETIME_YEAR]).intValue(),
    ((Integer)row[INDEX_CPB_ETIME_WDAY]).intValue(),
    ((Integer)row[INDEX_CPB_ETIME_YDAY]).intValue(),
    ((Integer)row[INDEX_CPB_ETIME_ISDST]).intValue()    
    ),
    ((Integer)row[INDEX_CPB_PRIORITY]).intValue(),
    ((Integer)row[INDEX_CPB_FLAG]).intValue()
    );

  }


  //  implements DBObjet interface
  // order should be same as in the IViewDB calling plan binding table fields
  public Object[] getObjectArray(){
    Object[]  data= new Object[23];
    data[0] = cpname;
    data[1] = crname;
    data[2] = new Long(refreshTime);
    data[3] = new Integer(tmStart.getSecond());
    data[4] = new Integer(tmStart.getMinute());
    data[5] = new Integer(tmStart.getHour());
    data[6] = new Integer(tmStart.getMday());
    data[7] = new Integer(tmStart.getMonth());
    data[8] = new Integer(tmStart.getYear());
    data[9] = new Integer(tmStart.getWday());
    data[10] = new Integer(tmStart.getYday());
    data[11] = new Integer(tmStart.getDST());
    data[12] = new Integer(tmEnd.getSecond());
    data[13] = new Integer(tmEnd.getMinute());
    data[14] = new Integer(tmEnd.getHour());
    data[15] = new Integer(tmEnd.getMday());
    data[16] = new Integer(tmEnd.getMonth());
    data[17] = new Integer(tmEnd.getYear());
    data[18] = new Integer(tmEnd.getWday());
    data[19] = new Integer(tmEnd.getYday());
    data[20] = new Integer(tmEnd.getDST());
    data[21] = new Integer(priority);
    data[22] = new Integer(flag);
    return data;
  }

  public String[] getKeys(){
    String[] ob = new String[2];
    ob[0] = "cpname";
    ob[1] = "crname";
    return ob;
  }

  public Object[] getValues(){
    Object[] ob = new Object[2];
    ob[0] = cpname;
    ob[1] = crname;
    return ob;

  }
}
