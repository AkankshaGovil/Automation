package com.nextone.common;

import java.util.*;
import java.io.*;
import java.net.*;

/**
* A general purpose class to receive any list of data from the iserver
* over a TCP connection. It is assumed that the data received are a
* bunch of classes, terminated by a Boolean class. This class
* does not take care of receiving primitive types
*/
public class GetListData {
  private int port, list;
  private InetAddress addr;
  private String message, details;
  private boolean status;
  private HashMap allLists;
  private ListDataRequester ldr;
  private Vector myList;
  private RouteData route;
  private boolean keepRunning;
  private int recordCount;

  private int timeout;

  /**
   * @param port the port to contact to get the data
   * @param addr the address of the iserver
   * @param listType the type of list being requested (this is used to 
   * validate the classes being received)
   * @param ldr the caller who started this thread (used to indicate to the
   * caller that the data retrieval is done)
   */
  public GetListData (int port, InetAddress addr, int listType,    ListDataRequester ldr) {
    this(port,addr,listType);
    this.ldr = ldr;
    timeout = ldr.getGetTimeout()*1000;
  }

  public GetListData (int port, InetAddress addr, int listType) {
    this.port = port;
    this.addr = addr;
    this.list = listType;
    this.allLists = new HashMap();
    this.myList	= new Vector();
    this.recordCount    =   0;
  }

  /**
   * return the list type that this object is currently chartered to list
   */
  public int getListType () {
    return this.list;
  }

  public void setListType (int listType) {
    this.list = listType;
  }

  public int getPort () {
    return this.port;
  }

  public void setPort (int port) {
    this.port = port;
  }

 /**
  * method which receives all the information
  */
 public void retrieve () {
   StringBuffer msg = new StringBuffer();
   StringBuffer det = new StringBuffer();
    keepRunning = true;
   try {
     recordCount = 0;
     Socket s = new Socket(addr, port);
     ObjectInputStream ois = new ObjectInputStream(s.getInputStream());
     ObjectOutputStream oos = new ObjectOutputStream(s.getOutputStream());
		
     //	Send the list id
     oos.writeObject(new Integer(CommonConstants.LIST_ALL));
     //  Send dummy filter
     oos.writeObject(new String(""));
     oos.flush();
     s.setSoTimeout(timeout);
     loop:
      while (keepRunning) {
        	Object o = null;
        	try {
	          o = ois.readObject();
        	} catch (InterruptedIOException ie) {
	          msg.append("iServer not responding anymore\n");
	          det.append(ie.toString() + "\n");
	          status = false;
	          break loop;
        	} catch (ClassNotFoundException cnfe) {
	          msg.append("Software mismatch between the iView and the iServer\n");
	          det.append(cnfe.toString() + "\n");
	          status = false;
	          break loop;
        	}


        	if (o instanceof Boolean) {
	          // status message received
	          status = ((Boolean)o).booleanValue();
	          break loop;
        	} else if (o instanceof Exception) {
	          // some exception received from the server
	          msg.append("Error happened on iServer while listing\n");
	          det.append(((Exception)o).toString() + "\n");
        	} else{
	          Class c = o.getClass();

            if (isClassValidForList(c)) {
              Vector v = (Vector)allLists.get(c);
              if (v == null) {
	              v = new Vector();
	              allLists.put(c, v);
              }
              v.add(o);
            } else {

              msg.append("Software version mismatch between the iView and the iServer\n");
              det.append(o.getClass().getName() + ": " + o.toString() + "\n");
              det.append("Unexpected class (" + o.getClass().getName() + ") received\n");
              status = false;
              break loop;
            }

          }
          recordCount++;
      }
      try {
	      s.close();
      } catch (Exception e) {}
    } catch (IOException ie) {
      if (ie instanceof InvalidClassException) {
	      msg.append("Software version mismatch between the iView and the iServer\n");
      } else {
	      msg.append("Error requesting information from the iServer\n");
      }
      det.append(ie.toString() + "\n");
      status = false;
    }

    if (status == false) {
      msg.insert(0, "List maybe incomplete:\n\n");
    }
    message = msg.toString();
    details = det.toString();

    // tell the caller that we are done
    if(keepRunning )
      ldr.listDone(this);  // tell him only if did not cancel us
  }


  /**
   * method which receives particular list of information
   * @param int listId	list id to recieve the corresponding list
   */
  public void retrieve (int listId) {
    StringBuffer msg = new StringBuffer();
    StringBuffer det = new StringBuffer();

    keepRunning = true;
    try {
      recordCount = 0;
      Socket s = new Socket(addr, port);

      ObjectInputStream ois = new ObjectInputStream(s.getInputStream());
      ObjectOutputStream oos = new ObjectOutputStream(s.getOutputStream());
			
      // Send the list id
      oos.writeObject(new Integer(listId));
      // Send dummy filter
      oos.writeObject(new String(""));
      oos.flush();

      s.setSoTimeout(timeout);
      loop:
      while (keepRunning) {
	      Object o = null;
	      try {
	        o = ois.readObject();
	      } catch (InterruptedIOException ie) {
	        msg.append("iServer not responding anymore\n");
	        det.append(ie.toString() + "\n");
	        status = false;
	        break loop;
	      } catch (ClassNotFoundException cnfe) {
	        msg.append("Software mismatch between the iView and the iServer\n");
	        det.append(cnfe.toString() + "\n");
	        status = false;
	        break loop;
	      }

	      if (o instanceof Boolean) {
	        // status message received
	        status = ((Boolean)o).booleanValue();
	        break loop;
	      } else if (o instanceof Exception) {
	        // some exception received from the server
	        msg.append("Error happened on iServer while listing\n");
	        det.append(((Exception)o).toString() + "\n");
	      } else {
	        Class c = o.getClass();

	        if (isClassValidForList(c, listId)) {
	          myList.add(o);
	        } else {
	          msg.append("Software version mismatch between the iView and the iServer\n");
	          det.append(o.getClass().getName() + ": " + o.toString() + "\n");
	          det.append("Unexpected class (" + o.getClass().getName() + ") received\n");
	          status = false;
	          break loop;
	        }
	      }

        recordCount++;
      }

      try {
	s.close();
      } catch (Exception e) {}
    } catch (IOException ie) {
      if (ie instanceof InvalidClassException) {
	      msg.append("Software version mismatch between the iView and the iServer\n");
      } else {
	      msg.append("Error requesting information from the iServer\n");
      }
      det.append(ie.toString() + "\n");
      status = false;
    }

    if (status == false) {
      msg.insert(0, "List maybe incomplete:\n\n");
    }
    message = msg.toString();
    details = det.toString();
    // tell the caller that we are done
    if (keepRunning)
      ldr.listDone(this);   // tell him only if did not cancel us
  }




  /**
   * method which receives filtered information
   * @param int dbName	id to recieve the corresponding list
   * @param String Filter	filter's the list
   */
  public void retrieve (int dbName, String filter) {
    StringBuffer msg = new StringBuffer();
    StringBuffer det = new StringBuffer();

    keepRunning = true;
    try {
      recordCount = 0;
      Socket s = new Socket(addr, port);

      ObjectInputStream ois = new ObjectInputStream(s.getInputStream());
      ObjectOutputStream oos = new ObjectOutputStream(s.getOutputStream());
			
      // Send the id
      oos.writeObject(new Integer(dbName));
      oos.writeObject(filter);
      oos.flush();

      s.setSoTimeout(timeout);
      loop:
      while (keepRunning) {
	      Object o = null;
	      try {
	        o = ois.readObject();
	      } catch (InterruptedIOException ie) {
	        msg.append("iServer not responding anymore\n");
	        det.append(ie.toString() + "\n");
	        status = false;
	        break loop;
	      } catch (ClassNotFoundException cnfe) {
	        msg.append("Software mismatch between the iView and the iServer\n");
	        det.append(cnfe.toString() + "\n");
	        status = false;
	        break loop;
	      }

	      if (o instanceof Boolean) {
	        // status message received
	        status = ((Boolean)o).booleanValue();
	        break loop;
	      } else if (o instanceof Exception) {
	        // some exception received from the server
	        msg.append("Error happened on iServer while listing\n");
	        det.append(((Exception)o).toString() + "\n");
	      } else {
	        Class c = o.getClass();

	        if (isClassValidForList(c, dbName)) {
	          myList.add(o);
	        } else {
	          msg.append("Software version mismatch between the iView and the iServer\n");
	          det.append(o.getClass().getName() + ": " + o.toString() + "\n");
	          det.append("Unexpected class (" + o.getClass().getName() + ") received\n");
	          status = false;
	          break loop;
	        }
	      }

        recordCount++;
      }

      try {
	s.close();
      } catch (Exception e) {}
    } catch (IOException ie) {
      if (ie instanceof InvalidClassException) {
	msg.append("Software version mismatch between the iView and the iServer\n");
      } else {
	msg.append("Error requesting information from the iServer\n");
      }
      det.append(ie.toString() + "\n");
      status = false;
    }

    if (status == false) {
      msg.insert(0, "List maybe incomplete:\n\n");
    }
    message = msg.toString();
    details = det.toString();
    // tell the caller that we are done
    if (keepRunning)
      ldr.listDone(this);   // tell him only if did not cancel us
  }   


  /**
   * method which receives call route
   */
  public void ProcessCallRoute (String key,boolean callerIdFlag, String srcPhone, String registrationId, int srcPort, String destPhone) {
    StringBuffer msg = new StringBuffer();
    StringBuffer det = new StringBuffer();

    keepRunning = true;
    try {
      Socket s = new Socket(addr, port);

      s.setSoTimeout(0);

      ObjectInputStream ois = new ObjectInputStream(s.getInputStream());
      ObjectOutputStream oos = new ObjectOutputStream(s.getOutputStream());
			
      //	Send the misc command

      oos.writeObject(new Integer(CommonConstants.CALL_ROUTE));
      oos.writeObject(new Boolean(callerIdFlag));
      if(callerIdFlag){
	      oos.writeObject(srcPhone);
      }
      else{
	      oos.writeObject(registrationId);
	      oos.writeObject(new Integer(srcPort));
      }
      oos.writeObject(destPhone);
      oos.writeUTF(key);
      oos.flush();

      recordCount = 0;
    loop:

      while (keepRunning) {
	      route	=	null;
	      Object o = null;
	      try {
	        o = ois.readObject();
	      } catch (InterruptedIOException ie) {
	        msg.append("iServer not responding anymore\n");
	        det.append(ie.toString() + "\n");
	        status = false;
	        break loop;
	      } catch (ClassNotFoundException cnfe) {
	        msg.append("Software mismatch between the iView and the iServer\n");
	        det.append(cnfe.toString() + "\n");
	        status = false;
	        break loop;
	      }

	      if (o instanceof Boolean) {
	        // status message received
	        status = ((Boolean)o).booleanValue();
	        break loop;
	      } else if (o instanceof Exception) {
	        // some exception received from the server
	        msg.append("Error happened on iServer while getting call route\n");
	        det.append(((Exception)o).toString() + "\n");
		      status = false;
	      } else if (o instanceof RouteData) {
	        route	=	(RouteData)o;
	        ldr.listDone(this);
	      } else {
	        msg.append("Software version mismatch between the iView and the iServer\n");
	        det.append(o.getClass().getName() + ": " + o.toString() + "\n");
	        det.append("Unexpected class (" + o.getClass().getName() + ") received\n");
	        status = false;
	        break loop;
	      }
        recordCount++;
      }

      try {
	s.close();
      } catch (Exception e) {}
    } catch (IOException ie) {
      if (ie instanceof InvalidClassException) {
	      msg.append("Software version mismatch between the iView and the iServer\n");
      } else {
	      msg.append("Error requesting information from the iServer\n");
      }
      det.append(ie.toString() + "\n");
      status = false;
    }

    message = msg.toString();
    details = det.toString();
    // tell the caller that we are done
    route	=	null;
    if (keepRunning)
      ldr.listDone(this);   // tell him only if did not cancel us
  }


  /**
   * method which receives call route
   */
  public void ProcessCallRouteWithRealm(String key, Vector args) {
    StringBuffer msg = new StringBuffer();
    StringBuffer det = new StringBuffer();

    keepRunning = true;
    try {
      Socket s = new Socket(addr, port);

      s.setSoTimeout(0);

      ObjectInputStream ois = new ObjectInputStream(s.getInputStream());
      ObjectOutputStream oos = new ObjectOutputStream(s.getOutputStream());
			
      //	Send the misc command
      oos.writeObject(new Integer(CommonConstants.CALL_ROUTE_WITH_REALM));
      int argc = args.size();
      oos.writeObject(new Integer(argc+2));
      
      oos.writeObject("iedge");
      oos.writeObject(key);
      for(int i=0; i<argc; i++){
	  oos.writeObject((String)args.get(i));
      }
      oos.flush();

      recordCount = 0;
      
    loop:

      while (keepRunning) {
	      route	=	null;
	      Object o = null;
	      try {
	        o = ois.readObject();
	      } catch (InterruptedIOException ie) {
	        msg.append("iServer not responding anymore\n");
	        det.append(ie.toString() + "\n");
	        status = false;
	        break loop;
	      } catch (ClassNotFoundException cnfe) {
	        msg.append("Software mismatch between the iView and the iServer\n");
	        det.append(cnfe.toString() + "\n");
	        status = false;
	        break loop;
	      }

	      if (o instanceof Boolean) {
	        // status message received
	        status = ((Boolean)o).booleanValue();
	        break loop;
	      } else if (o instanceof Exception) {
	        // some exception received from the server
	        msg.append("Error happened on iServer while getting call route\n");
	        det.append(((Exception)o).toString() + "\n");
		      status = false;
	      } else if (o instanceof RouteData) {
	        route	=	(RouteData)o;
	        ldr.listDone(this);
	      } else {
	        msg.append("Software version mismatch between the iView and the iServer\n");
	        det.append(o.getClass().getName() + ": " + o.toString() + "\n");
	        det.append("Unexpected class (" + o.getClass().getName() + ") received\n");
	        status = false;
	        break loop;
	      }
        recordCount++;
      }

      try {
	s.close();
      } catch (Exception e) {}
    } catch (IOException ie) {
      if (ie instanceof InvalidClassException) {
	      msg.append("Software version mismatch between the iView and the iServer\n");
      } else {
	      msg.append("Error requesting information from the iServer\n");
      }
      det.append(ie.toString() + "\n");
      status = false;
    }

    message = msg.toString();
    details = det.toString();
    // tell the caller that we are done
    route	=	null;
    if (keepRunning)
      ldr.listDone(this);   // tell him only if did not cancel us
  }


  /**
   * returns the status of the last list operation
   */
  public boolean getStatus () {
    return status;
  }

  /**
   * returns any error messages during the listing
   */
  public String getMessage () {
    return message;
  }

  /**
   * returns the details of any error messages during listing
   */
  public String getDetails () {
    return details;
  }

  /**
   * returns the vector containing the list of objects of the requested
   * class type
   * @param classname the classname of objects to return
   * @return null if no objects of that classname are received
   */
  public Vector getList (Class classname) {
    return (Vector)allLists.get(classname);
  }

  // validates if we are receiving any object we are not supposed to	
  // receive
  private boolean isClassValidForList (Class c) {
    switch (list) {
    case CommonConstants.IEDGE_LIST:
      if (c == VpnGroupList.class ||
	  c == VpnList.class ||
	  c == IEdgeList.class)
	return true;
      break;

    case CommonConstants.CALLING_PLAN_LIST:
      if (c == CallPlan.class ||
	  c == CallPlanBinding.class ||
	  c == CallPlanRoute.class)
	return true;
      break;
    }

    return false;
  }


  // validates if we are receiving any object we are not supposed to	
  // receive
  private boolean isClassValidForList (Class c, int listId) {
    switch (listId) {
    case CommonConstants.IEDGE_ONLY:
      if (c == IEdgeList.class)
	return true;
      break;

    case CommonConstants.IEDGE_VPN_ONLY:
      if (c == VpnList.class )
	return true;
      break;

    case CommonConstants.IEDGE_GROUP_ONLY:
      if (c == VpnGroupList.class )
	return true;
      break;

    case CommonConstants.CALLING_PLAN_BIND_ONLY:
      if (c == CallPlanBinding.class )
	return true;
      break;

    case CommonConstants.CALLING_PLAN_ONLY:
      if (c == CallPlan.class ||
	  c == CallPlanBinding.class)
	return true;
      break;
    }

    return false;
  }

 		
  /**
   * returns the vector containing the list of objects of the requested
   * class type
   * @return null if no objects of that classname are received
   */
  public Vector getList () {
    return myList;
  }

  public RouteData getRouteData(){
    return route;
  }

  public int getRecordCount(){
      return recordCount;
  }

  public void cancelTask(){
    keepRunning = false;
  }

	
  /**
   * the callback to indicate that the listing opration is done
   */
  public interface ListDataRequester extends TimeoutProvider {
    public void listDone (GetListData gld);
  }

}


