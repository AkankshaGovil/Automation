package com.nextone.common;

/**
 * this exception is thrown when an iServer does not know about the
 * operation that the iView is asking for
 * this exception is used for backward compatibility, for a newer iView
 * to detect older iServer capabilities
 */
public class OperationUnknownBridgeException extends BridgeException {

  static final long serialVersionUID = 7172776539172202874L;


  /**
   * @param oper the operation that is not known
   */
  public OperationUnknownBridgeException (int oper) {
    super("Operation not recognized by this iServer", "Error code: " + oper);
  }
}

