package com.nextone.util;

import javax.swing.tree.*;
import java.awt.dnd.*;
import java.awt.datatransfer.*;
import java.io.*;

/**
 * this class is responsible for providing the data in correct flavor from
 * the tree. the default flavors supported are DEFAULT_MUTABLE_TREENODE_FLAVOR
 * and DataFlavor.stringFlavor
 */
public class TransferableTreeNode extends DefaultMutableTreeNode implements Transferable {
	  public final static DataFlavor DEFAULT_MUTABLE_TREENODE_FLAVOR = new DataFlavor(DefaultMutableTreeNode.class, "Default Mutable Tree Node");
	  private final static DataFlavor defaultFlavors[] = {DEFAULT_MUTABLE_TREENODE_FLAVOR, DataFlavor.stringFlavor};
	  private DataFlavor [] flavors;
	  private DefaultMutableTreeNode data;

	  /**
	   * Create a TransferableTreeNode object which supports both the default
	   * data flavors
	   */
	  public TransferableTreeNode (DefaultMutableTreeNode data) {
		 this(data, null);
	  }

	  /**
	   * Create a TransferableTreeNode object which supports only the given
	   * data flavor (getTransferData() might have to be modified is the
	   * data flavor (df) passed is not one of the 2 default flavors supported
	   */
	  public TransferableTreeNode (DefaultMutableTreeNode data, DataFlavor df) {
		 this.data = data;
		 if (df == null)
			flavors = defaultFlavors;
		 else {
			flavors = new DataFlavor [1];
			flavors[0] = df;
		 }
	  }

	  public DataFlavor[] getTransferDataFlavors () {
		 return (DataFlavor[])flavors.clone();
	  }

	  public Object getTransferData (DataFlavor flavor) throws UnsupportedFlavorException, IOException {
		 Object returnObject;

		 if (flavor.equals(DEFAULT_MUTABLE_TREENODE_FLAVOR)) {
			Object userObject = data.getUserObject();
			if (userObject == null) {
			   returnObject = data;
			} else {
			   returnObject = userObject;
			}
		 } else if (flavor.equals(DataFlavor.stringFlavor)) {
			Object userObject = data.getUserObject();
			if (userObject == null) {
			   returnObject = data.toString();
			} else {
			   returnObject = userObject.toString();
			}
		 } else {
			throw new UnsupportedFlavorException(flavor);
		 }

		 return returnObject;
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

