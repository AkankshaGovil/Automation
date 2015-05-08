package com.nextone.common;

import java.io.*;
import java.util.*;

/**
 * This class is used to store the maintenenace groups
 */
public class MaintenanceGroup implements Serializable {
	  private String name;
	  private String comment;
	  private VpnGroupList [] vpnGroups;
	  private VpnList [] vpnNames;
	  private IEdgeCore [] iEdgeCores;
	  private Date creationDate;
	  private Date modificationDate;

	  static final long serialVersionUID = -2426871697896260071L;

	  public MaintenanceGroup () {
		 this("<new>", "", null, null);
	  }

	  public MaintenanceGroup (String name, String comment, Date cd, Date md) {
		 this.name = name;
		 this.comment = comment;
		 Date d = new Date();
		 creationDate = (cd == null)?d:cd;
		 modificationDate = (md == null)?d:md;
	  }

	  public void dump () {
		 System.out.println("Name: " + name);
		 System.out.println("Comment: " + comment);
		 System.out.println("Groups:");
		 for (int i = 0; vpnGroups != null && i < vpnGroups.length; i++)
			System.out.println("    " + vpnGroups[i].getVpnGroup());
		 System.out.println("Ids:");
		 for (int i = 0; vpnNames != null && i < vpnNames.length; i++)
			System.out.println("    " + vpnNames[i].getVpnName() + "/" + vpnNames[i].getExtLen());

		 System.out.println("iEdges:");
		 for (int i = 0; iEdgeCores != null && i < iEdgeCores.length; i++)
			System.out.println("    " + iEdgeCores[i].getRegId() + ":" + iEdgeCores[i].getDeviceType());
	  }

	  public synchronized String getName () {
		 return name;
	  }

	  public synchronized void setName (String newname) {
		 name = newname;
	  }

	  public synchronized String getComment () {
		 return comment;
	  }

	  public synchronized void setComment (String newcomment) {
		 comment = newcomment;
	  }

	  public synchronized VpnGroupList [] getVpnGroups () {
		 return vpnGroups;
	  }

	  public synchronized VpnList [] getVpnNames () {
		 return vpnNames;
	  }


	  public synchronized IEdgeCore [] getiEdgeCores () {
		 return iEdgeCores;
	  }

	  public synchronized void setVpnGroups (VpnGroupList [] newgrps) {
		 vpnGroups = newgrps;
	  }


	  public synchronized void setVpnNames (VpnList [] newnames) {
		 vpnNames = newnames;
	  }

	  public synchronized void setiEdges (IEdgeCore [] newiedges) {
		 iEdgeCores = newiedges;
	  }

	  public synchronized void addVpnGroup (VpnGroupList newgrp) throws ArrayStoreException, IndexOutOfBoundsException, NullPointerException {
		 if (vpnGroups == null) {
			vpnGroups = new VpnGroupList [1];
			vpnGroups[0] = newgrp;
		 } else {
			VpnGroupList [] tmp = new VpnGroupList [vpnGroups.length+1];
			System.arraycopy(vpnGroups, 0, tmp, 0, vpnGroups.length);
			tmp[vpnGroups.length] = newgrp;
			vpnGroups = tmp;
		 }
	  }



	  public synchronized void addVpnName (VpnList newname) throws ArrayStoreException, IndexOutOfBoundsException, NullPointerException {
		 if (vpnNames == null) {
			vpnNames = new VpnList [1];
			vpnNames[0] = newname;
		 } else {
			VpnList [] tmp = new VpnList [vpnNames.length+1];
			System.arraycopy(vpnNames, 0, tmp, 0, vpnNames.length);
			tmp[vpnNames.length] = newname;
			vpnNames = tmp;
		 }
	  }

	  public synchronized void addiEdgeCore (IEdgeCore newiedge) throws ArrayStoreException, IndexOutOfBoundsException, NullPointerException {
		 if (iEdgeCores == null) {
			iEdgeCores = new IEdgeCore [1];
			iEdgeCores[0] = newiedge;
		 } else {
			IEdgeCore [] ic = new IEdgeCore [iEdgeCores.length+1];
			System.arraycopy(iEdgeCores, 0, ic, 0, iEdgeCores.length);
			ic[iEdgeCores.length] = newiedge;
			iEdgeCores = ic;
		 }
	  }

	  public synchronized Date getCreationDate () {
		 return creationDate;
	  }

	  public synchronized Date getModificationDate () {
		 return modificationDate;
	  }

	  public synchronized void setModificationDate (Date newdate) {
		 modificationDate = newdate;
	  }

	  public String toString () {
		 return name;
	  }

}
