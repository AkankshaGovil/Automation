package com.nextone.common;

/**
 * this is the general exception that is thrown for any errors happening
 * in the communication between the iserver and iview
 */
public class BridgeException extends Exception {
  private String details;

  static final long serialVersionUID = -6522953088299933313L;

  public BridgeException () {
    super();
  }

  public BridgeException (String s) {
    super(s);
  }

  public BridgeException (String str, String det) {
    super(str);
    this.details = det;
  }

  public String toString () {
    if (details == null || details.length() == 0)
      return super.toString();

    StringBuffer sb = new StringBuffer(super.toString());
    sb.append("\n");
    sb.append(details);
    return sb.toString();
  }

  public String getDetails () {
    return (details == null)?"":details;
  }

  public void setDetails (String str) {
    details = str;
  }

  public void addDetails (String str) {
    details += "\n";
    details += str;
  }
}

