package com.nextone.JServer;

import java.util.*;
import com.nextone.common.MaintenanceRequest;


public class StopDownloadTask extends TimerTask {
  private MaintenanceRequest mr;
  private DownloadServer ds;
  private boolean running;

  StopDownloadTask (MaintenanceRequest mr, DownloadServer ds) {
    this.mr = mr;
    this.ds = ds;
  }

  public void run () {
    running = true;
    String request = mr.getName();

    JServer.printDebug("cancelling " + request, JServer.DEBUG_VERBOSE);

    // cancel the DownloadTask
    Map m = ds.getStartList();
    if (m.containsKey(request)) {
      DownloadTask dt = (DownloadTask)m.get(request);
      LogTask.logWarning(request, "Time to stop the request, canceling the remaining actions", Constants.MAINTENANCE_LOGFILE);
      dt.cancel(false);
      m.remove(request);
      JServer.printDebug("cancelled " + request, JServer.DEBUG_NORMAL);
    }
    ds.getEndList().remove(request);
  }

  public boolean cancel () {
    if (running) {
      // ----------cleanup here
    }

    return super.cancel();
  }

}

