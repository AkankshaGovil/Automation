package com.nextone.common;

import java.io.*;
import java.util.*;
	
public class BoardData{
	short	portStatus;
	short	portType;
	short	rollType;
	short	signallingMode;
	String	externalNumber;
	String	faxServer;
	String	h323Id;
	String	email;
	String	rollNumber;
	String  hbridType;


	public BoardData(){

		portStatus	=	0;
		portType	=	0;
		rollType	=	0;
		signallingMode	=	0;
		externalNumber	=	"";
		faxServer	=	"";
		h323Id	=	"";
		email	=	"";
		rollNumber	=	"";
		hbridType	=	"";
	}

	public void dump () {
		System.out.println("portStatus: " + portStatus );
		System.out.println("portType: " + portType );
		System.out.println("rollType: " + rollType );
		System.out.println("signallingMode: " + signallingMode );
		System.out.println("externalNumber: " + externalNumber );
		System.out.println("faxServer: " + faxServer );
		System.out.println("h323Id: " + h323Id );
		System.out.println("email: " + email );
		System.out.println("rollNumber: " + rollNumber );
		System.out.println("hbridType: " + hbridType );
	}


	public void setPortStatus(short status){
		portStatus	=	status;
	}


	public short getPortStatus(){
		return portStatus;
	}

	public void setPortType(short type){
		portType	=	type;
	}

	public short getPortType(){
		return portType;
	}

	public void setRollType(short type){
		rollType	=	type;
	}

	public short getRollType(){
		return rollType;
	}

	public void setRollNumber(String s){
		rollNumber	=	(s	==	null)?"":s;
	}

	public String getRollNumber(){
		return rollNumber;
	}


	public void setExternalNumber(String s){
		externalNumber	=	(s	==	null)?"":s;
	}

	public String getExternalNumber(){
		return externalNumber;
	}


	public void setEmail(String s){
		email	=	(s	==	null)?"":s;
	}

	public String getEmail(){
		return email;
	}

	public void setFaxServer(String s){
		faxServer	=	(s	==	null)? "": s;
	}

	public String getFaxServer(){
		return faxServer;
	}

	public void setH323Id(String s){
		h323Id	=	(s	==	null)?"":s;
	}

	public String getH323Id(){
		return 	h323Id;
	}

	public void setHybridType(String s){
		hbridType	=	(s	==	null)?"":s;
	}

	public String getHybridType(){
		return 	hbridType;
	}

	public void setSignallingMode(short s){
		signallingMode	=	s;
	}

	public short getSignallingMode(){
		return 	signallingMode;
	}


	public void setValue(int index, Object val){
    //  deprecated since 2.0.5. use the methods directly to get the value
/*
		switch(index){
			case CommonConstants.PORT_STATUS:
				setPortStatus( ((Short)val).shortValue());
				break;
			case CommonConstants.PORT_TYPE:
				setPortType (  ((Short)val).shortValue());
				break;
			case CommonConstants.ROLL_TYPE:
				setRollType( ( (Short)val).shortValue());
				break;
			case CommonConstants.ROLL_NUMBER:
				setRollNumber( (String)val);
				break;

			case CommonConstants.H323_ID:
				setH323Id( (String) val);
				break;

			case CommonConstants.EXT_GK_NUMBER:
				setExternalNumber( (String) val);
				break;
			case CommonConstants.EMAIL:
				setEmail( (String) val);
				break;
			case CommonConstants.HYBRID_TYPE:
				setHybridType( (String) val);
				break;

			case CommonConstants.SIGNALLING_MODE:
				setSignallingMode( (( Short)val).shortValue());
				break;


		}
    */
	}

	public Object getValue(int index){
    //  deprecated since 2.0.5. use the methods directly to set the value
/*
		switch(index){

			case CommonConstants.PORT_STATUS:
				return (new Short(getPortStatus()) );

			case CommonConstants.PORT_TYPE:
				return (new Short( getPortType() ));

			case CommonConstants.ROLL_TYPE:
				return (new Short( getRollType() ));

			case CommonConstants.ROLL_NUMBER:
				return getRollNumber();

			case CommonConstants.H323_ID:
				return getH323Id();

			case CommonConstants.EXT_GK_NUMBER:
				return getExternalNumber( );

			case CommonConstants.EMAIL:
				return getEmail();

			case CommonConstants.HYBRID_TYPE:
				return getHybridType();

			case CommonConstants.SIGNALLING_MODE:
				return (new Short( getSignallingMode()) );

		}
*/
		return "";
	}
  
	public void	copyData(BoardData b){

		setPortStatus( b.getPortStatus());
		setPortType( b.getPortType());
		setRollType( b.getRollType());
		setRollNumber( b.getRollNumber());
		setH323Id( b.getH323Id());
		setExternalNumber( b.getExternalNumber());
		setEmail( b.getEmail());
		setHybridType( b.getHybridType());
		setSignallingMode( b.getSignallingMode());

	}


}
