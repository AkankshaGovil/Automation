package com.nextone.JServer;

import java.net.*;
import java.util.*;
import java.util.zip.*;
import java.io.*;
import com.nextone.util.DeltaTime;
import com.nextone.util.LimitedDataInputStream;
import com.nextone.util.NextoneMulticastSocket;
import com.nextone.util.TCPServer;
import com.nextone.util.SysUtil;
import com.nextone.common.Bridge;
import com.nextone.common.BridgeException;
import com.nextone.common.CommonConstants;
import com.nextone.common.IEdgeList;
import com.nextone.common.iServerConfig;
import com.nextone.common.Capabilities;
import com.nextone.common.MaintenanceGroup;
import com.nextone.common.MaintenanceRequest;
import com.nextone.common.OperationUnknownBridgeException;
import com.nextone.common.ProvisionData;
import com.nextone.common.RegidPortPair;
import com.nextone.common.Registration;
import com.nextone.common.SingleInstance;
import com.nextone.common.Commands;


/**
 * a server thread similar to JServer, but works with TCP instead of UDP packets
 */
public class JServerTCP extends TCPServer implements Constants {
  private Object lockObject;
  private JServer js;
  private Bridge bs;
  private iServerConfig cachedConfig;

  JServerTCP (ThreadGroup tg, boolean debug, JServer js, Bridge bs, Object lockObject) throws IOException {
    super(tg, Constants.MCAST_JSERVER_LISTEN_PORT, debug);
    init(js,bs,lockObject);
  }

  JServerTCP (ThreadGroup tg, boolean debug, JServer js, Bridge bs, Object lockObject, InetAddress mgmtAddress) throws IOException {
    super(tg, Constants.MCAST_JSERVER_LISTEN_PORT, debug,mgmtAddress);
    init(js,bs,lockObject);
  }

  private void init(JServer js, Bridge bs,Object lockObject){
    this.js = js;
    this.bs = bs;
    this.lockObject = lockObject;
  }

  /**
   * handle status request
   */
  private void handleStatusRequest (Socket sock, DataInputStream dis, OutputStream os, int permit) {
    try {
      String ret = js.getStatus(permit);

      JServer.printDebug("Connecting to: " + sock.getInetAddress().getHostAddress() + ":" + sock.getPort() + " to send status", JServer.DEBUG_VERBOSE);
      ObjectOutputStream oos = new ObjectOutputStream(os);
      oos.writeObject(ret);
    } catch (IOException ie) {
      JServer.printException(sock.getInetAddress().getHostAddress() + ": handleStatusRequest caught an error", ie, JServer.DEBUG_ERROR);
    }
  }

  /**
   * handle status request
   */
  private void handleBulkRequest (Socket sock, DataInputStream dis, OutputStream os) {
    try {
      Object ret  = null;
      try {
        ObjectInputStream ois = new ObjectInputStream(dis);
        Commands [] cmds = (Commands[])ois.readObject();
        JServer.printDebug(sock.getInetAddress().getHostAddress() + "processing commands", JServer.DEBUG_VERBOSE);
        synchronized (lockObject) {
          ret = new Boolean(bs.processBulkCommands(cmds));
        }
      } catch (Exception e) {
        JServer.printException(sock.getInetAddress().getHostAddress() + ": Error processing commands", e, JServer.DEBUG_ERROR);
        ret = e;
      }

      ObjectOutputStream oos = new ObjectOutputStream(os);
      oos.writeObject(ret);
    } catch (IOException ie) {
      JServer.printException(sock.getInetAddress().getHostAddress() + ": handleBulkRequest caught an error", ie, JServer.DEBUG_ERROR);
    }
  }

  /**
   * handle the config requests
   */
  private void handleCfgRequest (Socket s, DataInputStream dis, OutputStream os, short op) {
    try {
      String addr = s.getInetAddress().getHostAddress();
      Object ret = null;
      boolean restartJServer = false;
        switch (op) {
        case Constants.ISERVER_GET_CFG:
        synchronized (lockObject) {
          try {
            JServer.printDebug(addr + ": retrieving iserver configuration", JServer.DEBUG_VERBOSE);
            cachedConfig = bs.getIserverConfig();
            ret = cachedConfig.getXMLString();
            JServer.printDebug("JServerTCP: Sending iServer config:", JServer.DEBUG_VERBOSE);
            JServer.printDebug(ret, JServer.DEBUG_VERBOSE);
          } catch (Exception e) {
            JServer.printException(addr + ": Error retrieving iserver configuration", e, JServer.DEBUG_ERROR);
            ret = e;
          }
        }
          break;

        case Constants.ISERVER_SET_CFG:
        synchronized (lockObject) {
          try {
            ObjectInputStream ois = new ObjectInputStream(dis);
            iServerConfig ic = new iServerConfig((String)ois.readObject());
            JServer.printDebug(addr + ": updating iserver configuration", JServer.DEBUG_VERBOSE);
            JServer.printDebug("JServerTCP: iServer config received:", JServer.DEBUG_VERBOSE);
            JServer.printDebug(ic.getXMLString(), JServer.DEBUG_VERBOSE);

            if (cachedConfig == null)
              cachedConfig = bs.getIserverConfig();

            boolean r = bs.setIserverConfig(ic);

            /* if some critical parameters have changed, restart the iServer */
            if (cachedConfig.getH323Config().getInstances() != ic.getH323Config().getInstances() ||
                cachedConfig.getH323Config().getSgkMaxCalls() != ic.getH323Config().getSgkMaxCalls() ||
                cachedConfig.getH323Config().getMaxCalls() != ic.getH323Config().getMaxCalls()) {
              // reconfig just ourselves
              js.handleDebug(js.reconfig());
              JServer.executeIserverCommand("jserver reconfig");

              // restart the iserver
              ((BridgeServer)bs).restartIServer();
            } else {
              if (r) {
                js.handleDebug(js.reconfig());
                JServer.executeIserverCommand("all reconfig");
              }
            }
/*
            // if ip filter config was disabled, run the ipfconfig command
            if ((cachedConfig.getFceConfig().getFirewallName().equals("ipfilter") ||
                 cachedConfig.getFceConfig().getFirewallName().equals("NSF")) && 
                !ic.getFceConfig().getFirewallName().equals("ipfilter") &&
                !ic.getFceConfig().getFirewallName().equals("NSF")) {
              JServer.printDebug("NSF configuration disabled, removing firewall configuration from the machine", JServer.DEBUG_NORMAL);
              JServer.executeCommand("/usr/local/nextone/bin/nsfconfig -d");
            }
*/
           String oldIp  =  cachedConfig.getSystemConfig().getMgmtIp();
           String newIp  =  ic.getSystemConfig().getMgmtIp();
           if( (oldIp == null &&
                newIp != null
               ) ||
              (oldIp != null &&
               newIp != null &&
              !(oldIp.equals(newIp))
             )
            )
              restartJServer = true;

            ret = new Boolean(r);
            js.declareDbStale();
          } catch (Exception e) {
            JServer.printException(addr + ": Error updating iserver configuration", e, JServer.DEBUG_ERROR);
            ret = e;
          }
        }
          break;
        }
      //}

      // return the result object over TCP
      ObjectOutputStream oos = new ObjectOutputStream(os);
      oos.writeObject(ret);
      if(restartJServer){
        JServer.printDebug("[JServerTCP/handle cfg request] Management ip address updated. Restarting JServer.....", JServer.DEBUG_NORMAL);
        JServerMain.handleRestart();
     } 

    } catch (IOException ie) {
      JServer.printException(s.getInetAddress().getHostAddress() + ": handleCfgRequest caught an error", ie, JServer.DEBUG_ERROR);
    }
  }

  

  /**
   * handle the cap requests
   */
  private void handleCapRequest(Socket s, DataInputStream dis, OutputStream os, short op) {
    try {
      String addr = s.getInetAddress().getHostAddress();
      Object ret = null;

      synchronized (lockObject) {
        switch (op) {
        case Constants.ISERVER_GET_CAP:
          try {
            JServer.printDebug(addr + ": retrieving iserver capabilities", JServer.DEBUG_VERBOSE);
            Capabilities cap= bs.getCapabilities();
            ret = cap.getXMLString();
            JServer.printDebug("JServerTCP: Sending iServer cap:", JServer.DEBUG_VERBOSE);
            JServer.printDebug(ret, JServer.DEBUG_VERBOSE);
          } catch (Exception e) {
            JServer.printException(addr + ": Error retrieving iserver cap", e, JServer.DEBUG_ERROR);
            ret = e;
          }
          break;

        }
      }

      // return the result object over TCP
      ObjectOutputStream oos = new ObjectOutputStream(os);
      oos.writeObject(ret);
    } catch (IOException ie) {
      JServer.printException(s.getInetAddress().getHostAddress() + ": handleCapRequest caught an error", ie, JServer.DEBUG_ERROR);
    }
  }


  /**
   * handle the db requests
   */
  private void handleDbRequest (Socket s, DataInputStream dis, OutputStream os, short op) {
    try {
      String addr = s.getInetAddress().getHostAddress();
      Object ret = null;

      switch (op) {
        case Constants.ISERVER_DB_IMPORT:
          JServer.printDebug(addr + ": import db into iServer", JServer.DEBUG_VERBOSE);
          boolean isLocal = dis.readBoolean();
          int cmd = dis.readInt();
          String dbName = dis.readUTF();
          long fileSize = 0;
          File dbFile = null;
          JServer.printDebug(addr + ": import db file - " + dbName, JServer.DEBUG_VERBOSE);

          try {
            ((BridgeServer)bs).createDbDirectory();
          } catch (BridgeException be) {
            JServer.printException("Error creating db dir", be, JServer.DEBUG_ERROR);
            ret = be;
            break;
          }

          if (isLocal) {
            // read the file from iView
            fileSize = dis.readLong();
            JServer.printDebug(addr + ": import db file size is " + fileSize, JServer.DEBUG_VERBOSE);
            dbFile = new File(Constants.DB_DIR + File.separator + dbName);
            FileOutputStream fos = new FileOutputStream(dbFile);
            byte [] data = new byte [CommonConstants.MAX_IVIEW_ISERVER_PACKET_SIZE];
            int len = -1;
            long count = 0;
            while (count < fileSize && (len = dis.read(data)) != -1) {
              fos.write(data, 0, len);
              count += len;
            }
            fos.flush();
            fos.close();
          }

          synchronized (lockObject) {
            try {
              bs.setDbOperationInProgress(true);
              boolean r = bs.importDB(isLocal, cmd, dbName, dbFile);
              ret = new Boolean(r);
            } catch (Exception e) {
              JServer.printException("Error importing database", e, JServer.DEBUG_ERROR);
              ret = e;
            }
            bs.setDbOperationInProgress(false);
          }
          break;

        case Constants.ISERVER_DB_EXPORT: 
          JServer.printDebug(addr + ": export db into iServer", JServer.DEBUG_VERBOSE);
          isLocal = dis.readBoolean();
          dbName = dis.readUTF();
          int listType  = dis.readInt();

          synchronized (lockObject) {
            try {
              bs.setDbOperationInProgress(true);
              ret = bs.exportDB(isLocal, dbName,listType,null);
              if (!isLocal)
                ret = null;   // return null when the file is stored on the iserver
            } catch(Exception e) {
              JServer.printException("Error exporting database", e, JServer.DEBUG_ERROR);
              ret = e;
            }
            bs.setDbOperationInProgress(false);
          }
          break;
      }

        // return the result object over TCP
      JServer.printDebug("handleDbRequest: returning : " + ret, JServer.DEBUG_VERBOSE);
      if (ret != null && ret.getClass().equals(File.class)) {
        // send db file to the iView
        DataOutputStream dos = new DataOutputStream(os);
        ObjectOutputStream oos = new ObjectOutputStream(dos);
        oos.writeObject(ret);

        File exportFile = (File)ret;
        long fileSize = exportFile.length();
        dos.writeLong(fileSize);
        FileInputStream fis = new FileInputStream(exportFile);

        JServer.printDebug(addr + ": exporting file size " + fileSize, JServer.DEBUG_VERBOSE);
        int len = -1;
        long count = 0;
        byte [] data = new byte [CommonConstants.MAX_IVIEW_ISERVER_PACKET_SIZE];
        while (count < fileSize && (len = fis.read(data)) != -1) {
          dos.write(data, 0, len);
          count += len;
        }
        fis.close();
        if (!exportFile.delete())
          JServer.printDebug("Error deleting the exported database file " + exportFile.getName(), JServer.DEBUG_WARNING);
      } else {
        ObjectOutputStream oos = new ObjectOutputStream(os);
        oos.writeObject(ret);
      }
    } catch (IOException ie) {
      JServer.printException(s.getInetAddress().getHostAddress() + ": handleDbRequest caught an error", ie, JServer.DEBUG_ERROR);
    }
  }


  /**
   * handle maintenance group/action related requests
   */
  private void handleMaintenanceRequest (Socket s, DataInputStream dis, OutputStream os, short op) {
    try {
      String addr = s.getInetAddress().getHostAddress();
      Object ret = null;

      synchronized (lockObject) {
        switch (op) {
        case Constants.MAINTENANCE_GET_GROUP_NAMES:
          try {
            JServer.printDebug(addr + ": Listing maintenance group names", JServer.DEBUG_NORMAL);
            ret = bs.getMaintenanceGroupNames();
          } catch (Exception e) {
            JServer.printException(addr + ": Error listing maintenance group names", e, JServer.DEBUG_ERROR);
            ret = e;
          }
          break;

        case Constants.MAINTENANCE_GET_GROUP:
          String name = dis.readUTF();
          try {
            JServer.printDebug(addr + ": Retrieving maintenance group (" + name + ")", JServer.DEBUG_NORMAL);
            ret = bs.getMaintenanceGroup(name);
          } catch (Exception e) {
            JServer.printException(addr + ": Error retrieving maintenance group (" + name + ")", e, JServer.DEBUG_ERROR);
            ret = e;
          }
          break;

        case Constants.MAINTENANCE_PUT_GROUP:
          try {
            ObjectInputStream ois = new ObjectInputStream(dis);
            MaintenanceGroup mg = (MaintenanceGroup)ois.readObject();
            JServer.printDebug(addr + ": Storing maintenance group (" + mg.getName() + ")", JServer.DEBUG_NORMAL);
            boolean r = bs.storeMaintenanceGroup(mg);
            ret = new Boolean(r);
            js.declareDbStale();
          } catch (Exception e) {
            JServer.printException(addr + ": Error storing maintenance group", e, JServer.DEBUG_ERROR);
            ret = e;
          }
          break;

        case Constants.MAINTENANCE_DELETE_GROUP:
          name = dis.readUTF();
          try {
            JServer.printDebug(addr + ": deleting maintenance group (" + name + ")", JServer.DEBUG_NORMAL);
            boolean r = bs.deleteMaintenanceGroup(name);
            ret = new Boolean(r);
            js.declareDbStale();
          } catch (Exception e) {
            JServer.printException(addr + ": Error deleting maintenance group (" + name + ")", e, JServer.DEBUG_ERROR);
            ret = e;
          }
          break;

        case Constants.MAINTENANCE_GET_GROUP_NAMES_COMMENTS:
          try {
            JServer.printDebug(addr + ": Listing maintenance group names and comments", JServer.DEBUG_NORMAL);
            ret = bs.getMaintenanceGroupNamesAndComments();
          } catch (Exception e) {
            JServer.printException(addr + ": Error listing maintenance group names and comments", e, JServer.DEBUG_ERROR);
            ret = e;
          }
          break;

        case Constants.MAINTENANCE_GET_REQUEST_NAMES:
          try {
            JServer.printDebug(addr + ": Listing maintenance request names", JServer.DEBUG_NORMAL);
            ret = bs.getMaintenanceRequestNames();
          } catch (Exception e) {
            JServer.printException(addr + ": Error listing maintenance request names", e, JServer.DEBUG_ERROR);
            ret = e;
          }
          break;

        case Constants.MAINTENANCE_GET_REQUEST:
          name = dis.readUTF();
          try {
            JServer.printDebug(addr + ": Retrieving maintenance request (" + name + ")", JServer.DEBUG_NORMAL);
            ret = bs.getMaintenanceRequest(name);
          } catch (Exception e) {
            JServer.printException(addr + ": Error retrieving maintenance request (" + name + ")", e, JServer.DEBUG_ERROR);
            ret = e;
          }
          break;

        case Constants.MAINTENANCE_PUT_REQUEST:
          try {
            ObjectInputStream ois = new ObjectInputStream(dis);
            MaintenanceRequest mr = (MaintenanceRequest)ois.readObject();
            JServer.printDebug(addr + ": Storing maintenance request (" + mr.getName() + ")", JServer.DEBUG_NORMAL);
            boolean r = bs.storeMaintenanceRequest(mr);
            ret = new Boolean(r);
            js.declareDbStale();
          } catch (Exception e) {
            JServer.printException(addr + ": Error storing maintenance request", e, JServer.DEBUG_ERROR);
            ret = e;
          }
          break;

        case Constants.MAINTENANCE_DELETE_REQUEST:
          name = dis.readUTF();
          try {
            JServer.printDebug(addr + ": deleting maintenance request (" + name + ")", JServer.DEBUG_NORMAL);
            boolean r = bs.deleteMaintenanceRequest(name);
            ret = new Boolean(r);
            js.declareDbStale();
          } catch (Exception e) {
            JServer.printException(addr + ": Error deleting maintenance request (" + name + ")", e, JServer.DEBUG_ERROR);
            ret = e;
          }
          break;

        case Constants.MAINTENANCE_REQUEST_ACTIVE:
          name = dis.readUTF();
          try {
            JServer.printDebug(addr + ": checking maintenance request (" + name + ") for active", JServer.DEBUG_VERBOSE);
            boolean r = bs.isMaintenanceRequestActive(name);
            ret = new Boolean(r);
          } catch (Exception e) {
            JServer.printException(addr + ": Error checking maintenance request (" + name + ") for active", e, JServer.DEBUG_ERROR);
            ret = e;
          }
          break;

        case Constants.MAINTENANCE_REQUEST_ABORT:
          name = dis.readUTF();
          try {
            JServer.printDebug(addr + ": aborting maintenance request (" + name + ")", JServer.DEBUG_NORMAL);
            bs.abortMaintenanceRequest(name);
            ret = new Boolean(true);  // implementation side effect
          } catch (Exception e) {
            JServer.printException(addr + ": Error aborting maintenance request (" + name + ")", e, JServer.DEBUG_ERROR);
            ret = e;
          }
          break;

        case Constants.MAINTENANCE_GET_REQUEST_NAMES_COMMENTS:
          try {
            JServer.printDebug(addr + ": Listing maintenance request names and comments", JServer.DEBUG_NORMAL);
            ret = bs.getMaintenanceRequestNamesAndComments();
          } catch (Exception e) {
            JServer.printException(addr + ": Error listing maintenance request names and comments", e, JServer.DEBUG_ERROR);
            ret = e;
          }
          break;

        case Constants.MAINTENANCE_GET_LOG_NAMES:
          try {
            JServer.printDebug(addr + ": Listing maintenance log names", JServer.DEBUG_NORMAL);
            ret = bs.getMaintenanceLogNames();
          } catch (Exception e) {
            JServer.printException(addr + ": Error listing maintenance log names", e, JServer.DEBUG_ERROR);
            ret = e;
          }
          break;

        case Constants.MAINTENANCE_DELETE_LOG:
          name = dis.readUTF();
          try {
            JServer.printDebug(addr + ": deleting maintenance log (" + name + ")", JServer.DEBUG_NORMAL);
            boolean r = bs.deleteMaintenanceLog(name);
            ret = new Boolean(r);
            js.declareDbStale();
          } catch (Exception e) {
            JServer.printException(addr + ": Error deleting maintenance log (" + name + ")", e, JServer.DEBUG_ERROR);
            ret = e;
          }
          break;
        }
      }

      // return the result object over a TCP connection
      ObjectOutputStream oos = new ObjectOutputStream(os);
      oos.writeObject(ret);
    } catch (IOException ie) {
      JServer.printException(s.getInetAddress().getHostAddress() + ": handleMaintenance caught an error", ie, JServer.DEBUG_ERROR);
    }
    System.gc();
  }


  /**
   * handle auto download related requests
   */
  private void handleAutoDownload (Socket s, DataInputStream dis, OutputStream os, short op) {
    try {
      String addr = s.getInetAddress().getHostAddress();
      Object ret = null;

      synchronized (lockObject) {
        switch (op) {
        case Constants.AUTO_DOWNLOAD_GET_NAMES:
          try {
            JServer.printDebug(addr + ": Listing auto-download group names", JServer.DEBUG_NORMAL);
            ret = bs.getAutoDownloadGroupNames();
          } catch (Exception e) {
            JServer.printException(addr + ": Error listing auto-download group names", e, JServer.DEBUG_ERROR);
            ret = e;
          }
          break;

        case Constants.AUTO_DOWNLOAD_GET_NAMES_COMMENTS:
          try {
            JServer.printDebug(addr + ": Listing auto-download group names and comments", JServer.DEBUG_NORMAL);
            ret = bs.getAutoDownloadGroupNamesAndComments();
          } catch (Exception e) {
            JServer.printException(addr + ": Error listing auto-download group names and comments", e, JServer.DEBUG_ERROR);
            ret = e;
          }
          break;

        case Constants.AUTO_DOWNLOAD_GET_CONFIG:
          String name = dis.readUTF();
          short isGroup = dis.readShort();
          try {
            JServer.printDebug(addr + ": Retrieving auto-download configuration for " + name + "/" + isGroup, JServer.DEBUG_NORMAL);
            ret = bs.getAutoDownloadConfig(name, (isGroup == 1));
          } catch (Exception e) {
            JServer.printException(addr + ": Error retrieving auto-download configuration for " + name + "/" + isGroup, e, JServer.DEBUG_ERROR);
            ret = e;
          }
          break;

        case Constants.AUTO_DOWNLOAD_PUT_CONFIG:
          isGroup = dis.readShort();
          try {
            ObjectInputStream ois = new ObjectInputStream(dis);
            MaintenanceRequest mr = (MaintenanceRequest)ois.readObject();
            JServer.printDebug(addr + ": Storing auto-download configuration for " + mr.getName() + "/" + isGroup, JServer.DEBUG_NORMAL);
            boolean r = bs.storeAutoDownloadConfig(mr, (isGroup == 1));
            ret = new Boolean(r);
            js.declareDbStale();
          } catch (Exception e) {
            JServer.printException(addr + ": Error storing auto-download configuration", e, JServer.DEBUG_ERROR);
            ret = e;
          }
          break;

        case Constants.AUTO_DOWNLOAD_DELETE:
          name = dis.readUTF();
          isGroup = dis.readShort();
          try {
            JServer.printDebug(addr + ": deleting auto-download configuration (" + name + "/" + isGroup + ")", JServer.DEBUG_NORMAL);
            boolean r = bs.deleteAutoDownloadConfig(name, (isGroup == 1));
            ret = new Boolean(r);
            js.declareDbStale();
          } catch (Exception e) {
            JServer.printException(addr + ": Error deleting auto-download configuration (" + name + "/" + isGroup + ")", e, JServer.DEBUG_ERROR);
            ret = e;
          }
          break;

        case Constants.AUTO_DOWNLOAD_ACTIVE:
          name = dis.readUTF();
          try {
            JServer.printDebug(addr + ": checking auto-download of " + name + " for active", JServer.DEBUG_VERBOSE);
            boolean r = bs.isAutoDownloadActive(name);
            ret = new Boolean(r);
          } catch (Exception e) {
            JServer.printException(addr + ": Error checking auto-download of " + name + " for active", e, JServer.DEBUG_ERROR);
            ret = e;
          }
          break;

        case Constants.AUTO_DOWNLOAD_ABORT:
          name = dis.readUTF();
          try {
            JServer.printDebug(addr + ": aborting auto-download of " + name, JServer.DEBUG_NORMAL);
            bs.abortAutoDownload(name);
            ret = new Boolean(true);  // implementation side effect
          } catch (Exception e) {
            JServer.printException(addr + ": Error aborting auto-download of " + name, e, JServer.DEBUG_ERROR);
            ret = e;
          }
          break;

        case Constants.AUTO_DOWNLOAD_DELETE_LOG:
          name = dis.readUTF();
          try {
            JServer.printDebug(addr + ": deleting auto-download log of " + name, JServer.DEBUG_NORMAL);
            boolean r = bs.deleteAutoDownloadLogFile(name);
            ret = new Boolean(r);
            js.declareDbStale();
          } catch (Exception e) {
            JServer.printException(addr + ": Error deleting auto-download log of " + name, e, JServer.DEBUG_ERROR);
            ret = e;
          }
          break;
        }
      }

      // return the result object over a TCP connection
      ObjectOutputStream oos = new ObjectOutputStream(os);
      oos.writeObject(ret);
    } catch (IOException ie) {
      JServer.printException(s.getInetAddress().getHostAddress() + ": handleAutoDownload caught an error", ie, JServer.DEBUG_ERROR);
    }
    System.gc();
  }


  public void TCPServerWork (Socket socket) {
    try {
      if (bs.isDbOperationInProgress()) {
        JServer.printDebug(socket.getInetAddress().toString() + ": JServer TCP thread busy doing database operations, ignoring request from client", JServer.DEBUG_WARNING);
        throw new BridgeException("iServer database is busy, please try again later");
      }

      socket.setSoLinger(true, 1);
      socket.setKeepAlive(true);

      DataInputStream dis = null;
      OutputStream os = null;
      BufferedOutputStream bos = null;
      GZIPOutputStream gos = null;
      if (js.compression == 0) {
        dis = new DataInputStream(new BufferedInputStream(socket.getInputStream()));
        os = bos = new BufferedOutputStream(socket.getOutputStream());
      } else {
        dis = new DataInputStream(new GZIPInputStream(new BufferedInputStream(socket.getInputStream())));
        bos = new BufferedOutputStream(socket.getOutputStream());
        os = gos = new GZIPOutputStream(bos);
      }

      short code = dis.readShort();
      switch (code) {
      case Constants.STOP:
      case Constants.DEBUG:
      case Constants.UPTIME:
      case Constants.RECONFIG:
      case Constants.ALIVE:
      case Constants.REGISTRATION:
      case Constants.HELLO:
      case Constants.STATUS:
        JServer.printDebug(socket.getInetAddress().toString() + ": JServer TCP thread does not handle this code: " + code, JServer.DEBUG_ERROR);
        break;

      case Constants.COMMAND:
        short cmdcode = dis.readShort();
        short protVersion = dis.readShort();
        String read = dis.readUTF();
        String write = dis.readUTF();
        short op = -1;
        int permit = 0;

        if (cmdcode == Constants.PROCESS_BULK_COMMANDS ||
            cmdcode == Constants.STATUS_COMMAND) {
          permit = js.checkPermission(protVersion, read, write, socket.getInetAddress());
          if (permit == JServer.NOT_ALLOWED)
            break;
        } else {
          op = dis.readShort();
          if (!js.isOperationAllowed(protVersion, read, write, op, socket.getInetAddress())) {
            break;
          }
        }

        switch(cmdcode) {
        case Constants.PROCESS_BULK_COMMANDS:
          js.checkDatabaseMaster();
          handleBulkRequest(socket, dis, os);
          break;

        case Constants.STATUS_COMMAND:
          handleStatusRequest(socket, dis, os, permit);
          break;

        case Constants.MAINTENANCE_COMMAND:
          handleMaintenanceRequest(socket, dis, os, op);
          break;

        case Constants.AUTO_DOWNLOAD_COMMAND:
          handleAutoDownload(socket, dis, os, op);
          break;

        case Constants.CFG_COMMAND:
          handleCfgRequest(socket, dis, os, op);
          break;

        case Constants.DB_COMMAND:
          js.checkDatabaseMaster();
          handleDbRequest(socket, dis, os, op);
          break;
        case  Constants.CAP_COMMAND:
          handleCapRequest(socket, dis, os, op);
          break;

        default:
          JServer.printDebug("JServerTCP: unknown sub-command code received (" + cmdcode + ") from " + socket.getInetAddress().toString(), JServer.DEBUG_WARNING);
          throw new OperationUnknownBridgeException(cmdcode);
        }
        break;

      default:
        JServer.printDebug("JServer: unknown command code received (" + code + ") from " + socket.getInetAddress().toString(), JServer.DEBUG_WARNING);
        throw new OperationUnknownBridgeException(code);
      }

      if (gos != null)
        gos.finish();
      bos.flush();
      socket.close();
    } catch (BridgeException be) {
      JServer.printException("JServerTCP: sending bridge exception: " + be.getMessage(), be, JServer.DEBUG_ERROR);
      try {
        BufferedOutputStream bos = new BufferedOutputStream(socket.getOutputStream());
        ObjectOutputStream oos = null;
        GZIPOutputStream gos = null;
        if (js.compression == 0)
          oos = new ObjectOutputStream(bos);
        else {
          gos = new GZIPOutputStream(bos);
          oos = new ObjectOutputStream(gos);
        }
        oos.writeObject(be);
        if (gos != null)
          gos.finish();
        bos.flush();
        socket.close();
      } catch (IOException ie) {
        JServer.printException(socket.getInetAddress().getHostAddress() + ": sending bridge exception caught an error", ie, JServer.DEBUG_ERROR);
      }
    } catch (IOException ie) {
      JServer.printException("JServerTCP receive/process (" + socket.getInetAddress().toString() + ") error", ie, JServer.DEBUG_ERROR);
      try {
        socket.close();
      } catch (Exception e) {}
    }
  }


  // stops this server
  public void stop () {
    super.stopAndWait();
    JServer.printDebug("JServerTCP exited", JServer.DEBUG_NORMAL);
  }

}

