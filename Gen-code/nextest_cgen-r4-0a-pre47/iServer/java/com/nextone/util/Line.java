package com.nextone.util;

import javax.swing.*;
import java.awt.*;

public class Line extends Component {
	  private Color c;
	  private int x, y;
	  private int orient;

	  public Line (int len) {
		 this(len, Color.black, SwingConstants.HORIZONTAL);
	  }

	  public Line (int len, int orientation) {
		 this(len, Color.black, orientation);
	  }

	  public Line (int len, Color c, int orientation) {
		 this.orient = orientation;
		 setColor(c);
		 setLength(len);
	  }

	  public void setLength (int len) {
		 setSize(len+1, 2);
		 if (orient == SwingConstants.VERTICAL) {
			x = 1;
			y = len;
		 } else {
			x = len;
			y = 1;
		 }
	  }

	  public void setColor (Color c) {
		 this.c = c;
	  }

	  public void paint (Graphics g) {
		 g.setColor(c);
		 g.fillRect(0, 0, x, y);
	  }

}

