package com.nextone.JServer;

import java.net.*;
import java.io.*;
import java.util.*;
import com.nextone.util.TCPServer;
import com.nextone.common.ProvisionData;
import com.nextone.common.BridgeException;
import com.nextone.common.CommonConstants;

/**
 * a TCP server which serves the contacting iview clients with the list
 * of vpn groups/vpn ids/iedges
 */
public class ListServer extends TCPServer {
  protected Object lockObject;

  static {
    System.loadLibrary("BridgeServer");
  }

  private native boolean listIedge (SendListItem sli) throws BridgeException;
  private native boolean listVpn (SendListItem sli) throws BridgeException;
  private native boolean listVpnGroup (SendListItem sli) throws BridgeException;

  ListServer (ThreadGroup tg, boolean debug, Object lockObject) throws IOException {
    super(tg, Constants.JSERVER_IEDGELIST_SERVER_PORT, debug);
    this.lockObject = lockObject;
  }
 
  // this function will be called from different threads, one for
  // each client connecting to get the information
  public void TCPServerWork (Socket sock) {
    synchronized (lockObject) {
      JServer.printDebug("client from " + sock.getInetAddress().toString() + " requesting vpn/iedge list", JServer.DEBUG_VERBOSE);

      SendListItem sli = null;

      try {
	sli = new SendListItem(new ObjectOutputStream(sock.getOutputStream()));
      } catch (IOException ie) {
	JServer.printDebug("Error getting output stream:", JServer.DEBUG_ERROR);
	JServer.printDebug(ie, JServer.DEBUG_ERROR);
	return;
      }

      ObjectInputStream ois = null;
      try {
	ois = new ObjectInputStream(sock.getInputStream());
      } catch (IOException ie) {
	JServer.printDebug("Error getting input stream:", JServer.DEBUG_ERROR);
	JServer.printDebug(ie, JServer.DEBUG_ERROR);
	return;
      }

      Object o = null;
      try {
	o = ois.readObject();
	if (!o.getClass().equals(Integer.class)) {
	  JServer.printDebug("Error getting input data. Input should be list type Integer" , JServer.DEBUG_ERROR);
	  return;
	}
      } catch (Exception e) { 
	JServer.printDebug("Error getting input data:" , JServer.DEBUG_ERROR);
	JServer.printDebug(e, JServer.DEBUG_ERROR);
	return;
      }

      int listId = ((Integer)o).intValue();

      boolean result3 = true;
      if ((listId == CommonConstants.LIST_ALL) ||
	  (listId == CommonConstants.IEDGE_GROUP_ONLY)) {
	// send the vpn group list 
	result3 = false;
	try {
	  result3 = listVpnGroup(sli);
	} catch (Exception e) {
	  JServer.printDebug("Error listing VPN groups:", JServer.DEBUG_ERROR);
	  JServer.printDebug(e, JServer.DEBUG_ERROR);
	  try {
	    sli.send(e);
	  } catch (IOException ie) {
	    JServer.printDebug("Error in sending vpn group list error status:", JServer.DEBUG_WARNING);
	    JServer.printDebug(ie, JServer.DEBUG_WARNING);
	  }
	}
      }

      boolean result2 = true;
      if (result3 &&
	  (listId == CommonConstants.LIST_ALL ||
	   listId == CommonConstants.IEDGE_VPN_ONLY)) {
	// send vpn list 
	result2 = false;
	try {
	  result2 = listVpn(sli);
	} catch (Exception e) {
	  JServer.printDebug("Error listing VPNs:", JServer.DEBUG_ERROR);
	  JServer.printDebug(e, JServer.DEBUG_ERROR);
	  try {
	    sli.send(e);
	  } catch (IOException ie) {
	    JServer.printDebug("Error in sending vpn list error status:", JServer.DEBUG_WARNING);
	    JServer.printDebug(ie, JServer.DEBUG_WARNING);
	  }
	}
      }

      boolean result1 = true;
      if (result3 && result2 &&
	  (listId == CommonConstants.LIST_ALL ||
	   listId == CommonConstants.IEDGE_ONLY)) {
	// send the iedge list 
	result1 = false;
	try {
	  result1 = listIedge(sli);
	} catch (Exception be) {
	  JServer.printDebug("Error listing endpoints:", JServer.DEBUG_ERROR);
	  JServer.printDebug(be, JServer.DEBUG_ERROR);
	  try {
	    sli.send(be);
	  } catch (IOException ie) {
	    JServer.printDebug("Error in sending iedge list error status:", JServer.DEBUG_WARNING);
	    JServer.printDebug(ie, JServer.DEBUG_WARNING);
	  }
	}
      }

      // send the status of the operation
      boolean result = false;
      if (result1 && result2 && result3)
	result = true;
      try {
	sli.send(new Boolean(result));
      } catch (IOException ie) {
	JServer.printDebug("Error sending vpn/iedge list final status:", JServer.DEBUG_ERROR);
	JServer.printDebug(ie, JServer.DEBUG_ERROR);
      }

      try {
	sock.shutdownOutput();
      } catch (IOException ie) {
	JServer.printDebug("Error closing vpn/iedge list output stream:", JServer.DEBUG_WARNING);
	JServer.printDebug(ie, JServer.DEBUG_WARNING);
      }
    }
  }

  // stops this server
  public void stop () {
    super.stopAndWait();
    JServer.printDebug("ListServer exited", JServer.DEBUG_NORMAL);
  }

}

