#include "queue.h"
#include "metadatamanager.h"
#include <unistd.h>
#include "datamanager.h"

/* Put data in the queue for database
*/
int 
DbPut(DbEnv env, Data data, int* error)
{
  static char fn[] = "DbPut";
  List current =NULL;
  FileInfo writeFileInfo=NULL;
  FileInfo currentFileInfo =NULL;
  Db db=NULL;

  int retVal = 0;

  db = (Db) env->dbHandle;

  //Obtain lock on file info list
  NETDEBUG(MQUEDB, NETLOG_DEBUG4, ("%s Obtaining lock for fileInfoList\n",fn));
  pthread_mutex_lock(&db->fileInfoMutex);

  writeFileInfo = (FileInfo) listGetEndItem( db->fileInfoList);
  current = db->fileInfoCurrent;
  if (current != NULL)
    {
      currentFileInfo = (FileInfo) db->fileInfoCurrent->item;
    }

  //Check if  file already has maximum records?
  if( writeFileInfo == NULL || writeFileInfo->numRecords >= env->numRecords  )
    {

      if ( currentFileInfo != NULL && 
	   writeFileInfo != NULL &&
	   currentFileInfo->index != writeFileInfo->index )
	{
	  NETDEBUG(MQUEDB, NETLOG_DEBUG4, ("%s Closing file descriptor for file %d\n",fn,writeFileInfo->index));

	  close( writeFileInfo->fd );
	  writeFileInfo->fd = -1;
	}
      if ( AddFileInfo( db, error) == -1 )
	{
	  NETERROR(MQUEDB, ("%s Error occured adding file info to fileInfo List\n",fn));
          retVal = -1;
	  goto _return;
	}
    }
  // write the record data to the database 
  writeFileInfo = (FileInfo) listGetEndItem( db->fileInfoList);
  NETDEBUG(MQUEDB,NETLOG_DEBUG1,("%s Writing record in file %d at offset%"PRIu32" \n",fn, writeFileInfo->index, db->writeOffset));

  if (  WriteRecord( db->env, writeFileInfo, data, &db->writeOffset, error) == -1 )
    {
        NETERROR(MQUEDB,("%s Error occured writing record in file %d \n",fn, writeFileInfo->index));
	retVal = -1;
	goto _return;
    }


_return:
  //Unlock lock on file info list
  NETDEBUG(MQUEDB, NETLOG_DEBUG4, ("%s Releasing lock for fileInfoList\n",fn));
  pthread_mutex_unlock(&db->fileInfoMutex);
  return(retVal);
  

}


