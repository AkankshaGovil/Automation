package com.nextone.util;

import java.awt.geom.*;
import java.awt.Color;
import java.awt.Graphics;
import java.awt.GradientPaint;
import java.awt.Graphics2D;
import java.awt.RenderingHints;
import java.awt.Dimension;


public class BookAnimation extends StatusAnimation{

  private boolean firstFlag = true;
  private Rectangle2D.Double rect = new Rectangle2D.Double(); 
  private Color lightPurple = new Color(202,202,255);
  private Color darkPurple = new Color(89,0,130);
  private Color bgColor;
  int animationCount;
  int offset  = 10;
  int width   = 80;
  int height  = 60;
  int time;

  int z=0;
  int x = offset+130;
  int y = offset;

  int ax1Points[] = {x, x+width, x+width-3, x+3,x};
  int ay1Points[] = {y+height, y+height, y, y,y+height};

  int ax2Points[] = {x+4, x+(width/2), x+(width/2), x+(width/2)-3,x+7,x+4};
  int ay2Points[] = {y+height-2, y+height-2, y+3, y,y,y+height-2};

  int ax3Points[] = {x+width/2, x+width-4, x+width-7, x+(width/2)+3,x+width/2,x+width/2};
  int ay3Points[] = {y+height-2, y+height-2, y, y,y+3,y+height-2};


  int bx1Points[] = {x+width/2, x+(width/4)+width/2,x+width/2+(width/4)-3, x+width/2,x+width/2};
  int by1Points[] = {y+height-2, y+height-5, y-4, y,y+height-2};

  int bx2Points[] = {x+(3*(width/4)), x+(3*(width/4))+(width/8),x+(3*(width/4))+(width/8)-3, x+(3*(width/4))-3,x+(3*(width/4))};
  int by2Points[] = {y+height-5, y+height-6, y, y,y+height-5};

  int cx1Points[] = {x+(width/2)+7, x+(width/2)+(width/4), x+(width/2)+(width/4)-3,x+(width/2)+5,x+(width/2)+5,x+(width/2)+2,x+(width/2)+2,x+(width/2)+7};
  int cy1Points[] = {y+height-2, y+height-7, y, y,y-5,y-6,y+height-8,y+height-2};

  int dx1Points[] = {x+width/2+2, x+width/2+(width/4)-5, x+width/2+(width/4)-8,x+width/2+1,x+width/2+1,x+width/2-2,x+width/2-5,x+width/2-5,x+width/2,x+width/2+2};
  int dy1Points[] = {y+height-2, y+height-10, y, y,y-5,y-8,y-8,y+height-10,y+height-5,y+height-2};

  int ex1Points[] = {x+width/2+1, x+width/2+6, x+width/2+6,x+width/2+1,x+width/2+1,x+width/2-1,x+width/2-15,x+width/2-19,x+width/2-2,x+width/2+1};
  int ey1Points[] = {y+height-2, y+height-7, y, y,y-4,y-6,y-7,y+height-11,y+height-5,y+height-2};

  int fx1Points[] = {x+width/2+1, x+width/2+5, x+width/2+5, x+width/2-5,x+width/2-(width/4)-4,x+width/2- (width/4)-6,x+width/2- 9,x+width/2+1};
  int fy1Points[] = {y+height-2, y+height-4, y,y-5,y-10,y+height-10,y+height-5,y+height-2};

  int gx1Points[] = {x+width/2, x+width/2, x+width/2-(width/4)+2,x+width/2-(width/4)-6,x+width/2-(width/4)-9,x+width/2-(width/4),x+width/2};
  int gy1Points[] = {y+height-2, y,y-2,y-4,y+height-4,y+height-3,y+height-2};




  Line2D.Double darkLine1 = new Line2D.Double(x+(width/2), y, x+(width/2) , y+height-2);
  Line2D.Double darkLine2 = new Line2D.Double(x+(width/2)+1, y, x+(width/2)+1 , y+height-2);

  GradientPaint lpurpletodpurple  = new GradientPaint(x,y,lightPurple,x+width, y,darkPurple);
  GradientPaint whitetogray       = new GradientPaint(x+4,y+3,Color.white,x+(width/2), y+3,Color.gray);
  GradientPaint whitetogray_1     = new GradientPaint(x+(width/2),y+3,Color.white,x+width-4, y+3,Color.gray);
  GradientPaint whitetogray_2     = new GradientPaint(x+(width/2)-(width/4)-9,y+height-2,Color.white,x+(width/2), y+height-2,Color.gray);
  GradientPaint whitetolightgray  = new GradientPaint(x+(width/2),y+3,Color.white,x+width-4, y+3,Color.lightGray);
  GradientPaint darkgraytoblack   = new GradientPaint(x+(3*(width/4)),y+3,Color.darkGray,x+(3*(width/4))+width-4, y+3,Color.black);
  GradientPaint darkgraytoblack_1 = new GradientPaint(x+(width/2),y+height-7,Color.darkGray,x+(width/2)+width/4, y+height-7,Color.black);
  GradientPaint darkgraytoblack_2 = new GradientPaint(x+(width/2)-5,y+height-10,Color.darkGray,x+(width/2)+(width/4)-5, y+height-10,Color.black);
  GradientPaint whitetoblack      = new GradientPaint(x+(width/2)-(width/4)-4,y+height-10,Color.white,x+(width/2)+6, y+height-10,Color.black);
  GradientPaint whitetoblack_1    = new GradientPaint(x+(width/2)-(width/4)-6,y+height-10,Color.white,x+(width/2)+5, y+height-10,Color.black);



  public BookAnimation(){
    bgColor = getBackground();
    updateFlag = false;
    setPreferredSize( new Dimension(200,70) );
    animationCount  = 0;
  }

  public void paint(Graphics g) {
    Graphics2D g2 = (Graphics2D) g;
         
    if(g2 == null)
      return;
    g2.setRenderingHint(RenderingHints.KEY_ANTIALIASING, RenderingHints.VALUE_ANTIALIAS_ON);

    if(!updateFlag ||  firstFlag){
        firstFlag = false;
        drawBase(g2);
        drawLeftSide(g2);
        drawRightSide(g2);
    }

    animationCount++;
    int rem = animationCount%8;
    g2.setPaint(bgColor);
   	rect.setRect(x,0,x+width,offset);
  	g2.fill(rect);


    switch(rem){
      case  0:{
              //  book left side
              drawLeftSide(g2);
              //  book right side
              drawRightSide(g2);
               break;
              }
      case  1:{
          //book turn1
            GeneralPath book2_0= new GeneralPath(GeneralPath.WIND_EVEN_ODD,bx1Points.length);
            book2_0.moveTo(bx1Points[0], by1Points[0]);
            for ( int index = 1; index < bx1Points.length; index++ ) {
                book2_0.lineTo(bx1Points[index], by1Points[index]);
            };
            book2_0.closePath();
            g2.setPaint(whitetolightgray);
            g2.fill(book2_0);
            GeneralPath book2_1 = new GeneralPath(GeneralPath.WIND_EVEN_ODD,bx2Points.length);
            book2_1.moveTo(bx2Points[0], by2Points[0]);
            for ( int index = 1; index < bx2Points.length; index++ ) {
                book2_1.lineTo(bx2Points[index], by2Points[index]);
            };
            book2_1.closePath();
            g2.setPaint(darkgraytoblack);
            g2.fill(book2_1);
            g2.draw(darkLine1);
            g2.draw(darkLine2);

            break;
          }

      case  2:{
          //  book right side
          drawRightSide(g2);
          GeneralPath book3_0 = new GeneralPath(GeneralPath.WIND_EVEN_ODD,cx1Points.length);
          book3_0.moveTo(cx1Points[0], cy1Points[0]);
          for ( int index = 1; index < cx1Points.length; index++ ) {
              book3_0.lineTo(cx1Points[index], cy1Points[index]);
          };
          book3_0.closePath();

          g2.setPaint(darkgraytoblack_1);
          g2.fill(book3_0);
          break;
      } 

      case  3:{

            drawLeftSide(g2);
            drawRightSide(g2);
            GeneralPath book4_0 = new GeneralPath(GeneralPath.WIND_EVEN_ODD,dx1Points.length);
            book4_0.moveTo(dx1Points[0], dy1Points[0]);
            for ( int index = 1; index < dx1Points.length; index++ ) {
                book4_0.lineTo(dx1Points[index], dy1Points[index]);
            };
            book4_0.closePath();
            g2.setPaint(darkgraytoblack_2);
            g2.fill(book4_0);
            break;
      }

      case  4:{
            drawLeftSide(g2);
            drawRightSide(g2);

            GeneralPath book5_0 = new GeneralPath(GeneralPath.WIND_EVEN_ODD,ex1Points.length);
            book5_0.moveTo(ex1Points[0], ey1Points[0]);
            for ( int index = 1; index < ex1Points.length; index++ ) {
                book5_0.lineTo(ex1Points[index], ey1Points[index]);
            };
            book5_0.closePath();
            g2.setPaint(whitetoblack);
            g2.fill(book5_0);
            break;
      }
      case 5:{
          drawLeftSide(g2);
          drawRightSide(g2);
          GeneralPath book6_0 = new GeneralPath(GeneralPath.WIND_EVEN_ODD,fx1Points.length);
          book6_0.moveTo(fx1Points[0], fy1Points[0]);
          for ( int index = 1; index < fx1Points.length; index++ ) {
              book6_0.lineTo(fx1Points[index], fy1Points[index]);
          };
          book6_0.closePath();

          g2.setPaint(whitetoblack_1);
          g2.fill(book6_0);
          break;
        }

      case  6:{
          drawLeftSide(g2);
          drawRightSide(g2);
          GeneralPath book7_0 = new GeneralPath(GeneralPath.WIND_EVEN_ODD,gx1Points.length);
          book7_0.moveTo(gx1Points[0], gy1Points[0]);
          for ( int index = 1; index < gx1Points.length; index++ ) {
              book7_0.lineTo(gx1Points[index], gy1Points[index]);
          };
          book7_0.closePath();
          g2.setPaint(whitetogray_2);
          g2.fill(book7_0);
          break;
        }

      }


    }


  public void drawBase(Graphics2D  g2){
    GeneralPath book1 = new GeneralPath(GeneralPath.WIND_EVEN_ODD,ax1Points.length);
    book1.moveTo(ax1Points[0], ay1Points[0]);
    for ( int index = 1; index < ax1Points.length; index++ ) {
        book1.lineTo(ax1Points[index], ay1Points[index]);
    };
    book1.closePath();
    g2.setPaint(lpurpletodpurple);
    g2.fill(book1);
    g2.setPaint(darkPurple);
    g2.draw(book1);
  }

  public void drawLeftSide(Graphics2D  g2){
    GeneralPath book1_1 = new GeneralPath(GeneralPath.WIND_EVEN_ODD,
                                          ax2Points.length);
    book1_1.moveTo(ax2Points[0], ay2Points[0]);
    for ( int index = 1; index < ax2Points.length; index++ ) {
        book1_1.lineTo(ax2Points[index], ay2Points[index]);
    };
    book1_1.closePath();

    g2.setPaint(getBackground());
   	rect.setRect(x+4,y,width/2-4,height-2);
  	g2.fill(rect);


    g2.setPaint(whitetogray);
    g2.fill(book1_1);
  }

  public void drawRightSide(Graphics2D  g2){
    GeneralPath book1_2 = new GeneralPath(GeneralPath.WIND_EVEN_ODD,ax3Points.length);
    book1_2.moveTo(ax3Points[0], ay3Points[0]);
    for ( int index = 1; index < ax3Points.length; index++ ) {
        book1_2.lineTo(ax3Points[index], ay3Points[index]);
    };
    book1_2.closePath();
    g2.setPaint(whitetogray_1);
    g2.fill(book1_2);
    g2.setPaint(Color.darkGray);
    g2.draw(darkLine1);
    g2.draw(darkLine2);
  }

}
