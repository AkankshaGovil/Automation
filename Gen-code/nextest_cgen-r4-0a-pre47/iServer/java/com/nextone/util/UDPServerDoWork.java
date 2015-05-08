package com.nextone.util;

import java.net.*;
import java.io.*;


class UDPServerDoWork extends Thread {
	  private DatagramSocket sock = null;
	  private UDPServer serv;

	  UDPServerDoWork (ThreadGroup tg, DatagramSocket socket, UDPServer serv) {
		 super(tg, "UDPServerDoWork");
		 this.sock = socket;
		 this.serv = serv;
	  }

	  UDPServerDoWork (DatagramSocket socket, UDPServer serv) {
		 super("UDPServerDoWork");
		 this.sock = socket;
		 this.serv = serv;
	  }

	  public void run() {
		 serv.UDPServerWork(sock);
	  }
}

