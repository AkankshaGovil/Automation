#include "bits.h"
#include "ipc.h"

#include "srvrlog.h"
#include "fdsets.h"
#include "timer.h"
#include "connapi.h"
#include "lsconfig.h"
#include "sconfig.h"
#include "mem.h"
#include "age.h"
#include "xmltags.h"

#include "gis.h"
#include <malloc.h>

List SipStackTimers;

SipBool 
fast_startTimer( 
	 SIP_U32bit duration, 
	 SIP_S8bit restart,
	 SipBool (*timeoutfunc)(SipTimerKey *key, SIP_Pvoid buf), 
	 SIP_Pvoid buffer, 
	 SipTimerKey *key, 
	 SipError *err
	 )
{
	 char fn[] = "fast_startTimer():";
	 SIPTimerEntry *elem = NULL;
	 SipError error;
	 static struct itimerval siptimer;

	return SipFail;

	 /* First thing is to make sure that the timer for this key does not 
	  * exist already 
	  */
	 elem = SipSearchTimerInList(key);
	 if (elem != NULL)
	 {
		  NETERROR(MSIP, 
				   ("%s Timer already exists for the key in the list!\n", fn));
		  *err = E_TIMER_DUPLICATE;
		  return SipFail;
	 }

	 /* Malloc an entry for timer */
	 elem = (SIPTimerEntry *)malloc(sizeof(SIPTimerEntry));
	 if (elem == NULL)
	 {
		  NETERROR(MSIP, 
				   ("%s Malloc for sipTimerEntry failed!\n", fn));
		  *err = E_TIMER_FULL;
		  return SipFail;
	 }

	 memset(elem, 0, sizeof(SIPTimerEntry));

	 /* Start a timer for the duration mentioned */
	 memset(&siptimer, 0, sizeof(struct itimerval));
	 siptimer.it_value.tv_sec = duration/1000;
	 siptimer.it_value.tv_usec = (duration%1000) * 1000;
	 elem->sip_timerid = 
		  timerAddToList(&localConfig.timerPrivate, &siptimer, 1,
						 PSOS_TIMER_REL, "SipTimer", sipTimerExpiry, elem);

	 if (elem->sip_timerid == 0)
	 {
		  NETERROR(MSIP, ("Add SipStackTimer failed!\n"));
		  *err = E_TIMER_FULL;
		  free (elem);
		  return SipFail;
	 }

	 /* Add siptimer into our list of siptimers */
	 elem->duration = duration;
	 elem->restart = restart;
	 elem->fn = timeoutfunc;
	 elem->buf = buffer;
	 elem->key = key;
#if 0
	 print_sip_timer_key( key);
#endif

	 /* Insert element into SipStackTimers list */
	 listAddItem(SipStackTimers, elem);

	 return SipSuccess;
}

SipBool 
fast_stopTimer(
	 SipTimerKey *inkey, 
	 SipTimerKey **outkey, 
	 SIP_Pvoid *buffer,  
	 SipError *err
)
{
	 char fn[] = "fast_stopTimer():";
	 SIPTimerEntry *elem = NULL;
	 SipError error;
	 void *timerdata;
	
	return SipFail;

#if 0
	 print_sip_timer_key( inkey);
#endif
	 /* Get the timer entry from sip stack timer list */
	 elem = SipSearchTimerInList(inkey);
	 if (elem == NULL)
	 {
		  NETERROR(MSIP, 
		   ("%s Stack asked to clear a Timer entry which does not exist in the list\n", fn));
		  *err = E_TIMER_NO_EXIST;
		  return SipFail;
	 }

	 /* Stop the timer first */
	 timerDeleteFromList(&localConfig.timerPrivate, elem->sip_timerid, &timerdata);
	 elem->sip_timerid = 0;
	 *outkey = elem->key;
	 *buffer = elem->buf;

	 /********** Note: the following section needs to be removed once HSS fixes their stack *****/
	 {
		  SipEventContext *context = ((SipTimerBuffer *)(elem->buf))->pContext;
		  if (context)
		  {
			   SipCheckFree(context->pTranspAddr);
			   free(context);
		  }
	 }

	 /* delete element from SipTimer List */
     listDeleteItem(SipStackTimers, elem);
	 /* free the timer list entry */
	 free(elem);

	 return SipSuccess;
}

/* This function can be called for deleting a timer for a specific method for a callid. It can
 * also be called with method == NULL, to delete all timers for this call_id. This is useful
 * when we want to delete a party!
 */
void 
sip_delete_timer_from_list(char * callid, char * method)
{
	 char fn[] = "sip_delete_timer_from_list():";
     ListHead * lh;
     ListStruct * ptr;
     int	found = 0;
	 int		method_present = 0;
     SIPTimerEntry *timerentry = 0;
	 SipTimerBuffer *tbuf;
	 void *timerdata;
     
	 if (method)
		  method_present = 1;
	 DEBUG(MSIP, NETLOG_DEBUG4, ("Deleting timer for call %s\n", callid));
	 if (method_present)
		  DEBUG(MSIP, NETLOG_DEBUG4, ("Deleting timer for method %s\n", method));

 begin_deletion:
     lh = SipStackTimers->head;
     ptr = lh->begin;
     found = 0;

     while (ptr)
     {
		  timerentry = (SIPTimerEntry *)ptr->item;
		  if (timerentry)
		  {
			   if (strcmp(timerentry->key->dCallid, callid) == 0)
			   {
					if(method_present)
					{
						 if (strcmp(timerentry->key->pMethod, method) == 0)
						 {
							  found = 1;
							  break;
						 }
					}
					else
					{
						 found = 1;
						 break;
					}
			   }
		  }
		  ptr = ptr->next;
     }
     
     if (found)
	 {
		  if (method_present)
			   DEBUG(MSIP, NETLOG_DEBUG4, 
					 ("%s Found timer for call %s, %s \n", fn, callid, method));
		  else
			   DEBUG(MSIP, NETLOG_DEBUG4, 
					 ("%s Found timer for call %s\n", fn, callid));

		  /* Delete this entry */

		  /* Stop the timer first */
		  timerDeleteFromList(&localConfig.timerPrivate, timerentry->sip_timerid, &timerdata);

		  /* Delete key and buffer data */
		  {
			   SipEventContext *context = ((SipTimerBuffer *)(timerentry->buf))->pContext;
			   if (context)
			   {
					SipCheckFree(context->pTranspAddr);
					free(context);
			   }
		  }
		  tbuf = (SipTimerBuffer *) (timerentry->buf);
		  fast_memfree(0,timerentry->key->dCallid,NULL);
		  fast_memfree(0,timerentry->key->pMethod,NULL);
		  sip_freeSipFromHeader(timerentry->key->pFromHdr);
		  sip_freeSipToHeader(timerentry->key->pToHdr);
		  sip_rpr_freeSipRAckHeader(timerentry->key->pRackHdr);
		  fast_memfree(0,timerentry->key,NULL);
		  fast_memfree(0,tbuf->pBuffer,NULL);
		  fast_memfree(0,tbuf,NULL);

		  /* delete element from SipTimer List */
		  listDeleteItem(SipStackTimers, timerentry);
		  /* free the timer list entry */
		  free(timerentry);
		
		  /* Now that we have successfully deleted entry, see if we need to go back
		   * to delete other entries for the same call, when method is null.
		   */
		  if (!method_present)
			   goto begin_deletion;
	 }
	 else
	 {
		  if (method_present)
			   DEBUG(MSIP, NETLOG_DEBUG4, ("No timer found for call %s, %s \n", callid, method));
		  else
			   DEBUG(MSIP, NETLOG_DEBUG4, ("No timer found for call %s\n", callid));
	 }

	 return;
}

SIPTimerEntry * 
SipSearchTimerInList(SipTimerKey * key)
{
     ListHead * lh = SipStackTimers->head;
     ListStruct * ptr = lh->begin;
     int	found = 0;
     SIPTimerEntry *timerentry = 0;
	 SipError err;
     
     while (ptr)
     {
		  timerentry = (SIPTimerEntry *)ptr->item;
		  if (timerentry)
		  {
#if 0
			   print_sip_timer_key( timerentry->key);
#endif
			   if (sip_compareTimerKeys(timerentry->key, key, &err) == SipSuccess)
			   {
					found = 1;
					break;
			   }
		  }
		  ptr = ptr->next;
     }
     
     if (found)
		  return (timerentry);
     else
		  return (NULL);
}

int 
sipTimerExpiry(struct Timer *timer)
{
	 SIPTimerEntry *elem = (SIPTimerEntry *) (timer->data);
	 SIPTimerEntry *newelem = NULL;
	
	 timerFreeHandle(timer);

	 if (elem == NULL)
	 {
		  NETERROR(MSIP, ("Call back data empty!\n"));
		  return(0);
	 }
	 /* Get the timer entry from sip stack timer list */
	 newelem = SipSearchTimerInList(elem->key);
	 if (newelem == NULL)
	 {
		  NETERROR(MSIP, ("Timer entry does not exist in the list\n"));
		  free(elem);
		  return(0);
	 }

	 /* delete element from SipTimer List */
     listDeleteItem(SipStackTimers, elem);

	 /* call back the  sip stack function */
	 elem->fn(elem->key, elem->buf);

	 /* free the timer list entry */
	 free(elem);

	 return(0);
}


#if 0
int
print_sip_timer_key(SipTimerKey * key)
{
	 int size = 0;
	 int i = 0;
	 char * args;
	 SipError err;
	
	 printf("Sip Timer Key:\n");
	 printf("dMatchType == %d\n", key->dMatchType);
	 printf("dMatchFlag == %d\n", key->dMatchFlag);
	 printf("dCallid == %s\n", key->dCallid);
	 printf("pMethod == %s\n", key->pMethod);
	 printf("dCseq == %d\n", key->dCseq);
	 printf("dCodeNum == %d\n", key->dCodeNum);
	 printf("From Hdr:\n");
#if 0
	 printf("pDispName == %s\n", key->pFromHdr->pDispName);
#endif
	 printf("Addr Spec:\n");
	 printf("pUser == %s\n", key->pFromHdr->pAddrSpec->u.pSipUrl->pUser);
	 printf("pPassword == %s\n", key->pFromHdr->pAddrSpec->u.pSipUrl->pPassword);
	 printf("pHost == %s\n", key->pFromHdr->pAddrSpec->u.pSipUrl->pHost);
	 printf("dPort == %d\n", key->pFromHdr->pAddrSpec->u.pSipUrl->dPort);
	 printf("pHeader == %s\n", key->pFromHdr->pAddrSpec->u.pSipUrl->pHeader);
	 sip_listSizeOf(&(key->pFromHdr->slTag), &size, &err);
	 if (size)
	 {
		  while (size --)
		  {
			   sip_listGetAt(&(key->pFromHdr->slTag), i++, args, &err);
			   printf("tag: %d: %s\n", args);
		  }
	 }
	 printf("From Hdr:\n");
#if 0
	 printf("pDispName == %s\n", key->pToHdr->pDispName);
#endif
	 printf("Addr Spec:\n");
	 printf("pUser == %s\n", key->pToHdr->pAddrSpec->u.pSipUrl->pUser);
	 printf("pPassword == %s\n", key->pToHdr->pAddrSpec->u.pSipUrl->pPassword);
	 printf("pHost == %s\n", key->pToHdr->pAddrSpec->u.pSipUrl->pHost);
	 printf("dPort == %d\n", key->pToHdr->pAddrSpec->u.pSipUrl->dPort);
	 printf("pHeader == %s\n", key->pToHdr->pAddrSpec->u.pSipUrl->pHeader);
	 sip_listSizeOf(&(key->pToHdr->slTag), &size, &err);
	 if (size)
	 {
		  while (size --)
		  {
			   sip_listGetAt(&(key->pToHdr->slTag), i++, args, &err);
			   printf("tag: %d: %s\n", args);
		  }
	 }
}
#endif
