package com.nextone.util;

import javax.swing.JPanel;
import java.awt.geom.*;
import java.awt.Graphics;

public class StatusAnimation extends JPanel{
  boolean updateFlag;

 // public void paint(Graphics g) {
 // }
         
  public void update(int n) {
    updateFlag  = true;
    update(getGraphics());
    updateFlag  = false;
  }

  public void cleanup(){
  }

}
