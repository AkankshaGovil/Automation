package com.nextone.util;

import javax.swing.BorderFactory;
import javax.swing.ImageIcon;
import javax.swing.JComponent;
import javax.swing.JLabel;
import javax.swing.JWindow;
import java.awt.BorderLayout;
import java.awt.Color;
import java.awt.Cursor;
import java.awt.Dimension;
import java.awt.Frame;
import java.awt.event.MouseAdapter;
import java.awt.Toolkit;

/**
 * a utility class which displays a "splash" screen
 * example usage: <code>
 * ...
 * SplashScreen ss = new SplashScreen(icon);
 * ss.setVisible(true);
 * .....  // do time consuming process here
 * ss.remove();
 * </code>
 */
public class SplashScreen extends JWindow {
	  private ImageIcon icon;

	  /**
	   * @param icon the icon to be displayed in the splash screen
	   */
	  public SplashScreen (ImageIcon icon) {
		 this(null, icon, Color.black);
	  }

	  /**
	   * @param icon the icon to be displayed in the splash screen
	   * @param bgColor the background color of the panel (useful if the
	   * image in the icon has a transparent background)
	   */
	  public SplashScreen (ImageIcon icon, Color bgColor) {
		 this(null, icon, bgColor);
	  }

	  /**
	   * @param owner the owner of this window
	   * @param icon the icon to be displayed in the splash screen
	   */
	  public SplashScreen (Frame owner, ImageIcon icon) {
		 this(owner, icon, Color.black);
	  }

	  /**
	   * @param owner the owner of this window
	   * @param icon the icon to be displayed in the splash screen
	   * @param bgColor the background color of the panel (useful if the
	   * image in the icon has a transparent background)
	   */
	  public SplashScreen (Frame owner, ImageIcon icon, Color bgColor) {
		 super(owner);
		 this.icon = icon;

		 JLabel sp = new JLabel(icon, JLabel.CENTER);
		 sp.setAlignmentX(JLabel.CENTER_ALIGNMENT);

		 getContentPane().add(sp, BorderLayout.CENTER);
		 getContentPane().setBackground(bgColor);
		 // we cheat here, assuming that getContentPane() returns a JComponent
		 ((JComponent)getContentPane()).setBorder(BorderFactory.createCompoundBorder(BorderFactory.createLoweredBevelBorder(), BorderFactory.createRaisedBevelBorder()));

		 pack();
		 Dimension d = Toolkit.getDefaultToolkit().getScreenSize();
		 setLocation( (d.width - getSize().width)/2,
					  (d.height - getSize().height)/2 );

		 setCursor(Cursor.getPredefinedCursor(Cursor.WAIT_CURSOR));
		 getGlassPane().addMouseListener(new MouseAdapter() {});
		 getGlassPane().setVisible(true);
	  }

	  /**
	   * call this method to make the splash screen disappear and release
	   * all of it's associated resources
	   */
	  public void remove () {
		 setVisible(false);
		 icon.getImage().flush();
		 dispose();
	  }

}

