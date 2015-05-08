package com.nextone.util;

import java.io.ByteArrayOutputStream;
import java.io.IOException;
import java.net.DatagramPacket;
import java.net.DatagramSocket;

/**
 * the class has a datagram packet in the underlying layer, and sends the
 * packet over the network whenever data is written into the stream
 * (need to call flush() or close() for the packet to be sent out)
 */

public class DatagramOutputStream extends ByteArrayOutputStream {
	  protected DatagramPacket dp;
	  protected DatagramSocket ds;

	  /**
	   * @param ds the datagram socket over which to send the packet
	   * @param dp the datagram packet to be sent
	   */
	  public DatagramOutputStream (DatagramSocket ds, DatagramPacket dp) {
		 super(dp.getLength());
		 this.ds = ds;
		 this.dp = dp;
	  }

	  /**
	   * sends out any data written to the stream, and then closes the stream
	   */
	  public void close () throws IOException {
		 sendPacket();
		 super.close();
	  }

	  /**
	   * sends out any data written to the stream so far
	   */
	  public void flush () throws IOException {
		 sendPacket();
		 super.flush();
	  }

	  private void sendPacket () throws IOException {
		 if (count > 0) {
			byte [] ba = toByteArray();
			dp.setData(ba);
			dp.setLength(ba.length);
			ds.send(dp);
			reset();
		 }
	  }
}
