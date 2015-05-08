package com.nextone.common;

import java.net.*;
import java.io.*;
import java.util.*;
import com.nextone.util.SysUtil;
import com.nextone.JUCon.JUCon;
import com.nextone.JUCon.NextoneDevice;
import com.nextone.JUCon.EdgeDevice;
import com.nextone.JUCon.Constants;

//
import com.nextone.JUCon.iServer.IServer;

/**
 * The PingTCP class is implemented to provide a mechanism
 * to determine the existence of devices that do not provide
 * an interface to the nextone MultiCast machanism.
 * A telnet connection is attempted. If the connection is 
 * successful then the device is assumed alive.
 */
public class PingTCP extends Thread implements Constants {
  private Socket socket;
  private boolean keepRunning = true;
  private NexToneFrame nf;
  private boolean isJFac = false;

  /**
   * Constructor
   * @param tg the threadgroup to belong to.
   * @param ds the NextoneMulticast socket to use.
   * @param nf the parent NextoneFrame that provides the 
   *           application specific (JUCon/JFac) info.
   */
  public PingTCP (ThreadGroup tg, NexToneFrame nf) {
    super(tg, "PingTCP");
    this.nf = nf;
  }

  /**
   * Stop the thread.
   */
    public void stopMcast () {
    keepRunning = false;
  }

  /**
   * Main processing method for the thread.
   * 
   */
  public void run () {
    NextoneDevice i = null;
    String s = null;

    while (keepRunning) {
      // JUCon specific code
      JUCon pc = (JUCon)nf;
      Map map = pc.getRemoteThirdPartyDevices();
      if (map != null) {
        synchronized (map) {
          // For each of the static (remote) devices send the ping.
          Enumeration e = Collections.enumeration(map.values());
          while (e.hasMoreElements()) {
            i = (NextoneDevice)e.nextElement();
            if (!i.isSaving()) {
              s = i.getAddress().toString();
              pc.appendDebug("PingTCP: hello to: "+s+":"+23);
              // SEND PING if it works, send a REGISTRATION message to the
              // RX Thread
              try {
                // Try to telnet to the given device.
                socket = new Socket(i.getAddress(),23);
                socket.close();
                System.out.println("Socket opened succesfully");
                // Send a REGISTRATION to the receive thread. TODO
                ReceiveReg rr = pc.getReceiveReg();
                rr.handleVegaStreamRegistration(i.getAddress());

              } catch (IOException ioe) {
                // Unable to 'ping' this device. TODO
              }
            }
          }
        }
      }
      s = "";

      // sleep...
      try {
        Thread.sleep(1000*nf.getPollInterval());
      } catch (InterruptedException e) {
      }
    }
    nf.appendDebug("PingTCP exited");
  }
}
