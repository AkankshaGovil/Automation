package com.nextone.util;

import java.awt.*;
import java.awt.event.*;
import javax.swing.*;
import java.util.*;

/**
 * due to a bug in swing, JDesktopPane is not scrollable. This class
 * provides the auto-scrolling funcationality
 * 
 */
public class ScrollableDesktopPane extends JDesktopPane /*implements Scrollable*/ {
	  protected Hashtable listeners = new Hashtable();
	  private int px, py;

	  public void setPreferredSize (Dimension d) {
		 super.setPreferredSize(d);
		 px = d.width;
		 py = d.height;
	  }

	  public Dimension getPreferredSize () {
		 JInternalFrame [] jif = getAllFrames();
		 int maxX = 0;
		 int maxY = 0;
		 for (int i = 0; i < jif.length; i++) {
			int x = Math.abs(jif[i].getX()) + jif[i].getWidth();
			if (x > maxX) maxX = x;
			int y = Math.abs(jif[i].getY()) + jif[i].getHeight();
			if (y > maxY) maxY = y;
		 }

		 return new Dimension(Math.max(maxX, px), Math.max(maxY, py));
	  }

	  // add a component listener to the component added
	  public void addComponent (Component comp) {
		 super.add(comp);

		 ComponentListener listener = new ComponentListener () {
				  public void componentResized (ComponentEvent ce) {
					 getParent().getParent().validate();
					 getParent().repaint();
				  }

				  public void componentMoved (ComponentEvent ce) {
					 componentResized(ce);
				  }

				  public void componentShown (ComponentEvent ce) {}
				  public void componentHidden (ComponentEvent ce) {}
			};
		 comp.addComponentListener(listener);
		 listeners.put(comp, listener);
	  }

	  // remove the component listener
	  public void removeComponent (Component comp) {
		 comp.removeComponentListener((ComponentListener)listeners.remove(comp));
		 super.remove(comp);
		 getParent().getParent().validate();
		 getParent().repaint();
	  }

  public void clearComponents () {
    Iterator it = listeners.keySet().iterator();
    while (it.hasNext()) {
      Component comp = (Component)it.next();
      comp.removeComponentListener((ComponentListener)listeners.get(comp));
    }
    listeners.clear();
  }

/*
	  public Dimension getPreferredScrollableViewportSize () {
		 return getPreferredSize();
	  }

	  public int getScrollableBlockIncrement (Rectangle r, int orientation, int direction) {
		 return (orientation == SwingConstants.VERTICAL)?r.height:r.width;
	  }

	  public boolean getScrollableTracksViewportWidth () {
		 if (getParent() instanceof JViewport) {
			return (((JViewport)getParent()).getWidth() > getPreferredSize().width);
		 }
		 return false;
	  }

	  public boolean getScrollableTracksViewportHeight() {
		 if (getParent() instanceof JViewport) {
			return (((JViewport)getParent()).getHeight() > getPreferredSize().height);
		 }
		 return false;
	  }

	  public int getScrollableUnitIncrement (Rectangle r, int orientation, int direction) {
		 return 1;
	  }
*/
}
