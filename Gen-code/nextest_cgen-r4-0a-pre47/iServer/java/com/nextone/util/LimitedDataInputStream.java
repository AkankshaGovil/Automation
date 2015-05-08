package com.nextone.util;

import java.io.*;

/**
 * This class keeps a count of the number of bytes read out of this
 * InputStream. It throws an EOFException if an attempt is made
 * to read more than the allowed number of bytes.
 */
public class LimitedDataInputStream extends java.io.FilterInputStream implements java.io.DataInput {
	  protected int limit;
	  protected int count;
	  protected DataInputStream din;

	  /**
	   * builds a DataInputStream from the InputStream passed
	   */
	  public LimitedDataInputStream (InputStream is, int limit) {
		 super(new DataInputStream(is));
		 this.din = (DataInputStream)super.in;
		 this.limit = limit;
		 this.count = 0;
	  }

	  /**
	   * Sets a new limit.
	   *
	   * @exception IOException if the new limit is smaller than the
	   * number of bytes already read
	   */
	  public synchronized void setLimit (int newLimit) throws IOException {
		 if (newLimit < count)
			throw new IOException("new limit smaller than bytes already read");
		 limit = newLimit;
	  }

	  /**
	   * returns the current limit on the stream
	   */
	  public synchronized int getLimit () {
		 return limit;
	  }

	  /**
	   * returns the number of bytes read so far
	   */
	  public synchronized int getCount () {
		 return count;
	  }

	  // check if we have reached the limit
	  private void checkLimit () throws IOException {
		 ensureOpen();
		 if (count == limit)
			throw new EOFException("limit(" + limit + ") reached");
	  }

	  // check if we have reached the limit and if we will reach
	  // the limit if we attempt to further read 'toRead' bytes
	  private void checkLimit (int toRead) throws IOException {
		 if (toRead == 0)
			return;
		 checkLimit();
		 if ((count + toRead) > limit)
			throw new EOFException("attempting to read(" + toRead + ") beyond limit(" + limit + ")");
	  }

	  /**
	   * check to make sure that this stream has not been closed
	   */
	  private void ensureOpen () throws IOException {
		 if (din == null)
			throw new IOException("Stream closed");
	  }

	  public int read () throws IOException {
		 return readUnsignedByte();
	  }

	  public synchronized final int read (byte[] b) throws IOException {
		 return read(b, 0, b.length);
	  }

	  public synchronized final int read (byte[] b, int offset, int len) throws IOException {
		 if (len > 0) {
			try {
			   checkLimit();
			} catch (EOFException e) {
			   return -1;
			}
		 }
		 int c = din.read(b, offset, len);
		 if (c == -1)
			return c;

		 // have we read more than the limit?
		 if ((count + c) > limit) {
			// eventhough we read more, we return only the allowed amount
			int amtRead = limit - count;
			count = limit;
			return amtRead;
		 }

		 count += c;
		 return c;
	  }

	  public synchronized final boolean readBoolean () throws IOException {
		 checkLimit(1);
		 boolean b = din.readBoolean();
		 count += 1;
		 return b;
	  }

	  public synchronized final byte readByte () throws IOException {
		 checkLimit(1);
		 byte b = din.readByte();
		 count += 1;
		 return b;
	  }

	  public synchronized final char readChar () throws IOException {
		 checkLimit(2);
		 char c = din.readChar();
		 count += 2;
		 return c;
	  }

	  public synchronized final double readDouble () throws IOException {
		 checkLimit(8);
		 double d = din.readDouble();
		 count += 8;
		 return d;
	  }

	  public synchronized final float readFloat () throws IOException {
		 checkLimit(4);
		 float f = din.readFloat();
		 count += 4;
		 return f;
	  }

	  public synchronized final void readFully (byte[] b) throws IOException {
		 checkLimit(b.length);
		 din.readFully(b);
		 count += b.length;
	  }


	  public synchronized final byte [] readAllBytes () throws IOException {
		 int toRead = limit - count;
		 byte [] result = new byte [toRead];
		 readFully(result);
		 return result;
	  }

	  public synchronized final void readFully (byte[] b, int offset, int len) throws IOException {
		 checkLimit(b.length);
		 din.readFully(b, offset, len);
		 count += b.length;
	  }

	  public synchronized final int readInt () throws IOException {
		 checkLimit(4);
		 int i = din.readInt();
		 count += 4;
		 return i;
	  }

	  public synchronized final String readLine () throws IOException {
		 throw new IOException("readLine unsupported in LimitedDataInputStream");
	  }

	  public synchronized final long readLong () throws IOException {
		 checkLimit(8);
		 long l = din.readLong();
		 count += 8;
		 return l;
	  }

	  public synchronized final short readShort () throws IOException {
		 checkLimit(2);
		 short s = din.readShort();
		 count += 2;
		 return s;
	  }

	  public synchronized final int readUnsignedByte () throws IOException {
		 checkLimit(1);
		 int i = din.readUnsignedByte();
		 count += 1;
		 return i;
	  }

	  public synchronized final int readUnsignedShort () throws IOException {
		 checkLimit(2);
		 int i = din.readUnsignedShort();
		 count += 2;
		 return i;
	  }

	  // look at the source code for DataInputStream.readUTF for
	  // the details of the implementation of this method
	  public synchronized final String readUTF () throws IOException {
		 checkLimit(2);
		 int utfLen = din.readUnsignedShort();
		 count += 2;

		 checkLimit(utfLen);
		 StringBuffer str = new StringBuffer(utfLen);
		 byte [] ba = new byte [utfLen];
		 din.readFully(ba, 0, utfLen);
		 count += utfLen;

		 // convert the byte [] into UTF format strings
		 int char1, char2, char3;
		 int _c = 0;
		 while (_c < utfLen) {
			char1 = (int)(ba[_c] & 0xff);
			switch (char1 >> 4) {
			   case 0: case 1: case 2: case 3: case 4: case 5: case 6: case 7:
				 /* 0xxx xxxx */
				 _c++;
				 str.append((char)char1);
				 break;

			   case 12: case 13:
				 /* 110x xxxx 10xx xxxx */
				 _c += 2;
				 if (_c > utfLen)
					throw new UTFDataFormatException();
				 char2 = (int)ba[_c-1];
				 if ((char2 & 0xC0) != 0x80)
					throw new UTFDataFormatException();
				 str.append((char)(((char1 & 0x1F) << 6) | (char2 & 0x3F)));
				 break;

			   case 14:
				 /* 1110 xxxx 10xx xxxx 10xx xxxx */
				 _c += 3;
				 if (_c > utfLen)
					throw new UTFDataFormatException();
				 char2 = (int)ba[_c-2];
				 char3 = (int)ba[_c-1];
				 if (((char2 & 0xC0) != 0x80) || ((char3 & 0xC0) != 0x80))
					throw new UTFDataFormatException();
				 str.append((char)(((char1 & 0x0F) << 12) |
								   ((char2 & 0x3F) << 6) |
								   (char3 & 0x3F)));
				 break;
			   default:
				 /* 10xx xxxx, 1111 xxxx */
				 throw new UTFDataFormatException();
			}
		 }

		 return str.toString();
	  }

	  public final int skipBytes (int n) throws IOException {
		 return din.skipBytes(n);
	  }

	  public void close () throws IOException {
		 if (din == null)
			return;
		 din.close();
		 din = null;
	  }

}

