package com.nextone.util;

import java.util.*;
import com.nextone.util.LimitExceededException;

/**
 * This class is a subclass of QueueProcessor, and adds the ability to add things
 * in the queue with a delay specified. The queue won't process the task added
 * until the specified time arrives.
 *
 * @see com.nextone.util.QueueProcessor
 */
public class DelayedQueueProcessor extends QueueProcessor {
  protected java.util.Timer interruptTimer;
  protected java.util.TimerTask interruptTimerTask;

  /**
   * get an instance of the DelayedQueueProcessor to process a single type
   * of data, can queue unlimited amount of data, need to cast the return value
   * to DelayedQueueProcessor type
   *
   * @param dp the data processor to process the data queued
   * @param tg the ThreadGroup this thread should belong to
   * @param threadName the name of this thread
   */
  public static QueueProcessor getInstance (QueueProcessor.DataProcessor dp, ThreadGroup tg, String threadName) {
    return (DelayedQueueProcessor)new DelayedQueueProcessor(dp, (dp == null)?HETEROGENOUS:HOMOGENOUS, tg, threadName);
  }


  public static QueueProcessor getInstance (QueueProcessor.DataProcessor dp) {
    return (DelayedQueueProcessor)DelayedQueueProcessor.getInstance(dp, Thread.currentThread().getThreadGroup(), "DelayedQueueProcessor-" + (++numQP));
  }


  public static QueueProcessor getInstance () {
    return (DelayedQueueProcessor)DelayedQueueProcessor.getInstance(null);
  }


  public static QueueProcessor getInstance (ThreadGroup tg, String threadName) {
    return (DelayedQueueProcessor)DelayedQueueProcessor.getInstance(null, tg, threadName);
  }


  protected DelayedQueueProcessor (QueueProcessor.DataProcessor dp, int procType, ThreadGroup tg, String threadName) {
    super(dp, procType, tg, threadName);
    interruptTimer = new java.util.Timer(true);
    interruptTimerTask = new java.util.TimerTask () {
        public void run () {
          synchronized (ll) {
            ll.notifyAll();
          }
        }
      };
  }


  /**
   * call this method to queue up a piece of data for processing by
   * a homogenous queue processor (processes only one type of data)
   *
   * @param data the data to be processed
   * @param time milliseconds since epoch (similar to System.currentTimeMillis)
   * at which this data has to be processed
   * @exception IllegalArgumentException if the QueueProcessor is created
   * as a HETEROGENOUS processor
   * @exception IllegalThreadStateException if the method is called
   * before the queue processor is started
   * @exception LimitExceededException if trying to add more messages
   * than the queue processor's limit
   */
  public void add (Object data, long time) {
    checkHomogenousAdd();
    addDataToQueue(new Pair(data, dp, time));
  }


  /**
   * call this method to queue up a piece of data for processing by
   * a heterogenous queue processor (processes different types of data)
   *
   * @param data the data to be processed
   * @param dataProc the DataProcessor which processes this data
   * @param time milliseconds since epoch (similar to System.currentTimeMillis)
   * at which this data has to be processed
   * @exception IllegalArgumentException if the QueueProcessor is created
   * as a HETEROGENOUS processor
   * @exception IllegalThreadStateException if the method is called
   * before the queue processor is started
   * @exception LimitExceededException if trying to add more messages
   * than the queue processor's limit
   */
  public void add (Object data, QueueProcessor.DataProcessor dataProc, long time) {
    checkHeterogenousAdd();
    addDataToQueue(new Pair(data, dataProc, time));
  }


  // this method re-orders the list according to the time delays
  protected void preProcess () {
    // re-order
    Comparator c = new Comparator () {
        public int compare (Object o1, Object o2) {
          return (int)(((Pair)o1).time - ((Pair)o2).time);
        }
      };
    Collections.sort(ll, c);
    super.preProcess();  // to limit queue size
  }


  // this method kicks off a timer to wake us up after a certain delay
  protected void postProcess () {
    try {
      Pair p = (Pair)ll.getFirst();
      interruptTimer.schedule(new java.util.TimerTask () {
          public void run () {
            synchronized (ll) {
              ll.notifyAll();
            }
          }
        }, p.time - System.currentTimeMillis());
    } catch (Exception e) {}
  }


  // see if the next guy is ready to proceed
  protected boolean canProceed () {
    if (super.canProceed()) {
      Pair p = (Pair)ll.getFirst();
      // go ahead and process things that are upto half seconds away
      return ((p.time - System.currentTimeMillis()) < 500);
    }

    return false;
  }


  protected class Pair extends QueueProcessor.Pair {
    long time;

    Pair (Object data, QueueProcessor.DataProcessor datap) {
      this(data, datap, 0);
    }

    Pair (Object data, QueueProcessor.DataProcessor datap, long time) {
      super(data, datap);
      this.time = time;
    }

    public String toString () {
      return super.toString() + "[" + time + "]";
    }
  }

  public String dumpQueue () {
    StringBuffer sb = new StringBuffer();
    synchronized (ll) {
      Iterator it = ll.iterator();
      while (it.hasNext()) {
        sb.append(it.next().toString());
        sb.append("\n");
      }
    }

    return sb.toString();
  }

}
