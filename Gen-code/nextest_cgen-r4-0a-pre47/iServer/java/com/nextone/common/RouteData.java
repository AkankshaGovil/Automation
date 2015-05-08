package com.nextone.common;

import java.io.*;
import java.util.*;
import com.nextone.util.IPUtil;

/**
 * Class which contains the call route information.
 * This class is passed  between the iServer and iView. 
 */


public class RouteData implements CommonConstants, Serializable{
  private int		branch;
  private short		srcFlag;
  private short		destFlag;
  private String  srcNode;
  private String  destNode;
  private String  forwardReason;
  private String  rejectReason;

  private String details;

  static final long serialVersionUID = -2164964791230901061L;

  public RouteData () {
    branch		=	0;
    forwardReason	=	"";
    rejectReason	=	"";
    srcNode    	=	"";
    destNode	=	"";
    srcFlag     =	0;
    destFlag	=	0;

  }


  public RouteData(int branch,short srcFlag,short destFlag,String srcNode,String destNode,String forwardReason,String rejectReason){
    this.branch		=	branch;
    this.forwardReason	=	(forwardReason	==	null)?"":forwardReason;
    this.rejectReason	=	(rejectReason	==	null)?"":rejectReason;
    this.srcNode	=	(srcNode	==	null)?"":srcNode;
    this.destNode      	=	(destNode	==	null)?"":destNode;
  }



  public RouteData(String details){
    this.details  = details;
  }
  public String toString () {
    return getRoute();
//    return details;
  }

  public String getRoute(){
	if (details != null  && details.length() > 0)
    		return details;

    StringBuffer	sb	=	new StringBuffer();

    switch(branch){
    case	1:	sb.append("(b2): ");
      break;
    case	0:	sb.append("(b1): ");
      break;
    default:
      break;
    }
    sb.append(srcNode+ "\n");

    if(forwardReason.length()	!=	0)
      sb.append(forwardReason);

    if(forwardReason.length()	==	0	&& rejectReason.length()	==	0)
      sb.append("\tfinal");

    if(rejectReason.length()	!=	0)
      sb.append(rejectReason);

    sb.append("->");
    sb.append(destNode+ "\n");

    return sb.toString();

  }


}
