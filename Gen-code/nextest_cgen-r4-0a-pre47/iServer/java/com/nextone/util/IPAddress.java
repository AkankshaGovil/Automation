package com.nextone.util;

import java.io.*;
import java.util.*;
import com.nextone.util.JMask.*;

/**
 * Class used to represent an IP address.
 * Byte 0 is the MSB. 
 */
public class IPAddress implements Comparator, Comparable, Serializable {
  private int []ipAddress; // int is required because byte is a signed value.

  /*
   * Default Constructor
   */
  public IPAddress() {
    ipAddress = new int[4];
		ipAddress[0] = 256;
  }

  /*
   * Construct from a LimitedDataInputStream
   */
  public IPAddress(LimitedDataInputStream dis) throws IOException {
    read(dis);
  }

  /*
   * Copy Constructor
   */
  public IPAddress(IPAddress addr) {
    ipAddress = new int[4];
    for (int i=0; i<4; i++) {
      ipAddress[i] = addr.ipAddress[i];
    }
  }

  /*
   * Constructor to build an IPAddress from a int[].
   */
  public IPAddress(int[] newAddress) {
    ipAddress = newAddress;
  }

  /*
   * Constructor to build an IPAddress from a String.
   */
  public IPAddress(String newAddress) {
		if (newAddress.equals("")) {
			ipAddress = new int[4];
			ipAddress[0] = 256;
		} else {
			ipAddress = IPUtil.ipStringToIntArray(newAddress);
		}
  }

  /*
   * Is this IP Address valid
   */
  public boolean isZero() {
		for (int i=0; i<4; i++) {
			if (ipAddress[i] != 0)
				return false;
		}
		return true;
	}

  /*
   * Is this IP Address valid
   */
  public boolean isValid() {
    if (ipAddress[0] == 256)
			return false;

		return true;
  }

  /**
   * Write the object to the given output stream
   */
  public void write(DataOutputStream dos) throws IOException {
		if (ipAddress[0] == 256) {
			// Write 0s.
			for (int i=0; i<4; i++) {
				dos.writeByte(0);
			}
		} else {
			for (int i=0; i<4; i++) {
				dos.writeByte(ipAddress[i]);
			}
    }
  }

  /**
   * Initialize the object from the given input stream.
   * @throws java.io.IOException
   */
  public void read(LimitedDataInputStream dis) throws IOException {
    ipAddress = new int[4];
    for (int i=0; i<4; i++) {
      ipAddress[i] = dis.readUnsignedByte();
    }
  }

  /**
   * Return the current IPAddress as a string.
   */
  public String toString() {
		if (ipAddress[0] == 256)
			return "";

    return IPUtil.intArrayToIPString(ipAddress);
  }

  /**
   * Apply the given netmask to this address.
   * Return the new value.
   */
  public IPAddress applyMask(IPAddress netmask) {
    IPAddress ip = new IPAddress(this);
    for (int i=0; i<4; i++) {
      ip.ipAddress[i] = ipAddress[i] & netmask.ipAddress[i];
    }
    return ip;
  }

  /**
   * Test if the given address is less than this address.
   * Return true if the given address is less.
   */
  public boolean isLessThan(IPAddress test) {
    // compare the values for each byte until a mismatch is found
    for (int i=0; i<4; i++) {
      // If the values are equal, keep looping
      if (ipAddress[i] < test.ipAddress[i]) {
        return true;
      } else if (ipAddress[i] > test.ipAddress[i]) {
        return false;
      }
    }
    
    // To reach here the values must be equal
    return false;
  }

  ///////////////////////////////////
  // Comparable Interface
  ///////////////////////////////////
  public int compareTo(Object o1) {
    return compare(o1,this);
  }
  
  ///////////////////////////////////
  // Comparator Interface
  ///////////////////////////////////
  // Return <0 for o1<o2
  // Return >0 for o1>o2
  // Return 0 for o1==o2
  public int compare(Object o1, Object o2) {
    if (((IPAddress)o1).isLessThan((IPAddress)o2)) {
      return -1;
    }
    
    if (o1.equals(o2)) {
      return 0;
    }

    return 1;
  }

  /**
   * Compare the two addresses
   */
  public boolean equals(IPAddress addr) {
    for (int i=0; i<4; i++) {
      if (ipAddress[i] != addr.ipAddress[i]) {
        return false;
      }
    }
    return true;
  }
}
