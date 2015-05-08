package com.nextone.util;

import java.net.*;
import java.io.*;


public class TCPServer implements TCPServerWorker {
  int clientsLeft = 0;
  boolean dbg = false;
  long id = 0;
  private TCPServerWorker target;
  private ServerSocket serverSocket;
  private Thread thisThread;
  private static int serverCount;
  private boolean keepRunning;
  private String threadName;

  public TCPServer () throws IOException {
    this(null, null, 10101, 0, false);  // call the const. with 10101 as port num
  }

  public TCPServer (TCPServerWorker target) throws IOException {
    this(null, target, 10101, 0, false);
  }

  public TCPServer (int portNo) throws IOException {
    this(null, null, portNo, 0, false);
  }

  public TCPServer (int portNo, boolean debug) throws IOException {
    this(null, null, portNo, 0, debug);
  }

  public TCPServer (int portNo, int times) throws IOException{
    this(null, null, portNo, times, false);
  }

  public TCPServer (ThreadGroup tg, int portNo, boolean debug) throws IOException {
    this(tg, null, portNo, 0, debug);
  }

  public TCPServer (ThreadGroup tg, int portNo, boolean debug, InetAddress addr) throws IOException {
    this(tg, null, portNo, 0, debug, addr);
  }
  public TCPServer (ThreadGroup tg, TCPServerWorker target, int portNo, int times, boolean debug) throws IOException {
    this(tg,target,portNo,times,debug,new ServerSocket(portNo));
  }
  public TCPServer (ThreadGroup tg, TCPServerWorker target, int portNo, int times, boolean debug, InetAddress addr) throws IOException {
    this(tg,target,portNo,times,debug,new ServerSocket(portNo,0,addr));

  }

  public TCPServer (ThreadGroup tg, TCPServerWorker target, int portNo, int times, boolean debug,ServerSocket socket) throws IOException {
    long count, modifier;
    dbg = debug;

    //serverSocket = new ServerSocket(portNo);
    serverSocket  =  socket;

    this.target = target;

    if (times == 0)
      modifier = -1;
    else
      modifier = 1;

    count = (modifier == 1)?0:-1;
    ThreadGroup tgrp = (tg == null)?Thread.currentThread().getThreadGroup():tg;
    new MainThread(tgrp, count, modifier, times).start();
  }

  /**
   * main thread waiting to serve clients... spawns off a different
   * thread for each client
   */
  private class MainThread extends Thread {
    private long count;
    private long modifier;
    private int times;

    MainThread (ThreadGroup tg, long c, long m, int t) {
      super(tg, "TCPServer-" + (++serverCount));
      count = c;
      modifier = m;
      times = t;
    }

    public void run () {
      thisThread = Thread.currentThread();
      if (threadName == null)
	threadName = thisThread.getName();
      keepRunning = true;
      while (keepRunning && count < times) {
	count += modifier;
	id = (modifier == 1)?(modifier*count):(modifier*count)-1;
	try {
	  new TCPServerDoWork(serverSocket.accept(), TCPServer.this).start();
	  clientsLeft++;
	} catch (InterruptedIOException iie) {
	  count = Integer.MAX_VALUE;
	  times = 0;
	} catch (IOException e) {
	  if (keepRunning)
	    System.err.println("Error accepting " + threadName + " tcp connections: " + e.toString());
	}
      }

      try {
	if (keepRunning)
	  serverSocket.close();
      } catch (IOException ie) {
	System.err.println("Error closing " + threadName + " tcp server socket:\n" + ie.toString());
      }

      if (dbg)
	System.out.println(threadName + ": exiting, currently " + clientsLeft + " clients are connected");
    }
  }


  /**
   * If a new TCP server is constructed using a separate <code>
   * TCPServerWorker</code> object, then that <code>TCPServerWorker</code>
   * object's <code>TCPServerWork</code> method is called; otherwise
   * this method does nothing and returns.
   * <p>
   * Subclasses of <code>TCPServer</code> should override this method.
   *
   * @see util.TCPServerWorker#TCPServerWork()
   */
  public void TCPServerWork (Socket sock) {
    if (target != null)
      target.TCPServerWork(sock);
  }

  public int getPort () {
    return serverSocket.getLocalPort();
  }

  public void setDebug (boolean debug) {
    dbg = debug;
  }

  // returns instance of the current thread
  public Thread getThread () {
    return thisThread;
  }

  public void setName (String name) {
    threadName = name;
  }

  // interrupts the current thread and waits until it dies
  public void stopAndWait () {
    // Due to the bug in the latest JVM (1.3) we cannot interrupt a thread
    // waiting on socket.accept(). It used to work in 1.2.2.
    // Instead the recommended way now is to call socket.close() (which does
    // not work on linux because of a JVM bug 4344135)
    //		 thisThread.interrupt();
    //		 try {
    //			thisThread.join();
    //		 } catch (InterruptedException ie) {}


    try {
      keepRunning = false;
      serverSocket.close();
    } catch (IOException ie) {
      System.err.println("Error closing " + threadName + " tcp server socket:\n" + ie.toString());
    }
    //    if (dbg)
//     System.out.println("Won't serve anymore clients (currently " + clientsLeft + " clients are connected)");
  }

}
