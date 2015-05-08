package com.nextone.common;

/**
 * this class provides an interface for a permission provider. A permission
 * provider provides with read/write permission for a device.
 */
public interface PermissionProvider {

	  // get the read permission
	  public String getReadPermission ();
    //  get the read permission for specified key
    public String getReadPermission(Object key);

	  // set the read permission
	  public void setReadPermission (String s);
	  // set the read permission
	  public void setReadPermission (Object key,String s);

	  // get the write permission
	  public String getWritePermission ();
	  // get the write permission
	  public String getWritePermission (Object key);

	  // set the write permission
	  public void setWritePermission (String s);
	  // set the write permission
	  public void setWritePermission (Object key,String s);

	  // set both the permission
	  public void setPermissions (String read, String write);

	  // get the permission set specifically for this device
	  // (not the default)
	  public String getSpecificReadPermission ();
	  public String getSpecificWritePermission ();

}
