package com.nextone.util;
import java.awt.*;
import java.awt.event.*;
import javax.swing.*;
import com.nextone.common.Task;

public class ProgressDialog extends JDialog implements ActionListener{
  private Task task;
  private Frame   parent;
  private String  title;
  private String  msg;
  private boolean showCancel;
  private boolean isClose;
  
  private int animation;

  private JButton cancel;
  private JLabel  msgLabel;
  private JPanel  animationPanel;
  private StatusAnimation statusAnimation;

  int i=0;
	public ProgressDialog (String title,String msg,Frame parent,Task task,int animation, boolean showCancel, boolean isClose) {
	  super(parent, title, true);
    this.title  = title;
    this.msg    = msg;
    this.parent = parent;
    this.task   = task;
    this.animation  = animation;
    this.showCancel = showCancel;
    this.isClose    = isClose;

    //  create the gui
    JPanel  panel = new JPanel(new BorderLayout());
    msgLabel  = new JLabel(msg, JLabel.CENTER);
	  panel.add(msgLabel,BorderLayout.SOUTH);

    animationPanel = new JPanel(new BorderLayout());
	  panel.add(animationPanel,BorderLayout.CENTER);

    statusAnimation = null;
    switch(animation){
      case  ProgressStatus.FILE_ANIMATION:
        statusAnimation = new FileAnimation();
        break;
      case  ProgressStatus.BOOK_ANIMATION:
        statusAnimation = new BookAnimation();
        break;
      case  ProgressStatus.PROGRESS_ANIMATION:
        statusAnimation = new ProgressAnimation(task.getMaxCount());
        break;
    }
    if(statusAnimation  !=  null)
      animationPanel.add(statusAnimation, BorderLayout.CENTER);

	  FocusListener fl = SysUtil.getDefaultButtonFocusListener();
	  cancel = new JButton("Cancel");
	  cancel.setActionCommand("cancel");
	  cancel.addFocusListener(fl);
	  cancel.addActionListener(this);
	  getRootPane().setDefaultButton(cancel);

	  Object [] stuff = {panel};
	  Object [] btns = {cancel};

	  JOptionPane optionPane = new JOptionPane(stuff, JOptionPane.PLAIN_MESSAGE, JOptionPane.YES_NO_OPTION, null, btns, null);
	  setContentPane(optionPane);

    if(isClose){
      addWindowListener(new WindowAdapter() {
       public void windowClosing (WindowEvent we) {
	        actionPerformed(new ActionEvent(cancel, 0, "cancel"));
	      }
       });
    }else
	    setDefaultCloseOperation(JDialog.DO_NOTHING_ON_CLOSE);

	  setSize(400,200);
	  if (parent  != null)
		  setLocation(((int) parent.getLocation().getX() + (parent.getSize().width - getBounds().width)/2), (int)(parent.getLocation().getY() + (parent.getSize().height - getBounds().height)/2));
    cancel.setVisible(showCancel);
	}


	public void update(){
    Runnable updateStatus = new Runnable() {
      public void run() {
        if(statusAnimation  !=  null){
          statusAnimation.update(task.getCurrentCount());
        }
      }
    };
    SwingUtilities.invokeLater(updateStatus);

  }

  public void update(String title, String msg, boolean showCancel, int animation){
    if(!this.title.equals(title))
      setTitle(title);
    if(!this.msg.equals(msg))
      msgLabel.setText(msg);
    cancel.setVisible(showCancel);
    if(this.animation !=  animation){

      switch(animation){
        case  ProgressStatus.FILE_ANIMATION:
          statusAnimation = new FileAnimation();
          break;
        case  ProgressStatus.BOOK_ANIMATION:
          statusAnimation = new BookAnimation();
          break;
        case  ProgressStatus.PROGRESS_ANIMATION:
          statusAnimation = new ProgressAnimation(task.getMaxCount());
          break;
      }
      if(statusAnimation  !=  null){
        animationPanel.remove(0);
        animationPanel.add(statusAnimation, BorderLayout.CENTER,0);
      }



      this.animation = animation;
      this.msg = msg;
      this.title = title;  
      validate();
      update(getGraphics());
      update();

    }

  }
         

	public void actionPerformed(ActionEvent evt) {
	  String cmd = evt.getActionCommand();
	  if(cmd.equals("cancel")){
      task.cancelTask();  // stop receiving the packets
	  }
	}

  public void cleanup(){
  }

}
