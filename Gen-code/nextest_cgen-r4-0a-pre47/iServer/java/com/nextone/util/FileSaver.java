/**
 * FileSaver.java  - a util interface which works in conjunction
 * with SaveFileListener to save bytes to a file
 */
package com.nextone.util;

import java.io.*;

public interface FileSaver {
	  // this method will be called by the SaveFileListener to
	  // write the data on to the file
	  public void writeData (FileOutputStream fos) throws IOException;

}


	  
