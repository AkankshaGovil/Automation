package com.nextone.util;

import java.awt.*;
import java.awt.event.*;
import javax.swing.*;
import javax.swing.event.*;
import javax.swing.text.*;
import java.beans.*;

/**
 * This class contains methods that can be used to remove event listeners
 * from various java objects. These methods are especially useful when
 * we no longer hold references to the listeners atatched to those objects.
 *
 * Suns's java seems to have a memory leak problem when a listener is 
 * not de-registered when it's parent is no longer used.
 */
public class RemoveListener {

  private RemoveListener () {}  // all methods in this class are static

  /**
   * this generic method tries to see what is the actual class
   * of the Component passed in and calls the right removelistener
   * method
   *
   * @see removeJComboBoxListeners for JComboBox
   * @see removeJListListeners for JList
   * @see removeJMenuItemListeners for JMenuItem
   * @see removeJMenuListeners for JMenu
   * @see removeJPopupMenuListeners for JPopupMenu
   * @see removeJProgressBarListeners for JProgressBar
   * @see removeJTabbedPaneListeners for JTabbedPane
   * @see removeJInternalFrameListeners for JInternalFrame
   * @see removeJTextFieldListeners for JTextField
   * @see removeJTextComponentListeners for JTextComponent
   * @see removeJTreeListeners for JTree
   * @see removeJViewportListeners for JViewport
   * @see removeButtonListeners for AbstractButton
   * @see removeJComponentListeners for JComponent
   * @see removeAllComponentListeners for Component
   */
  public static void removeListeners (Component comp) {
    // we have to use instanceof instead of getClass().equals()
    // because we might have subclassed these
    if (comp instanceof JComboBox)
      removeJComboBoxListeners((JComboBox)comp);
    else if (comp instanceof JList)
      removeJListListeners((JList)comp);
    else if (comp instanceof JMenu)
      removeJMenuListeners((JMenu)comp);
    else if (comp instanceof JMenuItem)
      removeJMenuItemListeners((JMenuItem)comp);
    else if (comp instanceof JPopupMenu)
      removeJPopupMenuListeners((JPopupMenu)comp);
    else if (comp instanceof JProgressBar)
      removeJProgressBarListeners((JProgressBar)comp);
    else if (comp instanceof JTabbedPane)
      removeJTabbedPaneListeners((JTabbedPane)comp);
    else if (comp instanceof JInternalFrame)
      removeJInternalFrameListeners((JInternalFrame)comp);
    else if (comp instanceof JTextField)
      removeJTextFieldListeners((JTextField)comp);
    else if (comp instanceof JTextComponent)
      removeJTextComponentListeners((JTextComponent)comp);
    else if (comp instanceof JTree)
      removeJTreeListeners((JTree)comp);
    else if (comp instanceof JViewport)
      removeJViewportListeners((JViewport)comp);
    else if (comp instanceof AbstractButton)
      removeButtonListeners((AbstractButton)comp);
    else if (comp instanceof JComponent)
      removeJComponentListeners((JComponent)comp);
    else if (comp instanceof Window)
      removeWindowListeners((Window)comp);
    else if (comp instanceof Container)
      removeContainerListeners((Container)comp);
    else
      removeAllComponentListeners(comp);
  }

  /**
   * removes all the listeners associated with this JComboBox
   * removes ActionListener, ItemListener and listeners
   * associated with a JComponent
   *
   * @see SysUtil.removeJComponentListeners
   */
  public static void removeJComboBoxListeners (JComboBox cb) {
    removeActionListeners(cb);
    removeItemListeners(cb);
    removeJComponentListeners(cb);
  }

  public static void removeActionListeners (JComboBox cb) {
    ActionListener [] al = (ActionListener [])cb.getListeners(ActionListener.class);
    for (int i = 0; i < al.length; cb.removeActionListener(al[i++]));
  }

  /**
   * removes all the listeners associated with this JList
   * removes ListSelectionListener and listeners associated with a JComponent
   *
   * @see SysUtil.removeJComponentListeners
   */
  public static void removeJListListeners (JList list) {
    removeListSelectionListeners(list);
    removeJComponentListeners(list);
  }

  public static void removeListSelectionListeners (JList list) {
    ListSelectionListener [] lsl = (ListSelectionListener [])list.getListeners(ListSelectionListener.class);
    for (int i = 0; i < lsl.length; list.removeListSelectionListener(lsl[i++]));
  }

  /**
   * removes all the listeners associated with this JMenuItem
   * removes MenuDragMouseListener, MenuKeyListener and listeners
   * associated with an AbstractButton
   *
   * @see SysUtil.removeButtonListeners
   */
  public static void removeJMenuItemListeners (JMenuItem jmi) {
    removeMenuDragMouseListeners(jmi);
    removeMenuKeyListeners(jmi);
    removeButtonListeners(jmi);
  }

  public static void removeMenuDragMouseListeners (JMenuItem jmi) {
    MenuDragMouseListener [] mdml = (MenuDragMouseListener [])jmi.getListeners(MenuDragMouseListener.class);
    for (int i = 0; i < mdml.length; jmi.removeMenuDragMouseListener(mdml[i++]));
  }

  public static void removeMenuKeyListeners (JMenuItem jmi) {
    MenuKeyListener [] mkl = (MenuKeyListener [])jmi.getListeners(MenuKeyListener.class);
    for (int i = 0; i < mkl.length; jmi.removeMenuKeyListener(mkl[i++]));
  }

  /**
   * removes all the listeners associated with this JMenu
   * removes MenuListener and listeners associated with a JMenuItem
   *
   * @see SysUtil.removeJMenuItemListeners
   */
  public static void removeJMenuListeners (JMenu jm) {
    removeMenuListeners(jm);
    removeJMenuItemListeners(jm);
  }

  public static void removeMenuListeners (JMenu jm) {
    MenuListener [] ml = (MenuListener [])jm.getListeners(MenuListener.class);
    for (int i = 0; i < ml.length; jm.removeMenuListener(ml[i++]));
  }

  /**
   * removes all the listeners associated with this JPopupMenu
   * removes PopupMenuListener and listeners associated with a JComponent
   *
   * @see SysUtil.removeJComponentListeners
   */
  public static void removeJPopupMenuListeners (JPopupMenu jpm) {
    removePopupMenuListeners(jpm);
    removeJComponentListeners(jpm);
  }

  public static void removePopupMenuListeners (JPopupMenu jpm) {
    PopupMenuListener [] pml = (PopupMenuListener [])jpm.getListeners(PopupMenuListener.class);
    for (int i = 0; i < pml.length; jpm.removePopupMenuListener(pml[i++]));
  }

  /**
   * removes all the listeners associated with this JProgressBar
   * removes ChangeListener and listeners associated with a JComponent
   *
   * @see SysUtil.removeJComponentListeners
   */
  public static void removeJProgressBarListeners (JProgressBar jpb) {
    removeChangeListeners(jpb);
    removeJComponentListeners(jpb);
  }

  public static void removeChangeListeners (JProgressBar jpb) {
    ChangeListener [] cl = (ChangeListener [])jpb.getListeners(ChangeListener.class);
    for (int i = 0; i < cl.length; jpb.removeChangeListener(cl[i++]));
  }

  /**
   * removes all the listeners associated with this JTabbedPane
   * removes ChangeListener and listeners associated with a JComponent
   *
   * @see SysUtil.removeJComponentListeners
   */
  public static void removeJTabbedPaneListeners (JTabbedPane jtp) {
    removeChangeListeners(jtp);
    removeJComponentListeners(jtp);
  }

  public static void removeChangeListeners (JTabbedPane jtp) {
    ChangeListener [] cl = (ChangeListener [])jtp.getListeners(ChangeListener.class);
    for (int i = 0; i < cl.length; jtp.removeChangeListener(cl[i++]));
  }

  /**
   * removes all the listeners associated with this JInternalFrame
   * removes InternalFrameListener and listeners associated with a JComponent
   *
   * @see SysUtil.removeJComponentListeners
   */
  public static void removeJInternalFrameListeners (JInternalFrame jif) {
    removeInternalFrameListeners(jif);
    removeJComponentListeners(jif);
  }

  public static void removeInternalFrameListeners (JInternalFrame jif) {
    InternalFrameListener [] ifl = (InternalFrameListener [])jif.getListeners(InternalFrameListener.class);
    for (int i = 0; i < ifl.length; jif.removeInternalFrameListener(ifl[i++]));
  }

  /**
   * removes all the listeners associated with this JTextField
   * removes ActionListener and listeners associated with a JTextComponent
   *
   * @see SysUtil.removeJTextComponentListeners
   */
  public static void removeJTextFieldListeners (JTextField jtf) {
    removeActionListeners(jtf);
    removeJTextComponentListeners(jtf);
  }

  public static void removeActionListeners (JTextField jtf) {
    ActionListener [] al = (ActionListener [])jtf.getListeners(ActionListener.class);
    for (int i = 0; i < al.length; jtf.removeActionListener(al[i++]));
  }

  /**
   * removes all the listeners associated with this JTextComponent
   * removes CaretListener and listeners associated with a JComponent
   *
   * @see SysUtil.removeJComponentListeners
   */
  public static void removeJTextComponentListeners (JTextComponent jtc) {
    removeCaretListeners(jtc);
    removeJComponentListeners(jtc);
  }

  public static void removeCaretListeners (JTextComponent jtc) {
    CaretListener [] cl = (CaretListener [])jtc.getListeners(CaretListener.class);
    for (int i = 0; i < cl.length; jtc.removeCaretListener(cl[i++]));
  }

  /**
   * removes all the listeners associated with this JTree
   * removes TreeExpansionListener, TreeSelectionListener, 
   * TreeWillExpandListener and listeners associated with a JComponent
   *
   * @see SysUtil.removeJComponentListeners
   */
  public static void removeJTreeListeners (JTree tree) {
    removeTreeExpansionListeners(tree);
    removeTreeSelectionListeners(tree);
    removeTreeWillExpandListeners(tree);
    removeJComponentListeners(tree);
  }

  public static void removeTreeExpansionListeners (JTree tree) {
    TreeExpansionListener [] tel = (TreeExpansionListener [])tree.getListeners(TreeExpansionListener.class);
    for (int i = 0; i < tel.length; tree.removeTreeExpansionListener(tel[i++]));
  }

  public static void removeTreeSelectionListeners (JTree tree) {
    TreeSelectionListener [] tsl = (TreeSelectionListener [])tree.getListeners(TreeSelectionListener.class);
    for (int i = 0; i < tsl.length; tree.removeTreeSelectionListener(tsl[i++]));
  }

  public static void removeTreeWillExpandListeners (JTree tree) {
    TreeWillExpandListener [] tel = (TreeWillExpandListener [])tree.getListeners(TreeWillExpandListener.class);
    for (int i = 0; i < tel.length; tree.removeTreeWillExpandListener(tel[i++]));
  }

  /**
   * removes all the listeners associated with this JViewport
   * removes ChangeListener and listeners associated with a JComponent
   *
   * @see SysUtil.removeJComponentListeners
   */
  public static void removeJViewportListeners (JViewport jvp) {
    removeChangeListeners(jvp);
    removeJComponentListeners(jvp);
  }

  public static void removeChangeListeners (JViewport jvp) {
    ChangeListener [] cl = (ChangeListener [])jvp.getListeners(ChangeListener.class);
    for (int i = 0; i < cl.length; jvp.removeChangeListener(cl[i++]));
  }

  /**
   * removes all the listeners associated with this button
   * removes ActionListener, ItemListener, ChangeListener and listeners
   * associated with a JComponent
   *
   * @see SysUtil.removeJComponentListeners
   */
  public static void removeButtonListeners (AbstractButton button) {
    removeActionListeners(button);
    removeItemListeners(button);
    removeChangeListeners(button);
    removeJComponentListeners(button);
  }

  public static void removeActionListeners (AbstractButton button) {
    ActionListener [] al = (ActionListener [])button.getListeners(ActionListener.class);
    for (int i = 0; i < al.length; button.removeActionListener(al[i++]));
  }

  public static void removeItemListeners (ItemSelectable is) {
    ItemListener [] il = (ItemListener [])((JComponent)is).getListeners(ItemListener.class);
    for (int i = 0; i < il.length; is.removeItemListener(il[i++]));
  }

  public static void removeChangeListeners (AbstractButton button) {
    ChangeListener [] cl = (ChangeListener [])button.getListeners(ChangeListener.class);
    for (int i = 0; i < cl.length; button.removeChangeListener(cl[i++]));
  }

  /**
   * removes all the listeners associated with this JComponent
   * removes AncestorListener, VetoableChangeListener and
   * listeners associated with a Component
   *
   * @see SysUtil.removeAllComponentListeners
   */
  public static void removeJComponentListeners (JComponent comp) {
    removeAncestorListeners(comp);
    removeVetoableChangeListeners(comp);
    removeAllComponentListeners(comp);
  }

  public static void removeAncestorListeners (JComponent comp) {
    AncestorListener [] al = (AncestorListener [])comp.getListeners(AncestorListener.class);
    for (int i = 0; i < al.length; comp.removeAncestorListener(al[i++]));
  }

  public static void removeVetoableChangeListeners (JComponent comp) {
    VetoableChangeListener [] vcl = (VetoableChangeListener [])comp.getListeners(VetoableChangeListener.class);
    for (int i = 0; i < vcl.length; comp.removeVetoableChangeListener(vcl[i++]));
  }

  /**
   * removes all the listeners associated with this Window
   * removes WindowListener and listeners associated with a Component
   *
   * @see SysUtil.removeAllComponentListeners
   */
  public static void removeAllWindowListeners (Window window) {
    removeWindowListeners(window);
    removeAllComponentListeners(window);
  }

  public static void removeWindowListeners (Window window) {
    WindowListener [] wl = (WindowListener [])window.getListeners(WindowListener.class);
    for (int i = 0; i < wl.length; window.removeWindowListener(wl[i++]));
  }

  /**
   * removes all the listeners associated with this Container
   * removes ContainerListener and listeners associated with a Component
   *
   * @see SysUtil.removeAllComponentListeners
   */
  public static void removeAllContainerListeners (Container cont) {
    removeContainerListeners(cont);
    removeAllComponentListeners(cont);
  }

  public static void removeContainerListeners (Container cont) {
    ContainerListener [] cl = (ContainerListener [])cont.getListeners(ContainerListener.class);
    for (int i = 0; i < cl.length; cont.removeContainerListener(cl[i++]));
  }

  /**
   * removes all the listeners associated with this Component
   * removes ComponentListener, FocusListener, HierarchyBoundsListener,
   * HierarchyListener, InputMethodListener, KeyListener, MouseListener,
   * MouseMotionListener and PropertyChangeListener
   */
  public static void removeAllComponentListeners (Component comp) {
    removeComponentListeners(comp);
    removeFocusListeners(comp);
    removeHierarchyBoundsListeners(comp);
    removeHierarchyListeners(comp);
    removeInputMethodListeners(comp);
    removeKeyListeners(comp);
    removeMouseListeners(comp);
    removeMouseMotionListeners(comp);
    removePropertyChangeListeners(comp);
  }

  public static void removeComponentListeners (Component comp) {
    ComponentListener [] cl = (ComponentListener [])comp.getListeners(ComponentListener.class);
    for (int i = 0; i < cl.length; comp.removeComponentListener(cl[i++]));
  }

  public static void removeFocusListeners (Component comp) {
    FocusListener [] fl = (FocusListener [])comp.getListeners(FocusListener.class);
    for (int i = 0; i < fl.length; comp.removeFocusListener(fl[i++]));
  }

  public static void removeHierarchyBoundsListeners (Component comp) {
    HierarchyBoundsListener [] hbl = (HierarchyBoundsListener [])comp.getListeners(HierarchyBoundsListener.class);
    for (int i = 0; i < hbl.length; comp.removeHierarchyBoundsListener(hbl[i++]));
  }

  public static void removeHierarchyListeners (Component comp) {
    HierarchyListener [] hl = (HierarchyListener [])comp.getListeners(HierarchyListener.class);
    for (int i = 0; i < hl.length; comp.removeHierarchyListener(hl[i++]));
  }

  public static void removeInputMethodListeners (Component comp) {
    InputMethodListener [] iml = (InputMethodListener [])comp.getListeners(InputMethodListener.class);
    for (int i = 0; i < iml.length; comp.removeInputMethodListener(iml[i++]));
  }

  public static void removeKeyListeners (Component comp) {
    KeyListener [] kl = (KeyListener [])comp.getListeners(KeyListener.class);
    for (int i = 0; i < kl.length; comp.removeKeyListener(kl[i++]));
  }

  public static void removeMouseListeners (Component comp) {
    MouseListener [] ml = (MouseListener [])comp.getListeners(MouseListener.class);
    for (int i = 0; i < ml.length; comp.removeMouseListener(ml[i++]));
  }

  public static void removeMouseMotionListeners (Component comp) {
    MouseMotionListener [] mml = (MouseMotionListener [])comp.getListeners(MouseMotionListener.class);
    for (int i = 0; i < mml.length; comp.removeMouseMotionListener(mml[i++]));
  }

  public static void removePropertyChangeListeners (Component comp) {
    PropertyChangeListener [] pcl = (PropertyChangeListener [])comp.getListeners(PropertyChangeListener.class);
    for (int i = 0; i < pcl.length; comp.removePropertyChangeListener(pcl[i++]));
  }

}
