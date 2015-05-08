package com.nextone.common;

import java.net.*;
import java.io.*;
import java.util.*;
import java.util.zip.*;
import com.nextone.util.SysUtil;

/**
 * This is the client side implementation of the Bridge interface. This is
 * used to communicate with the iServer
 */
public class BridgeClientImpl implements Bridge, CommonConstants {
  private InetAddress addr;
  private PermissionProvider iserver;
  private String readPass;
  private String writePass;
  private long lastContactTime;
  private TimeoutProvider tp;
  private short protVersion = CommonConstants.HELLO_VERSION_NORMAL;
  private boolean dbOp;
  private int compression; // 0 no compression, 1->compression for all tcp comm.

  /**
   * when using this constructor, the default protocol used is NO_AUTH
   *
   * @param addr the ip address of the iServer
   * @param tp the timeout provider
   */
  public BridgeClientImpl (InetAddress addr, TimeoutProvider tp) throws UnknownHostException {
    this.addr = addr;
    this.iserver = null;
    this.readPass = null;
    this.writePass = null;
    this.tp = tp;
    this.protVersion = CommonConstants.HELLO_VERSION_NO_AUTH;

    /* try to contact the server */
    handShake();
  }

  /**
   * when using this constructor, the default protocol used is NORMAL
   *
   * @param addr the ip address of the iServer
   * @param iserver the permissions provider
   * @param tp the timeout provider
   */
  public BridgeClientImpl (InetAddress addr, PermissionProvider iserver, TimeoutProvider tp) throws UnknownHostException {
    this.addr = addr;
    this.iserver = iserver;
    this.readPass = null;
    this.writePass = null;
    this.tp = tp;

    /* try and contact the server */
    handShake();
  }

  /**
   * when using this constructor, the default protocol used is NORMAL
   *
   * @param addr the ip address of the iServer
   * @param rp the read permission string to be used
   * @param wp the write permission string to be used
   * @param tp the timeout provider
   */
  public BridgeClientImpl (InetAddress addr, String rp, String wp, TimeoutProvider tp) throws UnknownHostException {
    this.addr = addr;
    this.iserver = null;
    this.readPass = rp;
    this.writePass = wp;
    this.tp = tp;

    /* try and contact the server */
    handShake();
  }

  public InetAddress getAddress () {
    return addr;
  }

  /**
   * set the protocol version to use for all future communications using
   * this bridge
   *
   * @param protVersion the protocol version to use
   */
  public void setProtocolVersion (short protVersion) {
    this.protVersion = protVersion;
  }

  /**
   * get the current protocol version used by this bridge
   */
  public short getProtocolVersion () {
    return protVersion;
  }

  /**
   * returns the read permission string to be used
   */
  private String getReadPermission () {
    String str = (iserver != null)?iserver.getReadPermission(addr):readPass;
    return (str == null)?"":str;
  }

  /**
   * returns the write permission string to be used
   */
  private String getWritePermission () {
    String str = (iserver != null)?iserver.getWritePermission(addr):writePass;
    return (str == null)?"":str;
  }

  /**
   * creates the packet header for a command packet to the iserver
   */
  private void createPacketHeader (DataOutputStream dos, int cmdCode, int subCmdCode) throws IOException {
    createPacketHeader(dos, cmdCode);
    dos.writeShort(subCmdCode);
  }

  /**
   * creates the packet header for a command packet to the iserver
   */
  private void createPacketHeader (DataOutputStream dos, int cmdCode) throws IOException {
    dos.writeShort(CommonConstants.COMMAND);
    dos.writeShort(cmdCode);
    dos.writeShort(protVersion);  // prot version, unused for now
    dos.writeUTF(getReadPermission());	
    dos.writeUTF(getWritePermission());
  }

  /**
   * a generic UDP packet processing method. Checks for a boolean object
   * from the iserver
   *
   * @param data the data to be sent
   */
  private boolean processUDPPacket (byte [] data) throws Exception {
    return processUDPPacket(data, tp.getGetTimeout()*1000);
  }

  /**
   * a generic UDP packet processing method. Checks for a boolean object
   * from the iserver
   *
   * @param data the data to be sent
   * @param timeout the timeout to be used in setSoTimeout()
   */
  private boolean processUDPPacket (byte [] data, int timeout) throws Exception {
    Boolean b = (Boolean)processUDPPacket(data, Boolean.class, timeout);
    return (b == null)?false:b.booleanValue();
  }

  /**
   * a generic TCP packet processing method. Checks for a boolean object
   * from the iserver
   *
   * @param data the data to be sent
   */
  private boolean processTCPPacket (byte [] data) throws Exception {
    return processTCPPacket(data, null);
  }

  /**
   * a generic TCP packet processing method. Checks for a boolean object
   * from the iserver
   *
   * @param data the data to be sent
   * @param timeout the timeout to be used in setSoTimeout()
   */
  private boolean processTCPPacket (byte [] data, int timeout) throws Exception {
    return processTCPPacket(data, null, timeout);
  }

  /**
   * a generic TCP packet processing method. Checks for a boolean object
   * from the iserver
   */
  private boolean processTCPPacket (byte [] data, Object sendObject) throws Exception {
    Boolean b = (Boolean)processTCPPacket(data, sendObject, Boolean.class);
    return (b == null)?false:b.booleanValue();
  }

  /**
   * a generic TCP packet processing method. Checks for a boolean object
   * from the iserver
   */
  private boolean processTCPPacket (byte [] data, Object sendObject, int timeout) throws Exception {
    Boolean b = (Boolean)processTCPPacket(data, sendObject, Boolean.class, timeout);
    return (b == null)?false:b.booleanValue();
  }

  /**
   * a generic UDP packet processing method. Checks for a passed
   * object type from the iserver
   *
   * @param data the data to be sent
   * @param cl the class to expect in the reply
   */
  private Object processUDPPacket (byte [] data, Class cl) throws Exception {
    return processUDPPacket(data, cl, tp.getGetTimeout()*1000);
  }

  /**
   * a generic UDP packet processing method. Checks for a passed
   * object type from the iserver
   *
   * @param data the data to be sent
   * @param cl the class to expect in the reply
   * @param timeout  the timeout to be used in setSoTimeout()
   */
  private Object processUDPPacket (byte [] data, Class cl, int timeout) throws Exception {
    if (data.length > CommonConstants.MAX_IVIEW_ISERVER_PACKET_SIZE)
      throw new BridgeException("packet size too big (" + data.length + " > " + CommonConstants.MAX_IVIEW_ISERVER_PACKET_SIZE + ")");

    int retryInterval = CommonConstants.RETRY_INTERVAL;
    // for larger data packets, iserver takes a long time to process, so don't
    // retransmit for atleast 20 seconds
    if (data.length > 768)
      retryInterval = 20;

    int times = timeout/(retryInterval*1000);
    int remain = timeout%(retryInterval*1000);
    if (remain != 0)
      times++;
    DatagramPacket dpo = new DatagramPacket(data, data.length, addr, CommonConstants.MCAST_SEND_PORT);
    byte [] in = new byte [CommonConstants.MAX_IVIEW_ISERVER_PACKET_SIZE];
    DatagramPacket dpi = new DatagramPacket(in, in.length);
    DatagramSocket s = new DatagramSocket();
    while (times-- > 0) {
      try {
	if (times == 0 && remain != 0)
	  s.setSoTimeout(remain);
	else
	  s.setSoTimeout(retryInterval*1000);
	s.send(dpo);
	s.receive(dpi);
	break;  // receive successfull, break out of the loop
      } catch (InterruptedIOException iie) {
	if (times == 0)
	  throw iie;
      }
    }
    lastContactTime = System.currentTimeMillis();


    ObjectInputStream ois = new ObjectInputStream(new ByteArrayInputStream(in));
    Object o = ois.readObject();

    if (o == null || cl == o.getClass()) {
      s.close();
      return o;
    } else if (!(o instanceof Exception)) {
      o = new BridgeException("Unexpected reply received.\n    Received: " + o.getClass().getName() + ": " + o.toString() + "\n    Expecting: " + cl.getName());
    }

    s.close();
    throw (Exception)o;
  }

  /**
   * returns an array of sockets, array[0] is a DatagramSocket and
   * array[1] is a ServersSocket. Both sockets have the save local
   * port number
   */
  private Object [] getLocalSockets () throws BridgeException, IOException {
    ServerSocket ss = null;
    DatagramSocket ds = null;
    int tries = 0;

    while (tries++ < 100) {
      try {
	ss = new ServerSocket(0);
      } catch (Exception e) {
	continue;
      }
      try {
	ds = new DatagramSocket(ss.getLocalPort());
      } catch (Exception e) {
	ss.close();
	continue;
      }

      ss.setSoTimeout(tp.getGetTimeout()*1000);
      ds.setSoTimeout(tp.getGetTimeout()*1000);
      Object [] result = new Object [2];
      result[0] = ds;
      result[1] = ss;
      return result;
    }

    throw new BridgeException("Could not open a local socket (tried " + tries + " times)");
  }

  /**
   * a generic UDP packet processing method. Checks for a passed
   * object type from the iserver
   */
  private Object processTCPPacket (byte [] data, Object sendObject, Class cl) throws Exception {
    return processTCPPacket(data, sendObject, cl, tp.getGetTimeout()*1000);
  }
  /**
   * a generic TCP packet processing method. Checks for a passed
   * object type from the iserver
   */
  private Object processTCPPacket (byte [] data, Object sendObject, Class cl, int timeout) throws Exception {
    return processTCPPacket (data,sendObject,cl,timeout,null);
  }

  /**
   * a generic TCP packet processing method. Checks for a passed
   * object type from the iserver
   */
  private Object processTCPPacket (byte [] data, Object sendObject, Class cl, int timeout,Socket socket) throws Exception {

    Socket sock = socket;
    if(sock ==  null)
      sock  = new Socket(addr, CommonConstants.MCAST_SEND_PORT);
    sock.setSoTimeout(timeout);

    BufferedOutputStream bos = new BufferedOutputStream(sock.getOutputStream());
    DataOutputStream dos = null;
    GZIPOutputStream gos = null;
    if (compression == 0)
      dos = new DataOutputStream(bos);
    else {
      gos = new GZIPOutputStream(bos);
      dos = new DataOutputStream(gos);
    }

    dos.write(data, 0, data.length);

    if (sendObject != null) {
      if (sendObject.getClass().equals(File.class)) {
        File db = (File)sendObject;
        long fileSize = db.length();
        dos.writeLong(fileSize);
        FileInputStream fis = new FileInputStream(db);
        data = new byte [CommonConstants.MAX_IVIEW_ISERVER_PACKET_SIZE];
        int len = -1;
        long count = 0;
        while (count < fileSize && (len = fis.read(data)) != -1) {
          dos.write(data, 0, len);
          count += len;
        }
        fis.close();
      } else {
        ObjectOutputStream oos = new ObjectOutputStream(dos);
        oos.writeObject(sendObject);
      }
    }

    if (gos != null)
      gos.finish();
    bos.flush();

    //    sock.shutdownOutput();
    //    dos.close(); // if we close this here, the whole socket gets closed

    DataInputStream dis = null;
    if (compression == 0)
      dis = new DataInputStream(sock.getInputStream());
    else
      dis = new DataInputStream(new GZIPInputStream(new BufferedInputStream(sock.getInputStream())));
    ObjectInputStream ois = new ObjectInputStream(dis);

    Object o = ois.readObject();

    lastContactTime = System.currentTimeMillis();

    if (o == null || cl == o.getClass()) {
      if (o != null) {
    	  if (cl.equals(File.class)) {
          int fileSize = (int)dis.readLong();
          File dbFile  = new File(CommonConstants.DATA_DIR+File.separator+((File)o).getName());
          FileOutputStream fos = new FileOutputStream(dbFile);
          data = new byte [CommonConstants.MAX_IVIEW_ISERVER_PACKET_SIZE];
          int len = -1;
          int count = 0;
          while (count < fileSize && (len = dis.read(data)) != -1) {
            fos.write(data, 0, len);
            count += len;
          }
          fos.flush();
          fos.close();        
          o = dbFile;
        }
      }
      dos.close();
      dis.close();
      sock.close();
      return o;
    } else if (!(o instanceof Exception)) {
      o = new BridgeException("Unexpected reply received.\n    Received: " + o.getClass().getName() + ": " + o.toString() + "\n    Expecting: " + cl.getName());
    }

    sock.close();	
    throw (Exception)o;
  }

  /**
   * Send and receive a hello to make sure you can talk to the server.
   * You can also do more in this call to find out more about the
   * capabilities of the iServer
   */
  private void handShake () throws UnknownHostException {
    try {
      ByteArrayOutputStream bos = new ByteArrayOutputStream(7);
      DataOutputStream dos = new DataOutputStream(bos);
      dos.writeShort(CommonConstants.HELLO);
      dos.writeShort(protVersion);
      Hello.fillPermission(dos, getReadPermission());
      Hello.fillPermission(dos, getWritePermission());
      dos.writeInt(0);
      dos.writeByte(0);
      dos.close();

      DatagramPacket dpo = new DatagramPacket(bos.toByteArray(), bos.size(), addr, CommonConstants.MCAST_SEND_PORT);
      byte [] in = new byte [1024];
      DatagramPacket dpi = new DatagramPacket(in, in.length);
      DatagramSocket s = new DatagramSocket();
      s.setSoTimeout(tp.getGetTimeout()*1000);
      s.send(dpo);
      s.receive(dpi);
      s.close();
      lastContactTime = System.currentTimeMillis();      
    } catch (Exception e) {
      throw new UnknownHostException(e.getMessage());
    }

    // see if this msw is compression capable
    try {
      ByteArrayOutputStream bos = new ByteArrayOutputStream(128);
      DataOutputStream dos = new DataOutputStream(bos);
      createPacketHeader(dos, CommonConstants.MISC_COMMAND, CommonConstants.ISERVER_COMPRESSION);
      dos.close();
      Integer in = (Integer)processUDPPacket(bos.toByteArray(), Integer.class);
      compression = (in == null)?0:in.intValue();
    } catch (OperationUnknownBridgeException oe) {
      compression = 0;
    } catch (Exception e) {
      compression = 0;
      throw new UnknownHostException(e.getMessage());
    }
//    System.out.println("-----compression = " + compression);
  }

  /**
   * returns the last time at which a successful contact was made to this
   * iserver
   */
  public long getLastContactTime () {
    return lastContactTime;
  }

  /**
   * send a request to get provisioning data of the given serial/port pair
   * send a packet of the following format:
   * short COMMAND;
   * short GET_COMMAND;
   * String read;
   * String write;
   * short numKeys;
   * Object [] keys;
   * short 0;
   */		 
  public Object getProvData (short [] cmds, Object [] keys) throws Exception {

    ByteArrayOutputStream bos = new ByteArrayOutputStream(512);
    DataOutputStream dos = new DataOutputStream(bos);
    createPacketHeader(dos, CommonConstants.GET_COMMAND);
    dos.writeShort(keys.length);
    ObjectOutputStream oos = new ObjectOutputStream(dos);
    for (int i = 0; i < keys.length; i++)
      oos.writeObject(keys[i]);
    dos.writeShort(0);
    dos.close();

    ProvisionData pd = (ProvisionData)processUDPPacket(bos.toByteArray(), ProvisionData.class);

    //		 Hashtable r = new Hashtable();
    //		 for (int i = 0; i < cmds.length; i++)
    //			r.put(new Short(cmds[i]), pd.getValue(cmds[i]));
    //		 return r;
    return pd;
  }
	  
  /**
   * find the port on the iServer to contact to, to receive different
   * types of data (provisioning/calling plan, etc)
   *
   * @param listType the type of data we are interested in
   */
  public int getListPort (int listType) throws Exception {
    ByteArrayOutputStream bos = new ByteArrayOutputStream(512);
    DataOutputStream dos = new DataOutputStream(bos);
    createPacketHeader(dos, CommonConstants.LIST_COMMAND, listType);
    dos.close();

    return ((Integer)processUDPPacket(bos.toByteArray(), Integer.class)).intValue();
  }


  /**
   * get the iedge parameters (IEdgeList object)
   * @param regid the registration id of the iedge
   * @param port the port of the iedge (-1 if any port would suffice)
   */
		 
  public IEdgeList getIedgeParams (String regid, int port) throws Exception {
    ByteArrayOutputStream bos = new ByteArrayOutputStream(512);
    DataOutputStream dos = new DataOutputStream(bos);
    createPacketHeader(dos, CommonConstants.IEDGE_COMMAND, CommonConstants.IEDGE_GET);
    dos.writeUTF(regid);
    dos.writeInt(port);
    dos.close();

    return (IEdgeList)processUDPPacket(bos.toByteArray(), IEdgeList.class);
  }


  /**
   * Request version number information from the iserver.
   * Older iservers which don't support this option will stay silent
   * for this request.
   * 	Packet format:
   * 	short COMMAND;
   * 	short VERSION_COMMAND;
   * 	String read;
   * 	String write;
   * The returned String [] contains following information:
   * index   string
   * -----   ------
   *   0     release name
   *   1     major version
   *   2     minor version
   *   3     build date
   *   4     copyright stuff
   *   5     server name
   *   6     NSF version
   */
  public String [] getVersion () throws Exception {
    ByteArrayOutputStream bos = new ByteArrayOutputStream(128);
    DataOutputStream dos = new DataOutputStream(bos);
    createPacketHeader(dos, CommonConstants.VERSION_COMMAND);
    dos.close();

    return (String [])processUDPPacket(bos.toByteArray(), (new String [0]).getClass());
  }

  /**
   * change the permission strings on the iserver
   *
   * @param rp the new read permission
   * @param wp the new write permission
   */
  public boolean changePermissions (String rp, String wp) throws Exception {
    ByteArrayOutputStream bos = new ByteArrayOutputStream(512);
    DataOutputStream dos = new DataOutputStream(bos);
    createPacketHeader(dos, CommonConstants.PERMISSION);
    // the new permissions
    dos.writeUTF(rp);
    dos.writeUTF(wp);
    dos.close();

    return processUDPPacket(bos.toByteArray());
  }

  /**
   * show the status of the iserver processes
   */
  public String getStatus () throws Exception {
    ByteArrayOutputStream bos = new ByteArrayOutputStream(512);
    DataOutputStream dos = new DataOutputStream(bos);
    createPacketHeader(dos, CommonConstants.STATUS_COMMAND);
    dos.close();

    return (String)processTCPPacket(bos.toByteArray(), null, String.class, tp.getGetTimeout()*2000);
  }


  /**
   * lookup the iserver for the given phone number and return the
   * corresponding IEdgeList object
   */
  public  CallPlanRoute getCallingPlanRoute (String routeName) throws Exception {
    ByteArrayOutputStream bos = new ByteArrayOutputStream(256);
    DataOutputStream dos = new DataOutputStream(bos);
    createPacketHeader(dos, CommonConstants.LOOKUP_COMMAND, CommonConstants.LOOKUP_BY_CALLINGPLAN_ROUTE_NAME);
    dos.writeUTF(routeName);
    dos.close();

    return (CallPlanRoute)processUDPPacket(bos.toByteArray(), CallPlanRoute.class);		 
  }

  /**
   *    get the presence number 
   */
  public  String getPresence (String destPhone) throws Exception {
    ByteArrayOutputStream bos = new ByteArrayOutputStream(256);
    DataOutputStream dos = new DataOutputStream(bos);
    createPacketHeader(dos, CommonConstants.MISC_COMMAND, CommonConstants.PRESENCE_NUMBER);
    dos.writeUTF(destPhone);
    dos.close();

    return (String)processUDPPacket(bos.toByteArray(), String.class);		 
  }


  /**
   * get the list of names for the given command/sub-command
   */
  private String [] getNames (int cmd, int subcmd) throws Exception {
    ByteArrayOutputStream bos = new ByteArrayOutputStream(128);
    DataOutputStream dos = new DataOutputStream(bos);
    createPacketHeader(dos, cmd, subcmd);
    dos.close();

    return (String [])processTCPPacket(bos.toByteArray(), null, (new String [0]).getClass());
  }

  /**
   * get the list of names and comments for the given command/sub-command
   */
  private String [][] getNamesAndComments (int cmd, int subcmd) throws Exception {
    ByteArrayOutputStream bos = new ByteArrayOutputStream(128);
    DataOutputStream dos = new DataOutputStream(bos);
    createPacketHeader(dos, cmd, subcmd);
    dos.close();

    return (String [][])processTCPPacket(bos.toByteArray(), null, (new String [0][0]).getClass());
  }

  /**
   * get the list of names of all the maintenance groups available
   */
  public String [] getMaintenanceGroupNames () throws Exception {
    return getNames(CommonConstants.MAINTENANCE_COMMAND, CommonConstants.MAINTENANCE_GET_GROUP_NAMES);
  }

  /**
   * get the maintenance group object for the given name
   */
  public MaintenanceGroup getMaintenanceGroup (String group) throws Exception {
    ByteArrayOutputStream bos = new ByteArrayOutputStream(128);
    DataOutputStream dos = new DataOutputStream(bos);
    createPacketHeader(dos, CommonConstants.MAINTENANCE_COMMAND, CommonConstants.MAINTENANCE_GET_GROUP);
    dos.writeUTF(group);
    dos.close();

    return (MaintenanceGroup)processTCPPacket(bos.toByteArray(), null, MaintenanceGroup.class);
  }

  /**
   * store the maintenance group object
   */
  public boolean storeMaintenanceGroup (MaintenanceGroup mg) throws Exception {
    ByteArrayOutputStream bos = new ByteArrayOutputStream(128);
    DataOutputStream dos = new DataOutputStream(bos);
    createPacketHeader(dos, CommonConstants.MAINTENANCE_COMMAND, CommonConstants.MAINTENANCE_PUT_GROUP);
    dos.close();

    return processTCPPacket(bos.toByteArray(), mg, tp.getSetTimeout()*1000);
  }

  /**
   * delete the maintenance group
   */
  public boolean deleteMaintenanceGroup (String group) throws Exception {
    ByteArrayOutputStream bos = new ByteArrayOutputStream(128);
    DataOutputStream dos = new DataOutputStream(bos);
    createPacketHeader(dos, CommonConstants.MAINTENANCE_COMMAND, CommonConstants.MAINTENANCE_DELETE_GROUP);
    dos.writeUTF(group);
    dos.close();

    return processTCPPacket(bos.toByteArray(), tp.getSetTimeout()*1000);
  }

  /**
   * get the list of names of all the maintenance requests available
   */
  public String [] getMaintenanceRequestNames () throws Exception {
    return getNames(CommonConstants.MAINTENANCE_COMMAND, CommonConstants.MAINTENANCE_GET_REQUEST_NAMES);
  }

  /**
   * get the maintenance request object for the given name
   */
  public MaintenanceRequest getMaintenanceRequest (String request) throws Exception {
    ByteArrayOutputStream bos = new ByteArrayOutputStream(128);
    DataOutputStream dos = new DataOutputStream(bos);
    createPacketHeader(dos, CommonConstants.MAINTENANCE_COMMAND, CommonConstants.MAINTENANCE_GET_REQUEST);
    dos.writeUTF(request);
    dos.close();

    return (MaintenanceRequest)processTCPPacket(bos.toByteArray(), null, MaintenanceRequest.class);
  }

  /**
   * store the maintenance requst object
   */
  public boolean storeMaintenanceRequest (MaintenanceRequest mr) throws Exception {
    ByteArrayOutputStream bos = new ByteArrayOutputStream(128);
    DataOutputStream dos = new DataOutputStream(bos);
    createPacketHeader(dos, CommonConstants.MAINTENANCE_COMMAND, CommonConstants.MAINTENANCE_PUT_REQUEST);
    dos.close();

    return processTCPPacket(bos.toByteArray(), mr, tp.getSetTimeout()*1000);
  }

  /**
   * delete the maintenance request
   */
  public boolean deleteMaintenanceRequest (String request) throws Exception {
    ByteArrayOutputStream bos = new ByteArrayOutputStream(128);
    DataOutputStream dos = new DataOutputStream(bos);
    createPacketHeader(dos, CommonConstants.MAINTENANCE_COMMAND, CommonConstants.MAINTENANCE_DELETE_REQUEST);
    dos.writeUTF(request);
    dos.close();

    return processTCPPacket(bos.toByteArray(), tp.getSetTimeout()*1000);
  }

  /**
   * is the maintenance request currently actively being worked on?
   */
  public boolean isMaintenanceRequestActive (String request) throws Exception {
    ByteArrayOutputStream bos = new ByteArrayOutputStream(128);
    DataOutputStream dos = new DataOutputStream(bos);
    createPacketHeader(dos, CommonConstants.MAINTENANCE_COMMAND, CommonConstants.MAINTENANCE_REQUEST_ACTIVE);
    dos.writeUTF(request);
    dos.close();

    return processTCPPacket(bos.toByteArray());
  }

  /**
   * abort the currently active maintenance request
   */
  public void abortMaintenanceRequest (String request) throws Exception {
    ByteArrayOutputStream bos = new ByteArrayOutputStream(128);
    DataOutputStream dos = new DataOutputStream(bos);
    createPacketHeader(dos, CommonConstants.MAINTENANCE_COMMAND, CommonConstants.MAINTENANCE_REQUEST_ABORT);
    dos.writeUTF(request);
    dos.close();

    processTCPPacket(bos.toByteArray(), tp.getSetTimeout()*1000);
  }

  /**
   * return a File handle for the log file (jserver log file and 
   * maintenance request log file)
   */
  public File getLogFile (String name) throws Exception {
    if (name == null)
      return getLogFile(CommonConstants.JSERVER_LOGFILE, name);
    else
      return getLogFile(CommonConstants.MAINTENANCE_LOGFILE, name);
  }

  /**
   * return a File handle for the given log file type and name
   */
  private File getLogFile (int logType, String name) throws Exception {
    // get the port to connect to, to retrieve the log file
    int port = 0;
    try {
      port = getListPort(CommonConstants.LOG_FILE);
    } catch (Exception e) {
      throw new BridgeException("Error receiving log file from the iServer:\n" + e.getMessage());
    }

    // open a socket to that port
    Socket s = null;
    try {
      s = new Socket(addr, port);
      s.setSoTimeout(tp.getGetTimeout()*1000);
    } catch (Exception e) {
      throw new BridgeException("Error opening connection to retrieve the log file:\n" + e.getMessage());
    }

    File logf = null;
    // send the request and retrieve the log file data
    try {
      ObjectOutputStream oos = null;
      GZIPOutputStream gos = null;
      BufferedOutputStream bos = null;
      if (compression == 0) {
        bos = new BufferedOutputStream(s.getOutputStream());
        oos = new ObjectOutputStream(bos);
      } else {
        bos = new BufferedOutputStream(s.getOutputStream());
        gos = new GZIPOutputStream(bos);
        oos = new ObjectOutputStream(gos);
      }

      oos.writeObject(new Integer(logType));
      // if filename is null, that means we are requesting
      // the jserverlogfile
      if (name != null)
	oos.writeObject(name);
      if (gos != null)
        gos.finish();
      bos.flush();

      ObjectInputStream ois = null;
      if (compression == 0)
        ois = new ObjectInputStream(s.getInputStream());
      else
        ois = new ObjectInputStream(new GZIPInputStream(new BufferedInputStream(s.getInputStream())));

      String log = name;
      if (log == null)
	log = "jserverlogfile";
      else if (log.length() < 3)
	log = log + "---";
      logf = File.createTempFile(log, null);
      logf.deleteOnExit();
      FileOutputStream fos = new FileOutputStream(logf);

      // receive the object
      Object o = ois.readObject();
      if (o.getClass().equals(Boolean.class)) {
	if (!((Boolean)o).booleanValue())
	  throw new BridgeException("File does not exist");
      } else if (o instanceof Exception) {
	throw (Exception)o;
      } else {
	throw new BridgeException("Unrecognized reply received from the iServer");
      }
      // receive the file length
      long len = ois.readLong();
      SysUtil.copyStream(ois, fos, len);
      fos.close();
      oos.close();
      ois.close();
    } catch (Exception e) {
      try {
	s.close();
      } catch (Exception ie) {}
      e.printStackTrace();
      throw new BridgeException("Error retrieving log file from the iserver:\n" + e.getMessage());
    }

    try {
      s.close();
    } catch (Exception e) {
      throw new BridgeException("Log file retrieved successfully.\nHowever error closing connection: " + e.getMessage());
    }

    return logf;
  }

  /**
   * get the list of names of all the maintenance logs available
   */
  public String [] getMaintenanceLogNames () throws Exception {
    return getNames(CommonConstants.MAINTENANCE_COMMAND, CommonConstants.MAINTENANCE_GET_LOG_NAMES);
  }

  /**
   * delete the maintenance log file
   */
  public boolean deleteMaintenanceLog (String name) throws Exception {
    ByteArrayOutputStream bos = new ByteArrayOutputStream(128);
    DataOutputStream dos = new DataOutputStream(bos);
    createPacketHeader(dos, CommonConstants.MAINTENANCE_COMMAND, CommonConstants.MAINTENANCE_DELETE_LOG);
    dos.writeUTF(name);
    dos.close();

    return processTCPPacket(bos.toByteArray(), tp.getSetTimeout()*1000);
  }

  /**
   * get the list of name and comments of all the maintenance groups
   */
  public String [][] getMaintenanceGroupNamesAndComments () throws Exception {
    return getNamesAndComments(CommonConstants.MAINTENANCE_COMMAND, CommonConstants.MAINTENANCE_GET_GROUP_NAMES_COMMENTS);
  }

  /**
   * get the list of name and comments of all the maintenance requests
   */
  public String [][] getMaintenanceRequestNamesAndComments () throws Exception {
    return getNamesAndComments(CommonConstants.MAINTENANCE_COMMAND, CommonConstants.MAINTENANCE_GET_REQUEST_NAMES_COMMENTS);
  }

  /**
   * get the list of names of all the auto-download groups available
   */
  public String [] getAutoDownloadGroupNames () throws Exception {
    return getNames(CommonConstants.AUTO_DOWNLOAD_COMMAND, CommonConstants.AUTO_DOWNLOAD_GET_NAMES);
  }

  /**
   * get the list of name and comments of all the auto-download groups
   */
  public String [][] getAutoDownloadGroupNamesAndComments () throws Exception {
    return getNamesAndComments(CommonConstants.AUTO_DOWNLOAD_COMMAND, CommonConstants.AUTO_DOWNLOAD_GET_NAMES_COMMENTS);
  }

  /**
   * get the auto-download configuration (MaintenanceRequest object)
   * for the given regid
   */
  public MaintenanceRequest getAutoDownloadConfig (String regid, boolean isGroup) throws Exception {
    ByteArrayOutputStream bos = new ByteArrayOutputStream(128);
    DataOutputStream dos = new DataOutputStream(bos);
    createPacketHeader(dos, CommonConstants.AUTO_DOWNLOAD_COMMAND, CommonConstants.AUTO_DOWNLOAD_GET_CONFIG);
    dos.writeUTF(regid);
    if (isGroup)
      dos.writeShort(1);
    else
      dos.writeShort(0);
    dos.close();

    return (MaintenanceRequest)processTCPPacket(bos.toByteArray(), null, MaintenanceRequest.class);
  }

  /**
   * store the auto-download configuration (the regid for which this
   * config applies, is the name of the MaintenanceRequest object)
   */
  public boolean storeAutoDownloadConfig (MaintenanceRequest mr, boolean isGroup) throws Exception {
    ByteArrayOutputStream bos = new ByteArrayOutputStream(128);
    DataOutputStream dos = new DataOutputStream(bos);
    createPacketHeader(dos, CommonConstants.AUTO_DOWNLOAD_COMMAND, CommonConstants.AUTO_DOWNLOAD_PUT_CONFIG);
    if (isGroup)
      dos.writeShort(1);
    else
      dos.writeShort(0);
    dos.close();

    return processTCPPacket(bos.toByteArray(), mr, tp.getSetTimeout()*1000);
  }

  /**
   * delete the auto-download configuration
   */
  public boolean deleteAutoDownloadConfig (String regid, boolean isGroup) throws Exception {
    ByteArrayOutputStream bos = new ByteArrayOutputStream(128);
    DataOutputStream dos = new DataOutputStream(bos);
    createPacketHeader(dos, CommonConstants.AUTO_DOWNLOAD_COMMAND, CommonConstants.AUTO_DOWNLOAD_DELETE);
    dos.writeUTF(regid);
    if (isGroup)
      dos.writeShort(1);
    else
      dos.writeShort(0);
    dos.close();

    return processTCPPacket(bos.toByteArray(), tp.getSetTimeout()*1000);
  }

  /**
   * is the auto-download session for this regid currently active?
   */
  public boolean isAutoDownloadActive (String regid) throws Exception {
    ByteArrayOutputStream bos = new ByteArrayOutputStream(128);
    DataOutputStream dos = new DataOutputStream(bos);
    createPacketHeader(dos, CommonConstants.AUTO_DOWNLOAD_COMMAND, CommonConstants.AUTO_DOWNLOAD_ACTIVE);
    dos.writeUTF(regid);
    dos.close();

    return processTCPPacket(bos.toByteArray());
  }

  /**
   * abort the active auto-download session of this regid
   */
  public void abortAutoDownload (String regid) throws Exception {
    ByteArrayOutputStream bos = new ByteArrayOutputStream(128);
    DataOutputStream dos = new DataOutputStream(bos);
    createPacketHeader(dos, CommonConstants.AUTO_DOWNLOAD_COMMAND, CommonConstants.AUTO_DOWNLOAD_ABORT);
    dos.writeUTF(regid);
    dos.close();

    processTCPPacket(bos.toByteArray(), tp.getSetTimeout()*1000);
  }

  /**
   * return a File handle for the auto-download log file for this regid
   */
  public File getAutoDownloadLogFile (String regid) throws Exception {
    return getLogFile(CommonConstants.AUTODOWNLOAD_LOGFILE, regid);
  }

  /**
   * delete the auto-download log file
   */
  public boolean deleteAutoDownloadLogFile (String regid) throws Exception {
    ByteArrayOutputStream bos = new ByteArrayOutputStream(128);
    DataOutputStream dos = new DataOutputStream(bos);
    createPacketHeader(dos, CommonConstants.AUTO_DOWNLOAD_COMMAND, CommonConstants.AUTO_DOWNLOAD_DELETE_LOG);
    dos.writeUTF(regid);
    dos.close();

    return processTCPPacket(bos.toByteArray(), tp.getSetTimeout()*1000);
  }

  /**
   * lookup the iserver for the given phone number and return the
   * corresponding IEdgeList object
   */
  public IEdgeList getIedgeParamsForPhone (String phone) throws Exception {
    ByteArrayOutputStream bos = new ByteArrayOutputStream(256);
    DataOutputStream dos = new DataOutputStream(bos);
    createPacketHeader(dos, CommonConstants.LOOKUP_COMMAND, CommonConstants.LOOKUP_BY_PHONE);
    dos.writeUTF(phone);
    dos.close();

    return (IEdgeList)processUDPPacket(bos.toByteArray(), IEdgeList.class);		 
  }

  /**
   * lookup the iserver for the given vpn phone number and return the
   * corresponding IEdgeList object
   */
  public IEdgeList getIedgeParamsForVpnPhone (String phone, int extLen) throws Exception {
    ByteArrayOutputStream bos = new ByteArrayOutputStream(256);
    DataOutputStream dos = new DataOutputStream(bos);
    createPacketHeader(dos, CommonConstants.LOOKUP_COMMAND, CommonConstants.LOOKUP_BY_VPNPHONE);
    dos.writeUTF(phone);
    dos.writeInt(extLen);
    dos.close();

    return (IEdgeList)processUDPPacket(bos.toByteArray(), IEdgeList.class);		 
  }

  /**
   * send (telnet) to the iserver the two numbers associated with
   * a butterfly call, and wait for the reply
   */
  public String initiateButterfly (String src, String dst) throws Exception {
    Socket sock = new Socket(addr, CommonConstants.ISERVER_BUTTERFLY_PORT);
    sock.setSoTimeout(tp.getGetTimeout()*1000);
    DataOutputStream dos = new DataOutputStream(sock.getOutputStream());
    dos.writeBytes(src + " " + dst + "\n");
    DataInputStream dis = new DataInputStream(sock.getInputStream());
    //		 String result = dis.readUTF();
    String result = "Initiated the call";
    dos.close();
    dis.close();
    sock.close();

    return result;
  }

  /** get the iserver configuration */
  public iServerConfig getIserverConfig () throws Exception {
    ByteArrayOutputStream bos = new ByteArrayOutputStream(128);
    DataOutputStream dos = new DataOutputStream(bos);
    createPacketHeader(dos, CommonConstants.CFG_COMMAND, CommonConstants.ISERVER_GET_CFG);
    dos.close();

    String xmlString = (String)processTCPPacket(bos.toByteArray(), null, String.class);
    //		System.out.println(xmlString);

    return new iServerConfig(xmlString);



  }
  /** set/update the iserver configuration */
  public boolean setIserverConfig (iServerConfig cfg) throws Exception {
    ByteArrayOutputStream bos = new ByteArrayOutputStream(128);
    DataOutputStream dos = new DataOutputStream(bos);
    createPacketHeader(dos, CommonConstants.CFG_COMMAND, CommonConstants.ISERVER_SET_CFG);
    dos.close();
    return (processTCPPacket(bos.toByteArray(), cfg.getXMLString(), tp.getSetTimeout()*2000));
  }


  /** import iserver database from the given file*/
  public boolean importDB (boolean isLocal, int cmd, String dbName, File db) throws Exception{
    ByteArrayOutputStream bos = new ByteArrayOutputStream(128);
    DataOutputStream dos = new DataOutputStream(bos);
    createPacketHeader(dos, CommonConstants.DB_COMMAND, CommonConstants.ISERVER_DB_IMPORT);
    dos.writeBoolean(isLocal);
    dos.writeInt(cmd);    
    dos.writeUTF(dbName);
    dos.close();

    // timeout is zero because we have no idea how long to wait for large databases
    return processTCPPacket(bos.toByteArray(), isLocal?db:null, 0);
  }

  /** export iserver database */
  public File exportDB (boolean isLocal,String dbName,int listType,Socket sock) throws Exception{
    ByteArrayOutputStream bos = new ByteArrayOutputStream(128);
    DataOutputStream dos = new DataOutputStream(bos);
    createPacketHeader(dos, CommonConstants.DB_COMMAND, CommonConstants.ISERVER_DB_EXPORT);
    dos.writeBoolean(isLocal);
    dos.writeUTF(dbName);
    dos.writeInt(listType);
    dos.close();

    // timeout is zero because we have no idea how long to wait for large databases
    return (File)processTCPPacket(bos.toByteArray(), null, File.class, 0,sock);
  }

  public String[] getDBFilenames () throws Exception{
    ByteArrayOutputStream bos = new ByteArrayOutputStream(128);
    DataOutputStream dos = new DataOutputStream(bos);
    createPacketHeader(dos, CommonConstants.DB_COMMAND, CommonConstants.ISERVER_DB_GET_FILENAMES);
    dos.close();
    String[]  dbFilenames = (String[])processUDPPacket(bos.toByteArray(), String[].class);
    return dbFilenames;

  }

  public int [][] getLicenseAlarms () throws Exception {
    Class c = (new int [0][0]).getClass();
    boolean da = true;
    try {
      // detect if the msw will send mr call alarms
      getMaxMRCalls();
    } catch (OperationUnknownBridgeException oe) {
      c = (new int [0]).getClass();
      da = false;
    }
    ByteArrayOutputStream bos = new ByteArrayOutputStream(128);
    DataOutputStream dos = new DataOutputStream(bos);
    createPacketHeader(dos, CommonConstants.LS_ALARM_COMMAND,CommonConstants.LS_ALARM_STATUS);
    dos.close();

    Object o = processUDPPacket(bos.toByteArray(), c);
    if (da)
      return (int [][])o;

    // old msws send only one array
    int [][] result = new int [2][0];
    result[0] = (int [])o;
    result[1] = null;
    return result;
  }

  public boolean clearLicenseAlarms() throws Exception{
    ByteArrayOutputStream bos = new ByteArrayOutputStream(128);
    DataOutputStream dos = new DataOutputStream(bos);
    createPacketHeader(dos, CommonConstants.LS_ALARM_COMMAND,CommonConstants.LS_ALARM_CLEAR);
    dos.close();
    return processUDPPacket(bos.toByteArray());
  }

  public int getMaximumRecords(int listType) throws Exception{
    ByteArrayOutputStream bos = new ByteArrayOutputStream(512);
    DataOutputStream dos = new DataOutputStream(bos);
    createPacketHeader(dos, CommonConstants.DB_COMMAND, CommonConstants.MAX_RECORDS);
    dos.writeInt(listType);
    dos.close();
    Integer in = (Integer)processUDPPacket(bos.toByteArray(),Integer.class);
    return (in == null)?-1:in.intValue();
  }

  public int getMaxCalls() throws Exception{
    ByteArrayOutputStream bos = new ByteArrayOutputStream(512);
    DataOutputStream dos = new DataOutputStream(bos);
    createPacketHeader(dos, CommonConstants.MISC_COMMAND, CommonConstants.ISERVER_MAX_CALLS);
    dos.close();

    Integer in = (Integer)processUDPPacket(bos.toByteArray(),Integer.class);
    return (in == null)?-1:in.intValue();
  }

  public int getMaxMRCalls() throws Exception{
    ByteArrayOutputStream bos = new ByteArrayOutputStream(512);
    DataOutputStream dos = new DataOutputStream(bos);
    createPacketHeader(dos, CommonConstants.MISC_COMMAND, CommonConstants.ISERVER_MAX_MR_CALLS);
    dos.close();

    Integer in = (Integer)processUDPPacket(bos.toByteArray(),Integer.class);
    return (in == null)?-1:in.intValue();
  }

  public synchronized boolean isDbOperationInProgress () {
    return dbOp;
  }

  public synchronized void setDbOperationInProgress (boolean newval) {
    dbOp = newval;
  }

  public boolean clearLog (int logFileType) throws Exception {
    ByteArrayOutputStream bos = new ByteArrayOutputStream(256);
    DataOutputStream dos = new DataOutputStream(bos);
    createPacketHeader(dos, CommonConstants.MISC_COMMAND, CommonConstants.CLEAR_LOG_FILE);
    dos.writeInt(logFileType);
    dos.close();

    return processUDPPacket(bos.toByteArray());
  }


  public boolean processCommands(Commands cmd) throws Exception{
    ByteArrayOutputStream bos = new ByteArrayOutputStream(256);
    DataOutputStream dos = new DataOutputStream(bos);
    createPacketHeader(dos, CommonConstants.MISC_COMMAND, CommonConstants.PROCESS_COMMAND);
    ObjectOutputStream oos = new ObjectOutputStream(dos);
    oos.writeObject(cmd);
    dos.close();
    return processUDPPacket(bos.toByteArray());
  }

  public boolean processBulkCommands(Commands cmds[]) throws Exception{
    ByteArrayOutputStream bos = new ByteArrayOutputStream(256);
    DataOutputStream dos = new DataOutputStream(bos);
    createPacketHeader(dos, CommonConstants.PROCESS_BULK_COMMANDS);
    ObjectOutputStream oos = new ObjectOutputStream(dos);
    oos.writeObject(cmds);
    dos.close();
    return processTCPPacket(bos.toByteArray(),0);
  }

  // get iServer capabilities
  public Capabilities getCapabilities() throws Exception{
    ByteArrayOutputStream bos = new ByteArrayOutputStream(128);
    DataOutputStream dos = new DataOutputStream(bos);
    createPacketHeader(dos, CommonConstants.CAP_COMMAND, CommonConstants.ISERVER_GET_CAP);
    dos.close();

    String xmlString = (String)processTCPPacket(bos.toByteArray(), null, String.class);
    return new Capabilities(xmlString);
  }

}

