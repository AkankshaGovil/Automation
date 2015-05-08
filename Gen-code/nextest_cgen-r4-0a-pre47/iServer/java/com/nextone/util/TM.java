package com.nextone.util;

import java.awt.*;
import javax.swing.*;
import java.util.Calendar;
import java.util.Date;
import java.text.NumberFormat;
import java.io.Serializable;

import com.nextone.util.JMask.NumberField;

/**
 * this class provides the functionalities equivalent to that of the C "struct tm"
 */
public class TM implements Serializable, Cloneable {
  protected int tm_sec = ANY;
  protected int tm_min = ANY;
  protected int tm_hour = ANY;
  protected int tm_mday = ANY;
  protected int tm_mon = ANY;
  protected int tm_year = ANY;  // range from 1900 to 2038
  protected int tm_wday = ANY;
  protected int tm_yday = ANY;
  protected int tm_isdst = -1;

  protected transient ConfigPanel configPanel;

  public static final int ANY = -1;

  public static final int SUNDAY = 0;
  public static final int MONDAY = 1;
  public static final int TUESDAY = 2;
  public static final int WEDNESDAY = 3;
  public static final int THURSDAY = 4;
  public static final int FRIDAY = 5;
  public static final int SATURDAY = 6;

  public static final int JANUARY = 0;
  public static final int FEBRUARY = 1;
  public static final int MARCH = 2;
  public static final int APRIL = 3;
  public static final int MAY = 4;
  public static final int JUNE = 5;
  public static final int JULY = 6;
  public static final int AUGUST = 7;
  public static final int SEPTEMBER = 8;
  public static final int OCTOBER = 9;
  public static final int NOVEMBER = 10;
  public static final int DECEMBER = 11;

  protected static final String [] days = {
    "Any",
    "Sun",
    "Mon",
    "Tue",
    "Wed",
    "Thu",
    "Fri",
    "Sat",
  };
  protected static final String [] months = {
    "Any",
    "Jan",
    "Feb",
    "Mar",
    "Apr",
    "May",
    "Jun",
    "Jul",
    "Aug",
    "Sep",
    "Oct",
    "Nov",
    "Dec",
  };
  protected static final String [] hours = new String [25];
  protected static final String [] minutes = new String [61];
  protected static final String [] seconds = new String [61];

  static {
    hours[0] = "Any";
    minutes[0] = "Any";
    seconds[0] = "Any";

    NumberFormat nf = NumberFormat.getNumberInstance();
    nf.setParseIntegerOnly(true);
    nf.setMinimumIntegerDigits(2);
    nf.setGroupingUsed(false);

    for (int i = 0; i <24; i++)
      hours[i+1] = nf.format(i);
    for (int i = 0; i < 60; i++) {
      String str = nf.format(i);
      minutes[i+1] = str;
      seconds[i+1] = str;
    }
  }    

  static final long serialVersionUID = -246442876392731811L;

  public TM () {

  }

  public TM(Calendar  cal){
    this(cal.get(Calendar.SECOND),
         cal.get(Calendar.MINUTE),
         cal.get(Calendar.HOUR),
         cal.get(Calendar.DAY_OF_MONTH),
         cal.get(Calendar.MONTH),
         cal.get(Calendar.YEAR),
         cal.get(Calendar.DAY_OF_WEEK),
         cal.get(Calendar.DAY_OF_YEAR),
         cal.get(Calendar.DST_OFFSET)
         );
  }

  /**
   * @param sec 0-59 or TM_ANY
   * @param min 0-59 or TM_ANY
   * @param hour 0-23 or TM_ANY
   * @param mday 0-31 or TM_ANY
   * @parm mon 0-11 or TM_ANY
   * @param year 1900 - 2038
   * @param wday 0-6 or TM_ANY
   * @param yday 0-264 or TM_ANY
   * @param -1, 0, 1
   */
  public TM (int sec, int min, int hour, int mday, int mon, int year, int wday, int yday, int isdst) {
    this();
    tm_sec = sec;
    tm_min = min;
    tm_hour = hour;
    tm_mday = mday;
    tm_mon = mon;
    tm_year = year;
    tm_wday = wday;
    tm_yday = yday;
    tm_isdst = isdst;
  }

  public synchronized void setSecond (int value) {
    tm_sec = value;
    if (configPanel != null)
      configPanel.setSecond();
  }

  public synchronized int getSecond () {
    if (configPanel != null)
      tm_sec = configPanel.getSecond();
    return tm_sec;
  }

  public synchronized void setMinute (int value) {
    tm_min = value;
    if (configPanel != null)
      configPanel.setMinute();
  }

  public synchronized int getMinute () {
    if (configPanel != null)
      tm_min = configPanel.getMinute();
    return tm_min;
  }

  public synchronized void setHour (int value) {
    tm_hour = value;
    if (configPanel != null)
      configPanel.setHour();
  }

  public synchronized int getHour () {
    if (configPanel != null)
      tm_hour = configPanel.getHour();
    return tm_hour;
  }

  public synchronized void setMday (int value) {
    tm_mday = value;
    if (configPanel != null)
      configPanel.setMday();
  }

  public synchronized int getMday () {
    if (configPanel != null)
      tm_mday = configPanel.getMday();
    return tm_mday;
  }

  public synchronized void setMonth (int value) {
    tm_mon = value;
    if (configPanel != null)
      configPanel.setMonth();
  }

  public synchronized int getMonth () {
    if (configPanel != null)
      tm_mon = configPanel.getMonth();
    return tm_mon;
  }

  public synchronized void setYear (int value) {
    tm_year = value;
    if (configPanel != null)
      configPanel.setYear();
  }

  public synchronized int getYear () {
    if (configPanel != null)
      tm_year = configPanel.getYear();
    return tm_year;
  }

  public synchronized void setWday (int value) {
    tm_wday = value;
    if (configPanel != null)
      configPanel.setWday();
  }

  public synchronized int getWday () {
    if (configPanel != null)
      tm_wday = configPanel.getWday();
    return tm_wday;
  }

  public synchronized void setYday (int value) {
    tm_yday = value;
    if (configPanel != null)
      configPanel.setYday();
  }

  public synchronized int getYday () {
    if (configPanel != null)
      tm_yday = configPanel.getYday();
    return tm_yday;
  }

  public synchronized void setDST (int value) {
    if (value < 0)
      tm_isdst = -1;
    else if (value > 0)
      tm_isdst = 1;
    else
      tm_isdst = 0;
  }

  public synchronized int getDST () {
    return tm_isdst;
  }

  public JPanel getConfigPanel () {
    if (configPanel == null)
      configPanel = new ConfigPanel();

    return configPanel;
  }

  public void setTM (TM newTM) {
    if (newTM == null)
      newTM = new TM();
    setSecond(newTM.getSecond());
    setMinute(newTM.getMinute());
    setHour(newTM.getHour());
    setMday(newTM.getMday());
    setMonth(newTM.getMonth());
    setYear(newTM.getYear());
    setWday(newTM.getWday());
    setYday(newTM.getYday());
    setDST(newTM.getDST());
  }

  public boolean equals (Object o) {
    if (o != null && o.getClass().equals(TM.class)) {
      TM tm = (TM)o;
      if (tm.getSecond() == getSecond() &&
	  tm.getMinute() == getMinute() &&
	  tm.getHour() == getHour() &&
	  tm.getMday() == getMday() &&
	  tm.getMonth() == getMonth() &&
	  tm.getYear() == getYear() &&
	  tm.getWday() == getWday() &&
	  tm.getYday() == getYday() &&
	  tm.getDST() == getDST()) {
	return true;
      }
    }

    return false;
  }

  public Object clone () throws CloneNotSupportedException {
    return super.clone();
  }

  public String toString () {
    return getString();
  }

  public String getString () {
    StringBuffer sb = new StringBuffer();
    int val = getYear();
    if (val == ANY)
      sb.append(val);
    else
      sb.append(val-1900);
    sb.append(":");
    val = getYday();
    sb.append(val);
    sb.append(":");
    val = getWday();
    sb.append(val);
    sb.append(":");
    val = getMonth();
    sb.append(val);
    sb.append(":");
    val = getMday();
    sb.append(val);
    sb.append(":");
    val = getHour();
    sb.append(val);
    sb.append(":");
    val = getMinute();
    sb.append(val);
    sb.append(":");
    val = getSecond();
    sb.append(val);

    return sb.toString();
  }

  protected class ConfigPanel extends JPanel {
    protected JComboBox day, month, hour, minute, second;
    protected NumberField year;

    ConfigPanel () {
      day = new JComboBox(days);
      day.setMaximumRowCount(4);

      month = new JComboBox(months);
      month.setMaximumRowCount(6);

      hour = new JComboBox(hours);
      hour.setMaximumRowCount(6);

      minute = new JComboBox(minutes);
      minute.setMaximumRowCount(10);

      second = new JComboBox(seconds);
      second.setMaximumRowCount(10);

      year = new NumberField(4, true, 4);

      setLayout(new GridLayout(2, 3));
      add(createPanel("Day:", day));
      add(createPanel("Month:", month));
      add(createPanel("Year:", year));
      add(createPanel("Hour:", hour));
      add(createPanel("Min:", minute));
      add(createPanel("Sec:", second));
    }

    private JPanel createPanel (String label, Component comp) {
      JPanel p = new JPanel();
      p.setLayout(new GridLayout(1, 2));

      JPanel lp = new JPanel();
      lp.setLayout(new FlowLayout(FlowLayout.RIGHT));
      JLabel lbl = new JLabel(label, JLabel.RIGHT);
      lbl.setVerticalAlignment(JLabel.BOTTOM);
      lp.add(lbl);
      p.add(lp);

      JPanel rp = new JPanel();
      rp.setLayout(new FlowLayout(FlowLayout.LEFT));
      rp.add(comp);
      p.add(rp);

      return p;
    }

    public int getSecond () {
      return second.getSelectedIndex()-1;
    }

    public void setSecond () {
      second.setSelectedIndex(tm_sec+1);
    }

    public int getMinute () {
      return minute.getSelectedIndex()-1;
    }

    public void setMinute () {
      minute.setSelectedIndex(tm_min+1);
    }

    public int getHour () {
      return hour.getSelectedIndex()-1;
    }

    public void setHour () {
      hour.setSelectedIndex(tm_hour+1);
    }

    public int getMday () {
      return tm_mday;
    }

    public void setMday () {

    }

    public int getMonth () {
      return month.getSelectedIndex()-1;
    }

    public void setMonth () {
      month.setSelectedIndex(tm_mon+1);
    }

    public int getYear () {
      if (year.getText().length() == 0)
	return ANY;

      return year.getValue();
    }

    public void setYear () {
      if (tm_year == ANY)
	year.setText("");
      else
	year.setValue(tm_year);
    }

    public int getWday () {
      return day.getSelectedIndex()-1;
    }

    public void setWday () {
      day.setSelectedIndex(tm_wday+1);
    }

    public int getYday () {
      return tm_yday;
    }

    public void setYday () {

    }
  }

  public static void main (String [] args) {
    TM tm = new TM();
    JFrame jf = new JFrame();
    jf.setDefaultCloseOperation(JFrame.EXIT_ON_CLOSE);
    jf.getContentPane().add(tm.getConfigPanel());
    jf.pack();
    jf.setVisible(true);
  }
}
