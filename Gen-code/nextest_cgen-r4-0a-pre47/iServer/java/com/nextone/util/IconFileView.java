package com.nextone.util;

import java.io.File;
import javax.swing.*;
import javax.swing.filechooser.*;
import java.util.*;

/**
 * this is a generic class that shows little icons for the iView files
 * in a file chooser dialog
 */
public class IconFileView extends FileView {
	  private Map iconMap, descMap;

	  /**
	   * creates an instance of IconFileView that does nothing
	   */
	  public IconFileView () {
		 this(null, "", null);
	  }

	  /**
	   * creates an instance of IconFileView that shows the specified icon
	   * and description for the given extension
	   *
	   * @param ext the file extension (case independent)
	   * @param desc the description to be displayed for this extension
	   * @param icon the icon to be displayed for this extension
	   */
	  public IconFileView (String ext, String desc, Icon icon) {
		 String [] exts = null;
		 String [] descs = null;
		 Icon [] icons = null;
		 if (ext != null) {
			exts = new String [1];
			exts[0] = ext;
			descs = new String [1];
			descs[0] = desc;
			icons = new Icon [1];
			icons[0] = icon;
		 }

		 init(exts, descs, icons);
	  }

	  /**
	   * creates an instance of IconFileView that shows the specified icons
	   * and descriptions for the given extensions
	   *
	   * @param exts the file extensions (case independent)
	   * @param descs the descriptions to be displayed for the extensions
	   * @param icons the icons to be displayed for this extensions
	   */
	  public IconFileView (String [] exts, String [] descs, Icon [] icons) {
		 init(exts, descs, icons);
	  }

	  private void init (String [] exts, String [] descs, Icon [] icons) {
		 iconMap = Collections.synchronizedMap(new HashMap());
		 descMap = Collections.synchronizedMap(new HashMap());
		 for (int i = 0; exts != null && i < exts.length; i++) {
			addExtension(exts[i], descs[i], icons[i]);
		 }
	  }

	  /**
	   * adds an extension and corresponding description/icon
	   *
	   * @param ext the file extension (case independent)
	   * @param desc the description to be displayed for this extension
	   * @param icon the icon to be displayed for this extension
	   */
	  public void addExtension (String ext, String desc, Icon icon) {
		 String lower = ext.toLowerCase();
		 String upper = ext.toUpperCase();
		 iconMap.put(lower, icon);
		 iconMap.put(upper, icon);
		 descMap.put(lower, desc);
		 descMap.put(upper, desc);
	  }

	  /**
	   * removes the given extension from previous description/icon
	   * associations
	   */
	  public void removeExtension (String ext) {
		 String lower = ext.toLowerCase();
		 String upper = ext.toUpperCase();
		 iconMap.remove(lower);
		 iconMap.remove(upper);
		 descMap.remove(lower);
		 descMap.remove(upper);
	  }

	  /**
	   * returns the icon associated with this extension
	   */
	  public Icon getExtensionIcon (String ext) {
		 return (Icon)iconMap.get(ext.toLowerCase());
	  }

	  /**
	   * returns the description associated with this extension
	   */
	  public String getExtensionDescription (String ext) {
		 return (String)descMap.get(ext.toLowerCase());
	  }

	  // Following methods are implemented to satisfy FileView abstract class

	  public String getName (File f) {
		 return null;  // let the L&F FileView figure this out
	  }

	  public String getDescription (File f) {
		 return null;  // let the L&F FileView figure this out
	  }

	  public Boolean isTraversable (File f) {
		 return null;  // let the L&F FileView figure this out
	  }

	  public String getTypeDescription (File f) {
		 return (String)descMap.get(ExtensionFileFilter.getExtension(f));
	  }

	  public Icon getIcon (File f) {
		 return (Icon)iconMap.get(ExtensionFileFilter.getExtension(f));
	  }
}

