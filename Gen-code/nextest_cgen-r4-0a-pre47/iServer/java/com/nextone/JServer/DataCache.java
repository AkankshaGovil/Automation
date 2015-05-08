package com.nextone.JServer;

import java.util.*;
import java.net.*;
import java.io.*;
import com.nextone.common.GetListData;
import com.nextone.common.IEdgeList;
import com.nextone.common.VpnGroupList;
import com.nextone.common.VpnList;


/**
 * This class is used to retrieve the iserver database (phone/vpn/etc)
 * information. This class acts as a cache and makes sure we are not
 * retrieving the information too many times
 */
public class DataCache {
  private static DataCache thisInstance;
  private static GetListData.ListDataRequester ldr;
  private Vector vpngrpList,  iedgeList, vpnnameList;
  private String listError, listErrorDetails;
  private boolean listStatus;
  private long lastTouched;

  private DataCache () {
    vpngrpList = new Vector();
    //		 vpnidList = new Vector();
    vpnnameList = new Vector();
    iedgeList = new Vector();

    ldr = new GetListData.ListDataRequester () {
	public void listDone (GetListData gld) {
	  vpngrpList = gld.getList(VpnGroupList.class);
	  vpnnameList = gld.getList(VpnList.class);
	  iedgeList = gld.getList(IEdgeList.class);
	  listError = gld.getMessage();
	  listErrorDetails = gld.getDetails();
	  listStatus = gld.getStatus();
	}

	public int getGetTimeout () {
	  return Constants.GET_TIMEOUT;
	}

	public int getSetTimeout () {
	  return Constants.SET_TIMEOUT;
	}
      };
  }

  /**
   * This method is the only way to get access to this class, this method
   * might block sometimes, depending on the age of the cache
   * Once a reference is obtained through this method, the data contents
   * can be accesses through the other public methods in this class
   *
   * @param port the port to connect to get the database
   */
  public synchronized static DataCache getInstance (int port) {
    if (thisInstance == null)
      thisInstance = new DataCache();

    long currentTime = System.currentTimeMillis();
    if (currentTime > (thisInstance.lastTouched + 300000)) {
      try {
	new GetListData(port, InetAddress.getLocalHost(), Constants.IEDGE_LIST, ldr).retrieve();
	thisInstance.lastTouched = currentTime;
      } catch (UnknownHostException uhe) {
	thisInstance.listError = "Error in network configuration, cannot find local host IP address!";
	thisInstance.listErrorDetails = uhe.toString();
	thisInstance.vpngrpList = new Vector();
	//			   thisInstance.vpnidList = new Vector();
	thisInstance.vpnnameList = new Vector();
	thisInstance.iedgeList = new Vector();
      }
    }

    return thisInstance;
  }

  public synchronized boolean getStatus () {
    return listStatus;
  }

  public synchronized String getErrorMessage () {
    return new String(listError);
  }

  public synchronized String getErrorDetails () {
    return new String(listErrorDetails);
  }

  public synchronized Vector getIEdgeList () throws IllegalStateException {
    if (listStatus)
      return iedgeList;
    throw new IllegalStateException("List incomplete: " + listError + "\n" + listErrorDetails);
  }

  public synchronized Vector getVpnNameList () throws IllegalStateException {
    if (listStatus)
      return vpnnameList;
    throw new IllegalStateException("List incomplete: " + listError + "\n" + listErrorDetails);
  }

  public synchronized Vector getVpnGroupList () throws IllegalStateException {
    if (listStatus)
      return vpngrpList;
    throw new IllegalStateException("List incomplete: " + listError + "\n" + listErrorDetails);
  }
}

