package com.nextone.common;

import java.util.*;
import java.io.*;
import java.lang.*;


public class Trigger implements Serializable, Comparable ,DBObject, Cloneable {
  String  name;
  int event;
  int script;
  String data;
  int srcVendor;
  int dstVendor;
  boolean override;

  static final long serialVersionUID = 33369047652010001L;

  public Trigger() {
    name  = "";
    event = 0;
    script  = 0;
    data  = "";
    srcVendor = 0;
    dstVendor = 0;
    override  = false;
  }

  public Trigger(
                String  name,
	              int event,
                int script,
                String data,
                int srcVendor,
                int dstVendor,
                boolean override
                ){
    this.name   = name;
	  this.event  = event;
    this.script = script;
    this.data = data;
	  this.srcVendor  = srcVendor;
    this.dstVendor  = dstVendor;
    this.override   = override;
  }

  public void setName(String name){
    this.name = (name ==  null)?"":name;
  }

  public String getName(){
    return name;
  }

  public void setEvent(int event){
    this.event  = event;
  }

  public int getEvent(){
    return event;
  }

  public void setSrcVendor(int srcVendor){
    this.srcVendor  = srcVendor;
  }

  public int getSrcVendor(){
    return srcVendor;
  }

  public void setDstVendor(int dstVendor){
    this.dstVendor  = dstVendor;
  }

  public int getDstVendor(){
    return dstVendor;
  }

  public void setOverride(boolean override){
    this.override  = override;
  }

  public boolean isOverride() {
    return override;
  }

  public void setScript(int script){
    this.script = script;
  }

  public int getScript(){
    return script;
  }

  public void setData(String data){
    this.data = (data ==  null)?"":data;
  }

  public String getData(){
    return data;
  }


  public String toString () {
    StringBuffer sb = new StringBuffer();
    sb.append("\nName = "+name);
    sb.append("\nEvent  = "+event);
    sb.append("\nScript = "+script);
    sb.append("\nData = "+data);
    sb.append("\nSrcVendor  = "+srcVendor);
    sb.append("\nDstVendor  = "+dstVendor);
    sb.append("\nOverride  = " + override);

    return sb.toString();
  }

  public boolean equals (Object o) {
   if (o instanceof Trigger &&
	   ((Trigger)o).getName().equals(getName()) &&
	   ((Trigger)o).getEvent() == getEvent() &&
     ((Trigger)o).getScript() == getScript() &&
     ((Trigger)o).getData().equals(getData()) &&
     ((Trigger)o).getSrcVendor() == getSrcVendor() &&
     ((Trigger)o).getDstVendor() == getDstVendor() 
     )
    return true;
    return false;
  }

	public int compareTo (Object o) {
	 if (o instanceof Trigger)
		return name.compareTo(((Trigger)o).getName());
   return -1;
	}

  public synchronized void update (Trigger t) {

	  setName(t.getName());
	  setEvent(t.getEvent());
	  setScript(t.getScript());
	  setData(t.getData());
	  setSrcVendor(t.getSrcVendor());
	  setDstVendor(t.getDstVendor());

  }

  public  void  resetAll(){
    name  = "";
    event = 0;
    script    = 0;
    data  = "";
    srcVendor = 0;
    dstVendor = 0;
    override  = false;

  }

  public static Object getObject(Object[] row){
    return new Trigger(
      (String)row[CommonConstants.INDEX_TG_NAME],
      ((Integer)row[CommonConstants.INDEX_TG_EVENT]).intValue(),
      ((Integer)row[CommonConstants.INDEX_TG_SCRIPT]).intValue(),
      (String)row[CommonConstants.INDEX_TG_DATA],
      ((Integer)row[CommonConstants.INDEX_TG_SRCVENDOR]).intValue(),
      ((Integer)row[CommonConstants.INDEX_TG_DSTVENDOR]).intValue(),
      // boolean type is not supported in Mysql database
      // Instead it is defined as TINYINT
      (((Short)row[CommonConstants.INDEX_TG_OVERRIDE]).shortValue() == 1)?true:false
      );
  }
  //  implements DBObjet interface
  // order should be same as in the IViewDB calling plan binding table fields
  public Object[] getObjectArray(){
    Object[]  vals= new Object[7];
    vals[0] = name;
    vals[1] = new Integer(event);
    vals[2] = new Integer(script);
    vals[3] = data;
    vals[4] = new Integer(srcVendor);
    vals[5] = new Integer(dstVendor);
  // boolean type is not supported in Mysql database
  // Instead it is defined as TINYINT

    vals[6] = getShort(override);

    return vals;
  }

  public Short getShort(boolean flag){
      if (flag)
          return new Short((short)1);
      else
          return new Short((short)0);

  }

  public String[] getKeys(){
    String[] ob = new String[1];
    ob[0] = "name";
    return ob;
  }

  public Object[] getValues(){
    Object[] ob = new Object[1];
    ob[0] = name;
    return ob;
  }


  public Trigger copy () throws Exception{
 
    // serialize calling plan binding into byte array

    ByteArrayOutputStream baos = new ByteArrayOutputStream();
    ObjectOutputStream oos = new ObjectOutputStream(baos);
    oos.writeObject(this);
    byte buf[] = baos.toByteArray();
    oos.close();

    // deserialize byte array into Calling plan binding

    ByteArrayInputStream bais =   new ByteArrayInputStream(buf);
    ObjectInputStream ois = new ObjectInputStream(bais);
    Trigger t  = (Trigger)ois.readObject();
    ois.close();

    return t;
      
  }

  public Object clone () throws CloneNotSupportedException {
     return super.clone();
  }
}

