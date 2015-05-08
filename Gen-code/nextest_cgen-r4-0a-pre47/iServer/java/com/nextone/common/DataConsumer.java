package com.nextone.common;

import java.io.*;
import com.nextone.util.LimitedDataInputStream;

/**
 * this interface is used while doing GET operation on the iEdge
 */
public interface DataConsumer {
	  public void extractGetReply (LimitedDataInputStream dis) throws IOException;
}
