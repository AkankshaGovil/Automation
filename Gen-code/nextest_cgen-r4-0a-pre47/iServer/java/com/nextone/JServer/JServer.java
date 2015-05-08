package com.nextone.JServer;

import java.net.*;
import java.util.*;
import java.io.*;
import com.nextone.util.DeltaTime;
import com.nextone.util.IPUtil;
import com.nextone.util.LimitedDataInputStream;
import com.nextone.util.NextoneMulticastSocket;
import com.nextone.util.UDPServer;
import com.nextone.util.UDPServerWorker;
import com.nextone.util.SysUtil;
import com.nextone.common.Bridge;
import com.nextone.common.BridgeException;
import com.nextone.common.IEdgeList;
import com.nextone.common.iServerConfig;
import com.nextone.common.MaintenanceGroup;
import com.nextone.common.MaintenanceRequest;
import com.nextone.common.OperationUnknownBridgeException;
import com.nextone.common.ProvisionData;
import com.nextone.common.RedundInfo;
import com.nextone.common.RegidPortPair;
import com.nextone.common.Registration;
import com.nextone.common.SingleInstance;
import com.nextone.common.Commands;
import com.nextone.common.Capabilities;


public class JServer extends UDPServer implements Constants {
  static {
    System.loadLibrary("BridgeServer");
  }

  private static SingleInstance instance;
  private static DatagramSocket ds;
  private static boolean keepRunning;
  private static InetAddress [] allLocalAddr;
  private static BridgeServer bs;
  public static final int NOT_ALLOWED = 0;
  public static final int READ_ALLOWED = 1;
  public static final int WRITE_ALLOWED = 2;
  public static final int READ_WRITE_ALLOWED = 3;
  private static int debugLevel = 10;  // allow all until a level is set
  private static String readPass;
  private static String writePass;
  private static String logfile;
  public static int compression;
  private static String curDir;
  private final static long startTime = System.currentTimeMillis();
  private static JServer thisInstance;
  private boolean nativeInitDone = false;

  // debug levels (corresponding definitions are also in BridgeServerImpl.c)
  public static final int DEBUG_OFF = 0;
  public static final int DEBUG_ERROR = 1;  // prints errors during ops
  public static final int DEBUG_NORMAL = 2;  // prints normal ops
  public static final int DEBUG_WARNING = 3;  // prints warnings during ops
  public static final int DEBUG_VERBOSE = 4;  // verbose printing
 
  public static final String CONFIG_FILE  = "/usr/local/nextone/bin/server.cfg"; 
  public static final String MGMT_IP  = "mgmt_ip"; 

  public static final String WRITE_STRING  = "Write password string:";

  public static final String [] debugStr = {
    "off",
    "errors",
    "normal",
    "warnings",
    "verbose",
  };

  public static String lastiView, lastCode, lastCommand, lastSubCommand;
  public static InetAddress mgmtIp;
  private final int DEFAULT_DEBUG_LEVEL = DEBUG_OFF;
  private static final Object lockObject = new Object();
  private static JServerTCP jstcp;
  private native int getUsedLicense();
  private native int getLicenseLimit();
  private native long getLicenseExpiryTime();
  private native int getCalls() throws Exception;
  private native int getMRCalls() throws Exception;
  private native void serverCfgInit () throws Exception;
  native void declareDbStale ();
  private native InetAddress getRedundPrimaryIfAddress ();
  public native boolean generateCfgFile ();
  public native boolean setJServerConfig (String read, String write);
  public native String getJServerReadPassword ();
  public native String getJServerWritePassword ();
  public native String getJServerLogLevel ();
  public native String getJServerLogFileName ();
  public native int getJServerCompression ();
  public native boolean isNetworkMaster();
  // moved to JServerMain
  //private static native int mswInit ();
  //private static native void nativeInit (iServerConfig config, Capabilities cap) throws BridgeException;
  public static native String getLastNativeCommand ();
  public static native int getPid ();
  public static native int getPgid ();
  public static native int getPpid ();
  private static native boolean isNativeInitDone ();
  public static native InetAddress [] getAllLocalAddress ();
  public static native int getDatabasePrimary ();
  public static native boolean isInterfaceUp (long ipaddr);
  public static native boolean isDatabaseOpAllowed ();

  /**
   * Only method to get access to an instance of the JServer class
   * (Singleton)
   */
  public static JServer getInstance (ThreadGroup tg, SingleInstance si) throws IOException, BridgeException {
    System.out.println("[JServer/getInstance] Getting JServer instance  " + thisInstance);
    if (thisInstance == null) {
      // moved to jservermain
      // initialize the native code stuff
      //nativeInit(new iServerConfig(), new Capabilities());
      //if (mswInit() != 0)
        //throw new BridgeException("Unable to initialize shared memory/attach to cache");
      InetAddress[] interfaces  =  null;
      mgmtIp  =  getMgmtIpAddress();
      if(mgmtIp == null){
        System.out.println("[JServer/getInstance] Unable to get management Ip Address. Please check the mgmt_ip in server.cfg");
        interfaces  =  getAllLocalAddress();
      }else{
        interfaces  =  new InetAddress[] {mgmtIp};
      }
      NextoneMulticastSocket s = new NextoneMulticastSocket(Constants.MCAST_JSERVER_LISTEN_PORT, interfaces);
      s.joinGroupOnAllIf(InetAddress.getByName(Constants.MCAST_ADDRESS));
      thisInstance = new JServer(tg, s, si);
    }

    return thisInstance;
  }

  public static InetAddress  getMgmtIpAddress(){
    try{
      BufferedReader br = new BufferedReader(new FileReader(CONFIG_FILE));
      String line = "";
      while( (line = br.readLine()) != null){
        if(line.indexOf(MGMT_IP)  != -1){
          StringTokenizer stk = new StringTokenizer(line);
          if(stk.hasMoreTokens()){
            stk.nextToken();
            String ip = stk.nextToken();
             // remove the ''
            ip	=  ip.substring(1);
            ip	=  ip.substring(0,ip.length()-1);
            System.out.println("Management Ip: "+ip);
            return InetAddress.getByName(ip); 
          }
        }
      } 
    }catch(Exception e){
      System.out.println("[JServer/getMgmtIpAddress] Exception "+ e.toString());
      e.printStackTrace();
      return null;
    } 
    return null;
  }

  // singleton class should be accessed through the getInstance method
  private JServer (ThreadGroup tg, DatagramSocket s, SingleInstance si) throws IOException, BridgeException {
    super(tg, s, Thread.NORM_PRIORITY, "JServer", false);
    ds = s;
    instance = si;

    allLocalAddr = getAllLocalAddress();


    // figure out where the iserver files are stored. we assume that
    // it is stored in the same directory as the jar file we are
    // executing
    String d = getClass().getResource("/com/nextone/JServer/JServer.class").getFile();
    if (d != null) {
      StringTokenizer st = new StringTokenizer(d, "!");
      if (st.hasMoreTokens()) {
        st = new StringTokenizer(st.nextToken(), ":");
        st.nextToken();

        String dir = new File(st.nextToken()).getParent();

        if (dir != null)
          curDir = dir;
        else {
          System.err.println("Error finding CWD!");
          curDir = "/usr/local/nextone/bin";
        }
      } else {
        System.err.println("Cannot find CWD!");
        curDir = "/usr/local/nextone/bin";
      }
    } else {
      System.err.println("Cannot determine CWD!");
      curDir = "/usr/local/nextone/bin";
    }

    // read server.cfg
    int dbglvl = reconfig();

    // print some version infos
    System.out.println(SysUtil.getDate() + ": Starting NexTone Configuration Server...");
    System.out.println(JServerMain.getName() + " " + JServerMain.getMajorVersion() + JServerMain.getMinorVersion() + ", " + JServerMain.getBuildDate());
    System.out.println(JServerMain.getCopyright());
    System.out.println("Java Version: " + System.getProperties().getProperty("java.version") + " (" + System.getProperties().getProperty("java.vm.vendor") + " " + System.getProperties().getProperty("java.vm.version") + "[" + System.getProperties().getProperty("java.vm.name") + "])");
    System.out.println("PID: " + getPid() + "  PGID: " + getPgid() + "  PPID: " + getPpid());
    if (readPass.equals(""))
      System.err.println("Config file did not contain a read password");

    if (writePass.equals(""))
      System.err.println("Config file did not contain a write password");

    System.out.print("Local interface IPs:");
    for (int i = 0; i < allLocalAddr.length; i++)
      System.out.print(" " + allLocalAddr[i]);
    System.out.println();

    if(mgmtIp == null){

      //LogTask.getInstance(tg, curDir); // start the logging server

      bs = new BridgeServer(tg, curDir, lockObject);  // start the bridge server

      handleDebug(dbglvl);

      jstcp = new JServerTCP(tg, isDebug(), this, bs, lockObject); // start the tcp server
    }else{
     // bind to management address 
      //LogTask.getInstance(tg, curDir,mgmtIp); // start the logging server

      bs = new BridgeServer(tg, curDir, lockObject,mgmtIp);  // start the bridge server

      handleDebug(dbglvl);

      jstcp = new JServerTCP(tg, isDebug(), this, bs, lockObject,mgmtIp); // start the tcp server
    }
    jstcp.setName("JServerTCP");
    start();  // start running the jserver
  }

  // the following constructors (inherited from the super class)
  // are not allowed
  public JServer () throws IOException {
    throw new IOException("Invalid invocation");
  }
  public JServer (UDPServerWorker tg) throws IOException {
    this();
  }
  public JServer (int p) throws IOException {
    this();
  }
  public JServer (UDPServerWorker tg, int p) throws IOException {
    this();
  }
  public JServer (ThreadGroup tg, DatagramSocket s, int p) throws IOException {
    this();
  }
  public JServer (ThreadGroup tg, DatagramSocket s, int p, String n) throws IOException {
    this();
  }
  public JServer (ThreadGroup tg, DatagramSocket s, int p, String n, boolean i) throws IOException {
    this();
  }
  public JServer (DatagramSocket ds, int p) throws IOException {
    this();
  }
  public JServer (UDPServerWorker tg, DatagramSocket ds) throws IOException {
    this();
  }
  public JServer (ThreadGroup t, UDPServerWorker tg, int p, DatagramSocket ds, int pr, boolean i) throws IOException {
    this();
  }

  // sets up the System.out and System.err to a FileOutputStream
  // initializes the read/write permission string
  // returns the debug level to use
  public int reconfig () {

    // make the native library parse and store the server.cfg file
    try {
      synchronized (lockObject) {
	serverCfgInit();
      }
    } catch (Exception e) {
      System.err.println("Error parsing server.cfg file");
      return -1;
    }

    int loglevel = -1;
    readPass = getJServerReadPassword();
    writePass = getJServerWritePassword();
    logfile = getJServerLogFileName();
    compression = getJServerCompression();
    String l = getJServerLogLevel();
    for (int i = 0; i < JServer.debugStr.length; i++) {
      if (l.equals(JServer.debugStr[i])) {
        loglevel = i;
        break;
      }
    }

    if (loglevel == -1)
      JServer.printDebug("log level in the config file is not one of <off | errors | normal | warnings | verbose>, defaulting to " + JServer.debugStr[DEFAULT_DEBUG_LEVEL], JServer.DEBUG_WARNING);

    // set the output/err streams
    try {
      if (logfile == null || logfile.equals(""))
	logfile = "/dev/null";
      PrintStream ps = new PrintStream(new FileOutputStream(logfile, true), true);
      System.setErr(ps);
      System.setOut(ps);
    } catch (IOException ie) {
      System.err.println("Error setting output/error streams to a file:");
      System.err.println(ie);
    }

    // set the read/write permissions
    if (readPass == null) {
      readPass = "";
    }

    if (writePass == null) {
      writePass = "";
    }

    return loglevel;
  }


  // changes the permission strings in the config file
  private Boolean changePermission (String rp, String wp) throws BridgeException {
    boolean result = setJServerConfig(rp, wp);
    generateCfgFile();

    // re-read the local permission strings
    readPass = getJServerReadPassword();
    writePass = getJServerWritePassword();

    return new Boolean(result);
  }

  /**
   * prints debug message according to the current debug level
   *
   * @param exception the exception whose stack trace will be logged
   * @param level the desired debug level at which this message will be logged
   */
  public static void printDebug (Exception exception, int level) {
    try {
      StringWriter sw = new StringWriter();
      exception.printStackTrace(new PrintWriter(sw));
      printDebug(sw, level, true);
      sw.close();
    } catch (Exception e) {
      printDebug("Error extracting stack trace: " + e, level, true);
      printDebug("Original exception: " + exception, level, true);
    }
  }

  /**
   * prints debug messages according to the current debug level
   */
  public static void printDebug (Object msg, int level) {
    printDebug(msg, level, true);
  }

  /**
   * prints debug messages according to the current debug level
   * if <code>newline</code> is true, prints a newline character at the end
   */
  public static void printDebug (Object msg, int level, boolean newline) {
    if (thisInstance == null)
      return;

    if (level <= debugLevel) {
      if (newline)
	      System.err.println(SysUtil.getDate() + ": " + msg);
      else
	      System.err.print(SysUtil.getDate() + ": " + msg);
      System.err.flush();
    }
  }

  /**
   * prints the debug message, and the getMessage from the exception and the stack trace
   *
   */
  public static void printException (String msg, Exception exc, int level) {
    printDebug(msg + ": " + exc.getMessage(), level);
    printDebug(exc, level);
  }

  public static boolean isDebug () {
    if (debugLevel > DEBUG_OFF && debugLevel <= DEBUG_VERBOSE)
      return true;
    return false;
  }


  public static boolean toOurIP (InetAddress inIp) {
    for (int i = 0; i < allLocalAddr.length; i++) {
      if (allLocalAddr[i].equals(inIp))
	return true;
    }
    return false;
  }


  /**
   * checks if the source is our own machine
   * (we check the address in the packet content and the packet source)
   */
  public static boolean isSecure (LimitedDataInputStream dis, InetAddress from) {
    try {
      ObjectInputStream ois = new ObjectInputStream(dis);
      InetAddress addr = (InetAddress)ois.readObject();
      if (from.equals(addr))
	return toOurIP(addr);
    } catch (Exception e) { }

    JServer.printDebug("packet from " + from.getHostAddress() + " deemed insecure", JServer.DEBUG_WARNING);

    return false;
  }


  /**
   * checks the read/write permission strings
   */
  public int checkPermission (short protVersion, String read, String write, InetAddress ip) {
    int result = 0;

    if (protVersion == Constants.HELLO_VERSION_NO_AUTH) {
      result = READ_WRITE_ALLOWED;
    } else {
      // check for read
      if (readPass == null || readPass.equals("") ||
	      readPass.equals(read))
	      result |= READ_ALLOWED;

      // check for write
      if (writePass == null || writePass.equals("") ||
	      writePass.equals(write))
	      result |= WRITE_ALLOWED;
    }

    if (result == NOT_ALLOWED)
      JServer.printDebug("packet from " + ip.getHostAddress() + " has no permissions (" + read + "/" + write + ")", JServer.DEBUG_WARNING);

    return result;
  }

  /**
   * returns if the operation is allowed for the given permission
   *
   * @exception BridgeException when the operation passed is unrecognized
   *            and the source of the request has a read permisssion
   */
  public boolean isOperationAllowed (short protVersion, String read, String write, int operation, InetAddress ip) throws BridgeException {
    int permit = checkPermission(protVersion, read, write, ip);

    switch (operation) {
      /* cases where either of read or write permission is enough */
    case Constants.IEDGE_GET:
    case Constants.MAINTENANCE_GET_GROUP_NAMES:
    case Constants.MAINTENANCE_GET_GROUP:
    case Constants.MAINTENANCE_GET_REQUEST_NAMES:
    case Constants.MAINTENANCE_GET_REQUEST:
    case Constants.MAINTENANCE_REQUEST_ACTIVE:
    case Constants.MAINTENANCE_GET_LOG_NAMES:
    case Constants.MAINTENANCE_GET_GROUP_NAMES_COMMENTS:
    case Constants.MAINTENANCE_GET_REQUEST_NAMES_COMMENTS:
    case Constants.AUTO_DOWNLOAD_GET_NAMES:
    case Constants.AUTO_DOWNLOAD_GET_NAMES_COMMENTS:
    case Constants.AUTO_DOWNLOAD_GET_CONFIG:
    case Constants.AUTO_DOWNLOAD_ACTIVE:
    case Constants.LOOKUP_BY_PHONE:
    case Constants.LOOKUP_BY_VPNPHONE:
    case Constants.LOOKUP_BY_CALLINGPLAN_ROUTE_NAME:
    case Constants.PRESENCE_NUMBER:
    case Constants.ISERVER_COMPRESSION:
    case Constants.ISERVER_GET_CFG:
    case Constants.MAX_RECORDS: 
    case Constants.ISERVER_DB_EXPORT:
    case Constants.ISERVER_DB_GET_FILENAMES:
    case Constants.LS_ALARM_STATUS:
    case Constants.ISERVER_MAX_CALLS:
    case Constants.ISERVER_MAX_MR_CALLS:
    if (permit != NOT_ALLOWED)
	      return true;
      break;

      /* cases where write permission is required */
    case Constants.MAINTENANCE_PUT_GROUP:
    case Constants.MAINTENANCE_DELETE_GROUP:
    case Constants.MAINTENANCE_PUT_REQUEST:
    case Constants.MAINTENANCE_DELETE_REQUEST:
    case Constants.MAINTENANCE_REQUEST_ABORT:
    case Constants.MAINTENANCE_DELETE_LOG:
    case Constants.AUTO_DOWNLOAD_PUT_CONFIG:
    case Constants.AUTO_DOWNLOAD_DELETE:
    case Constants.AUTO_DOWNLOAD_ABORT:
    case Constants.AUTO_DOWNLOAD_DELETE_LOG:
    case Constants.ISERVER_SET_CFG:
    case Constants.ISERVER_DB_IMPORT:
    case Constants.LS_ALARM_CLEAR:
    case Constants.CLEAR_LOG_FILE:
    case Constants.PROCESS_BULK_COMMANDS:
    case Constants.PROCESS_COMMAND:
    case  Constants.ISERVER_GET_CAP:
    if (permit == READ_WRITE_ALLOWED || permit == WRITE_ALLOWED)
	    return true;
      break;

    default:
      if (permit != NOT_ALLOWED) {
    	JServer.printDebug(ip.getHostAddress() + ": Operation not recognized by this iServer (Error code " + operation + ")", JServer.DEBUG_WARNING);
	      throw new OperationUnknownBridgeException(operation);
      }				
    }

    JServer.printDebug("Operation " +  operation + " from " + ip.getHostAddress() + " doesn't have sufficient permissions", JServer.DEBUG_WARNING);
    return false;
  }

  /**
   * handle hellos
   */
  private void handleHello (DatagramPacket dp, LimitedDataInputStream dis) {
    handleHello(dp,dis,null);
  }

  private void handleHello (DatagramPacket dp, LimitedDataInputStream dis, String toAddr) {
    JServer.printDebug("hello from " + dp.getAddress().getHostAddress(), JServer.DEBUG_VERBOSE);
    JServer.printDebug("hello from " + dp.getSocketAddress(), JServer.DEBUG_VERBOSE);

    try {
      if (nativeInitDone == false) {
	      nativeInitDone = isNativeInitDone();
	      if (nativeInitDone == false) {
	        JServer.printDebug(dp.getAddress().getHostAddress() + ": shared memory not be initialized, not replying for the hello" , JServer.DEBUG_VERBOSE);
	        return;
	      }
      }

      ByteArrayOutputStream bos = new ByteArrayOutputStream(100);
      DataOutputStream dos = new DataOutputStream(bos);
      dos.writeShort(Constants.REGISTRATION);
      dos.writeInt(0);  // mode and device id

      // information about the redundancy
      iServerConfig icfg = bs.getNativeIserverConfig();  // this only contains stuff in server.cfg
      InetAddress dbPrimary = InetAddress.getByName(IPUtil.intToIPString(getDatabasePrimary()));

     //VIP concept removed
/*
      InetAddress vip = null;
      String[]  str = icfg.getRedundsConfig().getNetworkConfig().getPrimaryInterfaceVips();

      if(str  !=  null  &&  str.length  ==  1)
        vip = InetAddress.getByName(str[0]);

//      InetAddress vip = InetAddress.getByName(icfg.getRedundsConfig().getNetworkConfig().getPrimaryInterfaceVip());
      boolean vipActive = false;
      InetAddress primaryIfAddr = null;

      if (icfg.getRedundsConfig().getNetworkConfig().isServerEnabled() && 
          vip !=  null &&
	        isInterfaceUp(IPUtil.ipStringToLong(vip.getHostAddress()))) {
  	      vipActive = true;
	        primaryIfAddr = getRedundPrimaryIfAddress();
      }

*/
      InetAddress clusterAddr = mgmtIp;
      InetAddress toAddress = null;
      try{
        toAddress = InetAddress.getByName(toAddr);
      }catch(Exception exp){
	   JServer.printException("Error in converting the to addresses "+toAddr, exp, JServer.DEBUG_ERROR);
      } 
      if(mgmtIp == null){
        InetAddress[] localAddr = getAllLocalAddress();
        if(localAddr  != null && 
           localAddr.length > 0 &&
           toAddress  != null
        ){
         clusterAddr  =  localAddr[0];
         for(int i=0; i < localAddr.length; i++){
           if(localAddr[i].equals(toAddress)){
             clusterAddr = localAddr[i];
             break;
           }
         }
           
       }
       else{
         try{
           clusterAddr  =  InetAddress.getByName("0.0.0.0");
         }catch(Exception e){
	   JServer.printException("Error in getting the local addresses", e, JServer.DEBUG_ERROR);
        }
       }
      }
      InetAddress vip = clusterAddr;
      InetAddress  primaryIfAddr  =  clusterAddr;
      boolean vipActive = isNetworkMaster();
      // fqdn
      String host = InetAddress.getLocalHost().getHostName();
      dos.writeUTF(host);

      // version of the hello packet
      dos.writeShort(2);
      
      int calls  =  0;
      int maxCalls  = 0;
      try {
	calls  =  getCalls();
	maxCalls  = bs.getMaxCalls();
	JServer.printDebug("used calls = " + calls, JServer.DEBUG_VERBOSE);
      } catch (Exception e) {
	JServer.printException("Error in getting the used calls", e, JServer.DEBUG_ERROR);
      }

      dos.writeInt(calls);
      dos.writeInt(maxCalls);

      int lsUsed    = getUsedLicense();
      int lsLimit   = getLicenseLimit();

      int threshhold  = lsLimit*90/100;
      int flag  = LS_LIMIT_OK;
      int[][] alarms  = bs.getLicenseAlarms();
      if (alarms[0] !=  null && alarms[0][0] > 0) {
	flag = LS_LIMIT_EXCEEDED;
      } else if (lsUsed >= threshhold)
	flag  = LS_LIMIT_THRESHOLD;
      dos.writeInt(flag);

      RedundInfo ri = new RedundInfo(icfg.getRedundsConfig().getDatabaseConfig().isServerEnabled(), dbPrimary, toOurIP(dbPrimary), icfg.getRedundsConfig().getNetworkConfig().isServerEnabled(), vip, vipActive, primaryIfAddr);
      dos.writeUTF(ri.toString());

      // write MR alarm details
      calls = 0;
      flag = LS_LIMIT_OK;
      maxCalls = bs.getMaxMRCalls();
      if (maxCalls > 0) {  // only if the MR is enabled
        try {
          calls = getMRCalls();
          JServer.printDebug("used mr calls = " + calls, JServer.DEBUG_VERBOSE);
        } catch (Exception e) {
          JServer.printException("Error in getting the used mr calls", e, JServer.DEBUG_ERROR);
        }
        threshhold = maxCalls*90/100;
        if (alarms[1] != null && alarms[1][0] > 0)
          flag = LS_LIMIT_EXCEEDED;
        else if (calls >= threshhold)
          flag = LS_LIMIT_THRESHOLD;
	
      }
      dos.writeInt(calls);
      dos.writeInt(maxCalls);
      dos.writeInt(flag);

      long expiryTime = getLicenseExpiryTime();
      dos.writeLong( expiryTime );
      // VIP is  not used from 3.1 MSWs 
      // write the vip flag 
      dos.writeBoolean(false);
      dp.setData(bos.toByteArray(), 0, bos.size());
      ds.send(dp);
      JServer.printDebug(dp.getAddress().getHostAddress() + ": used License = "+lsUsed + "License Limit = "+ lsLimit + " threshold = "+threshhold , JServer.DEBUG_VERBOSE);
    } catch (Exception ie) {
      JServer.printException(dp.getAddress().getHostAddress() + ": handleHello caught an error", ie, JServer.DEBUG_ERROR);
    }
  }


  /**
   * throws exception if database redund enabled and we are not the master
   */
  public static void checkDatabaseMaster () throws BridgeException {
    if (!isDatabaseOpAllowed())
      throw new BridgeException("iServer currently not the database master.\nPlease allow some time for synchronization and try again");
  }


  /**
   * handle status requests - these are requests coming from the same
   * machine as which the server is currently running
   */
  private void handleStatus (DatagramPacket dp) {
    try {
      StringBuffer st = new StringBuffer();

      st.append("Java Version: " + System.getProperties().getProperty("java.version") + " (" + System.getProperties().getProperty("java.vm.vendor") + " " + System.getProperties().getProperty("java.vm.version") + "[" + System.getProperties().getProperty("java.vm.name") + "])\n");
      st.append("Current active user threads: " + Thread.activeCount() + "\n");
      //      st.append("Current active auto-download tasks: " + AutoDownload.getAutoDownloadCount() + "\n");
      //      st.append("Current scheduled maintenance requests: " + bs.getMaintenanceRequestCount() + "\n");
      long [] mem = SysUtil.getMem();
      st.append("Memory Statistics:\n");
      st.append("Total: " + mem[0] + "  Free: " + mem[1] + "  Used: " + mem[2] + "\n\n");
      st.append("Log file: \"" + logfile + "\"\n");
      st.append("Read password string:  \"" + readPass + "\"\n");
      st.append(WRITE_STRING+" \"" + writePass + "\"\n");
      st.append("Compression: ");
      st.append((compression == 0)?"off":"on");

      ByteArrayOutputStream bos = new ByteArrayOutputStream(512);
      ObjectOutputStream oos = new ObjectOutputStream(bos);
      oos.writeObject(st.toString());
      dp.setData(bos.toByteArray(), 0, bos.size());
      ds.send(dp);
    } catch (IOException ie) {
      JServer.printException(dp.getAddress().getHostAddress() + ": handleStatus caught an error", ie, JServer.DEBUG_ERROR);
    }
  }


  /**
   * handle debug requests - these are requests coming from the same
   * machine as which the server is currently running
   */
  public void handleDebug (int level) {
    if (level < JServer.DEBUG_OFF ||
	    level > JServer.DEBUG_VERBOSE)
      level = DEFAULT_DEBUG_LEVEL;

    System.err.println("Setting debug level to "+ JServer.debugStr[level]);
    debugLevel = level;
    bs.setDebugLevel(level);
  }

  /**
   * handle alive requests - these are requests coming from the process
   * manager thread, if we are unable to reply to this, he will let the
   * PM retart us
   */
  private void handleAlive (DatagramPacket dp) {
    try {
      ds.send(dp);
    } catch (IOException ie) {
      JServer.printException(dp.getAddress().getHostAddress() + ": handleAlive caught an error", ie, JServer.DEBUG_ERROR);
    }
  }
  /**
   * handle uptime requests - these are requests coming from the same
   * machine as which the server is currently running
   */
  private void handleUptime (DatagramPacket dp) {
    try {
      long currentTime = System.currentTimeMillis();
      DeltaTime dt = new DeltaTime(currentTime-startTime);

      ByteArrayOutputStream bos = new ByteArrayOutputStream(512);
      ObjectOutputStream oos = new ObjectOutputStream(bos);
      oos.writeObject(dt.toString());
      dp.setData(bos.toByteArray(), 0, bos.size());
      ds.send(dp);
    } catch (IOException ie) {
      JServer.printException(dp.getAddress().getHostAddress() + ": handleUptime caught an error", ie, JServer.DEBUG_ERROR);
    }
  }


  /**
   * handle get requests
   */
  private void handleGetRequest (DatagramPacket dp, LimitedDataInputStream dis) {

    try {
      short protVersion = dis.readShort();
      String read = dis.readUTF();
      String write = dis.readUTF();

      if (checkPermission(protVersion, read, write, dp.getAddress()) == NOT_ALLOWED) {
	      dis.close();
	      return;  // requester will just timeout
      }

      short numKeys = dis.readShort();
      ObjectInputStream ois = new ObjectInputStream(dis);
      Vector v = new Vector();

      Object ret = null;
      short code = 0;
      try {
	      for (int i = 0; i < numKeys; i++)
	        v.add(ois.readObject());
	      code = dis.readShort();
	      lastSubCommand = v.elementAt(0) + "/" + v.elementAt(1);

	      if (code == 0) {
	        ret = (ProvisionData)bs.getProvData(null, v.toArray());
	      }
      } catch (Exception e) {
	      ret = e;
      }

      if (ret == null)
    	ret = new BridgeException("Internal error: invalid code " + code + " received in get request");
      ByteArrayOutputStream bos = new ByteArrayOutputStream(512);
      ObjectOutputStream oos = new ObjectOutputStream(bos);
      oos.writeObject(ret);
      dp.setData(bos.toByteArray(), 0, bos.size());
      ds.send(dp);
      dis.close();
    } catch (IOException ie) {
      JServer.printException(dp.getAddress().getHostAddress() + ": handleGetRequest caught an error", ie, JServer.DEBUG_ERROR);
    }
  }


 
  public static void executeIserverCommand (String command) throws IOException {
    JServer.executeIserverCommand(command, null);
  }

  public static void executeIserverCommand (String command, OutputStream out) throws IOException {
    JServer.executeCommand(curDir + File.separator + "iserver " + command, out);
  }

  public static void executeCommand (String command) throws IOException {
    JServer.executeCommand(command, null);
  }

  public static void executeCommand (String command, OutputStream out) throws IOException {
    JServer.printDebug("Executing: " + command, JServer.DEBUG_NORMAL);
    Process p = Runtime.getRuntime().exec(command);

    boolean outProvided = true;
    if (out == null) {
      out = new ByteArrayOutputStream();
      outProvided = false;
    }

    SequenceInputStream is = new SequenceInputStream(p.getInputStream(), p.getErrorStream());
    // read all the input stream and write to the output stream
    int i = -1;
    while ((i = is.read()) != -1)
      out.write(i);
 
    if (!outProvided) {
      JServer.printDebug(((ByteArrayOutputStream)out).toByteArray(), JServer.DEBUG_VERBOSE);
      out.close();
    }

    try {
      p.waitFor();
    } catch (InterruptedException ie) {}
    p.destroy();
  }


  /**
   * handle restart requests
   */
  private void handleRebootRequest (DatagramPacket dp, LimitedDataInputStream dis) {
    try {
      short protVersion = dis.readShort();
      String read = dis.readUTF();
      String write = dis.readUTF();

      int permit = checkPermission(protVersion, read, write, dp.getAddress());
      if (permit != WRITE_ALLOWED &&
	  permit != READ_WRITE_ALLOWED) {
	dis.close();
	return;  // requester will just timeout
      }

      short numReq = dis.readShort();
      Object [] ret = new Object [numReq];
      boolean status = bs.restartIServer();  // have the PM do the restart
      for (int i = 0; i < numReq; i++) {
	ret[i] = new Boolean(status); // we are not supporting individual service restart
      }

      ByteArrayOutputStream bos = new ByteArrayOutputStream(512);
      ObjectOutputStream oos = new ObjectOutputStream(bos);
      for (int i = 0; i < numReq; i++)
	oos.writeObject(ret[i]);
      dp.setData(bos.toByteArray(), 0, bos.size());
      ds.send(dp);
      dis.close();
    } catch (IOException ie) {
      JServer.printException(dp.getAddress().getHostAddress() + ": handleRebootRequest caught an error", ie, JServer.DEBUG_ERROR);
    }
  }



  /**
   * handle all list requests
   */			
  private void handleListRequest (DatagramPacket dp, LimitedDataInputStream dis) {
    try {
      short protVersion = dis.readShort();
      String read = dis.readUTF();
      String write = dis.readUTF();

      int permit = checkPermission(protVersion, read, write, dp.getAddress());
      if (permit == NOT_ALLOWED) {
	dis.close();
	return;  // stay silent, requester would just time out
      }

      int list = Constants.IEDGE_LIST;
      // determine what list the caller wants
      try {
	list = dis.readShort();
      } catch (EOFException ee) {
	list = Constants.IEDGE_LIST;  // older iviews don't send this field
      }

      lastSubCommand = String.valueOf(list);

      Object ret = null;
      try {
	int port = bs.getListPort(list);
	JServer.printDebug(dp.getAddress().getHostAddress() + ": Will ask " + dp.getAddress().toString() + " to connect to " + port + " to get the list", JServer.DEBUG_VERBOSE);
	ret = new Integer(port);
      } catch (Exception e) {
	JServer.printException(dp.getAddress().getHostAddress() + ": Error processing list request", e, JServer.DEBUG_ERROR);
	ret = e;
      }

      // return the port number to contact to get the list over tcp
      ByteArrayOutputStream bos = new ByteArrayOutputStream(512);
      ObjectOutputStream oos = new ObjectOutputStream(bos);
      oos.writeObject(ret);
      dp.setData(bos.toByteArray(), 0, bos.size());
      ds.send(dp);
      dis.close();
    } catch (IOException ie) {
      JServer.printException(dp.getAddress().getHostAddress() + ": handleListRequest caught an error", ie, JServer.DEBUG_ERROR);
    }
  }


  /**
   * handle other iedge related requests
   */
  private void handleIEdgeRequest (DatagramPacket dp, LimitedDataInputStream dis) {
    try {
      short protVersion = dis.readShort();
      String read = dis.readUTF();
      String write = dis.readUTF();
      short op = dis.readShort();

      Object ret = null;

      try {
	      if (!isOperationAllowed(protVersion, read, write, op, dp.getAddress())) {
	        dis.close();
	        return;  // stay silent, requester would just time out
	      }
      } catch (BridgeException be) {
      	ret = be;
        op = -1;
      }

      // the operation we want to do on the iedge entry
      switch (op) {
      case IEDGE_GET:
	      String serial = dis.readUTF();
	      int port = dis.readInt();
	      lastSubCommand = serial + "/" + port + "  get";
	      try {
	        JServer.printDebug(dp.getAddress().getHostAddress() + ": Attempting to get " + serial + "/" + port + " params", JServer.DEBUG_VERBOSE);
	        ret = bs.getIedgeParams(serial, port);
	      } catch (Exception e) {
	        JServer.printException(dp.getAddress().getHostAddress() + ": Error processing iedge get for " + serial + "/" + port, e, JServer.DEBUG_ERROR);
	        ret = e;
	      }
	      break;
      }

      // return the result object
      ByteArrayOutputStream bos = new ByteArrayOutputStream(512);
      ObjectOutputStream oos = new ObjectOutputStream(bos);
      oos.writeObject(ret);
      dp.setData(bos.toByteArray(), 0, bos.size());
      ds.send(dp);
      dis.close();
    } catch (Exception ie) {
      JServer.printException(dp.getAddress().getHostAddress() + ": handleIEdgeRequest caught an error", ie, JServer.DEBUG_ERROR);
    }
  }


  private void handleMiscReq (DatagramPacket dp, LimitedDataInputStream dis) {
    try {
      short protVersion = dis.readShort();
      String read = dis.readUTF();
      String write = dis.readUTF();
      short op = dis.readShort();

      Object ret = null;

      try {
	      if (!isOperationAllowed(protVersion, read, write, op, dp.getAddress())) {
	        dis.close();
	        return;  // stay silent, requester would just time out
	      }
      } catch (BridgeException be) {
      	ret = be;
      }

      // the operation we want to do on the iedge entry
      switch (op) {
      case Constants.PRESENCE_NUMBER:
	      String destPhone = dis.readUTF();
	      lastSubCommand = "get presence " + destPhone;
	      try {
	        JServer.printDebug(dp.getAddress().getHostAddress() + ": Attempting to get the presence number for [ destphone =  " + destPhone + "]", JServer.DEBUG_VERBOSE);
	        ret = bs.getPresence( destPhone);
	      } catch (Exception e) {
	        JServer.printException(dp.getAddress().getHostAddress() + ": Error processing get presence number request (" +  destPhone + ")", e, JServer.DEBUG_ERROR);
	        ret = e;
	      }
	      break;

      case Constants.ISERVER_COMPRESSION:
        ret = new Integer(compression);
        break;

      case Constants.ISERVER_MAX_CALLS:
	      lastSubCommand = "get max calls";
	      try {
	        JServer.printDebug(dp.getAddress().getHostAddress() + ": Attempting to get maximum number of calls", JServer.DEBUG_VERBOSE);
	        ret = new Integer(bs.getMaxCalls());
	      } catch (Exception e) {
	        JServer.printException(dp.getAddress().getHostAddress() + ": Error processing get max calls", e, JServer.DEBUG_ERROR);
	        ret = e;
	      }
	      break;

      case Constants.ISERVER_MAX_MR_CALLS:
	      lastSubCommand = "get max mr calls";
	      try {
	        JServer.printDebug(dp.getAddress().getHostAddress() + ": Attempting to get maximum number of mr calls", JServer.DEBUG_VERBOSE);
	        ret = new Integer(bs.getMaxMRCalls());
	      } catch (Exception e) {
	        JServer.printException(dp.getAddress().getHostAddress() + ": Error processing get max mr calls", e, JServer.DEBUG_ERROR);
	        ret = e;
	      }
	      break;

      case Constants.CLEAR_LOG_FILE:
	      lastSubCommand = "clear log file";
	      try {
	        int logFileType = dis.readInt();
	        JServer.printDebug(dp.getAddress().getHostAddress() + ": clearing log file (" + logFileType + ")", JServer.DEBUG_VERBOSE);
	        ret = new Boolean(bs.clearLog(logFileType));
	      } catch (Exception e) {
	        JServer.printException(dp.getAddress().getHostAddress() + ": Error processing clear log file", e, JServer.DEBUG_ERROR);
	        ret = e;
	      }
	      break;
      case  Constants.PROCESS_COMMAND:
	      lastSubCommand = "process command";
	      try {
	        ObjectInputStream ois = new ObjectInputStream(dis);
	        Commands cmd = (Commands)ois.readObject();
                checkDatabaseMaster();
	        JServer.printDebug(dp.getAddress().getHostAddress() + "processing commands", JServer.DEBUG_VERBOSE);
                ret = new Boolean(bs.processCommands(cmd));
	      } catch (Exception e) {
	        JServer.printException(dp.getAddress().getHostAddress() + ": Error processing commands", e, JServer.DEBUG_ERROR);
	        ret = e;
	      }
	      break;
      default:
        break;
      }
      // return the result object
      ByteArrayOutputStream bos = new ByteArrayOutputStream(512);
      ObjectOutputStream oos = new ObjectOutputStream(bos);
      oos.writeObject(ret);
      dp.setData(bos.toByteArray(), 0, bos.size());
      ds.send(dp);
      dis.close();
    } catch (Exception ie) {
      JServer.printException(dp.getAddress().getHostAddress() + ": handleMiscRequest caught an error:", ie, JServer.DEBUG_ERROR);
    }
  }



  /**
   * Handle alarm request
   **/
  void handleAlarmRequest(DatagramPacket dp, LimitedDataInputStream dis){
    try {
      short protVersion = dis.readShort();
      String read = dis.readUTF();
      String write = dis.readUTF();
      short op = dis.readShort();

      Object ret = null;

      try {
	if (!isOperationAllowed(protVersion, read, write, op, dp.getAddress()))
	  {
	    dis.close();
	    return;  // stay silent, requester would just time out
	  }
      } catch (BridgeException be) {
        ret = be;
      }

      // the operation we want to do on the iedge entry
      switch (op) {
      case Constants.LS_ALARM_STATUS:
	lastSubCommand = "status";
	try {
	  JServer.printDebug(dp.getAddress().getHostAddress() + ": Attempting to read license alarms" , JServer.DEBUG_VERBOSE);
	  ret = bs.getLicenseAlarms();
	} catch (Exception e) {
	  JServer.printException(dp.getAddress().getHostAddress() + ": Error processing get license alarms request", e, JServer.DEBUG_ERROR);
	  ret = e;
	}
	break;

      case Constants.LS_ALARM_CLEAR:
	lastSubCommand = "clear";
	try {
	  JServer.printDebug(dp.getAddress().getHostAddress() + ": Attempting to clear the licence alarms", JServer.DEBUG_VERBOSE);
	  boolean r = bs.clearLicenseAlarms();
	  ret = new Boolean(r);
	} catch (Exception e) {
	  JServer.printException(dp.getAddress().getHostAddress() + ": Error processing clear license alarms request", e, JServer.DEBUG_ERROR);
	  ret = e;
	}
	break;
      }

      // return the result object
      ByteArrayOutputStream bos = new ByteArrayOutputStream(512);
      ObjectOutputStream oos = new ObjectOutputStream(bos);
      oos.writeObject(ret);
      dp.setData(bos.toByteArray(), 0, bos.size());
      ds.send(dp);
      dis.close();
    } catch (IOException ie) {
      JServer.printException(dp.getAddress().getHostAddress() + ": handleAlarmRequest caught an error", ie, JServer.DEBUG_ERROR);
    }
  }
  
  /**
   * handle version number request
   */
  private class HandleVersionRequest extends Thread {
    private DatagramPacket dp;
    private LimitedDataInputStream dis;

    HandleVersionRequest (DatagramPacket dp, LimitedDataInputStream dis) {
      this.dp = new DatagramPacket(new byte[512], 512, dp.getAddress(), dp.getPort());
      this.dis = dis;
    }

    public void run () {

      try {
	short protVersion = dis.readShort();
	String read = dis.readUTF();
	String write = dis.readUTF();

	if (checkPermission(protVersion, read, write, dp.getAddress()) == NOT_ALLOWED) {
	  dis.close();
	  return;  // requester will just timeout
	}

	Object ret = null;
	try {
	  ret = bs.getVersion();
	} catch (Exception e) {
	  ret = e;
	}

	ByteArrayOutputStream bos = new ByteArrayOutputStream(1024);
	ObjectOutputStream oos = new ObjectOutputStream(bos);
	oos.writeObject(ret);
	dp.setData(bos.toByteArray(), 0, bos.size());
	ds.send(dp);
	dis.close();
      } catch (IOException ie) {
	JServer.printException(dp.getAddress().getHostAddress() + ": handleVersionRequest caught an error", ie, JServer.DEBUG_ERROR);
      }
    }

  }

  public String getStatus (int permit) {
    StringBuffer sb = new StringBuffer();
		 
    try {
      String cmd = new String(curDir + File.separator + "iserver all status");
      JServer.printDebug("Will execute - " + cmd, JServer.DEBUG_NORMAL);

      Process p = Runtime.getRuntime().exec(cmd);
      BufferedReader br = new BufferedReader(new InputStreamReader(new SequenceInputStream(p.getInputStream(), p.getErrorStream())), 1024);

      // read all the input streams
      String line = null;
      while ((line = br.readLine()) != null) {
	    // send write password only if the write permission matched
	      if (line.indexOf(WRITE_STRING) != -1 && permit == READ_ALLOWED)
	          continue;
	      sb.append(line + "\n");
      }

      try {
	      p.waitFor();
      } catch (InterruptedException ie) {}
      p.destroy();

    } catch (IOException ie) {
      sb.append("Error processing status request:\n\t" + ie.toString() + "\n");
    }

    return sb.toString();
  }

  /**
   * handle status request
   */
  private class HandleStatusRequest extends Thread {
    private InetAddress toAddr;
    private LimitedDataInputStream dis;
    private int toPort;

    HandleStatusRequest (DatagramPacket dp, LimitedDataInputStream dis) {
      this.toAddr = dp.getAddress();
      this.toPort = dp.getPort();
      this.dis = dis;
    }

    public void run () {

      try {
	      short protVersion = dis.readShort();
	      String read = dis.readUTF();
	      String write = dis.readUTF();

	      int permit = checkPermission(protVersion, read, write, toAddr);
	      if (permit == NOT_ALLOWED) {
	        dis.close();
	        return;  // requester will just timeout
	      }

	      String ret = getStatus(permit);

	      JServer.printDebug("Connecting to: " + toAddr.getHostAddress() + ":" + toPort + " to send status", JServer.DEBUG_VERBOSE);
	      Socket s = new Socket(toAddr, toPort);
	      ObjectOutputStream oos = new ObjectOutputStream(s.getOutputStream());
	      oos.writeObject(ret);
	      oos.flush();
	      oos.close();
	      s.close();
	      dis.close();
      } catch (IOException ie) {
      	JServer.printException(toAddr.getHostAddress() + ": handleStatusRequest caught an error", ie, JServer.DEBUG_ERROR);
      }
      System.gc();
    }

  }


  /**
   * handle permission change request
   */
  private void handlePermissionChangeRequest (DatagramPacket dp, LimitedDataInputStream dis) {
    try {
      short protVersion = dis.readShort();
      String read = dis.readUTF();
      String write = dis.readUTF();

      int permit = checkPermission(protVersion, read, write, dp.getAddress());
      if (permit != READ_WRITE_ALLOWED &&
	  permit != WRITE_ALLOWED) {
	dis.close();
	return;  // requester will just timeout
      }

      String rp = dis.readUTF();
      String wp = dis.readUTF();
      Object ret = null;
      try {
	JServer.printDebug(dp.getAddress().getHostAddress() + ": Changing permissions to: \"" + rp + "\" - \"" + wp + "\"", JServer.DEBUG_NORMAL);
	ret = changePermission(rp, wp);
	declareDbStale();
      } catch (Exception e) {
	JServer.printException(dp.getAddress().getHostAddress() + ": Error while changing permissions", e, JServer.DEBUG_ERROR);
	ret = e;
      }

      ByteArrayOutputStream bos = new ByteArrayOutputStream(1024);
      ObjectOutputStream oos = new ObjectOutputStream(bos);
      oos.writeObject(ret);
      dp.setData(bos.toByteArray(), 0, bos.size());
      ds.send(dp);
      dis.close();
    } catch (IOException ie) {
      JServer.printException(dp.getAddress().getHostAddress() + ": handleChangePermissionRequest caught an error", ie, JServer.DEBUG_ERROR);
    }
    System.gc();
  }



  /**
   * handle maintenance group/action related requests
   */
  private void handleMaintenanceRequest (DatagramPacket dp, LimitedDataInputStream dis) {
    try {
      short protVersion = dis.readShort();
      String read = dis.readUTF();
      String write = dis.readUTF();
      short op = dis.readShort();

      Object ret = null;
      Socket s = new Socket(dp.getAddress(), dp.getPort());

      try {
	if (!isOperationAllowed(protVersion, read, write, op, dp.getAddress())) {
	  dis.close();
	  return;  // stay silent, requester would just time out
	}
      } catch (BridgeException be) {
	ret = be;
        op = -1;
      }

      switch (op) {
      case Constants.MAINTENANCE_GET_GROUP_NAMES:
	try {
	  JServer.printDebug(dp.getAddress().getHostAddress() + ": Listing maintenance group names", JServer.DEBUG_NORMAL);
	  ret = bs.getMaintenanceGroupNames();
	} catch (Exception e) {
	  JServer.printException(dp.getAddress().getHostAddress() + ": Error listing maintenance group names", e, JServer.DEBUG_ERROR);
	  ret = e;
	}
	break;

      case Constants.MAINTENANCE_GET_GROUP:
	String name = dis.readUTF();
	try {
	  JServer.printDebug(dp.getAddress().getHostAddress() + ": Retrieving maintenance group (" + name + ")", JServer.DEBUG_NORMAL);
	  ret = bs.getMaintenanceGroup(name);
	} catch (Exception e) {
	  JServer.printException(dp.getAddress().getHostAddress() + ": Error retrieving maintenance group (" + name + ")", e, JServer.DEBUG_ERROR);
	  ret = e;
	}
	break;

      case Constants.MAINTENANCE_PUT_GROUP:
	try {
	  ObjectInputStream ois = new ObjectInputStream(s.getInputStream());
	  MaintenanceGroup mg = (MaintenanceGroup)ois.readObject();
	  JServer.printDebug(dp.getAddress().getHostAddress() + ": Storing maintenance group (" + mg.getName() + ")", JServer.DEBUG_NORMAL);
	  boolean r = bs.storeMaintenanceGroup(mg);
	  ret = new Boolean(r);
	  declareDbStale();
	} catch (Exception e) {
	  JServer.printException(dp.getAddress().getHostAddress() + ": Error storing maintenance group", e, JServer.DEBUG_ERROR);
	  ret = e;
	}
	break;

      case Constants.MAINTENANCE_DELETE_GROUP:
	name = dis.readUTF();
	try {
	  JServer.printDebug(dp.getAddress().getHostAddress() + ": deleting maintenance group (" + name + ")", JServer.DEBUG_NORMAL);
	  boolean r = bs.deleteMaintenanceGroup(name);
	  ret = new Boolean(r);
	  declareDbStale();
	} catch (Exception e) {
	  JServer.printException(dp.getAddress().getHostAddress() + ": Error deleting maintenance group (" + name + ")", e, JServer.DEBUG_ERROR);
	  ret = e;
	}
	break;

      case Constants.MAINTENANCE_GET_GROUP_NAMES_COMMENTS:
	try {
	  JServer.printDebug(dp.getAddress().getHostAddress() + ": Listing maintenance group names and comments", JServer.DEBUG_NORMAL);
	  ret = bs.getMaintenanceGroupNamesAndComments();
	} catch (Exception e) {
	  JServer.printException(dp.getAddress().getHostAddress() + ": Error listing maintenance group names and comments", e, JServer.DEBUG_ERROR);
	  ret = e;
	}
	break;

      case Constants.MAINTENANCE_GET_REQUEST_NAMES:
	try {
	  JServer.printDebug(dp.getAddress().getHostAddress() + ": Listing maintenance request names", JServer.DEBUG_NORMAL);
	  ret = bs.getMaintenanceRequestNames();
	} catch (Exception e) {
	  JServer.printException(dp.getAddress().getHostAddress() + ": Error listing maintenance request names", e, JServer.DEBUG_ERROR);
	  ret = e;
	}
	break;

      case Constants.MAINTENANCE_GET_REQUEST:
	name = dis.readUTF();
	try {
	  JServer.printDebug(dp.getAddress().getHostAddress() + ": Retrieving maintenance request (" + name + ")", JServer.DEBUG_NORMAL);
	  ret = bs.getMaintenanceRequest(name);
	} catch (Exception e) {
	  JServer.printException(dp.getAddress().getHostAddress() + ": Error retrieving maintenance request (" + name + ")", e, JServer.DEBUG_ERROR);
	  ret = e;
	}
	break;

      case Constants.MAINTENANCE_PUT_REQUEST:
	try {
	  ObjectInputStream ois = new ObjectInputStream(s.getInputStream());
	  MaintenanceRequest mr = (MaintenanceRequest)ois.readObject();
	  JServer.printDebug(dp.getAddress().getHostAddress() + ": Storing maintenance request (" + mr.getName() + ")", JServer.DEBUG_NORMAL);
	  boolean r = bs.storeMaintenanceRequest(mr);
	  ret = new Boolean(r);
	  declareDbStale();
	} catch (Exception e) {
	  JServer.printException(dp.getAddress().getHostAddress() + ": Error storing maintenance request", e, JServer.DEBUG_ERROR);
	  ret = e;
	}
	break;

      case Constants.MAINTENANCE_DELETE_REQUEST:
	name = dis.readUTF();
	try {
	  JServer.printDebug(dp.getAddress().getHostAddress() + ": deleting maintenance request (" + name + ")", JServer.DEBUG_NORMAL);
	  boolean r = bs.deleteMaintenanceRequest(name);
	  ret = new Boolean(r);
	  declareDbStale();
	} catch (Exception e) {
	  JServer.printException(dp.getAddress().getHostAddress() + ": Error deleting maintenance request (" + name + ")", e, JServer.DEBUG_ERROR);
	  ret = e;
	}
	break;

      case Constants.MAINTENANCE_REQUEST_ACTIVE:
	name = dis.readUTF();
	try {
	  JServer.printDebug(dp.getAddress().getHostAddress() + ": checking maintenance request (" + name + ") for active", JServer.DEBUG_VERBOSE);
	  boolean r = bs.isMaintenanceRequestActive(name);
	  ret = new Boolean(r);
	} catch (Exception e) {
	  JServer.printException(dp.getAddress().getHostAddress() + ": Error checking maintenance request (" + name + ") for active", e, JServer.DEBUG_ERROR);
	  ret = e;
	}
	break;

      case Constants.MAINTENANCE_REQUEST_ABORT:
	name = dis.readUTF();
	try {
	  JServer.printDebug(dp.getAddress().getHostAddress() + ": aborting maintenance request (" + name + ")", JServer.DEBUG_NORMAL);
	  bs.abortMaintenanceRequest(name);
	  ret = new Boolean(true);  // implementation side effect
	} catch (Exception e) {
	  JServer.printException(dp.getAddress().getHostAddress() + ": Error aborting maintenance request (" + name + ")", e, JServer.DEBUG_ERROR);
	  ret = e;
	}
	break;

      case Constants.MAINTENANCE_GET_REQUEST_NAMES_COMMENTS:
	try {
	  JServer.printDebug(dp.getAddress().getHostAddress() + ": Listing maintenance request names and comments", JServer.DEBUG_NORMAL);
	  ret = bs.getMaintenanceRequestNamesAndComments();
	} catch (Exception e) {
	  JServer.printException(dp.getAddress().getHostAddress() + ": Error listing maintenance request names and comments", e, JServer.DEBUG_ERROR);
	  ret = e;
	}
	break;

      case Constants.MAINTENANCE_GET_LOG_NAMES:
	try {
	  JServer.printDebug(dp.getAddress().getHostAddress() + ": Listing maintenance log names", JServer.DEBUG_NORMAL);
	  ret = bs.getMaintenanceLogNames();
	} catch (Exception e) {
	  JServer.printException(dp.getAddress().getHostAddress() + ": Error listing maintenance log names", e, JServer.DEBUG_ERROR);
	  ret = e;
	}
	break;

      case Constants.MAINTENANCE_DELETE_LOG:
	name = dis.readUTF();
	try {
	  JServer.printDebug(dp.getAddress().getHostAddress() + ": deleting maintenance log (" + name + ")", JServer.DEBUG_NORMAL);
	  boolean r = bs.deleteMaintenanceLog(name);
	  ret = new Boolean(r);
	  declareDbStale();
	} catch (Exception e) {
	  JServer.printException(dp.getAddress().getHostAddress() + ": Error deleting maintenance log (" + name + ")", e, JServer.DEBUG_ERROR);
	  ret = e;
	}
	break;
      }

      // return the result object over a TCP connection
      ObjectOutputStream oos = new ObjectOutputStream(s.getOutputStream());
      oos.writeObject(ret);
      oos.flush();
      s.close();
      dis.close();
    } catch (IOException ie) {
      JServer.printException(dp.getAddress().getHostAddress() + ": handleMaintenance caught an error", ie, JServer.DEBUG_ERROR);
    }
    System.gc();
  }


  /**
   * handle auto download related requests
   */
  private void handleAutoDownload (DatagramPacket dp, LimitedDataInputStream dis) {
    try {
      short protVersion = dis.readShort();
      String read = dis.readUTF();
      String write = dis.readUTF();
      short op = dis.readShort();

      Object ret = null;
      Socket s = new Socket(dp.getAddress(), dp.getPort());

      try {
	if (!isOperationAllowed(protVersion, read, write, op, dp.getAddress())) {
	  dis.close();
	  return;  // stay silent, requester would just time out
	}
      } catch (BridgeException be) {
	ret = be;
        op = -1;
      }

      switch (op) {
      case Constants.AUTO_DOWNLOAD_GET_NAMES:
	try {
	  JServer.printDebug(dp.getAddress().getHostAddress() + ": Listing auto-download group names", JServer.DEBUG_NORMAL);
	  ret = bs.getAutoDownloadGroupNames();
	} catch (Exception e) {
	  JServer.printException(dp.getAddress().getHostAddress() + ": Error listing auto-download group names", e, JServer.DEBUG_ERROR);
	  ret = e;
	}
	break;

      case Constants.AUTO_DOWNLOAD_GET_NAMES_COMMENTS:
	try {
	  JServer.printDebug(dp.getAddress().getHostAddress() + ": Listing auto-download group names and comments", JServer.DEBUG_NORMAL);
	  ret = bs.getAutoDownloadGroupNamesAndComments();
	} catch (Exception e) {
	  JServer.printException(dp.getAddress().getHostAddress() + ": Error listing auto-download group names and comments", e, JServer.DEBUG_ERROR);
	  ret = e;
	}
	break;

      case Constants.AUTO_DOWNLOAD_GET_CONFIG:
	String name = dis.readUTF();
	short isGroup = dis.readShort();
	try {
	  JServer.printDebug(dp.getAddress().getHostAddress() + ": Retrieving auto-download configuration for " + name + "/" + isGroup, JServer.DEBUG_NORMAL);
	  ret = bs.getAutoDownloadConfig(name, (isGroup == 1));
	} catch (Exception e) {
	  JServer.printException(dp.getAddress().getHostAddress() + ": Error retrieving auto-download configuration for " + name + "/" + isGroup, e, JServer.DEBUG_ERROR);
	  ret = e;
	}
	break;

      case Constants.AUTO_DOWNLOAD_PUT_CONFIG:
	isGroup = dis.readShort();
	try {
	  ObjectInputStream ois = new ObjectInputStream(s.getInputStream());
	  MaintenanceRequest mr = (MaintenanceRequest)ois.readObject();
	  JServer.printDebug(dp.getAddress().getHostAddress() + ": Storing auto-download configuration for " + mr.getName() + "/" + isGroup, JServer.DEBUG_NORMAL);
	  boolean r = bs.storeAutoDownloadConfig(mr, (isGroup == 1));
	  ret = new Boolean(r);
	  declareDbStale();
	} catch (Exception e) {
	  JServer.printException(dp.getAddress().getHostAddress() + ": Error storing auto-download configuration", e, JServer.DEBUG_ERROR);
	  ret = e;
	}
	break;

      case Constants.AUTO_DOWNLOAD_DELETE:
	name = dis.readUTF();
	isGroup = dis.readShort();
	try {
	  JServer.printDebug(dp.getAddress().getHostAddress() + ": deleting auto-download configuration (" + name + "/" + isGroup + ")", JServer.DEBUG_NORMAL);
	  boolean r = bs.deleteAutoDownloadConfig(name, (isGroup == 1));
	  ret = new Boolean(r);
	  declareDbStale();
	} catch (Exception e) {
	  JServer.printException(dp.getAddress().getHostAddress() + ": Error deleting auto-download configuration (" + name + "/" + isGroup + ")", e, JServer.DEBUG_ERROR);
	  ret = e;
	}
	break;

      case Constants.AUTO_DOWNLOAD_ACTIVE:
	name = dis.readUTF();
	try {
	  JServer.printDebug(dp.getAddress().getHostAddress() + ": checking auto-download of " + name + " for active", JServer.DEBUG_VERBOSE);
	  boolean r = bs.isAutoDownloadActive(name);
	  ret = new Boolean(r);
	} catch (Exception e) {
	  JServer.printException(dp.getAddress().getHostAddress() + ": Error checking auto-download of " + name + " for active", e, JServer.DEBUG_ERROR);
	  ret = e;
	}
	break;

      case Constants.AUTO_DOWNLOAD_ABORT:
	name = dis.readUTF();
	try {
	  JServer.printDebug(dp.getAddress().getHostAddress() + ": aborting auto-download of " + name, JServer.DEBUG_NORMAL);
	  bs.abortAutoDownload(name);
	  ret = new Boolean(true);  // implementation side effect
	} catch (Exception e) {
	  JServer.printException(dp.getAddress().getHostAddress() + ": Error aborting auto-download of " + name, e, JServer.DEBUG_ERROR);
	  ret = e;
	}
	break;

      case Constants.AUTO_DOWNLOAD_DELETE_LOG:
	name = dis.readUTF();
	try {
	  JServer.printDebug(dp.getAddress().getHostAddress() + ": deleting auto-download log of " + name, JServer.DEBUG_NORMAL);
	  boolean r = bs.deleteAutoDownloadLogFile(name);
	  ret = new Boolean(r);
	  declareDbStale();
	} catch (Exception e) {
	  JServer.printException(dp.getAddress().getHostAddress() + ": Error deleting auto-download log of " + name, e, JServer.DEBUG_ERROR);
	  ret = e;
	}
	break;
      }

      // return the result object over a TCP connection
      ObjectOutputStream oos = new ObjectOutputStream(s.getOutputStream());
      oos.writeObject(ret);
      oos.flush();
      s.close();
      dis.close();
    } catch (IOException ie) {
      JServer.printException(dp.getAddress().getHostAddress() + ": handleAutoDownload caught an error", ie, JServer.DEBUG_ERROR);
    }
    System.gc();
  }


  /**
   * handle lookup requests
   */
  private void handleLookup (DatagramPacket dp, LimitedDataInputStream dis) {
    try {
      short protVersion = dis.readShort();
      String read = dis.readUTF();
      String write = dis.readUTF();
      short op = dis.readShort();

      Object ret = null;

      try {
	if (!isOperationAllowed(protVersion, read, write, op, dp.getAddress())) {
	  dis.close();
	  return;  // stay silent, requester would just time out
	}
      } catch (BridgeException be) {
	ret = be;
      }

      switch (op) {
      case Constants.LOOKUP_BY_PHONE:
	String phone = dis.readUTF();
	lastSubCommand = "phone lookup " + phone;
	try {
	  JServer.printDebug(dp.getAddress().getHostAddress() + ": Lookup for phone (" + phone + ")", JServer.DEBUG_NORMAL);
	  ret = bs.getIedgeParamsForPhone(phone);
	} catch (Exception e) {
	  JServer.printException(dp.getAddress().getHostAddress() + ": Error lookup for phone (" + phone + ")", e, JServer.DEBUG_ERROR);
	  ret = e;
	}
	break;

      case Constants.LOOKUP_BY_VPNPHONE:
	phone = dis.readUTF();
	int extLen = dis.readInt();
	lastSubCommand = "vpn phone lookup " + phone + "/" + extLen;
	try {
	  JServer.printDebug(dp.getAddress().getHostAddress() + ": Lookup for vpn phone (" + phone + ":" + extLen + ")", JServer.DEBUG_NORMAL);
	  ret = bs.getIedgeParamsForVpnPhone(phone, extLen);
	} catch (Exception e) {
	  JServer.printException(dp.getAddress().getHostAddress() + ": Error lookup for vpn phone (" + phone + ":" + extLen + ")", e, JServer.DEBUG_ERROR);
	  ret = e;
	}
	break;

      case Constants.LOOKUP_BY_CALLINGPLAN_ROUTE_NAME:
	String routeName = dis.readUTF();
	lastSubCommand = "route name lookup " + routeName;
	try {
	  JServer.printDebug(dp.getAddress().getHostAddress() + ": Lookup for Calling plan route name(" + routeName+ ")", JServer.DEBUG_NORMAL);
	  ret = bs.getCallingPlanRoute(routeName);
	} catch (Exception e) {
	  JServer.printException(dp.getAddress().getHostAddress() + ": Error lookup for Calling plan route name(" + routeName+ ")", e, JServer.DEBUG_ERROR);
	  ret = e;
	}
	break;

      }

      // return the result object back over the same datagram packet
      ByteArrayOutputStream bos = new ByteArrayOutputStream(512);
      ObjectOutputStream oos = new ObjectOutputStream(bos);
      oos.writeObject(ret);
      oos.flush();
      dp.setData(bos.toByteArray(), 0, bos.size());
      ds.send(dp);
      dis.close();
    } catch (IOException ie) {
      JServer.printException(dp.getAddress().getHostAddress() + ": handleLookup caught an error", ie, JServer.DEBUG_ERROR);
    }
    System.gc();
  }

  private void handleDbRequest (DatagramPacket dp, LimitedDataInputStream dis)
  {

    try {
      short protVersion = dis.readShort();
      String read = dis.readUTF();
      String write = dis.readUTF();
      short op = dis.readShort();

      Object ret = null;

      try {
      	if (!isOperationAllowed(protVersion, read, write, op, dp.getAddress())) {
      	  dis.close();
	  return;  // stay silent, requester would just time out
	}
        checkDatabaseMaster();
      } catch (BridgeException be) {
	ret = be;
        op = -1;
      }

      switch (op) {
      case Constants.ISERVER_DB_GET_FILENAMES:
	JServer.printDebug("reading the db file names...",JServer.DEBUG_VERBOSE);
	lastSubCommand = "get filenames";
	try{
	  ret  = bs.getDBFilenames();

	} catch (Exception e) {
	  JServer.printException("Error while getting sb file names", e, JServer.DEBUG_ERROR);
	  ret = e;
	}
	break;
      case  Constants.MAX_RECORDS:
	JServer.printDebug("reading the maximum records...",JServer.DEBUG_VERBOSE);
	lastSubCommand = "get maximum records";
	int  listType = dis.readInt();
        try{
          ret = new Integer(bs.getMaximumRecords(listType));
	} catch (Exception e) {
	  JServer.printException("Error while reading the maximum records", e, JServer.DEBUG_ERROR);
	  ret = e;
	}
	break;
      default:
        break;
      }

      // return the result object back over the same datagram packet
      ByteArrayOutputStream bos = new ByteArrayOutputStream(512);
      ObjectOutputStream oos = new ObjectOutputStream(bos);
      oos.writeObject(ret);
      oos.flush();
      dp.setData(bos.toByteArray(), 0, bos.size());
      ds.send(dp);
      dis.close();
    } catch (IOException ie) {
      JServer.printException(dp.getAddress().getHostAddress() + ": handleDbRequest caught an error", ie, JServer.DEBUG_ERROR);
    }

  }


  /**
   * as required by the UDPServerWorker interface
   */
  public void UDPServerWork (DatagramSocket socket) {
    boolean restart = false;
    keepRunning = true;
    byte [] in = new byte [Constants.MAX_IVIEW_ISERVER_PACKET_SIZE];
    DatagramPacket dp = null;
    while (keepRunning) {
      try {
	Arrays.fill(in, (byte)0);
	dp = new DatagramPacket(in, in.length);

	try {
	  socket.receive(dp);
	} catch (IOException ie) {
	  JServer.printException("UDPServerWork: receive error", ie, JServer.DEBUG_ERROR);
	  continue;
	}

	lastiView = dp.getAddress().getHostAddress();
	lastCode = lastCommand = lastSubCommand = "";  // reset these for a new command
	LimitedDataInputStream dis = new LimitedDataInputStream(new ByteArrayInputStream(in), dp.getLength());
	short code = dis.readShort();
	switch (code) {
	case Constants.RESTART:    // to stop this thread
	  lastCode = "restart";
	  JServer.printDebug("restart  signal from " + dp.getAddress().getHostAddress(), JServer.DEBUG_NORMAL);
	  keepRunning = false;
          restart = true;
	  break;
	case Constants.STOP:    // to stop this thread
	  lastCode = "stop";
	  JServer.printDebug("Stop signal from " + dp.getAddress().getHostAddress(), JServer.DEBUG_NORMAL);
	  if (isSecure(dis, dp.getAddress()))
	    keepRunning = false;
	  break;

	case Constants.STATUS:   // send a status back
	  lastCode = "status";
	  if (isSecure(dis, dp.getAddress()))
	    handleStatus(dp);
	  break;

	case Constants.DEBUG:
	  lastCode = "debug";
	  short level = dis.readShort();
	  if (isSecure(dis, dp.getAddress()))
	    handleDebug(level);
	  break;

	case Constants.ALIVE:
	  lastCode = "isalive";
	  // the process manager thread sends this packet, if we do not
	  // reply to it, he will let PM restart jserver
	  if (isSecure(dis, dp.getAddress()))
	    handleAlive(dp);
	  break;

	case Constants.UPTIME:
	  lastCode = "uptime";
	  if (isSecure(dis, dp.getAddress()))
	    handleUptime(dp);
	  break;

	case Constants.RECONFIG:
	  lastCode = "reconfig";
	  if (isSecure(dis, dp.getAddress())) {
	    System.out.println("processing configuration file...");
	    int lvl = reconfig();
	    handleDebug(lvl);
	  }
	  break;

	  // iedges would send this upon startup
	case Constants.REGISTRATION:
	  lastCode = "registration";
	  JServer.printDebug("Startup registration from " + dp.getAddress().getHostAddress(), JServer.DEBUG_NORMAL);
          //	  AutoDownload.receivedRegistration(dis, dp, bs);
	  break;

	case Constants.HELLO:
	  lastCode = "hello";
	  short protVersion = dis.readShort();
	  // extract permission strings
	  String rp = dis.readUTF();
	  while (dis.getCount() < 24)
	    dis.readByte();
	  String wp = dis.readUTF();
	  while (dis.getCount() < 44)
	    dis.readByte();
          String toAddr = null;
          try{
            dis.readShort();
            dis.readShort();
            dis.readByte();
            toAddr  =  dis.readUTF(); 
	    JServer.printDebug(dp.getAddress().toString() + " To address  " +toAddr, JServer.DEBUG_WARNING);
          }catch(Exception e){
	    JServer.printDebug(dp.getAddress().toString() + "Client is using older version of iView " , JServer.DEBUG_WARNING);
	  JServer.printException("UDPServerWork: receive error", e, JServer.DEBUG_ERROR);
          } 

	  if (checkPermission(protVersion, rp, wp, dp.getAddress()) != NOT_ALLOWED)
	    handleHello(dp, dis,toAddr);
	  break;

	case Constants.COMMAND:
	  lastCode = "command";
	  short cmdcode = dis.readShort();
	  if (bs.isDbOperationInProgress()) {
	    JServer.printDebug(dp.getAddress().toString() + ": JServer thread busy doing database operations, ignoring command code: " + cmdcode, JServer.DEBUG_WARNING);
	    // return an error indicating we are busy
	    ByteArrayOutputStream bos = new ByteArrayOutputStream(512);
	    ObjectOutputStream oos = new ObjectOutputStream(bos);
	    oos.writeObject(new BridgeException("iServer database is busy, please try again later"));
	    oos.flush();
	    dp.setData(bos.toByteArray(), 0, bos.size());
	    ds.send(dp);
	    break;
	  }

	  int len = dp.getLength()-4;
	  byte [] data = new byte [len];
	  System.arraycopy(in, 4, data, 0, len);
	  LimitedDataInputStream newdis = new LimitedDataInputStream(new ByteArrayInputStream(data), len);

	  switch (cmdcode) {
	  case Constants.GET_COMMAND:
	    lastCommand = "get";
	    handleGetRequest(dp, newdis);
	    break;

	  case Constants.REBOOT_COMMAND:
	    lastCommand = "reboot";
	    handleRebootRequest(dp, newdis);
	    break;

	  case Constants.LIST_COMMAND:
	    lastCommand = "list";
	    handleListRequest(dp, newdis);
	    break;

	  case Constants.IEDGE_COMMAND:
	    lastCommand = "iedge";
	    handleIEdgeRequest(dp, newdis);
	    break;

	  case Constants.VERSION_COMMAND:
	    // this has to be a thread because it sends a packet
	    // to itself
	    lastCommand = "version";
	    new HandleVersionRequest(dp, newdis).start();
	    break;

	  case Constants.LS_ALARM_COMMAND:
	    lastCommand = "alarm";
	    handleAlarmRequest(dp, newdis);
	    break;

	  case Constants.STATUS_COMMAND:
	    lastCommand = "status";
	    // this request has to be handled in a separate thread
	    new HandleStatusRequest(dp, newdis).start();
	    break;

	  case Constants.PERMISSION:
	    lastCommand = "permission";
	    handlePermissionChangeRequest(dp, newdis);
	    break;

	  case Constants.MAINTENANCE_COMMAND:
	    lastCommand = "maintenance";
	    handleMaintenanceRequest(dp, newdis);
	    break;

	  case Constants.AUTO_DOWNLOAD_COMMAND:
	    lastCommand = "auto download";
	    handleAutoDownload(dp, newdis);
	    break;

	  case Constants.LOOKUP_COMMAND:
	    lastCommand = "lookup";
	    handleLookup(dp, newdis);
	    break;


	    // some multicast commands could reach us, ignore...
	  case Constants.HELLO_VERSION_NORMAL:
	  case Constants.HELLO_VERSION_NO_AUTH:
	    break;

	  case Constants.DB_COMMAND:
	    lastCommand = "db";
	    handleDbRequest(dp,newdis);
	    break;

	  case Constants.MISC_COMMAND:
	    lastCommand = "misc";
	    handleMiscReq(dp, newdis);
	    break;

	  default:
	    lastCommand = "unknown";
	    JServer.printDebug("JServer: unknown sub-command code received (" + cmdcode + ") from " + dp.getAddress().toString(), JServer.DEBUG_WARNING);
	    JServer.printDebug(JServer.dumpReceivedPacket(dp), JServer.DEBUG_VERBOSE);
	    ByteArrayOutputStream bos = new ByteArrayOutputStream(512);
	    ObjectOutputStream oos = new ObjectOutputStream(bos);
	    oos.writeObject(new OperationUnknownBridgeException(cmdcode));
	    dp.setData(bos.toByteArray(), 0, bos.size());
	    ds.send(dp);
	    newdis.close();
	    break;
	  }
	  break;

	default:
	  lastCode = "unknown";
	  JServer.printDebug("JServer: unknown command code received (" + code + ") from " + dp.getAddress().toString(), JServer.DEBUG_WARNING);
	  ByteArrayOutputStream bos = new ByteArrayOutputStream(512);
	  ObjectOutputStream oos = new ObjectOutputStream(bos);
	  oos.writeObject(new OperationUnknownBridgeException(code));
	  dp.setData(bos.toByteArray(), 0, bos.size());
	  ds.send(dp);
	  break;
	}
	dis.close();
      } catch (IOException ie) {
	if (dp != null && dp.getAddress() != null)
	  JServer.printException("JServer receive/process (" + dp.getAddress().toString() + ") error", ie, JServer.DEBUG_ERROR);
	else
	  JServer.printException("JServer receive/process error:", ie, JServer.DEBUG_ERROR);
      }
    }

    System.out.println(SysUtil.getDate() + ": Stopping NexTone Configuration Server...");

    // cleanup
    jstcp.stop();
    bs.stop();
    instance.stop();
    try {
      instance.getThread().join();
    } catch (InterruptedException ie) {}

    try {
      NextoneMulticastSocket m = (NextoneMulticastSocket)ds;
      m.leaveGroupOnAllIf(InetAddress.getByName(Constants.MCAST_ADDRESS));
    } catch (IOException ie) {
      System.err.println("JServer: error leaving group:" + ie.toString());
    }
    JServer.printDebug("JServer exited\n", JServer.DEBUG_NORMAL);
    if(restart){
      try{ 
        ByteArrayOutputStream bos = new ByteArrayOutputStream(512);
        ObjectOutputStream oos = new ObjectOutputStream(bos);
        oos.writeObject(new Boolean(true));
        dp.setData(bos.toByteArray(), 0, bos.size());
        ds.send(dp);
        ds.close();
      }catch(Exception e){
        JServer.printException("Error sending restart response code "+e.toString(),e,JServer.DEBUG_ERROR); 
      }
    }
    thisInstance = null;
    mgmtIp  =  null;

    //System.out.close();
    //System.err.close();
    //System.runFinalization();
    //System.exit(0);
  }



  public static String dumpReceivedPacket (DatagramPacket dp) {
    StringBuffer sb = new StringBuffer();
    try {
      LimitedDataInputStream dis = new LimitedDataInputStream(new ByteArrayInputStream(dp.getData()), dp.getLength());
      byte [] ba = dis.readAllBytes();
      for (int i = 0; i < ba.length; i++, sb.append(" "))
	sb.append(Integer.toHexString((ba[i]&0x000000ff)));
    } catch (Exception e) {
      JServer.printException("Error dumping packet", e, JServer.DEBUG_ERROR);
    }

    return sb.toString();
  }

  public static File getLogFile () throws BridgeException {
    File lf = new File(logfile);
    if (!lf.exists() || !lf.isFile())
      throw new BridgeException("File " + logfile + " does not exist");
    if (!lf.canRead())
      throw new BridgeException("Cannot read file " + logfile);
    return lf;
  }

  public static String getLastCommand () {
    StringBuffer sb = new StringBuffer("Packet from: ");
    sb.append(lastiView);
    sb.append("\nJServer Command: ");
    sb.append("Code: ");
    sb.append(lastCode);
    sb.append("  Command: ");
    sb.append(lastCommand);
    sb.append("  Sub Comand: ");
    sb.append(lastSubCommand);
    sb.append("\nNative Command: ");
    sb.append(getLastNativeCommand());
    sb.append("\nPID: ");
    sb.append(getPid());
    sb.append("  PGID: ");
    sb.append(getPgid());
    sb.append("  PPID: ");
    sb.append(getPpid());
    return sb.toString();
  }

}


