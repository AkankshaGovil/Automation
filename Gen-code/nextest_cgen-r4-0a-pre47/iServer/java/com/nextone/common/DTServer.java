package com.nextone.common;

import java.net.*;
import java.io.*;

/**
 * a utility class to store information about a server which holds
 * the file(s) of interest
 */
public class DTServer implements Cloneable, Serializable {
	  private String serverName;
	  private InetAddress serverAddress;
	  private String user;
	  private String password;
	  private String [] dir;
	  private String [] file;

	  private static final int I500 = 0;
	  private static final int I510 = 1;
	  private static final int I1000 = 2;
	  private static final int MAX_DEV_TYPES = 3;

	  private int action;
	  private FileVersion [] fileVersions;

	  static final long serialVersionUID = 2035457601087806934L;

	  public DTServer (int action) {
		 this("", "", "", null, null, action);
	  }

	  public DTServer (String sn, String u, String p, String [] d, String [] f, int a) {
		 setServerName(sn);
		 this.user = u;
		 this.password = p;
		 this.dir = d;
		 if (dir == null)
			dir = new String [MAX_DEV_TYPES];
		 this.file = f;
		 if (file == null)
			file = new String [MAX_DEV_TYPES];
		 this.action = a;
		 MaintenanceRequest.getActionString(action); // validate the action
	  }

	  public int getAction () {
		 return action;
	  }

	  public static int [] getSupportedDeviceTypes () {
		 int [] result = new int [3];
		 result[0] = CommonConstants.DEVTYPE_I500;
		 result[1] = CommonConstants.DEVTYPE_I510;
		 result[2] = CommonConstants.DEVTYPE_I1000;
		 return result;
	  }

	  private int getIndex (int devType) throws IllegalArgumentException {
		 switch (devType) {
			case CommonConstants.DEVTYPE_I500:
			  return I500;
			case CommonConstants.DEVTYPE_I510:
			  return I510;
			case CommonConstants.DEVTYPE_I1000:
			  return I1000;
		 }

		 throw new IllegalArgumentException("Device Type (" + devType + ") is not in valid range [" + CommonConstants.DEVTYPE_I500 + ", " + CommonConstants.DEVTYPE_I510 + ", " + CommonConstants.DEVTYPE_I1000 + "]");
	  }

	  private int getDevType (int index) throws IllegalArgumentException {
		 switch (index) {
			case I500:
			  return CommonConstants.DEVTYPE_I500;
			case I510:
			  return CommonConstants.DEVTYPE_I510;
			case I1000:
			  return CommonConstants.DEVTYPE_I1000;
		 }

		 throw new IllegalArgumentException("Index (" + index + ") is invalid");
	  }

	  public synchronized String getServerName () {
			return serverName;
	  }

	  public synchronized void setServerName (String val) {
		 serverName = val;
		 if (serverName != "") {
			try {
			   serverAddress = InetAddress.getByName(serverName);
			} catch (Exception e) {
			   serverAddress = null;
			}
		 } else
			serverAddress = null;
	  }

	  public synchronized InetAddress getServerAddress () {
		 return serverAddress;
	  }

	  public synchronized void setServerAddress (InetAddress val) {
		 serverAddress = val;
		 if (serverAddress == null)
			serverName = null;
		 else
			serverName = serverAddress.getHostName();
	  }

	  public synchronized String getUsername () {
		 return user;
	  }

	  public synchronized void setUsername (String val) {
		 user = val;
	  }

	  public synchronized String getPassword () {
		 return password;
	  }

	  public synchronized void setPassword (String val) {
		 password = val;
	  }

	  public synchronized String getDirectory (int devType) {
		 return dir[getIndex(devType)];
	  }

	  public synchronized void setDirectory (int devType, String val) {
		 dir[getIndex(devType)] = val;
	  }

	  public synchronized String getFile (int devType) {
		 return file[getIndex(devType)];
	  }

	  public synchronized void setFile (int devType, String val) {
		 file[getIndex(devType)] = val;
	  }

	  public synchronized String getFilePath (int devType) {
		 StringBuffer sb = new StringBuffer();
		 
		 String d = dir[getIndex(devType)];
		 if (d != null && !d.equals("")) {
			sb.append(d);
			String sep = getSeparator(devType);
			if (!sb.toString().endsWith(sep))
			   sb.append(getSeparator(devType));
		 }
		 d = file[getIndex(devType)];
		 if (d != null && !d.equals(""))
			sb.append(d);

		 return sb.toString();
	  }

	  public synchronized void setFilePath (int devType, String val) {

		 // if the file separator is "/"
		 int index = val.lastIndexOf("/");
		 if (index != -1) {
			// separator is "/"
			dir[getIndex(devType)] = val.substring(0, index+1);
			file[getIndex(devType)] = val.substring(index+1, val.length());
		 } else {
			index = val.lastIndexOf("\\");
			if (index != -1) {
			   // separator is "\"
			   dir[getIndex(devType)] = val.substring(0, index+1);
			   file[getIndex(devType)] = val.substring(index+1, val.length());
			} else {
			   // could not find either "/" or "\"
			   dir[getIndex(devType)] = "";
			   file[getIndex(devType)] = val;
			}
		 }

	  }

	  public String getSeparator (int devType) {
		 String d = dir[getIndex(devType)];

		 if (d == null || d.equals(""))
			return "/";

		 if (d.lastIndexOf("/") != -1)
			return "/";
		 if (d.lastIndexOf("\\") != -1)
			return "\\";

		 return "/";
	  }

	  public boolean isServerEquals (DTServer s) {
		 if (s == null)
			return false;

		 if ((serverName == null ||
			  serverName.equals(s.getServerName())) &&
			 user.equals(s.getUsername()) &&
			 password.equals(s.getPassword()))
			return true;

		 return false;
	  }

	  public boolean isDirEquals (DTServer s) {
		 for (int i = 0; i < MAX_DEV_TYPES; i++) {
			String d1 = getDirectory(getDevType(i));
			String d2 = s.getDirectory(getDevType(i));
			if ((d1 != null && !d1.equals(d2)) ||
				(d2 != null && !d2.equals(d1)) ||
				d1 != d2)
			   return false;
		 }
		 return true;
	  }

	  public boolean isFileEquals (DTServer s) {
		 for (int i = 0; i < MAX_DEV_TYPES; i++) {
			String f1 = getFile(getDevType(i));
			String f2 = s.getFile(getDevType(i));
			if ((f1 != null && !f1.equals(f2)) ||
				(f2 != null && !f2.equals(f1)) ||
				f1 != f2)
			   return false;
		 }
		 return true;
	  }

	  public boolean isFilePathEquals (DTServer s) {
		 for (int i = 0; i < MAX_DEV_TYPES; i++) {
			String fp1 = getFilePath(getDevType(i));
			String fp2 = s.getFilePath(getDevType(i));
			if (!fp1.equals(fp2))
			   return false;
		 }

		 return true;
	  }

	  public boolean equals (Object o) {
		 if (o instanceof DTServer &&
			 isServerEquals((DTServer)o) &&
			 isFilePathEquals((DTServer)o))
			return true;

		 return false;
	  }

	  public Object clone () {
		 return new DTServer(serverName, user, password, dir, file, action);
	  }

	  public String toString () {
		 StringBuffer sb = new StringBuffer("com.nextone.util.DTServer");
		 sb.append("[");
		 sb.append(serverName);
		 sb.append("][");
		 sb.append(user);
		 sb.append("][");
		 sb.append(password);
		 sb.append("][");
		 for (int i = 0; i < MAX_DEV_TYPES; i++) {
			sb.append(dir[i]);
			sb.append(":");
			sb.append(file[i]);
			sb.append("][");
		 }

		 return sb.substring(0, sb.length()-1);
	  }

	  private FileVersion getFileVersion (int devType) {
		 if (fileVersions == null)
			fileVersions = new FileVersion [MAX_DEV_TYPES];

		 FileVersion fv = fileVersions[getIndex(devType)];
		 if (fv == null) {
			fileVersions[getIndex(devType)] = new FileVersion(action, devType, this);
			fv = fileVersions[getIndex(devType)];
		 }

		 return fv;
	  }

	  public synchronized String getVersion (int devType) throws IOException {

		 FileVersion fv = getFileVersion(devType);

		 if (fv.getVersion() == null)  // try once to retrieve the file
			fv.retrieveVersion();  // will block until the file is downloaded

		 return fv.getVersion();  // may or may not have the version
	  }

	  public synchronized void setVersion (int devType, String ver) throws IOException {
		 getFileVersion(devType).setVersion(ver);
	  }

	  public synchronized String getConfig (int devType) throws IOException {
		 FileVersion fv = getFileVersion(devType);
		 byte [] b = fv.getFile();
		 if (b == null)
			fv.retrieveFile();  // blocks until ftp is done
		 if ((b = fv.getFile()) == null) // still couldn't get the file
			throw new IOException("Unable to retrieve the configuration file");

		 return new String(b);
	  }

	  public synchronized void setConfig (int devType, String val) throws IOException {
		 getFileVersion(devType).setFile(val.getBytes());
	  }

	  public synchronized void saveConfig (int devType, String regid, String config) throws IOException {
		 getFileVersion(devType).saveConfig(regid, config);
	  }
}
