package com.nextone.common;

import javax.swing.*;
import java.awt.*;
import java.awt.event.*;
import java.awt.image.*;

public class AnimPanel extends JPanel implements ActionListener {
	  private Timer timer;
	  private Point point;
	  private final int Y = 25;
	  private final Color myBlue = new Color(85, 220, 240);
	  private final int PX;
	  private final int PY;
	  private int len = 10;
	  private Image comp;
	  private boolean debug;
	  private int delay = 0;

	  public AnimPanel () {
		 this(50, 10);
	  }

	  public AnimPanel (int x, int y) {
		 this(x, y, false);
	  }

	  public AnimPanel (int x, int y, boolean d) {
		 PX = x;
		 PY = y;
		 debug = d;

		 setLayout(new FlowLayout(FlowLayout.RIGHT));
		 if (debug)
			setBorder(BorderFactory.createLoweredBevelBorder());
		 setPreferredSize(new Dimension(PX, PY));

		 comp = new ImageIcon(getClass().getResource("/com/nextone/images/Computer.gif")).getImage();
		 timer = new Timer(30, this);
		 timer.setCoalesce(true);
		 timer.stop();
		 point = new Point(0, Y);
	  }

	  public void paintComponent (Graphics g) {
		 super.paintComponent(g);
		 if (timer.isRunning()) {
			g.drawImage(comp, PX-20, PY, null);
			if (len > 0) {
			   g.drawRect(point.x, point.y, len, 3);
			   g.setColor(myBlue);
			   g.fillRect(point.x, point.y, len, 3);
			}
		 }
	  }

	  public void startAnimation () {
		 if (timer.isRunning())
			return;
		 point.setLocation(0, Y);
		 len = 10;
		 timer.start();
	  }

	  public void stopAnimation () {
		 if (timer.isRunning())
			timer.stop();
	  }


	  public void actionPerformed (ActionEvent ae) {
		 if (point.x >= PX-len-20) {
			if (point.x == PX-20) {
			   // delay effect
			   if (delay > 15) {
				  point.setLocation(0, Y);
				  delay = 0;
			   } else
				  delay++;
			} else {
			   --len;
			   point.setLocation(point.x+1, Y);
			}
		 } else {
			point.setLocation(point.x+1, Y);
			if (len < 10)
			   len++;
//			len = 10;
		 }

		 repaint();
	  }

}
