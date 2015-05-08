package com.nextone.util;

import javax.swing.*;
import java.awt.*;
import java.awt.event.*;
import java.util.*;

/**
 * This component displays a calendar and generates events when a date 
 * is chosen from that calendar. Clicking on the month label will set the
 * chosen date to the current date
 */
public class DateChooser extends JPanel {
	  protected String [] days = {"Su", "Mo", "Tu", "We", "Th", "Fr", "Sa"};
	  protected String [] months = {"January",
									"February",
									"March",
									"April",
									"May",
									"June",
									"July",
									"August",
									"September",
									"October",
									"November",
									"December"};

	  protected JLabel monthLabel;
	  protected Calendar calendar;
	  protected JButton [] dayButtons;
	  private int prevButton = -1;
	  private int prevDayOfMonth = -1;
	  private String selectedMonth;
	  protected static Color lightBlue = new Color(114, 193, 255);
	  protected Vector listeners;
	  protected boolean moveSelectedDate = false;
	  private ButtonActionListener buttonActionListener;
	  private ButtonMouseAdapter buttonMouseAdapter;
	  protected JButton rarrow, larrow;	 

	  /**
	   * the initial date chosen would be the current date
	   */
	  public DateChooser () {
		 this(new Date());
	  }

	  /**
	   * sets curDate as the initial chosen date
	   */
	  public DateChooser (Date curDate) {
		 calendar = Calendar.getInstance();
		 calendar.setTime(curDate);

		 buttonActionListener = new ButtonActionListener();
		 buttonMouseAdapter = new ButtonMouseAdapter();
		 listeners = new Vector();

		 // top of the panel
		 JPanel tp = new JPanel();
		 tp.setLayout(new BorderLayout());
		 larrow = getSmallButton("<");
		 tp.add(larrow, BorderLayout.WEST);
		 rarrow = getSmallButton(">");
		 tp.add(rarrow, BorderLayout.EAST);
		 monthLabel = new JLabel("", JLabel.CENTER);
		 monthLabel.addMouseListener(new MonthMouseListener());
		 setMonthText();
		 tp.add(monthLabel, BorderLayout.CENTER);

		 // stuff in the middle
		 JPanel cp = new JPanel();
		 cp.setLayout(new GridLayout(7, 7, 2, 2));
		 for (int i = 0; i < 7; i++)
			cp.add(new JLabel(days[i], JLabel.CENTER));
		 dayButtons = new JButton [42];
		 for (int i = 0; i < 42; i++) {
			dayButtons[i] = getSmallButton(String.valueOf(i));
			cp.add(dayButtons[i]);
		 }
		 setButtonTexts();
		 highlightCurrentDay();

		 setLayout(new BorderLayout());
		 add(tp, BorderLayout.NORTH);
		 add(cp, BorderLayout.CENTER);
		 setBorder(BorderFactory.createCompoundBorder(BorderFactory.createEtchedBorder(), BorderFactory.createEmptyBorder(1, 3, 1, 3)));
	  }

	  /**
	   * enables/disables the display
	   */
	  public void setEnabled (boolean b) {
		 super.setEnabled(b);
		 monthLabel.setEnabled(b);
		 for (int i = 0; i < 42; i++) {
			if (b == true && !dayButtons[i].getText().equals(""))
			   dayButtons[i].setEnabled(true);
			else
			   dayButtons[i].setEnabled(false);
		 }
		 rarrow.setEnabled(b);
		 larrow.setEnabled(b);
	  }

	  /**
	   * calls the dateChosen method of the listeners whenever a particular
	   * date is chosen
	   */
	  public void addDateListener (DateListener dl) {
		 listeners.add(dl);
	  }

	  /**
	   * removes the date listener
	   */
	  public void removeDateListener (DateListener dl) {
		 listeners.remove(dl);
	  }

	  /**
	   * returns if moving the selected date along with the month is enabled
	   * or not
	   */
	  public boolean isMoveSelectedDate () {
		 return moveSelectedDate;
	  }

	  /**
	   * If set true and when moving between months, the chosen date 
	   * also moves to every month. For eg, if June 20 was chosen first
	   * and then the selection moves to the month of July, then July 20
	   * will become the chosen date.
	   */
	  public void setMoveSelectedDate (boolean b) {
		 moveSelectedDate = b;
	  }

	  /**
	   * returns the current date set in the calendar
	   */
	  public Date getDate () {
		 return calendar.getTime();
	  }


	  /**
	   * sets the currently selected date in the calendar. does not call
	   * the listeners
	   */
	  public void setDate (Date date) {
		 setDate(date, false);
	  }

	  /**
	   * sets the currently selected date in the calendar
	   */
	  public void setDate (Date date, boolean generateEvents) {
		 calendar.setTime(date);
		 redrawCalendar();
		 highlightCurrentDay();
		 if (generateEvents)
			callListeners();
	  }

	  protected void setMonthText () {
		 monthLabel.setText(months[calendar.get(Calendar.MONTH)] + " " + calendar.get(Calendar.YEAR));
	  }

	  protected void setButtonTexts () {
		 int curDay = calendar.get(Calendar.DAY_OF_MONTH);
		 calendar.set(Calendar.DAY_OF_MONTH, 1);
		 int startDay = calendar.get(Calendar.DAY_OF_WEEK);
		 int endDay = calendar.getActualMaximum(Calendar.DAY_OF_MONTH);
		 calendar.set(Calendar.DAY_OF_MONTH, curDay);
		 
		 int k = 1;
		 for (int i = 0; i < 42; i++, k++) {
			if (i < (startDay-1) || (k-startDay+1) > endDay) {
			   dayButtons[i].setText("");
			   dayButtons[i].setEnabled(false);
			} else {
			   dayButtons[i].setText(String.valueOf(k-startDay+1));
			   dayButtons[i].setEnabled(true);
			}
		 }
	  }

	  protected void redrawCalendar () {
		 setMonthText();
		 setButtonTexts();
		 repaint();
	  }

	  private JButton getSmallButton (String txt) {
		 JButton but = new JButton();

		 but.setActionCommand(txt);
		 if (txt.equals(">"))
			but.setIcon(new ImageIcon(getClass().getResource("/com/nextone/images/RightArrow.gif")));
		 else if (txt.equals("<"))
			but.setIcon(new ImageIcon(getClass().getResource("/com/nextone/images/LeftArrow.gif")));
		 else
			but.setText(txt);

		 Dimension d = new Dimension(20, 20);
		 but.setPreferredSize(d);
		 but.setMinimumSize(d);
		 but.setBorder(null);
		 but.setFocusPainted(false);
		 but.addActionListener(buttonActionListener);
		 but.addMouseListener(buttonMouseAdapter);

		 return but;
	  }

	  private class ButtonMouseAdapter extends MouseAdapter {
			public void mouseEntered (MouseEvent me) {
			   JButton but = (JButton)me.getComponent();
			   if (but.isEnabled() &&
				   prevButton != -1 &&
				   !but.getActionCommand().equals(dayButtons[prevButton].getActionCommand()))
				  but.setBorder(BorderFactory.createLineBorder(Color.black));
			}

			public void mouseExited (MouseEvent me) {
			   JButton but = (JButton)me.getComponent();
			   if (but.isEnabled() &&
				   prevButton != -1 &&
				   !but.getActionCommand().equals(dayButtons[prevButton].getActionCommand()))
				  but.setBorder(null);
			}
	  }

	  private class ButtonActionListener implements ActionListener {
			public void actionPerformed (ActionEvent ae) {
			   String cmd = ae.getActionCommand();

			   int year = calendar.get(Calendar.YEAR);
			   int month = calendar.get(Calendar.MONTH);
			   int day = calendar.get(Calendar.DATE);

			   boolean callListeners = false;
			   boolean adjustHighlight = true;
			   if (cmd.equals(">")) {
				  if (++month == 12) {
					 month = 0;
					 year++;
				  }
			   } else if (cmd.equals("<")) {
				  if (--month == -1) {
					 month = 11;
					 year--;
				  }
			   } else {
				  int d = Integer.valueOf(cmd).intValue();
				  dayButtons[d].setBackground(lightBlue);
				  dayButtons[d].setBorder(BorderFactory.createLoweredBevelBorder());
				  if (prevButton != -1 && prevButton != d) {
					 dayButtons[prevButton].setBackground(null);
					 dayButtons[prevButton].setBorder(null);
				  }
				  prevButton = d;
				  day = Integer.valueOf(dayButtons[d].getText()).intValue();
				  prevDayOfMonth = day;
				  selectedMonth = monthLabel.getText();
				  adjustHighlight = false;
				  callListeners = true;
			   }

			   calendar.set(year, month, 1);
			   if (day < calendar.getActualMinimum(Calendar.DAY_OF_MONTH) ||
				   day > calendar.getActualMaximum(Calendar.DAY_OF_MONTH))
				  day = calendar.getActualMinimum(Calendar.DAY_OF_MONTH);
			   calendar.set(Calendar.DATE, day);
			   redrawCalendar();

			   if (adjustHighlight && moveSelectedDate) {
				  if (prevButton != -1) {
					 dayButtons[prevButton].setBackground(null);
					 dayButtons[prevButton].setBorder(null);
				  }
				  int d = getButtonIndexForString(String.valueOf(prevDayOfMonth));
				  if (d != -1) {
					 dayButtons[d].setBackground(lightBlue);
					 dayButtons[d].setBorder(BorderFactory.createLoweredBevelBorder());
					 callListeners = true;
				  } else
					 prevDayOfMonth = -1;
				  prevButton = d;
				  selectedMonth = monthLabel.getText();
			   }

			   if (!moveSelectedDate && selectedMonth != null) {
				  if (selectedMonth.equals(monthLabel.getText())) {
					 dayButtons[prevButton].setBackground(lightBlue);
					 dayButtons[prevButton].setBorder(BorderFactory.createLoweredBevelBorder());
				  } else if (prevButton != -1) {
					 dayButtons[prevButton].setBackground(null);
					 dayButtons[prevButton].setBorder(null);
				  }
			   }

			   // call the listeners
			   if (callListeners)
				  callListeners();
			}
	  }

	  private class MonthMouseListener extends MouseAdapter {
			public void mouseEntered (MouseEvent me) {
			   JLabel lbl = (JLabel)me.getComponent();
			   if (lbl.isEnabled())
				  lbl.setBorder(BorderFactory.createLineBorder(Color.black));
			}

			public void mouseExited (MouseEvent me) {
			   JLabel lbl = (JLabel)me.getComponent();
			   if (lbl.isEnabled())
				  lbl.setBorder(null);
			}

			public void mouseClicked (MouseEvent me) {
			   if (!monthLabel.isEnabled())
				  return;
			   calendar.setTime(new Date());
			   redrawCalendar();
			   highlightCurrentDay();
			   callListeners();
			}
	  }

	  private void callListeners () {
		 Enumeration e = listeners.elements();
		 while (e.hasMoreElements())
			((DateListener)e.nextElement()).dateChosen(calendar.getTime());
	  }

	  private int getButtonIndexForString (String str) {
		 for (int i = 0; i < 42; i++) {
			if (dayButtons[i].getText().equals(str))
			   return i;
		 }

		 return -1;
	  }

	  private void highlightCurrentDay () {
		 if (prevButton != -1) {
			dayButtons[prevButton].setBackground(null);
			dayButtons[prevButton].setBorder(null);
		 }
		 int day = calendar.get(Calendar.DATE);
		 int curb = getButtonIndexForString(String.valueOf(day));
		 if (curb != -1) {
			dayButtons[curb].setBackground(lightBlue);
			dayButtons[curb].setBorder(BorderFactory.createLoweredBevelBorder());
		 }
		 prevButton = curb;
		 prevDayOfMonth = day;
		 selectedMonth = monthLabel.getText();
	  }

	  public static void main (String [] args) {
		 JFrame jf = new JFrame();
		 jf.setDefaultCloseOperation(JFrame.EXIT_ON_CLOSE);
		 jf.getContentPane().add(new DateChooser());
		 jf.pack();
		 jf.setVisible(true);
	  }
}

