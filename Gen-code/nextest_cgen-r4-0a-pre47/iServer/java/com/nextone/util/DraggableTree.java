package com.nextone.util;

import java.awt.*;
import java.awt.dnd.*;
import java.awt.datatransfer.*;
import java.util.*;
import java.io.*;
import javax.swing.*;
import javax.swing.tree.*;

public class DraggableTree extends JTree implements DragSourceListener, DragGestureListener {
  protected DragSource dragSource;
  protected DraggableTreeProvider provider;
  protected JPopupMenu popupMenu;

  public DraggableTree () {
    this(null,null, null);
  }

  public DraggableTree (TreeModel tm, DraggableTreeProvider provider) {
    this(tm, provider, null);
  }

  public DraggableTree (TreeModel tm, DraggableTreeProvider provider, JPopupMenu pm) {
    super(tm);
    this.provider = provider;
    this.popupMenu = pm;

    dragSource = DragSource.getDefaultDragSource();
    dragSource.createDefaultDragGestureRecognizer(this, DnDConstants.ACTION_LINK, this);
  }

  public synchronized void setTreeModel (TreeModel tm) {
    setModel(tm);
  }  
  
  public synchronized void setTreeProvider (DraggableTreeProvider provider) {
    this.provider = provider;
  } 
  
  public synchronized void setPopupMenu (JPopupMenu pm) {
    popupMenu = pm;
  }

  public synchronized JPopupMenu getPopupMenu () {
    return popupMenu;
  }

  public void dragGestureRecognized (DragGestureEvent dge) {
    if (getSelectionPath() == null ||
	(popupMenu != null && popupMenu.isVisible()))
      return;

    DefaultMutableTreeNode selected = (DefaultMutableTreeNode)getSelectionPath().getLastPathComponent();

    if (provider.canDrag(selected)) {
      Image image = provider.dragImage(selected);
      Transferable t = provider.getTransferable(selected);
      try {
	if (image != null && DragSource.isDragImageSupported()) {
	  dragSource.startDrag(dge, DragSource.DefaultLinkDrop, image, new Point(), t, this);
	} else {
	  dragSource.startDrag(dge, DragSource.DefaultLinkDrop, t, this);
	}
      } catch (InvalidDnDOperationException idoe) {
	Logger.error("error while dragging in tree", idoe);
      }
    }
  }

  public void dragDropEnd (DragSourceDropEvent dsde) {
    provider.dragDropEnd(dsde);
  }

  public void dragEnter (DragSourceDragEvent dsde) {
  }

  public void dragExit (DragSourceEvent dse) {
  }

  public void dragOver (DragSourceDragEvent dsde) {
  }

  public void dropActionChanged (DragSourceDragEvent dsde) {
  }


  public interface DraggableTreeProvider {
    // informs whether this node can be dragged
    public boolean canDrag (DefaultMutableTreeNode dmtn);
    // supplies the icon image to be used while dragging
    public Image dragImage (DefaultMutableTreeNode dmtn);
    // supplies the Transferable object
    public Transferable getTransferable (DefaultMutableTreeNode dmtn);
    // informs end of drag and drop
    public void dragDropEnd (DragSourceDropEvent dsde);
  }

}
