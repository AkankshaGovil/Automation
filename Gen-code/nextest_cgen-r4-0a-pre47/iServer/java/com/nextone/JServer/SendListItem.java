package com.nextone.JServer;

import java.net.*;
import java.io.*;

/**
 * this class is used in the JNI calls to send a java object over
 * a socket stream
 */
public class SendListItem {
  private ObjectOutputStream oos;

  SendListItem (ObjectOutputStream oos) {
    this.oos = oos;
  }

  public void send (Object o) throws IOException {
    oos.writeObject(o);
    oos.reset();
  }

  public void flush () throws IOException {
      oos.flush();
  }

}

