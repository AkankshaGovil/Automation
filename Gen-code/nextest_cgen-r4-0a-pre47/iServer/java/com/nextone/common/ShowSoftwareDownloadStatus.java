package com.nextone.common;

import javax.swing.*;
import javax.swing.event.*;
import javax.swing.border.*;
import java.awt.*;
import java.awt.event.*;
import java.io.*;
import java.net.*;
import java.util.*;
import com.nextone.util.FileSaver;
import com.nextone.util.LimitedDataInputStream;
import com.nextone.util.SaveFileListener;
import com.nextone.util.SysUtil;

public class ShowSoftwareDownloadStatus extends JDialog implements FileSaver, Runnable, ActionListener {
	  private JTextArea jt;
	  private DatagramSocket sock;
	  private Thread thisThread;
	  private JButton [] ba;
	  private JScrollBar statusPaneVerticalBar;
	  private JFrame parent;

	  public ShowSoftwareDownloadStatus (JFrame parent, DatagramSocket sock, JButton [] ba) {
		 super(parent, "Software Download Status", false);
		 this.sock = sock;
		 this.parent = parent;
		 this.ba = ba;
		 jt = new JTextArea(8, 15);
		 jt.setLineWrap(true);
		 jt.setEditable(false);
		 JScrollPane jsp = new JScrollPane(jt, JScrollPane.VERTICAL_SCROLLBAR_AS_NEEDED, JScrollPane.HORIZONTAL_SCROLLBAR_NEVER);
		 statusPaneVerticalBar = jsp.getVerticalScrollBar();

		 FocusListener fl = SysUtil.getDefaultButtonFocusListener();
		 JButton set = new JButton("Save To File");
		 set.setActionCommand("set");
		 set.addFocusListener(fl);
		 set.addActionListener(this);
		 final JButton cancel = new JButton("Close");
		 cancel.setActionCommand("cancel");
		 cancel.addFocusListener(fl);
		 cancel.addActionListener(this);
		 getRootPane().setDefaultButton(cancel);

		 Object [] content = {jsp};
		 Object [] btns = {set, cancel};

		 final JOptionPane optionPane = new JOptionPane(content, JOptionPane.PLAIN_MESSAGE, JOptionPane.YES_NO_OPTION, null, btns, null);
		 setContentPane(optionPane);
		 setDefaultCloseOperation(JDialog.DISPOSE_ON_CLOSE);
		 addWindowListener(new WindowAdapter() {
				  public void windowClosing (WindowEvent we) {
					 actionPerformed(new ActionEvent(cancel, 0, cancel.getActionCommand()));
				  }
			});

		 setSize(new Dimension(550, 400));
		 // center this over the parent pane
		 setLocation(((int)parent.getLocation().getX() + (parent.getSize().width - getBounds().width)/2), ((int)parent.getLocation().getY() + (parent.getSize().height - getBounds().height)/2));
		 setVisible(true);
	  }

	  public void actionPerformed (ActionEvent ae) {
		 if (ae.getActionCommand().equals("set")) {
			new SaveFileListener(parent, ShowSoftwareDownloadStatus.this).actionPerformed(new ActionEvent(this, ActionEvent.ACTION_PERFORMED, "save"));
			return;
		 }
		 setVisible(false);
		 stopThisThread();
		 dispose();
	  }

	  private void stopThisThread () {
		 try {
			ByteArrayOutputStream bos = new ByteArrayOutputStream(64);
			DataOutputStream dos = new DataOutputStream(bos);
			dos.writeUTF("----stop");
			DatagramPacket dp = new DatagramPacket(bos.toByteArray(), bos.size(), InetAddress.getLocalHost(), sock.getLocalPort());
			DatagramSocket s = new DatagramSocket();
			s.send(dp);
			s.close();
		 } catch (IOException ie) {}
	  }

    /**
     * Main thread processing.
     * 
     */
	  public void run () {
		 thisThread = Thread.currentThread();

		 byte [] in = new byte [512];
		 while (true) {
			Arrays.fill(in, (byte)0);
			DatagramPacket dpi = new DatagramPacket(in, in.length);
			try {
			   statusPaneVerticalBar.setValue(statusPaneVerticalBar.getMaximum());
			   sock.setSoTimeout(90*1000);  // wait upto 90 seconds to timeout
			   sock.receive(dpi);
			   LimitedDataInputStream dis = new LimitedDataInputStream(new ByteArrayInputStream(in), dpi.getLength());
			   String msg = null;
			   try {
				  msg = dis.readUTF();
			   } catch (EOFException e) {
				  jt.append(">>>>Error reading packet contents<<<<\n");
				  jt.append(e.getMessage() + "\n");
				  jt.append("Packet contents:\n" + new String(dis.readAllBytes()));
				  jt.append("\n");
				  continue;
			   }
			   dis.close();
			   if (msg.startsWith("----")) {
				  // these are internal messages saying no more waiting
				  // for packets
				  break;
			   } else
				  jt.append(msg);
			} catch (InterruptedIOException iioe) {
			   jt.append(">>>>iEdge not responding anymore<<<<\n");
			   break;
			} catch (IOException ie) {
			   jt.append(">>>>Error while receiving status:\n");
			   jt.append(ie.getMessage());
			   break;
			}
		 }
		 statusPaneVerticalBar.setValue(statusPaneVerticalBar.getMaximum());
		 sock.close();
		 for (int i = 0; ba != null && i < ba.length; i++)
			ba[i].setEnabled(true);
	  }

	  // as required by FileSaver interface
	  public void writeData (FileOutputStream fos) throws IOException {
		 String s = jt.getText();
		 fos.write(s.getBytes());
		 fos.flush();
	  }

}

