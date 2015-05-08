package com.nextone.common;

import java.io.*;

public class IEdgeCore implements Serializable {
	  private String regId;
	  private int devType = -1;

	  static final long serialVersionUID = -2622017259406261206L;

	  public IEdgeCore (String s, int dt) {
		 regId = s;
		 devType = dt;
	  }

	  public String getRegId () {
		 return regId;
	  }

	  public int getDeviceType () {
		 return devType;
	  }
}

