package com.nextone.common;

import java.io.*;

/**
 * this class is used to provide a way to sort ports properly in
 * a table
 */

public class RegidPortPair implements Serializable, Comparable {
	  private String regid;
	  private int port;

	  static final long serialVersionUID = 5411071027775549421L;

	  public RegidPortPair (String regid, int port) {
		 this.regid = regid;
		 this.port = port;
	  }

	  public String toString () {
		 return regid + "/" + (port+1);
	  }

	  public String getRegId () {
		 return regid;
	  }

	  public int getPort () {
		 return port;
	  }

	  public int compareTo (Object o) {
		 RegidPortPair rpp = (RegidPortPair)o;
		 int result = regid.compareTo(rpp.getRegId());
		 if (result == 0)
			result = port - rpp.getPort();

		 return result;
	  }

    public boolean equals(Object o){
		 RegidPortPair rpp = (RegidPortPair)o;
		 int result = regid.compareTo(rpp.getRegId());
		 if (result == 0)
			result = port - rpp.getPort();

		 if (result ==  0)
       return true;
     return false;
    }

    public int hashCode(){
      int hash  = 0;
      hash  = regid.hashCode();
      hash  +=  port;
      return hash;

    }

}

