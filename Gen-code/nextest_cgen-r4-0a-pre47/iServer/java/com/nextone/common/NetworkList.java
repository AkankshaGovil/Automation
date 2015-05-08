package com.nextone.common;

import java.util.*;

import com.nextone.util.IPMask;

public class NetworkList {
  private IPMask ipmask;
  private boolean isPublic;

  public NetworkList (IPMask ipm, boolean isp) {
    ipmask = ipm;
    isPublic = isp;
  }

  public void setIPMask (IPMask ipm) {
    ipmask = ipm;
  }

  public IPMask getIPMask () {
    return ipmask;
  }

  public void setIsNetworkPublic (boolean value) {
    isPublic = value;
  }

  public boolean isNetworkPublic () {
    return isPublic;
  }

  public String toString () {
    StringBuffer sb = new StringBuffer(ipmask.toString());
    sb.append("  ");
    if (isPublic)
      sb.append("public");
    else
      sb.append("private");

    return sb.toString();
  }

  public boolean equals (Object o) {
    if (o.getClass().equals(NetworkList.class) &&
	((NetworkList)o).getIPMask().equals(ipmask) &&
	((NetworkList)o).isNetworkPublic() == isPublic)
      return true;

    return false;
  }
}

