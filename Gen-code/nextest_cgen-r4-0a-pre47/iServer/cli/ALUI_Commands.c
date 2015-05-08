/*
 * ALUI_Commands.c
 * Contains commands which integrate the C and GUI
 * parts of the Aloid User Interface
 */

#include "tk.h"
#include "ALUI_Commands.h"
#include <syslog.h>
#include <errno.h>
#include <time.h>

#include "alerror.h"
#include "ipc.h"
#include "db.h"
#include "profile.h"
#include "log.h"
#include "mem.h"

char netoid_list[80], netoid_scroll[80];
int debug = 4;

void *alShmStart = (void *)AL_SHARED_MEMORY_ADDRESS;
CacheTableInfo *alListHead;

int
ALUI_SetNetoidList( ClientData clientdata, Tcl_Interp *interp,
               int argc, char *argv[] )
{
     char fn[] = "ALUI_SetNetoidList():";

     /* The list and scroll specified here, will be used
      * in all commands
      */
     
     if (argc != 3)
     {
	  log(LOG_ERR, 0, "%s: Invalid Arguments\n", fn);
	  return TCL_ERROR;
     }

     strcpy(netoid_list, argv[1]);
     netoid_list[79]='\0';
     strcpy(netoid_scroll, argv[2]);
     
     return TCL_OK;
}

int
ALUI_PopulateNetoidList( ClientData clientdata, Tcl_Interp *interp,
               int argc, char *argv[] )
{
     char fn[] = "ALUI_PopulateNetoidList():";
     DB db;
     NetoidInfoEntry *info;
     long serNo;

     if (argc != 3)
     {
	  log(LOG_ERR, 0, "%s Invalid Arguments\n", fn);
	  return TCL_ERROR;
     }
     
     /* Read netoids from database, and add them one by one
      * to the list */

     /* Initialize the list */
     Tcl_VarEval(interp, argv[1], " delete 0 end", (char *)NULL);
     
     /* Now add */
     if (!(db = DbOpen(NETOIDS_DB_FILE, NETOIDS_LOCK_FILE)))
     {
	  log(LOG_ERR, errno, "%s Unable to open database %s\n", NETOIDS_DB_FILE);
	  return AL_FILENOTFOUND;
     }

     /* Database if open! */
     
     for (info = DbGetFirstInfoEntry(db); info != 0; 
	  info = DbGetNextInfoEntry(db, serNo))
     {
	  char netoidListField[80];
	  char phoneNo[PHONE_NUM_LEN];
	  int vpnPhoneLen = strlen(info->phone);

	  if (info->vpnInfo.vpnExtLen)
	  {
	       memset(phoneNo, 0, PHONE_NUM_LEN);
	       strcpy(phoneNo, info->phone);
	       
	       strcpy(phoneNo+(vpnPhoneLen - info->vpnInfo.vpnExtLen)+1, 
		      info->phone + 
		      (vpnPhoneLen - info->vpnInfo.vpnExtLen));
	       phoneNo[(vpnPhoneLen - info->vpnInfo.vpnExtLen)] = ' ';
	  }
	  else
	  {
	       if (BIT_TEST(info->stateFlags, PROTOID_LINE_EXISTS_BIT))
	       {
		    strcpy(phoneNo, info->vpnInfo.truePhone);
	       }
	       else
	       {
		    strcpy(phoneNo, info->phone);
	       }
	  }
	  
	  log(LOG_DEBUG, 0, "%s --- \n", phoneNo);

	  sprintf(netoidListField, "\"%d %s %s\"\n", info->regid,
		  stateString[info->stateFlags], phoneNo);

	  log(LOG_DEBUG, 0, netoidListField);

	  serNo = info->regid;
	  free(info);
	  
	  Tcl_VarEval(interp, argv[1], " insert end ", netoidListField, (char )NULL);
     }

     /* Close the database */
     DbClose(db);

     return TCL_OK;
}

int
ALUI_ProfileNetoid( ClientData clientdata, Tcl_Interp *interp,
               int argc, char *argv[] )
{
     char fn[] = "ALUI_ProfileNetoid():";
     int seqNo = 0;
     DB db;
     DbProfileCall *profile;
     unsigned long serNo;
     int n = 0;
     char profStr[256];

     if (argc != 3)
     {
	  log(LOG_ERR, 0, "%s Invalid Arguments\n", fn);
	  return TCL_ERROR;
     }
     
     serNo = atoi(argv[2]);
     
     log(LOG_DEBUG, 0, "%s Netoid reg id is %d\n", fn, serNo);
     sprintf(profStr, "\"CALLED PHONE CALL START                        CALL FINISH\n\n\"");

     Tcl_VarEval(interp, argv[1], ".profile insert end ", profStr, (char *)0);
	  

     /* Loop thru the profile database, and add eveything to the text
      * window specified by argv 2
      */
     if (!(db = DbOpen(NETOIDS_PROFILE_FILE, PROFILE_LOCK_FILE)))
     {
	  log(LOG_ERR, errno, "%s Unable to open profile database %s\n",
	      NETOIDS_PROFILE_FILE);
	  return TCL_ERROR;
     }
     
     for (profile = DbProfileGet(db, Db_eProfileCall, serNo, &seqNo);
	  profile != 0; profile =  DbProfileGet(db, Db_eProfileCall, serNo, &seqNo))
     {
	char timebufC[32];
	char timebufF[32];

	strcpy (timebufC, ctime(&profile->cTime));
	strcpy (timebufF, ctime(&profile->fTime));

	timebufC[strlen(timebufC)-1] = '\0';
	timebufF[strlen(timebufF)-1] = '\0';

	  /* Insert this into the profile text window - argv 1 */
	  sprintf(profStr, "\"%s   %s  %s\n\n\"",
		profile->dPhone,
 		timebufC, timebufF);
	  log(LOG_DEBUG, 0, "Inserting %s\n", profStr);

	  Tcl_VarEval(interp, argv[1], ".profile insert end ", profStr, (char *)0);
	  
	  n++;
     } 

     sprintf(profStr, "\"%d Entries found in profile\n\"", n);
     Tcl_VarEval(interp, argv[1], ".profile insert end ", profStr, (char *)0);

     /* Close the database */
     DbClose(db);

     return TCL_OK;
}

int
ALUI_DetailNetoid( ClientData clientdata, Tcl_Interp *interp,
               int argc, char *argv[] )
{
     
     
}

int
ALUI_DeleteNetoid( ClientData clientdata, Tcl_Interp *interp,
               int argc, char *argv[] )
{
     /* This is simple. We just delete the netoid from
      * the database, and the gui deletes it from itself
      * on its own.
      */
     DB db;
     long serNo;
     char fn[] = "ALUI_DeleteNetoid():";
     CacheTableInfo *info;
     int shmId;

     if (argc != 2)
     {
	  log(LOG_ERR, 0, "%s Invalid Arguments\n", fn);
	  return TCL_ERROR;
     }

     if (!(db = DbOpen(NETOIDS_DB_FILE, NETOIDS_LOCK_FILE)))
     {
	  log(LOG_ERR, errno, "%s Unable to open database %s\n", NETOIDS_DB_FILE);
	  return AL_FILENOTFOUND;
     }

     serNo = atoi(argv[1]);
     log(LOG_DEBUG, 0, "%s RegId is %d\n", fn, serNo);

     if (DbDeleteInfoEntry(db, serNo) < 0)
     {
          log(LOG_ERR, errno, "%s database delete error\n", fn);
     }
     else
     {
          log(LOG_DEBUG, 0, "%s Entry Deleted Successfully\n", fn);
     }
     
     /* We must attach to the shared memory here,
      * We need to do it for delete only 
      */
     
     /* Attach to shared memory */
     if ((shmId = 
	  MemCacheAtt(AL_SHARED_CACHE_KEY, SHARED_CACHE_SIZE, IPC_CREAT|SHARED_CACHE_MODE,
		    &alShmStart,
		    &alListHead, CACHE_TABLE_SIZE,
		    CACHE_TABLE_SIZE*CACHE_TABLE_CHAIN_SIZE*sizeof(CacheTableInfo))) 
	 == AL_FILENOTFOUND)
     {
	  log(LOG_DEBUG, 0, "%s Shared Memory Missing...\n", fn);
	  goto _return;
     }

     /* We must delete this guy from the cache as well
      * This should be done after we have deleted from the db,
      * since there might be a chance that someone will lookup around
      * this time, and maybe add the guy back to the cache
      */
     if (info = CacheLookupInfoByRegId(serNo))
     {
	  /* Delete this from cache */
	  /* Acquire locks */
	  if (MemGetRwLock(&((CacheTableEntry *)info->head)->mutex, LOCK_WRITE, LOCK_BLOCK) == AL_OK)
	  {
	       CacheDeleteInfo(info);
	       MemReleaseRwLock(&((CacheTableEntry*)info->head)->mutex);
	       log(LOG_DEBUG, 0, "%s Entry Deleted from Cache also\n", fn);
	  }
	  else
	  {
	       log(LOG_ERR, 0, "%s Could not acquire locks!\n");
	  }
     }

     /* detach from shared memory */
     MemShmDetach(alShmStart);

     _return:
     
     /* Close the database */
     DbClose(db);

     return TCL_OK;
}
  
int
ALUI_AddNetoid( ClientData clientdata, Tcl_Interp *interp,
               int argc, char *argv[] )
{
     char fn[] = "ALUI_AddNetoid():";
     DB db;
     NetoidInfoEntry *netInfo;
     long serNo;
     int portNo;
     char *vpnNo, *extNo, *phoneNo;

     if (argc != 6)
     {
	  log(LOG_ERR, 0, "%s Invalid Arguments\n", fn);
	  return TCL_ERROR;
     }

     if (!(db = DbOpen(NETOIDS_DB_FILE, NETOIDS_LOCK_FILE)))
     {
	  log(LOG_ERR, errno, "%s Unable to open database %s\n", NETOIDS_DB_FILE);
	  return AL_FILENOTFOUND;
     }

     serNo = atoi(argv[1]);
     portNo = atoi(argv[2]);
     log(LOG_DEBUG, 0, "%s RegId is %d port No is %d\n", fn, serNo, portNo);
     
     /* Compute the new reg id */
     serNo = serNo * 10;
     log(LOG_DEBUG, 0, "%s Index RegId is %d\n", fn, serNo);
     
     vpnNo = argv[3]; extNo = argv[4]; phoneNo = argv[5];

     log(LOG_DEBUG, 0, "%s vpn %s ext %s phone %s\n", 
	 fn, vpnNo, extNo, phoneNo);
     
     if (netInfo = DbFindInfoEntry(db, serNo))
     {
#if 0
	  /* Support this kind of functionality until we support updates */
        free (netInfo);
	log(LOG_DEBUG, 0, "Entry already exists\n");
	Tcl_SetResult(interp, "0", TCL_STATIC);
	goto _return;
#endif
     }

     netInfo = GetNewNetInfoEntry();
     InitNetoidInfoEntry(netInfo);

     netInfo->regid = serNo;
     netInfo->iTime = time(0);
     netInfo->rTime = (time_t)0;

     if (strlen(vpnNo))
     {
	  strcpy(netInfo->phone, vpnNo);
     }

     if (strlen(extNo))
     {
	  strcat(netInfo->phone, extNo);
	  netInfo->vpnInfo.vpnExtLen = strlen(extNo);
     }

     if (strlen(phoneNo))
     {
	  strcpy(netInfo->vpnInfo.truePhone, phoneNo);
	  BIT_SET(netInfo->stateFlags, PROTOID_LINE_EXISTS_BIT);
     }

#if 0
     BIT_SET(netInfo->stateFlags, PROTOID_ACTIVE_BIT);
#endif

     if (DbStoreInfoEntry(db, netInfo) < 0)
     {
          log(LOG_DEBUG, errno, "database store error \n");
     }

     free(netInfo);

     Tcl_SetResult(interp, "1", TCL_STATIC);

_return:

     /* Close the database */
     DbClose(db);
     return TCL_OK;
}   
     
int
ALUI_FindNetoid( ClientData clientdata, Tcl_Interp *interp,
               int argc, char *argv[] )
{
     DB db;
     NetoidInfoEntry *netInfo;
     long serNo;
     char fn[] = "ALUI_FindNetoid():";
     char result[256];

     if (argc != 2)
     {
	  log(LOG_ERR, 0, "%s Invalid Arguments\n", fn);
	  return TCL_ERROR;
     }

     if (!(db = DbOpen(NETOIDS_DB_FILE, NETOIDS_LOCK_FILE)))
     {
	  log(LOG_ERR, errno, "%s Unable to open database %s\n", NETOIDS_DB_FILE);
	  return AL_FILENOTFOUND;
     }

     serNo = atoi(argv[1]);
     log(LOG_DEBUG, 0, "%s RegId is %d\n", fn, serNo);
     
     netInfo = DbFindInfoEntry(db, serNo);
     if (!netInfo)
     {
          log(LOG_DEBUG, 0, "No entry of such kind found in db\n");
	  sprintf(result, "0");
     }
     else
     {
	  char s1[256];
	  char vpnNo[PHONE_NUM_LEN], extNo[PHONE_NUM_LEN], phoneNo[PHONE_NUM_LEN];
	  int vpnPhoneLen = strlen(netInfo->phone);
	  
	  if (netInfo->vpnInfo.vpnExtLen)
	  {
	       memset(vpnNo, 0, PHONE_NUM_LEN);
	       strncpy(vpnNo, netInfo->phone, 
		       vpnPhoneLen - netInfo->vpnInfo.vpnExtLen);
	       vpnNo[strlen(vpnNo)] = '\0';

	       strcpy(extNo, netInfo->phone + 
		      (vpnPhoneLen - netInfo->vpnInfo.vpnExtLen));
	       strcpy(phoneNo, netInfo->vpnInfo.truePhone);
	  }
	  else
	  {
	       strcpy(vpnNo, "");
	       strcpy(extNo, "");
	       if (BIT_TEST(netInfo->stateFlags, PROTOID_LINE_EXISTS_BIT))
	       {
		    strcpy(phoneNo, netInfo->vpnInfo.truePhone);
	       }
	       else
	       {
		    strcpy(phoneNo, netInfo->phone);
	       }
	  }

	  sprintf(result, "1 * %s * %s * %s * %s * %s * %s", 
		  vpnNo,
		  extNo,  
		  phoneNo,
		  FormatIpAddress(netInfo->ipAddress.l, s1), 
		  stateString[netInfo->stateFlags],
		  ctime(&netInfo->iTime));

          free(netInfo);
     }

     Tcl_SetResult(interp, result, TCL_VOLATILE);

     /* Close the database */
     DbClose(db);

     return TCL_OK;
}   


