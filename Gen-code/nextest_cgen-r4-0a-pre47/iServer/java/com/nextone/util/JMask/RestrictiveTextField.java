package com.nextone.util.JMask;

import javax.swing.*;
import javax.swing.text.*;
import java.awt.*;
import java.awt.event.*;
import java.text.*;
import java.util.*;

/**
 * Restricts the number of characters that can be input in this text field.
 */
public class RestrictiveTextField extends JTextField {
	  protected int fieldLen, colLen;
	  protected boolean grow;
	  protected int fudgeFactor = 2;  // a default column length added to 
	                                // make certain fonts look better
	  protected int growLen;
	  protected boolean packAncestor = true;


	  /**
	   * simply calls this(cols, cols)
	   * @param cols number of columns in the text field
	   * @see RestrictiveTextField(int cols, int len)
	   */
	  public RestrictiveTextField (int cols) {
		 this(cols, cols);
	  }

	  /**
	   * simply calls this(cols, len, false)
	   * @param cols number of columns in the text field
	   * @param len  the length at which no more input will be accepted by
	   *             this text field
	   * @see RestrictiveTextField(int cols, int len, boolean grow)
	   */
	  public RestrictiveTextField (int cols, int len) {
		 this(cols, len, false);
	  }

	  /**
	   * simply calls this(cols, len, false)
	   * @param cols number of columns in the text field
	   * @param len  the length at which no more input will be accepted by
	   *             this text field
	   * @param grow if true, the field width would grow to fit the contents
	   *             of the text field (but would not shrink for less than
	   *             the initial columns requested)
	   */
	  public RestrictiveTextField (int cols, int len, boolean grow) {
		 this.colLen = cols;
		 this.fieldLen = len;
		 this.grow = grow;
		 this.growLen = fieldLen;
		 setFont(new Font("Courier", Font.PLAIN, 12));
		 setColumns(cols+fudgeFactor);
	  }

	  public synchronized boolean getGrow () {
		 return grow;
	  }

	  public synchronized void setGrow (boolean newval) {
		 grow = newval;
	  }

	  public synchronized int getGrowLength () {
		 return growLen;
	  }

	  public synchronized void setGrowLength (int newlen) throws IllegalArgumentException {
		 if (newlen < colLen || newlen > fieldLen)
			throw new IllegalArgumentException("Should be between " + colLen + " and " + fieldLen);
		 growLen = newlen;
	  }

	  public synchronized boolean getPackAncestor () {
		 return packAncestor;
	  }

	  public synchronized void setPackAncestor (boolean newval) {
		 packAncestor = newval;
	  }

	  private int getColsToSet (int curFieldLen) {
		 int curCols = getColumns() - fudgeFactor;

		 if (curFieldLen <= colLen) {
			// maybe needs shrinking
			if (curCols != colLen)
			   return colLen;
			return 0;  // no need to grow
		 }

		 // size to fit the field, upto the max growLen
		 if (curFieldLen <= growLen)
			return curFieldLen;
		 else
			return 0;  // no more growing
	  }

	  protected Document createDefaultModel() {
		 return new RestrictiveTextDocument();
	  }

	  protected class RestrictiveTextDocument extends PlainDocument {
			public void insertString(int offs, 
									 String str,
									 AttributeSet a) 
			   throws BadLocationException {

			   // no more input than the field length
			   if ((getLength()+str.length()) > fieldLen) {
				  Toolkit.getDefaultToolkit().beep();
				  return;
			   }

			   /**
				* peter doesn't like the growing :-( so we won't grow no more
				*/
/*
			   if (grow) {
				  int col = getColsToSet(getLength() + str.length());
				  if (col > 0) {
					 setColumns(col+fudgeFactor);
					 if (packAncestor) {
						Container c = getTopLevelAncestor();
						if (c != null) {
						   if (c instanceof JFrame)
							  ((JFrame)c).pack();
						   else if (c instanceof JDialog)
							  ((JDialog)c).pack();
						   else if (c instanceof JWindow)
							  ((JWindow)c).pack();
						}
					 }
				  }
			   }
*/

			   super.insertString(offs, str, a);
			}

			public void remove (int offs, int len) throws BadLocationException {
			   super.remove(offs, len);

			   /**
				* peter doesn't like the growing :-( so we won't grow no more
				*/
/*
			   if (grow) {
				  int col = getColsToSet(getLength());
				  if (col > 0) {
					 setColumns(col+fudgeFactor);
					 Container c = getTopLevelAncestor();
					 if (c != null) {
						if (c instanceof JFrame)
						   ((JFrame)c).pack();
						else if (c instanceof JDialog)
						   ((JDialog)c).pack();
						else if (c instanceof JWindow)
						   ((JWindow)c).pack();
					 }
				  }
			   }
*/
			}
	  }

}

