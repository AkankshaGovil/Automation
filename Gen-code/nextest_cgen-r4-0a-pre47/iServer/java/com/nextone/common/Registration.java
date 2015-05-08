package com.nextone.common;

import java.io.*;
import java.net.*;
import com.nextone.util.LimitedDataInputStream;

/**
 * this class makes up the registration reply received from an iEdge 5x0
 *
 * TODO: Make this class process registrations from 1000 and iServer also
 */
public class Registration implements Serializable{
	  private short mode = -1, deviceid = -1, ipmode = -1, version = -1;
	  private String serial, fqdn, name, regid, configv, softromv, ramv;
	  private InetAddress ipaddr;
	  private short t1Status;

	  static final long serialVersionUID = 5226746421780120496L;

	  public Registration (String regid, int mode, int devid) {
		 this.regid = regid;
		 this.serial = regid;
		 this.mode = (short)mode;
		 this.deviceid = (short)devid;
	  }

	  public Registration (DatagramPacket dp) throws IOException {
		 this(dp, new ByteArrayInputStream(dp.getData()), dp.getLength());
	  }

	  public Registration (DatagramPacket dp, InputStream is, int len) throws IOException {
		 LimitedDataInputStream dis = null;

		 // already determined if this is a registration message
		 if (is instanceof LimitedDataInputStream && len == -1) {
			dis = (LimitedDataInputStream)is;
		 } else {
			dis = new LimitedDataInputStream(is, len);
			int code = dis.readShort();
			if (code != CommonConstants.REGISTRATION)
			   throw new IOException("Not a registration message (" + code + ")");
		 }

		 ipaddr = dp.getAddress();

		 mode = dis.readShort();
		 deviceid = dis.readShort();

		 serial = dis.readUTF();
		 fqdn = dis.readUTF();
		 ipmode = dis.readShort();

		 name = dis.readUTF();
		 version = dis.readShort();
		 regid = dis.readUTF();

		 if (deviceid != CommonConstants.DEVICE_ID_1000) {

			configv = dis.readUTF();
			softromv = dis.readUTF();
			if (mode == CommonConstants.RAM_MODE)
			   ramv = dis.readUTF();
		 }

		 if (deviceid == CommonConstants.DEVICE_ID_1000) {

			 try {
				t1Status = dis.readShort();
				// ram version number was added for 1.3 release
				ramv = dis.readUTF();


			 }catch(EOFException eofe) {
			 // For older version display without T1 status.
                           //				t1Status = iEdge1000Constants.T1_NOSTATUS;
			}
		 }

//		 System.out.println(System.currentTimeMillis() + ": received reg from " + regid);
	  }
	  public synchronized InetAddress getAddress () {
		 return ipaddr;
	  }

	  public synchronized short getMode () {
		 return mode;
	  }

	  public synchronized short getDeviceId () {
		 return deviceid;
	  }

	  public synchronized short getIPMode () {
		 return ipmode;
	  }

	  public synchronized short getVersion () {
		 return version;
	  }

	  public synchronized String getSerialNumber () {
		 return serial;
	  }

	  public synchronized String getFqdn () {
		 return fqdn;
	  }

	  public synchronized String getName () {
		 return name;
	  }

	  public synchronized String getRegId () {
		 return regid;
	  }

	  public synchronized String getConfigVersion () {
		 return configv;
	  }

	  public synchronized String getSoftromVersion () {
		 return softromv;
	  }

	  public synchronized String getRamVersion () {
		 return ramv;
	  }

	  public synchronized short getT1Status () {
		 return t1Status;
	  }
	  public String toString () {
		 StringBuffer sb = new StringBuffer();
		 sb.append("IP Address: " + ipaddr + "\n");
		 sb.append("mode: " + mode + "\n");
		 sb.append("deviceid: " + deviceid + "\n");
		 sb.append("ip mode: " + ipmode + "\n");
		 sb.append("hello version: " + version + "\n");
		 sb.append("serial number: " + serial + "\n");
		 sb.append("fqdn: " + fqdn + "\n");
		 sb.append("name: " + name + "\n");
		 sb.append("registration ID: " + regid + "\n");
		 sb.append("config version: " + configv + "\n");
		 sb.append("softrom version: " + softromv + "\n");
		 sb.append("ram version: " + ramv);
		 sb.append("T1 status: " + t1Status);

		 return sb.toString();
	  }

	  public void dump () {
		 System.out.println(toString());
	  }

}

