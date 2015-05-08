package com.nextone.common;

/**
 * this exception is thrown when the data is exists in the iserver
 */
public class ExistException extends BridgeException {

  static final long serialVersionUID = 7313909917665227628L;

  public ExistException () {
    super();
  }

  public ExistException (String s) {
    super(s);
  }

  public ExistException (String s, String d) {
    super(s, d);
  }
}
