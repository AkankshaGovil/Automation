package com.nextone.common;

import java.io.IOException;

/**
 * interface to retrieve the actual configurations
 */
public interface ConfigRetriever {
	  /**
	   * retrieves the configuration for the given code from the iedge
	   *
	   * @param code the code for the configuration to be retrieved
	   * @param dc the DataConsumer which is going to consume the configuration
	   *
	   * @return true if retrieval was successful, false otherwise
	   */
	  public boolean retrieve (int code, DataConsumer dc) throws IOException;
}
