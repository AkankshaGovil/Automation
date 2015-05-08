package com.nextone.JServer;

import java.net.*;
import java.io.*;
import java.util.*;
import com.nextone.util.DeltaTime;
import com.nextone.util.LimitedDataInputStream;
import com.nextone.util.NextoneMulticastSocket;
import com.nextone.util.NextoneThreadGroup;
import com.nextone.common.SingleInstance;
import com.nextone.common.iServerConfig;
import com.nextone.common.Capabilities;
import com.nextone.common.BridgeException;


public class JServerMain implements Constants {
  // load the library containing version number information
  static {
    System.loadLibrary("BridgeServer");
  }

  public native static String getName ();
  public native static String getMajorVersion ();
  public native static String getMinorVersion ();
  public native static String getReleaseName ();
  public native static String getBuildDate ();
  public native static String getCopyright ();
  public native static int getUptimeServerPort ();
  public native static int getUptimePacketSize ();
  private static native int mswInit ();
  private static native void nativeInit (iServerConfig config, Capabilities cap) throws BridgeException;

  static String [] versionInfo;
  private static InetAddress toIp = null;
  private static int utServerPort, utPacketSize;
  private static SingleInstance si;

  public static void main (String [] args) {

    boolean debug = false;
    int debugLevel = -1;

    try {
      // getLocalHost() only returns eth0 address in linux with
      // multiple network interfaces, we really want
      // the 127.0.0.1 kind of address
      //			toIp = InetAddress.getLocalHost();
      // sometimes "localhost" entry may not be in the hosts table
      //			toIp = InetAddress.getByName("localhost");
      toIp = InetAddress.getByName("127.0.0.1");
    } catch (UnknownHostException ue) {
      System.err.println("Cannot find an IP address on the local host!");
      System.err.println(ue);
      System.exit(0);
    }

    utServerPort = getUptimeServerPort();
    utPacketSize = getUptimePacketSize();

    if (args.length > 2 ||
	(args.length == 2 && !args[0].equals("debug")) ||
	(args.length == 1 && !args[0].equals("stop") && !args[0].equals("start") && !args[0].equals("status") && !args[0].equals("debug") && !args[0].equals("version") && !args[0].equals("uptime") && !args[0].equals("reconfig"))) {
      System.err.println("valid options: [ [start] | stop | status | version | debug [" + JServer.debugStr[0] + " | " + JServer.debugStr[1] + " | " + JServer.debugStr[2] + " | " + JServer.debugStr[3] + " | " + JServer.debugStr[4] + "] | uptime | reconfig ]");
      System.exit(0);
    }

    if (args.length == 1) {
      if (args[0].equals("version")) {
	System.out.println(getName() + " " + getMajorVersion() + getMinorVersion() + ", " + getBuildDate());
	System.out.println(getCopyright());
	System.exit(0);
      }
      if (args[0].equals("stop"))
	handleStop();
      else if (args[0].equals("status"))
	handleStatus();
      else if (args[0].equals("debug")) {
	debug = true;
      } else if (args[0].equals("uptime"))
	handleUptime();
      else if (args[0].equals("reconfig"))
	handleReconfig();
    }

    if (args.length == 2) {
      debug = true;
      for (int i = 0; i < JServer.debugStr.length; i++) {
	if (args[1].equals(JServer.debugStr[i])) {
	  debugLevel = i;
	  break;
	}
      }
      if (debugLevel == -1) {
	System.err.println("valid options: debug [" + JServer.debugStr[0] + " | " + JServer.debugStr[1] + " | " + JServer.debugStr[2] + " | " + JServer.debugStr[3] + " | " + JServer.debugStr[4] + "]");
	System.exit(0);
      }
    }

    if (debug)
      handleDebug(debugLevel);

    // proceed to start a new instance of jserver

    // store version information locally
    versionInfo = new String [7];
    versionInfo[0] = getReleaseName();
    versionInfo[1] = getMajorVersion();
    versionInfo[2] = getMinorVersion();
    versionInfo[3] = getBuildDate();
    versionInfo[4] = getCopyright();
    versionInfo[5] = getName();
    try {
      Process p = Runtime.getRuntime().exec("modinfo");
      BufferedReader br = new BufferedReader(new InputStreamReader(new SequenceInputStream(p.getInputStream(), p.getErrorStream())), 1024);
      String line = null;
      while ((line = br.readLine()) != null) {
        if (line.indexOf("NSF") != -1) {
          StringTokenizer st = new StringTokenizer(line, ":");
          st.nextToken();
          line = st.nextToken();
          if (line != null && line.trim().charAt(0) == 'v')
            versionInfo[6] = line.substring(0, line.indexOf(")")).trim();
        }
      }

      try {
        p.waitFor();
      } catch (InterruptedException ie) {}
      p.destroy();
    } catch (IOException ie) {
      System.err.println("Unable to get NSF version: " + ie.getMessage());
    }

    // secure a single instance of the server
    //si = SingleInstance.getSingleInstance(new NextoneThreadGroup("jServer main threads"), Constants.JSERVER_INSTANCE_PORT);

    // give a chance for the other thread to run
    //Thread.yield();

    System.out.println("[JServerMain] Initializing Configuration Server...."); 
    //if (si != null) {
      try{
             // initialize the native code stuff
      iServerConfig cfg  =  new iServerConfig();
      nativeInit(new iServerConfig(), new Capabilities());
      if (mswInit() != 0)
       exit(new Exception("Unable to initialize shared memory/attach to cache"))
;
       startJServer();
     }catch(Exception e){
       exit(e);
     }
//    try {
//	JServer.getInstance(si.getThreadGroup(), si);
//     } catch (Exception ie) {
//       exit(ie);
//    }
   //} else {
      //System.out.println("A previous instance of jServer is still running on this machine");
      //System.exit(0);
    //}
  }
 
  public static void exit(Exception e){
    System.err.println("Error starting jServer: ");
    System.err.println(e);
    e.printStackTrace();
    System.exit(-1);
  }


  public static void startJServer() throws Exception{
     try {

    // secure a single instance of the server
       si = SingleInstance.getSingleInstance(new NextoneThreadGroup("jServer main threads"), Constants.JSERVER_INSTANCE_PORT);
       // give a chance for the other thread to run
       Thread.yield();

       if (si != null) {
        JServer.getInstance(null, si);
        System.out.println("[JServerMain] Started  jserver");
       } else {
         System.out.println("A previous instance of jServer is still running on this machine");
         System.exit(0);
       }
     } catch (Exception ie) {
      System.out.println("A previous instance of jServer is still running on this machine");
        exit(ie);
     }
  }

  public static void handleRestart () {
    try{
      System.out.println("[JServerMain] Stopping jserver .....");
      stopJServer();
      System.out.println("[JServerMain] Starting jserver .....");
      startJServer();
    }catch(Exception e){
      System.out.println("Error occured while restarting the jServer");
        exit(e);
    } 
  
  }

  private static void stopJServer () {
    try {
      ByteArrayOutputStream bos = new ByteArrayOutputStream(256);
      DataOutputStream dos = new DataOutputStream(bos);
      dos.writeShort(Constants.RESTART);
      ObjectOutputStream oos = new ObjectOutputStream(dos);
      oos.writeObject(toIp);

      DatagramPacket dp = new DatagramPacket(bos.toByteArray(), bos.size(), toIp, Constants.MCAST_JSERVER_LISTEN_PORT);
      DatagramSocket ds = new DatagramSocket();
      ds.send(dp);

      byte [] in = new byte [MAX_IVIEW_ISERVER_PACKET_SIZE];
      DatagramPacket dpi = new DatagramPacket(in, in.length);
      int retryInterval = RETRY_INTERVAL;
      int timeout = GET_TIMEOUT*1000;
      int times = 5;   
      int remain = timeout%(retryInterval*1000); 
      if (remain != 0)   
        times++;

      ds.setSoTimeout(1000);
      boolean keepRunning = true; 
      while (keepRunning) {
        try {
          Thread.yield();
	  ds.receive(dpi);
          
	  break;  // receive successfull, break out of the loop
        } catch (InterruptedIOException iie) {
          System.out.println("Could not stop the jserver "+iie.toString());
        }
      }

      ds.close();
    } catch (IOException ie) {
      System.err.println("jServer Stop error:\n" + ie.toString() + "\n");
    }
  }

  // signal stop to the currently running jserver
 private static void handleStop(){
    try {
      ByteArrayOutputStream bos = new ByteArrayOutputStream(256);
      DataOutputStream dos = new DataOutputStream(bos);
      dos.writeShort(Constants.STOP);
      ObjectOutputStream oos = new ObjectOutputStream(dos);
      oos.writeObject(toIp);

      DatagramPacket dp = new DatagramPacket(bos.toByteArray(), bos.size(), toIp, Constants.MCAST_JSERVER_LISTEN_PORT);
      DatagramSocket ds = new DatagramSocket();
      ds.send(dp);
      ds.close();
    } catch (IOException ie) {
      System.err.println("jServer Stop error:\n" + ie.toString() + "\n");
    }

    System.out.close();
    System.err.close();
    System.runFinalization();
    System.exit(0);
  }


  // set a new debug level for the currently running jserver
  private static void handleDebug (int debugLevel) {
    try {
      ByteArrayOutputStream bos = new ByteArrayOutputStream(256);
      DataOutputStream dos = new DataOutputStream(bos);
      dos.writeShort(Constants.DEBUG);
      if (debugLevel == -1)
	debugLevel = JServer.DEBUG_NORMAL;
      dos.writeShort(debugLevel);
      ObjectOutputStream oos = new ObjectOutputStream(dos);
      oos.writeObject(toIp);

      DatagramPacket dp = new DatagramPacket(bos.toByteArray(), bos.size(), toIp, Constants.MCAST_JSERVER_LISTEN_PORT);
      DatagramSocket ds = new DatagramSocket();
      ds.send(dp);
      ds.close();
    } catch (IOException ie) {
      System.err.println("Error sending debug request to the jServer:\n" + ie.getMessage() + "\n");
    }

    System.exit(0);
  }


  // contacts pm for uptime on bcs/gis/faxs and then contacts currently
  // running jserver for jserver uptime
  private static void handleUptime () {
    DatagramSocket ds = null;
    long currentTime = System.currentTimeMillis();

    try {
      ds = new DatagramSocket();
    } catch (IOException ie) {
      System.out.println("Cannot open an UDP socket: " + ie);
      System.exit(-1);
    }

    // contact pm
    try {
      byte [] req = new byte [utPacketSize];
      Arrays.fill(req, (byte)0xff);
      DatagramPacket dp = new DatagramPacket(req, req.length, toIp, utServerPort);
      byte [] in = new byte [1024];
      DatagramPacket dpi = new DatagramPacket(in, in.length);
      ds.setSoTimeout(Constants.GET_TIMEOUT*1000);
      ds.send(dp);
      ds.receive(dpi);

      LimitedDataInputStream dis = new LimitedDataInputStream(new ByteArrayInputStream(in), dpi.getLength());
      short times = dis.readShort();
      for (int i = 0; i < times; i++) {
	String s = dis.readUTF();
	long t = dis.readLong()*1000;
	if (t != 0) {
	  System.out.println(s);
	  System.out.println("\t" + new DeltaTime(currentTime - t).toString());
	}
      }
    } catch (InterruptedIOException ie) {
      System.out.println("pm not responding to uptime requests");
    } catch (Exception e) {
      System.err.println("Could not receive uptime for BCS/GIS/FAXS:\n");
      System.err.println(e);
    }

    try {
      ByteArrayOutputStream bos = new ByteArrayOutputStream(256);
      DataOutputStream dos = new DataOutputStream(bos);
      dos.writeShort(Constants.UPTIME);
      ObjectOutputStream oos = new ObjectOutputStream(dos);
      oos.writeObject(toIp);

      DatagramPacket dp = new DatagramPacket(bos.toByteArray(), bos.size(), toIp, Constants.MCAST_JSERVER_LISTEN_PORT);
      byte [] in = new byte [1024];
      DatagramPacket dpi = new DatagramPacket(in, in.length);
      ds.send(dp);
      ds.receive(dpi);

      ObjectInputStream ois = new ObjectInputStream(new ByteArrayInputStream(in));
      System.out.print("Uptime for: ");
      System.out.println(getName() + " " + getMajorVersion() + getMinorVersion() + ", " + getBuildDate());
      System.out.println("\t" + (String)ois.readObject());
    } catch (InterruptedIOException ie) {
      System.out.println("jServer is not replying, is it running?");
    } catch (IOException ie) {
      if (ie.getMessage().equals("socket closed"))
	System.out.println("jServer is not replying, is it running?");
      else
	System.err.println("Error sending uptime request to the jServer:\n" + ie.getMessage() + "\n");
    } catch (Exception e) {
      System.err.println("Unrecognized error: " + e.toString());
    }

    ds.close();
    System.exit(0);
  }


  // retrieve status from the currently running jserver
  private static void handleStatus () {
    DatagramSocket ds = null;
    try {
      ds = new DatagramSocket();
      ByteArrayOutputStream bos = new ByteArrayOutputStream(256);
      DataOutputStream dos = new DataOutputStream(bos);
      dos.writeShort(Constants.STATUS);
      ObjectOutputStream oos = new ObjectOutputStream(dos);
      oos.writeObject(toIp);

      DatagramPacket dp = new DatagramPacket(bos.toByteArray(), bos.size(), toIp, Constants.MCAST_JSERVER_LISTEN_PORT);
      byte [] in = new byte [1000];
      DatagramPacket dpi = new DatagramPacket(in, in.length);
      ds.setSoTimeout(Constants.GET_TIMEOUT*1000);
      ds.send(dp);
      ds.receive(dpi);
      ObjectInputStream ois = new ObjectInputStream(new ByteArrayInputStream(in));
      System.out.println((String)ois.readObject() + "\n");
    } catch (InterruptedIOException ie) {
      System.out.println("jServer is not replying, is it running?");
    } catch (IOException ie) {
      if (ie.getMessage().toLowerCase().equals("socket closed"))
	System.out.println("jServer is not replying, is it running?");
      else
	System.err.println("Error sending status request to the jServer:\n" + ie.getMessage() + "\n");
    } catch (Exception e) {
      System.err.println("Unrecognized error: " + e.toString());
    }

    ds.close();
    System.exit(0);
  }

  // reconfigure the currently running jserver
  // re-reads the password and log file parameters from the cfg file
  private static void handleReconfig () {
    try {
      ByteArrayOutputStream bos = new ByteArrayOutputStream(256);
      DataOutputStream dos = new DataOutputStream(bos);
      dos.writeShort(Constants.RECONFIG);
      ObjectOutputStream oos = new ObjectOutputStream(dos);
      oos.writeObject(toIp);

      DatagramPacket dp = new DatagramPacket(bos.toByteArray(), bos.size(), toIp, Constants.MCAST_JSERVER_LISTEN_PORT);
      DatagramSocket ds = new DatagramSocket();
      ds.send(dp);
      ds.close();
    } catch (IOException ie) {
      System.err.println("Error sending reconfig request to the jServer:\n" + ie.getMessage() + "\n");
    }

    System.exit(0);
  }

}

