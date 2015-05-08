package com.nextone.util;

import java.util.*;
import com.nextone.util.LimitExceededException;

/**
 * This is a generic class which takes in a bunch of data, stores them
 * in a queue and processes them one by one.
 *
 * There are two ways to use this class:
 *
 * 1) Method 1 is when the queue processeor needs to process only one type
 * of data, i.e., there is only one DataProcessor for processing the data
 * queued. (homogenous QueueProcessor)
 * To use this method, get an instance of QueueProcessor by calling
 * <code>getInstance(DataProcessor dp)</code>. Then use the method <code>
 * add(Object data)</code> to queue up for processing. Calling the method
 * <code>add(Object data, DataProcessor dp)</code> on this instance of
 * the QueueProcessor will result in an IllegalArgumentException.
 *
 * 2) Method 2 is when a queue processor needs to process different types
 * of data, i.e., there are different DataProcessor instances for processing
 * the data queued. (heterogenous QueueProcessor)
 * To use this method, get an instance of QueueProcessor by calling <code>
 * getInstance()</code>. Then use the method <code>add(Object Data,
 * DataProcessor dp)</code> to queue up for processing. Calling the method
 * <code>add(Object data)</code> on this instance of the QueueProcessor will
 * result in an IllegalArgumentException.
 *
 * After getting an instance of the QueueProcessor, you need to call the
 * <code>start()</code> method before starting to use the class (i.e., call
 * any of it's other methods).
 */
public class QueueProcessor extends Thread {
  protected QueueProcessor.DataProcessor dp;
  protected final int procType;
  protected LinkedList ll;
  protected boolean sleeping, keepRunning;
  protected static int numQP;
  protected int limit = 0;

  protected static final int HETEROGENOUS = 0;
  protected static final int HOMOGENOUS = 1;

  /**
   * get an instance of the QueueProcessor to process a single type
   * of data, can queue unlimited amount of data
   *
   * @param dp the data processor to process the data queued
   */
  public static QueueProcessor getInstance (QueueProcessor.DataProcessor dp) {
    return QueueProcessor.getInstance(dp, Thread.currentThread().getThreadGroup(), "QueueProcessor-" + (++numQP));
  }

  /**
   * get an instance of the QueueProcessor to process a single type
   * of data, can queue unlimited amount of data
   *
   * @param dp the data processor to process the data queued
   * @param tg the ThreadGroup this thread should belong to
   * @param threadName the name of this thread
   */
  public static QueueProcessor getInstance (QueueProcessor.DataProcessor dp, ThreadGroup tg, String threadName) {
    int pt = (dp == null)?HETEROGENOUS:HOMOGENOUS;
    return new QueueProcessor(dp, pt, tg, threadName);
  }

  /**
   * get an instance of the QueueProcessor to process different types
   * of data, can queue unlimited amount of data
   * same as calling getInstance(null)
   */
  public static QueueProcessor getInstance () {
    return QueueProcessor.getInstance(null);
  }

  /**
   * get an instance of the QueueProcessor to process different types
   * of data, can queue umlimited amount of data
   *
   * @param dp the data processor to process the data queued
   * @param tg the ThreadGroup this thread should belong to
   */
  public static QueueProcessor getInstance (ThreadGroup tg, String threadName) {
    return QueueProcessor.getInstance(null, tg, threadName);
  }

  protected QueueProcessor (QueueProcessor.DataProcessor dp, int procType, ThreadGroup tg, String threadName) {
    super(tg, threadName);
    this.dp = dp;
    this.procType = procType;
    this.ll = new LinkedList();
    setDaemon(true);
  }

  /**
   * call this method to queue up a piece of data for processing by
   * a homogenous queue processor (processes only one type of data)
   *
   * @param data the data to be processed
   * @exception IllegalArgumentException if the QueueProcessor is created
   * as a HETEROGENOUS processor
   * @exception IllegalThreadStateException if the method is called
   * before the queue processor is started
   * @exception LimitExceededException if trying to add more messages
   * than the queue processor's limit
   */
  public void add (Object data) {
    checkHomogenousAdd();
    addDataToQueue(data);
  }

  /**
   * call this method to queue up a piece of data for processing by a
   * heterogenous queue processor (processes different types of data)
   *
   * @param data the data to be processed
   * @param dataProc the DataProcessor which processes this data
   * @exception IllegalArgumentException if the QueueProcessor is created
   * as a HOMOGENOUS processor
   * @exception IllegalThreadStateException if the method is called
   * before the queue processor is started
   * @exception LimitExceededException if trying to add more messages
   * than the queue processor's limit
   */
  public void add (Object data, QueueProcessor.DataProcessor dataProc) {
    checkHeterogenousAdd();
    addDataToQueue(new Pair(data, dataProc));
  }

  protected void checkHomogenousAdd () {
    if (procType == HETEROGENOUS)
      throw new IllegalArgumentException(getName() + ": cannot call this method for a heterogenous queue processor");

    checkCommonAdd();
  }

  protected void checkHeterogenousAdd () {
    if (procType == HOMOGENOUS)
      throw new IllegalArgumentException(getName() + ": cannot call this method for a homogenous queue processor");

    checkCommonAdd();
  }

  protected void checkCommonAdd () {
    if (keepRunning == false)
      throw new IllegalThreadStateException(getName() + ": thread currently not running");

    if (limit != 0 && getNumInQueue() >= limit)
      throw new LimitExceededException(getName() + ": limit " + limit + " exceeded");
  }

  protected void addDataToQueue (Object pair) {
    synchronized(ll) {
      ll.addLast(pair);
      ll.notifyAll();
    }
  }

  /**
   * return the number of data objects sitting in the queue to be
   * processed
   */
  public int getNumInQueue () {
    int size = 0;

    if (keepRunning) {
      size = ll.size();
      if (!sleeping)
        size++;
    }

    return size;
  }

  /**
   * clears all the waiting data in the queue, does not process them
   */
  public void clearQueue () {
    if (keepRunning) {
      synchronized(ll) {
        ll.clear();
      }
    }
  }

  /**
   * call this method to set the maximum number of messages the
   * queue processor will queue up before dropping any additional
   * message being queued
   *
   * @param limit the limit (zero for no limits)
   */
  public void setLimit (int limit) {
    this.limit = limit;
  }

  /**
   * returns the number of messages this queue processor will process,
   * 0 if there is no such limit
   */
  public int getLimit () {
    return limit;
  }

  /**
   * call this to stop this thread
   */
  public void stopRunning () {
    if (keepRunning == false)
      return;

    keepRunning = false;
    synchronized (ll) {
      ll.notifyAll();
    }
    dp = null;
    try {
      join(2000);
    } catch (InterruptedException ie) {}
  }


  // do any pre-processing of the list before we take the first one
  // on the list and process him
  // (this method checks for the new limit and removes items from the
  // back of the list to confirm to the new limit)
  protected void preProcess () {
    // if the current queue limit was changed, drop the last
    // comers to fit the limit
    while (limit != 0 && ll.size() > limit) {
      ll.removeLast();
    }
  }


  // do any post processing here
  // (this method is empty for now)
  protected void postProcess () {

  }


  // check if we should continue processing this queue
  // (this method simply checks if the queue is non-empty)
  protected boolean canProceed () {
    ll.getFirst();  // throws NoSuchElementException if queue is empty
    return keepRunning;
  }


  // the thread which keeps running and processes the data
  public void run () {
    keepRunning = true;
    while (keepRunning) {
      synchronized (ll) {
        sleeping = false;
      }
      try {
        Object data = null;
        DataProcessor datap = dp;
        synchronized (ll) {
          preProcess();

          if (canProceed()) {
            data = ll.removeFirst();
            if (data instanceof Pair) {
              datap = ((Pair)data).datap;
              data = ((Pair)data).data;
            }
            datap.processData(data);
          } else
            throw new NoSuchElementException();
        }
      } catch (NoSuchElementException ne) {
        synchronized (ll) {
          // do any post processing
          postProcess();

          // nothing more to do, just sleep
          sleeping = true;
          try {
            ll.wait();
          } catch (InterruptedException ie) {}
        }
      }
    }
  }


  // used in heterogenous DataProcessor to hold the data and it's
  // processor together
  protected class Pair {
    QueueProcessor.DataProcessor datap;
    Object data;

    Pair (Object data, QueueProcessor.DataProcessor datap) {
      this.data = data;
      this.datap = datap;
    }

    public String toString () {
      return data.toString() + "[" + datap.toString() + "]";
    }
  }

  public String toString () {
    String s = (procType == HOMOGENOUS)?"homogenous":"heterogenous";
    StringBuffer sb = new StringBuffer(getClass().toString());
    sb.append(" [");
    sb.append(s);
    sb.append("][started=");
    sb.append(keepRunning);
    sb.append("][sleeping=");
    sb.append(sleeping);
    sb.append("]");
    //		 Object [] o = ll.toArray();
    //		 for (int i = 0; o != null && i < o.length; i++) {
    //			sb.append("\n");
    //			sb.append(o[i].toString());
    //		 }
    return sb.toString();
  }

  /**
   * this is the interface that a class needs to implement in order to
   * use this QueueProcessor class
   */
  public static interface DataProcessor {
    public void processData (Object data);
  }

}
