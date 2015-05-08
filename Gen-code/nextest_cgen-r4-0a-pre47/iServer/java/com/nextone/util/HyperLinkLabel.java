package com.nextone.util;

import javax.swing.*;
import java.util.*;
import java.awt.*;
import java.awt.event.*;

/**
 * This class creates a label that works similar to a hyperlink in browsers. When the label is
 * clicked on, an event is sent to all registred HyperLinkLabelListeners. The label text entered,
 * using setText() or in the constructor could be HTML text.
 */
public class HyperLinkLabel extends JLabel {
  protected Vector listeners = new Vector(1);

  public HyperLinkLabel () {
    super();
    addMouseListener(new LabelMouseListener());
  }

  public HyperLinkLabel (Icon image) {
    super(image);
    addMouseListener(new LabelMouseListener());
  }

  public HyperLinkLabel (Icon image, int horizontalAlignment) {
    super(image, horizontalAlignment);
    addMouseListener(new LabelMouseListener());
  }

  public HyperLinkLabel (String text) {
    super(text);
    addMouseListener(new LabelMouseListener());
  }

  public HyperLinkLabel (String text, Icon image, int horizontalAlignment) {
    super(text, image, horizontalAlignment);
    addMouseListener(new LabelMouseListener());
  }

  public HyperLinkLabel (String text, int horizontalAlignment) {
    super(text, horizontalAlignment);
    addMouseListener(new LabelMouseListener());
  }

  protected class LabelMouseListener extends MouseAdapter {
    public void mouseEntered (MouseEvent me) {
      HyperLinkLabel.this.setCursor(new Cursor(Cursor.HAND_CURSOR));
    }

    public void mouseExited (MouseEvent me) {
      HyperLinkLabel.this.setCursor(new Cursor(Cursor.DEFAULT_CURSOR));
    }

    public void mouseClicked (MouseEvent me) {
      Iterator it = listeners.iterator();
      while (it.hasNext())
	((HyperLinkLabelListener)it.next()).clicked();
    }
  }

  public void addLabelListener (HyperLinkLabelListener listener) {
    if (!listeners.contains(listener))
      listeners.add(listener);
  }

  /*
  public void removeNotify () {
    if (listeners != null) {
      listeners.clear();
      listeners = null;
    }
    super.removeNotify();
  }
  */
}
