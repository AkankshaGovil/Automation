package com.nextone.util;

import javax.swing.*;
import java.awt.*;
import java.awt.event.*;
import javax.swing.event.*;
import java.util.*;

/**
 * This class provides a GUI with a check box names "Auto-refresh" and a slider.
 * If the check box gets enabled, it generates an ActionEvent for every duration
 * specified by the slider
 */
public class AutoRefreshPanel extends JPanel implements ItemListener, ChangeListener {
  private JSlider slider;
  private JLabel sliderLabel;
  private ActionableTimer timer;
  private final int INITIAL = 60;

  public static int VERTICAL_LAYOUT = 0;
  public static int HORIZONTAL_LAYOUT = 1;

  /**
   * creates an auto refresh panel with the components laid out vertically
   *
   * @param al the ActionListener that will receive the ActionEvents
   * @param actionCmd the action command sent in the ActionEvent
   */
  public AutoRefreshPanel (ActionListener al, String actionCmd) {
    this(VERTICAL_LAYOUT, al, actionCmd);
  }

  /**
   * creates an auto refresh panel with the components laid out according to the
   * alignment specified.
   *
   * @param alignment one of VERTICAL_LAYOUT or HORIZONTAL_LAYOUT
   * @param al the ActionListener that will receive the ActionEvents
   * @param actionCmd the action command sent in the ActionEvent
   */
  public AutoRefreshPanel (int alignment, ActionListener al, String actionCmd) {
    timer = new ActionableTimer(INITIAL*1000, al, actionCmd);
    timer.setCoalesce(true);
    timer.setRepeats(true);
    timer.setInitialDelay(INITIAL*1000);

    if (alignment == VERTICAL_LAYOUT)
      setLayout(new BoxLayout(this, BoxLayout.Y_AXIS));
    else
      setLayout(new FlowLayout(FlowLayout.CENTER));

    JCheckBox cb = new JCheckBox("Auto-refresh");
    cb.addItemListener(this);
    add(cb);

    Box sb = null;
    if (alignment != VERTICAL_LAYOUT)
      sb = new Box(BoxLayout.Y_AXIS);

    sliderLabel = new JLabel(INITIAL + " secs", JLabel.CENTER);
    if (alignment == VERTICAL_LAYOUT)
      add(sliderLabel);
    else
      sb.add(sliderLabel);

    Dictionary dict = new Hashtable();
    for (int i = 0; i <= 10; i += 2)
      dict.put(new Integer(i*60), new JLabel(new Integer(i).toString()));
    slider = new JSlider(JSlider.HORIZONTAL, 10, 600, INITIAL);
    slider.setMajorTickSpacing(10);
    slider.setMinorTickSpacing(1);
    slider.setLabelTable(dict);
    slider.setPaintLabels(true);
    slider.setSnapToTicks(true);
    slider.addChangeListener(this);
    slider.setPreferredSize(new Dimension(120, 30));
    if (alignment == VERTICAL_LAYOUT)
      add(slider);
    else
      sb.add(slider);
    if (alignment == VERTICAL_LAYOUT)
      add(new JLabel("(mins)", JLabel.LEFT));
    else
      sb.add(new JLabel("(mins)", JLabel.LEFT));

    if (alignment != VERTICAL_LAYOUT)
      add(sb);
  }

  public void itemStateChanged (ItemEvent ie) {
    if (ie.getStateChange() == ItemEvent.SELECTED) {
      if (!timer.isRunning()) {
	int delay = slider.getValue();
	timer.setInitialDelay(delay*1000);
	timer.setDelay(delay*1000);
	timer.restart();
      }
    } else {
      if (timer.isRunning())
	timer.stop();
    }
  }

  public void stateChanged (ChangeEvent ce) {
    int delay = slider.getValue();
    sliderLabel.setText(delay + " secs");
    if (!slider.getValueIsAdjusting() &&
	timer.isRunning()) {
      // timer is running, reset the timer values
      timer.stop();
      timer.setDelay(delay*1000);
      timer.setInitialDelay(3000);
      timer.restart();
    }
  }

  /**
   * stops sending any further events, call this when this panel is no longer used
   */
  public void stopTimers () {
    timer.stop();
  }
}

