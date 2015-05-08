/**
 * Static classes to pop up messages and play sound
 */
package com.nextone.util;

import javax.swing.*;
import javax.swing.event.*;
import javax.swing.text.*;
import javax.swing.border.*;
import java.awt.*;
import java.awt.event.*;
import java.applet.*;
import java.beans.*;
import java.io.*;
import java.util.*;

import com.nextone.util.JMask.*;

public class PopMessage {
  private static AudioClip errorClip, successClip, infoClip;
  private static boolean canPlaySound = false;

  // one time init stuff
  static {
    // load sounds
    /*
    try {
      errorClip = Applet.newAudioClip(Class.forName("com.nextone.util.PopMessage").getResource("/com/nextone/util/chord.wav"));
      successClip = Applet.newAudioClip(Class.forName("com.nextone.util.PopMessage").getResource("/com/nextone/util/tada.wav"));
      infoClip = Applet.newAudioClip(Class.forName("com.nextone.util.PopMessage").getResource("/com/nextone/util/ding.wav"));
    } catch (ClassNotFoundException ce) {
      Logger.fatal("Could not find sound files");
    }
    */
  }

  // no PopMessage object possible
  private PopMessage () {}

  /**
   * enable/disable sounds, by default it is disabled
   */
  public static void setSoundEnabled (boolean flag) {
    canPlaySound = flag;
  }

  /**
   * make error sound
   */
  public static void playError () {
    //    if (canPlaySound)
    //      errorClip.play();
  }

  /**
   * make success sound
   */
  public static void playSuccess () {
    //    if (canPlaySound)
    //      successClip.play();
  }

  /**
   * make info sound
   */
  public static void playInfo () {
    //    if (canPlaySound)
    //      infoClip.play();
  }

  private static void showFromThread (Runnable runnable) {
    try {
      if (SwingUtilities.isEventDispatchThread())
	runnable.run();
      else
	SwingUtilities.invokeAndWait(runnable);
    } catch (Exception e) {
      e.printStackTrace();
    }
  }

  /**
   * make error sound and pop up a modal dialog window containing the
   * string passed
   */
  public static void showError (Component parent, String str) {
    playError();
    JOptionPane.showMessageDialog(parent, str + "\n", "Error", JOptionPane.ERROR_MESSAGE);
  }

  /**
   * make error sound and pop up a modal dialog window containing the
   * string passed
   */
  public static void showError (Component parent, String str, String details) {
    showError(parent, "Error", str, details);
  }

  /**
   * thread safe popups
   *
   * @param parent the parent component on which this dialog will anchor
   * @param str the error message
   * @param details the detailed error message
   */
  public static void showErrorFromThread (final Component parent, final String str, final String details) {
    showFromThread(new Runnable() {
	public void run () {
	  showError(parent, str, details);
	}
      });
  }

  /**
   * make error sound and pop up a modal dialog window containing the
   * string passed
   */
  public static void showError (Component parent, String title, String str, String details) {
    playError();
    if (parent == null || parent instanceof JFrame) {
      new DetailDialog((JFrame)parent, str + "\n", title, JOptionPane.ERROR_MESSAGE, details).setVisible(true);
    } else {
      if (details == null || details.length() == 0)
	JOptionPane.showMessageDialog(parent, str + "\n", title, JOptionPane.ERROR_MESSAGE);
      else
	JOptionPane.showMessageDialog(parent, str + "\n\nDetails: \n" + details, title, JOptionPane.ERROR_MESSAGE);
    }
  }

  public static void showErrorFromThread (final Component parent, final String title, final String str, final String details) {
    showFromThread(new Runnable () {
	public void run () {
	  showError(parent, title, str, details);
	}
      });
  }

  /**
   * make error sound and pop up a modal dialog window containing the
   * string and the stackTrace of the exception passed
   */
  public static void showError (Component parent, String str, Object o) {
    showError(parent, "Error", str, o);
  }

  public static void showErrorFromThread (final Component parent, final String str, final Object o) {
    showFromThread(new Runnable () {
	public void run () {
	  showError(parent, str, o);
	}
      });
  }

  /**
   * make error sound and pop up a modal dialog window containing the
   * string passed
   */
  public static void showError (Component parent, String title, String str, Object o) {
    if (o instanceof Exception) {
      StringWriter sw = new StringWriter();
      ((Exception)o).printStackTrace(new PrintWriter(sw));
      sw.flush();
      try {
	sw.close();
      } catch (Exception e) {}
      showError(parent, title, str, sw.toString());
    } else {
      showError(parent, title, str, o.toString());
    }
  }

  public static void showErrorFromThread (final Component parent, final String title, final String str, final Object o) {
    showFromThread(new Runnable () {
	public void run () {
	  showError(parent, title, str, o);
	}
      });
  }

  /**
   * a generic message to show when a device is not responding
   * (used by iView, though not clean to have it here, it is
   *  here for convenience)
   */
  public static void showNoResponse (Component parent, String deviceId, String deviceType, Exception e) {
    if (deviceId == null)
      deviceId = "the " + deviceType;

    PopMessage.showError(parent, "No response from " + deviceId, "Please check if:\n    a. the " + deviceType + " is up\n    b. the read/write passwords match\n    c. the iView and the " + deviceType + " have compatible software versions\n\n" + e.toString());
  }

  /**
   * a generic message to show when a device is not responding
   * (used by iView, though not clean to have it here, it is
   *  here for convenience)
   */
  public static void showNoResponseFromThread (final Component parent, final String deviceId, final String deviceType, final Exception exception) {
    showFromThread(new Runnable () {
	public void run () {
	  showNoResponse(parent, deviceId, deviceType, exception);
	}
      });
  }

  /**
   * make a success sound and pop up a modal dialog window containing the
   * string passed
   */
  public static void showSuccess (Component parent, String str) {
    playSuccess();
    JOptionPane.showMessageDialog(parent, str + "\n", "Success", JOptionPane.PLAIN_MESSAGE);
  }

  /**
   * make a success sound and pop up a modal dialog window containing the
   * string passed
   */
  public static void showSuccess (Component parent, String str, String details) {
    showSuccess(parent, "Success", str, details);
  }

  public static void showSuccessFromThread (final Component parent, final String str, final String details) {
    showFromThread(new Runnable () {
	public void run () {
	  showSuccess(parent, str, details);
	}
      });
  }

  /**
   * make a success sound and pop up a modal dialog window containing the
   * string passed
   */
  public static void showSuccess (Component parent, String title, String str, String details) {
    playSuccess();
    if (parent == null || parent instanceof JFrame) {
      new DetailDialog((JFrame)parent, str + "\n", title, JOptionPane.PLAIN_MESSAGE, details).setVisible(true);
    } else {
      if (details == null || details.length() == 0)
	JOptionPane.showMessageDialog(parent, str + "\n", title, JOptionPane.PLAIN_MESSAGE);
      else
	JOptionPane.showMessageDialog(parent, str + "\n\nDetails: \n" + details, title, JOptionPane.PLAIN_MESSAGE);
    }
  }

  public static void showSuccessFromThread (final Component parent, final String title, final String str, final String details) {
    showFromThread(new Runnable () {
	public void run () {
	  showSuccess(parent, title, str, details);
	}
      });
  }

  /**
   * make warning sound and pop up a modal dialog window containing the
   * string passed
   */
  public static void showWarning (Component parent, String str) {
    playError();
    JOptionPane.showMessageDialog(parent, str + "\n", "Warning", JOptionPane.WARNING_MESSAGE);
  }

  /**
   * make error sound and pop up a non-modal dialog window containing the
   * string passed
   */
  public static void showWarning (Component parent, String str, String details) {
    playError();
    if (parent == null || parent instanceof JFrame) {
      new DetailDialog((JFrame)parent, str + "\n", "Warning", JOptionPane.WARNING_MESSAGE, details, false).setVisible(true);
    } else {
      if (details == null || details.length() == 0)
	JOptionPane.showMessageDialog(parent, str + "\n", "Warning", JOptionPane.WARNING_MESSAGE);
      else
	JOptionPane.showMessageDialog(parent, str + "\n\nDetails: \n" + details, "Warning", JOptionPane.WARNING_MESSAGE);
    }
  }

  public static void showWarningFromThread (final Component parent, final String str, final String details) {
    showFromThread(new Runnable () {
	public void run () {
	  showWarning(parent, str, details);
	}
      });
  }

  /**
   * make info sound and pop up a modal dialog window containing the
   * string passed
   */
  public static void showInfo (Component parent, String str) {
    playInfo();
    JOptionPane.showMessageDialog(parent, str + "\n", "Info", JOptionPane.INFORMATION_MESSAGE);
  }

  /**
   * make info sound and pop up a modal dialog window containing the
   * string passed
   */
  public static void showInfo (Component parent, String str, String details) {
    showInfo(parent, "Info", str, details);
  }

  public static void showInfoFromThread (final Component parent, final String str, final String details) {
    showFromThread(new Runnable () {
	public void run () {
	  showInfo(parent, str, details);
	}
      });
  }

  /**
   * make info sound and pop up a modal dialog window containing the
   * string passed
   */
  public static void showInfo (Component parent, String title, String str, String details) {
    playInfo();
    if (parent == null || parent instanceof JFrame) {
      new DetailDialog((JFrame)parent, str + "\n", title, JOptionPane.INFORMATION_MESSAGE, details).setVisible(true);
    } else {
      if (details == null || details.length() == 0)
	JOptionPane.showMessageDialog(parent, str + "\n", title, JOptionPane.INFORMATION_MESSAGE);
      else
	JOptionPane.showMessageDialog(parent, str + "\n\nDetails: \n" + details, title, JOptionPane.INFORMATION_MESSAGE);
    }
  }

  public static void showInfoFromThread (final Component parent, final String title, final String str, final String details) {
    showFromThread (new Runnable () {
	public void run () {
	  showInfo(parent, title, str, details);
	}
      });
  }

  private static class DetailDialog extends JDialog implements FileSaver {
    private JOptionPane optionPane;
    private String msg, details;
    private Component parent;
    private JButton okb, db;
    private int width, height;
    private JPanel dp;
    private FocusListener fl;

    DetailDialog (JFrame parent, String msg, String title, int msgType, String details) {
      this(parent, msg, title, msgType, details, true);
    }

    DetailDialog (JFrame parent, String msg, String title, int msgType, String details, boolean isModal) {
      super(parent, title, isModal);

      this.parent = parent;
      this.msg = msg;
      this.details = details;

      dp = new JPanel();
      dp.setLayout(new BoxLayout(dp, BoxLayout.Y_AXIS));
      dp.add(Box.createVerticalStrut(15));
      JTextArea jt = new JTextArea();
      if (details != null)
	jt.setText(details);
      jt.setBackground(null);
      jt.setEditable(false);
      JScrollPane jsp = new JScrollPane(jt);
      dp.add(jsp);
      JButton saveButton = new JButton("Save To File");
      saveButton.setToolTipText("Save the details to a file");
      saveButton.addActionListener(new SaveFileListener(this, this, null));
      dp.add(Box.createVerticalStrut(5));
      dp.add(saveButton);

      fl = SysUtil.getDefaultButtonFocusListener();
      Object [] stuff = new Object[1];
      if (SysUtil.getNumLines(msg) < 10)
	stuff[0] = msg;
      else {
	JTextArea jta = new JTextArea(msg, 5, 40);
	jta.setBackground(null);
	jta.setEditable(false);
	JScrollPane sp = new JScrollPane(jta);
	sp.setPreferredSize(new Dimension(400, 150));
	stuff[0] = sp;
      }
      okb = new JButton("OK");
      okb.addFocusListener(fl);
      okb.addActionListener(new ActionListener () {
	  public void actionPerformed (ActionEvent ae) {
	    close();
	  }
	});
      db = new JButton("Details");
      db.addFocusListener(fl);
      db.setActionCommand("details");
      db.addActionListener(new ActionListener () {
	  public void actionPerformed (ActionEvent ae) {
	    String cmd = ae.getActionCommand();

	    if (cmd.equals("details")) {
	      db.setText("No details");
	      db.setActionCommand("no-details");
	      addDetails();
	    } else {
	      db.setText("Details");
	      db.setActionCommand("details");
	      removeDetails();
	    }
	  }
	});
      Object [] btns = null;
      if (details == null) {
	btns = new Object [1];
	btns[0] = okb;
      } else {
	btns = new Object [2];
	btns[0] = okb;
	btns[1] = db;
      }
      optionPane = new JOptionPane(stuff, msgType, JOptionPane.YES_OPTION, null, btns, btns[0]);
      setContentPane(optionPane);
      setDefaultCloseOperation(JDialog.DO_NOTHING_ON_CLOSE);
      addWindowListener(new WindowAdapter() {
	  public void windowClosing (WindowEvent we) {
	    close();
	  }
	});

      addComponentListener( new ComponentListener() {
	  public void componentHidden (ComponentEvent ce) {}
	  public void componentMoved (ComponentEvent ce) {
	    repaint();
	  }
	  public void componentShown (ComponentEvent ce) {
	    repaint();
	  }
	  // don't let the size to be less than the preferred size
	  public void componentResized (ComponentEvent ce) {
	    int w = width, h = height;
	    Dimension curSize = getSize();
	    if (curSize.width > width)
	      w = curSize.width;
	    if (curSize.height > height)
	      h = curSize.height;
	    if (curSize.width < w || curSize.height < h)
	      setSize(w, h);
	    //					 pack();
	    repaint();
	  }
	});

      pack();
      // center this over the main screen
      if (parent == null) {
	Dimension d = Toolkit.getDefaultToolkit().getScreenSize();
	setLocation( (d.width - getBounds().width)/2,
		     (d.height - getBounds().height)/2 );
      } else {
	setLocation(((int)parent.getLocation().getX() + (parent.getSize().width - getBounds().width)/2), ((int)parent.getLocation().getY() + (parent.getSize().height - getBounds().height)/2));
      }
      width = getBounds().width;
      height = getBounds().height;
      jsp.setPreferredSize(new Dimension(width, height/2));
    }

    // to save the info to a file
    public void writeData (FileOutputStream fos) throws IOException {
      DataOutputStream dos = new DataOutputStream(fos);
      if (msg != null)
	dos.writeBytes(msg);
      dos.writeBytes("\n");
      if (details != null)
	dos.writeBytes(details);
      dos.close();
    }

    private void close () {
      okb.removeFocusListener(fl);
      db.removeFocusListener(fl);
      setVisible(false);
      dispose();
      destroy();
    }

    private void addDetails () {
      getContentPane().add(dp);
      validate();
      pack();
    }

    private void removeDetails () {
      getContentPane().remove(dp);
      validate();
      pack();
    }
			   
    private void destroy () {
      optionPane = null;
      parent = null;
      details = null;
      msg = null;
      okb = null;
      db = null;
      dp = null;
    }

  }

  public interface InputDialogRequester {
    /**
     * "input" will be either an Object (usually String) if only one
     * input component was used, or an Object [] (usually String [])
     * if several input components were passed in the dialog
     */
    public boolean isValidInput (Object input);
  }

  /**
   * simply calls PopMessage.showInputDialog(parent, inputComponent, title, message, prompt, borderMessage, idr, null, JOptionPane.PLAIN_MESSAGE)
   *
   */
  public static Object showInputDialog (Component parent, JTextComponent inputComponent, String title, String message, String prompt, String borderMessage, InputDialogRequester idr) {
    return showInputDialog(parent, inputComponent, title, message, prompt, borderMessage, idr, null, JOptionPane.PLAIN_MESSAGE);
  }

  /**
   * simply calls PopMessage.showInputDialog(parent, inputComponent, title, message, prompt, borderMessage, idr, btnStr, JOptionPane.PLAIN_MESSAGE)
   *
   */
  public static Object showInputDialog (Component parent, JTextComponent inputComponent, String title, String message, String prompt, String borderMessage, InputDialogRequester idr, String btnStr) {
    return showInputDialog(parent, inputComponent, title, message, prompt, borderMessage, idr, btnStr, JOptionPane.PLAIN_MESSAGE);
  }

  /**
   * simply calls PopMessage.showInputDialog(parent, inputComponent, title, message, prompt, borderMessage, idr, null, JOptionPane.PLAIN_MESSAGE)
   *
   */
  public static Object showInputDialog (Component parent, JTextComponent [] inputComponent, String title, String message, String [] prompt, String borderMessage, InputDialogRequester idr) {
    return showInputDialog(parent, inputComponent, title, message, prompt, borderMessage, idr, null, JOptionPane.PLAIN_MESSAGE);
  }

  /**
   * simply calls PopMessage.showInputDialog(parent, inputComponent, title, message, prompt, borderMessage, idr, btnStr, JOptionPane.PLAIN_MESSAGE)
   *
   */
  public static Object showInputDialog (Component parent, JTextComponent [] inputComponent, String title, String message, String [] prompt, String borderMessage, InputDialogRequester idr, String btnStr) {
    return showInputDialog(parent, inputComponent, title, message, prompt, borderMessage, idr, btnStr, JOptionPane.PLAIN_MESSAGE);
  }

  /**
   * converts the inputComponent to jtc [] and prompt to pr [] and calls
   * PopMessage.showInputDialog(parent, jtc, title, message, pr, borderMessage, idr, null, msgType)
   *
   */
  public static Object showInputDialog (Component parent, JTextComponent inputComponent, String title, String message, String prompt, String borderMessage, InputDialogRequester idr, int msgType) {
    JTextComponent [] jtc = new JTextComponent [1];
    jtc[0] = inputComponent;
    String [] pr = new String [1];
    pr[0] = prompt;
    return showInputDialog(parent, jtc, title, message, pr, borderMessage, idr, null, msgType);
  }

  /**
   * converts the inputComponent to jtc [] and prompt to pr [] and calls
   * PopMessage.showInputDialog(parent, jtc, title, message, pr, borderMessage, idr, btnStr, msgType)
   *
   */
  public static Object showInputDialog (Component parent, JTextComponent inputComponent, String title, String message, String prompt, String borderMessage, InputDialogRequester idr, String btnStr, int msgType) {
    JTextComponent [] jtc = new JTextComponent [1];
    jtc[0] = inputComponent;
    String [] pr = new String [1];
    pr[0] = prompt;
    return showInputDialog(parent, jtc, title, message, pr, borderMessage, idr, btnStr, msgType);
  }

  /**
   * Shows an inout dialog. Returns the inputted value, or null if the
   * dialog was closed without any input.
   *
   * @param parent the parent component to lock on, should be an 
   * instance of a Frame or a Dialog
   * @param inputComponent the text component [] to take the input
   * @param title the title to be shown on the dialog window
   * @param message an optional message to precede the input component,
   * can be null
   * @param prompt an optional prompt [] to precede each of the input
   * component, can be null
   * @param borderMessage an optional string to display on the border
   * surrounding the input component, null if no border needed
   * @param idr an InputDialogRequester, used to validate the input
   * @param btnStr an optional string to be displayed for the action button
   * @param msgType the message type icon to show, default is plain message
   *
   * @see PopMessage.InputDialogRequester
   */
  public static Object showInputDialog (Component parent, JTextComponent [] inputComponent, String title, String message, String [] prompt, String borderMessage, InputDialogRequester idreq, String btnStr, int msgType) {
    if (prompt != null && inputComponent.length != prompt.length)
      return new IllegalArgumentException("Should have same number of prompts as the components");

    final ArrayList inputs = new ArrayList(inputComponent.length);

    class InputDialog extends JDialog implements ActionListener {
      private InputDialogRequester idr;
      private JTextComponent [] ic;

      InputDialog (Frame parent, JTextComponent [] inputComponent, String title, String message, String [] prompt, String borderMessage, InputDialogRequester idr, String btnStr, int msgType) {
	super(parent, title, true);
	constructGUI(parent, inputComponent, message, prompt, borderMessage, btnStr, msgType);
	this.idr = idr;
      }

      InputDialog (Dialog parent, JTextComponent [] inputComponent, String title, String message, String [] prompt, String borderMessage, InputDialogRequester idr, String btnStr, int msgType) {
	super(parent, title, true);
	constructGUI(parent, inputComponent, message, prompt, borderMessage, btnStr, msgType);
	this.idr = idr;
      }

      private void constructGUI (Component parent, JTextComponent [] inputComponent, String message, String [] prompt, String borderMessage, String btnStr, int msgType) {
	this.ic = inputComponent;

	Box b = new Box(BoxLayout.Y_AXIS);
	b.add(Box.createVerticalStrut(5));

	if (message != null) {
	  String m = null;
	  if (SysUtil.getNumLines(message) > 0) {
	    StringBuffer sb = new StringBuffer("<html><font color=\"#000000\">");
	    String s = message;
	    String line = null;
	    while (!s.equals("")) {
	      int index = s.indexOf("\n");
	      if (index == -1) {
		line = s;
		s = "";
	      } else {
		line = s.substring(0, index);
		s = s.substring(index+1, s.length());
	      }
	      sb.append("<p>");
	      sb.append(line);
	      sb.append("</p>");
	    }
	    sb.append("</font></html>");
	    m = sb.toString();
	  } else
	    m = message;
	  JPanel p = new JPanel();
	  p.setLayout(new FlowLayout(FlowLayout.LEFT));
	  JLabel mesg = new JLabel(m, JLabel.LEFT);
	  mesg.setForeground(Color.black);
	  p.add(mesg);
	  b.add(p);
	  b.add(Box.createVerticalStrut(3));
	}

	JPanel p = new JPanel();
	if (inputComponent.length == 1) {
	  // handle single input case differently, for optimum
	  // GUI display
	  p.setLayout(new FlowLayout(FlowLayout.CENTER));
	  if (prompt != null)
	    p.add(new JLabel(prompt[0], JLabel.CENTER));
	  p.add(inputComponent[0]);
	} else if (inputComponent.length < 4) {
	  // the following lays out in two vertical boxes and
	  // puts them together in a flowlayout. (doesn't align well
	  // for more than 3 rows)
	  p.setLayout(new FlowLayout(FlowLayout.LEFT));
	  if (prompt != null) {
	    Box bb = new Box(BoxLayout.Y_AXIS);
	    for (int i = 0; i < prompt.length; i++) {
	      JPanel jp = new JPanel();
	      jp.setLayout(new FlowLayout(FlowLayout.RIGHT));
	      jp.add(new JLabel(prompt[i], JLabel.RIGHT));
	      bb.add(jp);
	    }
	    p.add(bb);
	  }
	  Box bb = new Box(BoxLayout.Y_AXIS);
	  for (int i = 0; i < inputComponent.length; i++) {
	    inputComponent[i].selectAll();
	    JPanel jp = new JPanel();
	    jp.setLayout(new FlowLayout(FlowLayout.LEFT));
	    jp.add(inputComponent[i]);
	    bb.add(jp);
	  }
	  p.add(bb);
	} else {
	  // lay prompts and components in a grid layout
	  // (doesn't look good if any of the prompt is much
	  // smaller/larger than the components)
	  if (prompt != null)
	    p.setLayout(new GridLayout(inputComponent.length, 2));
	  else
	    p.setLayout(new GridLayout(inputComponent.length, 1));
	  for (int i = 0; i < inputComponent.length; i++) {
	    JPanel lp = new JPanel();
	    if (prompt != null) {
	      lp.setLayout(new FlowLayout(FlowLayout.RIGHT));
	      lp.add(new JLabel(prompt[i], JLabel.RIGHT));
	      p.add(lp);
	    }
	    lp = new JPanel();
	    lp.setLayout(new FlowLayout(FlowLayout.LEFT));
	    lp.add(inputComponent[i]);
	    p.add(lp);
	  }
	}

	b.add(p);
	inputComponent[0].requestFocus();

	if (borderMessage != null) {
	  TitledBorder border = BorderFactory.createTitledBorder(BorderFactory.createEtchedBorder(), borderMessage);
	  border.setTitleJustification(TitledBorder.LEFT);
	  p.setBorder(border);
	}
	b.add(Box.createVerticalStrut(5));

	FocusListener fl = SysUtil.getDefaultButtonFocusListener();
	String bs = "Set";
	if (btnStr != null)
	  bs = btnStr;
	JButton set = new JButton(bs);
	JButton cancel = new JButton("Cancel");
	set.setActionCommand("set");
	set.addFocusListener(fl);
	set.addActionListener(this);
	cancel.setActionCommand("cancel");
	cancel.addFocusListener(fl);
	cancel.addActionListener(this);
	getRootPane().setDefaultButton(set);

	Object [] stuff = {b};
	Object [] btns = {set, cancel};

	JOptionPane optionPane = new JOptionPane(stuff, msgType, JOptionPane.YES_NO_OPTION, null, btns, null);
	setContentPane(optionPane);
	setDefaultCloseOperation(JDialog.DO_NOTHING_ON_CLOSE);

	addWindowListener(new WindowAdapter() {
	    public void windowClosing (WindowEvent we) {
	      exit();
	    }
	  });

	pack();
	setLocation(((int)parent.getLocation().getX() + (parent.getSize().width - getBounds().width)/2), ((int)parent.getLocation().getY() + (parent.getSize().height - getBounds().height)/2));
      }

      public void actionPerformed (ActionEvent ae) {
	String cmd = ae.getActionCommand();

	if (cmd.equals("set")) {
	  for (int i = 0; i < ic.length; i++)
	    inputs.add(ic[i].getText());

	  // validation by the user supplies validator
	  if (idr != null) {
	    if (ic.length == 1) {
	      if (!idr.isValidInput(inputs.get(0))) {
		inputs.clear();
		return;
	      }
	    } else {
	      if (!idr.isValidInput(inputs.toArray())) {
		inputs.clear();
		return;
	      }
	    }
	  } else {
	    // any default validations...
	    for (int i = 0; i < ic.length; i++) {
	      if (ic[i] instanceof IPAddressField &&
		  !IPUtil.isValidIP(ic[i].getText())) {
		PopMessage.showError(this, "Invalid IP Address: " + ic[i].getText());
		ic[i].requestFocus();
		inputs.clear();
		return;
	      }
	    }
	  }
	}
	exit();
      }

      private void exit () {
	if (inputs.size() == 0)
	  inputs.add(null);
	setVisible(false);
	dispose();
      }
    }

    if (parent instanceof Frame)
      new InputDialog((Frame)parent, inputComponent, title, message, prompt, borderMessage, idreq, btnStr, msgType).setVisible(true);
    else if (parent instanceof Dialog)
      new InputDialog((Dialog)parent, inputComponent, title, message, prompt, borderMessage, idreq, btnStr, msgType).setVisible(true);
    else
      throw new IllegalArgumentException("parent component must be a Frame or a Dialog");

    if (inputs.size() == 1)
      return inputs.get(0);

    String [] result = new String [inputs.size()];
    return inputs.toArray(result);
  }

}

