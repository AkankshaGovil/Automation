package com.nextone.common;

/**
 * this exception is thrown when an iServer does not know about the
 * iedge that the iView is asking for
 */
public class NotProvisionedBridgeException extends BridgeException {
  protected String regid;
  protected int port;

  static final long serialVersionUID = 400082920985226149L;

  public NotProvisionedBridgeException () {
    super(Bridge.NOT_PROVISIONED);
  }

  /**
   * @param regid the regid of the iedge that is not provisioned
   * @param port the port of the iedge that is not provisioned
   */
  public NotProvisionedBridgeException (String regid, int port) {
    this();
    this.regid = regid;
    this.port = port;
  }

  public String getRegid () {
    return regid;
  }

  public int getPort () {
    return port;
  }
}

