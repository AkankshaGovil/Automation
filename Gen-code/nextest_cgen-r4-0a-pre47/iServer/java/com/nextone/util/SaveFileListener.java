/**
 * SaveFileListener.java  - a util class which shows a fileChooser dialog
 * and saves the given bytes to the file. Works in conjunction
 * with FileSaver.java interface
 *
 * To use this, simply add a new instance of this as an actionListener to
 * a button
 */
package com.nextone.util;

import javax.swing.*;
import java.awt.*;
import java.awt.event.*;
import java.io.*;
import com.nextone.util.PopMessage;
import com.nextone.util.WindowsAltFileSystemView;



public class SaveFileListener implements ActionListener {
	  private JFileChooser fileChooser;
	  private Component parent;
	  private FileSaver saver;

	  public SaveFileListener (Component p, FileSaver s) {
		 this(p, s, null);
	  }

	  public SaveFileListener (Component p, FileSaver s, JFileChooser c) {
		 if (c == null) {
			c = new JFileChooser(".",new WindowsAltFileSystemView ());
			c.setFileHidingEnabled(false);
		 }
		 parent = p;
		 saver = s;
		 fileChooser = c;
	  }

	  public JFileChooser getFileChooser () {
		 return fileChooser;
	  }

	  public void actionPerformed (ActionEvent ae) {
		 File saveFile = null;
		 while (true) {
			if (fileChooser.showSaveDialog(parent) == JFileChooser.APPROVE_OPTION) {
			   saveFile = fileChooser.getSelectedFile();
			   if (saveFile == null || saveFile.getName() == null)
				  continue; // some bug in Java runtime...
			   else
				  break;
			} else
			   return;
		 }

		 if (saveFile == null)
			return;

		 boolean success = false;
		 try {
			FileOutputStream fos = new FileOutputStream(saveFile);
			saver.writeData(fos);
			fos.close();
			success = true;
		 } catch (FileNotFoundException fe) {
			PopMessage.showError(parent, "File not found", fe.toString() + "\n" + fe.getMessage());
		 } catch (IOException ie) {
			PopMessage.showError(parent, "Error while saving...", ie.toString() + "\n" + ie.getMessage());
		 }

		 if (success)
			PopMessage.showSuccess(parent, "Saved successfully");
	  }

}
