package com.nextone.common;

import java.net.*;
import java.io.*;
import java.awt.*;
import java.util.*;
import com.nextone.util.LimitedDataInputStream;
import com.nextone.util.NextoneMulticastSocket;

/**
 * This class is mostly used by jFac/jServer, as none of the methods in
 * this class requires an I500 object
 *
 * Also, the mothods in this class uses a hello protocol version which
 * does not use password authorization.
 */
public class LimitedCommandComm implements CommonConstants {
	  // This needs to be able to handle the max message received.
	  protected static final int MAX_DGRAM_SIZE = 32000;

	  public static boolean extractSetReply (DataInputStream dis, NexToneFrame jc) throws IOException {
		 return esr(dis, new BufferedReader(new InputStreamReader(dis)));
	  }

	  public static boolean extractSetReply (LimitedDataInputStream ldis) throws IOException {
		 return esr(ldis, new BufferedReader(new InputStreamReader(ldis)));
	  }

	  private static boolean esr (DataInput di, BufferedReader r) throws IOException {
		 short code = di.readShort();
		 if (code != CommonConstants.COMMAND_REPLY) {
			return false;
		 }

		 String reply = r.readLine();
		 if (reply.equals("success")) {
			return true;
		 }

		 return false;
	  }

	  /**
	   * This function takes a BufferedReader stream, and creates a < 512
	   * byte size datagram packets and returns a Vector containing those
	   * packets
	   */
	  public static Vector createSetRequest (BufferedReader in, String sno, InetAddress t, boolean isFileExecution) throws IOException {
		 if (sno == null)
			return LimitedCommandComm.createSetRequest(in, null, t, isFileExecution, false);

		 return LimitedCommandComm.createSetRequest(in, sno.getBytes(), t, isFileExecution, false);
	  }

	  // the presence of "sno" determines if the packet will be multicasted
	  // or not
	  public static Vector createSetRequest (BufferedReader in, byte [] sno, InetAddress t, boolean isFileExecution, boolean dummy) throws IOException {
		 Vector p = new Vector();
		 ByteArrayOutputStream bos = new ByteArrayOutputStream(512);
		 DataOutputStream dos = new DataOutputStream(bos);
		 String line;
		 StringBuffer commands = new StringBuffer(512);
		 boolean byeSeen = false;
		 int headerLen;
		 InetAddress toAddr = null;

		 if (sno != null) {
			// commands should be sent multicast
			headerLen = sno.length + 8;
			toAddr = InetAddress.getByName(CommonConstants.MCAST_ADDRESS);
		 } else {
			headerLen = 6;
			toAddr = t;
		 }

		 while ((line = in.readLine()) != null) {
			if (line.startsWith("#")) {
			   // this is a comment line
			   continue;
			}
			if (line.startsWith("get")) // gets might crash the board in RAM
			   continue;
			if ((headerLen + commands.length() + line.length()) > 490) {
			   commands.append("bye\n");
			   dos.writeShort(CommonConstants.COMMAND);
			   dos.writeShort(CommonConstants.HELLO_VERSION_NO_AUTH);
			   Hello.fillPermission(dos, null);
			   Hello.fillPermission(dos, null);
			   if (sno != null) {
				  dos.writeShort(CommonConstants.MCAST_SET_COMMAND);
				  dos.writeShort(sno.length);
				  dos.write(sno);
			   } else
				  dos.writeShort(CommonConstants.SET_COMMAND);
			   dos.writeShort(commands.length());
			   dos.writeBytes(new String(commands));
			   p.add(new DatagramPacket(bos.toByteArray(),
										bos.size(),
										toAddr,
										CommonConstants.MCAST_SEND_PORT));
			   bos = new ByteArrayOutputStream(512);
			   dos = new DataOutputStream(bos);
			   commands = new StringBuffer(512);
			   byeSeen = false;
			}
			commands.append(line + "\n");
			if (line.startsWith("bye"))
			   byeSeen = true;
		 }
		 if (isFileExecution) {
			if (!byeSeen)
			   commands.append("bye\n");
		 } else
			commands.append("bye\n");
		 dos.writeShort(CommonConstants.COMMAND);
		 dos.writeShort(CommonConstants.HELLO_VERSION_NO_AUTH);
		 Hello.fillPermission(dos, null);
		 Hello.fillPermission(dos, null);
		 if (sno != null) {
			dos.writeShort(CommonConstants.MCAST_SET_COMMAND);
			dos.writeShort(sno.length);
			dos.write(sno);
		 } else
			dos.writeShort(CommonConstants.SET_COMMAND);
		 dos.writeShort(commands.length());
		 dos.writeBytes(new String(commands));
		 p.add(new DatagramPacket(bos.toByteArray(),
								  bos.size(),
								  toAddr,
								  CommonConstants.MCAST_SEND_PORT));

		 in.close();
		 return p;
	  }

	  /**
	   * this method sends a multicast GET request
	   *
	   * @param cmd the command code for the GET request
	   * @param sno the serial number of the device we are sending the GET to
	   */
	  public static DatagramPacket createGetRequest (int cmd, String sno) throws IOException {
		 return LimitedCommandComm.createGetRequest(cmd, sno, InetAddress.getByName(CommonConstants.MCAST_ADDRESS));
	  }

	  /**
	   * this method sends a unicast GET request
	   *
	   * @param cmd the command code for the GET request
	   * @param toAddr the IP address of the device we sending the GET to
	   */
	  public static DatagramPacket createGetRequest (int cmd, InetAddress toAddr) throws IOException {
		 return LimitedCommandComm.createGetRequest(cmd, null, toAddr);
	  }

	  private static DatagramPacket createGetRequest (int cmd, String sno, InetAddress toAddr) throws IOException {
		 ByteArrayOutputStream bos = new ByteArrayOutputStream(128);
		 DataOutputStream dos = new DataOutputStream(bos);
		 dos.writeShort(CommonConstants.COMMAND);
		 dos.writeShort(CommonConstants.HELLO_VERSION_NO_AUTH);
		 Hello.fillPermission(dos, null);
		 Hello.fillPermission(dos, null);
		 if (sno == null) {
			dos.writeShort(CommonConstants.GET_COMMAND);
		 } else {
			dos.writeShort(CommonConstants.MCAST_GET_COMMAND);
			dos.writeShort(sno.length());
			dos.writeBytes(sno);
		 }
		 dos.writeShort(cmd);
		 dos.writeByte(0);

		 return new DatagramPacket(bos.toByteArray(),
								   bos.size(),
								   toAddr,
								   CommonConstants.MCAST_SEND_PORT);
	  }

	  /**
	   * sends a unicast GET request for the given command code
	   *
	   * @param code the command code for the GET request
	   * @param toAddr the IP address of the device we sending the GET to
	   * @param con the DataConsumer interested in the result of this GET
	   * @param tp the TimeoutProvider to provide timeouts
	   */
	  public static boolean doGet (int code, InetAddress toAddr, DataConsumer con, TimeoutProvider tp) throws IOException {
		 return LimitedCommandComm.doGet(code, null, toAddr, con, tp);
	  }

	  /**
	   * sends a multicast GET request for the given command code
	   *
	   * @param code the command code for the GET request
	   * @param sno the serial number of the device we are sending the GET to
	   * @param con the DataConsumer interested in the result of this GET
	   * @param tp the TimeoutProvider to provide timeouts
	   */
	  public static boolean doGet (int code, String sno, DataConsumer con, TimeoutProvider tp) throws IOException {
		 return LimitedCommandComm.doGet(code, sno, InetAddress.getByName(CommonConstants.MCAST_ADDRESS), con, tp);
	  }

	  private static boolean doGet (int code, String sno, InetAddress toAddr, DataConsumer con, TimeoutProvider tp) throws IOException {
		 byte [] in = new byte [MAX_DGRAM_SIZE];
		 DatagramPacket dpi = new DatagramPacket(in, in.length);
		 DatagramPacket dpo = LimitedCommandComm.createGetRequest(code, sno, toAddr);
		 DatagramSocket s = null;
		 NextoneMulticastSocket m = null;
		 if (sno == null) {
			s = new DatagramSocket();
		 } else {
			m = new NextoneMulticastSocket();
			m.joinGroupOnAllIf(toAddr);
			s = m;
		 }

		 int times = tp.getGetTimeout()/CommonConstants.RETRY_INTERVAL;
		 int remain = tp.getGetTimeout()%CommonConstants.RETRY_INTERVAL;
		 if (remain != 0)
			times++;
		 while (times-- > 0) {
			try {
			   if (times == 0 && remain != 0)
				  // last try, only wait for the remaining interval
				  s.setSoTimeout(remain*1000);
			   else
				  // wait for the retry interval time
				  s.setSoTimeout(CommonConstants.RETRY_INTERVAL*1000);
			   s.send(dpo);
			   s.receive(dpi);
			   break; // receive successful, break out of the loop
			} catch (InterruptedIOException iie) {
			   if (times == 0)
				  throw iie;
			}
		 }

		 LimitedDataInputStream dis = new LimitedDataInputStream(new ByteArrayInputStream(in), dpi.getLength());
		 con.extractGetReply(dis);
		 if (sno != null)
			m.leaveGroupOnAllIf(toAddr);
		 dis.close();
		 s.close();

		 return true;
	  }

}
