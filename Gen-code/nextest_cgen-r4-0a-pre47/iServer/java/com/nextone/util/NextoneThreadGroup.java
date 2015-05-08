package com.nextone.util;

import java.util.*;
import java.io.*;

/**
 * a ThreadGroup which prints out some additional information about
 * any of it's child's death due to uncaught exception
 */
public class NextoneThreadGroup extends ThreadGroup {
	  private PrintStream printStream;

	  /**
	   * @param name the thread group's name
	   *
	   * prints messages to System.out
	   */
	  public NextoneThreadGroup (String name) {
		 this(name, null);
	  }

	  /**
	   * @param name the thread group's name
	   * @param ps the print stream to print the messages on
	   *
	   * prints messages to System.out if ps is null
	   */
	  public NextoneThreadGroup (String name, PrintStream ps) {
		 super(name);
		 printStream = (ps == null)?System.err:ps;
	  }

	  public void uncaughtException (Thread t, Throwable e) {
		 printStream.println(new Date() + ": Thread=" + t.getName() + " Group=" + getName() + " suffered: ");
		 printStream.println(e);

		 super.uncaughtException(t, e);
	  }

}

