package com.nextone.util;

import java.awt.*;
import java.awt.event.*;
import javax.swing.*; 
import javax.swing.ImageIcon.*; 
import java.awt.geom.*;

public class JSpinButton extends JButton implements MouseListener{
  private Color bgColor;
  private int width = 10;
  private int height = 5;
  private int state;
  private int type;
  private Graphics graphics;
  private int xPoints[] = {0,0,0}; 
  private int yPoints[] = {0,0,0}; 


  public static final int PRESSED   = 0;
  public static final int RELEASED  = 1;
  public static final int DISABLED  = 2;
  public static final int ENABLED   = 3;

  public static final int UP  = 0;
  public static final int DOWN  = 1;
  GeneralPath filledPolygon;
  final Rectangle2D.Double rect = new Rectangle2D.Double();

  public static final Color pressColor = new Color(164,164,164);


  public JSpinButton(){
    super();
  }

  public JSpinButton(Color bgColor, int type){
    super();
    this.bgColor  = bgColor;
    this.type  = type;
    state  = RELEASED;
    setPreferredSize(new Dimension(width,width));
    this.addMouseListener(this);


    switch(type){
      case  UP:
        xPoints[0] = width/2;
        xPoints[1] = 0;
        xPoints[2] = width;

        yPoints[0] = 0;
        yPoints[1] = height;
        yPoints[2] = height;
        break;
      case  DOWN:
        xPoints[0] = 0;
        xPoints[1] = width/2;
        xPoints[2] = width;

        yPoints[0] = 0;
        yPoints[1] = height;
        yPoints[2] = 0;
        break;
    }
    filledPolygon = new GeneralPath(GeneralPath.WIND_EVEN_ODD,xPoints.length);

  }

  public void paint(Graphics g){

    Graphics2D g2 = (Graphics2D) g;

    g2.setPaint(bgColor);
	  rect.setRect(0,0,width,height);
	  g2.fill(rect);
    g2.setPaint(Color.black);
    g2.draw(rect);

   // fill and stroke GeneralPath
    filledPolygon.moveTo(xPoints[0], yPoints[0]);
    for ( int index = 1; index < xPoints.length; index++ ) {
        filledPolygon.lineTo(xPoints[index], yPoints[index]);
    };
    filledPolygon.closePath();

    if(state  ==  RELEASED){
      g2.setPaint(bgColor);
      g2.fill(filledPolygon);
      g2.setPaint(Color.darkGray);
      g2.draw(filledPolygon);

    }
    else if(state  ==  PRESSED){
      g2.setPaint(pressColor);
      g2.fill(filledPolygon);
      g2.setPaint(bgColor);
      g2.draw(filledPolygon);
    }else if(state  ==  DISABLED){
      g2.setPaint(Color.darkGray);
      g2.fill(filledPolygon);
      g2.setPaint(Color.darkGray);
      g2.draw(filledPolygon);
    }
  }

  public void mouseClicked(MouseEvent e){
  }

  public void mousePressed(MouseEvent e){
    state =  PRESSED;
    update(getGraphics());
  }
  public void mouseReleased(MouseEvent e){
    state =  RELEASED;
    update(getGraphics());
  }
  public void mouseEntered(MouseEvent e){
  }
  public void mouseExited(MouseEvent e){
  }

  public void setDisabled(){
    state = DISABLED;
    update(getGraphics());
  }

  public void setEnabled(){
    state = RELEASED;
    update(getGraphics());
  }
}
