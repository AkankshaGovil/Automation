package com.nextone.common;
/**
 * the callback to indicate that the listing opration is done
 */
public interface Task {
  public int  getCurrentCount();
  public void cancelTask();
  public int getMaxCount();
  public boolean isRunning();
}
