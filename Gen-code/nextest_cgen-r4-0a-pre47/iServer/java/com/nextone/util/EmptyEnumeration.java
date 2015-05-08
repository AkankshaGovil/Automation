package com.nextone.util;

import java.util.Enumeration;
import java.util.NoSuchElementException;

/**
 * An empty enumeration, returning this instead of null could avoid a lot
 * of trouble!
 */
public final class EmptyEnumeration implements Enumeration {

	  public boolean hasMoreElements () {
		 return false;
	  }

	  public Object nextElement () {
		 throw new NoSuchElementException("empty enumeration");
	  }

}
