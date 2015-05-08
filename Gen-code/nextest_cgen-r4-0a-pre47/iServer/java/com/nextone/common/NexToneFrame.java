package com.nextone.common;

import javax.swing.*;
import java.awt.*;

/**
 * defines a common set of APIs to be provided by all the nextone
 * frames. These APIs would be used in the common code shared by
 * the different tools (JUCon, JFac)
 */
public abstract class NexToneFrame extends ConsumeMouseFrame {

	  public NexToneFrame () {
		 super();
	  }

	  public NexToneFrame (String str) {
		 super(str);
	  }

	  /**
	   * default prints out on the screen, can be overridden to 
	   * provide better messages to the user
	   */
	  public void setStatus (Object ob) {
		 System.out.println(ob);
	  }

	  /**
	   * when overridden prints out debug messages every "poll" interval
	   */
	  public void printDebug () { };

	  // this returns the multicast interval for the send thread
	  public abstract int getPollInterval ();

	  /**
	   * when overridden appends debug messages to a convenient place
	   */
	  public void appendDebug (String s) {};

	  /**
	   * returns the default read permission string
	   */
	  public String getDefaultReadPermission () {
		 return "";
	  }

	  /**
	   * returns the default write permission string
	   */
	  public String getDefaultWritePermission () {
		 return "";
	  }

	  /**
	   * identifying string for this provider
	   */
	  public abstract String getID ();

}

