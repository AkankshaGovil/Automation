package com.nextone.JServer;

import java.util.*;
import java.net.*;
import java.io.*;
import com.nextone.common.Bridge;
import com.nextone.common.ConfigFile;
import com.nextone.common.IEdgeCore;
import com.nextone.common.IEdgeList;
import com.nextone.common.MaintenanceGroup;
import com.nextone.common.MaintenanceRequest;
import com.nextone.common.VpnGroupList;
import com.nextone.common.VpnList;
import com.nextone.util.SysUtil;


/**
 * for each maintenance request, one of these tasks will run
 */
public class DownloadTask extends TimerTask {
  protected MaintenanceRequest mr;
  protected Bridge bs;
  protected DownloadServer ds;
  protected boolean running, keepRunning;
  protected Thread thisThread;
  protected String request;
  protected boolean serverShuttingDown;
  protected int logFileType;
  protected Map edges;  // the edges this task is working on
  protected Map numEdges;
  // we only handle requests for 500 and 510 for now
  protected int [] devices = {Constants.DEVTYPE_I500, Constants.DEVTYPE_I510, Constants.DEVTYPE_I1000};

  protected DownloadTask (MaintenanceRequest mr, Bridge bs, int lt) {
    this.mr = mr;
    this.bs = bs;
    this.logFileType = lt;
    this.request = mr.getName();
  }

  /**
   * this is the constructor used from the DownloadServer to schedule
   * maintenance downloads
   *
   * @param mr the maintenance request to be executed
   * @param ds reference to the DownloadServer object, used to schedule
   * stopping this download task
   * @param bs reference to the bridge server to invoke storing/reading
   * maintenance requests
   */
  DownloadTask (MaintenanceRequest mr, DownloadServer ds, Bridge bs) {
    this(mr, bs, Constants.MAINTENANCE_LOGFILE);
    this.ds = ds;
  }

  public void run () {
    running = true;
    thisThread = Thread.currentThread();
    thisThread.setPriority(Thread.MIN_PRIORITY);

    LogTask.logMedium(request, "Starting the download actions", logFileType);

    if (!init()) {
      LogTask.logMedium(request, "Init failed, done with actions\n", logFileType);
      cleanup();
      return;
    }

    scheduleStopDownload();

    // execute the actions...
    actionLoop();

    // we could get here if
    //    a. actions are finished normally
    //    b. request is aborted
    //    c. server is shutting down
    LogTask.logMedium(request, "Done with the download actions\n", logFileType);

    cancelStopDownload();
    running = false;

    // do the final cleanup
    cleanup();

    JServer.printDebug("finished executing downloads for " + request, JServer.DEBUG_NORMAL);
  }

  /**
   * do any initialization here
   *
   * @return true if init is successful and can proceed, false otherwise
   */
  protected boolean init () {
    return true;
  }

  /**
   * schedules a task to stop this download task at a specified time
   */
  protected void scheduleStopDownload () {
    Calendar tmp = Calendar.getInstance();
    tmp.setTime(mr.getScheduleEnd());
    if (tmp.get(Calendar.YEAR) != 3000) {  // 3000 means until done...
      // schedule to stop this task 10 minutes before the maintenance
      // window finishes
      StopDownloadTask sdt = new StopDownloadTask(mr, ds);
      ds.getEndList().put(request, sdt);
      Date ed = SysUtil.substractFromDate(mr.getScheduleEnd(), Calendar.MINUTE, 10);
      new Timer(true).schedule(sdt, ed);
      JServer.printDebug("scheduled to stop " + request + " at " + ed, JServer.DEBUG_NORMAL);
    }
  }

  /**
   * cancels the task which has been scheduled to stop this download task
   */
  protected void cancelStopDownload () {
    // done with everything, cancel the StopDownloadTask
    Map m = ds.getEndList();
    if (m.containsKey(request)) {
      StopDownloadTask sdt = (StopDownloadTask)m.get(request);
      sdt.cancel();
      m.remove(request);
    }
    ds.getStartList().remove(request);
  }

  /**
   * do any final cleanup acts before this download task ends
   */
  protected void cleanup () {
    // if there was a SAVE_CONFIG_ACTION scheduled, cleanup any ftp connections
    // we may have opened up
    int [] actions = mr.getActions();
    for (int i = 0; actions != null && i < actions.length; i++) {
      for (int j = 0; actions[i] == MaintenanceRequest.SAVE_CONFIG_ACTION && j < devices.length; j++) {
	// if there are no edges of this type, we would not have opened a connection
	if (getEdgeCount(devices[j]) == 0)
	  continue;

	try {
	  // passing null will close any open ftp connections
	  mr.saveConfig(devices[j], null, null);
	} catch (IOException ie) {
	  LogTask.logError(request, "Error closing FTP connection to the server :" + ie.getMessage(), logFileType);
	}
      }
    }

    // nothing to do if we are exiting because of a server shutdown
    if (serverShuttingDown)
      return;

    // reset the to be scheduled flag, so that this won't get run
    // automatically next time the iserver restarts (do this only
    // is the request was cancelled or it finished normally)
    try {
      MaintenanceRequest mreq = bs.getMaintenanceRequest(request);
      mreq.setToBeScheduled(false);
      bs.storeMaintenanceRequest(mreq);
    } catch (Exception e) {
      JServer.printDebug("Error marking request to not run in the future:", JServer.DEBUG_WARNING);
      JServer.printDebug(e, JServer.DEBUG_WARNING);
    }
  }

  public boolean cancel (boolean serverStatus) {
    if (running) {
      serverShuttingDown = serverStatus;
      if (serverShuttingDown)
	LogTask.logWarning(request, "Aborting the request, server is shutting down\nRequests will be executed, if eligible, once the server starts back up again", logFileType);
      else
	LogTask.logWarning(request, "Actions may not be completed, request is being canceled", logFileType);
      keepRunning = false;
      thisThread.interrupt();
      try {
	thisThread.join();
      } catch (InterruptedException ie) {}
    }

    return super.cancel();
  }

  public synchronized boolean isRunning () {
    return running;
  }

  private void actionLoop () {
    keepRunning = true;

    // list the iedges to be contacted
    createEdgeList();
    if (edges.isEmpty()) {
      LogTask.logVerbose(request, "No iEdges to process for this request", logFileType);
      return;
    }

    // retrieve the file versions
    LogTask.logMedium(request, "Checking download file locations...", logFileType);
    int count = 0;
    while (keepRunning && count++ < 10) {
      if (retrieveVersions())
	break;
      else {
	try {
	  if (keepRunning) {
	    LogTask.logVerbose(request, "Will try again after 2 minutes", logFileType);
	    Thread.sleep(120000);
	  }
	} catch (InterruptedException ie) {}
	continue;
      }
    }
    if (count >= 10) {
      LogTask.logError(request, "Error accessing file download locations (even tried " + (count-1) + " times!), giving up", logFileType);
      return;
    }

    if (!keepRunning)
      return;

    // validate the config file
    int [] actions = mr.getActions();
    boolean doConfigAction = false;
    for (int i = 0; actions != null && i < actions.length; i++) {
      if (actions[i] == MaintenanceRequest.DOWNLOAD_CONFIG_ACTION) {
	doConfigAction = true;
	break;
      }
    }
    count = 0;
    while (doConfigAction && keepRunning && count++ < 10) {
      if (validateConfigs())
	break;
      else {
	try {
	  if (keepRunning) {
	    LogTask.logVerbose(request, "Will try again after 5 minutes", logFileType);
	    Thread.sleep(300000);
	  }
	} catch (InterruptedException ie) {}
	continue;
      }
    }
    if (count >= 10) {
      LogTask.logError(request, "Error validating config file (even tried " + (count-1) + " times!), giving up", logFileType);
      return;
    }

    // do the actions on all iedges
    while (keepRunning) {
      boolean done = true;

      Set ks = edges.keySet();
      Iterator i = ks.iterator();
      long sleepTime = 60*60*1000;  // start with an hour
      while (i.hasNext()) {
	ActionState as = (ActionState)edges.get(i.next());

	long cur = System.currentTimeMillis() - as.getActionInitiatedTime();
	switch (as.getState()) {
	case ActionState.INIT:
	  // check if auto download is happening on this guy before
	  // doing anything on this guy
	  if (logFileType != Constants.AUTODOWNLOAD_LOGFILE &&
	      AutoDownload.isActive(as.getRegId())) {
	    LogTask.logWarning(request, "iEdge " + as.getRegId() + " is auto-downloading, will try again after 5 minutes", logFileType);
	    sleepTime = Math.min(sleepTime, 300000);
	  } else {
	    if (cur >= as.getActionWaitTime())
	      as.validateInetAddress(mr);

	    if (as.getAddrResolveTries() >=
		ActionState.MAX_ADDR_RESOLVE_TRIES) {
	      LogTask.logWarning(request, "Could not determine IP address of " + as.getRegId() + ", giving up", logFileType);
	      as.setDone();
	      sleepTime = 0;
	    } else {
	      cur = System.currentTimeMillis() - as.getActionInitiatedTime();
	      sleepTime = Math.min(sleepTime,
				   as.getActionWaitTime() - cur);
	    }
	  }
	  done = false;
	  break;

	case ActionState.ADDR_RESOLVED:
	  LogTask.logMedium(request, "Address of " + as.getRegId() + " is " + as.getAddress().getHostAddress(), logFileType);
	  as.doActions(mr);

	  cur = System.currentTimeMillis() - as.getActionInitiatedTime();
	  sleepTime = Math.min(sleepTime,
			       as.getActionWaitTime() - cur);
	  done = false;
	  break;

	case ActionState.ACTION_INITIATED:
	  // action is already initiated... wait a few minutes
	  // before finding the status of it
	  if (cur >= as.getActionWaitTime() )
	    as.doActions(mr);  // this might set the task done...

	  cur = System.currentTimeMillis() - as.getActionInitiatedTime();
	  sleepTime = Math.min(sleepTime,
			       as.getActionWaitTime() - cur);
	  done = false;
	  break;

	case ActionState.DONE:
	  // maybe we should remove the action state from the list
	  break;

	default:
	  LogTask.logError(request, as.getRegId() + " in unknown state - " + as.getState(), logFileType);
	  done = false;
	}

      }

      if (keepRunning == true)
	keepRunning = (!done);

      //			System.out.println("sleeping time is " + sleepTime);
      if (keepRunning && sleepTime > 0) {
	//			   System.out.println("download task sleeping for " + sleepTime);
	// sleep here if needed
	try {
	  Thread.sleep(sleepTime);
	} catch (InterruptedException ie) {}
      }

    }
  }

  /**
   * create the list of edges to operate on and stores in "edges"
   * (also calculates the number of iedges in each device type and
   * stores it in "numEdges")
   */
  protected synchronized void createEdgeList ()   {
    edges = new HashMap();

    String [] groups = mr.getGroups();
    if (groups == null)
      return;

    // contact the list server to get the vpn/iedge lists
    LogTask.logVerbose(request, "Contacting the list server to get vpn/iedge lists", logFileType);
    DataCache dc = null;
    try {
      dc = DataCache.getInstance(bs.getListPort(Constants.IEDGE_LIST));
    } catch (Exception e) {
      LogTask.logError(request, "Error recieving vpn/iedge list:\n" + e, logFileType);
      return;
    }

    if (dc.getStatus() == true) {
      LogTask.logVerbose(request, "Successfully retrieved vpn/iedge list", logFileType);
    } else {
      LogTask.logWarning(request, "Error while retrieving vpn/iedge list:\n" + dc.getErrorMessage() + "\n" + dc.getErrorDetails(), logFileType);
    }

    Vector vpnnameList = dc.getVpnNameList();
    Vector iedgeList = dc.getIEdgeList();
    if ((vpnnameList == null || vpnnameList.isEmpty()) &&
	(iedgeList == null || iedgeList.isEmpty())) {
      LogTask.logWarning(request, "vpn/iedge list retrieved were empty", logFileType);
      return;
    }

    // make a map which is keyed on vpn groups, and gives a Vector of
    // all the vpn ids in it
    HashMap idmap = new HashMap();

    if (vpnnameList != null) {
      Enumeration e = vpnnameList.elements();
      while (e.hasMoreElements()) {
	VpnList vl = (VpnList)e.nextElement();
	Vector v = (Vector)idmap.get(vl.getVpnGroup());
	if (v == null) {
	  v = new Vector();
	  idmap.put(vl.getVpnGroup(), v);
	}
	v.add(vl.getVpnName());
      }
    }

    // make a map which is keyed on vpn ids, and gives a Vector of
    // all the iedges in it
    // make a map which is keyed on regid and gives an IEdgeList object
    // for it
    HashMap edmap = new HashMap();
    HashMap ilmap = new HashMap();
    if (iedgeList != null) {
      Enumeration e = iedgeList.elements();
      while (e.hasMoreElements()) {
	IEdgeList il = (IEdgeList)e.nextElement();

	ilmap.put(il.getSerialNumber(), il);

	Vector v = (Vector)edmap.get(il.getVpnName());
	if (v == null) {
	  v = new Vector();
	  edmap.put(il.getVpnName(), v);
	}
	int dt = il.getDeviceType();
	if (dt == Constants.DEVTYPE_I500 ||
	    dt == Constants.DEVTYPE_I510 ||
	    dt == Constants.DEVTYPE_I1000)
	  v.add(il);
      }
    }

    TreeSet edt = new TreeSet(new Comparator() {
	public int compare (Object o1, Object o2) {
	  return ((IEdgeList)o1).getSerialNumber().compareTo(((IEdgeList)o2).getSerialNumber());
	}
      });

    // for each maintenance group in this request, extract the
    // iedge information
    for (int j = 0; j < groups.length; j++) {
      MaintenanceGroup mg = null;
      try {
	LogTask.logVerbose(request, "Reading maintenance group: " + groups[j], logFileType);
	mg = bs.getMaintenanceGroup(groups[j]);
      } catch (Exception e) {
	LogTask.logError(request, "Error reading maintenance group information:\n" + e, logFileType);
	continue;
      }

      Vector idv = new Vector();

      // handle all the vpn groups listed
      VpnGroupList [] vgl = mg.getVpnGroups();
      if (vgl != null) {
	for (int i = 0; i < vgl.length; i++) {
	  // list all the ids in the group
	  Vector v = (Vector)idmap.get(vgl[i].getVpnGroup());
	  if (v != null)
	    idv.addAll(v);
	}
      }

      // handle all the vpn ids listed

      VpnList [] vl = mg.getVpnNames();
      if (vl != null) {
	for (int i = 0; i < vl.length; i++)
	  idv.add(vl[i].getVpnName());
      }
      Enumeration e = idv.elements();
      while (e.hasMoreElements()) {
	// list all the edges in this id
	Vector v = (Vector)edmap.get(e.nextElement());
	if (v != null)
	  edt.addAll(v);
      }

      // handle all the iedges listed
      IEdgeCore [] ic = mg.getiEdgeCores();
      if (ic != null) {
	for (int i = 0; i < ic.length; i++) {
	  IEdgeList il = (IEdgeList)ilmap.get(ic[i].getRegId());
	  if (il != null)
	    edt.add(il);
	}
      }
    }

    // for all the regids in the list, create a new object
    Iterator i = edt.iterator();
    StringBuffer sb = new StringBuffer();
    while (i.hasNext()) {
      IEdgeList il = (IEdgeList)i.next();
      sb.append("\n" + il.getSerialNumber());
    }
    LogTask.logMedium(request, "Eligible iEdges in this request are:" + sb.toString(), logFileType);
    i = edt.iterator();
    while (i.hasNext()) {
      IEdgeList il = (IEdgeList)i.next();
      try {
	ActionState as = new ActionState(il.getSerialNumber(), logFileType);
	edges.put(il.getSerialNumber(), as);
	updateEdgeCount(il.getDeviceType());
	LogTask.logMedium(request, "Starting maintenance actions on " + il.getSerialNumber(), logFileType);
      } catch (IOException ie) {
	LogTask.logError(request, "Error forming ActionState:\n" + ie, logFileType);
      }
    }
  }

  // updates the device count for each device type
  protected void updateEdgeCount (int dtype) {
    if (numEdges == null)
      numEdges = new HashMap();

    Integer key = new Integer(dtype);
    Integer old = (Integer)numEdges.get(key);
    if (old == null)
      old = new Integer(0);

    numEdges.put(key, new Integer(old.intValue() + 1));
  }

  protected int getEdgeCount (int dtype) {
    int numDevices = 0;
    if (numEdges != null) {
      Integer ne = (Integer)numEdges.get(new Integer(dtype));
      if (ne != null)
	numDevices = ne.intValue();
    }
        
    return numDevices;
  }

  // retrieves the file versions in this request
  private boolean retrieveVersions () {
    int [] actions = mr.getActions();

    for (int i = 0; actions != null && i < actions.length; i++) {
      for (int j = 0; j < devices.length; j++) {
	// if there are no edges of this type, don't bother
	// retrieving the version information
	if (getEdgeCount(devices[j]) == 0)
	  continue;
	try {

	  if (mr.getVersion(actions[i], devices[j]) == null)
	    return false;  // version was null...
	} catch (IOException ie) {
	  LogTask.logError(request, "Error determining configured file version for " + ConfigFile.getDeviceTypeString(devices[j]) + "/" + MaintenanceRequest.getActionString(actions[i]) + ": " + ie.getMessage(), logFileType);
	  return false;
	}
      }
    }

    return true;
  }

  // validate all the config files in this request
  private boolean validateConfigs () {
    for (int i = 0; i < devices.length; i++) {
      // no edges of this device type in the list..
      if (getEdgeCount(devices[i]) == 0)
	continue;

      LogTask.logMedium(request, "Validating configuration file for " + ConfigFile.getDeviceTypeString(devices[i]) + "...", logFileType);

      // validate the config file type with the device type
      try {
	int fileDevType = ConfigFile.getConfigDeviceType(mr.getConfig(devices[i]));
	if (fileDevType != Constants.DEVTYPE_ANY) {
	  if (fileDevType == Constants.DEVTYPE_UNKNOWN)
	    LogTask.logWarning(request, "Configuration file is for an unknown type, destination is an " + ConfigFile.getDeviceTypeString(devices[i]), logFileType);
	  else if (fileDevType != devices[i])
	    LogTask.logWarning(request, "Configuration file is for an " + ConfigFile.getDeviceTypeString(fileDevType) + ", destination is an " + ConfigFile.getDeviceTypeString(devices[i]), logFileType);
	}
	mr.setConfig(devices[i], validateConfigFile(devices[i]));
      } catch (Exception e) {
	LogTask.logError(request, "Error validating config file: " + e.getMessage(), logFileType);
	return false;
      }
    }
    return true;
  }

  /**
   * validate the commands contained in the config file
   * this method filters out commands pertaining to an unique iedge (such
   * as ip address, registration id, etc)
   * this method also appends a "program" command at the end
   *
   * @param dtype the device type this config is meant for
   * @return the filtered command file
   * @exception IOException if error reading the file
   */
  protected String validateConfigFile (int dtype) throws IOException {
    BufferedReader br = new BufferedReader(new StringReader(mr.getConfig(dtype)));
    String line = null;
    StringBuffer sb = new StringBuffer();
    boolean programSeen = false, byeSeen = false, multipleByes = false;
    StringTokenizer st;

    while ((line = br.readLine()) != null) {

      if (line.startsWith("#")) {
	sb.append(line);
	sb.append("\n");
      } else if (line.equals("bye")) {
	if (byeSeen)
	  multipleByes = true;
	byeSeen = true;
      } else if (line.equals("program"))
	programSeen = true;
      else if (line.startsWith("set ") || line.startsWith("erase ")) {
	st = new StringTokenizer(line);
	if (st.countTokens() < 2)
	  LogTask.logWarning(request, "Ignoring invalid command (" + line + ")", logFileType);
	else {
	  //				  st.nextToken();
	  //				  String cmd = st.nextToken();
	  try {
	    if (ConfigFile.isUniqueCommand(line,dtype) && (getEdgeCount(dtype) > 1))
	      LogTask.logWarning(request, "Ignoring non-unique command (" + line + ")", logFileType);
	    else {
	      sb.append(line);
	      sb.append("\n");
	    }
	  } catch (IllegalArgumentException iae) {
	    LogTask.logWarning(request, "Command (" + line + " not recognized by this version of jServer, proceeding anyway", logFileType);
	    sb.append(line);
	    sb.append("\n");
	  }
	}
      } else {
	LogTask.logWarning(request, "Ignoring invalid command (" + line + ")", logFileType);
      }
    }

    // dish out some warnings if we saw some conditions we didn't like
    if (!programSeen)
      LogTask.logWarning(request, "Configuration file did not contain a \"program\" command, adding one", logFileType);
    if (!byeSeen)
      LogTask.logWarning(request, "Configuration file did not contain a \"bye\" command, adding one", logFileType);
    if (multipleByes)
      LogTask.logWarning(request, "Configuration file contained too many \"bye\" commands, pruning", logFileType);

    // add a program and a bye
    sb.append("program\nbye\n");

    String str = sb.toString();
    LogTask.logVerbose(request, "Validated configuration file:\n" + str, logFileType);
    return str;
  }


  public String toString () {
    return "Execute " + mr + " at " + new Date(scheduledExecutionTime());
  }

  /**
   * returns if the given regid is among the list of iedges this
   * task is going to work on
   *
   * @return the name of the maintenance request or null if this regid is
   * not a part of this
   */
  public synchronized String contains (String regid) {
    if (edges == null)
      createEdgeList();

    if (edges.containsKey(regid))
      return new String(mr.getName());

    return null;
  }

}
