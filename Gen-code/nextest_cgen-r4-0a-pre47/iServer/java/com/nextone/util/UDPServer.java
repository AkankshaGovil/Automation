/*
 * @(#)UDPServer.java
 *
 */

package com.nextone.util;

/**
 * A <i>UDPServer<i> implements the code for a server using UDP as the
 * transport layer protocol. 
 * <p>
 * There are two ways to create a new UDP server. One is to declare a class
 * to be a subclass of <code>UDPServer</code>. This subclass should override
 * the <code>UDPServerWork</code> method of class <code>UDPServer</code>. An
 * instance of the subclass can then be allocated and started. For example,
 * a UDP server that prints out the incoming strings could be written as
 * follows:
 * <p><hr><blockquote><pre>
 *      public class PrintStringServer extends UDPServer {
 *          
 *          PrintStringServer (int port) {
 *              super(port);
 *          }
 *
 *          public void UDPServerWork (java.net.DatagramSocket socket) { 
 *              // prints the incoming string
 *              byte [] buf = new byte[256];
 *              DatagramPacket rd = new DatagramPacket(buf, 256);
 *              try {
 *                  sock.receive(rd);
 *              } catch (IOException ie) {
 *                  System.err.println("receive error: " + ie);
 *              }
 *              System.out.println("received: " + new String(rd.getData()));
 *          }
 *      }
 * </pre></blockquote><hr>
 * <p>
 * The following code would then create the server and start it running:
 * <p><blockquote><pre>
 *      PrintStringServer s = new PrintStringServer(10101);
 * </pre></blockquote>
 * <p>
 * The other way to create a UDP server is to declare a class that implements
 * the <code>UDPServerWorker</code> interface. The class then implements
 * the <code>UDPServerWork</code> method. An instance of the class can
 * then be allocated, passed as an argument when creating and starting a
 * <code>UDPServer</code>. The same example in this other style looks like
 * the following:
 * <p><hr><blockquote><pre>
 *      public class PrintStringServer implements UDPServerWorker {
 *          
 *          PrintStringServer (int port) {
 *              UDPServer s = new UDPServer(this, port);
 *          }
 *
 *          public void UDPServerWork (java.net.DatagramSocket socket) { 
 *              // prints the incoming string
 *              byte [] buf = new byte[256];
 *              DatagramPacket rd = new DatagramPacket(buf, 256);
 *              try {
 *                  sock.receive(rd);
 *              } catch (IOException ie) {
 *                  System.err.println("receive error: " + ie);
 *              }
 *              System.out.println("received: " + new String(rd.getData()));
 *          }
 *      }
 * </pre></blockquote><hr>
 * <p>
 * The following code would then create the server and start it running:
 * <p><blockquote><pre>
 *      PrintStringServer s = new PrintStringServer(10101);
 * </pre></blockquote>
 * <p>
 *
 * @author Santosh Mallesan, sansud@yahoo.com
 * @version 1.0, 02/12/99
 * @see util.UDPServerWorker
 * @see util.UDPServer#UDPServerWork(java.net.DatagramSocket)
 */

import java.net.*;
import java.io.*;

public class UDPServer implements UDPServerWorker {
	  /* what will be run */
	  private UDPServerWorker target;
	  private UDPServerDoWork udw;

	  /**
	   * Allocates a new <code>UDPServer</code> object and starts the server.
	   * This constructor is the same as <code>UDPServer(null, null, 10101, null, Thread.NORM_PRIORITY, false)</code>.
	   *
	   * @see util.UDPServer#util.UDPServer(ThreadGroup, util.UDPServerWorker, int, DatagramSocket, int, boolean)
	   */
	  public UDPServer () throws IOException {
		 this(null, null, 10101, null, Thread.NORM_PRIORITY, false);
	  }

	  /**
	   * Allocates a new <code>UDPServer</code> object and starts the server.
	   * This constructor is the same as <code>UDPServer(null, target, 10101, null, Thread.NORM_PRIORITY, false)
	   * </code>.
	   *
	   * @param target    the object whose <code>UDPServerWork</code> method is called
	   * @see util.UDPServer#util.UDPServer(ThreadGroup, util.UDPServerWorker, int, DatagramSocket, int, boolean)
	   */
	  public UDPServer (UDPServerWorker target) throws IOException {
		 this(null, target, 10101, null, Thread.NORM_PRIORITY, false);
	  }

	  /**
	   * Allocates a new <code>UDPServer</code> object and starts the server.
	   * This constructor is the same as <code>UDPServer(null, null, port, null, Thread.NORM_PRIORITY, false)
	   * </code>.
	   *
	   * @param port    the port number where the server waits on
	   * @see util.UDPServer#util.UDPServer(ThreadGroup, util.UDPServerWorker, int, DatagramSocket, int, boolean)
	   */
	  public UDPServer (int portNo) throws IOException {
		 this(null, null, portNo, null, Thread.NORM_PRIORITY, false);
	  }

	  /**
	   * Allocates a new <code>UDPServer</code> object and starts the server.
	   * This constructor is the same as <code>UDPServer(null, target, port, null, Thread.NORM_PRIORITY, false)
	   * </code>.
	   *
	   * @param target    the object whose <code>UDPServerWork</code> method is called
	   * @param port      the port number where the server waits on
	   * @see util.UDPServer#util.UDPServer(ThreadGroup, util.UDPServerWorker, int, DatagramSocket, int, boolean)
	   */
	  public UDPServer (UDPServerWorker target, int portNo) throws IOException {
		 this(null, target, portNo, null, Thread.NORM_PRIORITY, false);
	  }

	  /**
	   * Allocates a new <code>UDPServer</code> object and starts the server.
	   * This constructor is the same as <code>UDPServer(tg, null, 0, sock, priority, false)
	   * </code>.
	   *
	   * @param tg        the thread group to belong to
	   * @param socket    the server datagram socket
	   * @param priority  the thread priority
	   * @see util.UDPServer#util.UDPServer(ThreadGroup, util.UDPServerWorker, int, DatagramSocket, int, boolean)
	   */
	  public UDPServer (ThreadGroup tg, DatagramSocket socket, int priority) throws IOException {
		 this(tg, null, 0, socket, priority, false);
	  }

	  /**
	   * Allocates a new <code>UDPServer</code> object and starts the server.
	   * This constructor is the same as <code>UDPServer(tg, null, 0, sock, priority, false)
	   * </code>.
	   *
	   * @param tg        the thread group to belong to
	   * @param socket    the server datagram socket
	   * @param priority  the thread priority
	   * @param name      the thread name
	   * @see util.UDPServer#util.UDPServer(ThreadGroup, util.UDPServerWorker, int, DatagramSocket, int, boolean)
	   */
	  public UDPServer (ThreadGroup tg, DatagramSocket socket, int priority, String name) throws IOException {
		 this(tg, null, 0, socket, priority, false);
		 udw.setName(name);
	  }

	  /**
	   * Allocates a new <code>UDPServer</code> object and starts the server.
	   * This constructor is the same as <code>UDPServer(tg, null, 0, sock, priority, isDaemon)
	   * </code>.
	   *
	   * @param tg        the thread group to belong to
	   * @param socket    the server datagram socket
	   * @param priority  the thread priority
	   * @param name      the thread name
	   * @param isDaemon  whether to set the threads as daemon
	   * @see util.UDPServer#util.UDPServer(ThreadGroup, util.UDPServerWorker, int, DatagramSocket, int, boolean)
	   */
	  public UDPServer (ThreadGroup tg, DatagramSocket socket, int priority, String name, boolean isDaemon) throws IOException {
		 this(tg, null, 0, socket, priority, isDaemon);
		 udw.setName(name);
	  }

	  /**
	   * Allocates a new <code>UDPServer</code> object and starts the server.
	   * This constructor is the same as <code>UDPServer(null, null, 0, sock, priority, false)
	   * </code>.
	   *
	   * @param socket    the server datagram socket
	   * @param priority  the thread priority
	   * @see util.UDPServer#util.UDPServer(ThreadGroup, util.UDPServerWorker, int, DatagramSocket, int, boolean)
	   */
	  public UDPServer (DatagramSocket socket, int priority) throws IOException {
		 this(null, null, 0, socket, priority, false);
	  }

	  /**
	   * Allocates a new <code>UDPServer</code> object and starts the server.
	   * This constructor is the same as <code>UDPServer(null, target, 0, sock, Thread.NORM_PRIORITY, false)
	   * </code>.
	   *
	   * @param socket    the server datagram socket
	   * @param target    the object whose <code>UDPServerWork</code> method is called
	   * @see util.UDPServer#util.UDPServer(ThreadGroup, util.UDPServerWorker, int, DatagramSocket, int, boolean)
	   */
	  public UDPServer (UDPServerWorker target, DatagramSocket socket) throws IOException {
		 this(null, target, 0, socket, Thread.NORM_PRIORITY, false);
	  }

	  /**
	   * Allocates a new <code>UDPServer</code> object and starts the server.
	   * 
	   * @param tg        the thread group to belong to
	   * @param target    the object whose <code>UDPServerWork</code> method is called
	   * @param port      the port number where the server waits on
	   * @param socket    the server datagram socket
	   * @param priority  the thread priority
	   * @param isDaemon  whether to set the threads as daemon
	   * @see util.UDPServerWorker#UDPServerWork(java.net.DatagramSocket)
	   * @see util.UDPServer#UDPServerWork(java.net.DatagramSocket)
	   */
	  public UDPServer (ThreadGroup tg, UDPServerWorker target, int portNo, DatagramSocket socket, int priority, boolean isDaemon) throws IOException {
		 DatagramSocket serverSocket;

		 if (socket == null)
			serverSocket = new DatagramSocket(portNo);
		 else
			serverSocket = socket;

		 this.target = target;

		 if (tg != null)
			udw = new UDPServerDoWork(tg, serverSocket, this);
		 else
			udw = new UDPServerDoWork(serverSocket, this);

		 udw.setDaemon(isDaemon);
		 udw.setPriority(priority);
	  }

	  /**
	   * If this UDP server was constructed using a separate <code>UDPServerWorker</code>
	   * run object, then that <code>UDPServerWorker</code> object's 
	   * <code>UDPServerWork</code> method is called; otherwise, this method
	   * does nothing and returns.
	   * <p>
	   * Subclasses of <code>UDPServer</code> should override this method.
	   *
	   * @param socket   the server's socket on which to send and recieve data
	   * @see util.UDPServer#util.UDPServer(util.UDPServerWorker, int, int)
	   * @see util.UDPServerWorker#UDPServerWork(java.net.DatagramSocket)
	   */
	  public void UDPServerWork (DatagramSocket socket) {
		 if (target != null)
			target.UDPServerWork(socket);
	  }

	  /**
	   * return the threadgroup to which this server thread belongs to
	   */
	  public ThreadGroup getThreadGroup () {
		 return udw.getThreadGroup();
	  }

	  /**
	   * return the server thread
	   */
	  public Thread getThread () {
		 return udw;
	  }

	  /**
	   * start running the server
	   */
	  public void start () throws IllegalThreadStateException {
		 udw.start();
	  }

}

