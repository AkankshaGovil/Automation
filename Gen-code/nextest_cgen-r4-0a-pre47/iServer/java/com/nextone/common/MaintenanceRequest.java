package com.nextone.common;

import java.io.*;
import java.util.*;
import com.nextone.util.SysUtil;

/**
 * This class is used to store the maintenenace actions
 */
public class MaintenanceRequest implements Serializable {
	  /** download maintenance code action */
	  public static final int DOWNLOAD_SOFTROM_ACTION = 0;
	  /** download runtime code action */
	  public static final int DOWNLOAD_RAM_ACTION = 1;
	  /** download configuration */
	  public static final int DOWNLOAD_CONFIG_ACTION = 2;
	  /** save configuration */
	  public static final int SAVE_CONFIG_ACTION = 3;

	  public static final int MAX_ACTIONS = 4;

	  private String name; // name of this action
	  private String comment;  // comments for this action
	  private String [] groups;  // maintenance groups affected by this action
	  private int [] actions;  // the work need to be done for this action
	  private boolean [] force; // does the work need to be forced?
  //	  private DTServer [] servers; // servers for the work
	  private Date scheduleStart;  // start of scheduled time
	  private Date scheduleEnd;  // end of scheduled time
	  private Date creationDate;  // creation date of this action
	  private Date modificationDate;  // last modified
	  private boolean schedule;  // does this have to be scheduled right away?

	  private static final String dlmaint = "d/l Maintenance";
	  private static final String dlrun = "d/l Runtime";
	  private static final String dlconf = "d/l Configuration";
	  private static final String saveconf = "Save Configuration";

	  static final long serialVersionUID = 2943397687076411184L;

	  public MaintenanceRequest () {
		 this("<new>", "", null, null);
	  }

	  public MaintenanceRequest (String name, String comment, Date cd, Date md) {
		 this.name = name;
		 this.comment = comment;
		 Date d = new Date();
		 creationDate = (cd == null)?d:cd;
		 modificationDate = (md == null)?d:md;
	  }

	  public static String getActionString (int action) throws IllegalArgumentException {
		 switch (action) {
			case MaintenanceRequest.DOWNLOAD_SOFTROM_ACTION:
			  return dlmaint;
			case MaintenanceRequest.DOWNLOAD_RAM_ACTION:
			  return dlrun;
			case MaintenanceRequest.DOWNLOAD_CONFIG_ACTION:
			  return dlconf;
			case MaintenanceRequest.SAVE_CONFIG_ACTION:
			  return saveconf;
		 }

		 throw new IllegalArgumentException("Unknown Action (" + action + ")");
	  }

	  public static int getActionForString (String str) throws IllegalArgumentException {
		 if (str.equals(dlmaint))
			return MaintenanceRequest.DOWNLOAD_SOFTROM_ACTION;
		 else if (str.equals(dlrun))
			return MaintenanceRequest.DOWNLOAD_RAM_ACTION;
		 else if (str.equals(dlconf))
			return MaintenanceRequest.DOWNLOAD_CONFIG_ACTION;
		 else if (str.equals(saveconf))
			return MaintenanceRequest.SAVE_CONFIG_ACTION;

		 throw new IllegalArgumentException("Action " + str + " is unrecognized");
	  }

	  public void dump () {
		 System.out.println("Name: " + name);
		 System.out.println("Comment: " + comment);
		 System.out.println("Groups:");
		 for (int i = 0; groups != null && i < groups.length; i++)
			System.out.println("    " + groups[i]);
		 System.out.println("Actions:");
		 for (int i = 0; actions != null && i < actions.length; i++) {
			System.out.println("    " + MaintenanceRequest.getActionString(actions[i]) + ":");
                        //			System.out.println("        " + servers[i]);
		 }
		 System.out.println("Schedule Start: " + scheduleStart);
		 System.out.println("Schedule End: " + scheduleEnd);
		 System.out.println("Creation Date: " + creationDate);
		 System.out.println("Modification Date: " + modificationDate);
	  }

	  public synchronized String getName () {
		 return name;
	  }

	  public synchronized void setName (String newname) {
		 name = newname;
	  }

	  public synchronized String getComment () {
		 return comment;
	  }

	  public synchronized void setComment (String newcomment) {
		 comment = newcomment;
	  }

	  public synchronized String [] getGroups () {
		 return groups;
	  }

	  public synchronized int [] getActions () {
		 return actions;
	  }

	  public synchronized boolean [] getForceFlags () {
		 return force;
	  }

	  /**
	   * determines if the specified action is forced or not
	   */
	  public synchronized boolean isForced (int action) {
		 for (int i = 0; actions != null && i < actions.length; i++) {
			if (actions[i] == action)
			   return force[i];
		 }

		 return false;
	  }

	  /**
	   * returns the server object configured for the given action
	   */
  /*
	  public synchronized DTServer getServer (int action) throws IllegalArgumentException {
		 String act = getActionString(action);  // validates action

		 for (int i = 0; actions != null && i < actions.length; i++) {
			if (actions[i] == action)
			   return servers[i];
		 }

		 throw new IllegalArgumentException("Nothing configured for the action (" + act + ")");
	  }

	  public synchronized DTServer [] getServers () {
		 return servers;
	  }
  */
	  public synchronized void setGroups (String [] newgrps) {
		 groups = newgrps;
	  }
  /*
	  public synchronized void setActions (int [] newacs, boolean [] newforce, DTServer [] newsrvs) throws IllegalArgumentException {
		 if (newacs != null && newforce != null && newsrvs != null && 
			 newacs.length != newsrvs.length && newacs.length != newforce.length)
			throw new IllegalArgumentException("actions length should be equal to servers length");

		 actions = newacs;
		 servers = newsrvs;
		 force = newforce;
	  }
  */
	  public synchronized void addGroup (String newgrp) throws ArrayStoreException, IndexOutOfBoundsException, NullPointerException {
		 if (groups == null) {
			groups = new String [1];
			groups[0] = newgrp;
		 } else {
			String [] tmp = new String [groups.length+1];
			System.arraycopy(groups, 0, tmp, 0, groups.length);
			tmp[groups.length] = newgrp;
			groups = tmp;
		 }
	  }
  /*
	  public synchronized void addAction (int act, boolean forceFlag, DTServer si) throws ArrayStoreException, IndexOutOfBoundsException, NullPointerException {
		 if (actions == null) {
			actions = new int [1];
			actions[0] = act;
			force = new boolean [1];
			force[0] = forceFlag;
			servers = new DTServer [1];
			servers[0] = si;
		 } else {
			int [] tmp = new int [actions.length+1];
			System.arraycopy(actions, 0, tmp, 0, actions.length);
			tmp[actions.length] = act;
			actions = tmp;
			boolean [] tmpb = new boolean [force.length+1];
			System.arraycopy(force, 0, tmpb, 0, force.length);
			tmpb[force.length] = forceFlag;
			force = tmpb;
			DTServer [] tmpsi = new DTServer [servers.length+1];
			System.arraycopy(servers, 0, tmpsi, 0, servers.length);
			tmpsi[servers.length] = si;
			servers = tmpsi;
		 }
	  }
  */
	  /**
	   * returns if this action is currently scheduled (happening right
	   * now or to happen sometime in the future)
	   */
	  public boolean isScheduled () {
		 if (scheduleStart != null) {
			Date curDate = new Date();
			// is the start in the future?
			if (scheduleStart.after(curDate))
			   return true;
			else
			   // are we in the window?
			   return isCurrentlyScheduled(curDate);
		 }

		 return false;
	  }

	  /**
	   * returns if we are currently in the scheduling window
	   */
	  public boolean isCurrentlyScheduled () {
		 return isCurrentlyScheduled(new Date());
	  }

	  public boolean isCurrentlyScheduled (Date d) {
		 return SysUtil.isDateInRange(d, scheduleStart, scheduleEnd);
	  }

	  public void setSchedule (Date start, Date end) {
		 scheduleStart = start;
		 scheduleEnd = end;
	  }

	  public Date getScheduleStart () {
		 return scheduleStart;
	  }

	  public Date getScheduleEnd () {
		 return scheduleEnd;
	  }

	  public synchronized Date getCreationDate () {
		 return creationDate;
	  }

	  public synchronized Date getModificationDate () {
		 return modificationDate;
	  }

	  public synchronized void setModificationDate (Date newdate) {
		 modificationDate = newdate;
	  }

	  public String toString () {
		 return name;
	  }

	  public String getVersion (int action, int devType) throws IOException {
            /*
		 DTServer srvr = getServer(action);
		 if (srvr != null)
			return srvr.getVersion(devType);  // may block
            */
		 throw new IOException("No server configured");
	  }

	  public void setVersion (int action, String ver, int devType) throws IOException {
            /*		 DTServer srvr = getServer(action);
		 if (srvr != null)
			srvr.setVersion(devType, ver);
                        else*/
			throw new IOException("No server configured");
	  }

	  public synchronized boolean toBeScheduled () {
		 return schedule;
	  }

	  public synchronized void setToBeScheduled (boolean b) {
		 schedule = b;
	  }

	  public String getConfig (int devType) throws IOException {
            /*		 DTServer srvr = getServer(DOWNLOAD_CONFIG_ACTION);
                         if (srvr == null)*/
			throw new IOException("No server configured");

                        //		 return srvr.getConfig(devType);
	  }

	  public void setConfig (int devType, String newConfig) throws IOException {
            /*		 DTServer srvr = getServer(DOWNLOAD_CONFIG_ACTION);
                         if (srvr == null)*/
			throw new IOException("No server configured");
                        //		 else
                        //			srvr.setConfig(devType, newConfig);
	  }

	  public void saveConfig (int devType, String regid, String config) throws IOException {
            /*		 DTServer srvr = getServer(SAVE_CONFIG_ACTION);
		 if (srvr != null)
			srvr.saveConfig(devType, regid, config);
                        else*/
			throw new IOException("No server configured");
	  }

}
