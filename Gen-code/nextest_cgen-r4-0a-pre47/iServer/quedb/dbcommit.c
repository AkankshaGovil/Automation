#include "queue.h"
#include "metadatamanager.h"
#include "datamanager.h"

int 
DbCommit(DbTxn t, int* error)
{
  static char fn[] ="DbCommit";
  FileInfo finfo;
  Db       db; 
  int retVal = 0;
  
  // Mark record as deleted in the database file
  finfo = (FileInfo) t->fileHandle;
  db = (Db) t->dbHandle; 

  //Lock file info list
  NETDEBUG(MQUEDB, NETLOG_DEBUG4, ("%s Obtaining lock for fileInfoList\n",fn));
  pthread_mutex_lock(&db->fileInfoMutex);
  
  NETDEBUG(MQUEDB, NETLOG_DEBUG1, ("%s Attempting to delete record from file %d at offset %"PRIu32" \n",fn, finfo->index, t->offset));
  if( DeleteRecord( db->env, finfo, t->offset, error ) == -1)
    {
      NETERROR(MQUEDB,  ("%s Error occured deleting record from file %d at offset %"PRIu32" \n",fn, finfo->index, t->offset));
      retVal = -1;
      goto _return;
    }
  NETDEBUG(MQUEDB, NETLOG_DEBUG1, ("%s Record deleted successfully from file %d at offset %"PRIu32" \n",fn, finfo->index, t->offset));
    
  // check if all records in the file have status deleted?  
  if ( finfo->numRecords >= db->env->numRecords
           && finfo->numActiveRecords == 0 )
    {
      //Delete file info metadata entry
      NETDEBUG(MQUEDB, NETLOG_DEBUG1, ("%s Deleting file %d \n",fn, finfo->index));
      if(DeleteFileInfo( db, finfo, error) == -1)
	{
	  NETERROR(MQUEDB, ("%s Error occured deleting file\n",fn));
	  retVal = -1;
	  goto _return;
	}

    }

  _return:
	  //Unlock file info list
          NETDEBUG(MQUEDB, NETLOG_DEBUG4, ("%s Releasing lock for fileInfoList\n",fn));
	  pthread_mutex_unlock(&db->fileInfoMutex);
	  return retVal;


}

