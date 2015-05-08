package com.nextone.util;

import java.io.*;

/**
 * This class extends the RandomAccessFile and adds the ability the read from and write
 * to java objects. Objects that can be written to or read from this file should
 * implement Serializable interface (obviously).
 *
 * The write is done by serializing the objects, and storing the resultant byte array.
 * The length of the array is stored first and then the array itself. The read is the
 * exact reverse of the write process.
 */
public class RandomAccessObjectFile extends RandomAccessFile {

  /** simply calls the super's constructor */
  public RandomAccessObjectFile (File file, String mode) throws FileNotFoundException {
    super(file, mode);
  }

  /** simply calls the super's constructor */
  public RandomAccessObjectFile (String name, String mode) throws FileNotFoundException {
    super(name, mode);
  }

  /**
   * read a java object from the file
   */
  public Object readObject () throws IOException, ClassNotFoundException {
    int numBytes = readInt();
    byte [] data = new byte [numBytes];
    readFully(data);

    ObjectInputStream ois = new ObjectInputStream(new ByteArrayInputStream(data));
    Object o = ois.readObject();
    ois.close();

    return o;
  }

  /**
   * write a java object to the file
   */
  public void writeObject (Object o) throws IOException {
    ByteArrayOutputStream bos = new ByteArrayOutputStream();
    ObjectOutputStream oos  = new ObjectOutputStream(bos);
    oos.writeObject(o);
    oos.close();
    bos.close();

    byte [] data = bos.toByteArray();
    writeInt(data.length);
    write(data);
  }

}
