package com.nextone.util;

import java.util.Iterator;
import java.util.TreeSet;
import javax.swing.JTree;
import javax.swing.tree.TreePath;
import java.util.Enumeration;

/**
 * This class will take a snapshot of the expansion state
 * of a tree. This state can be retrieved.
 */
public class TreeRowExpander {

  private TreeSet expandedTreeRows = new TreeSet();
  private JTree theTree = null;

  /**
   * 
   */
  public TreeRowExpander(JTree tree) {
    setTree(tree);
  }

  /**
   * Take a snapshot of the expanded rows.
   */
  public void setTree(JTree tree) {
    theTree = tree;
    Enumeration e = tree.getExpandedDescendants(tree.getPathForRow(0));
    if (e != null) {
      while (e.hasMoreElements()) {
        TreePath p = (TreePath)e.nextElement();
        int row = tree.getRowForPath(p);
        expandedTreeRows.add(new Integer(row));
      }
    }
  }

  /**
   * Expand the rows of the Tree.
   */
  public void expandRows() {
    Iterator iter = expandedTreeRows.iterator();
    while (iter.hasNext()) {
      Integer row = (Integer)iter.next();
      theTree.expandRow(row.intValue());
    }
  }
}
