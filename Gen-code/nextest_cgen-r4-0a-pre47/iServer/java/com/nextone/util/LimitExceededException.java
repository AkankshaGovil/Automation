package com.nextone.util;

/**
 * exception thrown when some limit is exceeded
 */
public class LimitExceededException extends RuntimeException {
	  public LimitExceededException () {
		 super();
	  }

	  public LimitExceededException (String msg) {
		 super(msg);
	  }
}
