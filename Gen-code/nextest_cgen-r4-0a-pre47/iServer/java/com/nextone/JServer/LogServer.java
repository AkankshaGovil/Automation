package com.nextone.JServer;

import java.net.*;
import java.io.*;
import java.util.zip.*;
import com.nextone.common.BridgeException;
import com.nextone.util.SysUtil;
import com.nextone.util.TCPServer;


/**
 * a TCP server which serves the contacting iview clients with the log files
 * of the requested maintenance requests
 */
public class LogServer extends TCPServer {
  private BridgeServer bs;

  LogServer (ThreadGroup tg, BridgeServer bs, boolean debug) throws IOException {
    super(tg, Constants.JSERVER_LOGFILE_SERVER_PORT, debug);
    init(bs);
  }

  LogServer (ThreadGroup tg, BridgeServer bs, boolean debug, InetAddress mgmtIp) throws IOException {
    super(tg, Constants.JSERVER_LOGFILE_SERVER_PORT, debug,mgmtIp);
    init(bs);
  }

  private void init(BridgeServer bs){
    this.bs  =  bs;
  }

  // this function will be called from different threads, one for
  // each client connecting to get the information
  public void TCPServerWork (Socket sock) {
    String client = sock.getInetAddress().toString();
    JServer.printDebug("client from " + client + " requesting log, compression = " + JServer.compression, JServer.DEBUG_NORMAL);

    try {
      sock.setSoTimeout(5000); // wait a max of 5 seconds
    } catch (SocketException se) {
      JServer.printDebug("LogServer: setSoTimeout:", JServer.DEBUG_WARNING);
      JServer.printDebug(se, JServer.DEBUG_WARNING);
    }

    // get the input/output streams
    ObjectInputStream ois = null;
    try {
      if (JServer.compression == 0)
        ois = new ObjectInputStream(sock.getInputStream());
      else
        ois = new ObjectInputStream(new GZIPInputStream(new BufferedInputStream(sock.getInputStream())));
    } catch (IOException ie) {
      JServer.printDebug("LogServer: getInputStream:", JServer.DEBUG_ERROR);
      JServer.printDebug(ie, JServer.DEBUG_ERROR);
      try {
	sock.close();
      } catch (IOException iie) {
	JServer.printDebug("LogServer: sock.close()1: ", JServer.DEBUG_WARNING);
	JServer.printDebug(iie, JServer.DEBUG_WARNING);
      }
      return;
    }

    BufferedOutputStream bos = null;
    GZIPOutputStream gos = null;
    ObjectOutputStream oos = null;
    try {
      bos = new BufferedOutputStream(sock.getOutputStream());
      if (JServer.compression == 0)
        oos = new ObjectOutputStream(bos);
      else {
        gos = new GZIPOutputStream(bos);
        oos = new ObjectOutputStream(gos);
      }
    } catch (IOException ie) {
      JServer.printDebug("LogServer: getOutputStream:", JServer.DEBUG_ERROR);
      JServer.printDebug(ie, JServer.DEBUG_ERROR);
      try {
	sock.close();
      } catch (IOException iie) {
	JServer.printDebug("LogServer: sock.close()2:", JServer.DEBUG_WARNING);
	JServer.printDebug(iie, JServer.DEBUG_ERROR);
      }
      return;
    }

    while (true) {
      Object in = null;
      try {
	in = ois.readObject();
      } catch (InterruptedIOException iioe) {
	JServer.printDebug("LogServer: timed out waiting for a request from " + client, JServer.DEBUG_WARNING);
	break;
      } catch (Exception e) {
	JServer.printDebug("LogServer: error reading object:", JServer.DEBUG_ERROR);
	JServer.printDebug(e, JServer.DEBUG_ERROR);
	// send the error back to the source
	try {
	  oos.writeObject(new BridgeException(e.toString()));
	} catch (Exception ee) {
	  JServer.printDebug("LogServer: cannot send error message1:", JServer.DEBUG_WARNING);
	  JServer.printDebug(ee, JServer.DEBUG_WARNING);
	}
	break;
      }

      // validate the input object
      Class c = in.getClass();
      if (!c.equals(Integer.class)) {
	JServer.printDebug("LogServer: " + client + " sent invalid request (" + c.toString() + ")", JServer.DEBUG_WARNING);
	try {
	  oos.writeObject(new BridgeException("software mismatch: don't recognize the request"));
	} catch (Exception ee) {
	  JServer.printDebug("LogServer: cannot send error message2:", JServer.DEBUG_WARNING);
	  JServer.printDebug(ee, JServer.DEBUG_WARNING);
	}
	break;
      }

      // extract the file name information
      int logType = ((Integer)in).intValue();
      if (logType != Constants.JSERVER_LOGFILE) {
	try {
	  in = ois.readObject();
	} catch (InterruptedIOException iioe) {
	  JServer.printDebug("LogServer: timed out waiting for the file name from " + client, JServer.DEBUG_WARNING);
	  break;
	} catch (Exception e) {
	  JServer.printDebug("LogServer: error reading file name object:", JServer.DEBUG_ERROR);
	  JServer.printDebug(e, JServer.DEBUG_ERROR);
	  try {
	    oos.writeObject(new BridgeException(e.toString()));
	  } catch (Exception ee) {
	    JServer.printDebug("LogServer: cannot send error message3:", JServer.DEBUG_WARNING);
	    JServer.printDebug(ee, JServer.DEBUG_WARNING);
	  }
	  break;
	}

	// validate the input object
	c = in.getClass();
	if (!c.equals(String.class)) {
	  JServer.printDebug("LogServer: " + client + " sent invalid request (" + c.toString() + ")", JServer.DEBUG_WARNING);
	  try {
	    oos.writeObject(new BridgeException("software mismatch: don't recognize the request"));
	  } catch (Exception ee) {
	    JServer.printDebug("LogServer: cannot send error message4:", JServer.DEBUG_WARNING);
	    JServer.printDebug(ee, JServer.DEBUG_WARNING);
	  }
	  break;
	}
      }

      File f = null;
      try {
	switch (logType) {
	case Constants.JSERVER_LOGFILE:
	  f = JServer.getLogFile();
	  break;
	case Constants.MAINTENANCE_LOGFILE:
	  f = bs.getLogFile((String)in);
	  break;
	case Constants.AUTODOWNLOAD_LOGFILE:
	  f = bs.getAutoDownloadLogFile((String)in);
	  break;
	default:
	  throw new BridgeException("request not recognized");
	}
	JServer.printDebug("LogServer: reading log file (" + f + ")", JServer.DEBUG_VERBOSE);
	oos.writeObject(new Boolean(true));  // indicate that we have a file
      } catch (Exception e) {
	JServer.printDebug("LogServer: cannot serve " + client + "'s request:", JServer.DEBUG_ERROR);
	JServer.printDebug(e, JServer.DEBUG_ERROR);
	try {
	  oos.writeObject(new BridgeException(e.getMessage()));
	} catch (Exception ee) {
	  JServer.printDebug("LogServer: cannot send error message5:", JServer.DEBUG_WARNING);
	  JServer.printDebug(ee, JServer.DEBUG_WARNING);
	}
	break;
      }

      try {
	oos.writeLong(f.length());
	SysUtil.copyStream(new BufferedInputStream(new FileInputStream(f)), oos);
      } catch (IOException ie) {
	JServer.printDebug("LogServer: error sending data:", JServer.DEBUG_ERROR);
	JServer.printDebug(ie, JServer.DEBUG_ERROR);
	try {
	  oos.writeObject(new BridgeException("LogServer: error sending data: " + ie.getMessage()));
	} catch (Exception ee) {
	  JServer.printDebug("LogServer: cannot send error message6:", JServer.DEBUG_WARNING);
	  JServer.printDebug(ee, JServer.DEBUG_WARNING);
	}
	break;
      }

      break;  // out of while-true
    }

    // close i/o streams and socket
    try {
      if (gos != null)
        gos.finish();
      bos.flush();
    } catch (IOException ie) {
      JServer.printDebug("LogServer: error flushing output stream:", JServer.DEBUG_WARNING);
      JServer.printDebug(ie, JServer.DEBUG_WARNING);
    }
    try {
      sock.close();
    } catch (IOException ie) {
      JServer.printDebug("LogServer: error closing socket:", JServer.DEBUG_WARNING);
      JServer.printDebug(ie, JServer.DEBUG_WARNING);
    }
  }


  // stops this server
  public void stop () {
    super.stopAndWait();
    JServer.printDebug("LogServer exited", JServer.DEBUG_NORMAL);
  }

}


