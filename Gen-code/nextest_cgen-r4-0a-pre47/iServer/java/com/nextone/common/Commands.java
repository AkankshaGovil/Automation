/*
 * @(#)Commands.java	1.9 05/14/02
 */

package com.nextone.common;
import java.io.Serializable;
import java.util.Set;

import com.nextone.util.LinkedHashMap;

/**
 * Implementation of cli commands.It defines the commands insertion and 
 * access of commands in the order they were inserted
 **/

public class Commands implements Serializable{
  public static final String ADD    = "add";
  public static final String EDIT   = "edit";
  public static final String DELETE = "delete";
  public static final String PURGE  = "purge";

  private LinkedHashMap subCommands;
  private int maxSubCommands;
  private int currentSubCommand;
  private int currentMapPos;
  private String  command;
  private String  primaryKey;
  private String  secondaryKey;

  // need this, otherwise there will be class incompatible exceptions
  static final long serialVersionUID = -5509906583490670621L;

  public  Commands(String cmd){
    this(cmd,null,null);
  }

  public Commands(String cmd, String pKey, String sKey){
    this.command      = cmd;
    this.primaryKey   = pKey;
    this.secondaryKey = sKey;
    maxSubCommands = currentSubCommand  = currentMapPos = 0;
    subCommands = new LinkedHashMap();
  }

  public void setCommand(String cmd){
    command = cmd;
  }

  public String getCommand(){
    return command;
  }
  
  public void setKeys(String pKey, String sKey){
    primaryKey    = pKey;
    secondaryKey  = sKey;
  }

  public void setPrimaryKey(String key){
    primaryKey  = key;
  }

  public String getPrimaryKey(){
    return primaryKey;
  }

  public void setSecondaryKey(String key){
    secondaryKey  = key;
  }

  public String getSecondaryKey(){
    return secondaryKey;
  }

  public void addSubCommand(String cmd){
      addSubCommand(cmd,(String[])null);
  }

  public void addSubCommand(String cmd, String val){
    String[]  vals  = new String[1];
    vals[0] = val;
    addSubCommand(cmd,vals);
  }

  public void addSubCommand(String cmd, String[] vals){
    subCommands.put(cmd,vals);
    maxSubCommands++;
  }

  public void addSubCommand(String cmd, LinkedHashMap vals){
    subCommands.put(cmd,vals);
    maxSubCommands++;
  }

  public boolean hasMoreCommands(){
    return(currentSubCommand <  maxSubCommands);
  }

  public int size(){
    return maxSubCommands;
  }

  public boolean isDeleteCommand(){
    return subCommands.containsKey(DELETE);
  }
  public String[] nextCommand(){
    Set set = subCommands.keySet();
    String[]  temp  = new String[set.size()];
    String[] keys  = (String[])(set.toArray(temp));

    Object  o = subCommands.get(keys[currentSubCommand]);
    String  key = keys[currentSubCommand];
    String[]  vals  = null;


    if(o  !=  null){
      if(o instanceof String[]){
        currentMapPos = 0;
        vals  = (String[])o;
        currentSubCommand++;
      }else if(o instanceof LinkedHashMap){
        LinkedHashMap map = (LinkedHashMap)o;
        set = map.keySet();
        temp   = new String[set.size()];
        String[] subKeys  = (String[])set.toArray(temp);
        vals  = new String[4];
        vals[0] = "/";
        vals[1] = subKeys[currentMapPos];
        vals[2] = (String)map.get(vals[1]);
        vals[3] = "/";
        currentMapPos++;
        if(currentMapPos >= map.size()){
          currentMapPos = 0;
          currentSubCommand++;
        }
      }else
        currentSubCommand = maxSubCommands;
    }else
      currentSubCommand++;
    return makeCommand(key,vals);

  }

  private String[] makeCommand(String subCmd, String[]  val){
      int size    =0;

      if  (val !=  null)
        size  = val.length;
      //  for command
      size++;
      //  for subcommand
      size++;
      if(secondaryKey !=  null)
        size++;
      if(primaryKey !=  null)
        size++;

      String[]  cmd = new String[size];
      int i=  0;
      cmd[i++] = command;
      cmd[i++] = subCmd;
      if(primaryKey !=  null)
        cmd[i++] = primaryKey;
      if(secondaryKey !=  null)
        cmd[i++] = secondaryKey;
      if(val  !=  null){
        for(int j=0 ; j < val.length; j++){
          cmd[i+j]  = val[j];
        }
      }
      return cmd;
  }

  public String[] deleteCommand(){
    Object  o = subCommands.get(DELETE);
    int size =0;

    String[]  vals  = null;
    if(o  !=  null  &&  o instanceof String[]){
      vals  = (String[])o;
    }
    return makeCommand(DELETE,vals);
  }

  public boolean isAddCommand(){
    return subCommands.containsKey(ADD);
  }

  public String toString(){
    StringBuffer result = new StringBuffer();
    result.append("command ="+ command);
    result.append("\npKey ="+ primaryKey);
    result.append("\nsKey ="+ secondaryKey);
    result.append(subCommands.toString());
    return result.toString();

  }

}
