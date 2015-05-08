package com.nextone.JServer;

import java.io.BufferedReader;
import java.io.BufferedWriter;
import java.io.DataOutputStream;
import java.io.File;
import java.io.FileFilter;
import java.io.FileInputStream;
import java.io.FileOutputStream;
import java.io.FileReader;
import java.io.FileWriter;
import java.io.IOException;
import java.io.InputStreamReader;
import java.io.InvalidClassException;
import java.io.ObjectInputStream;
import java.io.ObjectOutputStream;
import java.io.OptionalDataException;
import java.io.SequenceInputStream;
import java.io.StreamCorruptedException;
import java.net.Socket;
import java.net.InetAddress;

import com.nextone.JServer.Constants;
import com.nextone.common.Bridge;
import com.nextone.common.BridgeException;
import com.nextone.common.CallPlanRoute;
import com.nextone.common.Capabilities;
import com.nextone.common.Commands;
import com.nextone.common.ExistException;
import com.nextone.common.IEdgeList;
import com.nextone.common.MaintenanceGroup;
import com.nextone.common.MaintenanceRequest;
import com.nextone.common.NoEntryException;
import com.nextone.common.NotProvisionedBridgeException;
import com.nextone.common.ProvisionData;
import com.nextone.common.iServerConfig;
import com.nextone.util.DelayedQueueProcessor;
import com.nextone.util.QueueProcessor;
import com.nextone.util.SysUtil;
import com.nextone.util.WeakEncryptionInputStream;
import com.nextone.util.WeakEncryptionOutputStream;

public class BridgeServer implements Bridge, Constants {
  //  private Timer timer;
  private MiscServer miscServer;
  private LogServer logServer;
  private File dataDir, groupDir, requestDir, logDir;
  private File autoDlDir, autoDlRequestDir, autoDlLogDir;
  private FileFilter groupFileFilter, requestFileFilter, logFileFilter;
  private FileFilter autoDlRequestFileFilter, autoDlLogFileFilter;
  private byte [] encryptKey = {
    (byte)0x00, (byte)0x11, (byte)0xca, (byte)0xbe,
    (byte)0x45, (byte)0xef, (byte)0x0b, (byte)0x25,
    (byte)0x19, (byte)0xf3, (byte)0xd9, (byte)0x69,
    (byte)0x00, (byte)0x11, (byte)0x0b, (byte)0x25,
    (byte)0xca, (byte)0x22, (byte)0xd9, (byte)0xa8,
    (byte)0x19, (byte)0xbc, (byte)0x89, (byte)0x22,
    (byte)0xce, (byte)0x33, (byte)0x49, (byte)0xdb,
    (byte)0x2b, (byte)0xff, (byte)0x29, (byte)0xdd,
  };
  //  private DownloadServer downloadServer;
  private Object lockObject;
  private int [][] licenseAlarms = new int [2][0];
  private boolean dbOp;
  private DelayedQueueProcessor fileDeleteQueue;
  private ProcessManagerClient pmc;

  /**
   * all the native methods
   */
  private native ProvisionData findIedge (String serial, int port);
  private native boolean processCommand (int argc, String [] cmds) throws BridgeException;
  public native boolean restartRSD();
  public native boolean restartIServer();
  private native int getPhoneNumLen ();
  private native int getVpnGroupLen ();
  private native int getVpnNumLen ();
  private native int getVpnNameLen ();
  private native int getProcessManagerPollInterval ();
  private native int getCallPlanAttrLen();
  private native int getCallingPlanRouteLen();
  private native int[] getIServerVportLicenseAlarms();
  private native int[] getIServerMRVportLicenseAlarms();
  private native IEdgeList getIedgeList (String regid, int port);
  private native IEdgeList getIedgeListForPhone (String phone);
  private native int getCallingPlanCount();
  private native int getCallingPlanBindingCount();
  private native int getCallRouteCount();
  private native int getVpnGroupCount();
  private native int getVpnCount();
  private native int getIEdgeCount();
  private native int getMaximumCalls();
  private native int getMaximumMRCalls();
  private native int getTriggerCount();
  private native int getRealmCount();
  private native int getVnetCount();
  private native int getIGRPCount();
  private native IEdgeList getIedgeListForVpnPhone (String phone, int extLen);
  private native boolean getCallingPlanRouteImpl (String routeName, CallPlanRoute cpr);
  private native String getPresenceNumber   (String destPhone);
  public native iServerConfig getNativeIserverConfig () throws Exception;
  public native boolean setNativeIserverConfig (iServerConfig cfg) throws Exception;
  public native String getAravoxConfig () throws Exception;
  public native String [] getInterfaceNames () throws Exception;
  public native Capabilities getIServerCapabilities() throws Exception;

  public BridgeServer (ThreadGroup tg, String cd, Object lockObject) throws IOException, BridgeException {
     this(tg,cd,lockObject,null);
  }

  public BridgeServer (ThreadGroup tg, String cd, Object lockObject, InetAddress mgmtAddress) throws IOException, BridgeException {
    super();

    this.lockObject = lockObject;
    this.dataDir = new File(cd + File.separator + Constants.dataDirName);
    this.groupDir = new File(dataDir.getPath() + File.separator + Constants.groupDirName);
    this.requestDir = new File(dataDir.getPath() + File.separator + Constants.requestDirName);
    this.logDir = new File(dataDir.getPath() + File.separator + Constants.logDirName);
    this.autoDlDir = new File(dataDir.getPath() + File.separator + Constants.autoDlDirName);
    this.autoDlRequestDir = new File(autoDlDir.getPath() + File.separator + Constants.requestDirName);
    this.autoDlLogDir = new File(autoDlDir.getPath() + File.separator + Constants.logDirName);

    this.groupFileFilter = new FileFilter () {
	public boolean accept (File pathname) {
	  if (!pathname.isFile()) {
	    return false;
	  }
	  try {
	    MaintenanceGroup m = readMaintenanceGroup(pathname);
	  } catch (Exception ie) {
	    return false;
	  }
	  return true;
	}
      };
    this.requestFileFilter = new FileFilter () {
	public boolean accept (File pathname) {
	  if (!pathname.isFile()) {
	    return false;
	  }
	  try {
	    MaintenanceRequest m = readMaintenanceRequest(pathname);
	  } catch (Exception ie) {
	    return false;
	  }
	  return true;
	}
      };
    this.logFileFilter = new FileFilter () {
	public boolean accept (File pathname) {
	  if (!pathname.isFile()) {
	    return false;
	  }
	  // if we could check for the file being an ascii file...

	  return true;
	}
      };
    this.autoDlRequestFileFilter = this.requestFileFilter;
    this.autoDlLogFileFilter = this.logFileFilter;

    // start the timer to send keep alives to the process manager
    //    timer = new Timer(true);
    //    timer.schedule(new ProcessManagerClient(), 1000, 1000*getProcessManagerPollInterval());
    pmc = new ProcessManagerClient(1000*getProcessManagerPollInterval());
    pmc.setPriority(Thread.MAX_PRIORITY);
    pmc.setDaemon(true);
    pmc.start();

    if(mgmtAddress  ==  null){
      logServer = new LogServer(tg, this, JServer.isDebug());
      miscServer = new MiscServer(tg, JServer.isDebug(), lockObject);
    }else{
      logServer = new LogServer(tg, this, JServer.isDebug(), mgmtAddress);
      miscServer = new MiscServer(tg, JServer.isDebug(), lockObject,mgmtAddress);
    }
    logServer.setName("LogServer");
    //    downloadServer = new DownloadServer(tg, this);
    miscServer.setName("Miscellaneous Server");

    fileDeleteQueue = (DelayedQueueProcessor)DelayedQueueProcessor.getInstance(new DelayedFileDeleter());
    fileDeleteQueue.setPriority(Thread.MIN_PRIORITY);
    fileDeleteQueue.start();
  }

  /* return a ProvisionData object for the given keys */
  public Object getProvData (short [] cmds, Object [] keys) throws Exception {
    String serial = (String)keys[0];
    Integer port = (Integer)keys[1];

    ProvisionData pd = null;

    try {
      synchronized (lockObject) {
	pd = findIedge(serial, port.intValue());
      }

    } catch (NoSuchMethodError ne) {
      throw new BridgeException("Software mismatch: " + ne.getMessage(), ne.toString());
    }

    if (pd == null) {
      throw new NotProvisionedBridgeException(serial, port.intValue());
    }

    return pd;
  }


  public int getListPort (int listType) throws Exception {
    switch (listType) {

    case Constants.LOG_FILE:
      if (logServer == null)
	throw new BridgeException("Log Server not found");
      return logServer.getPort();

    case Constants.MISC_REQ:
      if (miscServer == null)
	throw new BridgeException("Misc Server not found");
      return miscServer.getPort();
    }

    throw new BridgeException("Requested list type (" + listType + ") not supported");
  }


  public void setDebugLevel (int level) {
    logServer.setDebug(JServer.isDebug());
    miscServer.setDebug(JServer.isDebug());
  }

  /* get the iedge parameters (IEdgeList object) */
  public IEdgeList getIedgeParams (String regid, int port) throws Exception {
    IEdgeList il = null;
    try {
      synchronized (lockObject) {
	il = getIedgeList(regid, port);
      }
    } catch (NoSuchMethodError ne) {
      throw new BridgeException("Software mismatch: " + ne.getMessage(), ne.toString());
    }

    if (il == null)
      throw new NotProvisionedBridgeException(regid, port);

    return il;
  }


  /* return version info */
  public String [] getVersion () {
    return JServerMain.versionInfo;
  }


  // we have these here because the Bridge interface requires it
  // however, the action for this is contained in the JServer itself
  public boolean changePermissions (String rp, String wp) throws Exception {
    throw new BridgeException("invalid usage");
  }

  public String getStatus () throws Exception {
    throw new BridgeException("invalid usage");
  }



  /* get the list of names of all the maintenance groups available */
  public String [] getMaintenanceGroupNames () throws Exception {
    String [] result = null;

    // check to see if the jserver data directory is present
    // and if the group dir is also present
    if (dataDir.exists() && dataDir.isDirectory() &&
	groupDir.exists() && groupDir.isDirectory()) {
      File [] files = groupDir.listFiles(groupFileFilter);
      if (files != null && files.length > 0) {
	result = new String [files.length];
	for (int i = 0; i < files.length; i++)
	  result[i] = files[i].getName();
      }
    }

    return result;
  }

  private MaintenanceGroup readMaintenanceGroup (File file) throws ClassNotFoundException, IOException {

    ObjectInputStream ois = new ObjectInputStream(new WeakEncryptionInputStream(new FileInputStream(file), encryptKey));
    MaintenanceGroup mg = (MaintenanceGroup)ois.readObject();
    ois.close();
    return mg;
  }

  /* get the maintenance group object for the given name */
  public MaintenanceGroup getMaintenanceGroup (String group) throws Exception {
    if (!dataDir.exists() || !dataDir.isDirectory())
      throw new BridgeException("Data does not exist (" + dataDir.getPath() + ")");

    if (!groupDir.exists() || !groupDir.isDirectory())
      throw new BridgeException("Data does not exist (" + groupDir.getPath() + ")");

    File mf = new File(groupDir + File.separator + group);
    if (!mf.exists() || !mf.isFile())
      throw new BridgeException("Group does not exist (" + mf.getPath() + ")");

    try {
      MaintenanceGroup mg = readMaintenanceGroup(mf);
      if (!group.equals(mg.getName()))
	mg.setName(group);
      return mg;
    } catch (ClassNotFoundException cnfe) {
      throw new BridgeException("Internal error: " + cnfe.getMessage(), cnfe.toString());
    } catch (InvalidClassException ice) {
      throw new BridgeException("Incompatible software: " + ice.getMessage(), ice.toString());
    } catch (StreamCorruptedException sce) {
      throw new BridgeException("Incompatible software: " + sce.getMessage(), sce.toString());
    } catch (OptionalDataException ode) {
      throw new BridgeException("Incompatible software: " + ode.getMessage(), ode.toString());
    } catch (IOException ie) {
      throw new BridgeException("Error reading maintenance group information: " + ie.getMessage(), ie.toString());
    }

  }

  /* store the maintenance group object */
  public boolean storeMaintenanceGroup (MaintenanceGroup mg) throws Exception {
    // create the directories if not there
    if (!groupDir.exists()) {
      boolean status = false;
      StringBuffer sb = new StringBuffer();
      try {
	status = groupDir.mkdirs();
      } catch (Exception e) {
	status = false;
	sb.append(": ");
	sb.append(e.toString());
      }
      if (status == false)
	throw new BridgeException("Error creating group storage directory", sb.toString());
    }

    // write the file
    File mf = new File(groupDir + File.separator + mg.getName());
    try {
      ObjectOutputStream oos = new ObjectOutputStream(new WeakEncryptionOutputStream(new FileOutputStream(mf), encryptKey));
      oos.writeObject(mg);
      oos.flush();
      oos.close();
    } catch (IOException ie) {
      throw new BridgeException("Error writing file: " + ie.getMessage(), ie.toString());
    }

    return true;			
  }

  /* delete the maintenance group */
  public boolean deleteMaintenanceGroup (String group) throws Exception {
    if (!dataDir.exists() || !dataDir.isDirectory())
      throw new BridgeException("Data does not exist (" + dataDir.getPath() + ")");

    if (!groupDir.exists() || !groupDir.isDirectory())
      throw new BridgeException("Data does not exist (" + groupDir.getPath() + ")");

    File mf = new File(groupDir + File.separator + group);
    if (!mf.exists() || !mf.isFile())
      throw new BridgeException("Maintenance Group does not exist (" + mf.getPath() + ")");
    mf.delete();
    return true;
  }


  /* get the list of names of all the maintenance requests available */
  public String [] getMaintenanceRequestNames () throws Exception {
    String [] result = null;

    // check to see if the jserver data directory is present
    // and if the requests directory is present
    if (dataDir.exists() && dataDir.isDirectory() &&
	requestDir.exists() && requestDir.isDirectory()) {
      File [] files = requestDir.listFiles(requestFileFilter);
      if (files != null && files.length > 0) {
	result = new String [files.length];
	for (int i = 0; i < files.length; i++)
	  result[i] = files[i].getName();
      }
    }

    return result;
  }

  private MaintenanceRequest readMaintenanceRequest (File file) throws ClassNotFoundException, IOException {

    ObjectInputStream ois = new ObjectInputStream(new WeakEncryptionInputStream(new FileInputStream(file), encryptKey));
    MaintenanceRequest mr = (MaintenanceRequest)ois.readObject();
    ois.close();
    return mr;
  }

  /* get the maintenance request object for the given name */
  public MaintenanceRequest getMaintenanceRequest (String request) throws Exception {
    if (!dataDir.exists() || !dataDir.isDirectory())
      throw new BridgeException("Data does not exist (" + dataDir.getPath() + ")");

    if (!requestDir.exists() || !requestDir.isDirectory())
      throw new BridgeException("Data does not exist (" + requestDir.getPath() + ")");

    File mf = new File(requestDir + File.separator + request);
    if (!mf.exists() || !mf.isFile())
      throw new BridgeException("Maintenance Request does not exist (" + mf.getPath() + ")");

    try {
      MaintenanceRequest mr = readMaintenanceRequest(mf);
      if (!request.equals(mr.getName()))
	mr.setName(request);
      return mr;
    } catch (ClassNotFoundException cnfe) {
      throw new BridgeException("Internal error: " + cnfe.getMessage(), cnfe.toString());
    } catch (InvalidClassException ice) {
      throw new BridgeException("Incompatible software: " + ice.getMessage(), ice.toString());
    } catch (StreamCorruptedException sce) {
      throw new BridgeException("Incompatible software: " + sce.getMessage(), sce.toString());
    } catch (OptionalDataException ode) {
      throw new BridgeException("Incompatible software: " + ode.getMessage(), ode.toString());
    } catch (IOException ie) {
      throw new BridgeException("Error reading maintenance request information: " + ie.getMessage(), ie.toString());
    }

  }

  /* store the maintenance request object */
  public boolean storeMaintenanceRequest (MaintenanceRequest mr) throws Exception {


    // create the directories if not there
    if (!requestDir.exists()) {
      boolean status = false;
      StringBuffer sb = new StringBuffer();
      try {
	status = requestDir.mkdirs();
      } catch (Exception e) {
	status = false;
	sb.append(": ");
	sb.append(e.toString());
      }
      if (status == false)
	throw new BridgeException("Error creating request storage directory", sb.toString());
    }

    // write the file
    File mf = new File(requestDir + File.separator + mr.getName());
    try {
      ObjectOutputStream oos = new ObjectOutputStream(new WeakEncryptionOutputStream(new FileOutputStream(mf), encryptKey));
      oos.writeObject(mr);
      oos.flush();
      oos.close();
    } catch (IOException ie) {
      throw new BridgeException("Error writing file: " + ie.getMessage(), ie.toString());
    }

    //    try {
    //      if (mr.toBeScheduled())
    //	downloadServer.initRequest(mr.getName());
    //    } catch (Exception e) {
    throw new BridgeException("Request saved successfully.\nHowever, error saving scheduling information:\n");// + e.getMessage(), e.toString());
      //    }

    //    return true;			
  }

  /* delete the maintenance request */
  public boolean deleteMaintenanceRequest (String request) throws Exception {
    if (!dataDir.exists() || !dataDir.isDirectory())
      throw new BridgeException("Data does not exist (" + dataDir.getPath() + ")");

    if (!requestDir.exists() || !requestDir.isDirectory())
      throw new BridgeException("Data does not exist (" + requestDir.getPath() + ")");

    File mf = new File(requestDir + File.separator + request);
    if (!mf.exists() || !mf.isFile())
      throw new BridgeException("Maintenance Request does not exist (" + mf.getPath() + ")");
    mf.delete();
    return true;
  }


  // called right before the program exits, do any cleanup...
  public void stop () {
    logServer.stop();
    //    downloadServer.stop();
    miscServer.stop();
    //    timer.cancel();
    pmc.stopRunning();
    fileDeleteQueue.stopRunning();
    JServer.printDebug("BridgeServer exited", JServer.DEBUG_NORMAL);
  }


  /* is the maintenance request currently actively being worked on? */
  public boolean isMaintenanceRequestActive (String request) throws Exception {
    //    return downloadServer.isMaintenanceRequestActive(request);
    return false;
  }

  /* abort the currently active maintenance request */
  public void abortMaintenanceRequest (String request) throws Exception {
    //    downloadServer.abortMaintenanceRequest(request);
  }

  /* return a File handle for the log file */
  public File getLogFile (String name) throws Exception {
    if (!dataDir.exists() || !dataDir.isDirectory())
      throw new BridgeException("Data does not exist (" + dataDir.getPath() + ")");

    if (!logDir.exists() || !logDir.isDirectory())
      throw new BridgeException("Data does not exist (" + logDir.getPath() + ")");

    File mf = new File(logDir + File.separator + name);
    if (!mf.exists() || !mf.isFile())
      throw new BridgeException("Log file does not exist (" + mf.getPath() + ")");
    if (!mf.canRead())
      throw new BridgeException("Cannot read log file (" + mf.getPath() + ")");

    return mf;
  }

  /* get the list of names of all the maintenance logs available */
  public String [] getMaintenanceLogNames () throws Exception {
    String [] result = null;

    // check to see if the jserver data directory is present
    // and if the log directory also present
    if (dataDir.exists() && dataDir.isDirectory() &&
	logDir.exists() && logDir.isDirectory()) {
      File [] files = logDir.listFiles(logFileFilter);
      if (files != null && files.length > 0) {
	result = new String [files.length];
	for (int i = 0; i < files.length; i++)
	  result[i] = files[i].getName();
      }
    }

    return result;
  }

  /* delete the maintenance log file */
  public boolean deleteMaintenanceLog (String name) throws Exception {
    if (!dataDir.exists() || !dataDir.isDirectory())
      throw new BridgeException("Data does not exist (" + dataDir.getPath() + ")");

    if (!logDir.exists() || !logDir.isDirectory())
      throw new BridgeException("Data does not exist (" + logDir.getPath() + ")");

    File mf = new File(logDir + File.separator + name);
    if (!mf.exists() || !mf.isFile())
      throw new BridgeException("Log file does not exist (" + mf.getPath() + ")");
    mf.delete();
    return true;
  }

  /* get the list of name and comments of all the maintenance groups */
  public String [][] getMaintenanceGroupNamesAndComments () throws Exception {
    String [][] result = new String[0][0];

    if (dataDir.exists() && dataDir.isDirectory() &&
	groupDir.exists() && groupDir.isDirectory()) {
      File [] files = groupDir.listFiles(groupFileFilter);
      if (files != null && files.length > 0) {
	result = new String [files.length][2];
	for (int i = 0; i < files.length; i++) {
	  result[i][0] = files[i].getName();
	  result[i][1] = readMaintenanceGroup(files[i]).getComment();
	}
      }
    }

    return result;
  }

  /* get the list of name and comments of all the maintenance requests */
  public String [][] getMaintenanceRequestNamesAndComments () throws Exception {
    String [][] result = new String[0][0];

    if (dataDir.exists() && dataDir.isDirectory() &&
	requestDir.exists() && requestDir.isDirectory()) {
      File [] files = requestDir.listFiles(requestFileFilter);
      if (files != null && files.length > 0) {
	result = new String [files.length][2];
	for (int i = 0; i < files.length; i++) {
	  result[i][0] = files[i].getName();
	  result[i][1] = readMaintenanceRequest(files[i]).getComment();
	}
      }
    }

    return result;
  }



  /** get the list of names of all the auto-download groups available */
  public String [] getAutoDownloadGroupNames () throws Exception {
    String [] result = null;

    // check to see if the jserver data directory is present
    // and if the auto-download dir is also present
    if (dataDir.exists() && dataDir.isDirectory() &&
	autoDlDir.exists() && autoDlDir.isDirectory()) {
      File [] files = autoDlDir.listFiles(autoDlRequestFileFilter);
      if (files != null && files.length > 0) {
	result = new String [files.length];
	for (int i = 0; i < files.length; i++)
	  result[i] = files[i].getName();
      }
    }

    return result;
  }

  /** get the list of name and comments of all the auto-download groups */
  public String [][] getAutoDownloadGroupNamesAndComments () throws Exception {
    String [][] result = new String[0][0];

    if (dataDir.exists() && dataDir.isDirectory() &&
	autoDlDir.exists() && autoDlDir.isDirectory()) {
      File [] files = autoDlDir.listFiles(autoDlRequestFileFilter);
      if (files != null && files.length > 0) {
	result = new String [files.length][2];
	for (int i = 0; i < files.length; i++) {
	  result[i][0] = files[i].getName();
	  result[i][1] = readMaintenanceRequest(files[i]).getComment();
	}
      }
    }

    return result;
  }

  /** get the auto-download configuration (MaintenanceRequest object)
   * for the given regid */
  public MaintenanceRequest getAutoDownloadConfig (String regid, boolean isGroup) throws Exception {
    if (!dataDir.exists() || !dataDir.isDirectory())
      throw new BridgeException("Data does not exist (" + dataDir.getPath() + ")");
    if (!autoDlDir.exists() || !autoDlDir.isDirectory())
      throw new BridgeException("Data does not exist (" + autoDlDir.getPath() + ")");

    File mf = null;
    if (isGroup)
      mf = new File(autoDlDir + File.separator + regid);
    else
      mf = new File(autoDlRequestDir + File.separator + regid);

    MaintenanceRequest mr = null;
    if (!mf.exists() || !mf.isFile()) {
      if (isGroup)
	throw new BridgeException("Auto-download Configuration for " + regid + " does not exist \n(" + mf.getPath() + ")");
      else {
	// see if we have to return a group configuration
	JServer.printDebug("Looking up auto-download group configuration for " + regid, JServer.DEBUG_VERBOSE);
        //	mr = AutoDownload.getMaintenanceRequest(this, regid, true);
	if (mr == null)
	  throw new BridgeException("Auto-download Configuration for " + regid + " does not exist \n(" + mf.getPath() + ")");
      }
    } else {
      try {
	mr = readMaintenanceRequest(mf);
      } catch (ClassNotFoundException cnfe) {
	throw new BridgeException("Internal error: " + cnfe.getMessage(), cnfe.toString());
      } catch (InvalidClassException ice) {
	throw new BridgeException("Incompatible software: " + ice.getMessage(), ice.toString());
      } catch (StreamCorruptedException sce) {
	throw new BridgeException("Incompatible software: " + sce.getMessage(), sce.toString());
      } catch (OptionalDataException ode) {
	throw new BridgeException("Incompatible software: " + ode.getMessage(), ode.toString());
      } catch (IOException ie) {
	throw new BridgeException("Error reading auto-download configuration information: " + ie.getMessage(), ie.toString());
      }
    }

    if (!regid.equals(mr.getName()))
      mr.setName(regid);

    return mr;
  }

  /** store the auto-download configuration (the regid for which this
   * config applies, is the name of the MaintenanceRequest object) */
  public boolean storeAutoDownloadConfig (MaintenanceRequest mr, boolean isGroup) throws Exception {
    File storePath = autoDlRequestDir;
    if (isGroup)
      storePath = autoDlDir;
		 
    // create the directories if not there
    if (!storePath.exists()) {
      boolean status = false;
      StringBuffer sb = new StringBuffer();
      try {
	status = storePath.mkdirs();
      } catch (Exception e) {
	status = false;
	sb.append(": ");
	sb.append(e.toString());
      }
      if (status == false)
	throw new BridgeException("Error creating auto-download storage directory", sb.toString());
    }

    // write the file
    File mf = new File(storePath + File.separator + mr.getName());
    try {
      ObjectOutputStream oos = new ObjectOutputStream(new WeakEncryptionOutputStream(new FileOutputStream(mf), encryptKey));
      oos.writeObject(mr);
      oos.flush();
      oos.close();
    } catch (IOException ie) {
      throw new BridgeException("Error writing file: " + ie.getMessage(), ie.toString());
    }

    return true;			
  }


  /** delete the auto-download configuration */
  public boolean deleteAutoDownloadConfig (String regid, boolean isGroup) throws Exception {
    if (!dataDir.exists() || !dataDir.isDirectory())
      throw new BridgeException("Data does not exist (" + dataDir.getPath() + ")");
    if (!autoDlDir.exists() || !autoDlDir.isDirectory())
      throw new BridgeException("Data does not exist (" + autoDlDir.getPath() + ")");
    if (!isGroup &&
	(!autoDlRequestDir.exists() || !autoDlRequestDir.isDirectory()))
      throw new BridgeException("Data does not exist (" + autoDlRequestDir.getPath() + ")");

    //--------------need to delete all associated configs
    File mf = null;
    if (isGroup)
      mf = new File(autoDlDir + File.separator + regid);
    else
      mf = new File(autoDlRequestDir + File.separator + regid);
    if (!mf.exists() || !mf.isFile())
      throw new BridgeException("Auto-download configuration does not exist (" + mf.getPath() + ")");
    mf.delete();
    return true;
  }

  /** is the auto-download session for this regid currently active? */
  public boolean isAutoDownloadActive (String regid) throws Exception {
    //    return AutoDownload.isActive(regid);
    return false;
  }

  /** abort the active auto-download session of this regid */
  public void abortAutoDownload (String regid) throws Exception {
    //    AutoDownload.abort(regid);
  }

  /** return a File handle for the auto-download log file for this regid */
  public File getAutoDownloadLogFile (String regid) throws Exception {
    if (!dataDir.exists() || !dataDir.isDirectory())
      throw new BridgeException("Data does not exist (" + dataDir.getPath() + ")");

    if (!autoDlLogDir.exists() || !autoDlLogDir.isDirectory())
      throw new BridgeException("Data does not exist (" + autoDlLogDir.getPath() + ")");

    File mf = new File(autoDlLogDir + File.separator + regid);
    if (!mf.exists() || !mf.isFile())
      throw new BridgeException("Auto-download log file does not exist (" + mf.getPath() + ")");
    if (!mf.canRead())
      throw new BridgeException("Cannot read auto-download log file (" + mf.getPath() + ")");

    return mf;
  }

  /** delete the auto-download log file */
  public boolean deleteAutoDownloadLogFile (String regid) throws Exception {
    if (!dataDir.exists() || !dataDir.isDirectory())
      throw new BridgeException("Data does not exist (" + dataDir.getPath() + ")");

    if (!autoDlLogDir.exists() || !autoDlLogDir.isDirectory())
      throw new BridgeException("Data does not exist (" + autoDlLogDir.getPath() + ")");

    File mf = new File(autoDlLogDir + File.separator + regid);
    if (!mf.exists() || !mf.isFile())
      throw new BridgeException("Auto-download log file does not exist (" + mf.getPath() + ")");
    mf.delete();
    return true;
  }

  public String isRegidCurrentlyActive (String regid) {
    //    return downloadServer.isRegidCurrentlyActive(regid);
    return null;
  }

  public int getMaintenanceRequestCount () {
    //    return downloadServer.getStartList().size();
    return 0;
  }

  /**
   * get the iedge parameters (IEdgeList object) for the given phone
   *
   * @param phone the phone number
   */
  public IEdgeList getIedgeParamsForPhone (String phone) throws Exception {
    // validate the phone number
    if (phone == null ||
	phone.length() == 0 ||
	phone.length() >= getPhoneNumLen())
      throw new BridgeException("Invalid phone number length");

    IEdgeList il = null;
    try {
      synchronized (lockObject) {
	il = getIedgeListForPhone(phone);
      }
    } catch (NoSuchMethodError ne) {
      throw new BridgeException("Software mismatch: " + ne.getMessage(), ne.toString());
    }

    if (il == null)
      throw new NotProvisionedBridgeException();

    return il;
  }

  /**
   * get the iedge parameters (IEdgeList object) for the given phone
   *
   * @param phone the vpn phone number
   * @param extLen the extension length
   */
  public IEdgeList getIedgeParamsForVpnPhone (String phone, int extLen) throws Exception {
    // validate the phone number
    if (phone == null ||
	phone.length() == 0 ||
	phone.length() >= getVpnNumLen())
      throw new BridgeException("Invalid VPN phone number length");

    IEdgeList il = null;
    try {
      synchronized (lockObject) {
	il = getIedgeListForVpnPhone(phone, extLen);
      }
    } catch (NoSuchMethodError ne) {
      throw new BridgeException("Software mismatch: " + ne.getMessage(), ne.toString());
    }

    if (il == null)
      throw new NotProvisionedBridgeException();

    return il;
  }

  /**
   * get the calling plan route parameters (CallPlanRoute object) for the given route name
   *
   * @param routeName
   */
  public CallPlanRoute getCallingPlanRoute (String routeName) throws Exception {
    // validate the phone number
    if (routeName == null ||
	routeName.length() == 0 ||
	routeName.length() >= getCallingPlanRouteLen())
      throw new BridgeException("Invalid Calling plan route name");

    CallPlanRoute route = new CallPlanRoute("", "", 0, "", 0, "", "", 0, 0);
    try {
      synchronized (lockObject) {
	if (getCallingPlanRouteImpl(routeName, route) == false)
	  throw new NotProvisionedBridgeException();
      }
    } catch (NoSuchMethodError ne) {
      throw new BridgeException("Software mismatch: " + ne.getMessage(), ne.toString());
    }

    return route;
  }


  /**
   * get the presence number. It gets the destination number's presence number
   *
   * @param destPhone
   */
  public String getPresence (String destPhone) throws Exception {
    // validate the phone number
    if (destPhone == null ||
	destPhone.length() == 0 ||
	destPhone.length() >= getPhoneNumLen())
      throw new BridgeException("Invalid destination phone number");

    String presence = null;
    try {
      synchronized (lockObject) {
	presence = getPresenceNumber(destPhone);
      }
    } catch (NoSuchMethodError ne) {
      throw new BridgeException("Software mismatch: " + ne.getMessage(), ne.toString());
    }

    if (presence == null) 
      throw new NotProvisionedBridgeException();

    return presence;
  }


  // class that deletes the given file after a certain delay
  private class DelayedFileDeleter implements QueueProcessor.DataProcessor {
    public void processData (Object data) {
      JServer.printDebug("deleting imported file: " + ((File)data).getName(), JServer.DEBUG_NORMAL);
      ((File)data).delete();
    }
  }

  public boolean importDB (boolean isLocal, int command, String dbName, File db) throws BridgeException {
    JServer.printDebug("importing " + dbName + " ...", JServer.DEBUG_VERBOSE);

    // create a backup copy of the current db
    JServer.printDebug("creating backup copy of current DB...", JServer.DEBUG_VERBOSE);
    File backupFile = exportDB(false, Constants.DB_DEFAULT_NAME, Constants.LIST_ALL,null);

    boolean ret = false;
    BridgeException importException = null;
    try {
      ret = importDatabase(command, Constants.DB_DIR + File.separator + dbName);
    } catch (BridgeException e) {
      ret = false;
      importException = e;
    }

    // delete the database file that was imported from iview
    if (isLocal)
      fileDeleteQueue.add(new File(Constants.DB_DIR + File.separator + dbName), System.currentTimeMillis() + Constants.DB_IMPORT_FILE_DELETE_TIME);

    if (!ret) {
      JServer.printDebug("importing database " + dbName + " failed, restoring old database", JServer.DEBUG_ERROR);
      if (importException != null)
	JServer.printDebug(importException, JServer.DEBUG_ERROR);
      // restore the backup copy
      try {
	ret = importDatabase(Constants.IMPORT_ALL, Constants.DB_DIR + File.separator + Constants.DB_DEFAULT_NAME);
	if (!ret)
	  throw new BridgeException("Error importing backup database", "No further messages");
      } catch (Exception ie) {
	JServer.printException("could not import backup database", ie, JServer.DEBUG_ERROR);
	backupFile.delete();
	if (importException == null)
	  importException = new BridgeException("Error importing database, could not restore previous database");
	else
	  importException.addDetails("Could not restore previous database");
	importException.addDetails(ie.toString());
	throw importException;
      }
    }

    // remove the backup file
    backupFile.delete();

    return ret;
  }

  public boolean importDatabase (int cmd, String dbName) throws BridgeException {
    boolean ret = false;
    boolean isCreate  =  false;
    String [] command = {"db", "create", dbName};
    String fileExt  = ".gdbm";

    switch (cmd) {
    case  Constants.IMPORT_ALL:
      command[1]  =  "create";
      isCreate  =  true;
      break;

    case  Constants.IMPORT_ADD:
      command[1]  =  "add";
      break;

    case Constants.IMPORT_DELETE:
      command[1]  =  "delete";
      break;

    case Constants.IMPORT_REPLACE:
      command[1]  =  "replace";
      break;

    default:
      throw new BridgeException("Unknown database import command (" + cmd + ")");
    }

    BridgeException be = null;

    try{
      ret = processCommand(3, command);
      JServer.printDebug("importDatabase: ret = " + ret, JServer.DEBUG_VERBOSE);

      if (ret && isCreate) {
        command[1] = "copy";
        ret = processCommand(3, command);
        JServer.printDebug("importDatabase: copy ret = " + ret, JServer.DEBUG_VERBOSE);
      }


    }catch(BridgeException exp){
       be = exp;
    }catch(Exception e){

     
      be =  new BridgeException(e.toString());
    }finally{
      // clean up the files
      JServer.printDebug("removing the temporary files", JServer.DEBUG_VERBOSE);

      for (int i = 0; i < Constants.dbFileExts.length; i++)
      {
        new File(dbName + dbFileExts[i]).delete();
      }
    }
    if(be != null)
      throw be; 
      return ret;
  }

  /**
   * export the current active database to the given filename
   *
   * @param isLocal is the exported file to be sent to iView (true means to be sent to iView,
   * false means stored on the iServer)
   * @param dbName the name of the file to export the db to
   * @param sock not used
   */
  public File exportDB (boolean isLocal, String dbName, int listType,Socket sock) throws BridgeException {
    JServer.printDebug("exportDB: exporting iserver database to " + dbName, JServer.DEBUG_VERBOSE);

    // create the default database dir if needed
    createDbDirectory();

    String dbFileName = Constants.DB_DIR + File.separator + dbName;

    String [] command = new String[2];
    command[0] = "db";
    command[1] = "export";
    
    if( !((listType&LIST_ALL) ==  Constants.LIST_ALL) ) { 
	if ((listType&IEDGE_ONLY) ==  Constants.IEDGE_ONLY) 
	    command = (String [])SysUtil.createObjectArray(command, Constants.IEDGE );
	if((listType&IEDGE_VPN_ONLY) ==  Constants.IEDGE_VPN_ONLY) 
	    command = (String [])SysUtil.createObjectArray(command, Constants.VPN );
	if( (listType&IEDGE_GROUP_ONLY) ==  Constants.IEDGE_GROUP_ONLY) 
	    command = (String [])SysUtil.createObjectArray(command, Constants.VPNG );
	if( (listType&CALLING_PLAN_ONLY) ==  Constants.CALLING_PLAN_ONLY) 
	    command = (String [])SysUtil.createObjectArray(command, Constants.CP );
	if( (listType&CALLING_PLAN_BIND_ONLY) ==  Constants.CALLING_PLAN_BIND_ONLY) 
	    command = (String [])SysUtil.createObjectArray(command, Constants.CPB );
	if( (listType&CALLING_PLAN_ROUTE_ONLY) ==  Constants.CALLING_PLAN_ROUTE_ONLY) 
	    command = (String [])SysUtil.createObjectArray(command, Constants.CR);
	if( (listType&TRIGGER_ONLY) ==  Constants.TRIGGER_ONLY) 
	    command = (String [])SysUtil.createObjectArray(command, Constants.TRIGGER);
	if( (listType&IGRP_ONLY) ==  Constants.IGRP_ONLY) 
	    command = (String [])SysUtil.createObjectArray(command, Constants.IGRP );
	if( (listType&DYNAMIC_CPR_ONLY) ==  Constants.DYNAMIC_CPR_ONLY) 
	    command = (String [])SysUtil.createObjectArray(command, Constants.DCR );
    }
    command = (String [])SysUtil.createObjectArray(command, dbFileName);

    boolean ret = processCommand(command.length, command);

    JServer.printDebug("exportDB: export command status = " + ret, JServer.DEBUG_VERBOSE);

    File exportFile = null;

    try{
      if (ret) {
        exportFile = new File( dbFileName );
        JServer.printDebug("exportDB: export file length = " + exportFile.length(), JServer.DEBUG_VERBOSE);
      } else
        throw new BridgeException("Error in exporting the database", "No further messages");

      if (exportFile != null && !exportFile.exists())
	  throw new BridgeException("Error in exporting the database", "Exported file does not exist!");
    }catch(Exception e){
      try{
        if(exportFile !=  null)
          SysUtil.deleteFile(exportFile);
      }catch(Exception exp){}

      JServer.printException("exportDB: Exception ocured while exporting the dataabse", e,JServer.DEBUG_ERROR);      
      throw new BridgeException(e.toString());
    }

    return exportFile;
  }


  private void filterDB(String parsedDbName, String  dbName, int listType) throws Exception{

    boolean isRootStart = false;
    boolean writeData   = false;
    boolean validTag    = false;
    boolean newLine     = false;  

    //  no filtering. copy the whole file.
    if(listType ==  Constants.LIST_ALL){
      SysUtil.copyFile(dbName,parsedDbName);
      return;
    }

    BufferedWriter bw = new BufferedWriter(new FileWriter(parsedDbName));

    BufferedReader br = new BufferedReader(new FileReader(dbName));
    String line;


    while ((line  = br.readLine()) !=  null) {
      writeData = false;
      newLine   = false;
      //  Filter the data
      if(isRootStart){
        //  Endpoints only
        if ((listType&IEDGE_ONLY) ==  Constants.IEDGE_ONLY) 
        {
          if(line.equals(Constants.IEDGE_START_TAG) ||  validTag){
            writeData = true;
            validTag  = true;
          }
          if(line.equals(Constants.IEDGE_END_TAG)){
            newLine = true;
            validTag  = false;
          }
        }
        //  vpn only
        if((listType&IEDGE_VPN_ONLY) ==  Constants.IEDGE_VPN_ONLY) 
        {
          if(line.equals(Constants.VPN_START_TAG) ||  validTag){
            writeData = true;
            validTag  = true;
          }
          if(line.equals(Constants.VPN_END_TAG)){
            newLine = true;
            validTag  = false;
          }
        }
        //  vpn groups only
        if( (listType&IEDGE_GROUP_ONLY) ==  Constants.IEDGE_GROUP_ONLY) 
        {
          if(line.equals(Constants.GROUP_START_TAG) ||  validTag){
            writeData = true;
            validTag  = true;
          }
          if(line.equals(Constants.GROUP_END_TAG)){
            newLine = true;
            validTag  = false;
          }
        }
        //  calling plans only
        if( (listType&CALLING_PLAN_ONLY) ==  Constants.CALLING_PLAN_ONLY) 
        {
          if(line.equals(Constants.CP_START_TAG) ||  validTag){
            writeData = true;
            validTag  = true;
          }
          if(line.equals(Constants.CP_END_TAG)){
            newLine = true;
            validTag  = false;
          }
        }
        //  calling plan bindings only
        if( (listType&CALLING_PLAN_BIND_ONLY) ==  Constants.CALLING_PLAN_BIND_ONLY) 
        {
          if(line.equals(Constants.CPB_START_TAG) ||  validTag){
            writeData = true;
            validTag  = true;
          }
          if(line.equals(Constants.CPB_END_TAG)){
            newLine = true;
            validTag  = false;
          }
        }

        //  calling plan routes only
        if( (listType&CALLING_PLAN_ROUTE_ONLY) ==  Constants.CALLING_PLAN_ROUTE_ONLY) 
        {
          if(line.equals(Constants.CR_START_TAG) ||  validTag){
            writeData = true;
            validTag  = true;
          }
          if(line.equals(Constants.CR_END_TAG)){
            newLine = true;
            validTag  = false;
          }
        }


        // triggers only 
        if( (listType&TRIGGER_ONLY) ==  Constants.TRIGGER_ONLY) 
        {
          if(line.equals(Constants.TG_START_TAG) ||  validTag){
            writeData = true;
            validTag  = true;
          }
          if(line.equals(Constants.TG_END_TAG)){
            newLine = true;
            validTag  = false;
          }
        }

        //  calling plan routes only
        if( (listType&IGRP_ONLY) ==  Constants.IGRP_ONLY) {
          if(line.equals(Constants.IGRP_START_TAG) || validTag) {
            writeData = true;
            validTag  = true;
          }
          if(line.equals(Constants.IGRP_END_TAG)) {
            newLine = true;
            validTag  = false;
          }
        }
                // triggers only 
        if( (listType&TRIGGER_ONLY) ==  Constants.TRIGGER_ONLY) 
        {
	        if(line.equals(Constants.TG_START_TAG) ||  validTag){
		        writeData = true;
		        validTag  = true;
	        }
	        if(line.equals(Constants.TG_END_TAG)){
		        newLine = true;
		        validTag  = false;
	        }
        }
                // realm only 
        if( (listType&REALM_ONLY) ==  Constants.REALM_ONLY) 
        {
	        if(line.equals(Constants.RM_START_TAG) ||  validTag){
		        writeData = true;
		        validTag  = true;
	        }
	        if(line.equals(Constants.RM_END_TAG)){
		        newLine = true;
		        validTag  = false;
	        }
        }

                // vnet only 
        if( (listType&VNET_ONLY) ==  Constants.VNET_ONLY) 
        {
	        if(line.equals(Constants.RM_START_TAG) ||  validTag){
		        writeData = true;
		        validTag  = true;
	        }
	        if(line.equals(Constants.RM_END_TAG)){
		        newLine = true;
		        validTag  = false;
	        }
        }

        //  calling plan routes only
        if( (listType&IGRP_ONLY) ==  Constants.IGRP_ONLY) {
          if(line.equals(Constants.IGRP_START_TAG) || validTag) {
            writeData = true;
            validTag  = true;
          }
          if(line.equals(Constants.IGRP_END_TAG)) {
            newLine = true;
            validTag  = false;
          }
        }
      }

      //  unfiltered data
      if(!isRootStart   ||  line.equals(Constants.ROOT_END_TAG)){
        writeData = true;
      }

      //  root element
      if(!isRootStart && line.equals(Constants.ROOT_START_TAG)){
        isRootStart = true;
        writeData = true;
      }

      //  write the filtered data
      if(writeData){
        bw.write(line);
        bw.newLine();
        if(newLine)
          bw.newLine();
      }

    }
    br.close();
    bw.close();
  }


  public File createDbDirectory () throws BridgeException {
    try {
      return SysUtil.createDirectory(Constants.DB_DIR);
    } catch (IOException ie) {
      throw new BridgeException("Error creating default database directory '" + Constants.DB_DIR + "'", ie.toString());
    }
  }


  public String [] getDBFilenames () throws Exception {
    File dbDir = createDbDirectory();
    File [] dbFiles = dbDir.listFiles();
    if (dbFiles != null && dbFiles.length > 0) {
      String [] dbFileNames  = new String [dbFiles.length];
      for (int i = 0; i < dbFiles.length; dbFileNames[i]  = dbFiles[i++].getName());
      return dbFileNames;
    }

    return null;
  }


  private String getPath(String data) throws Exception {
    String path = "";
    int index = data.indexOf('\n');
    if (index != -1) {
      path = data.substring(0,index);
      path = path.trim();
    }
    return path;
  }


  private String [] createSyslogClassNames (String syslogClass) {
    String [] result = new String [4];

    StringBuffer sb = new StringBuffer(syslogClass);
    sb.append(".");
    sb.append(Constants.SYSLOG_ERROR_CLASS_SUFFIX);
    result[0] = sb.toString();

    sb.replace(0, sb.length(), syslogClass);
    sb.append(".");
    sb.append(Constants.SYSLOG_DEBUG_CLASS_SUFFIX);
    result[1] = sb.toString();

    sb.replace(0, sb.length(), syslogClass);
    sb.append(".");
    sb.append(Constants.SYSLOG_ERROR_CLASS_SUFFIX);
    sb.append(";");
    sb.append(syslogClass);
    sb.append(".");
    sb.append(Constants.SYSLOG_DEBUG_CLASS_SUFFIX);
    result[2] = sb.toString();

    sb.replace(0, sb.length(), syslogClass);
    sb.append(".");
    sb.append(Constants.SYSLOG_DEBUG_CLASS_SUFFIX);
    sb.append(";");
    sb.append(syslogClass);
    sb.append(".");
    sb.append(Constants.SYSLOG_ERROR_CLASS_SUFFIX);
    result[3] = sb.toString();

    return result;
  }


  /**
   * get the syslog paths
   * 
   * @param String the syslog class (i.e., local1, local2, etc)
   *
   * @return [0] is the errPath and [1] is the debugPath
   */
  private String [] getSyslogPath (String syslogClass) {
    String [] path = new String [2];
    path[0] = "";
    path[1] = "";
    StringBuffer sBuffer = new StringBuffer();
    try {
      BufferedReader br = new BufferedReader(new FileReader(Constants.SYSLOG_CONF_FILE));
      String line;
      while ((line  = br.readLine()) !=  null) {
	sBuffer.append(line + "\n");
      }
      String sysLogData = sBuffer.toString();

      String [] names = createSyslogClassNames(syslogClass);

      String err = names[0];
      String debug = names[1];
      String errdebug = names[2];
      String debugerr = names[3];

      String subData = sysLogData;

      int index;
      //path  found
      if ( (index = getPathIndex(debugerr, sysLogData)) !=  -1)  {
	subData = sysLogData.substring(index + debugerr.length());
	path[0] = getPath(subData);
	path[1] = path[0];
      } else if ( (index = getPathIndex(errdebug, sysLogData)) != -1){
	subData = sysLogData.substring(index + errdebug.length());
	path[0] = getPath(subData);
	path[1] = path[0];
      } else {
	if ( (index = getPathIndex(err, sysLogData)) != -1){
	  subData = sysLogData.substring(index + err.length());
	  path[0] = getPath(subData);
	}
	if ( (index = getPathIndex(debug, sysLogData)) != -1){
	  subData = sysLogData.substring(index + debug.length());
	  path[1] = getPath(subData);
	}
      }
    } catch (Exception ie) {
      JServer.printException("Error reading '" + Constants.SYSLOG_CONF_FILE + "'", ie, JServer.DEBUG_ERROR);
    }

    return path;
  }


  private int getPathIndex(String subData, String data){
    int index =0;
    while(true){
      index = data.indexOf(subData,index);
      if(index  ==  -1)
        break;
      int startIndex  = data.lastIndexOf("\n",index);
      if(isCommented(data.substring(startIndex,index))){
        index +=  subData.length();
        continue;
      }else
        break;
    }
    return index;
  }


  private boolean isCommented(String data){
    int index;
    if ((index = data.indexOf("#")) != -1)
      return true;
    return false;
  }


  /**
   * set the syslog paths
   */
  private void setSyslogPath (String [] path, String syslogClass) {
    if (path[0] == null || path[0].length() == 0) {
      JServer.printDebug("No error path specified", JServer.DEBUG_WARNING);
      path[0] = "";
    }
    if (path[1] == null || path[1].length() == 0) {
      JServer.printDebug("No debug path specified", JServer.DEBUG_WARNING);
      path[1] = "";
    }

    StringBuffer sBuffer = new StringBuffer();
    try {
      // create bkup
      Process p = Runtime.getRuntime().exec("cp " + Constants.SYSLOG_CONF_FILE + " " + Constants.SYSLOG_CONF_FILE + ".bak");  
      try {
	p.waitFor();
      } catch (InterruptedException ie) {}
      p.destroy();

      // read in file
      BufferedReader br = new BufferedReader(new FileReader(Constants.SYSLOG_CONF_FILE));
      String line = null;
      while ((line = br.readLine()) != null) {
	sBuffer.append(line + "\n");
      }
      String sysLogData = sBuffer.toString();

      String [] names = createSyslogClassNames(syslogClass);
      String err = names[0];
      String debug = names[1];
      String errdebug = names[2];
      String debugerr = names[3];

      String errPath = "";
      if (path[0].length() > 0)
	errPath = err + "\t\t\t\t\t" + path[0] + "\n";
      String debugPath = "";
      if (path[1].length() > 0)
	debugPath = debug + "\t\t\t\t\t" + path[1] + "\n";
        
      StringBuffer newBuff = new StringBuffer(sysLogData);
      int startIndex;
      int endIndex = 0;

      // replace if path  found
      if ( (startIndex = getPathIndex(debugerr, sysLogData)) !=  -1 ||
	   (startIndex = getPathIndex(errdebug, sysLogData)) !=  -1 ) {
	String newData = errPath + debugPath;
	endIndex = sysLogData.indexOf('\n', startIndex);
	newBuff.replace(startIndex, endIndex+1, newData);
      } else {
        if ( (startIndex = getPathIndex(err, sysLogData)) !=  -1) {
	  String newData = errPath;
	  endIndex = sysLogData.indexOf('\n', startIndex);
	  newBuff.replace(startIndex, endIndex+1, newData);
        } else
          newBuff.append(errPath); 
        sysLogData  = newBuff.toString();

        if ( (startIndex = getPathIndex(debug, sysLogData)) !=  -1) {
	  String newData = debugPath;
	  endIndex = sysLogData.indexOf('\n', startIndex);
	  newBuff.replace(startIndex, endIndex+1, newData);
	} else
          newBuff.append(debugPath);                    
      } 
        
      checkPath(path[0]);
      checkPath(path[1]);

      FileOutputStream  fos  = new FileOutputStream(Constants.SYSLOG_CONF_FILE);
      DataOutputStream dos = new DataOutputStream(fos);
      dos.writeBytes(newBuff.toString());
      dos.close();
      fos.close();

      // restart syslog
      p = Runtime.getRuntime().exec(Constants.SYSLOGD_STOP_COMMAND); 
      try {
	p.waitFor();
      } catch (InterruptedException ie) {}
      p.destroy();
      JServer.printDebug("syslog stopped", JServer.DEBUG_VERBOSE);

      p = Runtime.getRuntime().exec(Constants.SYSLOGD_START_COMMAND); 
      try {
	p.waitFor();
      } catch (InterruptedException ie) {}
      p.destroy();
      JServer.printDebug("syslog started", JServer.DEBUG_VERBOSE);

    } catch (Exception ie) {
      JServer.printException("Error writing '" + Constants.SYSLOG_CONF_FILE + "'", ie, JServer.DEBUG_ERROR);
    }			  
  }

  // creates the necessary directories and files
  private void checkPath (String path) throws Exception {
    if (path == null || path.length() == 0) {
      JServer.printDebug("checkPath: empty path being checked", JServer.DEBUG_WARNING);
      return;
    }

    File file = new File(path);
    if (!file.exists()) {
      int index = path.lastIndexOf(File.separator);
      if (index != -1)
	new File(path.substring(0, index)).mkdirs();
      file.createNewFile();
    }
  }

  /** return the iserver configuration */
  public iServerConfig getIserverConfig () throws Exception {
    iServerConfig cfg = getNativeIserverConfig();

    // set the logging parameters
    String [] path = getSyslogPath(Constants.DEBUG_LOG_SYSLOG_CLASS);
    JServer.printDebug("BridgeServer: sending iserver syslogpath = " + path[0] + " " + path[1], JServer.DEBUG_VERBOSE);
    cfg.getLoggingConfig().setsLogErrPath(path[0]);
    cfg.getLoggingConfig().setsLogDebugPath(path[1]);
    path = getSyslogPath(Constants.H323_LOG_SYSLOG_CLASS);
    JServer.printDebug("BridgeServer: sending h323 syslogpath = " + path[0] + " " + path[1], JServer.DEBUG_VERBOSE);
    cfg.getLoggingConfig().setH323LogPath(path[0]);


    // get the pool configuration
    String pcfg = null;
	  try {
      
	    pcfg = SysUtil.readFile("/usr/local/nextone/bin/pools.xml"); // get config from file
      JServer.printDebug("BridgeServer: sending pools data = "+pcfg, JServer.DEBUG_VERBOSE);
	  } catch (IOException ie) {
	    JServer.printException("Error reading pools XML file", ie, JServer.DEBUG_ERROR);
	    pcfg = "";
	  }
    cfg.getFceConfigNew().setPoolConfig(pcfg);

    // set the aravox configuration
/*  try {
      String acfg = null;
      if (cfg.getFceConfig().getFirewallName().equals("aravox"))
	      acfg = getAravoxConfig();  // get active config if firewall type is aravox
      else {
	      try {
	        acfg = SysUtil.readFile("/usr/local/nextone/bin/aravox.xml"); // get config from file
	      } catch (IOException ie) {
	        JServer.printException("Error reading aravox XML file", ie, JServer.DEBUG_ERROR);
	        acfg = "";
	      }
      }
      JServer.printDebug("BridgeServer: ** sending aravox cfg:", JServer.DEBUG_VERBOSE);
     // JServer.printDebug(acfg, JServer.DEBUG_VERBOSE);
      cfg.getFceConfig().setAravoxConfig(acfg);
    } catch (Exception e) {
      JServer.printException("Error retrieving aravox configuration", e, JServer.DEBUG_ERROR);
      throw new BridgeException("Error retrieving aravox configuration: " + e.getMessage(), e.toString());
    }

    // set the nsf configuration
    try {
      cfg.getFceConfig().getIpFilterConfig().setAvailableInterfaces(getInterfaceNames());
      Properties prop = new Properties();
      prop.load(new FileInputStream("/usr/local/nextone/bin/nsf.cfg"));
      String prStr = "";
      try {
	      prStr = prop.getProperty("fwPublicInterface");
	      if (prStr == null) {
	        JServer.printDebug("fwPublicInterface not found in nsf.cfg", JServer.DEBUG_WARNING);
	        prStr = "";
	      }
      	cfg.getFceConfig().getIpFilterConfig().setPublicInterface(prStr.trim());
      } catch (BridgeException ile) {
      	JServer.printException("Error reading public address configuration", ile, JServer.DEBUG_ERROR);
      }
      try {
	      prStr = prop.getProperty("fwPrivateInterface");
	      if (prStr == null) {
	        JServer.printDebug("fwPrivateInterface not found in nsf.cfg", JServer.DEBUG_WARNING);
	        prStr = "";
	      }
	      cfg.getFceConfig().getIpFilterConfig().setPrivateInterface(prStr.trim());
      } catch (BridgeException ile) {
      	JServer.printException("Error reading private address configuration", ile, JServer.DEBUG_ERROR);
      }

      prStr = prop.getProperty("mediaRouteInterfaces");
      if (prStr == null) {
	      JServer.printDebug("mediaRouteInterfaces not found in nsf.cfg", JServer.DEBUG_WARNING);
	      prStr = "";
      }
      StringTokenizer st = new StringTokenizer(prStr.trim(), ",");
      String [] mrifs = new String [0];
      while (st.hasMoreTokens()) {
      	String intf = st.nextToken().trim();
	      if (intf.length() > 0)
	        mrifs = SysUtil.createStringArray(mrifs, intf);
      }
      try {
      	cfg.getFceConfig().getIpFilterConfig().setMediaRouteInterface(mrifs);
      } catch (BridgeException ile) {
	      JServer.printException("Error reading media route address configuration", ile, JServer.DEBUG_ERROR);
      }

      prStr = prop.getProperty("localNatEnabled");
      if (prStr == null) {
	      JServer.printDebug("localNatEnabled not found in nsf.cfg", JServer.DEBUG_WARNING);
	      prStr = "";
      }
      cfg.getFceConfig().getIpFilterConfig().setNatEnabled(prStr.trim().equals("yes"));

      prStr = prop.getProperty("remoteNatEnabled");
      if (prStr == null) {
  	      JServer.printDebug("remoteNatEnabled not found in nsf.cfg", JServer.DEBUG_WARNING);
	        prStr = "";
      }
      cfg.getFceConfig().getIpFilterConfig().setPacketSteeringEnabled(prStr.trim().equals("yes"));

      JServer.printDebug("BridgeServer: sending nsf cfg:", JServer.DEBUG_VERBOSE);
      //JServer.printDebug(cfg.getFceConfig().getIpFilterConfig().toString(), JServer.DEBUG_VERBOSE);
    } catch (Exception e) {
      JServer.printException("Error reading ip filter configuration", e, JServer.DEBUG_ERROR);
      throw new BridgeException("Error reading IP filter configuration: " + e.getMessage(), e.toString());
    }
*/

    // TODO - seperate this from ipfilter config
    cfg.getFceConfig().getIpFilterConfig().setAvailableInterfaces(getInterfaceNames());
	
    JServer.printDebug(cfg.toString(), JServer.DEBUG_VERBOSE);

    return cfg;
  }

  /** set/update the iserver configuration */
  public boolean setIserverConfig (iServerConfig cfg) throws Exception {
    
    // set the syslog path
    String [] path = getSyslogPath(Constants.DEBUG_LOG_SYSLOG_CLASS);
    String debugPath = cfg.getLoggingConfig().getsLogDebugPath();
    String errPath = cfg.getLoggingConfig().getsLogErrPath();
    JServer.printDebug("BridgeServer: received debug syslogpath = " + debugPath + " " + errPath, JServer.DEBUG_VERBOSE);

    if (!errPath.equals(path[0])  || !debugPath.equals(path[1])) {
      path = new String [2];
      path[0] = errPath;
      path[1] = debugPath;
      try {
	      setSyslogPath(path, Constants.DEBUG_LOG_SYSLOG_CLASS);
      } catch (Exception e) {
	      throw new BridgeException("Error setting debug syslog path: " + e.getMessage(), e.toString());
      }
    }

    // set the h323 log path
    path = getSyslogPath(Constants.H323_LOG_SYSLOG_CLASS);
    String h323Path = cfg.getLoggingConfig().getH323LogPath();
    JServer.printDebug("BridgeServer: received h323 syslogpath = " + h323Path, JServer.DEBUG_VERBOSE);

    if (!h323Path.equals(path[0])) {
      path = new String [2];
      path[0] = h323Path;
      path[1] = h323Path;
      try {
        setSyslogPath(path, Constants.H323_LOG_SYSLOG_CLASS);
      } catch (Exception e) {
        throw new BridgeException("Error setting h323 syslog path: " + e.getMessage(), e.toString());
      }
    }

    // if the cdr format is set to syslog, create entries in syslog.conf, enable
    // cdr logging and restart syslogd
    handleCdrSyslog(cfg);

    // set the pool configuration
    String poolCfg = (String)cfg.getFceConfigNew().getPoolConfig().getXMLString();
    String filename = "/usr/local/nextone/bin/pools.xml";
    JServer.printDebug("BridgeServer: received pools configuration...", JServer.DEBUG_VERBOSE);
    JServer.printDebug(poolCfg, JServer.DEBUG_VERBOSE);
    try {
      FileOutputStream fos  = new FileOutputStream(filename);
      DataOutputStream dos = new DataOutputStream(fos);
      dos.writeBytes(poolCfg);
      dos.close();
      fos.close();
    } catch (Exception e) {
      throw new BridgeException("Error setting pool configuration: " + e.getMessage(), e.toString());
    }

    // set the aravox config
/*  String aravoxCfg = (String)cfg.getFceConfig().getAravoxConfig().getXMLStringWithNoStats();
    String filename = "/usr/local/nextone/bin/aravox.xml";
    JServer.printDebug("BridgeServer: received aravox configuration...", JServer.DEBUG_VERBOSE);
    JServer.printDebug(aravoxCfg, JServer.DEBUG_VERBOSE);
    try {
      FileOutputStream fos  = new FileOutputStream(filename);
      DataOutputStream dos = new DataOutputStream(fos);
      dos.writeBytes(aravoxCfg);
      dos.close();
      fos.close();
    } catch (Exception e) {
      throw new BridgeException("Error setting aravox configuration: " + e.getMessage(), e.toString());
    }

    // set the nsf config
    Properties prop = new Properties();
    prop.setProperty("fwPrivateInterface", cfg.getFceConfig().getIpFilterConfig().getPrivateInterface());
    prop.setProperty("fwPublicInterface", cfg.getFceConfig().getIpFilterConfig().getPublicInterface());
    prop.setProperty("localNatEnabled", cfg.getFceConfig().getIpFilterConfig().isNatEnabled()?"yes":"no");
    prop.setProperty("remoteNatEnabled", cfg.getFceConfig().getIpFilterConfig().isPacketSteeringEnabled()?"yes":"no");
    String [] mrifs = cfg.getFceConfig().getIpFilterConfig().getMediaRouteInterface();
    String mrif = "";
    for (int i = 0; i < mrifs.length; i++) {
      if (i != 0)
      	mrif += ",";
      mrif += mrifs[i];
    }
    prop.setProperty("mediaRouteInterfaces", mrif);
    try {
      SysUtil.copyFile("/usr/local/nextone/bin/nsf.cfg", "/usr/local/nextone/bin/nsf.cfg.prev");
    } catch (IOException ie) {
      JServer.printException("Error backing up IP filter configuration: " + ie.getMessage(), ie, JServer.DEBUG_ERROR);
    }
    try {
      prop.store(new FileOutputStream("/usr/local/nextone/bin/nsf.cfg"), "Generated by iView");
    } catch (IOException ie) {
      JServer.printException("Error storing IP filter configuration: " + ie.getMessage(), ie, JServer.DEBUG_ERROR);
    }
*/
    // set the server.cfg stuff
    return  setNativeIserverConfig(cfg);
  }

  public void handleCdrSyslog (iServerConfig cfg) {
    if (cfg.getBillingConfig().getCdrFormat() != iServerConfig.CDRFORMAT_SYSLOG)
      return;

    try {
      String [] path = getSyslogPath(Constants.CDR_LOG_SYSLOG_CLASS);
      path[0] = "";
	    path[1] = cfg.getBillingConfig().getDirectory();

      String fileName = "";
      if((path[1].substring(path[1].length()-1)).equals(File.separator))
        fileName  = Constants.CDR_FILE_NAME;
      else
        fileName  = File.separator+Constants.CDR_FILE_NAME;
      path[1] +=  fileName;

	    JServer.printDebug("updating cdr path:  " + path[1]  , JServer.DEBUG_VERBOSE);
	    setSyslogPath(path, Constants.CDR_LOG_SYSLOG_CLASS);
	    JServer.printDebug("updating path[1]" +path[1], JServer.DEBUG_VERBOSE);
    } catch (Exception e) {
      JServer.printException("Error updating " + Constants.SYSLOG_CONF_FILE + " to include cdr log class", e, JServer.DEBUG_ERROR);
    }

    // enable cdr logging in gis
    cfg.getLoggingConfig().setGisModuleEnabled(iServerConfig.CDR, true);
  }

  public synchronized boolean isDbOperationInProgress () {
    return dbOp;
  }

  public synchronized void setDbOperationInProgress (boolean newval) {
    dbOp = newval;
  }

  public int[][] getLicenseAlarms() {
    if (isDbOperationInProgress() == false) {
      licenseAlarms[0] = getIServerVportLicenseAlarms();
      licenseAlarms[1] = getIServerMRVportLicenseAlarms();
    } else {
      JServer.printDebug("Reusing previously stored vport alarm values", JServer.DEBUG_VERBOSE);
    }
	
    return licenseAlarms;
  }

  public boolean clearLicenseAlarms() throws Exception{
     String [] command = new String[2];
     command[0] = "lsalarm";
     command[1] = "clear";
     return processCommand(command.length, command);
  }


    /** Get the maximum number of calls **/
  public int getMaxCalls() throws Exception{
      return getMaximumCalls();
  }

    /** Get the maximum MR number of calls **/
  public int getMaxMRCalls() throws Exception{
      return getMaximumMRCalls();
  }

  public boolean clearLog (int logFileType) throws Exception {
    File [] logfiles = new File [0];

    switch (logFileType) {
    case JSERVER_LOGFILE:
      logfiles = new File [1];
      logfiles[0] = JServer.getLogFile();
      break;

    case ISERVER_LOGFILE:
      String [] path = getSyslogPath(Constants.DEBUG_LOG_SYSLOG_CLASS);
      logfiles = new File[((path[0].length() == 0)?0:1) + ((path[1].length() == 0)?0:1)];
      int i = 0;
      if (path[0].length() != 0)
	logfiles[i++] = new File(path[0]);
      if (path[1].length() != 0)
	logfiles[i] = new File(path[1]);
      break;

    default:
      throw new BridgeException("This version of iServer has no support to clear that log");
    }

    // clear the log files
    for (int i = 0; i < logfiles.length; i++) {
      FileOutputStream fos = new FileOutputStream(logfiles[i]);
      fos.write("\n".getBytes());
      fos.close();
    }

    return true;
  } 

  public boolean processCommands(Commands cmds) throws Exception{
    Exception   exp = null;
    String[]  cmd = null;
    boolean ret = true;
    boolean isExist = false;
    int cmdCount = 0;
    while(cmds.hasMoreCommands() && ret){
      cmd  = cmds.nextCommand();
      if(cmd  !=  null){
        try {
    	    ret =   processCommand(cmd.length, cmd);
        }catch(ExistException ie){
          //if it exist alreay ignore it
          if(cmds.isAddCommand() && cmdCount<= 0) {
            JServer.printDebug(getDetails(cmd)+" (Duplicate entry exists ignoring)",JServer.DEBUG_ERROR);
            isExist = true;
          }
          ret = false;
          exp = ie;
        }catch (NoEntryException nee){
         // if the  command is delete and the entry is not existinf ignore it
         if(cmds.isDeleteCommand()){
            JServer.printDebug(getDetails(cmd)+" (Entry  not exists ignoring)",JServer.DEBUG_ERROR);
            ret = true;
         }
         else{
           exp = nee;
           ret = false;
         }
        }catch (Exception e) {
          exp = e;
          ret = false;
        }
      }
      cmdCount++;
    }
    if(!ret){

      if(cmds.isAddCommand() && !isExist){
        cmd=   cmds.deleteCommand();
        if(cmd  !=  null){
          try{
    	      ret =   processCommand(cmd.length, cmd);
          }catch(Exception e){
//	          JServer.printDebug(getDetails(cmd)+" failed("+ e.toString() +")",JServer.DEBUG_ERROR);
          }
        }
      }
      try{
        JServer.printException(getDetails(cmd)+" failed",exp,JServer.DEBUG_ERROR);
      }catch(Exception e){
      }

      if(exp  !=  null)
        throw exp;
    }
    return ret;
  }


  public boolean processBulkCommands(Commands[] cmds) throws Exception{
    boolean ret = false;
    for(int i=0; i < cmds.length; i++)
      ret |=  processCommands(cmds[i]);
    return ret;
  }



  public String getDetails(String[] cmd){
      StringBuffer details = new StringBuffer("processing command =");
      for(int i=0; i < cmd.length; i++){
        details.append(cmd[i]);
        details.append(":");
      }
      return details.toString();
  }

  public int getMaximumRecords(int listType){
    int maxRecords  = 0;
    if ((listType&IEDGE_ONLY) ==  Constants.IEDGE_ONLY) 
      maxRecords  +=  getIEdgeCount();

    if((listType&IEDGE_VPN_ONLY) ==  Constants.IEDGE_VPN_ONLY) 
      maxRecords  +=  getVpnCount();

    if((listType&IEDGE_GROUP_ONLY) ==  Constants.IEDGE_GROUP_ONLY) 
      maxRecords  +=  getVpnGroupCount();

    if((listType&CALLING_PLAN_ONLY) ==  Constants.CALLING_PLAN_ONLY) 
      maxRecords  +=  getCallingPlanCount();

    if( (listType&CALLING_PLAN_BIND_ONLY) ==  Constants.CALLING_PLAN_BIND_ONLY) 
      maxRecords  +=  getCallingPlanBindingCount();

    if( (listType&CALLING_PLAN_ROUTE_ONLY) ==  Constants.CALLING_PLAN_ROUTE_ONLY) 
      maxRecords  +=  getCallRouteCount();

    if( (listType&TRIGGER_ONLY) ==  Constants.TRIGGER_ONLY) 
      maxRecords  +=  getTriggerCount();

    if( (listType&REALM_ONLY) ==  Constants.REALM_ONLY) 
      maxRecords  +=  getRealmCount();

    if( (listType&VNET_ONLY) ==  Constants.VNET_ONLY) 
      maxRecords  +=  getVnetCount();

    if( (listType&IGRP_ONLY) ==  Constants.IGRP_ONLY) 
      maxRecords  +=  getIGRPCount();

    return maxRecords;
  }

  public Capabilities getCapabilities() throws Exception{
    // get the iserver capabilities from native code
    Capabilities cap  =  getIServerCapabilities();
    // set the interfacenames
    cap.setInterfaceNames(getInterfaceNames());
    // set realm enable
    cap.setRealmEnabled(true);
    
    cap.setCapabilitiesEnabled(true);

    cap.getSipCapability().setSipDomainEnabled(false);

    String os = System.getProperties().getProperty("os.name");
    boolean isLinux = !(os.indexOf("LINUX")<0 && os.indexOf("Linux")<0);

    cap.setLinux(isLinux);
    
    if(cap.getFCE().isHotKnifeEnabled()) {
	boolean hotKnifeEnabled = false;	
	try{
	    String command = Constants.HOTKNIFE_DETECT_COMMAND_LINUX;
	    String gstring = Constants.HOTKNIFE_GREP_STRING_LINUX;
	    if(!isLinux){
		command = Constants.HOTKNIFE_DETECT_COMMAND;
		gstring = Constants.HOTKNIFE_GREP_STRING;
	    }
	    Process p = Runtime.getRuntime().exec(command);
	    BufferedReader br = new BufferedReader(new InputStreamReader(new SequenceInputStream(p.getInputStream(), p.getErrorStream())), 1024);
	    String line = null;
	    while((line = br.readLine()) != null) {		 		
		if(!hotKnifeEnabled) {
		    hotKnifeEnabled = (line.indexOf(gstring) == -1) ? false : true;
		}
		else
		    continue;
	    }

	    try {
		p.waitFor();
	    } catch (InterruptedException ie) {}
	    p.destroy();
	} catch (IOException ie) {
	    System.err.println("Unable to detect HotKnife: " + ie.getMessage());
	}
   
	cap.getFCE().setHotKnifeEnabled(hotKnifeEnabled);
    }

    JServer.printDebug("/--------------------"+ cap.toString(), JServer.DEBUG_NORMAL);
    return cap;
  }
}
