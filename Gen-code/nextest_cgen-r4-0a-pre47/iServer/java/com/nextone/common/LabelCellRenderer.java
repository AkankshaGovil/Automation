package com.nextone.common;

import javax.swing.*;
import java.awt.*;

public class LabelCellRenderer extends DefaultListCellRenderer {

	  public LabelCellRenderer () {
		 setOpaque(true);
	  }

	  public Component getListCellRendererComponent (JList list, Object value, int index, boolean isSelected, boolean cellHasFocus) {

		 if (isSelected) {
			setBackground(list.getSelectionBackground());
			setForeground(list.getSelectionForeground());
		 } else {
			setBackground(list.getBackground());
			setForeground(list.getForeground());
		 }

		 if (value instanceof Icon) {
			setIcon((Icon)value);
		 } else if (value instanceof JLabel) {
			setText(((JLabel)value).getText());
			setForeground(((JLabel)value).getForeground());
			setToolTipText(((JLabel)value).getToolTipText());
		 } else {
			setText((value == null) ? "" : value.toString());
		 }

		 setEnabled(list.isEnabled());
//		 setFont(list.getFont());
		 setBorder((cellHasFocus) ? UIManager.getBorder("List.focusCellHighlightBorder") : noFocusBorder);

		 return this;
	  }

}

