package com.nextone.JServer;

import java.net.*;
import java.util.*;
import java.io.*;
import com.nextone.util.LimitedDataInputStream;
import com.nextone.util.UDPServer;


/**
 * A thread which logs messages related to maintenance requests. Receives
 * UDP packets containing the maintenance request name and the message to
 * be logged, and logs the message to the correct file. Works as a low 
 * priority thread and provides serial access to the log files from the
 * other threads.
 */
public class LogTask extends UDPServer {
  private boolean keepRunning = false;
  private static LogTask thisInstance;
  private Map cache;
  private CacheCleanupTask cct;
  private File maintLogDir, autoDlLogDir;
  private static DatagramPacket dp;

  private static final int STOP = 0;
  private static final int CLEAN = 1;
  private static final int LOG = 2;

  private static final int CLEANUP_TIME = 5*60*1000;  // 5 minutes
  private static final int CLEANUP_WAKE_TIME = 60*1000; // 1 minute

  /**
   * use this to get the instance of this class
   */
  public static LogTask getInstance () {
    return thisInstance;
  }


  /**
   * use this to get the instance of this class (and to create it the first
   * time)
   * makes the thread belong to the passed ThreadGroup
   */
  public static LogTask getInstance (ThreadGroup tg, String cd) throws IOException {
    if (thisInstance == null)
      thisInstance = new LogTask(tg, new DatagramSocket(Constants.JSERVER_LOG_PORT), cd);

    return thisInstance;
  }

  /**
   * protected constructor so that no one can call it directly. access
   * to this class is limited through the static methods.
   */
  protected LogTask (ThreadGroup tg, DatagramSocket ds, String cd) throws IOException {
    super(tg, ds, Thread.MIN_PRIORITY, "LogServer", true);

    File dataDir = new File(cd + File.separator + Constants.dataDirName);
    this.maintLogDir = new File(dataDir.getPath() + File.separator + Constants.logDirName);
    File autoDlDir = new File(dataDir.getPath() + File.separator + Constants.autoDlDirName);
    this.autoDlLogDir = new File(autoDlDir.getPath() + File.separator + Constants.logDirName);

    dp = new DatagramPacket(new byte [0], 0, InetAddress.getByName("127.0.0.1"), Constants.JSERVER_LOG_PORT);

    cache = Collections.synchronizedMap(new HashMap());
    cct = new CacheCleanupTask();
    new Timer(true).schedule(cct, CLEANUP_WAKE_TIME, CLEANUP_WAKE_TIME);


    start();
  }


  /**
   * to stop this thread
   */
  public static void stop () throws IOException {
    if (thisInstance == null)
      return;

    ByteArrayOutputStream bos = new ByteArrayOutputStream(64);
    DataOutputStream dos = new DataOutputStream(bos);
    dos.writeShort(STOP);
    ObjectOutputStream oos = new ObjectOutputStream(dos);
    oos.writeObject(dp.getAddress());
    oos.close();

    dp.setData(bos.toByteArray(), 0, bos.size());
    DatagramSocket ds = new DatagramSocket();
    ds.send(dp);
    ds.close();
  }


  /**
   * to log a message regarding maintenance requests
   *
   * @param name the name of the maintenance request
   * @param msg the message to be logged
   */
  /*	  public static void logMaintError (String name, Object msg) {
	  logError(name, msg, Constants.MAINTENANCE_LOGFILE);
	  }

	  public static void logMaintWarning (String name, Object msg) {
	  logWarning(name, msg, Constants.MAINTENANCE_LOGFILE);
	  }

	  public static void logMaintMedium (String name, Object msg) {
	  logMedium(name, msg, Constants.MAINTENANCE_LOGFILE);
	  }

	  public static void logMaintVerbose (String name, Object msg) {
	  logVerbose(name, msg, Constants.MAINTENANCE_LOGFILE);
	  }
  */
  /**
   * to log a message regarding auto-download procedure
   *
   * @param name the regid
   * @param msg the message to be logged
   */
  /*	  public static void logAutoDlError (String name, Object msg) {
	  logError(name, msg, Constants.AUTODOWNLOAD_LOGFILE);
	  }

	  public static void logAutoDlWarning (String name, Object msg) {
	  logWarning(name, msg, Constants.AUTODOWNLOAD_LOGFILE);
	  }

	  public static void logAutoDlMedium (String name, Object msg) {
	  logMedium(name, msg, Constants.AUTODOWNLOAD_LOGFILE);
	  }

	  public static void logAutoDlVerbose (String name, Object msg) {
	  logVerbose(name, msg, Constants.AUTODOWNLOAD_LOGFILE);
	  }
  */
  /**
   * to log a message
   *
   * @param name the name of the log file
   * @param msg the message to be logged
   * @param lt type of the log file (i.e., MAINTENANCE_LOGFILE, 
   * AUTODOWNLOAD_LOGFILE, etc)
   */
  public static void logError (String name, Object msg, int lt) {
    log(name, msg, lt, Constants.LOG_LEVEL_ERROR);
  }

  public static void logWarning (String name, Object msg, int lt) {
    log(name, msg, lt, Constants.LOG_LEVEL_WARNING);
  }

  public static void logMedium (String name, Object msg, int lt) {
    log(name, msg, lt, Constants.LOG_LEVEL_MEDIUM);
  }

  public static void logVerbose (String name, Object msg, int lt) {
    log(name, msg, lt, Constants.LOG_LEVEL_VERBOSE);
  }

  private static void log (String name, Object msg, int logFileType, int logLevel) {
    try {
      ByteArrayOutputStream bos = new ByteArrayOutputStream(512);
      DataOutputStream dos = new DataOutputStream(bos);
      dos.writeShort(LOG);
      dos.writeInt(logFileType);
      dos.writeInt(logLevel);
      dos.writeUTF(name);
      dos.writeUTF(msg.toString());
      dos.close();

      dp.setData(bos.toByteArray(), 0, bos.size());
      DatagramSocket ds = new DatagramSocket();
      ds.send(dp);
      ds.close();
    } catch (IOException ie) {
      JServer.printDebug("LogTask: error logging for " + name + "/" + logFileType + ":", JServer.DEBUG_ERROR);
      JServer.printDebug(msg, JServer.DEBUG_ERROR);
      JServer.printDebug(ie, JServer.DEBUG_ERROR);
    }
  }


  // used internally to log the message to a file
  private void internalLog (String name, String msg, int logFileType, int lvl) {
    String newname = name;
    LogCache lc = (LogCache)cache.get(newname + "/" + logFileType);
    File logDir = null;
    switch (logFileType) {
    case Constants.MAINTENANCE_LOGFILE:
      logDir = maintLogDir;
      break;
    case Constants.AUTODOWNLOAD_LOGFILE:
      logDir = autoDlLogDir;
      break;
    default:
      JServer.printDebug("LogTask: log file type invalid for " + name + ":\n" + msg, JServer.DEBUG_ERROR);
      return;
    }

    if (lc == null) {
      if (!logDir.exists()) {
	boolean status = false;
	StringBuffer sb = new StringBuffer("LogTask: Error creating log directory");
	try {
	  status = logDir.mkdirs();
	} catch (Exception e) {
	  status = false;
	  sb.append(":\n");
	  sb.append(e.getMessage());
	}
	if (status == false) {
	  JServer.printDebug(sb.toString(), JServer.DEBUG_ERROR);
	  JServer.printDebug(name + ": \n" + msg, JServer.DEBUG_ERROR);
	  return;
	}
      }

      String file = new String(logDir + File.separator + newname);
      try {
	JServer.printDebug("LogTask: opening file (" + file + "/" + logFileType + ") for logging", JServer.DEBUG_VERBOSE);
	lc = new LogCache(new DataOutputStream(new FileOutputStream(file, true)), new Date());
      } catch (IOException ie) {
	JServer.printDebug("LogTask: error opening log file: " + file + "/" + logFileType, JServer.DEBUG_ERROR);
	JServer.printDebug(msg, JServer.DEBUG_ERROR);
	JServer.printDebug(ie, JServer.DEBUG_ERROR);
	return;
      }
      cache.put(newname + "/" + logFileType, lc);
    }

    try {
      DataOutputStream dos = lc.getDataOutputStream();
      dos.writeBytes("%" + lvl + System.currentTimeMillis() + ":" + msg + "\n");
      dos.flush();
      lc.touch();
    } catch (IOException ie) {
      JServer.printDebug("LogTask: error writing log message for " + name + "/" + logFileType, JServer.DEBUG_ERROR);
      JServer.printDebug(msg, JServer.DEBUG_ERROR);
      JServer.printDebug(ie, JServer.DEBUG_ERROR);
    }
  }


  /**
   * required by inheriting UDPServer. received UDP packets containing
   * messages to be logged, and logs them
   */
  public void UDPServerWork (DatagramSocket socket) {
    keepRunning = true;
    byte [] in = new byte [Constants.MAX_IVIEW_ISERVER_PACKET_SIZE];
    DatagramPacket rdp = null;

    while (keepRunning) {
      try {
	Arrays.fill(in, (byte)0);
	rdp = new DatagramPacket(in, in.length);

	try {
	  socket.receive(rdp);
	} catch (IOException ie) {
	  JServer.printDebug("LogTask: receive error:", JServer.DEBUG_ERROR);
	  JServer.printDebug(ie, JServer.DEBUG_ERROR);
	  continue;
	}

	LimitedDataInputStream dis = new LimitedDataInputStream(new ByteArrayInputStream(in), rdp.getLength());

	short code = dis.readShort();
	switch (code) {
	case STOP:    // to stop this thread
	  if (JServer.isSecure(dis, rdp.getAddress())) {
	    cct.cancel();
	    cleanCache(true);
	    keepRunning = false;
	  }
	  break;

	case CLEAN:
	  if (JServer.isSecure(dis, rdp.getAddress())) {
	    cleanCache(false);
	  }
	  break;

	case LOG:
	  int type = dis.readInt();
	  int lvl = dis.readInt();
	  String name = dis.readUTF();  // maintenance request name;
	  String msg = dis.readUTF();
	  internalLog(name, msg, type, lvl);
	  break;

	default:
	  JServer.printDebug("LogTask: received unknown code - " + code, JServer.DEBUG_WARNING);
	}
	dis.close();
      } catch (IOException ie) {
	if (rdp != null && rdp.getAddress() != null)
	  JServer.printDebug("LogTask receive/process (" + rdp.getAddress().toString() + ") error:", JServer.DEBUG_ERROR);
	else
	  JServer.printDebug("LogTask receive/process error:", JServer.DEBUG_ERROR);
	JServer.printDebug(ie, JServer.DEBUG_ERROR);
      }
    }

    JServer.printDebug("LogTask exited", JServer.DEBUG_NORMAL);
  }


  // a cache so that we don't have to open/close the files too often
  private class LogCache {
    private DataOutputStream os;
    private Date date;

    LogCache (DataOutputStream o, Date d) {
      os = o;
      date = d;
    }

    public DataOutputStream getDataOutputStream () {
      return os;
    }

    public synchronized Date getDate () {
      return date;
    }

    public synchronized void setDate (Date d) {
      date = d;
    }

    public void touch () {
      setDate(new Date());
    }

  }


  // close the files which have been open idly for too long
  private void cleanCache (boolean force) {
    Set s = cache.keySet();
    synchronized (cache) {
      Iterator i = s.iterator();
      while (i.hasNext()) {
	Object key = i.next();
	LogCache lc = (LogCache)cache.get(key);
	if ( force || 
	     ((System.currentTimeMillis() - lc.getDate().getTime()) >
	      CLEANUP_TIME) ) {
	  try {
	    lc.getDataOutputStream().close();
	    JServer.printDebug("LogTask: closed file handle for " + key, JServer.DEBUG_VERBOSE);
	  } catch (IOException ie) {
	    JServer.printDebug("LogTask: error closing file for " + key, JServer.DEBUG_ERROR);
	    JServer.printDebug(ie, JServer.DEBUG_ERROR);
	  }
	  i.remove();
	}
      }
    }
  }


  // triggers cleaning up of the cache periodically
  private class CacheCleanupTask extends TimerTask {
    public void run () {
      try {
	ByteArrayOutputStream bos = new ByteArrayOutputStream(128);
	DataOutputStream dos = new DataOutputStream(bos);
	dos.writeShort(CLEAN);
	ObjectOutputStream oos = new ObjectOutputStream(dos);
	oos.writeObject(dp.getAddress());
	oos.close();

	dp.setData(bos.toByteArray(), 0, bos.size());
	DatagramSocket ds = new DatagramSocket();
	ds.send(dp);
	ds.close();
      } catch (IOException ie) {
	JServer.printDebug("LogTask: error triggering cache cleanup:", JServer.DEBUG_ERROR);
	JServer.printDebug(ie, JServer.DEBUG_ERROR);
      }
    }

  }
}

