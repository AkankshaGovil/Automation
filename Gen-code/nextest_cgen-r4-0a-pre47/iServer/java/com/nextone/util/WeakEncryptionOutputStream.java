package com.nextone.util;

import java.io.*;
import java.util.zip.*;

/**
 * Does a sort of weak encryption on the stream. The weak encryption
 * algorithm is:
 * The key is a byte []. For each data byte attempted to be written
 * two bytes will be written to the underlying stream. The two bytes
 * are created by taking the nibbles of a byte from the key and making
 * them the upper nibbles of the resultant bytes, and the nibbles of the
 * data byte be the lower nibbles of the resultant bytes. e.g.
 *    key byte -  00001111
 *    data byte - 10101010
 *    result - 00001010 11111010
 *
 * The stream is further passed through a GZIPOutputStream to attempt
 * compressing some of the bloated bytes. 
 */
public class WeakEncryptionOutputStream extends FilterOutputStream {
  private byte [] key;
  private int index = 0;


  // this is a static key data used to encrypt/decrypt data for the static method
  static byte [] keyData = { (byte)0xa1, (byte)0xcd, (byte)0x23,
			     (byte)0xcc, (byte)0x1a, (byte)0x45,
			     (byte)0x00, (byte)0xef, (byte)0xdf,
			     (byte)0xfb, (byte)0x2c, (byte)0xb6,
			     (byte)0xa9, (byte)0xd1, (byte)0x66,
			     (byte)0x01, (byte)0x70, (byte)0xa8,
			     (byte)0x20, (byte)0xee, (byte)0x34,
			     (byte)0x81, (byte)0xff, (byte)0xaa,
  };

  /**
   * takes a String, does a weak encrytion on it and returns the resultant string
   *
   * @param string the string to encrypt
   */
  public static String getEncryptedString (String string) throws IOException {
    ByteArrayOutputStream bos = new ByteArrayOutputStream(2*string.length());
    DataOutputStream dos = new DataOutputStream(new WeakEncryptionOutputStream(bos, keyData));
    dos.writeUTF(string);
    dos.close();

    // this is the encrypted byte array
    byte [] data = bos.toByteArray();

    // now convert the encrypted stream into a printable characters (0x20 - 0x7e)
    byte [] result = new byte [data.length*2];
    for (int i = 0, j = 0; i < data.length; i++) {
      // extract the 4 bits, push it to the middle 00xxxx00, and OR with 01000000
      // they will be in the printable range
      result[j++] = (byte)((((data[i] & 0xf0) >>> 2) & 0x3c) | 0x40);
      result[j++] = (byte)((((data[i] & 0x0f) << 2) & 0x3c) | 0x40);
    }

    return new String(result);
  }

  /**
   * contructs a new WeakEncryptionOutputStream.
   *
   * @exception IOException if creating the underlying GZIPOutputStream
   * throws an exception.
   */
  public WeakEncryptionOutputStream (OutputStream os, byte [] k) throws IOException {
    super(new GZIPOutputStream(os));
    this.key = new byte [2*k.length];
    // make an array with nibbles from the original array
    for (int i = 0; i < key.length; i++) {
      int n = i/2;
      if (i%2 == 0)
	key[i] = (byte)(k[n] & 0x000000f0);
      else
	key[i] = (byte)((k[n] & 0x0000000f)<<4);
    }
  }

  /**
   * Writes one byte of data. The underlying stream gets 2 encrypted bytes
   * written.
   *
   * @exception IOException if the underlying write throws an exception
   */
  public void write (int b) throws IOException {
    writeByte(b);
  }

  /**
   * Writes a byte []. The underlying stream gets 2*byte [] encrypted
   * bytes written.
   *
   * @exception IOException if the underlying write throws an exception
   */
  public void write (byte [] b, int off, int len) throws IOException {
    for (int i = 0; i < len; i++) {
      writeByte(b[i+off]);
    }
  }

  // implement the weak encryption
  private void writeByte (int b) throws IOException {
    int int1 = (((b & 0x000000f0)>>4) | getNext());
    out.write(int1);
    int int2 = ((b & 0x0000000f) | getNext());
    out.write(int2);
  }

  // returns the next available byte from the key. this is cyclic.
  private int getNext () {
    int result = ((key[index++] & 0x000000ff) | 0x0);
    if (index == key.length)
      index = 0;
    return result;
  }

}

