#include "cli.h"
#include "serverp.h"
#include "hello_common.h"

int
HandleRsdClear(Command *comm, int argc, char **argv)
{
  char fn[] = "HandleRsdClear():";
  int rc = xleOk;

  if (argc > 0)
  {
    HandleCommandUsage(comm, argc, argv);
    return -xleInvalArgs;
  }

  // attach to the cache
  if (CacheAttach() > 0)
  {
    // lock the mutex
    if (LockGetLock(&(lsMem->rsdmutex), LOCK_WRITE, LOCK_BLOCK) == AL_OK) {
      // clear the info
      memset(lsMem->rsdInfo, 0, sizeof(RSDInfo));

      // unlock the mutex
      LockReleaseLock(&(lsMem->rsdmutex));
    }
    else
    {
      rc = -xleNoAccess;
    }

    // detach from the cache
    CacheDetach();
  }
  else
  {
    CLIPRINTF((stdout, "unable to attach to cache\n"));
    rc = -xleOpNoPerm;
  }

  return rc;
}


static unsigned int
ExtractIp (char *str)
{
  unsigned int retval;

  if (inet_pton(AF_INET, str, &retval) <= 0)
  {
    CLIPRINTF((stderr, "ip address %s is in invalid format\n", str));
    retval = -1;
  }
  else
    retval = ntohl(retval);

  return retval;
}
static unsigned int
ExtractPort (char *str)
{
	return(atoi(str));
}


static int
ExtractStatus (char *str)
{
  if (!strcasecmp(str, "master"))
    return RS_MASTER;

  return RS_SLAVE;
}


int
HandleRsdAdd(Command *comm, int argc, char **argv)
{
  char fn[] = "HandleRsdAdd():";
  int rc = xleOk;
  unsigned int ip, port;
  int status;
  int i, found = 0; 
  RSDInfo *rsp;

  /* added the port an argument.*/
  if (argc != 3)
  {
    HandleCommandUsage(comm, argc, argv);
    return -xleInsuffArgs;
  }
  
  // validate the args
  port = ExtractPort(argv[0]);
  ip = ExtractIp(argv[1]);
  if (ip == -1)
  {
    return -xleInvalArgs;
  }
  status = ExtractStatus(argv[2]);

  // attach to the cache
  if (CacheAttach() > 0)
  {
    // lock the mutex
    if (LockGetLock(&(lsMem->rsdmutex), LOCK_WRITE, LOCK_BLOCK) == AL_OK) {
      rsp = lsMem->rsdInfo;

      // Check if a record already exists for this ip address
      for (i = 0; (i < rsp->count); i++) {
	if (rsp->records[i].ipaddr == ip) {
          found = 1;
          break;
	}
      }			

      if (found) {
        // Update the new info
	rsp->records[i].status = status;
	rsp->records[i].port = port;
      }
      else if (rsp->count < MAX_RSD_RECORDS)
      {
        // add the new info
	rsp->records[rsp->count].ipaddr = ip;
	rsp->records[rsp->count].status = status;
	rsp->records[rsp->count].port = port;
	rsp->count++;
      }
      else
	rc = -xleNoEntry;

      // unlock the mutex
      LockReleaseLock(&(lsMem->rsdmutex));
    }
    else
    {
      rc = -xleNoAccess;
    }

    // detach from the cache
    CacheDetach();
  }
  else
  {
    CLIPRINTF((stdout, "unable to attach to cache\n"));
    rc = -xleOpNoPerm;
  }

  return rc;
}


int
HandleRsdDelete(Command *comm, int argc, char **argv)
{
  char fn[] = "HandleRsdDelete():";
  int rc = xleOk;
  unsigned int ip;
  int status, i, found;

  if (argc != 1)
  {
    HandleCommandUsage(comm, argc, argv);
    return -xleInsuffArgs;
  }

  // validate the args
  ip = ExtractIp(argv[0]);
  if (ip == -1)
  {
    return -xleInvalArgs;
  }

  // attach to the cache
  if (CacheAttach() > 0)
  {
    // lock the mutex
    if (LockGetLock(&(lsMem->rsdmutex), LOCK_WRITE, LOCK_BLOCK) == AL_OK) {
      // delete the entry
      for (found = i = 0; i < lsMem->rsdInfo->count; i++)
      {
	if (found)
	{
	  // entry has been found already, just copy this record to the previous record
	  memcpy(&lsMem->rsdInfo->records[i-1], &lsMem->rsdInfo->records[i], sizeof(RSDRecord));
	}
	else
	{
	  if (ip == lsMem->rsdInfo->records[i].ipaddr)
	  {
	    found = 1;
	    memset(&lsMem->rsdInfo->records[i], 0, sizeof(RSDRecord));
	  }
	}
      }
      if (found)
	lsMem->rsdInfo->count--;
      else
	rc = -xleNoEntry;

      // unlock the mutex
      LockReleaseLock(&(lsMem->rsdmutex));
    }
    else
    {
      rc = -xleNoAccess;
    }

    // detach from the cache
    CacheDetach();
  }
  else
  {
    CLIPRINTF((stdout, "unable to attach to cache\n"));
    rc = -xleOpNoPerm;
  }

  return rc;
}


int
HandleRsdList(Command *comm, int argc, char **argv)
{
  char fn[] = "HandleRsdList():";
  int rc = xleOk;
  int i;
  char str[32];

  if (argc > 0)
  {
    HandleCommandUsage(comm, argc, argv);
    return -xleInvalArgs;
  }

  // attach to the cache
  if (CacheAttach() > 0)
  {
    // lock the mutex
    if (LockGetLock(&(lsMem->rsdmutex), LOCK_READ, LOCK_BLOCK) == AL_OK) {
      // list the information
      for (i = 0; i < lsMem->rsdInfo->count; i++)
	CLIPRINTF((stdout, "%2d. IP: %s\tStatus: %s\n", (i+1), FormatIpAddress(lsMem->rsdInfo->records[i].ipaddr, str), (lsMem->rsdInfo->records[i].status == RS_MASTER)?"Master":"Slave"));

      // unlock the mutex
      LockReleaseLock(&(lsMem->rsdmutex));
    }
    else
    {
      rc = -xleNoAccess;
    }

    // detach from the cache
    CacheDetach();
  }
  else
  {
    CLIPRINTF((stdout, "unable to attach to cache\n"));
    rc = -xleOpNoPerm;
  }

  return rc;
}
