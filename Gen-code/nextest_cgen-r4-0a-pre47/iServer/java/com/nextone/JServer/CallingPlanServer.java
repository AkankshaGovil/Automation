package com.nextone.JServer;

import java.net.*;
import java.io.*;
import java.util.*;
import com.nextone.util.TCPServer;
import com.nextone.common.BridgeException;
import com.nextone.common.CallPlan;
import com.nextone.common.CallPlanBinding;
import com.nextone.common.CallPlanRoute;
import com.nextone.common.CommonConstants;

/**
 * a TCP server which serves the contacting iview clients with the list
 * of calling plans available on the system
 */
public class CallingPlanServer extends TCPServer {
  protected Object lockObject;

  static {
    System.loadLibrary("BridgeServer");
  }

  private native boolean listCallingPlan (SendListItem sli, CallPlan cp) throws BridgeException;
  private native boolean listCallingPlanRoute (SendListItem sli, CallPlanRoute cpr) throws BridgeException;
  private native boolean listCallingPlanBind (SendListItem sli, String filter, CallPlanBinding cpb) throws BridgeException;

  CallingPlanServer (ThreadGroup tg, boolean debug, Object lockObject) throws IOException {
    super(tg, Constants.JSERVER_CALLINGPLAN_SERVER_PORT, debug);
    this.lockObject = lockObject;
  }


  // this function will be called from different threads, one for
  // each client connecting to get the information
  public void TCPServerWork (Socket sock) {
    synchronized (lockObject) {
      JServer.printDebug("client from " + sock.getInetAddress().toString() + " requesting calling plan list", JServer.DEBUG_VERBOSE);

      boolean keepSending = true;
      SendListItem sli = null;
      try {
	sock.setSoLinger(true,2);
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
	  JServer.printDebug("Error getting input data. Input should be type Integer", JServer.DEBUG_ERROR);
	  return;
	}
      } catch (Exception e) { 
	JServer.printDebug("Error getting input data:" , JServer.DEBUG_ERROR);
	JServer.printDebug(e, JServer.DEBUG_ERROR);
	return;
      }

      int id = ((Integer)o).intValue();
      o = null;
      String filter = "";
      try {
	o = ois.readObject();
	if (!o.getClass().equals(String.class)) {
	  JServer.printDebug("Error getting input data. Input should be type(String)" , JServer.DEBUG_ERROR);
	  return;
	}
	filter = (String)o;
      } catch (Exception e) { 
	JServer.printDebug("Filter is not specified:", JServer.DEBUG_VERBOSE);
	JServer.printDebug(e, JServer.DEBUG_ERROR);
      }
         
      // send a dummy calling plan bind list, this will trigger an
      // exception in the older iViews (1.1), and will confirm the new
      // capabilities in the newer iViews (1.2)
      // ALWAYS SEND THIS OBJECT FIRST, THERE ARE MANY THINGS IN IVIEW DEPENDING ON RECEIVING THIS FIRST
      boolean result = false;
      try {
	// any older iviews, which get this object would complain about
	// a software mismatch and exit gracefully
	CallPlanBinding cpb = new CallPlanBinding("", "", 0, 0, 0);
	cpb.setDummyPlan(true);
	sli.send(cpb);
	result = true;
      } catch (Exception e) {
	JServer.printDebug("Error sending bind capability:", JServer.DEBUG_ERROR);
	JServer.printDebug(e, JServer.DEBUG_ERROR);
	try {
	  sli.send(e);
	} catch (IOException ie) {
	  keepSending = false;  // don't even try sending anymore
	  JServer.printDebug("Error in sending bind capability error status:", JServer.DEBUG_WARNING);
	  JServer.printDebug(ie, JServer.DEBUG_WARNING);
	}
      }

      try {
	sli.flush();
      } catch (IOException ie) {
	JServer.printDebug(ie, JServer.DEBUG_WARNING);
      }

      // send the calling plan bind list
      boolean result3 = true;
      if (keepSending && result &&
	  (id == CommonConstants.LIST_ALL ||
	   id == CommonConstants.CALLING_PLAN_BIND_ONLY)) {
	result3 = false;
	try {
	  result3 = listCallingPlanBind(sli, filter, new CallPlanBinding("", "", 0, 0, 0));
	  if (result3)
	    JServer.printDebug("CallingPlanServer: listed all calling plan binds successfully", JServer.DEBUG_VERBOSE);
	} catch (Exception e) {
	  JServer.printDebug("Error sending calling plan bind list:", JServer.DEBUG_ERROR);
	  JServer.printDebug(e, JServer.DEBUG_ERROR);
	  try {
	    sli.send(e);
	  } catch (IOException ie) {
	    // if the other side has closed the connection, we would get
	    // these messages
	    keepSending = false;
	    JServer.printDebug("Error in sending calling plan bind list error status:", JServer.DEBUG_WARNING);
	    JServer.printDebug(ie, JServer.DEBUG_WARNING);
	  }
	}
      }

      try {
	if (result3)
	  sli.flush();
      } catch (IOException ie) {
	JServer.printDebug(ie, JServer.DEBUG_WARNING);
      }

      // send the calling plan list
      boolean result1 = true;
      if (keepSending && result && result3 &&
	  (id == CommonConstants.LIST_ALL ||
	   id == CommonConstants.CALLING_PLAN_ONLY)) {
	result1 = false;
	try {
	  result1 = listCallingPlan(sli, new CallPlan("", "", 0));
	  if (result1)
	    JServer.printDebug("CallingPlanServer: listed all calling plans successfully", JServer.DEBUG_VERBOSE);
	} catch (Exception e) {
	  JServer.printDebug("Error sending calling plan list:", JServer.DEBUG_ERROR);
	  JServer.printDebug(e, JServer.DEBUG_ERROR);
	  try {
	    sli.send(e);
	  } catch (IOException ie) {
	    keepSending = false;
	    JServer.printDebug("Error in sending calling plan list error status:", JServer.DEBUG_WARNING);
	    JServer.printDebug(ie, JServer.DEBUG_WARNING);
	  }
	}
      }

      try {
	if (result1)
	  sli.flush();
      } catch (IOException ie) {
	JServer.printDebug(ie, JServer.DEBUG_WARNING);
      }

      // send the calling plan route list
      boolean result2 = true;
      if (keepSending && result && result3 && result1 &&
	  (id == CommonConstants.LIST_ALL ||
	   id == CommonConstants.CALLING_PLAN_ROUTE_ONLY)) {
	result2 = false;
	try {
	  result2 = listCallingPlanRoute(sli, new CallPlanRoute("", "", 0, "", 0, "", "", 0, 0));
	  if (result2)
	    JServer.printDebug("CallingPlanServer: listed all calling plan routes successfully", JServer.DEBUG_VERBOSE);
	} catch (Exception e) {
	  JServer.printDebug("Error sending calling plan route list:", JServer.DEBUG_ERROR);
	  JServer.printDebug(e, JServer.DEBUG_ERROR);
	  try {
	    sli.send(e);
	  } catch (IOException ie) {
	    keepSending = false;
	    JServer.printDebug("Error in sending calling plan route list error status:", JServer.DEBUG_WARNING);
	    JServer.printDebug(ie, JServer.DEBUG_WARNING);
	  }
	}
      }

      if (keepSending) {
	if (result && result1 && result2 && result3)
	  result = true;
	else
	  result = false;
	try {
	  JServer.printDebug("Sending calling plan list final result: " + result + "(" + result3 + ", " + result1 + ", " + result2 + ")", JServer.DEBUG_VERBOSE);
	  sli.send(new Boolean(result));
	} catch (IOException ie) {
	  JServer.printDebug("Error sending calling plan list final status (" + result + "):", JServer.DEBUG_ERROR);
	  JServer.printDebug(ie, JServer.DEBUG_ERROR);
	}
      }

      try {
	sli.flush();
      } catch (IOException ie) {
	JServer.printDebug(ie, JServer.DEBUG_WARNING);
      }

      try {
	sock.shutdownOutput();
      } catch (IOException ie) {
	JServer.printDebug("Error closing calling plan list output stream:", JServer.DEBUG_ERROR);
	JServer.printDebug(ie, JServer.DEBUG_ERROR);
      }
    }

  }


  // stops this server
  public void stop () {
    super.stopAndWait();
    JServer.printDebug("CallingPlanServer exited", JServer.DEBUG_NORMAL);
  }

}

