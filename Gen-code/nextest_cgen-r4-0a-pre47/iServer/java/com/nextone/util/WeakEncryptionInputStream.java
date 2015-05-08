package com.nextone.util;

import java.io.*;
import java.util.zip.*;

/**
 * Decrypts the weak-encrypted stream created by a WeakEncrytionOutputStream.
 * Reads two bytes, takes the lower nibbles, and creates one resultant byte.
 * Takes the upper nibbles, created a byte and authenticate against the
 * resulting key byte.
 * 
 * The stream is also passed through a GZIPInputStream.
 *
 * @see WeakEncryptionOutputStream
 */
public class WeakEncryptionInputStream extends FilterInputStream {
  private byte [] key;
  private int index = 0;

  /**
   * takes a String encrypted by WeakEncryptionOutputStream.getEncryptedString,
   * decrypts it and returns the original string
   *
   * @param string the string to decrypt
   */
  public static String getDecryptedString (String string) throws IOException {
    byte [] source = string.getBytes();

    // our algorithm makes sure that we always have even number of chars, if not, it is an error
    if (source.length%2 != 0)
      throw new IOException("String length is " + source.length + ", cannot decrypt");

    byte [] result = new byte [source.length/2];
    for (int i = 0, j = 0; i < source.length; i += 2, j++) {
      // extract the middle bits, 00xxxx00, they contain the information we want
      byte firstNibble = (byte)((source[i] << 2) & 0xf0);
      byte secondNibble = (byte)((source[i+1] >>> 2) & 0x0f);
      result[j] = (byte)((firstNibble | secondNibble) & 0xff);
    }

    DataInputStream dis = new DataInputStream(new WeakEncryptionInputStream(new ByteArrayInputStream(result), WeakEncryptionOutputStream.keyData));
    String decrypt = dis.readUTF();
    dis.close();

    return decrypt;
  }

  /**
   * constructs a new WeakEncrytion stream
   *
   * @exception IOException if creating the underlying GZIPInputStream
   * throws an exception.
   */
  public WeakEncryptionInputStream (InputStream is, byte [] k) throws IOException {
    super(new GZIPInputStream(is));
    this.key = new byte [k.length];
    System.arraycopy(k, 0, key, 0, k.length);
  }

  /**
   * simply returns the underlying stream's available() divided by 2
   *
   * @exception IOException if the underlying available() throws exception
   */
  public int available () throws IOException {
    return in.available()/2;
  }

  /**
   * returns false
   */
  public boolean markSupported () {
    return false;
  }

  /**
   * reads one byte of data. Reads two bytes from the underlying stream,
   * decrypts it and returns the resulting byte.
   *
   * @exception IOException if the underlying read returns an exception
   */
  public int read () throws IOException {
    return readByte();
  }

  /**
   * Reads byte [] of data. Reads 2*byte [] from the underlying stream,
   * and decrypts it. If b is null, it simply reads as many bytes from
   * the stream.
   *
   * @exception IOException if the underlying read returns an exception
   */
  public int read (byte [] b, int off, int len) throws IOException {
    int i;
    for (i = 0; i < len; i++) {
      int r = readByte();
      if (r == -1)
	break;
      if (b != null)
	b[i+off] = (byte)r;
    }
    if (len == 0)
      return 0;
    return (i==0)?-1:i;
  }

  /**
   * reset is not supported for this stream
   *
   * @exception IOException if this method is called
   */
  public void reset () throws IOException {
    throw new IOException("reset unsupported for WeakEncryptionInputStream");
  }

  /**
   * skips two times as many bytes from the underlying stream
   *
   * @exception IOException if the underlying skip throws an exception
   */
  public long skip (long n) throws IOException {
    long skipped = in.skip(2*n)/2;
    index += skipped;
    if (index >= key.length)
      index = 0;
    return skipped;
  }

  // implement the weak decryption
  private int readByte () throws IOException {
    int int1 = in.read();
    if (int1 == -1)
      return -1;
    int int2 = in.read();
    if (int2 == -1)
      return -1;
    int keybyte = ((int1 & 0x000000f0) | ((int2 & 0x000000f0)>>4));
    if (keybyte != getNext())
      throw new IOException("Authentication failed");
    return ( ((int1 & 0x0000000f)<<4) | (int2 & 0x0000000f) );
  }

  // returns the next available byte from the key. this is cyclic.
  private int getNext () {
    int result = ((key[index++] & 0x000000ff) | 0x0);
    if (index == key.length)
      index = 0;
    return result;
  }

}

