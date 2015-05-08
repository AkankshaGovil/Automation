package com.nextone.common;

import java.io.*;
import java.net.*;
import javax.xml.parsers.*;
import org.w3c.dom.*;
import org.xml.sax.*;


/**
 * this class handles the redundancy information passed between iView and iServer
 */
public class RedundInfo {
  private InetAddress vip, db, fromAddr;
  private boolean drEnabled, vipEnabled, isMaster, vipActive;

  private static DocumentBuilder docb = null;
  private static Document doc = null;

  static {
    try {
      docb = DocumentBuilderFactory.newInstance().newDocumentBuilder();
      doc = docb.newDocument();
    } catch (Exception e) {
      e.printStackTrace();
    }
  }


  public RedundInfo (boolean isDbRedundEnabled, InetAddress dbMaster, boolean isDbMaster, boolean isVipRedundEnabled, InetAddress vip, boolean vipActive, InetAddress primaryIfAddr) {
    drEnabled = isDbRedundEnabled;
    db = dbMaster;
    isMaster = isDbMaster;
    vipEnabled = isVipRedundEnabled;
    this.vip = vip;
    this.vipActive = vipActive;
    fromAddr = primaryIfAddr;
  }

  public RedundInfo (String xmlString) throws IOException, SAXException, DOMException {
    this(docb.parse(new InputSource(new StringReader(xmlString))).getDocumentElement());
  }

  public RedundInfo (Element element) throws DOMException {
    NodeList list = element.getElementsByTagName("network");
    if (list != null && list.getLength() == 1)
      readNetworkElement((Element)list.item(0));
    list = element.getElementsByTagName("database");
    if (list != null && list.getLength() == 1)
      readDatabaseElement((Element)list.item(0));
  }

  private void readNetworkElement (Element element) throws DOMException {
    vipEnabled = "enabled".equals(element.getAttribute("status"));

    NodeList list = element.getElementsByTagName("vip");
    if (list != null && list.getLength() == 1) {
      Element childElem = (Element)list.item(0);
      vipActive = "true".equals(childElem.getAttribute("self"));
      try {
	vip = null;
	String addrStr = childElem.getLastChild().getNodeValue();
	if (addrStr != null && addrStr.length() > 0)
	  vip = InetAddress.getByName(addrStr);
      } catch (Exception e) {}
    }

    fromAddr = null;
    list = element.getElementsByTagName("fromAddress");
    if (list != null && list.getLength() == 1) {
      Element childElem = (Element)list.item(0);
      try {
	String addrStr = childElem.getLastChild().getNodeValue();
	if (addrStr != null && addrStr.length() > 0)
	  fromAddr = InetAddress.getByName(addrStr);
      } catch (Exception e) {}
    }
  }

  private void readDatabaseElement (Element element) throws DOMException {
    drEnabled = "enabled".equals(element.getAttribute("status"));

    NodeList list = element.getElementsByTagName("master");
    if (list != null && list.getLength() == 1) {
      Element childElem = (Element)list.item(0);
      isMaster = "true".equals(childElem.getAttribute("self"));
      try {
	db = null;
	String addrStr = childElem.getLastChild().getNodeValue();
	if (addrStr != null && addrStr.length() > 0)
	  db = InetAddress.getByName(addrStr);
      } catch (Exception e) {}
    }
  }

  private Element createNetworkElement () throws DOMException {
    Element root = doc.createElement("network");
    root.setAttribute("status", vipEnabled?"enabled":"disabled");

    Element tag = doc.createElement("vip");
    tag.setAttribute("self", vipActive?"true":"false");
    Text data = doc.createTextNode((vip == null)?"":vip.getHostAddress());
    tag.appendChild(data);
    root.appendChild(tag);

    if (fromAddr != null) {
      tag = doc.createElement("fromAddress");
      tag.appendChild(doc.createTextNode(fromAddr.getHostAddress()));
      root.appendChild(tag);
    }

    return root;
  }

  private Element createDatabaseElement () throws DOMException {
    Element root = doc.createElement("database");
    root.setAttribute("status", drEnabled?"enabled":"disabled");

    Element tag = doc.createElement("master");
    tag.setAttribute("self", isMaster?"true":"false");
    Text data = doc.createTextNode((db == null)?"":db.getHostAddress());
    tag.appendChild(data);

    root.appendChild(tag);

    return root;
  }

  public String toString () {
    try {
      Element root = doc.createElement("redundInfo");

      root.appendChild(createNetworkElement());
      root.appendChild(createDatabaseElement());

      return root.toString();
    } catch (Exception e) {
      e.printStackTrace();
    }
    return "";
  }

  public synchronized InetAddress getVip () {
    return vip;
  }

  public synchronized void setVip (InetAddress addr) {
    vip = addr;
  }

  public synchronized InetAddress getDatabaseAddress () {
    return db;
  }

  public synchronized void setDatabaseAddress (InetAddress addr) {
    db = addr;
  }

  public synchronized boolean isNetworkRedundEnabled () {
    return vipEnabled;
  }

  public synchronized void setNetworkRedundEnabled (boolean value) {
    vipEnabled = value;
  }

  public synchronized boolean isDatabaseRedundEnabled () {
    return drEnabled;
  }

  public synchronized void setDatabaseRedundEnabled (boolean value) {
    drEnabled = value;
  }

  public synchronized boolean isDatabaseMaster () {
    return isMaster;
  }

  public synchronized void setDatabaseMaster (boolean value) {
    isMaster = value;
  }

  public synchronized boolean isVipActive () {
    return vipActive;
  }

  public synchronized void setVipActive (boolean value) {
    vipActive = value;
  }

  public synchronized InetAddress getFromAddress () {
    return fromAddr;
  }

  public synchronized void setFromAddress (InetAddress addr) {
    fromAddr = addr;
  }

}
