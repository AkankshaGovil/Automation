package com.nextone.JServer;

import java.util.*;
import java.net.*;
import java.io.*;

/**
 * A thread which sends keep alives to the process manager
 */
public class ProcessManagerClient extends Thread {
  static {
    System.loadLibrary("BridgeServer");
  }

  private boolean pollStarted;

  private native boolean initPoll ();
  private native boolean sendPoll ();

  private long sleepTime;
  private boolean keepRunning = true;
  private Thread thisThread;

  ProcessManagerClient (long st) {
    super();

    this.sleepTime = st;

    this.pollStarted = initPoll();

    if (!pollStarted)
      JServer.printDebug("Failed to init process manager polling", JServer.DEBUG_ERROR);
    else
      JServer.printDebug("Registered with process manager", JServer.DEBUG_NORMAL);
  }

  public void run () {
    thisThread = Thread.currentThread();
    try {
      thisThread.sleep(1000);  // initial delay
    } catch (InterruptedException ie) {}

    JServer.printDebug("PMC priority is " + thisThread.getPriority() + ", allowed max is " + Thread.MAX_PRIORITY, JServer.DEBUG_NORMAL);

    while (keepRunning) {
      try {
        if (!pollStarted)
          pollStarted = initPoll();

        long st = sleepTime;

        if (pollStarted) {
          JServer.printDebug("attempting to send keep alive to the process manager", JServer.DEBUG_VERBOSE);
          if (sendPoll())
            JServer.printDebug("sent keep alive to the process manager", JServer.DEBUG_VERBOSE);
          else {
            JServer.printDebug("Failed to send keep alive to the process manager", JServer.DEBUG_WARNING);
            st = 1000;  // try again after 1 sec
          }
        } else {
          JServer.printDebug("Failed to init process manager polling", JServer.DEBUG_ERROR);
          st = 1000;  // try again after 1 sec
        }

        // sleep for the desired time
        thisThread.sleep(st);

      } catch (InterruptedException ie) {}
    }

    JServer.printDebug("PMC exited", JServer.DEBUG_NORMAL);
  }


  public void stopRunning () {
    keepRunning = false;
    thisThread.interrupt();
  }
}
