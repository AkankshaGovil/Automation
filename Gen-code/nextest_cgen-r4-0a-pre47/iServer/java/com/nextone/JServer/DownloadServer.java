package com.nextone.JServer;

import java.util.*;
import java.net.*;
import com.nextone.common.BridgeException;
import com.nextone.common.MaintenanceRequest;
import com.nextone.common.*;
import com.nextone.util.SysUtil;

/**
 * this class creates a download task for each maintenance request eligible
 * for runnning
 */
public class DownloadServer {
  private BridgeServer bs;
  private Map tobeStarted, tobeEnded;

  DownloadServer (ThreadGroup tg, BridgeServer bs) {
    this.bs = bs;

    tobeStarted = Collections.synchronizedMap(new HashMap());
    tobeEnded = Collections.synchronizedMap(new HashMap());
    initRequests();
    AutoDownload.reinitQueueProcessor(tg, "RegistrationProcessor");
  }


  private void initRequests () {
    String [] names = null;
    try {
      names = bs.getMaintenanceRequestNames();
    } catch (Exception e) {
      JServer.printDebug("Error reading maintenance request files:", JServer.DEBUG_ERROR);
      JServer.printDebug(e, JServer.DEBUG_ERROR);
      return;
    }

    if (names != null) {
      for (int i = 0; i < names.length; i++) {
	try {
	  initRequest(names[i]);
	} catch (Exception e) {
	  JServer.printDebug("Error reading maintenance request " + names[i] + ":", JServer.DEBUG_ERROR);
	  JServer.printDebug(e, JServer.DEBUG_ERROR);
	}
      }
    }
  }

  public void initRequest (String request) throws Exception {
    MaintenanceRequest mr = bs.getMaintenanceRequest(request);
    if (!mr.toBeScheduled())
      return;

    DTServer	[]testDT	=	mr.getServers();
    for(int i=0; i < testDT.length; i++){

      DTServer temp	=	testDT[i];
    }

    if (isMaintenanceRequestActive(request))
      throw new BridgeException("The request is currently active, please try scheduling after it is done");

    if (tobeStarted.containsKey(request)) {
      DownloadTask dt = (DownloadTask)tobeStarted.get(request);
      dt.cancel();
    }

    // we start shutting down the download threads 10 minutes before 
    // the scheduled end time, don't let a new task start in between
    if (tobeEnded.containsKey(request)) {
      Date in10Minutes = SysUtil.addToDate(new Date(), Calendar.MINUTE, 10);
      if (mr.getScheduleStart() != null &&
	  mr.getScheduleStart().before(in10Minutes))
	throw new BridgeException("A previous scheduled request is running to completion.\nPlease wait another 10 minutes before retrying.");
    }

    Date end = mr.getScheduleEnd();
    if (end != null && end.after(new Date())) {
      DownloadTask dt = new DownloadTask(mr, this, bs);
      tobeStarted.put(request, dt);
      new Timer(true).schedule(dt, mr.getScheduleStart());
      JServer.printDebug("scheduled " + request + " to start at " + mr.getScheduleStart(), JServer.DEBUG_NORMAL);
    }

  }

  /* is the maintenance request currently actively being worked on? */
  public boolean isMaintenanceRequestActive (String request) throws Exception {
    DownloadTask dt = (DownloadTask)tobeStarted.get(request);
    if (dt != null)
      return dt.isRunning();

    return false;
  }

  /* abort the currently active maintenance request */
  public void abortMaintenanceRequest (String request) throws Exception {
    if (!isMaintenanceRequestActive(request))
      return;

    StopDownloadTask sdt = (StopDownloadTask)tobeEnded.get(request);
    if (sdt != null)
      sdt.cancel();  // cancel the currently scheduled one
    sdt = new StopDownloadTask(bs.getMaintenanceRequest(request), this);
    tobeEnded.put(request, sdt);
    new Timer(true).schedule(sdt, new Date());  // schedule a new one
  }

  public Map getStartList () {
    return tobeStarted;
  }

  public Map getEndList () {
    return tobeEnded;
  }

  // shutdown threads...
  public void stop () {
    Set s = tobeStarted.keySet();
    Iterator i = s.iterator();
    while (i.hasNext())
      ((DownloadTask)tobeStarted.get(i.next())).cancel(true);
    s = tobeEnded.keySet();
    i = s.iterator();
    while (i.hasNext())
      ((StopDownloadTask)tobeEnded.get(i.next())).cancel();

    // stop the auto-download thread
    AutoDownload.stop();

    JServer.printDebug("DownloadServer exited", JServer.DEBUG_NORMAL);
  }

  /**
   * returns if the given regid is part of a currently active DownloadTask
   *
   * @return the name of the maintenance request or null if this regid is
   * not a part of any scheduled requests
   */
  public String isRegidCurrentlyActive (String regid) {
    Set s = tobeStarted.keySet();
    Iterator i = s.iterator();
    while (i.hasNext()) {
      DownloadTask dt = (DownloadTask)tobeStarted.get(i.next());
      // if this is currently running or scheduled to run within
      // another 10 minutes, we check to see if the given regid
      // is a part of this task
      if (dt.isRunning() ||
	  (dt.scheduledExecutionTime() - System.currentTimeMillis()) < 10*60*1000) {
	String m = dt.contains(regid);
	if (m != null)
	  return m;
      }
    }

    return null;
  }

}
