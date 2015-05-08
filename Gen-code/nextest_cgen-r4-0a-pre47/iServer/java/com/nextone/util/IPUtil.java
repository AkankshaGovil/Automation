/**
 * Static classes related to IP
 */
package com.nextone.util;

import java.util.*;
import java.io.*;
import java.net.*;
import java.text.*;

public class IPUtil {
	  public static final short TCP = 0x6;
	  public static final short UDP = 0x11;
	  public static final short ANY = 0x0;


    /**
     * Static method to return the string representation of
     * an int IP address.
     */
	  public static String intToIPString (int addr) {
		 int [] ip = new int [4];
		 ip[0] = (addr & 0xff000000) >>> 24;
		 ip[1] = (addr & 0x00ff0000) >>> 16;
		 ip[2] = (addr & 0x0000ff00) >>> 8;
		 ip[3] = (addr & 0x000000ff);

		 return intArrayToIPString(ip);
	  }

    /**
     * Static method to return the string representation of
     * an int[] IP address.
     */
	  public static String intArrayToIPString (int [] in) {
		 return new String(new StringBuffer(15).append(in[0]).append('.').append(in[1]).append('.').append(in[2]).append('.').append(in[3]));
	  }

    /**
     * Static method to return the int[] representation of
     * a String IP address.
     */
	  public static int [] ipStringToIntArray (String str) {
		 int [] ip = new int[4];
		 StringTokenizer st = new StringTokenizer(str, ".");
		 if (st.countTokens() == 4) {
			for (int i = 0; st.hasMoreElements(); i++) {
			   try {
				  ip[i] = Integer.parseInt(st.nextToken());
			   } catch (NumberFormatException nfe) {
				  return null;
			   }
			}
			return ip;
		 }
		 return null;
	  }

	  public static long ipStringToLong (String str) {
		 return IPUtil.ipIntArrayToLong(IPUtil.ipStringToIntArray(str));
	  }

	  public static long ipIntArrayToLong (int [] ip) {
		 long ret = 0;
		 if (ip != null) {
			ret |= ((ip[0] & 0x000000ff) << 24);
			ret |= ((ip[1] & 0x000000ff) << 16);
			ret |= ((ip[2] & 0x000000ff) << 8);
			ret |= (ip[3] & 0x000000ff);
		 }
		 return ret;
	  }

	  public static boolean isValidIP (String str) {
		 return isValidIP(IPUtil.ipStringToIntArray(str));
	  }

	  public static boolean isValidIP (int [] ip) {
		 if (ip == null)
			return false;

		 if (  (ip[0] == 0) ||
			   (ip[0] > 255) || (ip[1] > 255) || (ip[2] > 255) || (ip[3] > 255)
			) {
			return false;
		 }

		 long lip = IPUtil.ipIntArrayToLong(ip);
		 if (!IPUtil.isClassA(lip) && !IPUtil.isClassB(lip) && !IPUtil.isClassC(lip))
			return false;

		 return true;
	  }

	  public static boolean isValidMask (String str) {
		 int [] ip = IPUtil.ipStringToIntArray(str);

		 if (ip == null) {
			return false;
		 }

		 if (  (ip[0] == 0) ||
			   (ip[0] > 255) || (ip[1] > 255) || (ip[2] > 255) || (ip[3] > 255)
			) {
			return false;
		 }
		 return true;
	  }

	  public static boolean isClassA (long ip) {
		 ip = (ip & 0xffffffff);
		 if ((ip & 0x80000000) == 0)
			return true;
		 return false;
	  }

	  public static boolean isClassB (long ip) {
		 ip = (ip & 0xffffffff);
		 if ((ip & 0xc0000000) == 0x80000000)
			return true;
		 return false;
	  }

	  public static boolean isClassC (long ip) {
		 ip = (ip & 0xffffffff);
		 if ((ip & 0xe0000000) == 0xc0000000)
			return true;
		 return false;
	  }

	  public static boolean isIPConsistantWithMask (long ip) {
		 return IPUtil.isIPConsistantWithMask(ip, 0, false);
	  }

	  public static boolean isIPConsistantWithMask (long ip, long mask) {
		 return IPUtil.isIPConsistantWithMask(ip, mask, true);
	  }

	  private static boolean isIPConsistantWithMask (long ip, long mask, boolean isMaskValid) {
		 ip = (ip & 0xffffffff);

		 long bitmask = 0;
		 if (IPUtil.isClassA(ip))
			bitmask = 0x00ffffff;
		 else if (IPUtil.isClassB(ip))
			bitmask = 0x0000ffff;
		 else if (IPUtil.isClassC(ip))
			bitmask = 0x000000ff;
		 else
			return false;

		 if (!isMaskValid)
			mask = ~bitmask;
		 mask = (mask & 0xffffffff);
/*
		 if ((mask & bitmask) != 0) {  // there is a subnet part
			long netPart = ((mask & bitmask) & ip);
			if (netPart == 0 ||
				netPart == (mask & bitmask)) {
			   return false;
			}
		 }
*/
		 long hostPart = (ip & ~mask);
		 if (hostPart == 0 ||
			 hostPart == ~mask) {
			return false;
		 }

		 return true;
	  }

	  public static String ipStringFromStream (DataInputStream dis) throws IOException {
		 int [] ip = new int [4];
		 for (int i = 0; i < 4; i++)
			ip[i] = dis.readUnsignedByte();

		 return intArrayToIPString(ip);
	  }

	  public static String ipStringFromStream (LimitedDataInputStream dis) throws IOException {
		 int [] ip = new int [4];
		 for (int i = 0; i < 4; i++)
			ip[i] = dis.readUnsignedByte();

		 return intArrayToIPString(ip);
	  }

	  public static boolean isValidShortValue (int val) {
		 if (val < 0 || val > 65535)
			return false;

		 return true;
	  }

	  public static int [] macStringToIntArray (String mc) throws ParseException {
		 StringTokenizer st = new StringTokenizer(mc, ":");
		 if (st.countTokens() != 6)
			throw new ParseException("Invalid MAC Address format, expecting xx:xx:xx:xx:xx:xx", 0);
		 int [] elements = new int [6];
		 int k = 0;
		 while (st.hasMoreTokens())
			elements[k++] = Integer.parseInt(st.nextToken(), 16);
		 return elements;
	  }

	  public static String intArrayToMACString (int [] in) {
		 return IPUtil.intArrayToMACString(in, false);
	  }

	  public static String intArrayToMACString (int [] in, boolean pad) {
		 StringBuffer mac = new StringBuffer(20);
		 for (int i = 0; i < 6; i++) {
			mac.append(SysUtil.prePad(Integer.toHexString(in[i]), '0', 2));
			if (i < 5)
			   mac.append(":");
		 }
		 return new String(mac);
	  }


	  /**
	   * returns the protocol string for the given protocol number
	   */
	  public static String getProtocolString (short prot) {
		 switch (prot) {
			case TCP:
			  return new String("tcp");
			case UDP:
			  return new String("udp");
			case ANY:
			  return new String("any");
		 }

		 return new String("unknown");
	  }

  /**
   * returns a comparator that compares InetAddress objects
   */
  public static Comparator newInetAddressComparator () {
    return new Comparator () {
	public int compare (Object o1, Object o2) {
	  return ((InetAddress)o1).getHostAddress().compareTo(((InetAddress)o2).getHostAddress());
	}

      };
  }

}



