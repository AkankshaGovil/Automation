/*
 * profile.c
 * contains generic interface for profile
 * database
 */
#include <stdio.h>
#include <sys/file.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <syslog.h>
#include <errno.h>

#include <gdbm.h>

#include "alerror.h"
#include "ipc.h"
#include "key.h"
#include "db.h"
#include "profile.h"
#include <malloc.h>

AlStatus
DbProfileStore(DB db, void *profile, DbProfileType type, char *skey, int skeylen)
{
     char fn[] = "DbProfileStore():";

     switch (type)
     {
     case Db_eProfileCall:
     {
	  DbProfileCallKey *pKey = (DbProfileCallKey *)skey;
	  datum key, data;
	  DbProfileCall *pprofile;
	  unsigned short lastSeqNo = 0;
	  /* For this profile, get the last sequence no */
	  if (pprofile = DbProfileGet(db, type, skey, skeylen, &lastSeqNo))
	  {
	       lastSeqNo = pprofile->lastSeqNo;
	  }
	  else
	  {
	       lastSeqNo = 0
;
	  }
     
	  log(LOG_DEBUG, 0, "Got %d as the next to be used sequence number for %s\n", 
	      lastSeqNo, pKey->snkey.regid);

	  pKey->profileType = type;
	  pKey->seqNo = lastSeqNo;
	  
	  key.dptr = (char *)pKey;
	  key.dsize = skeylen;

	  ++lastSeqNo;

	  ((DbProfileCall *)profile)->lastSeqNo = lastSeqNo;
	  data.dptr = profile;
	  data.dsize = sizeof(DbProfileCall);

	  if (DbStore(db, key, data) != AL_OK)
	  {
	       log(LOG_ERR, 0, "%s Storing Error\n", fn);
	       return (AL_DBERRSTORE);
	  }

	  if (pprofile)
	  {

	       /* Also store the first guy again */
	       pKey->seqNo = 0;
	       
	       key.dptr = (char *)&pKey;
	       key.dsize = sizeof(pKey);
	       
	       pprofile->lastSeqNo = lastSeqNo;
	       
	       data.dptr = (void *)pprofile;
	       data.dsize = sizeof(DbProfileCall);

	       log(LOG_DEBUG, 0, "Storing %d as the next to be used seq no for the first entry\n", 
		   lastSeqNo);
	       
	       DbDelete(db, key);

	       if (DbStore(db, key, data) != AL_OK)
	       {
		    log(LOG_ERR, 0, "%s Storing Error 2\n", fn);
		    return (AL_DBERRSTORE);
	       }
	       
	       free(pprofile);
	  }

	  break;
     }
     default:
	  log(LOG_ERR, 0, "%s Undefined case!\n", fn);
	  return (AL_GENERROR);
     }
	
     return AL_OK;
}  

void *
DbProfileGet(DB db, DbProfileType type, char *skey, int skeylen, void *handle)
{
    unsigned short *seqNo = (unsigned short *)handle; 
    char fn[] = "DbProfileGet():";
    void *profile = NULL;

    log(LOG_DEBUG, 0, "%s Seq No is %d\n", fn, *seqNo);

    switch (type)
    {
    case Db_eProfileCall:
    {
	 DbProfileCallKey *pKey = (DbProfileCallKey *)skey;
	 datum key, data;
	 
	 pKey->profileType = type;
	 pKey->seqNo = (*seqNo);
	 
	 key.dptr = (char *)pKey;
	 key.dsize = skeylen;
	 
	 data = DbFind(db, key);

	 if (data.dptr == NULL)
	 {
	      break;
	 }

	 if (data.dsize != sizeof(DbProfileCall))
	 {
	      log(LOG_ERR, 0, "%s Error in stored intry !\n", fn);
	 }
	 else
	 {
	      profile = data.dptr;
	      *seqNo = ++pKey->seqNo;
	 }
	 break;
    }
    default:
	 log(LOG_ERR, 0, "%s Undefined case!\n", fn);
	 break;
    }

    log(LOG_DEBUG, 0, "%s returning Seq No %d\n", fn, *seqNo);    
    return profile;
}  

AlStatus
DbProfileDelete(DB db, DbProfileType type, char *skey, int skeylen)
{
    char fn[] = "DbProfileDelete():";
    
    switch (type)
    {
    case Db_eProfileCall:
    {
	 DbProfileCallKey *pKey = (DbProfileCallKey *)skey;
	 datum key, data;
	 
	 pKey->profileType = type;
	 pKey->seqNo = 0;
	 
	 key.dptr = (char *)&pKey;
	 key.dsize = sizeof(pKey);
	 
	 for (data = DbFind(db, key); data.dptr != 0; data = DbFind(db, key))
	 {
	      log(LOG_DEBUG, 0, "%s Deleting....\n", fn);
	      DbDelete(db, key);
	      pKey->seqNo++;
	 }
	 break;
    }
    default:
	 log(LOG_ERR, 0, "%s Undefined case!\n", fn);
	 break;
    }
    
    return AL_OK;
}  

AlStatus
DbAliasStore(char *alias, char *phone)
{
     	char fn[] = "DbAliasStore():";
	datum key, data;
	DB db;
	DB_tDb dbstruct;

	dbstruct.read_write = GDBM_WRCREAT;
	db = DbOpen(NETOIDS_ALIAS_FILE, ALIAS_LOCK_FILE, &dbstruct);

	key.dptr = alias;
	key.dsize = strlen(alias);

	data = DbFind(db, key);
	if (data.dptr)
	{
		/* Delete this alias */
		DbDelete(db, key);
		free(data.dptr);
	}

	/* Now add again */
	
	data.dptr = phone;
	data.dsize = strlen(phone);

	if (DbStore(db, key, data) != AL_DBOK)
	{
		log(LOG_DEBUG, 0, "%s Insertion problem for %s -> %s\n", fn, alias,
		phone);
	}
	else
	{
		log(LOG_DEBUG, 0, "%s Insertion successful for %s -> %s\n", fn, 
				alias, phone);
	}

	DbClose(&dbstruct);
}

char *
DbAliasFind(char *alias)
{
     	char fn[] = "DbAliasFind():";
	datum key, data;
	DB db;
	DB_tDb dbstruct;

	db = DbOpen(NETOIDS_ALIAS_FILE, ALIAS_LOCK_FILE, &dbstruct);

	key.dptr = alias;
	key.dsize = strlen(alias);

	data = DbFind(db, key);
	if (data.dptr)
	{
#if 0
		/* Delete this alias */
		DbClose(db);
		return (char *)data.dptr;
#endif
	}

	DbClose(&dbstruct);
	return 0;
}
