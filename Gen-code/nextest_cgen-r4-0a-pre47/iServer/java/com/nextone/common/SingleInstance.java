package com.nextone.common;

import java.awt.*;
import java.io.*;
import java.net.*;
import javax.swing.*;
import com.nextone.util.UDPServer;
import com.nextone.util.NextoneThreadGroup;

public class SingleInstance extends UDPServer implements CommonConstants {
	  private boolean keepRunning = true;
	  private DatagramSocket ds;
	  private NexToneFrame nf;
	  private int instancePort;
	  private static boolean isWindows = false;
	  private static final short STOP = 0;
	  private static final short TOFRONT = 1;

	  static {
		 if(System.getProperties().getProperty("os.name").indexOf("Windows") != -1)
			isWindows = true;
	  }

	  private SingleInstance (DatagramSocket s) throws IOException {
		 super(s, Thread.NORM_PRIORITY);
		 ds = s;
		 instancePort = s.getLocalPort();
		 keepRunning = true;
		 start();
	  }

	  private SingleInstance (ThreadGroup tg, DatagramSocket s) throws IOException {
		 super(tg, s, Thread.NORM_PRIORITY, "SingleInstance");
		 ds = s;
		 instancePort = s.getLocalPort();
		 keepRunning = true;
		 start();
	  }

	  private static void showMesg (String msg) {
		 if (isWindows)
			JOptionPane.showMessageDialog(null, msg, "Error", JOptionPane.ERROR_MESSAGE);
		 else
			System.err.println(msg);
	  }

	  public static SingleInstance getSingleInstance (String str) {
		 return SingleInstance.getSingleInstance(new NextoneThreadGroup(str));
	  }

	  public static SingleInstance getSingleInstance (ThreadGroup tg) {
		 return SingleInstance.getSingleInstance(tg, CommonConstants.INSTANCE_PORT);
	  }

	  public static SingleInstance getSingleInstance (ThreadGroup tg, int port) {
		 ThreadGroup threadGroup = null;
		 if (tg == null)
			threadGroup = new NextoneThreadGroup("NextoneThreadGroup");
		 else
			threadGroup = tg;
		 SingleInstance si = null;
		 try {
			si = new SingleInstance(tg, new DatagramSocket(port));
		 } catch (BindException be) {
			try {
			   showMesg("A previous instance of this program is already running on this machine");
			   DatagramSocket s = new DatagramSocket();
			   send(TOFRONT, s, port);
			   s.close();
			} catch (IOException ie) {
			   showMesg(ie.toString() + "\n");
			}
		 } catch (IOException ie) {
			showMesg(ie.toString() + "\n");
		 }

		 return si;
	  }

	  public void stop () {
		 keepRunning = false;

		 try {
			DatagramSocket s = new DatagramSocket();
			send(STOP, s, instancePort);
			s.close();
		 } catch (IOException ie) {
			println("SI stop error:\n" + ie.toString());
		 }
	  }

	  private static void send (int code, DatagramSocket s, int port) throws IOException {
		 ByteArrayOutputStream bos = new ByteArrayOutputStream(2);
		 DataOutputStream dos = new DataOutputStream(bos);
		 dos.writeShort((short)code);

//		 InetAddress toIp = InetAddress.getByName("localhost");
		 InetAddress toIp = InetAddress.getByName("127.0.0.1");
		 DatagramPacket dp = new DatagramPacket(bos.toByteArray(), bos.size(), toIp, port);
		 s.send(dp);
	  }
		 
	  public void setNexToneFrame (NexToneFrame nf) {
		 this.nf = nf;
	  }

	  public void UDPServerWork (DatagramSocket socket) {
		 keepRunning = true;
		finish:
		 while (keepRunning) {
			try {
			   byte [] in = new byte [1000];
			   DatagramPacket dp = new DatagramPacket(in, in.length);
			   socket.receive(dp);

			   DataInputStream dis = new DataInputStream(new ByteArrayInputStream(in));
			   short code = dis.readShort();
			   switch (code) {
				  case STOP:   // thread is being stopped
					break finish;
				 
				  case TOFRONT:   // bring the focus back
					if (nf != null) {
					   if (nf.getState() == Frame.ICONIFIED)
						  nf.setState(Frame.NORMAL);
					   nf.toFront();
					}
					break;

				  default:
					println("SI: unknown response code " + code + " from " + dp.getAddress().toString());
			   }
			   dis.close();
			} catch (IOException ie) {
			   println("SI receive error:\n" + ie.toString());
			}
		 }
		 ds.close();
		 if (nf != null)
			nf.appendDebug("SingleInstance exited");
	  }

	  private void println (String str) {
		 if (nf != null)
			nf.appendDebug(str + "\n");
		 else
			showMesg(str);
	  }

}

