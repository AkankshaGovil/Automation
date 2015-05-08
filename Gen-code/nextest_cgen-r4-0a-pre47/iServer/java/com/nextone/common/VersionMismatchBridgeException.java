package com.nextone.common;

/**
 * this exception is thrown when an iServer realizes that the iView
 * is of an incompatible software version
 */
public class VersionMismatchBridgeException extends BridgeException {
  private String version;

  static final long serialVersionUID = 4317160408153341096L;

  /**
   * @param version the version number that the iServer expects
   */
  public VersionMismatchBridgeException (String version) {
    super("Software version mismatch: operation requires a software upgrade");
    this.version = version;
  }

  public String getVersion () {
    return version;
  }
}

