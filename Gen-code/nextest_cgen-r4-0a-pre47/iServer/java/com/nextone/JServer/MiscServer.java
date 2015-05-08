package com.nextone.JServer;

import java.net.*;
import java.io.*;
import java.util.*;
import com.nextone.util.TCPServer;
import com.nextone.common.ProvisionData;
import com.nextone.common.BridgeException;
import com.nextone.common.CommonConstants;


/**
 * a TCP server which serves Miscellaneous requests for the contacting iview clients 
 */
public class MiscServer extends TCPServer implements CommonConstants{
    protected Object lockObject;

    static {
	System.loadLibrary("BridgeServer");
    }

    private native boolean getRegIdCallRoute (String key,String regId, int port, String destPhone, SendListItem sli) throws BridgeException;
    private native boolean getCallerIdCallRoute (String key,String srcPhone, String destPhone, SendListItem sli) throws BridgeException;
    private native boolean processCommand (int argc, String [] cmds, SendListItem sli) throws BridgeException;


    MiscServer (ThreadGroup tg, boolean debug, Object lockObject) throws IOException {
	super(tg, Constants.JSERVER_MISC_SERVER_PORT, debug);
	this.lockObject = lockObject;
    }

 
    MiscServer (ThreadGroup tg, boolean debug, Object lockObject,InetAddress addr) throws IOException {
	super(tg, Constants.JSERVER_MISC_SERVER_PORT, debug,addr);
	this.lockObject = lockObject;
    }
    // this function will be called from different threads, one for
    // each client connecting to get the information
    public void TCPServerWork (Socket sock) {
	synchronized (lockObject) {
	    JServer.printDebug("client from " + sock.getInetAddress().toString() + " requesting route", JServer.DEBUG_VERBOSE);

	    SendListItem sli = null;

	    try {
		sli = new SendListItem(new ObjectOutputStream(sock.getOutputStream()));
	    } catch (IOException ie) {
		JServer.printDebug("Error getting output stream:", JServer.DEBUG_ERROR);
		JServer.printDebug(ie, JServer.DEBUG_ERROR);
		return;
	    }

	    ObjectInputStream ois = null;
	    try {
		ois = new ObjectInputStream(sock.getInputStream());
	    } catch (IOException ie) {
		JServer.printDebug("Error getting input stream:", JServer.DEBUG_ERROR);
		JServer.printDebug(ie, JServer.DEBUG_ERROR);
		return;
	    }

	    Object o = null;
	    try {
		o = ois.readObject();
		if (!o.getClass().equals(Integer.class)) {
		    JServer.printDebug("Error getting input data. Input should be request type(Integer) " , JServer.DEBUG_ERROR);
		    return;
		}
	    } catch (Exception e) { 
		JServer.printDebug("Error getting input data:", JServer.DEBUG_ERROR);
		JServer.printDebug(e, JServer.DEBUG_ERROR);
		return;
	    }

	    int cmd = ((Integer)o).intValue();
	    boolean result = false;
			
	    switch (cmd) {
	    case CommonConstants.CALL_ROUTE:
		try {
		    o = ois.readObject();
		    if (!o.getClass().equals(Boolean.class)) {
			JServer.printDebug("Error getting input data. Input should be Boolean", JServer.DEBUG_ERROR);
			return;
		    }

		    boolean callerIdFlag = ((Boolean)o).booleanValue();
		    o = ois.readObject();
		    if (!o.getClass().equals(String.class)) {
			JServer.printDebug("Error getting input data. Input should be String", JServer.DEBUG_ERROR);
			return;
		    }

		    String srcPhone = "";
		    String regId = "";
		    int port = 0;
		    if (callerIdFlag)
			srcPhone = (String)o;
		    else {
			regId = (String)o;
			o = ois.readObject();
			if (!o.getClass().equals(Integer.class)) {
			    JServer.printDebug("Error getting input data. Input should be PortNumber(Integer)", JServer.DEBUG_ERROR);
			    return;
			}
			port = ((Integer)o).intValue();
		    }

		    o = ois.readObject();
		    if (!o.getClass().equals(String.class)) {
			JServer.printDebug("Error getting input data. Input should be Phone(String)", JServer.DEBUG_ERROR);
			return;
		    }
		    String destPhone = (String)o;


		    //
		    String key  = ROUTE;
		    try{
			key = (String)ois.readUTF();
			if(!key.equals(ROUTE)  &&
			   !key.equals(HUNT) 
			   )
			    key = ROUTE;
		    }catch(Exception e){}

		    JServer.printDebug("calling old trace route", JServer.DEBUG_ERROR);
		    if (callerIdFlag)
			//result = getCallerIdCallRoute(key,srcPhone, destPhone, sli);
			result = processCommand(4, new String[]{"iedge", key, srcPhone, destPhone}, sli);
		    else
			//result = getRegIdCallRoute(key,regId, port, destPhone, sli);
			result = processCommand(5, new String[]{"iedge", key, regId, Integer.toString(port), destPhone}, sli);
		} catch(Exception e) {
		    JServer.printDebug("Error sending call route:", JServer.DEBUG_ERROR);
		    JServer.printDebug(e, JServer.DEBUG_ERROR);
		    try {
			sli.send(e);
		    } catch (IOException ie) {
			JServer.printDebug("Error in sending call route error status:", JServer.DEBUG_WARNING);
			JServer.printDebug(ie, JServer.DEBUG_WARNING);
		    }
		}
		break;
	    case CommonConstants.CALL_ROUTE_WITH_REALM:
		try {
		    o = ois.readObject();
		    if (!o.getClass().equals(Integer.class)) {
			JServer.printDebug("Error getting input data. Input should be Integer of argument length", JServer.DEBUG_ERROR);
			return;
		    }

		    int argc = ((Integer)o).intValue();
		    String[] argv = new String[argc];
		    for(int i=0; i<argc; i++){
			o = ois.readObject();
			if (!o.getClass().equals(String.class)) {
			    JServer.printDebug("Error getting input data. Input should be String of argument value", JServer.DEBUG_ERROR);
			    return;
			}else
			    argv[i] = (String)o;
		    }

		    JServer.printDebug("calling new trace route", JServer.DEBUG_ERROR);
		    result = processCommand(argc, argv, sli);
		} catch(Exception e) {
		    JServer.printDebug("Error sending call route:", JServer.DEBUG_ERROR);
		    JServer.printDebug(e, JServer.DEBUG_ERROR);
		    try {
			sli.send(e);
		    } catch (IOException ie) {
			JServer.printDebug("Error in sending call route error status:", JServer.DEBUG_WARNING);
			JServer.printDebug(ie, JServer.DEBUG_WARNING);
		    }
		}
		break;
	    }

	    try {
		sli.send(new Boolean(result));
	    } catch (IOException ie) {
		JServer.printDebug("Error sending call route final status:", JServer.DEBUG_ERROR);
		JServer.printDebug(ie, JServer.DEBUG_ERROR);
	    }

	    try {
		sock.shutdownOutput();
	    } catch (IOException ie) {
		JServer.printDebug("Error closing common request socket:", JServer.DEBUG_WARNING);
		JServer.printDebug(ie, JServer.DEBUG_WARNING);
	    }
	}
    }

    // stops this server
    public void stop () {
	super.stopAndWait();
	JServer.printDebug("Miscellaneous Server exited", JServer.DEBUG_NORMAL);
    }

}

