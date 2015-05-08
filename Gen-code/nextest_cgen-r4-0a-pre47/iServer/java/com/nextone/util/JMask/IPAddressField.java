package com.nextone.util.JMask;

import javax.swing.*;
import javax.swing.text.*;
import java.awt.*;
import java.awt.event.*;
import java.text.*;
import java.util.*;
import com.nextone.util.*;

public class IPAddressField extends JTextField {

	  public IPAddressField () {
		 Font f = new Font("Courier", Font.PLAIN, 12);
		 setFont(f);
		 setColumns(17);
	  }

	public IPAddressField (String value) {
		this();
		setText(value);
	}


	public boolean isRequestFocusEnabled(){ 
		return true; 
	} 


/*
	  public static void main (String [] args) {
		 IPAddressField iaf = new IPAddressField();
		 JFrame f = new JFrame();
		 JPanel p = new JPanel();
		 p.setLayout(new FlowLayout(FlowLayout.LEFT));
		 p.add(new JLabel("IP Address:  ", JLabel.LEFT));
		 p.add(iaf);
		 f.getContentPane().add(p);
		 f.setSize(new Dimension(200, 100));
		 f.setDefaultCloseOperation(JFrame.EXIT_ON_CLOSE);
		 f.setVisible(true);

	  }
*/

		// Return an IP address
		public IPAddress getIPAddress() {
			return new IPAddress(getText());
		}

		// Return the IP address whenever a string is required
		public String toString() {
			return getText();
		}
	  
	  protected Document createDefaultModel() {
		 return new IpAddressDocument();
	  }

	  protected class IpAddressDocument extends PlainDocument {
		
	  		// The IP address format should be checked
			// against a valid format ddd.ddd.ddd.ddd
			// where all entered values are in decimal.
			public void insertString(int offs, 
									 String str,
									 AttributeSet a) 
			   throws BadLocationException {
				
			   char[] source = str.toCharArray();
				// Add 3 to the result length to allow for the 
				// insertion of periods.
			   char[] result = new char[source.length+3];

			   // no more input than the field length
			   if ((getLength()+source.length) > 15) {
				  Toolkit.getDefaultToolkit().beep();
				  return;
			   }

				// Determine how many digits have been encountered 
				// since the start of the text field or since
				// the last period.
				String currentText = getText(0,getLength());
				String afterText = currentText.substring(offs,currentText.length());
				int charIndex = 0;
				int numberOfPeriods = 0;

				// Determine the number of periods in the current entry
				for (int i=0; i<currentText.length(); i++) {
					if (currentText.charAt(i) == '.') {
						numberOfPeriods++;
					}
				}

				// Determine the current character index
				for (int i=0; i<offs; i++) {
					if (currentText.charAt(i) == '.') {
						charIndex = 0;
					} else {
						charIndex++;
					}
				}

				// Determine the number of characters between the
				// index and the next period
				int charsAfterIndex = 0;
				for (; (charsAfterIndex < 3) && 
						(charsAfterIndex < afterText.length()) && 
						(afterText.charAt(charsAfterIndex) != '.');
						charsAfterIndex++);

				// Ensure that only digits are entered.
			   int j = 0;
			   for (int i = 0; i < source.length; i++) {
					if (Character.isDigit(source[i]) ||
					  source[i] == '.') {

						// If charIndex is 3, then a period is required.
						if ((charIndex >= 3)&&(source[i] != '.')) {
							// Automatically add a period.
							if (numberOfPeriods < 3) {
								result[j++] = '.';
								numberOfPeriods++;
								charIndex = 0;
							} 
						}
						
						// Allow only 3 periods and any numerical value
						if ((numberOfPeriods < 3)||(source[i] != '.')) {

							// Only 3 characters are allowed between each period
							if (((charIndex+charsAfterIndex)<3) || (source[i] == '.')) {
								if (source[i] != '.') {
									// Insert the numeric characters
									result[j++] = source[i];
									charIndex++;
								} else {
									if (offs == 0 && afterText.length() == 0) {
										result[j++] = '.';
										numberOfPeriods++;
										charIndex = 0;
									} else if ((offs > 0)&&(currentText.charAt(offs-1) != '.')) {
										// Do not allow adjacent periods (pre).
										result[j++] = '.';
										numberOfPeriods++;
										charIndex = 0;
									} else if ((offs < currentText.length()) && (currentText.charAt(offs) != '.')) {
										// Do not allow adjacent periods (post).
										result[j++] = '.';
										numberOfPeriods++;
										charIndex = 0;
									} else {
									 Toolkit.getDefaultToolkit().beep();
									}
								}
							} else {
							 Toolkit.getDefaultToolkit().beep();
							}
						} else {
							 Toolkit.getDefaultToolkit().beep();
						}
					 } else {
						 Toolkit.getDefaultToolkit().beep();
					 }
			   }
			   super.insertString(offs, new String(result, 0, j), a);
			}
	  }
}

