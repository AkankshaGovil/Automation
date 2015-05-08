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
#include "profile.h"
#include "db.h"
#include "log.h"
#include "mem.h"

char netoid_list[80], netoid_scroll[80];
int debug = 4;

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
     if (!(db = DbOpen(NETOIDS_DB_FILE, LOCK_DB_FILE)))
     {
	  log(LOG_ERR, errno, "%s Unable to open database %s\n", NETOIDS_DB_FILE);
	  return AL_FILENOTFOUND;
     }

     /* Database if open! */
     
     for (info = DbGetFirstInfoEntry(db); info != 0; 
	  info = DbGetNextInfoEntry(db, serNo))
     {
	  char netoidListField[80];

	  sprintf(netoidListField, "\"%d %s\"\n", info->regid,
		  stateString[info->stateFlags]);

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
     
     log(LOG_DEBUG, 0, "%s Netoid RegId is %d\n", fn, serNo);

     /* Loop thru the profile database, and add eveything to the text
      * window specified by argv 2
      */
     if (!(db = DbOpen(NETOIDS_PROFILE_FILE, LOCK_PROFILE_FILE)))
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

	  /* Insert this into the profile text window - argv 1 */
	  sprintf(profStr, "\"CALL START: %sCALL FIN: %sCALLED PHONE %s\n\n\"", 		timebufC, timebufF, profile->dPhone);
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

     if (argc != 2)
     {
	  log(LOG_ERR, 0, "%s Invalid Arguments\n", fn);
	  return TCL_ERROR;
     }

     if (!(db = DbOpen(NETOIDS_DB_FILE, LOCK_DB_FILE)))
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
     MemShmAtt();
     MemChk();

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
     MemShmDetach();
     
     /* Close the database */
     DbClose(db);

     return TCL_OK;
}
  
int
ALUI_AddNetoid( ClientData clientdata, Tcl_Interp *interp,
               int argc, char *argv[] )
{
     DB db;
     NetoidInfoEntry *netInfo;
     long serNo;
     char fn[] = "ALUI_AddNetoid():";

     if (argc != 2)
     {
	  log(LOG_ERR, 0, "%s Invalid Arguments\n", fn);
	  return TCL_ERROR;
     }

     if (!(db = DbOpen(NETOIDS_DB_FILE, LOCK_DB_FILE)))
     {
	  log(LOG_ERR, errno, "%s Unable to open database %s\n", NETOIDS_DB_FILE);
	  return AL_FILENOTFOUND;
     }

     serNo = atoi(argv[1]);
     log(LOG_DEBUG, 0, "%s RegId is %d\n", fn, serNo);
     
     if (netInfo = DbFindInfoEntry(db, serNo))
     {
        free (netInfo);
	log(LOG_DEBUG, 0, "Entry already exists\n");
	Tcl_SetResult(interp, "0", TCL_STATIC);
	goto _return;
     }

     netInfo = GetNewNetInfoEntry();
     InitNetoidInfoEntry(netInfo);

     netInfo->regid = serNo;
     netInfo->iTime = time(0);
     netInfo->rTime = (time_t)0;
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

     if (!(db = DbOpen(NETOIDS_DB_FILE, LOCK_DB_FILE)))
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
	  char s1[16];
	  sprintf(result, "1 * %s * %s * %s", FormatIpAddress(netInfo->ipAddress.l, s1), stateString[netInfo->stateFlags],
		  ctime(&netInfo->iTime));
          free(netInfo);
     }

     Tcl_SetResult(interp, result, TCL_VOLATILE);

     /* Close the database */
     DbClose(db);

     return TCL_OK;
}   
