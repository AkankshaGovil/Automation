package com.nextone.util;

import java.io.*;

public class CloneableDataInputStream extends DataInputStream implements Cloneable {

	  public CloneableDataInputStream (InputStream is) {
		 super(is);
	  }

	  public Object clone () throws CloneNotSupportedException {
		 return super.clone();
	  }
}


