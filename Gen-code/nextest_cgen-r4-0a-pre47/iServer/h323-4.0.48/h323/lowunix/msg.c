#ifdef __cplusplus
extern "C" {
#endif

/*
***********************************************************************************

NOTICE:
This document contains information that is proprietary to RADVISION LTD..
No part of this publication may be reproduced in any form whatsoever without
written prior approval by RADVISION LTD..

RADVISION LTD. reserves the right to revise this publication and make changes
without obligation to notify any person of such revisions or changes.

***********************************************************************************
*/

/*
  msg.c

  Messages printing routines
  for libms.a

  Ron Shpilman
  15 August 1995

  Revised: 10 Oct 1995. Add msa.
  Revised: July 14 1997. Add logger support.

  */

#ifndef NOLOGSUPPORT

#include <rvinternal.h>

#include <time.h>
#include <string.h>
#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>

#ifdef UNDER_CE
/* --------------------- Windows CE ------------------- */
#pragma warning (disable : 4201 4214)
#include <windows.h>
#include <winsock.h>

#elif __VXWORKS__
/* No special include files for VxWorks */

#else
/* ------------------- UNIX & pSOS ------------------ */
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>
#endif  /* UNDER_CE / __VXWORKS__ */


#include <ms.h>
#include <msg.h>
#include <seli.h>
#include <token.h>

/* nextone */
#include <syslog.h>

#define MSG_CONF    "msg.conf"
#define LOG_FILE    "logfile.log"

#define DEBUG_LINE    11
#define MSG_MAX_SINKS 10


static void (*stackNotify)( char *line,...);

static char msConfFileName[128]=MSG_CONF; /* configuration file name */
static char msLogFileName[128]=LOG_FILE; /* log file name */

static BOOL msSelected[100];

static char msSinkList[MSG_MAX_SINKS];
typedef enum
{
    msSinkTypeTerminal,
    msSinkTypeFile,
    msSinkTypeLogger
} msSinkType;

tokenT msSinkTokens[] = {
  {(char*)"terminal", msSinkTypeTerminal},
  {(char*)"file", msSinkTypeFile},
  {(char*)"logger", msSinkTypeLogger},
  {NULL, 0}
};


static int msDebugLevel=0;

static FILE *logfiled =0;


void
msSetStackNotify(void (*sN)( char *line,...))
{
    stackNotify=sN;
}

void
msFile(
       /* Set the configuration file name. */
       char *name
       )
{
  strncpy(msConfFileName, name, sizeof(msConfFileName));
}


void
msLogFile(
      /* Set the configuration file name.  */
      char *name
      )
{
  strncpy(msLogFileName, name, sizeof(msLogFileName));
}



/*____________________________Debug Filtering______________________*/

int /* TRUE or RVERROR */
msAdd(
      /* Add module to debug printing list */
      char* moduleName
      )
{
    int loc;

    loc = (int)logRegister((RVHLOGMGR)1, moduleName, (char*)"");
    if (loc > 0)
    {
        msSelected[loc] = TRUE;
        return loc;
    }

    return RVERROR;
}


int /* TRUE or RVERROR */
msDelete(
     /* Delete module from debug printing list */
     char* moduleName
     )
{
    int loc;

    loc = (int)logGetSource(moduleName);
    if (loc > 0)
    {
        msSelected[loc] = FALSE;
        return loc;
    }

    return RVERROR;
}


int /* TRUE or RVERROR */
msDeleteAll(void
        /* Delete all modules from debug printing list */
        )
{
    memset(msSelected, 0, sizeof(msSelected));
    return TRUE;
}


BOOL /* TRUE if module exist in debug list. Otherwise FALSE */
msIsExist(
      char* moduleName
      )
{
    int loc;
    loc = (int)logGetSource(moduleName);
    return loc > 0;
}


/*____________________________Sink Handling______________________*/

int /* TRUE if valid sink name */
msSinkAdd(
      /* Add sink */
      char* sinkName
      )
{
  int id=-1;
  char firstName[64], secondName[64];
  int numOfNames; /* number of separated strings in sinkName */

  numOfNames=sscanf(sinkName, "%s %s", firstName, secondName);

  if ( (id = GetTokenID(msSinkTokens, firstName)) <0) return RVERROR;
  msSinkList[id]=TRUE;

  switch(id) {
  case msSinkTypeFile:
    if (numOfNames>1)
      msLogFile(secondName); /* register new logfile name */
    if (logfiled)
      fclose(logfiled); /* close current logfile */
    /* Open the new file */
    if ((logfiled = fopen(msLogFileName, "wb")) == NULL)
      msaPrintFormat(-1, "MSG:msOpen: Cannot open log file: '%s' for writing.", msLogFileName);

    break;
  default:
    break;
  }

  return TRUE;
}


int /* TRUE if valid sink name */
msSinkDelete(
         /* Delete sink */
         char* sinkName
         )
{
  int id = GetTokenID(msSinkTokens, sinkName);
  if (id>=0) {
    msSinkList[id]=FALSE;
    return TRUE;
  }
  return RVERROR;
}




/*____________________________Debug Handling______________________*/




static int msCalls = 0;

void
msOpen(void)
{
  FILE *fd;
  char line[200];

  msCalls++;

  if (msCalls<=1)
      /* initialization */
      memset(msSelected, 0, sizeof(msSelected));

  if (msConfFileName[0] && ((fd = fopen(msConfFileName, "r")) != 0))
  {
      while (fgets(line, 200, fd))
      {
          if (strlen(line) < 2) continue; /* ignore empty lines */
          if (line[0] == '#') continue; /* ignore remarks */

          if (line[0] == '%')
          {
              msDebugLevel = atoi(line+1);
              continue;
          }
          line[strlen(line)-1] = 0; /* remove \n */

          if (line[0] == '>')
          {
              msSinkAdd(line+1);
              continue;
          }

          msAdd(line);
      }
      fclose(fd);
  }
}


void
msClose(void)
{
  msCalls--;
  if (msCalls>0) return;

  seliEnd();
  if ( logfiled )
    fclose(logfiled);
}




void
msPrint(int type, char *line)
{
    if (!line) return;

    if ((type == -1)  || (msaGetDebugLevel() > 3) || msSelected[type])
    {
        if (msSinkList[msSinkTypeTerminal])
        {
            printf(line);
            printf("\n");
        }

        if (msSinkList[msSinkTypeFile])
        {
            fwrite(line, (int)strlen(line), 1, logfiled);
            fwrite("\n", 1, 1, logfiled);
            fflush(logfiled);
/* nextone (use syslog) */
#if 0
	    syslog(LOG_LOCAL3|LOG_DEBUG, "%s", line);
#endif
        }

        if (stackNotify)
            stackNotify(line);

#if 0
        /* !!!!!!!!!!!!!SHOULD BE REMOVED !!!!!!!*/
        /* -- send line to logger */
        if (msSinkList[msSinkTypeLogger])
        {
            logFmtPrtNorm(line);
        }
#endif
    }
}


int
msGetDebugLevel(void)
{
    return msDebugLevel;
}

int
msSetDebugLevel(int debugLevel)
{
    msDebugLevel = debugLevel;
    return TRUE;
}


BOOL /* TRUE if type debugging should be printed */
msIsSelected(int type)
{
    if (msGetDebugLevel() > 3)
        return TRUE;
    return msSelected[type];
}


#else  /* NOLOGSUPPORT */

int RV_dummyMsg = 0;

#endif  /* NOLOGSUPPORT */

#ifdef __cplusplus
}
#endif
