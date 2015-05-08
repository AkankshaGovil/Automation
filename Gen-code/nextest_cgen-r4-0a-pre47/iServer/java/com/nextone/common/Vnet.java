package com.nextone.common;

import java.util.*;
import java.io.*;
import java.lang.*;
import com.nextone.util.IPUtil;

public class Vnet implements Serializable, Comparable ,DBObject, Cloneable {
    private String  name;
    private String  ifName;
    private int id;
    private int rtgTblId;
    private String gateway;

    static final long serialVersionUID = 33369047652010002L;

    public Vnet() {
	name    = "";
	ifName  = "";
	id    = -1;
	rtgTblId  = -1;
	gateway  = "";
    }

    public Vnet(
		String  name,
		String  ifName,
		int id,
		int rtgTblId,
		String gateway
		){

	this.name = (name ==  null)?"":name;
	this.ifName = (ifName ==  null)?"":ifName;
	this.id = id;
	this.rtgTblId  = rtgTblId;
	this.gateway = (gateway ==  null)?"":gateway;
    }

    public String  getName(){
	return name;
    }

    public void setName(String name){
	this.name = name;
    }

    public String  getIfName(){
	return ifName;
    }

    public void setIfName(String ifName){
	this.ifName = ifName;
    }

    public int getId(){
	return id;
    }

    public void setId(int id){
	this.id = id;
    }

    public void setRtgTblId(int rtgTblId){
	this.rtgTblId  = rtgTblId;
    }

    public int getRtgTblId() {
	return rtgTblId;
    }

    public void setGateway(String id){
	gateway  = id;
    }

    public String getGateway(){
	return gateway;
    }

    public String toString () {
	StringBuffer sb = new StringBuffer();
	sb.append("\n Name = "+name);
	sb.append("\n IfName = "+ifName);
	sb.append("\n Id = "+id);
	sb.append("\n RtgTblId = "+rtgTblId);
	sb.append("\n proxyegId = "+gateway);
	return sb.toString();
    }

    public boolean equals (Object o) {
	if (o instanceof Vnet &&
	    ((Vnet)o).getName().equals(getName()) &&
	    ((Vnet)o).getIfName().equals(getIfName()) &&
	    ((Vnet)o).getId()  ==  getId() &&
	    ((Vnet)o).getRtgTblId() ==  getRtgTblId()  &&
	    ((Vnet)o).getGateway() ==  getGateway()
	    )
	    return true;
	return false;
    }

    public int compareTo (Object o) {
	if (o instanceof Vnet)
	    return name.compareTo(((Vnet)o).getName());
	return -1;
    }

    public synchronized void update (Vnet r) {
	setName(r.getName());
	setIfName(r.getIfName());
	setId(r.getId());
	setRtgTblId(r.getRtgTblId());
	setGateway(r.getGateway());
    }

    public  void  resetAll(){
	name    = "";
	ifName  = "";
	id    = -1;
	rtgTblId   = -1;
	gateway  = "";
    }


    public static Object getObject(Object[] row){
	return new Vnet(
			(String)row[CommonConstants.INDEX_VNET_NAME],
			(String)row[CommonConstants.INDEX_VNET_IFNAME],
			((Integer)row[CommonConstants.INDEX_VNET_ID]).intValue(),
			((Integer)row[CommonConstants.INDEX_VNET_RTGTBLID]).intValue(),
			((String)row[CommonConstants.INDEX_VNET_GATEWAY])
			);
    }
    //  implements DBObjet interface
    // order should be same as in the IViewDB calling plan binding table fields
    public Object[] getObjectArray(){
	Object[]  vals= new Object[5];
	vals[0] = name;
	vals[1] = ifName;
	vals[2] = new Integer(id);
	vals[3] = new Integer(rtgTblId);
	vals[4] = gateway;

	return vals;
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


    public Vnet copy () throws Exception{
 
	// serialize Vnet into byte array

	ByteArrayOutputStream baos = new ByteArrayOutputStream();
	ObjectOutputStream oos = new ObjectOutputStream(baos);
	oos.writeObject(this);
	byte buf[] = baos.toByteArray();
	oos.close();

	// deserialize byte array into Vnet

	ByteArrayInputStream bais =   new ByteArrayInputStream(buf);
	ObjectInputStream ois = new ObjectInputStream(bais);
	Vnet t  = (Vnet)ois.readObject();
	ois.close();

	return t;
      
    }

    public Object clone () throws CloneNotSupportedException {
	return super.clone();
    }
}

