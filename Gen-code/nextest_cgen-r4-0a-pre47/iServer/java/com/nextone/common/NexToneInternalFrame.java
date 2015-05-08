package com.nextone.common;

import javax.swing.*;
import javax.swing.border.*;
import javax.swing.event.*;
import java.awt.*;
import java.awt.event.*;
import java.beans.*;
import com.nextone.util.ScrollableDesktopPane;

/**
 * defines a common set of APIs to be provided by all the nextone
 * frames. These APIs would be used in the common code shared by
 * the different tools (JUCon, JFac, JProCon)
 */
public class NexToneInternalFrame extends JInternalFrame {
  protected Border border;
  protected MouseAdapter ma;
  protected MouseListener gpml;
  protected MouseMotionListener gpmml;
  protected ComponentListener cl;
  protected InternalFrameAdapter ifa;
  protected int currentCursor = Cursor.DEFAULT_CURSOR;

  public NexToneInternalFrame () {
    super();
    doNexToneSpecifics();
    setMaximizable(true);
  }

  public NexToneInternalFrame (String title) {
    super(title);
    doNexToneSpecifics();
    setMaximizable(true);
  }

  public NexToneInternalFrame (String title, boolean resizable) {
    super(title, resizable);
    doNexToneSpecifics();
    setMaximizable(true);
  }

  public NexToneInternalFrame (String title, boolean resizable, boolean closable) {
    super(title, resizable, closable);
    doNexToneSpecifics();
    setMaximizable(true);
  }

  public NexToneInternalFrame (String title, boolean resizable, boolean closable, boolean maximizable) {
    super(title, resizable, closable, maximizable);
    doNexToneSpecifics();
  }

  public NexToneInternalFrame (String title, boolean resizable, boolean closable, boolean maximizable, boolean iconifiable) {
    super(title, resizable, closable, maximizable, iconifiable);
    doNexToneSpecifics();
  }

  private void doNexToneSpecifics () {
    createGlassPane();

    cl = new ComponentListener () {
	public void componentResized (ComponentEvent ce) {
	  Border b = getBorder();

	  if (isMaximum()) {
	    if (b == null)
	      return;
	    border = b;
	    setBorder(null);
	    getParent().validate();
	    getParent().repaint();
	  } else {
	    if (border != null) {
	      setBorder(border);
	      border = null;
	      getParent().validate();
	      getParent().repaint();
	    }
	  }
	}

	public void componentMoved (ComponentEvent ce) { }
	public void componentShown (ComponentEvent ce) {}
	public void componentHidden (ComponentEvent ce) {}
      };
    addComponentListener(cl);

    ifa = new InternalFrameAdapter () {
	public void internalFrameIconified (InternalFrameEvent ife) {
	  // some stupid bug in JDK does not restore the maximized
	  // frames properly
	  if (border != null) {
	    setBorder(border);
	    border = null;
	  }
	}

	// bug in java - everytime a frame is activated the 
	public void internalFrameActivated (InternalFrameEvent ife) {
	  //	  System.out.println("internal frame activated: " + getTitle());
	  setGlassCursor(currentCursor);
	}
      };
    addInternalFrameListener(ifa);

    ma = new MouseAdapter () {
	public void mouseEntered (MouseEvent me) {
	  try {
	    setSelected(true);
	  } catch (PropertyVetoException pve) {}
	}
      };
    addMouseListener(ma);
  }

  private void createGlassPane () {
    Component gp = getGlassPane();

    gpml = new MouseListener() {
	public void mouseClicked (MouseEvent me) {
	  //	  System.out.println("mouse click consumed");
	  me.consume();
	}
	public void mouseEntered (MouseEvent me) {
	  me.consume();
	}
	public void mouseExited (MouseEvent me) {
	  me.consume();
	}
	public void mousePressed (MouseEvent me) {
	  me.consume();
	}
	public void mouseReleased (MouseEvent me) {
	  me.consume();
	}
      };
    gp.addMouseListener(gpml);

    gpmml = new MouseMotionListener () {
	public void mouseDragged (MouseEvent me) {
	  if (currentCursor == Cursor.DEFAULT_CURSOR)
	    me.consume();
	}

	public void mouseMoved (MouseEvent me) {
	  if (currentCursor == Cursor.DEFAULT_CURSOR)
	    me.consume();
	}
      };
    gp.addMouseMotionListener(gpmml);
  }

  /**
   * sets the cursor displayed on the frame
   */
  public void setGlassCursor (int c) {
    if (c == Cursor.WAIT_CURSOR) {
      getGlassPane().setVisible(true);
      //      System.out.println("setting cursor to wait");
    } else if (c == Cursor.DEFAULT_CURSOR) {
      getGlassPane().setVisible(false);
      //      System.out.println("setting cursor to default");
    }

    getGlassPane().setCursor(new Cursor(c));
    currentCursor = c;
  }

  /**
   * method called when the window is closing
   * need to externally implement a listener to call this method automatically
   * returns true if the exit can happen or false if anything prevents
   * from exiting
   */
  public boolean exit () {
    getGlassPane().removeMouseListener(gpml);
    gpml = null;
    getGlassPane().removeMouseMotionListener(gpmml);
    gpmml = null;
    setVisible(false);
    JDesktopPane jdp = getDesktopPane();
    if (jdp instanceof ScrollableDesktopPane)
      ((ScrollableDesktopPane)jdp).removeComponent(this);
    removeMouseListener(ma);
    ma = null;
    removeInternalFrameListener(ifa);
    ifa = null;
    removeComponentListener(cl);
    cl = null;
    dispose();
    return true;
  }

}




