package com.nextone.common;
import javax.swing.border.*;
import javax.swing.*;
import java.util.*;
import java.awt.*;
import java.awt.event.*;
import java.awt.Color;


/**
 * Class which contains all the common functions.
 */


public class CommonFunctions {

	private static int maxOffset	=	120;

	/**
	 *	Set the maximum offset value. Used by wrap Objects function
	 *	@param	offset	maximum offset
	 */
	public static void setMaxOffset(int offset){
		maxOffset	=	offset;
	}

	/**
	 *	wrap and align the given components into a panel
	 *	@param	components components to be wraped
	 *	@return panel
	 */

	public static JPanel wrapObjects(Vector components){

		if (components.size()	==	0)
			return null;

		FlowLayout	fl	=	new FlowLayout(FlowLayout.RIGHT);
		JPanel wrpPanel = new JPanel();
		wrpPanel.setLayout(fl);

		int diff	=	maxOffset	-	(int)( ((Component)components.elementAt(0)).getMaximumSize()).getWidth();

		if( diff >0)
			wrpPanel.add(Box.createHorizontalStrut(diff));
		wrpPanel.add((Component)components.elementAt(0));
		fl.setAlignment(FlowLayout.LEFT);
		for(int i=0; i < components.size(); i++){
			wrpPanel.add(Box.createHorizontalStrut(10));
			wrpPanel.add((Component)components.elementAt(i),fl);
		}
		return wrpPanel;
	}

	/**
	 *	wrap and align two components into a panel
	 *	@param	l, d
	 *	@return panel
	 */

	public static JPanel wrapObjects(Component l, Component d){

		FlowLayout	fl	=	new FlowLayout(FlowLayout.RIGHT);
		JPanel wrpPanel = new JPanel();
		wrpPanel.setLayout(fl);

		int diff	=	maxOffset	-	(int)(l.getMaximumSize()).getWidth();

		if( diff >0)
			wrpPanel.add(Box.createHorizontalStrut(diff));
		wrpPanel.add(l);

		wrpPanel.add(Box.createHorizontalStrut(10));
		fl.setAlignment(FlowLayout.LEFT);
		wrpPanel.add(d,fl);
		return wrpPanel;
	}

	public static JPanel setObjectBorder(Vector components,String title){

		Box	panelBox	=	new Box(BoxLayout.Y_AXIS);

		JPanel borderPanel = new JPanel();
		borderPanel.setLayout(new BorderLayout());

		for(int i=0; i < components.size(); i++)
			panelBox.add((Component)components.elementAt(i));

		borderPanel.add(panelBox);
		TitledBorder b1 = BorderFactory.createTitledBorder(BorderFactory.createEtchedBorder(), title);
		b1.setTitleJustification(TitledBorder.LEFT);
		b1.setTitlePosition(TitledBorder.DEFAULT_POSITION);

		borderPanel.setBorder(b1);
		return borderPanel;
	}


	/**
	 *	Please do not change this function. It depends on  iserver bits.h
	 */ 

	public static boolean BIT_TEST(short x,short n){
		if ( ( x &  (1 << n)) >0) 
			return true;
		else
			return false;
	}


	public static short BIT_SET(short x,short n){
    short result =  x |=  (1 << n);
		return (result);
	}


	public static short BIT_RESET(short x,short n){
		return (x &=  ~(1 << n));
	}


}

