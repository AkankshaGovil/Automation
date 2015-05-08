package com.nextone.util;

import javax.swing.DefaultListModel;
import java.util.Comparator;

/**
 * This class provides a list model to be used with JList, which provides
 * a way to display the list contents in a sorted order.
 *
 * Users of this class must use the addElement method to insert elements
 * that need to be sorted in order. Using the add or set method directly
 * may result in an unsorted content.
 */

public class SortableListModel extends DefaultListModel {
	  private Comparator comparator;

	  /**
	   * a SortableListModel instantiated using this constructor would
	   * require that the elements being added implement the <i>Comparable</i>
	   * interface.
	   */
	  public SortableListModel () {
		 comparator = null;
	  }

	  /**
	   * a SortableListModel instantiated using this constructor would
	   * use the given comparator while sorting the contents of the model
	   *
	   * @param comp the comparator to use while sorting elements
	   */
	  public SortableListModel (Comparator comp) {
		 this.comparator = comp;
	  }

	  /**
	   * adds the specified element in a sorted order in the list
	   *
	   * @param obj element to be added
	   *
	   * @exception IllegalArgumentException if the element needs to be
	   * Comparable and it is not
	   */
	  public void addElement (Object obj) {
		 add(getIndex(obj), obj);
	  }

	  /**
	   * find out at what index would this object be added in the list
	   *
	   * @param obj element to be added
	   *
	   * @exception IllegalArgumentException if the element needs to be
	   * Comparable and it is not
	   */
	  public int getIndex (Object obj) throws IllegalArgumentException {
		 if (comparator == null && !(obj instanceof Comparable))
			throw new IllegalArgumentException("Element (" + obj.getClass().getName() + ") must be an instance of Comparable");

		 Object [] children = toArray();
		 int index = 0;
		 for (int i = 0; i < children.length; i++, index++) {
			if (comparator != null) {
			   if (comparator.compare(children[i], obj) > 0)
				  break;
			} else {
			   if (((Comparable)children[i]).compareTo(obj) > 0)
				  break;
			}
		 }

		 return index;
	  }

}
