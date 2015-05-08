package com.nextone.util;

import java.awt.*;

import com.nextone.util.JMask.NumberField;

/**
 * this is a spinner class, which by default spins a range of numbers
 */
public class NumberSpinner extends Spinner implements SpinListener {
  protected int minVal, maxVal;
  protected SpinListener sl;

  public NumberSpinner (int width, int minVal, int maxVal, SpinListener sl) {
    super(new NumberField(width));
    this.minVal = minVal;
    this.maxVal = maxVal;
    this.sl = sl;
    ((NumberField)component).setEditable(false);
    component.setBackground(Color.white);
    addSpinListener(this);
  }

  public void spinnerSpunUp (SpinEvent se) {
    NumberField nf = (NumberField)component;
    synchronized (this) {
      int val = nf.getValue();
      if (val < maxVal)
	val++;
      else
	val = minVal;
      nf.setValue(val);
    }
    if (sl != null)
      sl.spinnerSpunUp(se);
  }

  public void spinnerSpunDown (SpinEvent se) {
    NumberField nf = (NumberField)component;
    synchronized (this) {
      int val = nf.getValue();
      if (val > minVal)
	val--;
      else
	val = maxVal;
      nf.setValue(val);
    }
    if (sl != null)
      sl.spinnerSpunDown(se);
  }

  public synchronized void setMax (int max) {
    maxVal = max;
  }

  public synchronized int getMax () {
    return maxVal;
  }

  public synchronized void setMin (int min) {
    minVal = min;
  }

  public synchronized int getMin () {
    return minVal;
  }
}
