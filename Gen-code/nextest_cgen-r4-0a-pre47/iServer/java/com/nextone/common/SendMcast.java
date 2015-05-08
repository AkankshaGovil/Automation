package com.nextone.common;

import java.net.*;
import java.io.*;
import java.util.*;
import com.nextone.util.NextoneMulticastSocket;
import com.nextone.util.SysUtil;
import com.nextone.JFac.JFac;
import com.nextone.JFac.UniqueId;
import com.nextone.JUCon.JUCon;
import com.nextone.JUCon.NextoneDevice;
import com.nextone.JUCon.EdgeDevice;
import com.nextone.JUCon.Constants;
import com.nextone.JUCon.iServer.AddressInfo;
import com.nextone.JUCon.iServer.IServer;
import com.nextone.JUCon.iServer.StaticClusteredIServer;

public class SendMcast extends Thread implements Constants {
  public static final int HELLO_LIMIT = 4;
  public static final int SLEEP_LIMIT = 2000; // 2 seconds
  private NextoneMulticastSocket mcastSock;
  private boolean keepRunning = true;
  private NexToneFrame nf;
  private DatagramPacket dp = null;  // packets sent on multicast
  private boolean sendEnabled = false; // no discover on startup
  private boolean isJUCon = false;
  private boolean isJFac = false;
  private int sleepTime = 0;

  /**
   * Constructor
   * @param tg the threadgroup to belong to.
   * @param ds the NextoneMulticast socket to use.
   * @param nf the parent NextoneFrame that provides the 
   *           application specific (JUCon/JFac) info.
   */
  public SendMcast (ThreadGroup tg, NextoneMulticastSocket ds, NexToneFrame nf) {
    super(tg, "SendMcast");
    mcastSock = ds;
    this.nf = nf;
    if (nf.getID().equals(JUCon.id))
      isJUCon = true;
    else if (nf.getID().equals(JFac.id))
      isJFac = true;

    // put together the multicast packet
    try {
      byte [] d = SendMcast.createHelloData(nf.getDefaultReadPermission(), nf.getDefaultWritePermission());
      InetAddress toIp = InetAddress.getByName(CommonConstants.MCAST_ADDRESS);
      this.dp = new DatagramPacket(d, d.length, toIp, CommonConstants.MCAST_SEND_PORT);

    } catch (IOException ie) {
      nf.setStatus("error creating multicast packet\n");
      nf.setStatus(ie.toString() + "\n");
    }
  }

  /**
   * rebuild the hello packets with the new permission strings
   */
  public synchronized void rebuildHelloPacket () throws IOException {
    byte [] d = SendMcast.createHelloData(nf.getDefaultReadPermission(), nf.getDefaultWritePermission());
    synchronized (dp) {
      dp.setData(d);
    }
  }

  /**
   * fills in the permission string
   * after writing the string, it adds a pad so that 20 bytes would
   * be written to the underlying stream
   */
  public static void fillPermission (DataOutputStream ds, String s) throws IOException {
    Hello.fillPermission(ds, s);
  }

  public static byte [] createRemoteHelloData (String read, String write) throws IOException {
    return Hello.createRemoteHelloData(read, write, (short)0);
  }

  public synchronized static byte [] createHelloData (String read, String write) throws IOException {
    return Hello.createHelloData(read, write, (short)0);
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

    // when thread starts the first time, give a little bit of time before sending hellos
    try {
      Thread.sleep(2000);
    } catch (InterruptedException ie) {}

    while (keepRunning) {
      try {
	      if (isJUCon) {
	        // JUCon specific code
	        JUCon pc = (JUCon)nf;
	        Map map = pc.getRemoteDevices();
	        synchronized (map) {
	          // For each of the static (remote) devices send the ping.
	          Enumeration e = Collections.enumeration(map.values());
	          while (e.hasMoreElements()) {
	            i = (NextoneDevice)e.nextElement();
	            if (!i.isSaving()) {
		            s = i.getAddress().toString();
		            pc.appendDebug("SendMcast: hello to: " + s + ":" + CommonConstants.MCAST_SEND_PORT);
		            byte [] rem = SendMcast.createRemoteHelloData(i.getReadPermission(), i.getWritePermission());
		            mcastSock.send(new DatagramPacket(rem, rem.length, i.getAddress(), CommonConstants.MCAST_SEND_PORT));
	            }
	          }
	        }

                // Now go through the devices that have been added using their
                // registration IDs.
	        map = pc.getRegRemoteDevices();
          EdgeDevice ie = null;
	        synchronized (map) {
	          Enumeration e = Collections.enumeration(map.values());
	          while (e.hasMoreElements()) {
	            ie = (EdgeDevice)e.nextElement();

	            try {
		              if(ie.getAddress().equals(InetAddress.getByName(NULL_IPADDR))){
		                BridgeClientImpl b = null;
		                try {
		                  b = new BridgeClientImpl(ie.getServerAddress(), pc.getInstance());
		                } catch (Exception be) {
		                  nf.appendDebug("SendMcast: iServer unreachable" + be.toString()+"\n");
		                  continue;
		                }

		                IEdgeList iel	=	null;
		                try {
		                  iel = b.getIedgeParams(ie.getRegId(), -1);

		                } catch (Exception ex) {
		                  nf.appendDebug("SendMcast: Error processing iedge get params for" + ex.toString()+"\n");
		                  continue;
		                }
		                try {
		                  ie.setAddress(InetAddress.getByName(iel.getAddress()));
		                } catch (UnknownHostException he) {
		                  nf.appendDebug("SendMcast: no IP address for the device" + ie.getRegId()+"could be found.\n");
		                  continue;
		                }
		              }
		              if (!ie.isSaving()) {
		                pc.appendDebug("SendMcast: hello to: " + s + ":" + CommonConstants.MCAST_SEND_PORT);
		                byte [] rem = SendMcast.createRemoteHelloData(ie.getReadPermission(), ie.getWritePermission());
		                mcastSock.send(new DatagramPacket(rem, rem.length, ie.getAddress(), CommonConstants.MCAST_SEND_PORT));
	                }
							      
	              }
	              catch (UnknownHostException he) {
		              pc.appendDebug( "NULL_IPADDR  Constant is not defined.\n"  + he.getMessage());
		              continue;
	              }

	            }
	          }

	          s = "";
	          map = pc.getRemoteIServers();
	          synchronized (map) {
	          Enumeration e = Collections.enumeration(map.values());



            int epcount = 0;
            sleepTime = 0;
            while (e.hasMoreElements()) {
	            i = (NextoneDevice)e.nextElement();

	            if (!i.isSaving()) {
		            if (i instanceof StaticClusteredIServer) {
                  StaticClusteredIServer  sci = (StaticClusteredIServer)i;
		              Iterator it = sci.getCluster().iterator();
		              while (it.hasNext()) {
		                InetAddress inetaddr = ((AddressInfo)it.next()).addr;
		                s = inetaddr.toString();
		                pc.appendDebug("SendMcast: hello to: " + s + ":" + CommonConstants.MCAST_SEND_PORT);
		                byte [] rem = SendMcast.createRemoteHelloData(i.getReadPermission(inetaddr), i.getWritePermission(inetaddr));
		                mcastSock.send(new DatagramPacket(rem, rem.length, inetaddr, CommonConstants.MCAST_SEND_PORT));
                    epcount++;
		              }

                  InetAddress vip = sci.getVip();
                  if(vip  !=  null){
		                pc.appendDebug("SendMcast: hello to: " + vip.toString() + ":" + CommonConstants.MCAST_SEND_PORT);
		                byte [] rem = SendMcast.createRemoteHelloData(i.getReadPermission(vip), i.getWritePermission(vip));
		                mcastSock.send(new DatagramPacket(rem, rem.length, vip, CommonConstants.MCAST_SEND_PORT));
                  }
		            } else {
		              s = i.getAddress().toString();
		              pc.appendDebug("SendMcast: hello to: " + s + ":" + CommonConstants.MCAST_SEND_PORT);
		              byte [] rem = SendMcast.createRemoteHelloData(i.getReadPermission(), i.getWritePermission());
		              mcastSock.send(new DatagramPacket(rem, rem.length, i.getAddress(), CommonConstants.MCAST_SEND_PORT));
                  epcount++;
		            }

                // sleep for some time
                if(epcount > HELLO_LIMIT){
                  epcount = 0;
                  sleepTime +=  SLEEP_LIMIT;
                  try {
	                  Thread.sleep(SLEEP_LIMIT);
                  } catch (InterruptedException exp) {}
                }
	            }

	          }
	        }
	        s = "";
	      }
	      if (isJFac) {
	        // JFac specific code
	        // increment the tarnsmit cound on all the unique ids in
	        // the pool
	        UniqueId uid = ((JFac)nf).getUID();
	        Enumeration e = uid.getIdPool().elements();
	        while (e.hasMoreElements()) {
	          Object id = e.nextElement();
	          int state = uid.getState(id);
	          if (state == UniqueId.SUPPLIED_STATE ||
      		      state == UniqueId.SNO1_STATE)
	            uid.transmitting(id);
	        }
	      }

	      if (sendEnabled) {
	        synchronized (dp) {
	          nf.appendDebug("SendMcast: hello to: " + dp.getAddress().toString() + ":" + CommonConstants.MCAST_SEND_PORT);
	          mcastSock.send(dp);  // takes care of sending on all interfaces
	        }
	      }
      } catch (IOException ie) {
	      nf.appendDebug("SendMcast: " + ie.toString() + " (" + s + ")\n");
      }
      // sleep...
      sleepTime = (1000*nf.getPollInterval()) - sleepTime;
      if(sleepTime  < 0)
        sleepTime   = 0;

      try {
	      Thread.sleep(sleepTime);
      } catch (InterruptedException e) {}
    }
    mcastSock.close();
    nf.appendDebug("SendMcast exited");
  }

  public void sendEnable (boolean b) {
    if (b && !sendEnabled) {
      if (isJUCon)
	      nf.setStatus("Starting auto-discovery...\n");
      this.interrupt();
    } else if (!b && sendEnabled) {
      if (isJUCon)
	      nf.setStatus("Stopping auto-discovery...\n");
    }

    sendEnabled = b;
  }

  public boolean isSendEnabled () {
    return sendEnabled;
  }

}
