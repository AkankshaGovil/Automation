package com.nextone.util;

import java.net.*;
import java.io.*;

/** 
 * A utility class which provides convenient methods for handling multiple
 * interfaces when sending/receiving multicast messages.
 *
 * @author SM
 *
 */
public class NextoneMulticastSocket extends java.net.MulticastSocket {
//  private InetAddress [] allIf;
  private boolean debug = false;

  /**
   * simply calls the super constructor
   *
   * @exception IOException when super constructor throws IOException
   * @see java.net.MulticastSocket#MulticastSocket ()
   */
  public NextoneMulticastSocket () throws IOException {
    super();
//    listInterfaces();
  }

  /**
   * simply calls this(port, null)
   *
   * @param port the port for this socket
   *
   * @exception IOException when super constructor throws IOException
   * @see java.net.MulticastSocket#MulticastSocket (int port)
   */
  public NextoneMulticastSocket (int port) throws IOException {
    this(port, null);
  }

  /**
   * simply calls the super(port) constructor
   *
   * @param port the port for this socket
   * @param ifAddresses an InetAddress [] that contains the IP addresses of all the
   * interfaces on the host, if it is null, then the interfaces will be listed using
   * InetAddress.getAllByName(InetAddress.getLocalHost().getHostName())
   *
   * @exception IOException when super constructor throws IOException
   * @see java.net.MulticastSocket#MulticastSocket (int port)
   */
  public NextoneMulticastSocket (int port, InetAddress [] ifAddresses) throws IOException {
    super(port);
 //   if (ifAddresses == null)
 //     listInterfaces();
 //   else
 //     allIf = ifAddresses;
  }


  // lists all of host's interfaces and stores in a local array
/*
  private void listInterfaces () throws IOException {
    try {
      allIf = InetAddress.getAllByName(InetAddress.getLocalHost().getHostName());
    } catch (IOException ie) {
      throw new IOException("Error listing interfaces: " + ie.getMessage());
    }
  }
*/

  /** 
   * Overrides parent class's similar method. If destination is a 
   * multicast address, sends it on all interfaces, else simply calls
   * the parent's method. This is same as <code>sendOnAllIf (p, 1)</code>
   * 
   * @exception IOException when underlying send throws an exception
   * @param p the DatagramPacket to send
   * @see NextoneMulticastSocket#send (DatagramPacket p, byte ttl)
   */
  public void send (DatagramPacket p) throws IOException {
    sendPacket(p, (byte)1);
  }

  /** 
   * Overrides parent class's similar method. If destination is a 
   * multicast address, sends it on all interfaces, else simply calls
   * the parent's method.
   * 
   * @exception IOException when underlying send throws an exception
   * @param p the DatagramPacket to send
   * @param ttl the <i>time-to-live</i> for the packets sent out
   * @see java.net.MulticastSocket#send (DatagramPacket p, byte ttl)
   * @see NextoneMulticastSocket#sendOnAllIf (DatagramPacket p, byte ttl)
   */
  /*public void send (DatagramPacket p, byte ttl) throws IOException {
    if (p.getAddress().isMulticastAddress())
      sendOnAllIf(p, ttl);
    else
      sendPacket(p,ttl);
//      super.send(p, ttl);
  }*/

  /** 
   * Sends the multicast packet on the given interface. This is same as 
   * <code>sendOnAllIf (p, 1, intf)</code>
   * 
   * @exception IOException when underlying send throws an exception
   * @param p the DatagramPacket to send
   * @param intf the interface to send the packet on
   * @see NextoneMulticastSocket#send (DatagramPacket p, byte ttl, InetAddress intf)
   */
  public void send (DatagramPacket p, InetAddress intf) throws IOException {
    send(p, (byte)1, intf);
  }

  /** 
   * Sends the multicast packet on the given interface. 
   * 
   * @exception IOException when underlying send throws an exception
   * @param p the DatagramPacket to send
   * @param ttl the <i>time-to-live</i> for the packets sent out
   * @param intf the interface to send the packet on
   * @see java.net.MulticastSocket#send (DatagramPacket p, byte ttl)
   * @see java.net.MulticastSocket.setInterface (InetAddress addr)
   */
  public void send (DatagramPacket p, byte ttl, InetAddress intf) throws IOException {
    if (intf == null) {
      sendOnAllIf(p, ttl);
      return;
    }

    // save the current interface on the socket
    InetAddress old = getInterface();

    // set the new interface and send the packet
    if (debug)
      System.err.println("sending packet to " + p.getAddress().toString() + " on interface " + intf.toString());
    setInterface(intf);
    sendPacket(p,ttl);
//    super.send(p, ttl);

    // restore the old interface
    // (ugly hack because of some bug in linux 5.2 JRE)
 //   if (!System.getProperties().getProperty("os.version").equals("2.0.36"))
      setInterface(old);
  }

  /** 
   * Sends the DatagramPacket on all interfaces. This constructor is
   * same as <code>sendOnAllIf (p, 1)</code>
   * 
   * @exception IOException when underlying send throws an exception
   * @param p the DatagramPacket to send
   * @see NextoneMulticastSocket#sendOnAllIf (DatagramPacket p, byte ttl)
   */
  public void sendOnAllIf (DatagramPacket p) throws IOException {
    sendOnAllIf(p, (byte)1);
  }

  /** 
   * Sends the DatagramPacket on all interfaces.
   * (even if sending on one interface throws an exception, this
   * method tries all interfaces before propagating the exception)
   * 
   * @exception IOException when underlying send throws an exception
   * @param p the DatagramPacket to send
   * @param ttl the <i>time-to-live</i> for the packets sent out
   * @see java.net.MulticastSocket#send (DatagramPacket p, byte ttl)
   */
  public void sendOnAllIf (DatagramPacket p, byte ttl) throws IOException {
    // save the current interface on the socket
//    InetAddress old = getInterface();

    // For each interface in the machine send the multicast message
    int numErrors = 0;
//    Exception [] allExc = new Exception [allIf.length];
//    int [] excIndexes = new int [allIf.length];
//    for (int i = 0; i < allIf.length; i++) {
//      if (debug)
//      	System.err.println("sending packet to " + p.getAddress().toString() + " on interface " + allIf[i].toString());
      try {
//	      setInterface(allIf[i]);
        sendPacket(p,ttl);
//	      super.send(p, ttl);
      } catch (Exception e) {
	numErrors++;
//	      allExc[numErrors] = e;
//	      excIndexes[numErrors++] = i;
      }
//    }

    // restore the old interface
    // (ugly hack because of some bug in linux 5.2 JRE)
//    if (!System.getProperties().getProperty("os.version").equals("2.0.36"))
 //     setInterface(old);

    // if there were any exceptions, throw them
    StringBuffer sb = new StringBuffer();
//    for (int i = 0; i < numErrors; i++) {
//      sb.append("Interface " + allIf[excIndexes[i]] + ": " + allExc[i].toString() + "\n");
//    }
    if (numErrors > 0)
      throw new IOException(sb.toString());
  }

  /** 
   * join the multicast group on all interfaces
   * (even if joining group on one interface throws an exception, this
   * method tries all interfaces before propagating the exception)
   *
   * 
   * @exception IOException when underlying joinGroup throws an exception
   * @param addr multicast address to join
   * @see java.net.MulticastSocket#joinGroup (InetAddress addr)
   */
  public void joinGroupOnAllIf (InetAddress addr) throws IOException {
    // save the current interface on the socket
//    InetAddress old = getInterface();

    // For each interface in the machine send the multicast join message
    int numErrors = 0;
//    Exception [] allExc = new Exception [allIf.length];
//    int [] excIndexes = new int [allIf.length];
//    for (int i = 0; i < allIf.length; i++) {
//      if (debug)
//	      System.err.println("joining to group " + addr.toString() + " on interface " + allIf[i].toString());
      try {
//	      setInterface(allIf[i]);
	      super.joinGroup(addr);
      } catch (Exception e) {
	numErrors++;
//	      allExc[numErrors] = e;
//	      excIndexes[numErrors++] = i;
      }
//    }

    // restore the old interface
    // (ugly hack because of some bug in linux 5.2 JRE)
//    if (!System.getProperties().getProperty("os.version").equals("2.0.36"))
//      setInterface(old);

    // if there were any exceptions, throw them
    StringBuffer sb = new StringBuffer();
//    for (int i = 0; i < numErrors; i++) {
//      sb.append("Interface " + allIf[excIndexes[i]] + ": " + allExc[i].toString() + "\n");
//    }
    if (numErrors > 0)
      throw new IOException(sb.toString());
  }

  /** 
   * leave the multicast group on all interfaces
   * (even if leaving group on one interface throws an exception, this
   * method tries all interfaces before propagating the exception)
   *
   * @exception IOException when underlying leaveGroup throws an exception
   * @param addr multicast address to leave
   * @see java.net.MulticastSocket#leaveGroup (InetAddress addr)
   */
  public void leaveGroupOnAllIf (InetAddress addr) throws IOException {
    // save the current interface on the socket
//    InetAddress old = getInterface();

    // For each interface in the machine send the multicast leave message
    int numErrors = 0;
//    Exception [] allExc = new Exception [allIf.length];
//    int [] excIndexes = new int [allIf.length];
//    for (int i = 0; i < allIf.length; i++) {
//      if (debug)
//      	System.err.println("leaving group " + addr.toString() + " on interface " + allIf[i].toString());
      try {
//	      setInterface(allIf[i]);
	      super.leaveGroup(addr);
      } catch (Exception e) {
	numErrors++;
//	      allExc[numErrors] = e;
//	      excIndexes[numErrors++] = i;
      }
 //   }

    // restore the old interface
    // (ugly hack because of some bug in linux 5.2 JRE)
//    if (!System.getProperties().getProperty("os.version").equals("2.0.36"))
//      setInterface(old);

    // if there were any exceptions, throw them
    StringBuffer sb = new StringBuffer();
//    for (int i = 0; i < numErrors; i++) {
//      sb.append("Interface " + allIf[excIndexes[i]] + ": " + allExc[i].toString() + "\n");
//    }
    if (numErrors > 0)
      throw new IOException(sb.toString());
  }

  /**
   * turn on/off some debug printing
   *
   * @param b set debug printing on/off
   */
  public void setDebug (boolean b) {
    debug = b;
  }


  /**
   * send (DatagramPacket p, byte ttl) deprecated in 1.4
   **/

  private void sendPacket(DatagramPacket p, byte ttl) throws IOException{
    int oldTtl = getTimeToLive();
    setTimeToLive(ttl);
    super.send(p);
    setTimeToLive(oldTtl);
  }

}
