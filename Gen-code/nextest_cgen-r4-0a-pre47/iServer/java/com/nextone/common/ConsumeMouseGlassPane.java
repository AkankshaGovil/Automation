package com.nextone.common;

import javax.swing.*;
import java.awt.*;
import java.awt.event.*;

/**
 * Consumes all the mouse events
 */
public class ConsumeMouseGlassPane extends JComponent {
	  protected MouseListener ml;
	  protected MouseMotionListener mml;

	  public ConsumeMouseGlassPane () {
		 ml = new MouseListener() {
				  public void mouseClicked (MouseEvent me) {
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
		 addMouseListener(ml);

		 mml = new MouseMotionListener() {
				  public void mouseDragged (MouseEvent me) {
					 me.consume();
				  }
				  public void mouseMoved (MouseEvent me) {
					 me.consume();
				  }
			};
		 addMouseMotionListener(mml);
	  }

	  public void exit () {
		 removeMouseListener(ml);
		 removeMouseMotionListener(mml);
	  }
}
