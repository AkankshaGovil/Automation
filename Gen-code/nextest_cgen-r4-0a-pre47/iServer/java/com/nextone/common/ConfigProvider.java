package com.nextone.common;

import java.io.*;

/**
 * interface to retrieve timeout values
 */
public interface ConfigProvider {
	  /**
	   * returns the configuration of the iedge
	   */
	  public String getConfig () throws IOException;
}

