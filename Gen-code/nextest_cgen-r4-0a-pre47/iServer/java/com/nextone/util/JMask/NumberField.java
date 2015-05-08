package com.nextone.util.JMask;

import javax.swing.*;
import javax.swing.text.*;
import java.awt.*;
import java.awt.event.*;
import java.text.*;
import java.util.*;

public class NumberField extends JTextField {
  protected NumberFormat integerFormatter;
  protected int fieldLen,colLen;
  protected boolean strictLen;
  protected boolean grow;
  protected boolean packAncestor = true;
  protected int growLen;
  protected boolean signEnabled = false;
  protected char [] alsoAllowed;

  public NumberField (int cols, int len) {
    this(cols, true, len, null);
  }

  public NumberField (int cols, int len,boolean grow) {
    this(cols, true, len, null,grow);
  }


  public NumberField (int len) {
    this(len, false, len, null);
  }

  public NumberField (int len, boolean strictLen) {
    this(len, strictLen, len, null);
  }

  public NumberField (int cols, boolean strictLen, int len) {
    this(cols, strictLen, len, null);
  }

  public NumberField (int cols, boolean strictLen, int len, String allowedChars) {
    this(cols,strictLen,len,allowedChars,false);
  }

  public NumberField (int cols, boolean strictLen, int len, String allowedChars,boolean grow) {
    this.strictLen = strictLen;
    this.fieldLen = len;
    integerFormatter = NumberFormat.getNumberInstance();
    integerFormatter.setParseIntegerOnly(true);
    integerFormatter.setGroupingUsed(false);
    Font f = new Font("Courier", Font.PLAIN, 12);
    setFont(f);
    setColumns(cols+2);
    setAllowedChars(allowedChars);
    this.grow = grow;
    this.growLen = len;
    this.colLen = cols;
  }
	  
  public synchronized void setAllowedChars (String str) {
    if (str == null)	
      str = "";

    alsoAllowed = new char [str.length()];
    for (int i = 0; i < alsoAllowed.length; i++)
      alsoAllowed[i] = str.charAt(i);
  }

  // sets a new column size and a strict length
  public synchronized void setNewLength (int col, int len) {
    setColumns(col+2);
    setStrictLen(len);
  }

  // sets if the +/- characters are allowed
  public synchronized void setSignEnabled (boolean b) {
    signEnabled = b;
  }

  public synchronized boolean isSignEnabled () {
    return signEnabled;
  }

  public synchronized void setStrictLen (int len) {
    // validate length
    len = (len < 0)?0:len;

    // first truncate the current string
    if (getText().length() > len) {
      setText(getText().substring(0, len));
    }
    strictLen = true;
    fieldLen = len;
  }

  public synchronized void setStrictLen (boolean b) {
    strictLen = b;
  }

  public synchronized int getValue() {
    int retVal = 0;
    try {
      if (getText().length() > 0)
	retVal = integerFormatter.parse(getText()).intValue();
    } catch (ParseException e) {
      // this might happen if this is not a pure NumberField...
      //			System.err.println(this);
    }
    return retVal;
  }

  public synchronized void setValue(int value) {
    setText(integerFormatter.format(value));
  }

  protected Document createDefaultModel() {
    return new NumberDocument();
  }

  protected boolean isAllowedCharacter (char c) {
    boolean result = false;
    for (int i = 0; i < alsoAllowed.length; i++) {
      if (c == alsoAllowed[i]) {
	result = true;
	break;
      }
    }

    return result;
  }

  protected class NumberDocument extends PlainDocument {
    public void insertString(int offs, 
			     String str,
			     AttributeSet a) 
      throws BadLocationException {
      char[] source = str.toCharArray();
      char[] result = new char[source.length];
      int j = 0;
      // no more input than the field length
      if (strictLen && ((getLength()+source.length) > fieldLen)) {
	Toolkit.getDefaultToolkit().beep();
	//	Thread.dumpStack();
	return;
      }

      for (int i = 0; i < result.length; i++) {
	if (Character.isDigit(source[i]) ||
	    isAllowedCharacter(source[i]) ||
	    (signEnabled && offs == 0 && 
	     !NumberField.this.getText().startsWith("-") && 
	     !NumberField.this.getText().startsWith("+") &&
	     (source[i] == '-' || source[i] == '+')))
	  result[j++] = source[i];
	else {
	  /*
	  System.out.println("isDigit = " + Character.isDigit(source[i]));
	  System.out.println("isAllowedCharacter = " + isAllowedCharacter(source[i]));
	  System.out.println("signEnabled = " + signEnabled);
	  System.out.println("offs = " + offs);
	  System.out.println("NumberField.this.getText().startsWith(\"-\") = " + NumberField.this.getText().startsWith("-"));
	  System.out.println("NumberField.this.getText().startsWith(\"+\") = " + NumberField.this.getText().startsWith("+"));
	  System.out.println("source[" + i + "] = " + source[i]);
	  Thread.dumpStack();
	  */
	  Toolkit.getDefaultToolkit().beep();
	}
      }

      /**
       * peter doesn't like the growing :-( so we won't grow no more
       */
      /*
	if (grow) {
	int col = getColsToSet(getLength() + str.length());
	if (col > 0) {
	setColumns(col+2);
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

      super.insertString(offs, new String(result, 0, j), a);
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
	setColumns(col+2);
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

  private int getColsToSet (int curFieldLen) {

    int curCols = getColumns() - 2;

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

  /*
    public static void main (String [] args) {
    NumberField iaf = new NumberField(5);
    JFrame f = new JFrame();
    JPanel p = new JPanel();
    p.setLayout(new FlowLayout(FlowLayout.LEFT));
    p.add(new JLabel("Number:  ", JLabel.LEFT));
    p.add(iaf);
    f.getContentPane().add(p);
    f.setSize(new Dimension(200, 100));
    f.setVisible(true);
    f.setDefaultCloseOperation(JFrame.EXIT_ON_CLOSE);
    }
  */
}

