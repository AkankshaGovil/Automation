package com.nextone.JServer;

import java.util.*;
import java.io.*;
import com.nextone.common.Bridge;
import com.nextone.common.IEdgeCore;
import com.nextone.common.IEdgeList;
import com.nextone.common.MaintenanceGroup;
import com.nextone.common.MaintenanceRequest;
import com.nextone.common.VpnGroupList;
import com.nextone.common.VpnList;


/**
 * This class is used to query the auto download configuration
 * information. This class acts as a cache and makes sure we are not
 * building the information too many times
 *
 * Data is organized as:
 *   list --> hashmap (hash key is the MaintenanceRequest, hashed
 *              object is an array of Sets of size 3)
 *            Set 1 has a bunch of vpn groups covered by the MR
 *            Set 2 has a bunch of vpn ids covered by the MR
 *            Set 3 has a bunch of iedges covered by the MR
 */
public class AutoDownloadDataCache {
	  private static AutoDownloadDataCache thisInstance;
	  private HashMap list;
	  private boolean listStatus;
	  private String listError, listErrorDetails;
	  private long lastTouched;

	  private AutoDownloadDataCache () { }


	  public static AutoDownloadDataCache getInstance (Bridge bs) {
		 return AutoDownloadDataCache.getInstance(bs, false);
	  }

	  /**
	   * This method is the only way to get access to this class, this method
	   * might block sometimes, depending on the age of the cache
	   * Once a reference is obtained through this method, the data contents
	   * can be accesses through the other public methods in this class
	   *
	   */
	  public synchronized static AutoDownloadDataCache getInstance (Bridge bs, boolean forceRefresh) {
		 if (thisInstance == null)
			thisInstance = new AutoDownloadDataCache();

		 long currentTime = System.currentTimeMillis();
		 if (forceRefresh ||
			 currentTime > (thisInstance.lastTouched + 180000)) {
			thisInstance.listStatus = thisInstance.buildDatabase(bs);
			if (thisInstance.listStatus)
			   thisInstance.lastTouched = currentTime;
		 }

		 return thisInstance;
	  }

	  private boolean buildDatabase(Bridge bs) {
		 list = new HashMap();
		 listError = "";
		 listErrorDetails = "";

		 // find the list of auto-download groups
		 String [] groups = null;
		 try {
			groups = bs.getAutoDownloadGroupNames();
		 } catch (Exception e) {
			listError = "Error getting group names";
			listErrorDetails = e.toString();
			return false;
		 }

		 if (groups == null || groups.length == 0)
			return true;

		 for (int i = 0; i < groups.length; i++) {
			MaintenanceRequest mr = null;
			try {
			   mr = bs.getAutoDownloadConfig(groups[i], true);
			} catch (Exception e) {
			   listError = "Error reading configuration for " + groups[i];
			   listErrorDetails = e.toString();
			   return false;
			}

			Set [] sets = new Set [3];
			for (int l = 0; l < sets.length; sets[l] = new TreeSet(), l++);

			// for each maintenance group, find out the vpn groups/ids and
			// edges configured in it
			String [] mgroups = mr.getGroups();
			for (int j = 0; mgroups != null && j < mgroups.length; j++) {
			   MaintenanceGroup mg = null;
			   try {
				  mg = bs.getMaintenanceGroup(mgroups[j]);
			   } catch (Exception e) {
				  listError = "Error reading groups for " + groups[i];
				  listErrorDetails = e.toString();
				  return false;
			   }

			   VpnGroupList [] vgl = mg.getVpnGroups();
			   for (int k = 0;
					vgl != null && k < vgl.length;
					sets[0].add(vgl[k].getVpnGroup()), k++);

			   // check if the iedge is in any of the ids configured
//			   VpnList [] vl = mg.getVpnIds();
			   VpnList [] vl = mg.getVpnNames();
			   for (int k = 0;
					vl != null && k < vl.length;
//					sets[1].add(vl[k].getVpnId()), k++);
					sets[1].add(vl[k].getVpnName()), k++);

			   // check if the iedge is in any of the IEdgeCores configured
			   IEdgeCore [] ic = mg.getiEdgeCores();
			   for (int k = 0;
					ic != null && k < ic.length;
					sets[2].add(ic[k].getRegId()), k++);
			}

			list.put(mr, sets);
		 }

		 return true;
	  }

	  public synchronized boolean getStatus () {
		 return listStatus;
	  }

	  public synchronized String getErrorMessage () {
		 return new String(listError);
	  }

	  public synchronized String getErrorDetails () {
		 return new String(listErrorDetails);
	  }

	  public synchronized MaintenanceRequest getMaintenanceRequest (IEdgeList il) throws IllegalStateException {
		 if (!listStatus)
			throw new IllegalStateException("List incomplete: " + listError + "\n" + listErrorDetails);

		 if (list == null || list.isEmpty())
			return null;

		 Set ks = list.keySet();
		 Iterator i = ks.iterator();
		 while (i.hasNext()) {
			MaintenanceRequest mr = (MaintenanceRequest)i.next();
			Set [] sets = (Set [])list.get(mr);
      //  getVpnGroup() removed since 2.0
//			if (sets[0].contains(il.getVpnGroup()) ||
//				sets[1].contains(il.getVpnId()) ||
			if(	sets[1].contains(il.getVpnName()) ||
				sets[2].contains(il.getSerialNumber()))
			   return mr;
		 }

		 return null;
	  }

}

