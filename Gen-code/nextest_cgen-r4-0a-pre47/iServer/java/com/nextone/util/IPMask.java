package com.nextone.util;

import java.net.InetAddress;
import java.net.UnknownHostException;

/**
 * this class represents an IP address and a netmask combo
 *
 * TODO: also make this class deal with x.x.x.x/d format
 */
public class IPMask {
	private InetAddress ip, mask;

	/**
	 * parses a string that is in xx.xx.xx.xx/xx.xx.xx.xx format and returns
	 * the IPMask object
	 *
	 * @param str the string in x.x.x.x/x.x.x.x format
	 */
	public static IPMask parseString (String str) throws UnknownHostException {
	  int index = str.indexOf("/");

	  // no "/", assume that it is an ip address string
	  if (index == -1) {
		IPMask ipm = new IPMask();
		ipm.setIP(str);
		return ipm;
	  }

	  return new IPMask(str.substring(0, index), str.substring(index+1, str.length()));
	}

	private IPMask () {}

	public IPMask (String ip, String mask) throws UnknownHostException {
	  setIP(ip);
	  setMask(mask);
	}

	public IPMask (InetAddress ip, InetAddress mask) {
	  setIP(ip);
	  setMask(mask);
	}

	public void setIP (InetAddress addr) {
	  ip = addr;
	}

	public void setIP (String addr) throws UnknownHostException {
	  if (addr.length() > 0)
		setIP(InetAddress.getByName(addr));
	}

	public void setMask (InetAddress addr) {
	  mask = addr;
	}

	public void setMask (String addr) throws UnknownHostException {
	  if (addr.length() > 0)
		setMask(InetAddress.getByName(addr));
	}

	public InetAddress getIP () {
	  return ip;
	}

	public String getIPAsString () {
	  return (ip == null)?"":ip.getHostAddress();
	}

	public InetAddress getMask () {
	  return mask;
	}

	public String getMaskAsString () {
	  return (mask == null)?"":mask.getHostAddress();
	}

	public boolean equals (Object o) {
	  if (o.getClass().equals(com.nextone.util.IPMask.class)) {
		IPMask given = (IPMask)o;
		if (((ip == null && given.getIP() == null) ||
			 ip.equals(given.getIP())) &&
			((mask == null && given.getMask() == null) ||
			 mask.equals(given.getMask())))
		  return true;
	  }
	  return false;
	}
			
	public String toString () {
	  StringBuffer sb = new StringBuffer();
	  if (ip != null)
		sb.append(ip.getHostAddress());
	  if (mask != null) {
		sb.append("/");
		sb.append(mask.getHostAddress());
	  }

	  return sb.toString();
	}
}

