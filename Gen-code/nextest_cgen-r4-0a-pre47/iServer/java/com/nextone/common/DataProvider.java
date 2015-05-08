package com.nextone.common;

import java.io.*;

/**
 * interface to retrieve iEdge data
 */
public interface DataProvider {
	  /**
	   * returns the iedge data
	   */
	  public Object getData (short cmd);
}

