package com.nextone.JServer;

import java.util.*;
import java.io.*;
import java.net.*;
import com.nextone.common.Bridge;
import com.nextone.common.IEdgeCore;
import com.nextone.common.IEdgeList;
import com.nextone.common.MaintenanceGroup;
import com.nextone.common.MaintenanceRequest;
import com.nextone.common.Registration;
import com.nextone.common.VpnGroupList;
import com.nextone.common.VpnList;
import com.nextone.util.LimitedDataInputStream;
import com.nextone.util.LimitExceededException;
import com.nextone.util.QueueProcessor;


/**
 * class which handles auto-download process
 */
public class AutoDownload {
  private static Map downloadDevices;
  private static QueueProcessor qp;

  static {
    downloadDevices = Collections.synchronizedMap(new HashMap());
    qp = QueueProcessor.getInstance(new ProcessRegistration());
    qp.setLimit(Constants.MAX_AUTODOWNLOAD_REQUESTS);
    qp.start();
  }

  static native IEdgeList getIEdgeList (String serial);


  /**
   * re-initializes the QueueProcessor which processes the incoming 
   * registration, to use the specified ThreadGroup and thread name
   *
   * any processing which is happening currently will be aborted
   */
  public static void reinitQueueProcessor (ThreadGroup tg, String name) {
    if (qp != null)
      qp.stopRunning();
    qp = QueueProcessor.getInstance(new ProcessRegistration(), tg, name);
    qp.start();
  }

  /**
   * returns the number of auto-download tasks currently active
   */
  public static int getAutoDownloadCount () {
    return downloadDevices.size();
  }

  /**
   * returns if the the auto-download process is currently active for the
   * given regid
   */
  public static boolean isActive (String regid) {
    if (downloadDevices.containsKey(regid))
      return true;

    return false;
  }

  /**
   * aborts the currently active auto-download process for this regid
   */
  public synchronized static void abort (String regid) throws Exception {
    AutoDownloadTask adt = (AutoDownloadTask)downloadDevices.get(regid);
    if (adt != null)
      adt.cancel(false);
  }

  /**
   * called from JServer when a registration message is received from an
   * iedge
   */
  public static void receivedRegistration (LimitedDataInputStream dis, DatagramPacket dp, Bridge bs) {
    Registration reg = null;
    try {
      reg = new Registration(dp, dis, -1);
    } catch (IOException ie) {
      JServer.printDebug("Error processing startup registration from " + dp.getAddress().getHostAddress() + ":", JServer.DEBUG_WARNING);
      JServer.printDebug(ie, JServer.DEBUG_WARNING);
      return;
    }

    String regid = reg.getRegId();
    if (regid == null || regid.equals("")) {
      JServer.printDebug("Received startup registration from an iEdge (" + reg.getAddress().getHostAddress() + ") with no registration id", JServer.DEBUG_WARNING);
      return;
    }

    if (isActive(regid)) { // this dude is already active...
      JServer.printDebug("Received registration from " + regid + ", but auto-download is already active", JServer.DEBUG_VERBOSE);
      return;
    }

    // check if the scheduled maintenance is active for this guy
    String req = ((BridgeServer)bs).isRegidCurrentlyActive(regid);
    if (req != null) {
      JServer.printDebug("Will not initiate auto-download on " + regid + " - maintenance request " + req + " is currently working on (or) will soon work on this iEdge", JServer.DEBUG_WARNING);
      return;
    }

    // queue to the thread which will now process the auto-download of
    // this iedge
    //		 System.out.println(qp);
    try {
      qp.add(new RegData(bs, reg));
    } catch (LimitExceededException lee) {
      JServer.printDebug("Auto-download request from " + regid + " is more than I can handle, skipping (" + lee.getMessage() + ")", JServer.DEBUG_WARNING);
    }
  }

  public static void doneAutoDownload (String regid) {
    downloadDevices.remove(regid);
  }

  public static MaintenanceRequest getMaintenanceRequest (Bridge bs, String regid) {
    return AutoDownload.getMaintenanceRequest(bs, regid, false);
  }

  /**
   * returns the MaintenanceRequest configured for an auto-download group
   * which contains this regid, null otherwise
   * depending on the size of the database, etc. a call to this method
   * may be very time consuming
   */
  public static MaintenanceRequest getMaintenanceRequest (Bridge bs, String regid, boolean forceRefresh) {
    // first find the vpn id and vpn group of this iedge
    IEdgeList il = getIEdgeList(regid);
    if (il == null) {
      JServer.printDebug("AutoDownload: error getting info about " + regid, JServer.DEBUG_WARNING);
      return null;
    }

    AutoDownloadDataCache addc = AutoDownloadDataCache.getInstance(bs, forceRefresh);
    if (!addc.getStatus()) {
      JServer.printDebug("AutoDownload: " + addc.getErrorMessage() + "\n" + addc.getErrorDetails(), JServer.DEBUG_WARNING);
      return null;
    }

    return addc.getMaintenanceRequest(il);
  }

  // stop the thread associated with processing the auto-download thread
  public static void stop () {
    // stop the queue processor thread
    qp.stopRunning();

    // cancel all the currently running auto-download tasks
    Set s = downloadDevices.keySet();
    Iterator i = s.iterator();
    while (i.hasNext()) {
      AutoDownloadTask adt = (AutoDownloadTask)downloadDevices.get(i.next());
      if (adt != null)
	adt.cancel(true);
    }

    JServer.printDebug("AutoDownload stopped", JServer.DEBUG_NORMAL);
  }

  private static class RegData {
    Bridge bs;
    Registration reg;
    long receivedTime;

    RegData (Bridge bs, Registration reg) {
      this.bs = bs;
      this.reg = reg;
      this.receivedTime = System.currentTimeMillis();
    }
  }

  private static class ProcessRegistration implements QueueProcessor.DataProcessor{
    public void processData (Object data) {
      Bridge bs = ((RegData)data).bs;
      Registration reg = ((RegData)data).reg;
      long receivedTime = ((RegData)data).receivedTime;

      String regid = reg.getRegId();

      JServer.printDebug("processing startup registration from " + regid, JServer.DEBUG_VERBOSE);
      // retrieve configured request, if any
      MaintenanceRequest mr = null;
      try {
	mr = bs.getAutoDownloadConfig(regid, false);
      } catch (Exception e) {
	// if the configuration does not exist, stay silent
	if (e.getMessage().indexOf("does not exist") == -1) {
	  JServer.printDebug("Error starting auto-download for " + regid + ":", JServer.DEBUG_ERROR);
	  JServer.printDebug(e, JServer.DEBUG_ERROR);
	  return;
	} else {
	  JServer.printDebug("getting group configuration for startup registration from " + regid, JServer.DEBUG_VERBOSE);
	  // individual file does not exist, look for a group
	  // configuration
	  mr = AutoDownload.getMaintenanceRequest(bs, regid);
	  if (mr == null) { // nothing configured
	    JServer.printDebug("Nothing configured for auto-download for " + regid, JServer.DEBUG_VERBOSE);
	    return;
	  }
	}
      }

      AutoDownloadTask adt = new AutoDownloadTask(mr, reg, bs);
      downloadDevices.put(regid, adt);
      // if the edge is in ram mode, wait for 15 seconds before
      // initiating auto-download (in ram mode, iedge 5X0 ucon
      // task does not start responding for a while, as other higher
      // priority tasks are starting up)
      Date start = new Date(3000 + receivedTime);
      if (reg.getMode() == Constants.RAM_MODE)
	start = new Date(15000 + receivedTime);
      new Timer(true).schedule(adt, start);
    }

  }

}
