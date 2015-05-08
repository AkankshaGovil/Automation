package com.nextone.util;

import java.sql.*;
import java.io.*;

/**
 * creates a SQL BLOB type Blob object from a given string
 */
public class StringBlob implements Blob {
	  byte [] bytes;

	  public StringBlob (String source) {
		 if (source != null)
			bytes = source.getBytes();
	  }

	  public InputStream getBinaryStream () throws SQLException {
		 if (bytes == null)
			throw new SQLException("Blob source was null");
		 return new ByteArrayInputStream(bytes);
	  }

	  public byte [] getBytes (long pos, int len) throws SQLException {
		 if (bytes == null)
			throw new SQLException("Blob source was null");
		 if (len > (bytes.length + pos))
			throw new SQLException("Requesting too much information");
		 byte [] result = new byte [len];
		 for (int i = 0; i < len; i++)
			result[i] = bytes[(int)(pos+i)];
		 return result;
	  }

	  public long length () throws SQLException {
		 if (bytes == null)
			throw new SQLException("Blob source was null");
		 return bytes.length;
	  }

	  public long position (Blob pattern, long start) throws SQLException {
		 if (bytes == null)
			throw new SQLException("Blob source was null");
		 return position(pattern.getBytes(0L, (int)pattern.length()), start);
	  }

	  public long position (byte [] pattern, long start) throws SQLException {
		 if (bytes == null)
			throw new SQLException("Blob source was null");
		 throw new SQLException("Not implemented yet");
	  }

}

