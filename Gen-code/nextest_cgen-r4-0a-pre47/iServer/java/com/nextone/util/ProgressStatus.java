package com.nextone.util;

import javax.swing.JDialog;
import java.awt.Frame;
import com.nextone.common.Task;

public class ProgressStatus{
  int     maxRecords;
  String  title;
  String  msg;
  boolean showCancel;
  boolean isClose;
  boolean updateStatus;
  Task    task;
  Frame   parent;
  int     delay;
  int     animationDelay;
  int     animation;

  ProgressThread progressThread;

  public static final int INVALID  = -1;
  public static final int FILE_ANIMATION      = 0;
  public static final int BOOK_ANIMATION      = 1;
  public static final int PROGRESS_ANIMATION  = 2;

  public final static int ONE_SECOND = 1000;



  // status window, which shows only text message
  public ProgressStatus(String title,String msg,Frame parent,int delay,Task task){
    this(title,msg,parent,delay,task,-1);
  }

  // status window, which shows text message and animation
  public ProgressStatus(String title,String msg,Frame parent,int delay,Task task,int animation){
    this(title,msg,parent,delay,task,animation,true);
  }

  // cancel button is enabled/disabled.
  public ProgressStatus(String title,String msg,Frame parent,int delay,Task task,int animation, boolean showCancel){
    this(title,msg,parent,delay,task,animation,showCancel,false);
  }

  // window close is enabled/disabled
  public ProgressStatus(String title,String msg,Frame parent,int delay,Task task,int animation, boolean showCancel, boolean isClose){
    this.title  = title;
    this.msg    = msg;
    this.parent = parent;
    this.task   = task;
    this.animation  = animation;
    this.showCancel = showCancel;
    this.isClose    = isClose;
    this.delay      = delay;
    animationDelay  = ONE_SECOND;
    updateStatus  = false;
    setAnimationDelay();

    progressThread  = null;
  }

  public void show(){
    progressThread  = new ProgressThread();
    progressThread.start();
  }

  private void setAnimationDelay(){
    
    switch(animation){
      case FILE_ANIMATION:
        animationDelay  = ONE_SECOND;
        break;
      case BOOK_ANIMATION:
        animationDelay  = 400;
        break;
      case PROGRESS_ANIMATION:
        animationDelay  = ONE_SECOND;
        break;
    }
  }

  public void updateTitle(String t){
    update(t,msg,showCancel,animation);
  }

  public void updateMessage(String m){
    update(title,m,showCancel,animation);
  }

  public void showCancel(boolean isVisible){
    update(title,msg,isVisible,animation);
  }

  public void updateAnimation(int a){
    update(title,msg,showCancel,a);
  }

  public void update(String title, String msg, boolean showCancel, int animation){
    this.title  = title;
    this.msg  = msg;
    this.showCancel = showCancel;
    this.animation  = animation;
    setAnimationDelay();
    updateStatus  = true;
  }

  private void cleanup(){
    parent  = null;
    task  = null;
    progressThread  = null;
  }


 
  /**
   * Show the status bar as long as the task is not completed
   **/
  public class ProgressThread extends Thread{

    ProgressDialog  progressDialog;

    public ProgressThread(){
      progressDialog = null;
    }

    public void run(){
      //  for small tasks don't show the status bar. 
      try{
        Thread.sleep(delay);
      }catch(Exception e){}

      if(task.isRunning()){
         progressDialog  = new ProgressDialog(title,msg,parent,task,animation,showCancel,isClose);
        // make the dialog box visible in differnt thread, so that it will not block
        //  further processing
        new VisibleThread((JDialog)progressDialog).start();
        while(task.isRunning()){
          if(updateStatus){
              progressDialog.update(title,msg,showCancel,animation);
              updateStatus  = false;
          }else
              progressDialog.update();
          try{
            Thread.sleep(animationDelay);
          }catch(Exception e){}
        }

      }
      cleanup();
    }

    //  close the status window
    public void cleanup(){
      if(progressDialog !=  null  && progressDialog.isVisible()){
        progressDialog.setVisible(false);
        progressDialog.cleanup();
        progressDialog.dispose();
      }
      ProgressStatus.this.cleanup();
    }

  }

  public class VisibleThread extends Thread{
    JDialog dialog;

    public VisibleThread(JDialog dialog){
      this.dialog = dialog;
    }
    public void run(){
      dialog.setVisible(true);
    }
  }
}


