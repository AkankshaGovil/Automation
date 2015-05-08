package com.nextone.common;

import javax.swing.*;
import java.awt.*;

/**
 * this frame provides a method which enables showing a wait cursor on
 * the entire frame, and consuming all mouse events while the wait
 * cursor is being shown.
 */
public abstract class ConsumeMouseFrame extends JFrame implements CursorChanger {
	  protected ConsumeMouseGlassPane cmgp;

	  public ConsumeMouseFrame () {
		 super();
		 cmgp = new ConsumeMouseGlassPane();
		 setGlassPane(cmgp);
	  }

	  public ConsumeMouseFrame (String str) {
		 super(str);
		 cmgp = new ConsumeMouseGlassPane();
		 setGlassPane(cmgp);
	  }

	  /**
	   * sets the cursor displayed on the frame
	   */
	  public void setGlassCursor (int c) {
		 if (c == Cursor.WAIT_CURSOR)
			cmgp.setVisible(true);
		 else if (c == Cursor.DEFAULT_CURSOR)
			cmgp.setVisible(false);
		 setCursor(new Cursor(c));
	  }
}

