package com.nextone.util;

import javax.swing.*;

public class BottomLabel extends JLabel {

	  public BottomLabel (String str, int horizontal) {
		 super(str, horizontal);
		 setVerticalAlignment(SwingConstants.BOTTOM);
	  }

}

