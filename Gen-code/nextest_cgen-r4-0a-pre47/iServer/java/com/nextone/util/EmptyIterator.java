package com.nextone.util;

import java.util.Iterator;
import java.util.NoSuchElementException;

/**
 * An empty iterator, returning this instead of null could avoid a lot
 * of trouble!
 */
public final class EmptyIterator implements Iterator {

	  public boolean hasNext () {
		 return false;
	  }

	  public Object next () {
		 throw new NoSuchElementException("empty iterator");
	  }

	  public void remove () {
		 throw new IllegalStateException("empty iterator");
	  }

}
