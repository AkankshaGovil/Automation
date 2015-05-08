package com.nextone.JServer;


import java.util.*;
import java.io.*;
import com.nextone.common.Bridge;
import com.nextone.common.ConfigFile;
import com.nextone.common.MaintenanceRequest;
import com.nextone.common.Registration;

/**
 * this class contains code to perform the auto-download procedure
 */
public class AutoDownloadTask extends DownloadTask {
	  private Registration reg;

	  /**
	   * this is the constructor used from the AutoDownload to schedule
	   * automatic downloads (download-on-demand)
	   *
	   * @param mr the maintenance request to be executed
	   * @param reg the registration message from the iedge
	   * @param bs Bridge
	   */
	  AutoDownloadTask (MaintenanceRequest mr, Registration reg, Bridge bs) {
		 super(mr, bs, Constants.AUTODOWNLOAD_LOGFILE);
		 this.reg = reg;
	  }

	  /**
	   * do any initialization here
	   *
	   * @return true if init is successful and can proceed, false otherwise
	   */
	  protected boolean init () {
		 // make sure that there are some actions configured here...
		 int [] actions = mr.getActions();
		 if (actions == null || actions.length == 0) {
			LogTask.logMedium(request, "No actions configured", logFileType);
			return false;
		 }

		 return true;
	  }

	  /**
	   * override these methods to do nothing
	   */
	  protected void scheduleStopDownload () {}
	  protected void cancelStopDownload () {}

	  /**
	   * task is done, remove the reference from the AutoDownload class
	   */
	  protected void cleanup () {
		 AutoDownload.doneAutoDownload(reg.getRegId());
	  }

	  /**
	   * create the list of edges to be processed in this task
	   */
	  protected synchronized void createEdgeList () {
		 edges = new HashMap();
		 try {
			String regid = reg.getRegId();
			ActionState as = new ActionState(regid, logFileType);
			edges.put(regid, as);
			updateEdgeCount(ConfigFile.getDeviceTypeForId(reg.getDeviceId()));
		 } catch (IOException ie) {
			LogTask.logError(request, "Error forming ActionState:\n" + ie, logFileType);
		 }
	  }

	  /**
	   * get the validated config file using super's method, then embed the
	   * the config version into it
	   */
	  protected String validateConfigFile (int dtype) throws IOException {
		 String version = mr.getVersion(MaintenanceRequest.DOWNLOAD_CONFIG_ACTION, dtype);
		 if (version == null)
			throw new IOException("Unable to determine the version of the configuration file");

		 String con = super.validateConfigFile(dtype);

		 BufferedReader br = new BufferedReader(new StringReader(con));
		 String line = null;
		 StringBuffer sb = new StringBuffer();
		 while ((line = br.readLine()) != null) {
			// just before the "program" command, add a line to set the
			// config version
			if (line.startsWith("program")) {
			   sb.append("set config_version ");
			   sb.append(version);
			   sb.append("\n");
			}
			sb.append(line);
			sb.append("\n");
		 }

		 LogTask.logVerbose(request, "Added config version (" + version + ") to the configuration", logFileType);
		 return sb.toString();
	  }

}
