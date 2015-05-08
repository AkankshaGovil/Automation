package com.nextone.util.JMask;

import javax.swing.*;
import java.awt.*;
import java.awt.event.*;

public class PhoneNumber extends/* JMaskField*/ JTextField {

	  public PhoneNumber () {
//		 super("(###) ###-####", new MaskMacros('#', "[0-9]"), ' ');
		 Font f = new Font("Courier", Font.PLAIN, 12);
		 setFont(f);
		 setColumns(18);
	  }
/*
	  public static void main (String [] args) {
		 PhoneNumber iaf = new PhoneNumber();
		 JFrame f = new JFrame();
		 JPanel p = new JPanel();
		 p.setLayout(new FlowLayout(FlowLayout.LEFT));
		 p.add(new JLabel("Phone:  ", JLabel.LEFT));
		 p.add(iaf);
		 f.getContentPane().add(p);
		 f.setSize(new Dimension(200, 100));
		 f.setVisible(true);

	  }
*/
}

