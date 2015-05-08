package com.nextone.util;

import javax.swing.Timer;
import java.awt.event.*;

/**
 * this Timer class is exactly the same as the javax.swing.Timer class,
 * except that this allows you to set the action string (retrieved using
 * getActionCommand()) that will be used when the timer sends ActionEvents
 */

public class ActionableTimer extends javax.swing.Timer {
	  private String actionString;

	  public ActionableTimer (int delay, ActionListener al, String actionCommand) {
		 super(delay, al);
		 this.actionString = actionCommand;
	  }

	  /**
	   * override the super's method to send an ActionEvent with the
	   * correct action command
	   */
	  protected void fireActionPerformed (ActionEvent ae) {
		 super.fireActionPerformed(new ActionEvent(ae.getSource(), ae.getID(), actionString, ae.getModifiers()));
	  }
}
