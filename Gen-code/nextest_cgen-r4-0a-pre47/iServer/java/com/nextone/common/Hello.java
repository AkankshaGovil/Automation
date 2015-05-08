package com.nextone.common;

import java.io.*;
import java.net.*;
import java.util.*;
import com.nextone.util.LimitedDataInputStream;
import com.nextone.util.NextoneMulticastSocket;

/**
 * some utility methods to be used in the hello protocol between the iEdge
 * and iView/iServer
 */
public class Hello {

  private Hello () {};
	  
  /**
   * fills in the permission string
   * after writing the string, it adds a pad so that 20 bytes would
   * be written to the underlying stream
   */
  public static void fillPermission (DataOutputStream ds, String s) throws IOException {
    int bytesWritten = 0;

    if (s != null) {
      if (s.length() > 18)
	throw new IOException("permission string length too long (" + s.length() + ")");
      ds.writeUTF(s);
      bytesWritten = 2 + s.length();
    }

    for (int i = bytesWritten; i < 20; i++)
      ds.writeByte((byte)0);
    ds.flush();
  }

  /**
   * creates the packet content for the hello message sent from a
   * device identifying itself as a remote device
   */
  public static byte [] createRemoteHelloData (String read, String write, short protocolVersion) throws IOException {
    return createHelloData(read, write, protocolVersion, CommonConstants.REMOTE);
  }

  /**
   * creates the packet content for the hello message sent from a
   * device identifying itself as a local device (on the same LAN)
   */
  public static byte [] createHelloData (String read, String write, short protocolVersion) throws IOException {
    return createHelloData(read, write, protocolVersion, (short)0);
  }

  private static byte [] createHelloData (String read, String write, short protocolVersion, short deviceType) throws IOException {
    ByteArrayOutputStream rbos = new ByteArrayOutputStream(47);
    DataOutputStream rdos = new DataOutputStream(rbos);
    rdos.writeShort(CommonConstants.HELLO);
    rdos.writeShort(protocolVersion);
    Hello.fillPermission(rdos, read);
    Hello.fillPermission(rdos, write);
    rdos.writeShort(deviceType);
    rdos.writeShort(0);
    rdos.writeByte(0);
    return rbos.toByteArray();
  }

  /**
   * get the registration message from an edge device using
   * NORMAL protocol version
   */
  public static Registration getRegistration (InetAddress addr, String read, String write, TimeoutProvider tp) throws IOException {
    return getRegistration(addr, read, write, CommonConstants.HELLO_VERSION_NORMAL, tp);
  }

  /**
   * get the registration message from an edge device using 
   * NO_AUTH protocol version
   */
  public static Registration getRegistration (InetAddress addr, TimeoutProvider tp) throws IOException {
    return getRegistration(addr, null, null, CommonConstants.HELLO_VERSION_NO_AUTH, tp);
  }

  private static Registration getRegistration (InetAddress addr, String read, String write, short protVersion, TimeoutProvider tp) throws IOException {
    byte [] d = Hello.createRemoteHelloData(read, write, protVersion);
    DatagramPacket dpo = new DatagramPacket(d, d.length, addr, CommonConstants.MCAST_SEND_PORT);

    byte [] in = new byte [512];
    DatagramPacket dpi = new DatagramPacket(in, in.length);

    DatagramSocket ds = new DatagramSocket();
    ds.setSoTimeout(tp.getGetTimeout()*1000);
    ds.send(dpo);
    ds.receive(dpi);
    return new Registration(dpi);
  }

  /**
   * sends a reboot command to the iedge
   */
  public static void reboot (InetAddress addr, int code) throws IOException {
    try {
      ByteArrayOutputStream bos = new ByteArrayOutputStream(47);
      DataOutputStream dos = new DataOutputStream(bos);
      dos.writeShort(CommonConstants.COMMAND);
      dos.writeShort(CommonConstants.HELLO_VERSION_NO_AUTH);
      Hello.fillPermission(dos, null);
      Hello.fillPermission(dos, null);
      dos.writeShort(CommonConstants.REBOOT_COMMAND);
      dos.writeShort(code);
      dos.writeByte(0);
      dos.close();

      DatagramPacket dpo = new DatagramPacket(bos.toByteArray(),
					      bos.size(),
					      addr,
					      CommonConstants.MCAST_SEND_PORT);
      DatagramSocket ds = new DatagramSocket();
      ds.send(dpo);
      ds.close();
    } catch (InterruptedIOException ie) {
      throw new IOException("No response from the iEdge");
    }
  }

  /**
   * sends a reboot command to the iedge1000
   */
  public static void rebootI1000(InetAddress addr, int mode,String sno) throws IOException {
    try {
      ByteArrayOutputStream bos = new ByteArrayOutputStream(47);
      DataOutputStream dos = new DataOutputStream(bos);

      dos.writeShort(CommonConstants.COMMAND);
      dos.writeShort(CommonConstants.REBOOT_OS_COMMAND);
      dos.writeShort(sno.length());
      dos.writeBytes(sno);
      dos.writeShort(mode);
      dos.writeByte(0);

      DatagramPacket dpo = new DatagramPacket(bos.toByteArray(),
					      bos.size(),
					      addr,
					      CommonConstants.MCAST_SEND_PORT);
      DatagramSocket ds = new DatagramSocket();
      ds.send(dpo);
      ds.close();
      System.out.println("succesfully send the packet");
    } catch (InterruptedIOException ie) {
      throw new IOException("No response from the iEdge");
    }
  }
  /**
   * sends an initiate download command to the iedge
   */
  public static void initiateDownload (InetAddress addr, int softwareCode, int timeout) throws IOException {
    try {
      ByteArrayOutputStream bos = new ByteArrayOutputStream(47);
      DataOutputStream dos = new DataOutputStream(bos);
      dos.writeShort(CommonConstants.COMMAND);
      dos.writeShort(CommonConstants.HELLO_VERSION_NO_AUTH);
      Hello.fillPermission(dos, null);
      Hello.fillPermission(dos, null);
      dos.writeShort(CommonConstants.DOWNLOAD_REBOOT_COMMAND);
      dos.writeShort(softwareCode);
      dos.writeByte(0);
      dos.close();

      DatagramPacket dpo = new DatagramPacket(bos.toByteArray(),
					      bos.size(),
					      addr,
					      CommonConstants.MCAST_SEND_PORT);
      byte [] in = new byte [128];
      DatagramPacket dpi = new DatagramPacket(in, in.length);
      DatagramSocket ds = new DatagramSocket();
      ds.setSoTimeout(timeout*1000);
      ds.send(dpo);
      ds.receive(dpi);
      LimitedDataInputStream dis = new LimitedDataInputStream(new ByteArrayInputStream(in), dpi.getLength());
      ds.close();
      if (!LimitedCommandComm.extractSetReply(dis))
	throw new IOException("iEdge could not initiate download");
    } catch (InterruptedIOException ie) {
      throw new IOException("No response from the iEdge");
    }
  }	 

  /**
   * send the set commands to the iedge
   */
  public static void sendCommands (InetAddress addr, int timeout, String commands) throws IOException {
    Hello.sendPackets(LimitedCommandComm.createSetRequest(new BufferedReader(new StringReader(commands)), null, addr, false), timeout);
  }

  /**
   * send the datagram packets in the vector...
   * (if there are any multicast packets in the vectors, they should
   * be the first packet in the vector)
   */
  public static void sendPackets (Vector v, int timeout) throws IOException {
    if (v == null)
      return;

    int packetsLeft = v.size();
    Enumeration e = v.elements();
    InetAddress replyAddr = null;
    DatagramSocket s = null;
    boolean saveStatus = true;
    while (e.hasMoreElements()) {
      try {
	byte [] in = new byte [24];
	DatagramPacket dpi = new DatagramPacket(in, in.length);
	DatagramPacket toSend = (DatagramPacket)e.nextElement();
	if (s == null) {
	  if (toSend.getAddress().isMulticastAddress()) {
	    NextoneMulticastSocket m = new NextoneMulticastSocket();
	    m.joinGroupOnAllIf(InetAddress.getByName(CommonConstants.MCAST_ADDRESS));
	    s = m;
	  } else
	    s = new DatagramSocket();
	  s.setSoTimeout(timeout*1000);
	}
	if (replyAddr != null &&
	    !toSend.getAddress().isMulticastAddress())
	  toSend.setAddress(replyAddr);
	s.send(toSend);
	s.receive(dpi);
	replyAddr = dpi.getAddress();
	LimitedDataInputStream ldis = new LimitedDataInputStream(new ByteArrayInputStream(in), dpi.getLength());
	saveStatus = LimitedCommandComm.extractSetReply(ldis);
	ldis.close();
	if (!saveStatus)
	  break;
	if (--packetsLeft > 0) {
	  try {
	    Thread.sleep(500);
	  } catch (InterruptedException ie) {}
	}
      } catch (InterruptedIOException iie) {
	try {
	  if (s.getClass().equals(com.nextone.util.NextoneMulticastSocket.class))
	    ((NextoneMulticastSocket)s).leaveGroupOnAllIf(InetAddress.getByName(CommonConstants.MCAST_ADDRESS));
	  s.close();
	} catch (IOException ee) {}
	throw new IOException("iEdge not responding");
      } catch (IOException ie) {
	try {
	  if (s.getClass().equals(com.nextone.util.NextoneMulticastSocket.class))
	    ((NextoneMulticastSocket)s).leaveGroupOnAllIf(InetAddress.getByName(CommonConstants.MCAST_ADDRESS));
	  s.close();
	} catch (IOException ee) {}
	throw ie;
      }
    }

    try {
      if (s != null) {
	if (s.getClass().equals(com.nextone.util.NextoneMulticastSocket.class))
	  ((NextoneMulticastSocket)s).leaveGroupOnAllIf(InetAddress.getByName(CommonConstants.MCAST_ADDRESS));
	s.close();
      }
    } catch (IOException ie) {}

    if (!saveStatus)
      throw new IOException("iEdge rejected commands (incompatible software?)");
  }

  /**
   * send the download parameters to the iedge
   */
  /*
  public static void sendDownloadParams (InetAddress addr, int mode, int timeout, DTServer server, int devType) throws IOException {
    if (server == null)
      throw new IOException("Internal error: server is null");

    StringBuffer sb = new StringBuffer();
    if (server.getServerAddress() == null)
      throw new IOException("no server address configured");
    else
      sb.append("set download_server_address " + server.getServerAddress().getHostAddress() + "\n");
    sb.append("set download_user " + server.getUsername() + "\n");
    sb.append("set download_password " + server.getPassword() + "\n");
    sb.append("set download_directory " + server.getDirectory(devType) + "\n");
    if (mode == CommonConstants.ROM_MODE)
      sb.append("set softrom_download_file " + server.getFile(devType) + "\n");
    else
      sb.append("set download_file " + server.getFile(devType) + "\n");

    sendCommands(addr, timeout, sb.toString());
  }
  */
}

