package com.nextone.common;
/**
 * the callback to indicate that the listing opration is done
 */
public interface DBObject{


  public Object[] getObjectArray();
  public String[] getKeys();
  public Object[] getValues();
}
