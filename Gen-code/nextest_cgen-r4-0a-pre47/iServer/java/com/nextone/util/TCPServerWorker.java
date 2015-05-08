package com.nextone.util;

/**
 * The <code>TCPServerWorker</code> interface should be implemented by any
 * class whose instances are intended to execute as a TCP server. The
 * class must define a method called <code>TCPServerWork</code> with <code>
 * Socket</code> as the argument.
 * <p>
 * This interface is designed to provide an easy way to write TCP server
 * programs and takes away the need to explicitly use threads in the
 * program.
 *
 * @author  Santosh Mallesan
 * @version 1.0 01/13/99
 * @see     util.TCPServer
 */

import java.net.*;

public interface TCPServerWorker {

	  /**
	   * When an object implementing interface <code>TCPServerWorker</code>
	   * is used to create a TCP server, the code for the work need to be
	   * done is put inside the <code>TCPServerWork</code> method.
	   */
	  public abstract void TCPServerWork (Socket sock);
}

