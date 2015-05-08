/****************************************************************************
* enum_resolver.c
*
* Copyright (c) 2001 NetNumber.com, Inc. All Rights Reserved.
*
* This application is free software; you can redistribute it and/or
* modify it under the terms of the GNU Lesser General Public
* License as published by the Free Software Foundation; either
* version 2.1 of the License, or (at your option) any later version.
*
* This application is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
* Lesser General Public License for more details.
*
* You should have received a copy of the GNU Lesser General Public
* License along with this application; if not, write to the Free Software
* Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*
*****************************************************************************/

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include <sys/types.h>
#include "formatter.h"
#include "resolvers.h"

static ENUM_record *create_enum_record(NAPTR_record *,char *);
static char *parse_protocol(const char *);
static char *parse_enum_uri(const char *);
static int filter_enum_service(ENUM_record *, char *);

/*
** enum_resolve()
**  Retrieves NAPTR records associated with a given E.164 number on given
**  root domains and parses to ENUM records.
**  The root domains will be queried serially in the given array order.
**
**  Parameters:
**   char *e164_number   : null terminated E.164 number string (telephone #)
**   char *service_filter: null terminated service filter string.
**                         This is case-insensitive.  Setting this to NULL
**                         will default to all results.
**   char **root_domains : pointer to array of root domain strings.
**                         Setting this to NULL will default to "e164.com".
**   ENUM_record **results : preallocated array buffer for results.
**   int max_results : size of results buffer (in number of ENUM_records)
**
**  Returns the total number of ENUM records.
*/
int enum_resolve(char *e164_number,
                 char *service_filter,
                 char **root_domains,
                 ENUM_record **results,
                 int max_results) {
   int i,j;
   int total_records = 0;
   NAPTR_record **naptr_records;

   /*
   ** Allocate NAPTR records
   */
   naptr_records = malloc(max_results*sizeof(NAPTR_record *));
   memset(naptr_records,0,max_results*sizeof(NAPTR_record *));

   /*
   ** Query for NAPTR records
   */
   total_records = naptr_resolve(e164_number,
                                 root_domains,
                                 naptr_records,
                                 max_results);

   /*
   ** Convert to ENUM records
   **  note: total_records returned from above will never be greater
   **        than max_results.
   */
   for(i=0,j=0; i<total_records; i++) {
      ENUM_record *enum_record;
      NAPTR_record *current_naptr_record = *(naptr_records+i);

      enum_record = create_enum_record(current_naptr_record,service_filter);
      if(enum_record!=NULL)  *(results+j++) = enum_record;
      free_naptr_record(current_naptr_record);
   }
   free(naptr_records);

   return j;
}



/*
** free_enum_record()
**  Release all underlying memory allocations for a single ENUM_record.
*/
void free_enum_record(ENUM_record *enum_record) {
   if(enum_record!=NULL) {
      free(enum_record->domain);
      free(enum_record->service);
      free(enum_record->uri);
      free(enum_record);
      enum_record=NULL;
   }
}



/*================ private =================================================*/


/*
** create_enum_record()
**  Given an NAPTR and service filter, create the ENUM record (if possible).
** Return: ENUM_record or NULL if any of the following occurred:
**              -Service filter does not match.
**              -NAPTR Flags field doesn't contain 'u'
**              -Failed to parse NAPTR Service field or RegExp field.
**              -Other memory allocation problems.
*/
static ENUM_record *create_enum_record(NAPTR_record *naptr_record,
                                       char *service_filter) {
   ENUM_record *enum_record = malloc(sizeof(ENUM_record));
   memset(enum_record,0,sizeof(ENUM_record));

   /*
   ** Check NAPTR flags field, it must contain 'u'
   */
   if(!(*naptr_record->flags=='u' || *naptr_record->flags=='U')) {
      free_enum_record(enum_record);
      return NULL;
   }

   /*
   ** Service field & check against service filter
   */
   if((enum_record->service = parse_protocol(naptr_record->service))==NULL ||
      filter_enum_service(enum_record,service_filter)!=1) {
      free_enum_record(enum_record);
      return NULL;
   }

   /*
   ** Order & preference field
   */
   enum_record->order = naptr_record->order;
   enum_record->preference = naptr_record->preference;

   /*
   ** Domain field
   */
   if((enum_record->domain = strdup(naptr_record->domain))==NULL) {
      free_enum_record(enum_record);
      return NULL;
   }


   /*
   ** URI field
   */
   if((enum_record->uri = parse_enum_uri(naptr_record->regexp))==NULL) {
      free_enum_record(enum_record);
      return NULL;
   }

   return enum_record;
}




/*
** parse_protocol
**  Get the ENUM "protocol" from the NAPTR service.
*/
static char *parse_protocol(const char *service) {
   char *ret=NULL;
   char *ptr=NULL;
   size_t len;

   ptr=strchr(service,'+');
   if(ptr!=NULL) {
      /* Check the service specifier is '+E2U' */
      if(strcmp("+E2U\0",ptr)==0 || strcmp("+e2u\0",ptr)==0 ) {
         /* Copy the protocol */
         len=ptr-service;
         if((ret=malloc(len+1))!=NULL) {
            strncpy(ret,service,len);
            *(ret+len)='\0';
         }
      }
   }
   return ret;
}



/*
** Get the URI from the given regular expression
*/
static char *parse_enum_uri(const char *regexp) {
   char *ret=NULL;
   char *s_ptr, *e_ptr;
   size_t len;

   /* Search for the last two '!' in the given regexp */
   e_ptr=(char *)(regexp+strlen(regexp));
   while(e_ptr>=regexp && *e_ptr!='!') --e_ptr;

   s_ptr=e_ptr;
   while(s_ptr>=regexp) {
      if(*(--s_ptr)=='!') {
         ++s_ptr;
         break;
      }
   }

   /* Copy string between the '!' */
   len=e_ptr-s_ptr;
   if( e_ptr!=0 && s_ptr!=0 && len>0 ) {
      if( (ret=malloc(len+1))!=NULL ) {
         *(ret+len)='\0';
         strncpy(ret,s_ptr,len);
      }
   }

   return ret;
}



/*
** filter_enum_service()
**  Compare the given filter and service in ENUM_record.  Matching
**  is _not_ case sensitive ("sip" equals "SiP").  If filter is NULL,
**  it will return a match.
**  Returns 1 if match is okay, otherwise 0.
*/
static int filter_enum_service(ENUM_record *record, char *filter) {
   char *new_filter;
   char *new_srvc;
   int ret;
   int i;

   if(filter==NULL) return 1;

   /* Convert all strings to lower-case */
   new_filter=strdup(filter);
   new_srvc=strdup(record->service);
   for(i=strlen(new_filter);i>=0;i--) *(new_filter+i)=tolower(*(new_filter+i));
   for(i=strlen(new_srvc);i>=0;i--) *(new_srvc+i)=tolower(*(new_srvc+i));
   ret=(strcmp(new_filter,new_srvc)==0)?1:0;
   free(new_filter);
   free(new_srvc);
   return ret;
}

