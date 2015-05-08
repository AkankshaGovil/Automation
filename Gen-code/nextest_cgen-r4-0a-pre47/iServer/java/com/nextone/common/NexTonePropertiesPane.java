package com.nextone.common;

public interface NexTonePropertiesPane {
  /**
   * Force release of resources.
   */
  public void fwoom();

  /**
   * update the pane contents
   */
  public boolean updatePane();

  /**
   * Rest the title on the frame.
   */
  public void resetTitle ();

}
