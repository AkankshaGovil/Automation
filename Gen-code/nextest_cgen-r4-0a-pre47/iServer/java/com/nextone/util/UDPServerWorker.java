/*
 * @(#)UDPServerWorker.java
 *
 */

package com.nextone.util;

/**
 * The <code>UDPServerWorker</code> interface should be implemented by
 * any class whose instances are intended to be executed as a UDP server.
 * The class must define a method called <code>UDPServerWork</code> with 
 * <code>java.net.DatagramSocket</code> as an argument.
 * <p>
 * <code>UDPServerWorker</code> provides the means for a class to be a UDP
 * server while not subclassing <code>UDPServer</code>. A class that implements
 * <code>UDPServerWorker</code> can run without subclassing <code>UDPServer</code>
 * by instantiating a <code>UDPServer</code> instance and passing itself in
 * as the target.
 *
 * @author Santosh Mallesan, sansud@yahoo.com
 * @version 1.0, 02/12/99
 * @see util.UDPServer
 * @see util.UDPServerWorker#UDPServerWork(java.net.DatagramSocket)
 */

public interface UDPServerWorker {

	  /**
	   * When an object implementing interface <code>UDPServerWorker</code>
	   * is used to create a UDP server, creating the <code>UDPServer</code>
	   * object with itself as the argument causes the object's 
	   * <code>UDPServerWork</code> method to be executed in that UDP server.
	   *
	   * @param socket   the server's socket on which to send and recieve data
	   * @see util.UDPServer#util.UDPServer(util.UDPServerWorker, int, int)
	   * @see util.UDPServer#UDPServerWork(java.net.DatagramSocket)
	   */
	  public abstract void UDPServerWork (java.net.DatagramSocket sock);
}

