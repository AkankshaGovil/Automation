package com.nextone.util;

import java.net.*;
import java.io.*;


class TCPServerDoWork extends Thread {
	  private Socket sock = null;
	  private TCPServer serv;
	  private long id;

	  TCPServerDoWork (Socket connSocket, TCPServer serv) {
		 super("TCPServerDoWork" + serv.id);
		 this.sock = connSocket;
		 this.serv = serv;
		 this.id = serv.id;
	  }

	  public void run() {
		 serv.TCPServerWork(sock);
//		 if (serv.dbg)
//			System.out.println("Client #" + id + " is finished");
		 serv.clientsLeft--;
		 try {
			sock.close();
		 } catch (IOException ioe) {
			System.err.println("trouble closing client " + id + " " + ioe);
		 }
	  }
}

