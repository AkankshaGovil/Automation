package com.nextone.util;

import javax.swing.*;
import java.awt.geom.*;
import java.awt.Dimension;

public class ProgressAnimation extends StatusAnimation{
  private JProgressBar progressBar;  
  private int maxRecords;
  int i=0;
  
  public ProgressAnimation(int maxRecords){
    this.maxRecords  = maxRecords;
		progressBar = new JProgressBar(JProgressBar.HORIZONTAL,0,maxRecords);    
		progressBar.setValue(0);
		progressBar.setStringPainted(true);
    add(progressBar);
    progressBar.setPreferredSize(new Dimension(350,20));
    progressBar.setVisible(true);

  }

  public void update(int n){
		progressBar.setValue(n);
		int val = (int)Math.floor(((((double)n))/((double)maxRecords))*100);
		if (val > 0)
			val--;  // never show 100% completed
		progressBar.setString(val + "%");
  }

  public void cleanup(){
    progressBar = null;
  }

}
