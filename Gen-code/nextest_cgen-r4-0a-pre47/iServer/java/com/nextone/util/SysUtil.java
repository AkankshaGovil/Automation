package com.nextone.util;

import javax.swing.*;
import java.awt.*;
import java.util.*;
import java.awt.event.*;
import java.io.*;
import java.text.*;
import java.lang.reflect.Array;

public class SysUtil {

  public static final String LINE_SEPARATOR = System.getProperty("line.separator");

  private static final Date date = new Date();

  private SysUtil () {}  // no one should call this

  /**
   * returns the memory used by the JVM. [0] is the total memory
   * used, [1] is the free memory left and [2] is the difference
   * between them.
   */
  public static long[] getMem () {
    long [] rv = new long [3];
    rv[0] = Runtime.getRuntime().totalMemory();
    rv[1] = Runtime.getRuntime().freeMemory();
    rv[2] = rv[0] - rv[1];

    return rv;
  }

  /**
   * prepends the <code>src</code> string with character <code>c</code>
   * until the string is of length <code>len</code>
   */
  public static String prePad (String src, char c, int len) {
    return padchar(src, c, len, true);
  }

  /**
   * appends the <code>src</code> string with character <code>c</code>
   * until the string is of length <code>len</code>
   */
  public static String postPad (String src, char c, int len) {
    return padchar(src, c, len, false);
  }

  /**
   * prepends the <code>src</code> byte array with byte <code>b</code>
   * until the array is of length <code>len</code>
   */
  public static byte [] prePad (byte [] src, byte b, int len) {
    return padbyte(src, b, len, true);
  }

  /**
   * appends the <code>src</code> byte array with byte <code>b</code>
   * until the array is of length <code>len</code>
   */
  public static byte [] postPad (byte [] src, byte b, int len) {
    return padbyte(src, b, len, false);
  }

  /**
   * pads (prepends) the source byte [] with a given byte to satisfy
   * the given length requirement
   */
  private static byte [] padbyte (byte [] src, byte b, int len, boolean prepend) {
    byte [] result = src;

    if (src.length < len) {
      byte [] pad = new byte [len];
      int padlen = len - src.length;
      int i = 0;
      if (!prepend) {
	System.arraycopy(src, 0, pad, 0, src.length);
	i += src.length;
      }
      Arrays.fill(pad, i, padlen+i, b);
      i += padlen;
      if (prepend) {
	System.arraycopy(src, 0, pad, i, src.length);
      }
      result = pad;
    }

    return result;
  }

  /**
   * pads (prepends) the source string with a given character to satisfy
   * the given length requirement
   */
  private static String padchar (String src, char c, int len, boolean prepend) {
    String result = src;

    if (src.length() < len) {
      int padlen = len - src.length();
      StringBuffer pad = new StringBuffer();
      if (!prepend)
	pad.append(src);
      for (int i = 0; i < padlen; i++)
	pad.append(c);
      if (prepend)
	pad.append(src);
      result = pad.toString();
    }

    return result;
  }

  /**
   * The source string should be numeric. Increase the given source string by one.
   *
   */

  public static String autoIncrease(String src){
    int length    =   src.length();
    String   first    =  "";
    String   second   =   src;

    if(length     ==  0)
      return "";

    if(length >  16){
      second    =  src.substring(length -16); 
      first     =   src.substring(0,length -16);
    }
            
    long l    =   Long.parseLong(second);
    l++;
    String temp   =   (new Long(l)).toString();

    if(temp.length()  <  second.length())
      temp    =   SysUtil.prePad(temp, '0', second.length());

    if(temp.length()  >16){
      second    =   temp.substring(1);
      first =   autoIncrease(first);
    }
    else
      second    =   temp;

            
    return (first + second);

  }

  /**
   * interrupt the calling thread after the specified amount of
   * time (in millisecs)
   */
  public static javax.swing.Timer interruptAfter (int delay) {
    return SysUtil.interruptAfter(delay, Thread.currentThread());
  }

  /**
   * interrupt the thread <code>t</code> after the specified amount of
   * time (in millisecs)
   */
  public static javax.swing.Timer interruptAfter (int delay, Thread t) {
    final Thread threadToInterrupt = t;
    ActionListener al = new ActionListener () {
	public void actionPerformed (ActionEvent ae) {
	  threadToInterrupt.interrupt();
	}
      };

    javax.swing.Timer timer = new javax.swing.Timer(delay, al);
    timer.setInitialDelay(delay);
    timer.setRepeats(false);
    timer.setCoalesce(true);
    timer.start();

    return timer;
  }

  /**
   * a focus listener for buttons, which upon receiving focus, sets
   * it as the default button for the root pane
   */
  public static FocusListener getDefaultButtonFocusListener () {
    return new FocusListener () {
	public void focusGained (FocusEvent fe) {
	  Component c = fe.getComponent();
	  if (c instanceof JButton)
	    ((JButton)c).getRootPane().setDefaultButton((JButton)c);
	}

	public void focusLost (FocusEvent fe) {
	  Component c = fe.getComponent();
	  if (c instanceof JButton)
	    ((JButton)c).getRootPane().setDefaultButton(null);
	}
      };
  }

  /**
   * returns true if the <code>curDate</code> falls between <code>startDate
   * </code> and <code>endDate</code>
   */
  public static boolean isDateInRange (Date curDate, Date startDate, Date endDate) {
    if (startDate != null && startDate.before(curDate) &&
	(endDate == null || endDate.after(curDate)))
      return true;

    return false;
  }

  /**
   * substracts the specified amount of time (<code>val</code>) from the
   * date <code>orig</code>, where <code>type</code> could be one
   * of Calendar.YEAR, Calendar.MONTH, Calendar.DATE (for specifying
   * days), Calendar.HOUR, Calendar.MINUTE, Calendar.SECOND and 
   * Calendar.MILLISECOND
   *
   * @exception IllegalArgumentException if type is not one of
   * the allowed values
   * @return the modified Date
   */
  public static Date substractFromDate (Date orig, int type, int val) {
    return modifyDate(orig, type, -val);
  }

  /**
   * adds the specified amount of time (<code>val</code>) to the
   * date <code>orig</code>, where <code>type</code> could be one
   * of Calendar.YEAR, Calendar.MONTH, Calendar.DATE (for specifying
   * days), Calendar.HOUR, Calendar.MINUTE, Calendar.SECOND and 
   * Calendar.MILLISECOND
   *
   * @exception IllegalArgumentException if type is not one of
   * the allowed values
   * @return the modified Date
   */
  public static Date addToDate (Date orig, int type, int val) {
    return modifyDate(orig, type, val);
  }

  private static Date modifyDate (Date orig, int type, int val) {
    Calendar cal = Calendar.getInstance();
    cal.setTime(orig);

    switch (type) {
    case Calendar.YEAR:
      cal.roll(Calendar.YEAR, val);
      break;
    case Calendar.MONTH:
      cal.roll(Calendar.MONTH, val);
      break;
    case Calendar.DATE:
      cal.roll(Calendar.HOUR, 24*val);
      break;
    case Calendar.HOUR:
      cal.roll(Calendar.HOUR, val);
      break;
    case Calendar.MINUTE:
      cal.roll(Calendar.MINUTE, val);
      break;
    case Calendar.SECOND:
      cal.roll(Calendar.SECOND, val);
      break;
    case Calendar.MILLISECOND:
      cal.roll(Calendar.MILLISECOND, val);
      break;
    default:
      throw new IllegalArgumentException("unknown type (" + type + ")");
    }

    return cal.getTime();
  }

  /**
   * copies <code>total</code> bytes from input stream to output stream
   * if <code>total</code> is zero, reads until EOF is reached
   *
   * @param is the input stream to read from
   * @param os the output stream to write to
   * @param total the total number of bytes to read
   *
   * @exception IOException if any exception happens during read/write
   * @exception ParseException if the number of bytes read is less than 
   *            the <code>total</code> bytes requested
   */
  public static void copyStream (InputStream is, OutputStream os, long total) throws IOException, ParseException {
    byte [] buf = new byte [1024];
    long count = 0;

    if (total < 0)
      total = Long.MAX_VALUE;

    for (int len = 0, tries = 0; count < total; count += len) {
      len = is.read(buf);
      if (len == -1) {
	// try a few more times... if the other side is not a
	// BufferedOutputStream, we may get some intermediate
	// end of stream indications
	if (tries++ > 10)
	  break;
	else {
	  len = 0;
	  continue;
	}
      }
      tries = 0;
      os.write(buf, 0, len);
    }
    os.flush();

    if (total != Long.MAX_VALUE && count < total)
      throw new ParseException("Attempted to read " + total + " bytes, read only " + count + " bytes", 0);
  }
	   
  /**
   * copies the input stream to the output stream until EOF
   * same as calling copyStream(is, os, -1)
   *
   *@see SysUtil.copyStream(InputStream is, OutputStream os, long total)
   */
  public static void copyStream (InputStream is, OutputStream os) throws IOException {
    try {
      SysUtil.copyStream(is, os, -1);
    } catch (ParseException pe) {} // this exception not applicable for us
  }

  /**
   * copies the src file to the destination file (unix command: cp srcFile dstFile)
   *
   * @param srcFile the source file
   * @param dstFile the destination file
   */
  public static void copyFile (File srcFile, File dstFile) throws IOException {
    FileInputStream fis = new FileInputStream(srcFile);
    FileOutputStream fos = new FileOutputStream(dstFile);
    copyStream(fis, fos);
    fis.close();
    fos.close();
  }

  /**
   * copies the src file to the destination file (unix command: cp srcFile dstFile)
   *
   * @param srcFile the source file
   * @param dstFile the destination file
   */
  public static void copyFile (String srcFile, String dstFile) throws IOException {
    copyFile(new File(srcFile), new File(dstFile));
  }

  /**
   * moves the src file to the destination file (unix command: mv srcFile dstFile)
   *
   * @param srcFile the source file
   * @param dstFile the destination file
   * @param isDstDir is the destination specified is actually a directory?
   */
  public static void moveFile (String srcFile, String dstFile, boolean isDstDir) throws IOException {
    SysUtil.moveFile(new File(srcFile), new File(dstFile), isDstDir);
  }

  /**
   * moves the src file to the destination file (unix command: mv srcFile dstFile)
   *
   * @param srcFile the source file
   * @param dstFile the destination file
   * @param isDstDir is the destination specified is actually a directory?
   */
  public static void moveFile (File srcFile, File dstFile, boolean isDstDir) throws IOException {
    FileInputStream fis = new FileInputStream(srcFile);
    FileOutputStream fos = isDstDir?new FileOutputStream(dstFile.getPath() + File.separator + srcFile.getName()):new FileOutputStream(dstFile);
    copyStream(fis, fos);
    fis.close();
    fos.close();
    srcFile.delete();
  }

  /**
   * renames the src file as the dst file (unix command: mv srcFile dstFile)
   *
   * @param srcFile the source file
   * @param dstFile the destination file
   */
  public static void renameFile (File srcFile, File dstFile) throws IOException {
    if (!srcFile.renameTo(dstFile))
      SysUtil.moveFile(srcFile, dstFile, false);
  }

  /**
   * deletes the file (unix command: rm file)
   * if 'file' is a directory, it should be empty in order to be deleted
   *
   * @param file the file to be removed
   */
  public static void deleteFile (String file) throws IOException {
    SysUtil.deleteFile(new File(file));
  }

  /**
   * deletes the file (unix command: rm file)
   * if 'file' is a directory, it should be empty in order to be deleted
   *
   * @param file the file to be removed
   */
  public static void deleteFile (File file) throws IOException {
    file.delete();
  }

  /**
   * return the RGB string for the color
   */
  public static String getRGBString (Color c) {
    StringBuffer sb = new StringBuffer();
    String r = Integer.toHexString(c.getRed());
    if (r.length() == 1)
      sb.append("0");
    sb.append(r);
    String g = Integer.toHexString(c.getGreen());
    if (g.length() == 1)
      sb.append("0");
    sb.append(g);
    String b = Integer.toHexString(c.getBlue());
    if (b.length() == 1)
      sb.append("0");
    sb.append(b);
    return sb.toString();
  }

  /**
   * return the Color object for the given RGB string
   * (assumes the RGB string in format RRGGBB)
   */
  public static Color getColorForRGB (String rgbString) {
    return new Color(Integer.parseInt(rgbString.substring(0, 2), 16),
		     Integer.parseInt(rgbString.substring(2, 4), 16),
		     Integer.parseInt(rgbString.substring(4, 6), 16));
  }

  /**
   * takes a String and compresses the spaces in it, i.e., if there were
   * multiple consecutive spaces, it will be replaced by one single space
   * this also replaces tab characters with space
   *
   * @exception IOException if any error occurs reading the input string
   */
  public static String compressSpaces (String data) throws IOException {
    // replace tab with a space
    data = data.replace('\t', ' ');

    StringBuffer sb = new StringBuffer();
    BufferedReader br = new BufferedReader(new StringReader(data));
    String line = null;
    while ((line = br.readLine()) != null) {
      line = compressChars(line, ' ');
      if (line.equals(""))
	continue;
      sb.append(line);
      sb.append(SysUtil.LINE_SEPARATOR);
    }

    return sb.toString();
  }

  public static String compressChars (String line, char ch) {
    // look for atleast 2 chars in a row
    String chars = "" + ch + ch;
    int si = line.indexOf(chars, 0);
    if (si == -1)
      return line;  // nothing more to do

    StringBuffer sb = new StringBuffer();
    int ei = si+1;
    int len = line.length();
    for (; (ei < len) && (line.charAt(ei) == ch); ei++);
    sb.append(line.substring(0, si+1));
    if (ei != len)
      sb.append(line.substring(ei, len));

    // recursively eliminate chars
    return SysUtil.compressChars(sb.toString(), ch);
  }

  /**
   * returns true if the first string contains any of the characters
   * from the second string, false otherwise
   *
   * @param source the string to check on
   * @param chars the string of characters to check on
   */
  public static boolean ifContainsAnyChar (String source, String chars) {
    char [] ca = chars.toCharArray();
    for (int i = 0; i < ca.length; i++) {
      if (source.indexOf(ca[i]) != -1)
	return true;
    }

    return false;
  }

  /**
   * returns the number of lines ('\n' characters) in the given string
   *
   * @param data the data to check on
   */
  public static int getNumLines (String data) {
    return SysUtil.getNumOccurences('\n', data);
  }

  /**
   * returns the number of occurences of a given character in the given
   * string
   *
   * @param ch the character to count
   * @param data the source data
   */
  public static int getNumOccurences (char ch, String data) {
    int count = 0;
    int index = 0;
    for (String subs = data; subs.length() > 0; count++, subs = subs.substring(index+1)) {
      index = subs.indexOf(ch);
      if (index == -1)
	break;
    }

    return count;
  }

  /**
   * Returns a String containing the hexadecimal representation of
   * the byte array passed.
   * Each byte in the array is converted to the equivalent hexadecimal
   *
   * @param ba the byte array to be converted to hexadecimal
   */
  public static String toHex (byte [] ba) {
    StringBuffer sb = new StringBuffer(2*ba.length);
    for (int i = 0; i < ba.length; i++)
      sb.append(Integer.toHexString((int)ba[i] & 0x00ff));

    return sb.toString();
  }


  /**
   * converts the given short value to a string representation
   * (and non-numeric character is returned as is)
   */
  public static String convertToString (short val) {
    if (val == -1)
      return "";

    if (val >= 0)
      return Short.toString(val);

    // either the $ or -
    char [] c = new char[1];
    c[0] = (char)(val + '0');
    return new String(c);
  }

  /**
   * checks if the current directory structure exists, and if not
   * creates it
   *
   * @param dir the directory to be created
   *
   * @exception IOException if there are any underlying exceptions
   * while creating the directories
   *
   * @return a File object pointing to the given directory
   */
  public static File createDirectory (String dir) throws IOException {
    File f = new File(dir);

    if (f.exists())
      return f;

    if (!f.mkdirs())
      throw new IOException("Could not create the given directory (" + dir + ")");

    return f;
  }

  /**
   * given an input string and a length, truncates the string to the
   * given length, appends "..." to it and returns it
   *
   * @param str the original String
   * @param len the length to display
   *
   * @return the formatted string
   */
  public static String truncateString (String str, int len) {
    StringBuffer sb = new StringBuffer();
    if (str.length() > (len + 3)) {
      sb.append(str.substring(0, len));
      sb.append("...");
    } else
      sb.append(str);

    return sb.toString();
  }

  /**
   * given an input string and a character to escape, returns a string
   * with the proper escape characters (\)
   *
   * @param str the input string
   * @param ch the character to escape
   *
   * @return the escaped string
   */
  public static String escape (String str, char ch) {
    StringBuffer sb = new StringBuffer();
    for (int i = 0; str != null && i < str.length(); i++) {
      char c = str.charAt(i);
      if (c == ch)
	sb.append("\\");
      sb.append(c);
    }

    return sb.toString();
  }

  /**
   * given a String, encodes all special HTML chars in it and returns
   * the encoded string
   *
   * @param str the string to be encoded
   * 
   * @return the encoded string
   */
  public static String HTMLEncode (String str) {
    if((str != null) && (str.length() != 0)) { 
      StringBuffer newStr = new StringBuffer(); 

      for(int i=0; i < str.length() ; i++) { 
	if(str.charAt(i) == '"') 
	  newStr.append("&quot;"); 
	else if(str.charAt(i) == '\n') 
	  newStr.append("<BR>"); 
	else if(str.charAt(i) == '\r') 
	  newStr.append(""); 
	else if(str.charAt(i) == '\'') 
	  newStr.append("&#039;"); 
	else if(str.charAt(i) == '>') 
	  newStr.append("&gt;"); 
	else if(str.charAt(i) == '<') 
	  newStr.append("&lt;"); 
	else if(str.charAt(i) == ' ') 
	  newStr.append("&nbsp;"); 
	else 
	  newStr.append(str.charAt(i)); 
      }

      return newStr.toString(); 
    } 

    return str; 
  } 

  /**
   * given a String, decodes all special HTML chars in it and returns
   * the decoded string
   *
   * @param str the string to be decoded
   * 
   * @return the decoded string
   */
  public static String HTMLDecode (String str) {
    if((str != null) && (str.length() != 0)) { 
      StringBuffer newStr = new StringBuffer(str);
      String [] encodes = { "&quot;", "<BR>", "&#039;", "&gt;",
			    "&lt;", "&nbsp;",
      };
      char [] decodes = { '\"', '\n', '\'', '>',
			  '<', ' ', 
      };

      for (int i = 0; i < encodes.length; i++) {
	int index = 0;
	while ((index = newStr.toString().indexOf(encodes[i], index)) != -1) {
	  newStr.replace(index, index + encodes[i].length(), String.valueOf(decodes[i]));
	}
      }

      return newStr.toString(); 
    } 

    return str; 
  }

  /**
   * convert a C string (null terminated) into a java UTF string
   *
   * @param the byte array containing the C string
   * @return the UTF string equivalent
   */
  public static String CStringToUTF (byte [] cstr) {
    int count = 0;
    for (; count < cstr.length && cstr[count] != 0; count++);
    if (count < cstr.length) {
      byte [] tmp = new byte [count];
      System.arraycopy(cstr, 0, tmp, 0, count);
      cstr = tmp;
    }

    return new String(cstr);
    /*
      StringBuffer sb = new StringBuffer(cstr.length+1);
      for (int i = 0; i < cstr.length; i++) {
      if (cstr[i] == 0)
      break;
      else
      sb.append(cstr[i]);
      }

      Logger.debug(new String(cstr) + "-->" + sb.toString());
      return sb.toString();
    */
  }

  /**
   * convert an UTF string into a C string and return it as a byte array
   *
   * @param str the UTF string
   * @return the byte array containing the C string
   */
  public static byte [] UTFToCString (String str) {
    int len = str.length();
    byte [] cstr = new byte [len+1];
    System.arraycopy(str.getBytes(), 0, cstr, 0, len);
    cstr[len] = 0;

    return cstr;
  }

  /**
   * truncates the leading spaces from the given string
   */
  public static String truncateLeadingSpaces (String str) {
    int len = str.length();
    int i = 0;
    for (; i < len; i++)
      if (str.charAt(i) > '\u0020')
	break;

    return str.substring(i, len);
  }

  /**
   * truncates traling spaces from the given string
   */
  public static String truncateTrailingSpaces (String str) {
    int len = str.length();
    int i = len;
    for (; i >= 0; i--)
      if (str.charAt(i) > '\u0020')
	break;

    return str.substring(0, i+1);
  }

  /**
   * truncates leading and trailing spaces from the given string
   */
  public static String truncateEnclosingSpaces (String str) {
    return str.trim();
  }

  /**
   * adds another element to the given string array, appends the given string to the
   * end of the array and returns the new array
   *
   * @param curArray the array to append this string to, null will create a new array
   * @param str the string to be appended
   */
  public static String [] createStringArray (String [] curArray, String str) {
    String [] newArray;

    if (curArray == null)
      newArray = new String [1];
    else {
      newArray = new String [curArray.length + 1];
      System.arraycopy(curArray, 0, newArray, 0, curArray.length);
    }

    newArray[newArray.length - 1] = new String(str);

    return newArray;
  }

  /**
   * adds another element to the given int array, appends the given int value to the
   * end of the array and returns the new array
   *
   * @param curArray the array to append this int to, null will create a new array
   * @param ival the int to be appended
   */
  public static int [] createIntArray (int [] curArray, int ival) {
    int [] newArray;

    if (curArray == null)
      newArray = new int [1];
    else {
      newArray = new int [curArray.length + 1];
      for (int i = 0; i < curArray.length; newArray[i] = curArray[i++]);
    }

    newArray[newArray.length - 1] = ival;

    return newArray;
  }


  /**
   * Adds another element to the given object array, appends the given object to the
   * end of the array and returns the new array. The runtime type of the new array
   * returned is the same as the once passed in.
   *
   * @param curArray the array to append this object to
   * @param obj the object to be appended
   */
  public static Object [] createObjectArray (Object [] curArray, Object obj) {
    Object [] newArray;

    if (curArray == null)
      newArray = (Object [])Array.newInstance(obj.getClass(), 1);
    else {
      newArray = (Object [])Array.newInstance(obj.getClass(), curArray.length + 1);
      System.arraycopy(curArray, 0, newArray, 0, curArray.length);
    }

    newArray[newArray.length - 1] = obj;

    return newArray;
  }


  /**
   * returns the given 'value' rounded down to the nearest 'near'
   */
  public static int nearestDown (int value, int near) {
    return nearest(value, near, false);
  }


  /**
   * returns the given 'value' rounded up to the nearest 'near'
   */
  public static int nearestUp (int value, int near) {
    return nearest(value, near, true);
  }


  /**
   * returns the given value rounded to the nearest 'near', rounds up if that results in the
   * smallest change, rounds down if that results in the smallest change
   */
  public static int nearest (int value, int near) {
    int remainder = value%near;
    return nearest(value, near, ((near - remainder) <= remainder));
  }


  private static int nearest (int value, int near, boolean up) {
    int remainder = value%near;
    if (remainder != 0)
      value += ((up?near:0) - remainder);
    return value;
  }


  /**
   * returns the content of the file as a String
   */
  public static String readFile (String fileName) throws IOException {
    return readFile(new File(fileName));
  }


  public static String readFile (File file) throws IOException {
    BufferedReader br = new BufferedReader(new FileReader(file), (int)file.length());
    StringBuffer sb = new StringBuffer();
    String line = null;
    while ((line = br.readLine()) != null) {
      sb.append(line);
      sb.append(SysUtil.LINE_SEPARATOR);
    }

    return sb.toString();
  }


  /**
   * returns a Date object containing the current time
   */
  public static Date getDate () {
    return SysUtil.getDate(System.currentTimeMillis());
  }


  /**
   * returns the Date object containing the time specified in the argument
   */
  public static Date getDate (long millis) {
    date.setTime(millis);
    return date;
  }

  /**
   * returns a String that is sized to the specified length (a truncated string
   * will have two dots attached to it)
   */
  public static String getSizedString (String str, int size) {
    StringBuffer sb = new StringBuffer();
    int l = str.length();

    if (l < size) {
      sb.append(str);
      int numSpaces = size - l;
      while (numSpaces-- > 0)
        sb.append(" ");
    } else if (l > size) {
      sb.append(str.substring(0, size-2));
      sb.append("..");
    } else {
      sb.append(str);
    }

    return sb.toString();
  }

}
