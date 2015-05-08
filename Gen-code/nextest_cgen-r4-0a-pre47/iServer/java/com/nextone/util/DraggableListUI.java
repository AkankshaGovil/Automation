package com.nextone.util;

import java.awt.event.MouseEvent;
import javax.swing.event.MouseInputListener;
import javax.swing.plaf.basic.BasicListUI;

/**
 * This class extends the javax.swing.plaf.basic.BasicListUI and overrides the MouseInputHandler
 * to ignore list selection during mousePressed, and instead do it during mouseClicked. We need
 * to do this so that a JListwith multiple rows selected can support Drag and Drop.
 */
public class DraggableListUI extends BasicListUI {

  /**
   * creates a new mouse input listener that does the list selection during mouseClicked
   * rather than the mousePressed
   */
  protected MouseInputListener createMouseInputListener () {
    return new DraggableMouseInputListener();
  }

  private class DraggableMouseInputListener extends MouseInputHandler {
    public void mousePressed (MouseEvent me) {}

    public void mouseClicked (MouseEvent me) {
      super.mousePressed(me);
    }

    public void mouseDragged (MouseEvent me) {}
  }
}

