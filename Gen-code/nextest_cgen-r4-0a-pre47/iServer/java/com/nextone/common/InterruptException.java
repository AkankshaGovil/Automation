package com.nextone.common;
import org.xml.sax.SAXException;
/**
 * this is the general exception that is thrown for any errors happening
 * in the communication between the iserver and iview
 */
public  class InterruptException  extends SAXException{

    public InterruptException(){
      this("");
    }

    public  InterruptException(String msg){
      super(msg);
    }
}


