package com.nextone.common;

import java.io.*;
import java.text.*;
import java.util.*;
import javax.swing.tree.*;
import com.nextone.util.IPUtil;

/**
 * class which holds all the details about an IEdgeGroup (as stored in the iserver)
 * The definition of the fields is based on struct IgrpInfo defined in key.h
 */
public class IEdgeGroup implements Serializable, Comparable, Cloneable, DBObject, CommonConstants {
  private String igrpName;
  private int    maxCallsIn;
  private int    maxCallsOut;
  private int    maxCallsTotal;

  private int    callsIn;
  private int    callsOut;
  private int    callsTotal;
  private long   dndTime;   /* last time run out of calls */  

  static final long serialVersionUID = 33369047652010001L;

  public IEdgeGroup() {
    init();
  }

  // do not update this. Not used from iserver_db21
  // this constructor is used in BridgeServerImpl.c
  public IEdgeGroup(String igrpName,
                   int    maxCallsIn, 
                   int    maxCallsOut,
                   int    maxCallsTotal,
                   int    callsIn,
                   int    callsOut,
                   int    callsTotal,
                   long   dndTime) {

    this.igrpName      = igrpName;
    this.maxCallsIn    = maxCallsIn;
    this.maxCallsOut   = maxCallsOut;
    this.maxCallsTotal = maxCallsTotal;
    this.callsIn       = callsIn;
    this.callsOut      = callsOut;
    this.callsTotal    = callsTotal;
    this.dndTime       = dndTime;
  }

  public IEdgeGroup(String igrpName) {
    this();
    this.igrpName = (igrpName  ==  null)?"":igrpName;
  }

  public String toString () {
    StringBuffer sb = new StringBuffer("igrpName:");
    sb.append(getName() + ", ");
    sb.append("maxCallsIn:"  + getMaxCallsIn() + ", ");
    sb.append("maxCallsOut:" + getMaxCallsOut() + ", ");
    sb.append("maxCallsTotal:" + getMaxCallsTotal() + ", ");
    sb.append("callsIn:"  + getCallsIn() + ", ");
    sb.append("callsOut:" + getCallsOut() + ", ");
    sb.append("callsTotal:" + getCallsTotal() + ", ");
    sb.append("dndTime:" + getDndTime().toString());
    return sb.toString();
  }

  public synchronized String getName() {
    return igrpName;
  }

  public synchronized void setName(String s) {
    igrpName = s;
  }

  public synchronized int getMaxCallsIn() {
    return maxCallsIn;
  }

  public synchronized void setMaxCallsIn (int mci) {
    maxCallsIn = mci;
  }

  public synchronized int getMaxCallsOut() {
    return maxCallsOut;
  }

  public synchronized void setMaxCallsOut (int mco) {
    maxCallsOut = mco;
  }

  public synchronized int getMaxCallsTotal() {
    return maxCallsTotal;
  }

  public synchronized void setMaxCallsTotal (int mt) {
    maxCallsTotal = mt;
  }

  public synchronized int getCallsIn() {
    return callsIn;
  }

  public synchronized void setCallsIn (int ci) {
    callsIn = ci;
  }

  public synchronized int getCallsOut() {
    return callsOut;
  }

  public synchronized void setCallsOut (int co) {
    callsOut = co;
  }

  public synchronized int getCallsTotal() {
    return callsTotal;
  }

  public synchronized void setCallsTotal (int t) {
    callsTotal = t;
  }

  public synchronized Date getDndTime() {
    return new Date(dndTime);
  }

  public synchronized void setDndTime (long t) {
    dndTime = t;
  }

  public int compareTo (Object o) {
    if (o instanceof IEdgeGroup) {
      return igrpName.compareTo(((IEdgeGroup)o).getName());
    }

    return -1; 
  }

  public Object clone () {
    try{
      return super.clone();
    }catch(Exception e){
      IEdgeGroup ieGroup = new IEdgeGroup();
      ieGroup.update(this);
      return ieGroup;
    }
  }

  public synchronized void update (IEdgeGroup ieg) {
    igrpName      = ieg.getName();
    maxCallsIn    = ieg.getMaxCallsIn();
    maxCallsOut   = ieg.getMaxCallsOut();
    maxCallsTotal = ieg.getMaxCallsTotal();
    callsIn       = ieg.getCallsIn();
    callsOut      = ieg.getCallsOut();
    callsTotal    = ieg.getCallsTotal();
    dndTime       = ieg.getDndTime().getTime();
  }

  public void init(){
    igrpName      = "";
    maxCallsIn    = 0;
    maxCallsOut   = 0;
    maxCallsTotal = 0;
    callsIn       = 0;
    callsOut      = 0;
    callsTotal    = 0;
    dndTime       = 0;
  }

  public void resetIEdgeGroup(){
    init();
  }

  public static Object getObject(Object[] row){
    return new IEdgeGroup(
      ((String)row[INDEX_IGRP_NAME]),
      ((Integer)row[INDEX_IGRP_MAXIN]).intValue(),
      ((Integer)row[INDEX_IGRP_MAXOUT]).intValue(),
      ((Integer)row[INDEX_IGRP_MAXTOTAL]).intValue(),
      ((Integer)row[INDEX_IGRP_IN]).intValue(),
      ((Integer)row[INDEX_IGRP_OUT]).intValue(),
      ((Integer)row[INDEX_IGRP_TOTAL]).intValue(),
      ((Long)row[INDEX_IGRP_TIME]).longValue()
      );
  }

  public IEdgeGroup copy () throws Exception {
    // serialize iedge group into byte array
    ByteArrayOutputStream baos = new ByteArrayOutputStream();
    ObjectOutputStream oos = new ObjectOutputStream(baos);
    oos.writeObject(this);
    byte buf[] = baos.toByteArray();
    oos.close();

    // deserialize byte array into Calling plan binding
    ByteArrayInputStream bais =   new ByteArrayInputStream(buf);
    ObjectInputStream ois = new ObjectInputStream(bais);
    IEdgeGroup igrp  = (IEdgeGroup)ois.readObject();
    ois.close();

    return igrp;

  }

  //  implements DBObjet interface
  // order should be same as in the IViewDB igrp table fields
  public Object[] getObjectArray(){
    Object[]  data= new Object[8];

    data[0] = igrpName;
    data[1] = new Integer(maxCallsIn);
    data[2] = new Integer(maxCallsOut);
    data[3] = new Integer(maxCallsTotal);
    data[4] = new Integer(callsIn);
    data[5] = new Integer(callsOut);
    data[6] = new Integer(callsTotal);
    data[7] = new Long(dndTime);

    return data;
  }

  public String[] getKeys(){
    String[] ob = new String[1];
    ob[0] = "igrpName";
    return ob;
  }

  public Object[] getValues(){
    Object[] ob = new Object[1];
    ob[0] = igrpName;
    return ob;
  }

  public boolean equals (Object o) {
    if (o instanceof IEdgeGroup && 
        getName().equals(((IEdgeGroup)o).getName()))
      return true;

    return false;
  }
}
