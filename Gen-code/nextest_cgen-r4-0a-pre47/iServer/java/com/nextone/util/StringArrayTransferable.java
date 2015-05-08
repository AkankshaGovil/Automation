package com.nextone.util;

import javax.swing.tree.*;
import java.awt.dnd.*;
import java.awt.datatransfer.*;
import java.io.*;

/**
 * this class represents a Transferable object support for String arrays
 */
public class StringArrayTransferable implements Transferable {
  public final static DataFlavor STRING_ARRAY_FLAVOR = new DataFlavor(String [].class, "String Array");
  private final static DataFlavor [] defaultFlavors = {STRING_ARRAY_FLAVOR};
  private DataFlavor [] flavors;
  private String [] data;

  /**
   * Create a StringArrayTransferable object for the given String array
   */
  public StringArrayTransferable (String [] data) {
    this.data = data;
    flavors = defaultFlavors;
  }

  public DataFlavor[] getTransferDataFlavors () {
    return (DataFlavor[])flavors.clone();
  }

  public Object getTransferData (DataFlavor flavor) throws UnsupportedFlavorException {
    if (flavor.equals(STRING_ARRAY_FLAVOR))
      return data;

    throw new UnsupportedFlavorException(flavor);
  }

  public boolean isDataFlavorSupported (DataFlavor flavor) {
    boolean returnValue = false;
    for (int i = 0; i < flavors.length; i++) {
      if (flavor.equals(flavors[i]))
	return true;
    }
    return false;
  }

}

