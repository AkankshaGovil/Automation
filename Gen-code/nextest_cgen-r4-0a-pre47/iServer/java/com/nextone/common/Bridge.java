package com.nextone.common;

//import java.rmi.Remote;
//import java.rmi.RemoteException;
import java.util.*;
import java.io.*;
import java.net.*;

public interface Bridge/* extends Remote*/ {
  // some standard exception strings
  public static final String NOT_PROVISIONED = "Not provisioned on this iServer";

  /** get the iedge provisioning data from the server */
  public Object getProvData (short [] cmds, Object [] keys) throws Exception;

  /** get the tcp port number to contact to get the list */
  public int getListPort (int listType) throws Exception;

  /** get the iedge parameters (IEdgeList object) */
  public IEdgeList getIedgeParams (String regid, int port) throws Exception;

  /** lookup the iserver for the given phone number and return the
   * corresponding IEdgeList object */
  public IEdgeList getIedgeParamsForPhone (String phone) throws Exception;

  /** lookup the iserver for the given vpn phone number and return the
   * corresponding IEdgeList object */
  public IEdgeList getIedgeParamsForVpnPhone (String phone, int extLen) throws Exception;
 
  /** lookup the iserver for the given calling plan route name and return the
   * corresponding Callingplan route object */
  public CallPlanRoute getCallingPlanRoute (String routeName) throws Exception;

  /** get the current presence from the calling plans **/
  public String getPresence (String destphone) throws Exception;

  /** get version information */
  public String [] getVersion () throws Exception;

  /** set the permission strings */
  public boolean changePermissions (String rp, String wp) throws Exception;

  /** get the status of the iserver */
  public String getStatus () throws Exception;


  /** get the list of names of all the maintenance groups available */
  public String [] getMaintenanceGroupNames () throws Exception;

  /** get the list of name and comments of all the maintenance groups */
  public String [][] getMaintenanceGroupNamesAndComments () throws Exception;

  /** get the maintenance group object for the given name */
  public MaintenanceGroup getMaintenanceGroup (String group) throws Exception;

  /** store the maintenance group object */
  public boolean storeMaintenanceGroup (MaintenanceGroup mg) throws Exception;

  /** delete the maintenance group */
  public boolean deleteMaintenanceGroup (String group) throws Exception;


  /** get the list of names of all the maintenance requests available */
  public String [] getMaintenanceRequestNames () throws Exception;

  /** get the list of name and comments of all the maintenance requests */
  public String [][] getMaintenanceRequestNamesAndComments () throws Exception;

  /** get the maintenance request object for the given name */
  public MaintenanceRequest getMaintenanceRequest (String request) throws Exception;

  /** store the maintenance request object */
  public boolean storeMaintenanceRequest (MaintenanceRequest mr) throws Exception;

  /** delete the maintenance request */
  public boolean deleteMaintenanceRequest (String request) throws Exception;


  /** is the maintenance request currently actively being worked on? */
  public boolean isMaintenanceRequestActive (String request) throws Exception;

  /** abort the currently active maintenance request */
  public void abortMaintenanceRequest (String request) throws Exception;


  /** get the list of names of all the maintenance logs available */
  public String [] getMaintenanceLogNames () throws Exception;

  /** return a File handle for the log file */
  public File getLogFile (String name) throws Exception;

  /** delete the maintenance log file */
  public boolean deleteMaintenanceLog (String name) throws Exception;



  /** get the list of names of all the auto-download groups available */
  public String [] getAutoDownloadGroupNames () throws Exception;

  /** get the list of name and comments of all the auto-download groups */
  public String [][] getAutoDownloadGroupNamesAndComments () throws Exception;

  /** get the auto-download configuration (MaintenanceRequest object)
   * for the given regid */
  public MaintenanceRequest getAutoDownloadConfig (String regid, boolean isGroup) throws Exception;

  /** store the auto-download configuration (the regid for which this
   * config applies, is the name of the MaintenanceRequest object) */
  public boolean storeAutoDownloadConfig (MaintenanceRequest mr, boolean isGroup) throws Exception;

  /** delete the auto-download configuration */
  public boolean deleteAutoDownloadConfig (String regid, boolean isGroup) throws Exception;

  /** is the auto-download session for this regid currently active? */
  public boolean isAutoDownloadActive (String regid) throws Exception;

  /** abort the active auto-download session of this regid */
  public void abortAutoDownload (String regid) throws Exception;

  /** return a File handle for the auto-download log file for this regid */
  public File getAutoDownloadLogFile (String regid) throws Exception;

  /** delete the auto-download log file */
  public boolean deleteAutoDownloadLogFile (String regid) throws Exception;

  /** get the iserver configuration */
  public iServerConfig getIserverConfig () throws Exception;

  /** set/update the iserver configuration */
  public boolean setIserverConfig (iServerConfig cfg) throws Exception;

  /** import iserver database from the given file*/
  public boolean importDB (boolean isLocal,int cmd,String dbName, File db) throws Exception;

  /** export iserver database
   * @return null when the exported file is stored in the iServer, a valid File object
   * if the exported file is sent to the iView
   */
  public File exportDB (boolean isLocal,String dbName, int listType,Socket sock) throws Exception;


  /** get the db file names */
  public String[] getDBFilenames() throws Exception;
    
  /** get the license alarms **/
  public int [][] getLicenseAlarms () throws Exception;
    
  /** clear the license alarms **/
  public boolean clearLicenseAlarms()  throws Exception;

  public int getMaximumRecords(int listType) throws Exception;

  /** Get the maximum calls**/
  public int getMaxCalls() throws Exception;
  public int getMaxMRCalls() throws Exception;

  /** return if any db operations are currently in progress */
  public boolean isDbOperationInProgress ();

  /** set if new db operation status */
  public void setDbOperationInProgress (boolean newval);

  /** clearss the log file */
  public boolean clearLog (int logFileType) throws Exception;

  /** process the set of commands **/
  public boolean processCommands(Commands cmd) throws Exception;

  public boolean processBulkCommands(Commands cmds[]) throws Exception;

  // get iServer capabilities
  public Capabilities getCapabilities() throws Exception;

}
