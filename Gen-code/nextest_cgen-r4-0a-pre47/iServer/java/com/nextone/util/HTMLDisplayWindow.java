package com.nextone.util;

import javax.swing.*;
import java.awt.*;
import java.awt.event.*;
import java.io.*;
import java.net.*;

/**
 * this class displays a HTML content in a separate frame
 */
public class HTMLDisplayWindow extends JDialog implements ActionListener {
  protected JButton close;

  public HTMLDisplayWindow (String title, URL resource) throws IOException {
    this((JFrame)null, title, resource);
  }

  public HTMLDisplayWindow (String title, URL resource, int width, int height) throws IOException {
    this((JFrame)null, title, resource, width, height, true);
  }

  public HTMLDisplayWindow (JFrame parent, String title, URL resource) throws IOException {
    this(parent, title, resource, 600, 500, true);
  }

  public HTMLDisplayWindow (JDialog parent, String title, URL resource) throws IOException {
    this(parent, title, resource, 600, 500, true);
  }

  public HTMLDisplayWindow (JDialog parent, String title, URL resource, int width, int height, boolean showClose) throws IOException {
    super(parent);
    createGUI(parent, title, resource, width, height, showClose);
  }

  public HTMLDisplayWindow (JFrame parent, String title, URL resource, int width, int height, boolean showClose) throws IOException {
    super(parent);
    createGUI(parent, title, resource, width, height, showClose);
  }

  protected void createGUI (Window parent, String title, URL resource, int width, int height, boolean showClose) throws IOException {
    setTitle(title);
        
    JEditorPane editorPane = new JEditorPane();
    editorPane.setEditable(false);
    editorPane.setPage(resource);

    JScrollPane editorScrollPane = new JScrollPane(editorPane);
    editorScrollPane.setVerticalScrollBarPolicy(JScrollPane.VERTICAL_SCROLLBAR_ALWAYS);

    FocusListener fl = SysUtil.getDefaultButtonFocusListener();
    close = new JButton("Close");
    close.setActionCommand("close");
    close.addFocusListener(fl);
    close.addActionListener(this);

    Object [] stuff = {editorScrollPane};
    Object [] btns = {close};
    if (!showClose)
      btns = new Object [0];
    JOptionPane optionPane  = new JOptionPane(stuff, JOptionPane.PLAIN_MESSAGE, JOptionPane.DEFAULT_OPTION, null, btns, null);

    setContentPane(optionPane);
    if (showClose)
      getRootPane().setDefaultButton(close);

    setDefaultCloseOperation(JDialog.DO_NOTHING_ON_CLOSE);
    addWindowListener(new WindowAdapter() {
	public void windowClosing (WindowEvent we) {
	  actionPerformed(new ActionEvent(close, 0, close.getActionCommand()));
	}
      });

    setSize(width, height);
    if (parent != null) {
      // see if we can set the window to the left of the dialog
      int x = (int)parent.getLocation().getX() - getBounds().width;
      if (x < 0)
	// maybe set it to the right of the dialog
	x = (int)parent.getLocation().getX() + parent.getSize().width;
      setLocation(x, (int)parent.getLocation().getY());
    }
  }

  public void closeWindow () {
    actionPerformed(new ActionEvent(close, 0, close.getActionCommand()));
  }

  public void actionPerformed (ActionEvent ae) {
    setVisible(false);
    dispose();
  }

}
