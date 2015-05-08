package com.nextone.common;

import java.io.*;
import java.net.*;
import java.util.Date;
import com.nextone.util.CloneableDataInputStream;
import com.nextone.util.LimitedDataInputStream;
import com.nextone.util.LimitExceededException;
import com.nextone.util.IPUtil;
import com.nextone.util.Logger;
import com.nextone.util.NextoneMulticastSocket;
import com.nextone.util.QueueProcessor;
import com.nextone.util.UDPServer;
import com.nextone.JFac.JFac;
import com.nextone.JUCon.JUCon;
import com.nextone.JUCon.iEdge500.I500;
import com.nextone.JUCon.iEdge1000.I1000;
import com.nextone.JUCon.PingTel.PingTel;
import com.nextone.JUCon.VegaStream.VegaStream;
import com.nextone.JUCon.iServer.ClusteredIServer;
import com.nextone.JUCon.iServer.IServer;

public class ReceiveReg extends UDPServer implements CommonConstants {
  private boolean keepRunning = true;
  private NextoneMulticastSocket mcastSock;
  private NexToneFrame nf;
  private boolean isJUCon = false;
  private boolean isJFac = false;
  private InetAddress [] allLocalAddr;
  private QueueProcessor qp;
  private ThreadGroup tg;

  /**
   * Constructor
   * @param tg the threadgroup to belong to.
   * @param s  the NextoneMulticast socket to use.
   * @param nf the parent NextoneFrame that provides the 
   *           application specific (JUCon/JFac) info.
   */
  public ReceiveReg (ThreadGroup tg, NextoneMulticastSocket s, NexToneFrame nf) throws IOException {
    super(tg, s, Thread.MAX_PRIORITY, "ReceiveReg");
    this.tg = tg;
    this.mcastSock = s;
    this.nf = nf;
    keepRunning = true;
    if (nf.getID().startsWith(JUCon.id))
      isJUCon = true;
    else if (nf.getID().startsWith(JFac.id))
      isJFac = true;
    InetAddress [] all = InetAddress.getAllByName(InetAddress.getLocalHost().getHostName());
    allLocalAddr = new InetAddress [all.length+1];
    System.arraycopy(all, 0, allLocalAddr, 1, all.length);
    allLocalAddr[0] = InetAddress.getByName("127.0.0.1");
    startReceiveRegQueueProcessor();
    start();
  }

  /**
   *
   */
  public void startReceiveRegQueueProcessor () {
    if (qp != null) {
      qp.stopRunning();
      nf.setStatus("Restarting Registration thread\n");
      qp.clearQueue();
    }
    qp = QueueProcessor.getInstance(new HandleRegistration(), tg, CommonConstants.receiveRegQPName);
    qp.setLimit(CommonConstants.MAX_REGISTRATIONS);
    qp.start();
  }

  /**
   * Stop the receive thread.
   */
  public void stop () {
    keepRunning = false;

    try {
      ByteArrayOutputStream bos = new ByteArrayOutputStream(2);
      DataOutputStream dos = new DataOutputStream(bos);
      dos.writeShort(STOP);

      //			InetAddress toIp = InetAddress.getByName("localhost");
      InetAddress toIp = InetAddress.getByName("127.0.0.1");
      DatagramPacket dp = new DatagramPacket(bos.toByteArray(), bos.size(), toIp, CommonConstants.MCAST_LISTEN_PORT);
      mcastSock.send(dp);
    } catch (IOException ie) {
      nf.setStatus("RR Stop error:\n" + ie.toString() + "\n");
    }
  }

  /**
   *
   */
  private InetAddress toOurIP (DataInputStream dis) throws IOException {
    int [] ip = new int [4];
    for (int i = 0; i < 4; i++)
      ip[i] = dis.readUnsignedByte();
    InetAddress inIp = InetAddress.getByName(IPUtil.intArrayToIPString(ip));
    for (int i = 0; i < allLocalAddr.length; i++) {
      if (allLocalAddr[i].equals(inIp))
	return inIp;
    }

    return null;
  }

  /**
   *
   */
  private void handleIdRequest (DatagramPacket dp, DataInputStream dis) {
    try {
      InetAddress intf = toOurIP(dis);
      if (intf == null) {
	//			   System.out.println("not to our IP");
	return;
      }

      int id = ((JFac)nf).getUID().getNewId(intf);
      if (id == 0)
	return;  // database connection hasn't been made

      // now send it out
      ByteArrayOutputStream bos = new ByteArrayOutputStream(49);
      DataOutputStream dos = new DataOutputStream(bos);
      dos.writeShort(CommonConstants.UNIQUE_ID);
      JFac.fillPermissionData(dos);
      dos.writeInt(id);
      dos.writeByte(0);
      mcastSock.send(new DatagramPacket(bos.toByteArray(), bos.size(),
					InetAddress.getByName(CommonConstants.MCAST_ADDRESS),
					CommonConstants.MCAST_SEND_PORT), intf);
    } catch (UnknownHostException ue) {
      nf.setStatus("Error recognizing IP addres for ID request:\n  " + ue.toString() + "\n");
    } catch (IOException ie) {
      nf.setStatus("Error sending new id:\n  " + ie.toString() + "\n");
    }
  }

  /**
   *
   */
  private void handleSerialNoRequest (DatagramPacket dp, DataInputStream dis) {
    try {
      int i = dis.readInt();
      InetAddress intf = toOurIP(dis);
      if (intf == null) {
	//			   System.out.println("not to our IP");
	return;
      }

      ((JFac)nf).getUID().sendSerialNum(i, intf);
    } catch (IOException ie) {
      nf.setStatus("Error sending serial no:\n  " + ie.toString() + "\n");
    }
  }

  /**
   *
   */
  public int getNumInQueue () {
    return qp.getNumInQueue();
  }

  /**
   *
   */
  public void clearQueue () {
    qp.clearQueue();
  }

  /**
   *
   */
  private class RegData {
    DatagramPacket dp;
    DataInputStream dis;
    long receivedTime;

    /**
     *
     */
    RegData (DatagramPacket dp, DataInputStream dis) {
      this.dp = dp;
      this.dis = dis;
      this.receivedTime = System.currentTimeMillis();
    }

    /**
     *
     */
    public String toString () {
      return new Date(receivedTime) + ": " + dp.getAddress().getHostAddress();
    }

  }

  private class HandleRegistration implements QueueProcessor.DataProcessor {
    private int errorCount = 0;
    // System.out.print() in this method seems to slow down the
    // entire thread!
    public void processData (Object data) {
      RegData rd = (RegData)data;
      long timePassed = System.currentTimeMillis() - rd.receivedTime;
      if (timePassed > (nf.getPollInterval()*1000)) {
	        nf.appendDebug("Registration was " + (timePassed/1000) + " seconds ago, no longer useful");
          if (isJUCon && ++errorCount > 10){
            ((JUCon)nf).setStatus("Registrations messages are being dropped, please check your DNS settings\nSome devices on the map may appear to be not responding\n");
            Logger.error("Registrations messages are being dropped, please check your DNS settings\nSome devices on the map may appear to be not responding\n");
	          return;
          }
      }
      handleRegistration(rd.dp, rd.dis);
      try {
	      rd.dis.close();
      } catch (IOException ie) {}
    }
  }

  private void handleRegistration (DatagramPacket dp, DataInputStream ds) {
    LimitedDataInputStream dis = new LimitedDataInputStream(ds, dp.getLength()-2);
    //		 System.out.println("From " + dp.getAddress().getHostAddress() + " packet size: " + dp.getLength());
    try {
      short mode = dis.readShort();
      short deviceid = dis.readShort();

      switch (deviceid) {
      case CommonConstants.DEVICE_ID_500:
      case CommonConstants.DEVICE_ID_510:
	      handleI500Registration(mode, deviceid, dis, dp);
	      break;
      case CommonConstants.DEVICE_ID_ISERVER:
	      handleIServerRegistration(mode, deviceid, dis, dp);
	      break;
      case CommonConstants.DEVICE_ID_1000:
	      handleI1000Registration(mode, deviceid, dis, dp);
	      break;
      case CommonConstants.DEVICE_ID_PINGTEL:
	      handlePingTelRegistration(mode, deviceid, dis, dp);
	      break;
      }

    } catch (IOException ie) {
      nf.appendDebug("error parsing registration from " + dp.getAddress().toString() + "\n");
      nf.appendDebug(ie.toString() + "\n");
      ie.printStackTrace();
      return;
    }
  }

  /**
   * Handle the received VegaStream registration.
   */
  public void handleVegaStreamRegistration (InetAddress addr) throws IOException {

    VegaStream i =  new VegaStream(addr);

    JUCon pc = (JUCon)nf;
    pc.appendDebug(new Date().toString() + ": ReceiveReg: registration from(VegaStream): " + addr.getHostAddress() + ":" + 23);

    if (pc.isKnownAsRemoteThirdPartyDevice(addr)) {
      // it is already on the screen, just need to update
      pc.updateVegaStream(addr);
    } else {
      pc.addVegaStream(i);
    }
  }

  /**
   * Handle the received PingTel registration.
   */
  private void handlePingTelRegistration (short mode, short deviceid,
					  LimitedDataInputStream dis,
					  DatagramPacket dp) throws IOException {

    Object o = processPingTelRegistration(mode, deviceid, dis, dp, nf);

    JUCon pc = (JUCon)nf;
    InetAddress fromAddr = dp.getAddress();
    pc.appendDebug(new Date().toString() + ": ReceiveReg: registration from(PingTel): " + fromAddr.getHostAddress() + ":" + dp.getPort());

    PingTel i = (PingTel)o;

    if (pc.isKnownAsRemoteEdgeDevice(fromAddr)) {
				// it is already on the screen, just need to update
      pc.updatePingTel(fromAddr, mode, deviceid, i.getSerialNumber(),
		       i.getFqdn(), i.hasSpecialAddress(), null,
		       i.getRegId());
    } else {
      pc.addPingTel(i);
    }
  }

  /**
   * Parse the PingTel registration
   */
  public static PingTel processPingTelRegistration (short mode, short deviceid,
						    LimitedDataInputStream dis,
						    DatagramPacket dp,
						    NexToneFrame ntf) throws IOException {
    // serial number
    String sno = dis.readUTF();

    // fqdn
    String fqdn = dis.readUTF();

    // ip address mode
    short ipmode = dis.readShort();
    boolean isSpecialMode = (ipmode == 1)?true:false;

    // an iedge can give itself a name here... currently not used
    String name = dis.readUTF();

    // version numbering is introduced in release 1.1. Not needed for this release.
    dis.readShort();

    // registration id
    String reg = dis.readUTF();

    PingTel newPingTel =  new PingTel(mode, deviceid, sno, fqdn, dp.getAddress(), isSpecialMode, name, reg);

    return newPingTel;
  }

  /**
   * Handle the received I1000 registration.
   */
  private void handleI1000Registration (short mode, short deviceid,
					LimitedDataInputStream dis,
					DatagramPacket dp) throws IOException {

    Object o = processI1000Registration(mode, deviceid, dis, dp, nf);

    JUCon pc = (JUCon)nf;
    InetAddress fromAddr = dp.getAddress();
    pc.appendDebug(new Date().toString() + ": ReceiveReg: registration from(I1000): " + fromAddr.getHostAddress() + ":" + dp.getPort());

    I1000 i = (I1000)o;

    if (pc.isKnownAsRemoteEdgeDevice(fromAddr)) {
				// it is already on the screen, just need to update
      pc.updateI1000(fromAddr, mode, deviceid, i.getSerialNumber(),
		     i.getFqdn(), i.hasSpecialAddress(), null,
		     i.getRegId(),i.getT1Status());
    } else if (pc.isKnownAsRegRemoteEdgeDevice(i.getRegId())) {
      // it is already on the screen, just need to update
      pc.updateI1000(fromAddr, mode, deviceid, i.getSerialNumber(),
		     i.getFqdn(), i.hasSpecialAddress(), null,
		     i.getRegId(),i.getT1Status());
    }

    else {
      pc.addI1000(i);
    }
  }

  /**
   * Parse the i1000 registration
   */
  public static I1000 processI1000Registration (short mode, short deviceid,
						LimitedDataInputStream dis,
						DatagramPacket dp,
						NexToneFrame ntf) throws IOException {
    // serial number
    String sno = dis.readUTF();

    // fqdn
    String fqdn = dis.readUTF();

    // ip address mode
    short ipmode = dis.readShort();
    boolean isSpecialMode = (ipmode == 1)?true:false;

    // an iedge can give itself a name here... currently not used
    String name = dis.readUTF();

    // version numbering is introduced in release 1.1. Not needed for this release.
    dis.readShort();

    // registration id
    String reg = dis.readUTF();

    // The following parameters were added during 1.2d7 release
    // to add the T1 status for the 1000.
    short t1Status;
    String version;
    try {
      t1Status = dis.readShort();
      // ram version number was added for 1.3 release
      version = dis.readUTF();
    } catch(EOFException eofe) {
      // For older version display without T1 status.
      t1Status = I1000.T1_NOSTATUS;
      version	=	"";
    }

    I1000 new1000 =  new I1000(mode, deviceid, sno, fqdn, dp.getAddress(), isSpecialMode, name, reg);
    new1000.setT1Status(t1Status);

    return new1000;
  }

  /**
   * Handle the recived I500 registration.
   */
  private void handleI500Registration (short mode, short deviceid,
				       LimitedDataInputStream dis,
				       DatagramPacket dp) throws IOException {
    Object o = processI500Registration(mode, deviceid, dis, dp, nf);
    if (o == null)
      return;

    // JFac specific code
    if (isJFac) {
      ((JFac)nf).getUID().receivedRegistration((String)o, mode, dp.getAddress());
      return;
    }

    // JUCon specific code
    JUCon pc = (JUCon)nf;
    InetAddress fromAddr = dp.getAddress();
    I500 i = (I500)o;
    pc.appendDebug(new Date().toString() + ": ReceiveReg: registration from(I500): " + fromAddr.getHostAddress() + ":" + dp.getPort());
    if (pc.isKnownAsRemoteEdgeDevice(fromAddr)) {
      // it is already on the screen, just need to update
      pc.updateI500(fromAddr, mode, deviceid, i.getSerialNumber(),
		    i.getFqdn(), i.hasSpecialAddress(), null,
		    i.getRegId());
    }

    else if (pc.isKnownAsRegRemoteEdgeDevice(i.getRegId())) {
      // it is already on the screen, just need to update
      pc.updateI500(fromAddr, mode, deviceid, i.getSerialNumber(),
		    i.getFqdn(), i.hasSpecialAddress(), null,
		    i.getRegId());
    }

    else {
      pc.addI500(i);
    }
  }

  // process the registration packet
  // returns   - null, if no further processing needed
  //           - I500, if further processing needed for jucon
  //           - String, if further processing needed for jfac
  public static Object processI500Registration (short mode, short deviceid,
						LimitedDataInputStream dis,
						DatagramPacket dp,
						NexToneFrame ntf) throws IOException {
    // serial number
    short snoSize = dis.readShort();
    if (snoSize > 68 || snoSize == 0) {
      ntf.appendDebug("received invalid serial number length (" + snoSize + ") from " + dp.getAddress().getHostAddress() + "\n");
      return null;
    }
    byte [] sno = new byte [snoSize];
    dis.readFully(sno, 0, snoSize);

    // for iFac we are done right here...
    if (ntf.getID().startsWith(JFac.id)) {
      return new String(sno);
    }

    // fqdn
    short fqdnSize = dis.readShort();
    byte [] fqdn = new byte [fqdnSize];
    dis.readFully(fqdn, 0, fqdnSize);

    // ip address mode
    short ipmode = dis.readShort();
    boolean isSpecialMode = (ipmode == 1)?true:false;

    // an iedge can give itself a name here... currently not used
    short nameSize = dis.readShort();
    byte [] name = new byte [nameSize];
    dis.readFully(name, 0, nameSize);

    // version numbering is introduced in release 1.1
    short helloVersion = 0;
    // read the version number of the registration packet
    try {
      helloVersion = dis.readShort();
    } catch (EOFException ee) {
      // pre 1.1 devices won't have this version number
      helloVersion = 1;
    }

    byte [] reg = null;
    switch (helloVersion) {
    case 1:
      // substitute regid with sno for pre 1.1 devices
      reg = new byte [sno.length];
      System.arraycopy(sno, 0, reg, 0, sno.length);
      break;

    case 2:
      // registration id
      short regsize = dis.readShort();
      reg = new byte [regsize];
      dis.readFully(reg, 0, regsize);
      break;

    default:
      ntf.appendDebug("Invalid hello version (" + helloVersion + ") from " + dp.getAddress().getHostAddress() + "\n");
      return null;
    }

    String fqdnString = null;
    if (fqdn != null && fqdn.length > 0)
      fqdnString = new String(fqdn);
    String nameString = null;
    if (name != null && name.length > 0)
      nameString = new String(name);
    String regId = null;
    if (reg != null && reg.length > 0)
      regId = new String(reg);

    return new I500(mode, deviceid, new String(sno), fqdnString, dp.getAddress(), isSpecialMode, nameString, regId);
  }

  public static IServer processIServerRegistration (short mode, short deviceid, LimitedDataInputStream dis, DatagramPacket dp, NexToneFrame ntf) throws IOException {
    // fqdn
    String fqdnString = dis.readUTF();
    if (fqdnString == null) {
      fqdnString = "";
      Logger.debug("FQDN from " + dp.getAddress() + " is null");
    }

    // version number of the registration packet
    short helloVersion = 0;
    try {
      helloVersion = dis.readShort();
    } catch (EOFException ee) {
      helloVersion = 1;
    }

    switch (helloVersion) {
    case 1:
    case 2:
      break;
    default:
      ntf.appendDebug("Invalid hello version (" + helloVersion + ") from " + dp.getAddress().getHostAddress() + "\n");
      return null;
    }
    
    int calls = 0;
    int maxCalls = 0;
    int status = 0;

    try{
      calls    = dis.readInt();
      maxCalls = dis.readInt();
      status   = dis.readInt();
    }catch(Exception e){
      Logger.error("unable to get call information", e);
    }

    IServer is = null;
    try {
      RedundInfo ri = new RedundInfo(dis.readUTF());
      is = new ClusteredIServer(dp.getAddress(), fqdnString, calls, maxCalls, status, ri);
    } catch (Exception e) {
      Logger.error("error creating clustered iserver", e);
      is = new IServer(dp.getAddress(), fqdnString, calls, maxCalls, status);
    }

    return is;
  }

  private void handleIServerRegistration (short mode, short deviceid,
					  LimitedDataInputStream dis,
					  DatagramPacket dp) throws IOException {
    if (!isJUCon)
      return;    // JFac doesn't have to handle this

    IServer is = processIServerRegistration(mode, deviceid, dis, dp, nf);
    if (is == null)
      return;

    JUCon pc = (JUCon)nf;
    InetAddress fromAddr = dp.getAddress();
    pc.appendDebug(new Date().toString() + ": ReceiveReg: registration from(IServer): " + fromAddr.getHostAddress() + ":" + dp.getPort());
    pc.announceIServerRegistration(fromAddr, is);
  }

  public void UDPServerWork (DatagramSocket socket) {

    keepRunning = true;
    while (keepRunning) {
      try {
	byte [] in = new byte [1000];
	DatagramPacket dp = new DatagramPacket(in, in.length);
	socket.receive(dp);
	nf.appendDebug(new Date().toString() + ": ReceiveReg: Packet received ("+dp.getAddress().toString() +")");

	CloneableDataInputStream dis = new CloneableDataInputStream(new ByteArrayInputStream(in));
	short code = dis.readShort();
	switch (code) {
	case STOP:    // to stop this thread 
	  qp.stopRunning();
	  break;

	case REGISTRATION:
	  try {
	    if (qp.isAlive())
	      qp.add(new RegData(dp, (DataInputStream)dis.clone()));
	  } catch (LimitExceededException slee) {
	    nf.appendDebug(new Date() + ": more registrations than I can handle: " + slee.getMessage());
	  } catch (CloneNotSupportedException cnse) {
	    nf.appendDebug("Clone not supported, handling inline");
	    handleRegistration(dp, dis);
	  }
	  break;

	case COMMAND_REPLY:
	  // shouldn't really get a command reply on this port
	  nf.appendDebug(new Date().toString() + ": command reply from " + dp.getAddress().toString() + "\n");
	  break;

	case NEED_UNIQUE_ID:
	  if (isJFac)
	    handleIdRequest(dp, dis);
	  break;

	case NEED_SERIAL_NUM:
	  if (isJFac)
	    handleSerialNoRequest(dp, dis);
	  break;

	  // hearing other NexToneFrames on the net...
	case HELLO:
	case COMMAND:
	case UNIQUE_ID:
	case RESET_UNIQUE_ID:
	  break;
					
	default:
	  nf.appendDebug(new Date().toString() + ": unknown response code " + code + " from " + dp.getAddress().toString() + "\n");
	}
	dis.close();
      } catch (IOException ie) {
	      nf.appendDebug("RR receive error:\n" + ie.toString() + "\n");

      }
    }

    try {
      mcastSock.leaveGroupOnAllIf(InetAddress.getByName(CommonConstants.MCAST_ADDRESS));
    } catch (IOException ie) {
      System.err.println("ReceiveReg: error leaving group:" + ie.toString());
    }
    nf.appendDebug(new Date().toString() + ": ReceiveReg exited");
  }

  public Thread getThread () {
    return super.getThread();
  }

}

