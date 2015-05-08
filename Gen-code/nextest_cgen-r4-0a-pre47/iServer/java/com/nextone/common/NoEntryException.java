package com.nextone.common;

/**
 * this exception is thrown when the data is not exists in the iserver
 */
public class NoEntryException extends BridgeException {

  static final long serialVersionUID = 7313909917665227628L;

  public NoEntryException () {
    super();
  }

  public NoEntryException (String s) {
    super(s);
  }

  public NoEntryException (String s, String d) {
    super(s, d);
  }
}
