package com.nextone.util;

import java.text.ParseException;
import java.util.*;

/**
 * The class extracts the numeric portion of the passed string. It does this
 * by starting at the end of the string, and working backwards. Thus the 
 * numeric portion contained at the furthest in the string would be the
 * digit returned by this class. Any appending and prepending strings
 * are also extracted.
 * e.g.  Original string - abcd01-0001-w
 *       getDigit() = 1
 *       getDigitLength() = 4
 *       getPrependString() = abcd01-
 *       getAppendString() = -w
 */
public class DigitExtractor {
	  private int num, len;
	  private String pr, ap;

	  public DigitExtractor (String orig) throws ParseException {
		 int startIndex = -1;
		 int endIndex = -1;
		 boolean endSeen = false;
		 int i = orig.length()-1;
		 for (; i >= 0; i--) {
			char c = orig.charAt(i);
			if (!endSeen && 
				Character.getType(c) == Character.DECIMAL_DIGIT_NUMBER) {
			   endSeen = true;
			   endIndex = i;
			}
			if (endSeen &&
				Character.getType(c) != Character.DECIMAL_DIGIT_NUMBER) {
			   startIndex = i+1;
			   break;
			}
		 }
		 if (i == -1) {
			if (endSeen)
			   startIndex = 0;
			else
			   throw new ParseException("String contained no digits", 0);
		 }

		 // now we have the start and end index if the digits in the string
		 // extract the variables needed
		 if (startIndex == 0)
			pr = "";
		 else
			pr = orig.substring(0, startIndex);

		 String dg = orig.substring(startIndex, endIndex+1);
		 len = dg.length();
		 num = Integer.parseInt(dg);

		 if (endIndex == orig.length()-1)
			ap = "";
		 else
			ap = orig.substring(endIndex+1, orig.length());
	  }

	  public int getDigit () {
		 return num;
	  }

	  public int getDigitLength () {
		 return len;
	  }

	  public String getPrependString () {
		 return pr;
	  }

	  public String getAppendString () {
		 return ap;
	  }
}

