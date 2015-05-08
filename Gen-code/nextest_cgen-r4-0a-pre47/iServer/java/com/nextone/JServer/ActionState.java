package com.nextone.JServer;

import java.io.*;
import java.net.*;
import java.util.*;
import com.nextone.common.ConfigFile;
import com.nextone.common.ConfigRetriever;
import com.nextone.common.DataConsumer;
import com.nextone.common.Hello;
import com.nextone.common.LimitedCommandComm;
import com.nextone.common.MaintenanceRequest;
import com.nextone.common.Registration;
import com.nextone.common.TimeoutProvider;

/**
 * this class stores the state of an iedge during the execution of
 * a maintenance request
 */

public class ActionState implements ConfigRetriever, TimeoutProvider {
  private String regid;
  private int state;
  private int curAction;
  private InetAddress addr;
  private int numAddrResolves = 0;
  private long actionInitTime, actionWaitTime;
  private boolean [] forceStates;  // yuk, maintain some states in case the force flag is turned on (so that we don't keep downloading things)
  private boolean [] doneStates;
  private int logFileType;

  public static final int MAX_ADDR_RESOLVE_TRIES = 4;

  public static final int INIT = 0;
  public static final int ADDR_RESOLVED = 1;
  public static final int ACTION_INITIATED = 2;
  public static final int DONE = 3;

  ActionState (String r, int lt) throws IOException {
    this.regid = r;
    this.state = INIT;
    this.curAction = -1;
    this.logFileType = lt;

    forceStates = new boolean [MaintenanceRequest.MAX_ACTIONS];
    for (int i = 0; i < forceStates.length; i++) {
      if (i == MaintenanceRequest.DOWNLOAD_CONFIG_ACTION)
	forceStates[i] = false;
      else
	forceStates[i] = true;  // needs download
    }

    doneStates = new boolean [MaintenanceRequest.MAX_ACTIONS];
    for (int i = 0; i < doneStates.length; i++)
      doneStates[i] = false;
  }	  
	  
  public synchronized String getRegId () {
    return regid;
  }

  public synchronized int getState () {
    return state;
  }

  public synchronized int getCurrentAction () {
    return curAction;
  }

  public synchronized InetAddress getAddress () {
    return addr;
  }

  public synchronized int getAddrResolveTries () {
    return numAddrResolves;
  }

  public void validateInetAddress (MaintenanceRequest mr) {
    actionInitTime = System.currentTimeMillis();
    numAddrResolves++;
    actionWaitTime = numAddrResolves*60*2000;  // wait for this long, before retrying this again
    try {
      // contact the iedge
      Registration r = getRegistration();
      //			r.dump();
    } catch (InterruptedIOException iie) {
      LogTask.logError(mr.getName(), "iEdge " + regid + " not responding", logFileType);
      return;
    } catch (IOException ie) {
      LogTask.logError(mr.getName(), "Error validating IP address of " + regid + ": " + ie.getMessage(), logFileType);
      LogTask.logVerbose(mr.getName(), "Next attempt would be after " + 2*numAddrResolves + " minutes", logFileType);
      return;
    }

    actionWaitTime = 0;
    numAddrResolves = 0;
    state = ADDR_RESOLVED;
  }

  public void doActions (MaintenanceRequest mr) {

    state = ACTION_INITIATED;
    String request = mr.getName();

    actionInitTime = System.currentTimeMillis();
    actionWaitTime = 60*1000;  // 1 minute
		

    // cycle through all actions until everything is done
    int [] actions = mr.getActions();
    for (int i = 0; actions != null && i < actions.length; i++) {
      if (doneStates[actions[i]] == true)
	continue;

      LogTask.logVerbose(request, "evaluating task: " + MaintenanceRequest.getActionString(actions[i]) + " for " + regid, logFileType);
      // revalidate the ip address of the iedge, retrieve a registration message
      LogTask.logVerbose(request, "re-validating IP address for " + regid + "...", logFileType);
      Registration reg = null;
      try {
	reg = getRegistration();
      } catch (InterruptedIOException iie) {
	LogTask.logWarning(request, "iEdge " + regid + " not responding, will try after 5 minutes", logFileType);
	// iedge maybe down, try after a while...
	actionInitTime = System.currentTimeMillis();
	actionWaitTime = 5*60*1000;  // 5 minutes
      } catch (IOException ie) {
	LogTask.logError(request, "iEdge " + regid + " registration error:\n" + ie, logFileType);
	// some other error, try again in a bit
	actionInitTime = System.currentTimeMillis();
	actionWaitTime = 60*1000;  // 1 minute
      }
      if (reg == null){
	return;
      }
      LogTask.logVerbose(request, "IP address of " + regid + " is " + addr.getHostAddress(), logFileType);

      switch (actions[i]) {
      case MaintenanceRequest.DOWNLOAD_SOFTROM_ACTION:
	if (processSoftromDownload(mr, reg))
	  continue;
	return;

      case MaintenanceRequest.DOWNLOAD_RAM_ACTION:
	if (processRamDownload(mr, reg))
	  continue;
	return;

      case MaintenanceRequest.DOWNLOAD_CONFIG_ACTION:
	if (processCfgDownload(mr, reg))
	  continue;
	return;

      case MaintenanceRequest.SAVE_CONFIG_ACTION:
	if (processSaveCfg(mr, reg))
	  continue;
	return;

      default:
	LogTask.logError(request, "task " + actions[i] + " for " + regid + " is unknown", logFileType);
	continue;
      }
    }

    LogTask.logMedium(request, "Done with all the actions for " + regid, logFileType);
    setDone();
  }

  public synchronized long getActionInitiatedTime () {
    return actionInitTime;
  }

  public synchronized long getActionWaitTime () {
    return actionWaitTime;
  }

  public synchronized void setDone () {
    state = DONE;
  }

  private void resolveIPAddress () throws IOException {

    try{
      String addrStr = AutoDownload.getIEdgeList(regid).getAddress();
      if (addrStr == null || addrStr.equals("")) {
	addr = null;
	throw new IOException("IP address unknown");
      }
      addr = InetAddress.getByName(addrStr);
    }catch(Exception ie){
      throw new IOException("IP address unknown");
    }

  }

  private Registration getRegistration () throws IOException {
    resolveIPAddress();

    byte [] d = Hello.createRemoteHelloData(null, null, Constants.HELLO_VERSION_NO_AUTH);
    DatagramPacket dpo = new DatagramPacket(d, d.length, addr, Constants.MCAST_SEND_PORT);

    byte [] in = new byte [512];
    DatagramPacket dpi = new DatagramPacket(in, in.length);

    DatagramSocket ds = new DatagramSocket();
    ds.setSoTimeout(Constants.GET_TIMEOUT*1000);
    ds.send(dpo);
    ds.receive(dpi);

    return new Registration(dpi);
  }

  // compares two version strings, takes out any '\0' characters
  // before comparison
  private boolean versionEquals (String ver1, String ver2) {

    if (ver1 == null)
      return false;
    int index = ver1.indexOf('\0');
    if (index != -1)
      ver1 = ver1.substring(0, index);
    if (ver2 != null) {
      index = ver2.indexOf('\0');
      if (index != -1)
	ver2 = ver2.substring(0, index);
    }
    return ver1.equals(ver2);
  }

  // do the softrom download action
  // returns true if the action is done, and need to proceed to
  // the next action, false if keep attempting this action
  private boolean processSoftromDownload (MaintenanceRequest mr, Registration reg) {
    String request = mr.getName();
    boolean force = mr.isForced(MaintenanceRequest.DOWNLOAD_SOFTROM_ACTION);
    int dtype = ConfigFile.getDeviceTypeForId(reg.getDeviceId());
    if (force)
      LogTask.logVerbose(request, "Maintenance software download action is forced", logFileType);

    String configuredVersion = null;
    try {
      configuredVersion = mr.getVersion(MaintenanceRequest.DOWNLOAD_SOFTROM_ACTION, dtype);
    } catch (IOException ie) {
      LogTask.logError(request, "Error determining configured maintenance s/w version: " + ie.getMessage(), logFileType);
      actionWaitTime = 4*60*1000;
      return false;  // try again later
    }

    if ((force && forceStates[MaintenanceRequest.DOWNLOAD_SOFTROM_ACTION]) ||
	!versionEquals(configuredVersion,reg.getSoftromVersion())) {
      // download new softrom to this dude
      LogTask.logVerbose(request, "File version: " + configuredVersion + " iEdge version: " + reg.getSoftromVersion(), logFileType);
      LogTask.logMedium(request, "Downloading maintenance s/w on " + regid, logFileType);
      // if the dude is not in maintenance mode, reboot him in
      // maintenance mode
      if (reg.getDeviceId() != Constants.DEVICE_ID_1000 &&
	  reg.getMode() != Constants.ROM_MODE) {
	LogTask.logVerbose(request, "Rebooting " + regid + " in maintenance mode", logFileType);
	try {
	  Hello.reboot(addr, Constants.REBOOT_ROM);
	} catch (IOException ie) {
	  LogTask.logWarning(request, "Error sending the reboot request to " + regid + ":\n" + ie.getMessage(), logFileType);
	}
	actionWaitTime = 30*1000;  // 30 seconds
	return false;
      }

      // in rom mode, initiate download
      LogTask.logVerbose(request, "Initiating maintenance s/w download on " + regid, logFileType);
      actionWaitTime = 2*60*1000;  // 2 minutes

      // first send the download parameters
      try {
	Hello.sendDownloadParams(addr, Constants.ROM_MODE, Constants.SET_TIMEOUT, mr.getServer(MaintenanceRequest.DOWNLOAD_SOFTROM_ACTION), dtype);
      } catch (IOException ie) {
	LogTask.logWarning(request, "Error sending download parameters: " + ie.getMessage(), logFileType);
	return false;
      }

      // initiate download
      try {
	Hello.initiateDownload(addr, Constants.ROM_MODE, Constants.GET_TIMEOUT);
      } catch (IOException ie) {
	LogTask.logWarning(request, "Error sending initiate download request to " + regid + ":\n" + ie.getMessage(), logFileType);
      }
			
      // mark it done
      forceStates[MaintenanceRequest.DOWNLOAD_SOFTROM_ACTION] = false;

      return false;
    } else {
      LogTask.logVerbose(request, "Nothing to be done on " + regid + " for task " + MaintenanceRequest.getActionString(MaintenanceRequest.DOWNLOAD_SOFTROM_ACTION), logFileType);
    }

    doneStates[MaintenanceRequest.DOWNLOAD_SOFTROM_ACTION] = true;
    actionWaitTime = 0;
    return true;
  }

  // do the ram download action
  // returns true if the action is done, and need to proceed to
  // the next action, false if keep attempting this action
  private boolean processRamDownload (MaintenanceRequest mr, Registration reg) {

    String request = mr.getName();
		 
    boolean force = mr.isForced(MaintenanceRequest.DOWNLOAD_RAM_ACTION);
    int dtype = ConfigFile.getDeviceTypeForId(reg.getDeviceId());
    if (force)
      LogTask.logVerbose(request, "Operational software download action is forced", logFileType);

    String configuredVersion = null;
    try {
      configuredVersion = mr.getVersion(MaintenanceRequest.DOWNLOAD_RAM_ACTION, dtype);
	
    } catch (IOException ie) {
      LogTask.logError(request, "Error determining configured runtime s/w version: " + ie.getMessage(), logFileType);
      actionWaitTime = 4*60*1000;
      return false;  // try again later
    }

	 

    if (((reg.getMode() == Constants.ROM_MODE) && (reg.getDeviceId() != Constants.DEVICE_ID_1000))||
	(force && forceStates[MaintenanceRequest.DOWNLOAD_RAM_ACTION]) ||
	!versionEquals(configuredVersion,reg.getRamVersion())) {
      // download new ram to this dude
      LogTask.logVerbose(request, "File version: " + configuredVersion + " iEdge version: " + reg.getRamVersion(), logFileType);
      LogTask.logMedium(request, "Downloading runtime s/w on " + regid, logFileType);
      // if the dude is not in maintenance mode, reboot him in
      // maintenance mode
      if (reg.getDeviceId() != Constants.DEVICE_ID_1000 &&		reg.getMode() != Constants.ROM_MODE) {
	LogTask.logVerbose(request, "Rebooting " + regid + " in maintenance mode", logFileType);
	try {
		   
	  Hello.reboot(addr, Constants.REBOOT_ROM);
					
	} catch (IOException ie) {
	  LogTask.logWarning(request, "Error sending the reboot request to " + regid + ":\n" + ie.getMessage(), logFileType);
	}
	actionWaitTime = 30*1000;  // 30 seconds
	return false;
      }

      // in rom mode, initiate download
      LogTask.logVerbose(request, "Initiating runtime s/w download on " + regid, logFileType);
      actionWaitTime = 3*60*1000;  // 3 minutes

      // first send the download parameters
      try {
	Hello.sendDownloadParams(addr, Constants.RAM_MODE, Constants.SET_TIMEOUT, mr.getServer(MaintenanceRequest.DOWNLOAD_RAM_ACTION), dtype);

      } catch (IOException ie) {
	LogTask.logWarning(request, "Error sending download parameters: " + ie.getMessage(), logFileType);
	return false;
      }
      // initiate download
      try {
	Hello.initiateDownload(addr, Constants.RAM_MODE, Constants.GET_TIMEOUT);
      } catch (IOException ie) {
	LogTask.logWarning(request, "Error sending initiate download request to " + regid + ":\n" + ie.getMessage(), logFileType);
      }

      // mark it done
      forceStates[MaintenanceRequest.DOWNLOAD_RAM_ACTION] = false;

      return false;
    } else {
      LogTask.logVerbose(request, "Nothing to be done on " + regid + " for task " + MaintenanceRequest.getActionString(MaintenanceRequest.DOWNLOAD_RAM_ACTION), logFileType);
    }

    doneStates[MaintenanceRequest.DOWNLOAD_RAM_ACTION] = true;
    actionWaitTime = 0;
    return true;
  }

  // do the config download action
  // returns true if the action is done, and need to proceed to
  // the next action, false if keep attempting this action
  private boolean processCfgDownload (MaintenanceRequest mr, Registration reg) {
    String request = mr.getName();
    boolean force = !mr.isForced(MaintenanceRequest.DOWNLOAD_CONFIG_ACTION);
    int dtype = ConfigFile.getDeviceTypeForId(reg.getDeviceId());

    // a small kludge here....
    // if we are doing auto-download, compare the config version
    // to see if we need to download the config into it
    if (logFileType == Constants.AUTODOWNLOAD_LOGFILE) {
      String configVersion = null;
      try {
	configVersion = mr.getVersion(MaintenanceRequest.DOWNLOAD_CONFIG_ACTION, dtype);
      } catch (IOException ie) {
	LogTask.logError(request, "Error determining configuration file version: " + ie.getMessage(), logFileType);
	actionWaitTime = 4*60*1000;
	return false;  // try again later
      }

      LogTask.logVerbose(request, "File version: " + configVersion + " iEdge version: " + reg.getConfigVersion(), logFileType);
      if (configVersion.equals(reg.getConfigVersion())) {
	LogTask.logVerbose(request, "Nothing to be done on " + regid + " for task " + MaintenanceRequest.getActionString(MaintenanceRequest.DOWNLOAD_CONFIG_ACTION), logFileType);
	doneStates[MaintenanceRequest.DOWNLOAD_CONFIG_ACTION] = true;
	actionWaitTime = 0;
	return true;
      }
    }

    String config = null;
    try {
      config = mr.getConfig(dtype);
      if (config == null)
	throw new IOException("No further messages");
    } catch (IOException ie) {
      LogTask.logError(request, "Error downloading config file from the FTP server: " + ie.getMessage(), logFileType);
      actionWaitTime = 4*60*1000;
      return false;  // try again later
    }

    if (force) {
      // if the dude is not in maintenance mode, reboot him in
      // maintenance mode

      if (reg.getDeviceId() != Constants.DEVICE_ID_1000){
	LogTask.logVerbose(request, "Need to download config in maintenance mode", logFileType);

	if(reg.getMode() != Constants.ROM_MODE) {
	  LogTask.logMedium(request, "Rebooting " + regid + " in maintenance mode", logFileType);
	  try {
	    Hello.reboot(addr, Constants.REBOOT_ROM);
	    // mark it so that we reboot him back in operational mode
	    forceStates[MaintenanceRequest.DOWNLOAD_CONFIG_ACTION] = true;
	  } catch (IOException ie) {
	    LogTask.logWarning(request, "Error sending the reboot request to " + regid + ":\n" + ie.getMessage(), logFileType);
	  }
	  actionWaitTime = 30*1000;  // 30 seconds
	  return false;
	} else {
	  LogTask.logVerbose(request, "iEdge already in maintenance mode", logFileType);
	  // the dude is already in ROM mode,
	  // we don't need to reboot him back in operational mode
	  // (doesn't work as well if the iserver reboots inbetween)
	  if (forceStates[MaintenanceRequest.DOWNLOAD_CONFIG_ACTION] != true)
	    forceStates[MaintenanceRequest.DOWNLOAD_CONFIG_ACTION] = false;
	}
      }
    }

    // check to see if we are configuring any parameters on this
    // device which are not compatible with the device id
    int edgeDevType = ConfigFile.getDeviceTypeForId(reg.getDeviceId());
    LogTask.logVerbose(request, "Validating config file for device type (" + ConfigFile.getDeviceTypeString(edgeDevType) + ")", logFileType);
    try {
      config = validateConfigFileForDeviceType(request, config, edgeDevType);
    } catch (IOException ie) {
      LogTask.logError(request, "Error validating config file: " + ie.getMessage(), logFileType);
      LogTask.logError(request, "Skipping the task for " + regid, logFileType);
      doneStates[MaintenanceRequest.DOWNLOAD_CONFIG_ACTION] = true;
      actionWaitTime = 0;
      return true;
    }

    // need atleast a program and a bye
    if (config.length() <= 10) {
      LogTask.logWarning(request, "Not enough commands to send (" + config + "), skipping the task for " + regid, logFileType);
      doneStates[MaintenanceRequest.DOWNLOAD_CONFIG_ACTION] = true;
      actionWaitTime = 0;
      return true;
    }

    LogTask.logMedium(request, "Downloading configuration on to " + regid, logFileType);
    // download the commands
    try {
      Hello.sendCommands(addr, Constants.SET_TIMEOUT, config);
    } catch (IOException ie) {
      LogTask.logWarning(request, "Error sending commands: " + ie.getMessage(), logFileType);
      actionWaitTime = 60000;
      return false;
    }

    // re-init this, sendCommands() might have taken a while to finish
    actionInitTime = System.currentTimeMillis();

    // see if the dude has to go back to the operational mode
    if (forceStates[MaintenanceRequest.DOWNLOAD_CONFIG_ACTION] == true || reg.getDeviceId() == Constants.DEVICE_ID_1000) {

      LogTask.logVerbose(request, "Rebooting " + regid + " in operational mode", logFileType);

      forceStates[MaintenanceRequest.DOWNLOAD_CONFIG_ACTION] = true;
      try {
	// in case of iedge 1000, this should potentially reset
	// the processes to read the new configuration
	if(reg.getDeviceId() != Constants.DEVICE_ID_1000)
	  Hello.reboot(addr, Constants.REBOOT_RAM);
	else
	  Hello.rebootI1000(addr, Constants.REBOOT_RAM,reg.getSerialNumber());

      } catch (IOException ie) {
	LogTask.logWarning(request, "Error sending the reboot request to " + regid + ":\n" + ie.getMessage(), logFileType);
	actionWaitTime = 30*1000;  // 30 seconds
	return false;
      }
    }
    doneStates[MaintenanceRequest.DOWNLOAD_CONFIG_ACTION] = true;
    actionWaitTime = 0;
    return true;
  }

  // make sure you don't send invalid commands to an iedge
  // any IOException here means something is seriously wrong, because
  // we already have validated the file as a whole
  private String validateConfigFileForDeviceType (String request, String config, int dt) throws IOException {
    BufferedReader br = new BufferedReader(new StringReader(config));
    StringBuffer sb = new StringBuffer();
    String line = null;

    while ((line = br.readLine()) != null) {
      if (line.startsWith("set ")) {
	if (ConfigFile.isCommandValidForDeviceType(line, dt))
	  sb.append(line);
	else
	  LogTask.logWarning(request, "skipping (" + line + ")", logFileType);
      } else
	sb.append(line);
      sb.append("\n");
    }

    String newConfig = sb.toString();
    if (!config.equals(newConfig)) {
      LogTask.logVerbose(request, "Validated config file:\n" + config, logFileType);
    }

    return newConfig;
  }

  private boolean processSaveCfg (MaintenanceRequest mr, Registration reg) {

    String request = mr.getName();
    int dtype = ConfigFile.getDeviceTypeForId(reg.getDeviceId());

    LogTask.logVerbose(request, "Retrieving configuration from the iEdge (" + regid + ")", logFileType);
    String config = null;
    try {
      config = "#\n# Generated by iServer version " + 
	JServerMain.versionInfo[1] + JServerMain.versionInfo[2] + "\n" +
	ConfigFile.createConfigProvider(this, reg).getConfig();
    } catch (IOException ie) {
      LogTask.logError(request, "Error retrieving configuration from iEdge " + regid + ":\n" + ie.getMessage() + "\nWill try again after 2 minutes", logFileType);
      actionWaitTime = 2*60*1000;  // try after 2 minutes
      return false;
    }

    // save the configuration to the ftp server
    LogTask.logVerbose(request, "Saving configuration for the iEdge (" + regid + ")", logFileType);
    try {
      mr.saveConfig(dtype, regid, config);
    } catch (IOException ie) {
      LogTask.logError(request, "Error uploading configuration of " + regid + " to the server:\n" + ie.getMessage() + "\nWill try again after 2 minutes", logFileType);
      actionWaitTime = 2*60*1000;
      return false;
    }

    doneStates[MaintenanceRequest.DOWNLOAD_CONFIG_ACTION] = true;
    actionWaitTime = 0;
    return true;
  }

  public boolean retrieve (int code, DataConsumer dc) throws IOException {
    return LimitedCommandComm.doGet(code, addr, dc, this);
  }

  public int getGetTimeout () {
    return Constants.GET_TIMEOUT;
  }

  public int getSetTimeout () {
    return Constants.SET_TIMEOUT;
  }

}

