package com.nextone.common;

import java.util.*;
import java.io.*;
import java.lang.*;
import com.nextone.util.IPUtil;

public class Realm implements Serializable, Comparable ,DBObject, Cloneable {
  private String  name;
  private String  ifName;
  private String  vnetName;
  private String  vipName;
  private long id;
  private long rsa;
  private long mask;
  private int sigPoolId;
  private int medPoolId;
  private int addrType;
  private int adminStatus;
  private int opStatus;
  private short imr;
  private short emr;



  static final long serialVersionUID = 33369047652010001L;

  public Realm() {
    name    = "";
    ifName  = "";
    vnetName  = "";
    vipName = "";
    id    = -1;
    rsa   = -1;
    mask  = -1;
    sigPoolId = -1;
    medPoolId = -1;
    addrType  = -1;
    adminStatus    = -1;
    opStatus    = -1;
  }

  public Realm(
      String  name,
      String  ifName,
      String  vnetName,
      String  vipName,
      long id,
      long rsa,
      long mask,
      int sigPoolId,
      int medPoolId,
      int addrType,
      int adminStatus,
      int opStatus,
      short imr,
      short emr
    ){

      this.name = (name ==  null)?"":name;
      this.ifName = (ifName ==  null)?"":ifName;
      this.vnetName = (vnetName ==  null)?"":vnetName;
      this.vipName  = vipName;
      this.id = id;
      this.rsa  = rsa;
      this.mask = mask;
      this.sigPoolId  = sigPoolId;
      this.medPoolId  = medPoolId;
      this.addrType = addrType;
      this.adminStatus   = adminStatus;
      this.opStatus   = opStatus;
      this.imr  = imr;
      this.emr  = emr;
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

  public String  getVnetName(){
    return vnetName;
  }

  public void setVnetName(String vnetName){
    this.vnetName = vnetName;
  }

  public String  getVipName(){
    return vipName;
  }

  public void setVipName(String vipName){
    this.vipName  = vipName;
  }

  public long getId(){
    return id;
  }

  public void setId(long id){
    this.id = id;
  }

  public String getRsa(){
    if (rsa == 0)
      return "";

    return IPUtil.intToIPString((int)rsa);

  }

  public void setRsa(long rsa){
    this.rsa  = rsa;
  }

  public void setRsa(String data){
    this.rsa  = IPUtil.ipStringToLong(data);
  }

  public long getRsaAsLong () {
    return rsa;
  }

  public String getMask(){
    if (mask== 0)
      return "";
    return IPUtil.intToIPString((int)mask);
  }

  public void setMask(long mask){
    this.mask = mask;
  }

  public void setMask(String data){
    this.mask = IPUtil.ipStringToLong(data);
  }

  public long getMaskAsLong () {
    return mask;
  }

  public int getSigPoolId(){
    return sigPoolId;
  }

  public void setSigPoolId(int sigPoolId){
    this.sigPoolId  = sigPoolId;
  }

  public int getMedPoolId(){
    return medPoolId;
  }

  public void setMedPoolId(int medPoolId){
    this.medPoolId  = medPoolId;
  }

  public int getAddrType(){
    return addrType;
  }

  public void setAddrType(int addrType){
    this.addrType = addrType;
  }

  public short getImr(){
    return imr;
  }

  public void setImr(short imr){
    this.imr  = imr;
  }

  public short getEmr(){
    return emr;
  }

  public void setEmr(short emr){
    this.emr  = emr;
  }

  public int getAdminStatus(){
    return adminStatus;
  }

  public void setAdminStatus(int status){
    this.adminStatus = status;
  }

  public int getOpStatus(){
    return opStatus;
  }

  public void setOpStatus(int status){
    this.opStatus = status;
  }


  public boolean isOpEnabled(){
    return (opStatus> 0 ? true: false);
  }

  public void setOpEnabled(boolean enabled){
    if(enabled)
      opStatus  = 1;
    else
      opStatus  = 0;

  }

  public boolean isAdminEnabled(){
    return (adminStatus> 0 ? true: false);
  }

  public void setAdminEnabled(boolean enabled){
    if(enabled)
      adminStatus  = 1;
    else
      adminStatus  = 0;

  }



  public String toString () {
    StringBuffer sb = new StringBuffer();
    sb.append("\n Name = "+name);
    sb.append("\n IfName = "+ifName);
    sb.append("\n VnetName = "+vnetName);
    sb.append("\n VipName = "+vipName);
    sb.append("\n Id = "+id);
    sb.append("\n Rsa = "+rsa);
    sb.append("\n Mask = "+mask);
    sb.append("\n SigPoolId = "+sigPoolId);
    sb.append("\n MedPoolId = "+medPoolId);
    sb.append("\n AddrType = "+addrType);
    sb.append("\n AdminStatus = "+adminStatus);
    sb.append("\n Op Status = "+opStatus);
    sb.append("\n Imr = "+imr);
    sb.append("\n Emr = "+emr);
    return sb.toString();
  }

  public boolean equals (Object o) {
   if (o instanceof Realm &&
      ((Realm)o).getName().equals(getName()) &&
      ((Realm)o).getIfName().equals(getIfName()) &&
      ((Realm)o).getVnetName().equals(getVnetName()) &&
      ((Realm)o).getVipName().equals(getVipName()) &&
      ((Realm)o).getId()  ==  getId() &&
      ((Realm)o).getRsa() ==  getRsa()  &&
      ((Realm)o).getMask().equals(getMask()) &&
      ((Realm)o).getSigPoolId() ==  getSigPoolId()  &&
      ((Realm)o).getMedPoolId() ==  getMedPoolId()  &&
      ((Realm)o).getAdminStatus() ==  getAdminStatus()  &&
      ((Realm)o).getOpStatus() ==  getOpStatus()  &&
      ((Realm)o).getAddrType()  ==  getAddrType() &&
      ((Realm)o).getImr() ==  getImr()  &&
      ((Realm)o).getEmr() ==  getEmr()
      )
      return true;
    return false;
  }

    public int compareTo (Object o) {
	if (o instanceof Realm)
	    return name.compareTo(((Realm)o).getName());
	return -1;
    }

  public synchronized void update (Realm r) {
    setName(r.getName());
    setIfName(r.getIfName());
    setVnetName(r.getVnetName());
    setVipName(r.getVipName());
    setId(r.getId());
    setRsa(r.getRsaAsLong());
    setMask(r.getMaskAsLong());
    setSigPoolId(r.getSigPoolId());
    setMedPoolId(r.getMedPoolId());
    setAddrType(r.getAddrType());
    setAdminStatus(r.getAdminStatus());
    setOpStatus(r.getOpStatus());
    setImr(r.getImr());
    setEmr(r.getEmr());

  }

  public  void  resetAll(){
    name    = "";
    ifName  = "";
    vnetName  = "";
    vipName = "";
    id    = -1;
    rsa   = -1;
    mask  = -1;
    sigPoolId = -1;
    medPoolId = -1;
    adminStatus  = -1;
    opStatus  = -1;
    addrType  = -1;
  }

  public static Object getObject(Object[] row){
    return new Realm(
      (String)row[CommonConstants.INDEX_REALM_NAME],
      (String)row[CommonConstants.INDEX_REALM_IFNAME],
      (String)row[CommonConstants.INDEX_REALM_VNETNAME],
      (String)row[CommonConstants.INDEX_REALM_VIPNAME],
      ((Long)row[CommonConstants.INDEX_REALM_ID]).longValue(),
      ((Long)row[CommonConstants.INDEX_REALM_RSA]).longValue(),
      ((Long)row[CommonConstants.INDEX_REALM_MASK]).longValue(),
      ((Integer)row[CommonConstants.INDEX_REALM_SIGPOOLID]).intValue(),
      ((Integer)row[CommonConstants.INDEX_REALM_MEDPOOLID]).intValue(),
      ((Integer)row[CommonConstants.INDEX_REALM_ADDRTYPE]).intValue(),
      ((Integer)row[CommonConstants.INDEX_REALM_ASTATUS]).intValue(),
      ((Integer)row[CommonConstants.INDEX_REALM_OSTATUS]).intValue(),
      ((Short)row[CommonConstants.INDEX_REALM_IMR]).shortValue(),
      ((Short)row[CommonConstants.INDEX_REALM_EMR]).shortValue()
      );
  }
  //  implements DBObjet interface
  // order should be same as in the IViewDB calling plan binding table fields
  public Object[] getObjectArray(){
    Object[]  vals= new Object[14];
    vals[0] = name;
    vals[1] = ifName;
    vals[2] = vipName;
    vals[3] = new Long(id);
    vals[4] = new Long(rsa);
    vals[5] = new Long(mask);
    vals[6] = new Integer(sigPoolId);
    vals[7] = new Integer(medPoolId);
    vals[8] = new Integer(addrType);
    vals[9] = new Integer(adminStatus);
    vals[10] = new Integer(opStatus);
    vals[11] = new Short(imr);
    vals[12] = new Short(emr);
    vals[14] = vnetName;

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


  public Realm copy () throws Exception{
 
    // serialize Realm into byte array

    ByteArrayOutputStream baos = new ByteArrayOutputStream();
    ObjectOutputStream oos = new ObjectOutputStream(baos);
    oos.writeObject(this);
    byte buf[] = baos.toByteArray();
    oos.close();

    // deserialize byte array into Realm

    ByteArrayInputStream bais =   new ByteArrayInputStream(buf);
    ObjectInputStream ois = new ObjectInputStream(bais);
    Realm t  = (Realm)ois.readObject();
    ois.close();

    return t;
      
  }

  public Object clone () throws CloneNotSupportedException {
     return super.clone();
  }
}

