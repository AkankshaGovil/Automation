package com.nextone.util;

import java.awt.*;
import java.awt.font.*;
import java.awt.event.*;
import java.net.*;
import java.util.*;
import javax.swing.*;
import javax.swing.event.*;
import javax.swing.border.*;

/**
 * This menu can contain large number of items, which can be scrolled using the arrow buttons
 */
public class AutoScrollMenu extends JMenu implements MenuListener {
  private MenuWindow menuWindow;
  private int orientation;

  public static final int VERTICAL_MENU = 0;   // used for vertical menu
  public static final int HORIZONTAL_MENU = 1; // used for horizontal menu (menu within menus)

  /**
   * @param name the menu name
   * @param the ImageIcon to indicate the up arrow
   * @param the ImageIcon to indicate the down arrow
   *
   * assumes that the orientation is VERTICAL (as in added to menubar)
   */
  public AutoScrollMenu (String name, ImageIcon upImage, ImageIcon downImage) {
    this(name, null, upImage, downImage);
  }

  /**
   * @param name the menu name
   * @param label the header label for the menu items in this menu, appears above the up arrow icon
   * @param the ImageIcon to indicate the up arrow
   * @param the ImageIcon to indicate the down arrow
   *
   * assumes that the orientation is VERTICAL (as in added to menubar)
   */
  public AutoScrollMenu (String name, String label, ImageIcon upImage, ImageIcon downImage) {
    this(name, label, VERTICAL_MENU, upImage, downImage);
  }


  /**
   * @param name the menu name
   * @param label the header label for the menu items in this menu, appears above the up arrow icon
   * @param orientation the orientation of the menu items, VERTICAL for a menu added directly
   * to the menu bar, HORIZONTAL for a menu added to another menu list
   * @param the ImageIcon to indicate the up arrow
   * @param the ImageIcon to indicate the down arrow
   *
   */
  public AutoScrollMenu (String name, String label, int orientation, ImageIcon upImage, ImageIcon downImage) {
    super(name);
    addMenuListener(this);
    menuWindow = new MenuWindow(this, label, upImage, downImage);
    this.orientation = orientation;
  }

  /**
   * adds the given menu item to the bottom of this autoscroll menu 
   */
  public JMenuItem add (JMenuItem item) {
    menuWindow.add(item);
    return item;
  }

  /**
   * clears all the menu items under this menu
   */
  public void clearItems () {
    menuWindow.clearItems();
  }

  public void menuSelected (MenuEvent e) {
    Point location = getLocationOnScreen();
    if (orientation == VERTICAL_MENU)
      menuWindow.setLocation(location.x, location.y + getSize().height);
    else
      menuWindow.setLocation(location.x + getSize().width, location.y);
    menuWindow.setVisible(true);
    menuWindow.requestFocus();
    menuWindow.repaint();
  }

  public void menuCanceled (MenuEvent e) {
    menuWindow.setVisible(false);
  }

  public void menuDeselected (MenuEvent e) {
    menuWindow.setVisible(false);
  }

  protected void fireMenuCanceled () {
    super.fireMenuCanceled();
  }

  /*
  public void removeNotify () {
    removeMenuListener(this);
    if (menuWindow != null) {
      menuWindow.removeListeners();
      menuWindow = null;
    }
    super.removeNotify();
  }
  */
}

class MenuWindow extends JWindow implements MouseListener, MouseMotionListener, ActionListener, FocusListener {
  private JViewport viewport;
  private Vector items;
  private JList list;
  private Font font;
  private JButton up, down;
  private int index;
  private javax.swing.Timer timer;
  private int increment = 0;
  private String label;
  private AutoScrollMenu menu;

  MenuWindow (AutoScrollMenu menu, String label, ImageIcon upImage, ImageIcon downImage) {
    super();
    this.label = label;
    this.menu = menu;

    items = new Vector();

    viewport = new JViewport();

    list = new JList();
    font = new Font("Dialog", Font.BOLD, 12);
    list.setFont(font);
    list.setBackground(null);
    list.setSelectionBackground(new Color(144, 151, 207));
    list.addMouseListener(this);
    list.addMouseMotionListener(this);
    viewport.setView(list);

    up = new JButton(upImage);
    up.setBorder(null);
    up.setFocusPainted(false);
    up.addMouseListener(this);

    down = new JButton(downImage);
    down.setBorder(null);
    down.setFocusPainted(false);
    down.addMouseListener(this);

    JPanel panel = new JPanel();
    panel.setBorder(BorderFactory.createRaisedBevelBorder());
    panel.setLayout(new BorderLayout());
    if (label == null)
      panel.add(up, BorderLayout.NORTH);
    else {
      JPanel tp = new JPanel();
      tp.setLayout(new GridLayout(2, 1));
      JLabel jl = new JLabel(label, JLabel.CENTER);
      jl.setFont(font);
      jl.setForeground(Color.black);
      JPanel jp = new JPanel();
      jp.setLayout(new BorderLayout());
      jp.add(Box.createHorizontalStrut(2), BorderLayout.WEST);
      jp.add(jl, BorderLayout.CENTER);
      jp.add(Box.createHorizontalStrut(2), BorderLayout.EAST);
      tp.add(jp);
      tp.add(up);
      panel.add(tp, BorderLayout.NORTH);
    }
    panel.add(viewport, BorderLayout.CENTER);
    panel.add(down, BorderLayout.SOUTH);

    getContentPane().add(panel);

    timer = new javax.swing.Timer(50, this);
    timer.setInitialDelay(200);
  }

  public void removeListeners () {
    list.removeMouseListener(this);
    list.removeMouseMotionListener(this);
    up.removeMouseListener(this);
    down.removeMouseListener(this);
    clearMenuItems();
    items = null;
    menu = null;
  }

  private void clearMenuItems () {
    Iterator it = items.iterator();
    while (it.hasNext()) {
      AutoScrollMenuItem asmi = (AutoScrollMenuItem)it.next();
      RemoveListener.removeJMenuItemListeners(asmi.menuItem);
      asmi.menuItem = null;
      asmi.label = null;
    }
    items.clear();
  }

  public void add (JMenuItem menuItem) {
    AutoScrollMenuItem item = new AutoScrollMenuItem(menuItem);
    items.addElement(item);
    list.setListData(items);
  }

  public void clearItems () {
    clearMenuItems();
    list.setListData(items);
  }

  public void setVisible (boolean visible) {
    if (visible) {
      int width = 0;
      int height = 0;

      // Calculate the maximum width of the items window
      FontRenderContext frc = new FontRenderContext(null, false, false);
      float stringWidth;
      for (int i = 0; i<items.size(); i++) {
	stringWidth = (float)font.getStringBounds(((AutoScrollMenuItem)items.elementAt(i)).label, frc).getWidth();
	if (stringWidth > width) {
	  width = (int)stringWidth;
	}
      }
      if (label != null) {
	stringWidth = (float)font.getStringBounds(label, frc).getWidth();
	if (stringWidth > width)
	  width = (int)stringWidth;
      }

      // Calculate the height of the items window
      Point location = getLocation();
      Dimension screenSize = Toolkit.getDefaultToolkit().getScreenSize();
      height = screenSize.height - location.y;
      // make the height to be in steps of 20
      height += (20 - (height % 20));
      height -= 20;  // leave a little space at the bottom

      index = 0;
      viewport.setViewPosition(list.indexToLocation(0));
      list.setSelectedIndex(-1);

      // height is either the remaining screen height or 20*numItemsInList + 2*20 for arrows
      setSize(width+5, Math.min(height, (2 + items.size())*20));
    }

    super.setVisible(visible);
    if (visible)
      requestFocus();
  }

  public void mousePressed (MouseEvent me) {
    increment = 0;
    Object src = me.getSource();
    if (src == up) {
      if (index > 0) {
	increment = -1;
	if (!timer.isRunning())
	  timer.start();
      }
    } else if (src == down) {
      int lastIndex = list.getLastVisibleIndex();
      if ( (lastIndex >=0) && (lastIndex< items.size() - 1)) {
	increment = 1;
	if (!timer.isRunning())
	  timer.start();
      }
    }
    index += increment;
    viewport.setViewPosition(list.indexToLocation(index));
  }

  public void mouseReleased (MouseEvent me) {
    if (timer != null) {
      timer.stop();
    }
    if (me.getSource() == list) {
      ((AutoScrollMenuItem)items.elementAt(list.getSelectedIndex())).menuItem.doClick();
      menu.fireMenuCanceled();
    }
  }

  public void mouseEntered(MouseEvent me) {
    mousePressed(me);
  }

  public void mouseExited(MouseEvent me) {
    if (timer != null) {
      timer.stop();
    }
  }

  public void mouseClicked(MouseEvent me) {}

  public void mouseDragged (MouseEvent me) {
    mouseMoved(me);
  }

  public void mouseMoved (MouseEvent me) {
    // highlight the list item
    int index = list.locationToIndex(me.getPoint());
    list.setSelectedIndex(index);
    list.ensureIndexIsVisible(index);
  }

  public void actionPerformed (ActionEvent e) {
    if (increment == 1) {
      if (list.getLastVisibleIndex() < items.size() - 1) {
	index++;
      }
    } else if (increment == -1) {
      if (index > 0) {
	index--;
      }
    }
    viewport.setViewPosition(list.indexToLocation(index));
  }

  public void focusGained (FocusEvent fe) {
    toFront();
  }

  public void focusLost (FocusEvent fe) {}
}

class AutoScrollMenuItem {
  JMenuItem menuItem;
  String label;

  AutoScrollMenuItem (JMenuItem menuItem) {
    this.menuItem = menuItem;
    this.label = menuItem.getText();
  }

  public String toString () {
    return label;
  }
}
