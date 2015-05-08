package com.nextone.util;

import javax.swing.*;
import javax.swing.tree.*;
import java.util.*;

public class SortableMutableTreeNode extends DefaultMutableTreeNode {
  protected Comparator c;
  protected Object repObject;
  protected static Comparator stringComparator = new Comparator () {
      public int compare (Object o1, Object o2) {
        if (o1 instanceof DefaultMutableTreeNode &&
            o2 instanceof DefaultMutableTreeNode) {
          if (((DefaultMutableTreeNode)o1).getUserObject() != null &&
              ((DefaultMutableTreeNode)o1).getUserObject() instanceof Comparable) {
            Comparable c1 = (Comparable)((DefaultMutableTreeNode)o1).getUserObject();
            if (((DefaultMutableTreeNode)o2).getUserObject() != null) {
              return c1.compareTo(((DefaultMutableTreeNode)o2).getUserObject());
            }
          }
        }

	if (o1 instanceof Comparable &&
	    o2 instanceof Comparable) {
	  return ((Comparable)o1).compareTo(o2);
        }

	return o1.toString().compareTo(o2.toString());
      }
    };

  public SortableMutableTreeNode (Object o, Comparator comp) {
    this(o, true, comp);
  }

  public SortableMutableTreeNode (Object o) {
    this(o, true, null);
  }

  public SortableMutableTreeNode (Object representation, Object userObject) {
    this(userObject);
    repObject = representation;
  }

  public SortableMutableTreeNode (Object o, boolean allowsChildren) {
    this(o, allowsChildren, null);
  }

  public SortableMutableTreeNode (Object o, boolean allowsChildren, Comparator comp) {
    super(o, allowsChildren);
    if (comp == null)
      c = stringComparator;
    else
      c = comp;
  }

  /**
   * Add a child to this node.
   */
  public void add (MutableTreeNode newChild) {
    super.add(newChild);
    super.children = getSortedVector(super.children);
  }

  // see what index this child would be added to
  public int getIndex (MutableTreeNode child) {
    Vector v = super.children;
    int i = 0;

    if (v == null)
      return i;

    int limit = v.size();
    for (; v != null && i < limit; i++) {
      if (c.compare(v.get(i), child) > 0)
	break;
    }

    return i;
  }

  public boolean isNodePresent (DefaultMutableTreeNode o) {
    Vector v = super.children;
    if (v == null)
      return false;
    for (int i = 0, limit = v.size(); i < limit; i++) {
      DefaultMutableTreeNode ch = (DefaultMutableTreeNode)v.get(i);
      Object o1 = ch.getUserObject();
      Object o2 = o.getUserObject();
      if (o1 != null && o2 != null) {
	if (o2.equals(o1))
	  return true;
      } else {
	if (ch.toString().equals(o.toString()))
	  return true;
      }
    }
    return false;
  }

  protected Vector getSortedVector (Vector in) {
    Collections.sort(in, c);

    return in;
  }

  public String toString () {
    return repObject == null?super.toString():repObject.toString();
  }

}

