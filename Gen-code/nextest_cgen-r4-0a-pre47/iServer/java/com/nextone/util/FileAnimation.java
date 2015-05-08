package com.nextone.util;

import java.awt.geom.*;
import java.awt.Color;
import java.awt.Graphics;
import java.awt.GradientPaint;
import java.awt.Graphics2D;
import java.awt.RenderingHints;
import java.awt.Dimension;


public class FileAnimation extends StatusAnimation {

  private Rectangle2D.Double rect = new Rectangle2D.Double(); 
  private Color folderColor = new Color(202,202,255);
  private Color bgColor;
  private int width   = 230;
  private int offset  = 20;  
  private int distX  = 50;
  private int distY  = 4;

  private int srcX1[] = {33+offset,3+offset,offset,30+offset,33+offset};
  private int srcY1[] = {28+offset,28+offset,10+offset,10+offset,28+offset};
  private int srcX2[] = {33+offset, 10+offset,10+offset,3+offset,1+offset,1+offset,30+offset,33+offset,33+offset };
  private int srcY2[] = {6+offset,6+offset, 2+offset, 2+offset,4+offset,10+offset,10+offset,28+offset,6+offset};
  private int srcX3[] = {30+offset,3+offset,10+offset,31+offset};
  private int srcY3[] = {10+offset,10+offset,offset,9+offset};

  private int dstX1[] = {33+offset+width,3+offset+width,offset+width,30+offset+width,33+offset+width};
  private int dstY1[] = {28+offset,28+offset,10+offset,10+offset,28+offset};
  private int dstX2[] = {33+offset+width, 10+offset+width,10+offset+width,3+offset+width,1+offset+width,1+offset+width,30+offset+width,33+offset+width,33+offset+width };
  private int dstY2[] = {6+offset,6+offset, 2+offset, 2+offset,4+offset,10+offset,10+offset,28+offset,6+offset};

  private int paper1X1[]  = {31+offset,34+offset,46+offset,42+offset,37+offset,31+offset,31+offset        };
  private int paper1Y1[]  = {8+offset,offset,4+offset,16+offset,18+offset,16+offset,8+offset};
  private int paper1X2[]  = {42+offset,35+offset,35+offset};
  private int paper1Y2[]  = {14+offset,14+offset,16+offset};

  private int paper2X1[]  = {28+offset+distX,33+offset+distX,35+offset+distX,46+offset+distX,42+offset+distX,37+offset+distX,28+offset+distX};
  private int paper2Y1[]  = {16+offset-distY,12+offset-distY,offset-distY,4+offset-distY,16+offset-distY,18+offset-distY,16+offset-distY};
  private int paper2X2[]  = {42+offset+distX,35+offset+distX,35+offset+distX};
  private int paper2Y2[]  = {14+offset-distY,14+offset-distY,16+offset-distY};

  private int paper3X1[]  = {28+offset+(distX*2),31+offset+(distX*2),36+offset+(distX*2),43+offset+(distX*2) ,42+offset+(distX*2),34+offset+(distX*2),28+offset+(distX*2)};
  private int paper3Y1[]  = {16+offset-(distY*2),6+offset-(distY*2),7+offset-(distY*2),4+offset-(distY*2),6+offset-(distY*2),14+offset-(distY*2),16+offset-(distY*2)};

  private int paper4X1[]  = {28+offset+(distX*3),39+offset+(distX*3),36+offset+(distX*3),33+offset+(distX*3),28+offset+(distX*3),22+offset+(distX*3),28+offset+(distX*3)};
  private int paper4Y1[]  = {16+offset-(distY/2),9+offset-(distY/2),7+offset-(distY/2),offset-(distY/2)-5,offset-(distY/2)-4,4+offset-(distY/2),16+offset-(distY/2)};

  private int paper4X2[]  = { 22+offset+(distX*3), 31+offset+(distX*3),29+offset+(distX*3)};
  private int paper4Y2[]  = {4+offset-(distY/2),1+offset-(distY/2),offset-(distY/2)-3};

  private int paper5X1[]  = {9+offset+width,7+offset+width,18+offset+width,21+offset+width,23+offset+width,9+offset+width};
  private int paper5Y1[]  = {10+offset,offset,offset-4,8+offset,9+offset,10+offset};
  private int fileAnimationCount;

  private GeneralPath srcPolyline1;
  private GeneralPath srcPolyline2;
  private GeneralPath srcPolyline3;
  private GeneralPath dstPolyline1;
  private GeneralPath dstPolyline2;
  private GeneralPath paper1,paper11;
  private GeneralPath paper2,paper21;
  private GeneralPath paper3;
  private GeneralPath paper4,paper41;
  private GeneralPath paper5;
  private Graphics  graphics;
  private boolean  firstFlag;
  private boolean keepRunning;

  public FileAnimation(){
    graphics  = getGraphics();
    bgColor = getBackground();
    updateFlag = false;
    firstFlag = true;
    setPreferredSize( new Dimension(250,50) );
    fileAnimationCount  = 0;
  }

  public void paint(Graphics g) {
      Graphics2D g2 = (Graphics2D) g;
      if(g2 == null)
        return;
	
      g2.setRenderingHint(RenderingHints.KEY_ANTIALIASING, RenderingHints.VALUE_ANTIALIAS_ON);


      srcPolyline1 = new GeneralPath(GeneralPath.WIND_NON_ZERO,srcX1.length);
      srcPolyline2 = new GeneralPath(GeneralPath.WIND_NON_ZERO,srcX2.length);
      srcPolyline3 = new GeneralPath(GeneralPath.WIND_NON_ZERO,srcX3.length);
      dstPolyline1 = new GeneralPath(GeneralPath.WIND_NON_ZERO,dstX1.length);
      dstPolyline2 = new GeneralPath(GeneralPath.WIND_NON_ZERO,dstX2.length);

      if(!updateFlag || firstFlag){
          firstFlag = false;
          g2.setPaint(bgColor);
	        rect.setRect(offset, offset-10, width+32, offset+34);
	        g2.fill(rect);


          srcPolyline1.moveTo (srcX1[0], srcY1[0]);
          for ( int index = 1; index < srcX1.length; index++ ) {
              srcPolyline1.lineTo(srcX1[index], srcY1[index]);
          };

          g2.setPaint(folderColor);
          g2.fill(srcPolyline1);
          g2.setPaint(Color.black);
          g2.draw(srcPolyline1);

          g2.setPaint(Color.black);
          srcPolyline2.moveTo (srcX2[0], srcY2[0]);
          for ( int index = 1; index < srcX2.length; index++ ) {
              srcPolyline2.lineTo(srcX2[index], srcY2[index]);
          };

          g2.setPaint(folderColor);
          g2.fill(srcPolyline2);
          g2.setPaint(Color.black);
          g2.draw(srcPolyline2);

          srcPolyline3.moveTo (srcX3[0], srcY3[0]);
          for ( int index = 1; index < srcX3.length; index++ ) {
              srcPolyline3.lineTo(srcX3[index], srcY3[index]);
          };

          g2.setPaint(Color.white);
          g2.fill(srcPolyline3);
          g2.setPaint(Color.black);
          g2.draw(srcPolyline3);

          g2.setPaint(Color.black);
          dstPolyline1.moveTo (dstX1[0], dstY1[0]);
          for ( int index = 1; index < dstX1.length; index++ ) {
              dstPolyline1.lineTo(dstX1[index], dstY1[index]);
          };


          g2.setPaint(folderColor);
          g2.fill(dstPolyline1);
          g2.setPaint(Color.black);
          g2.draw(dstPolyline1);


          g2.setPaint(Color.black);
          dstPolyline2.moveTo (dstX2[0], dstY2[0]);
          for ( int index = 1; index < dstX2.length; index++ ) {
              dstPolyline2.lineTo(dstX2[index], dstY2[index]);
          };
          g2.setPaint(folderColor);
          g2.fill(dstPolyline2);
          g2.setPaint(Color.black);
          g2.draw(dstPolyline2);
      }

      fileAnimationCount++;
      int rem = fileAnimationCount%6;
      switch(rem){

      case  1 :{
                g2.setPaint(Color.black);
                paper1       = new GeneralPath(GeneralPath.WIND_NON_ZERO,paper1X1.length);
                paper11      = new GeneralPath(GeneralPath.WIND_NON_ZERO,paper1X2.length);
                paper2      = new GeneralPath(GeneralPath.WIND_NON_ZERO,paper2X1.length);
                paper21     = new GeneralPath(GeneralPath.WIND_NON_ZERO,paper2X2.length);
                paper3     = new GeneralPath(GeneralPath.WIND_NON_ZERO,paper3X1.length);
                paper4     = new GeneralPath(GeneralPath.WIND_NON_ZERO,paper4X1.length);
                paper41     = new GeneralPath(GeneralPath.WIND_NON_ZERO,paper4X2.length);
                paper5     = new GeneralPath(GeneralPath.WIND_NON_ZERO,paper5X1.length);


                if(updateFlag){
                  g2.setPaint(bgColor);
	                rect.setRect(offset, offset-10, width+32, offset+34);
	                g2.fill(rect);


                  srcPolyline1.moveTo (srcX1[0], srcY1[0]);
                  for ( int index = 1; index < srcX1.length; index++ ) {
                      srcPolyline1.lineTo(srcX1[index], srcY1[index]);
                  };

                  g2.setPaint(folderColor);
                  g2.fill(srcPolyline1);
                  g2.setPaint(Color.black);
                  g2.draw(srcPolyline1);

                  g2.setPaint(Color.black);
                  srcPolyline2.moveTo (srcX2[0], srcY2[0]);
                  for ( int index = 1; index < srcX2.length; index++ ) {
                      srcPolyline2.lineTo(srcX2[index], srcY2[index]);
                  };

                  g2.setPaint(folderColor);
                  g2.fill(srcPolyline2);
                  g2.setPaint(Color.black);
                  g2.draw(srcPolyline2);

                  srcPolyline3.moveTo (srcX3[0], srcY3[0]);
                  for ( int index = 1; index < srcX3.length; index++ ) {
                      srcPolyline3.lineTo(srcX3[index], srcY3[index]);
                  };

                  g2.setPaint(Color.white);
                  g2.fill(srcPolyline3);
                  g2.setPaint(Color.black);
                  g2.draw(srcPolyline3);

                  g2.setPaint(Color.black);
                  dstPolyline1.moveTo (dstX1[0], dstY1[0]);
                  for ( int index = 1; index < dstX1.length; index++ ) {
                      dstPolyline1.lineTo(dstX1[index], dstY1[index]);
                  };


                  g2.setPaint(folderColor);
                  g2.fill(dstPolyline1);
                  g2.setPaint(Color.black);
                  g2.draw(dstPolyline1);
                }

                g2.setPaint(Color.black);
                dstPolyline2.moveTo (dstX2[0], dstY2[0]);
                for ( int index = 1; index < dstX2.length; index++ ) {
                    dstPolyline2.lineTo(dstX2[index], dstY2[index]);
                };
                g2.setPaint(folderColor);
                g2.fill(dstPolyline2);
                g2.setPaint(Color.black);
                g2.draw(dstPolyline2);

                g2.setPaint(Color.black);
                paper1.moveTo (paper1X1[0], paper1Y1[0]);
                for ( int index = 1; index < paper1X1.length; index++ ) {
                    paper1.lineTo(paper1X1[index], paper1Y1[index]);
                };

                g2.setPaint(Color.white);
                g2.fill(paper1);
                g2.setPaint(Color.black);
                g2.draw(paper1);


                g2.setPaint(Color.black);
                paper11.moveTo (paper1X2[0], paper1Y2[0]);
                for ( int index = 1; index < paper1X2.length; index++ ) {
                    paper11.lineTo(paper1X2[index], paper1Y2[index]);
                };
                g2.setPaint(Color.black);
                g2.draw(paper11);
                break;
               }
          case  2:{

            if(updateFlag){
                  g2.setPaint(bgColor);
	                rect.setRect(offset, offset-10, width-35, offset+34);
	                g2.fill(rect);


                  srcPolyline1.moveTo (srcX1[0], srcY1[0]);
                  for ( int index = 1; index < srcX1.length; index++ ) {
                      srcPolyline1.lineTo(srcX1[index], srcY1[index]);
                  };

                  g2.setPaint(folderColor);
                  g2.fill(srcPolyline1);
                  g2.setPaint(Color.black);
                  g2.draw(srcPolyline1);

                  g2.setPaint(Color.black);
                  srcPolyline2.moveTo (srcX2[0], srcY2[0]);
                  for ( int index = 1; index < srcX2.length; index++ ) {
                      srcPolyline2.lineTo(srcX2[index], srcY2[index]);
                  };

                  g2.setPaint(folderColor);
                  g2.fill(srcPolyline2);
                  g2.setPaint(Color.black);
                  g2.draw(srcPolyline2);

                  srcPolyline3.moveTo (srcX3[0], srcY3[0]);
                  for ( int index = 1; index < srcX3.length; index++ ) {
                      srcPolyline3.lineTo(srcX3[index], srcY3[index]);
                  };

                  g2.setPaint(Color.white);
                  g2.fill(srcPolyline3);
                  g2.setPaint(Color.black);
                  g2.draw(srcPolyline3);

                }

                g2.setPaint(Color.black);
                paper2 = new GeneralPath(GeneralPath.WIND_NON_ZERO,
                                                       paper2X1.length);
                paper2.moveTo (paper2X1[0], paper2Y1[0]);
                for ( int index = 1; index < paper2X1.length; index++ ) {
                    paper2.lineTo(paper2X1[index], paper2Y1[index]);
                };

                g2.setPaint(Color.white);
                g2.fill(paper2);
                g2.setPaint(Color.black);
                g2.draw(paper2);


                g2.setPaint(Color.black);
                paper21.moveTo (paper2X2[0], paper2Y2[0]);
                for ( int index = 1; index < paper2X2.length; index++ ) {
                    paper21.lineTo(paper2X2[index], paper2Y2[index]);
                };

                g2.setPaint(Color.black);
                g2.draw(paper21);
                break;
                }
          case  3:{

                g2.setPaint(bgColor);
	              rect.setRect(offset+35, offset-10, width-35, offset+34);
	              g2.fill(rect);
                g2.setPaint(Color.black);
                paper3 = new GeneralPath(GeneralPath.WIND_NON_ZERO,
                                                       paper3X1.length);
                paper3.moveTo (paper3X1[0], paper3Y1[0]);
                for ( int index = 1; index < paper3X1.length; index++ ) {
                    paper3.lineTo(paper3X1[index], paper3Y1[index]);
                };


                g2.setPaint(Color.white);
                g2.fill(paper3);
                g2.setPaint(Color.black);
                g2.draw(paper3);
                break;
              }
          case 4:{
                g2.setPaint(bgColor);
	              rect.setRect(offset+35, offset-10, width-35, offset+34);
	              g2.fill(rect);
                g2.setPaint(Color.black);
                paper4 = new GeneralPath(GeneralPath.WIND_NON_ZERO,
                                                       paper4X1.length);
                paper4.moveTo (paper4X1[0], paper4Y1[0]);
                for ( int index = 1; index < paper4X1.length; index++ ) {
                    paper4.lineTo(paper4X1[index], paper4Y1[index]);
                };

                g2.setPaint(Color.white);
                g2.fill(paper4);
                g2.setPaint(Color.black);
                g2.draw(paper4);

                g2.setPaint(Color.black);
                paper41 = new GeneralPath(GeneralPath.WIND_NON_ZERO,
                                                       paper4X2.length);
                paper41.moveTo (paper4X2[0], paper4Y2[0]);
                for ( int index = 1; index < paper4X2.length; index++ ) {
                    paper41.lineTo(paper4X2[index], paper4Y2[index]);
                };


                g2.setPaint(Color.black);
                g2.draw(paper41);
                break;
               }
          case  5:{

                g2.setPaint(bgColor);
	              rect.setRect(offset+35, offset-10, width-35, offset+34);
	              g2.fill(rect);

                g2.setPaint(Color.black);
                paper5 = new GeneralPath(GeneralPath.WIND_NON_ZERO,
                                                       paper5X1.length);
                paper5.moveTo (paper5X1[0], paper5Y1[0]);
                for ( int index = 1; index < paper5X1.length; index++ ) {
                    paper5.lineTo(paper5X1[index], paper5Y1[index]);
                };
                g2.setPaint(Color.white);
                g2.fill(paper5);
                g2.setPaint(Color.black);
                g2.draw(paper5);

                break;
              }

          }
    }

}
