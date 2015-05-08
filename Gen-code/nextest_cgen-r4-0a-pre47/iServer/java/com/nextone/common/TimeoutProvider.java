package com.nextone.common;

/**
 * interface to retrieve timeout values
 */
public interface TimeoutProvider {
  /**
   * return the number of seconds to wait for a GET operation
   */
  public int getGetTimeout ();

  /**
   * return the number of seconds to wait for a SET operation
   */
  public int getSetTimeout ();
}
