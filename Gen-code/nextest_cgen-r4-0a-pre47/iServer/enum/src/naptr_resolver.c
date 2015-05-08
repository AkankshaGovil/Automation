/****************************************************************************
* naptr_resolver.c
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
#include <sys/socket.h>
#include <netinet/in.h>
#include "arpa/nameser.h"
#include <resolv.h>
#include <netdb.h>
#include "formatter.h"
#include "resolvers.h"

static char *root_domain_defaults[]={"e164.com.",NULL,NULL,NULL};
static NAPTR_record *create_naptr_record(const ns_msg *,const ns_rr *);
static char *get_rdata_str(const u_char *,const u_char *,size_t *);
static int insertion_sort(NAPTR_record **,NAPTR_record *,int,int);
static signed int comp_records(NAPTR_record *,NAPTR_record *);


/*
** naptr_resolve()
**  Retrieves NAPTR records associated with a given E.164 number on given
**  root domains.  The root domains will be queried serially in the given
**  array order.
**
**  Note: This queries for the raw NAPTR record only.  It does not do
**        the NAPTR looping & rewrite.
**
**  Parameters:
**   char *e164_number   : null terminated E.164 number string (telephone #)
**   char **root_domains : pointer to array of root domain strings.
**                         Setting this to NULL will default to "e164.com".
**   NAPTR_record **results : preallocated buffer for results.
**   int max_results : size of results buffer (in number of NAPTR_records)
**
**  Returns the total number of NAPTR records.
*/
int naptr_resolve(char *e164_number,
                  char **root_domains,
                  NAPTR_record **results,
                  int max_results) {
   char *e164;
   char *e164_fqd;
   int  index;
   int  end_of_array=0;
   int  response_len;
   int  rr_num;
   int  total_records = 0;
   unsigned char buffer[NS_PACKETSZ];
   uint16_t msg_count;
   ns_msg handle;
   ns_rr rr;
   const ns_sect section  = ns_s_an;  /* We want the answer section */
   NAPTR_record **curr_pos = results;

   /*
   ** Set to default root domains if null
   */
   if(root_domains==NULL)  root_domains=root_domain_defaults;

   /*
   ** Clean up E.164 number
   */
   if((e164=reformat_e164(e164_number))==NULL) return 0;

   /*
   ** Loop through all given root domains
   */
   for(index=0;!end_of_array && index<MAX_ROOT_DOMAINS;index++) {
      char *curr_root_domain=*(root_domains+index);
      if(curr_root_domain==NULL) break;

      /*
      ** Convert E.164 number to E.164 fully qualified domain name.
      */
      e164_fqd=format_e164_fqd(NULL,e164,curr_root_domain);

      /*
      ** Do the query
      **  Set the class to Internet & type of query to NAPTR
      ** (See arpa/nameser.h)
      **  Note: The 'res_' functions are not thread-safe.
      */
      response_len=res_query(e164_fqd,
                             ns_c_in,
                             ns_t_naptr,
                             buffer,
                             NS_PACKETSZ);
      free(e164_fqd);

      if(response_len<0) continue;
      if(ns_initparse(buffer,response_len, &handle)<0) continue;

      /*
      ** Get number of messages
      */
      msg_count=ns_msg_count(handle,section);

      /*
      ** Loop through all messages for this root domain
      */
      for(rr_num=0;rr_num<msg_count;rr_num++) {
         if(ns_parserr(&handle,section,rr_num,&rr)>=0) {
            /*
            ** Create an NAPTR_record, insert into results array
            ** sorted by order and preference.
            */
            NAPTR_record *tmp_rec_p = create_naptr_record(&handle,&rr);
            if(tmp_rec_p!=NULL) {
               if(insertion_sort(curr_pos,tmp_rec_p,rr_num,max_results)==0) {
                  end_of_array=1;
                  break;
               }
               total_records++;
            }
         }
      }

      /*
      ** Update position into array such that any results from next
      ** root domain query will be appended (that is, don't mix it
      **  with results from previous root domains).
      ** Update the remaining space in the results buffer.
      */
      curr_pos = results+total_records;
      max_results -= rr_num;
   }
   free(e164);

   return total_records;
}


/*
** free_naptr_record()
**  Release all underlying memory allocations for a NAPTR_record.
*/
void free_naptr_record(NAPTR_record *naptr_record) {
   if(naptr_record!=NULL) {
      free(naptr_record->domain);
      free(naptr_record->flags);
      free(naptr_record->service);
      free(naptr_record->regexp);
      free(naptr_record->replacement);
      free(naptr_record);
      naptr_record=NULL;
   }
}


/*================ private =================================================*/


/**
 *  create_naptr_record()
 */
static NAPTR_record *create_naptr_record(const ns_msg *handle,
                                         const ns_rr *rr) {
   size_t rdchars;
   const u_char *rdata, *edata;
   const u_char *message = ns_msg_base(*handle);
   size_t message_len = ns_msg_size(*handle);
   char buf[NS_MAXDNAME]; /* defined in nameser.h */
   NAPTR_record *naptr_record = malloc(sizeof(NAPTR_record));
   memset(naptr_record,0,sizeof(NAPTR_record));

   /*
   ** Domain field
   */
   naptr_record->domain = strdup(ns_rr_name(*rr));

   /*
   ** TTL field
   */
   naptr_record->ttl = ns_rr_ttl(*rr);

   /*
   ** Copy from rdata
   ** (borrowed from BIND-8.2.2 ns_print.c)
   */
   rdata = ns_rr_rdata(*rr);
   edata = rdata+ns_rr_rdlen(*rr);

   /*
   ** Order & preference field
   */
   naptr_record->order = ns_get16(rdata);
   rdata += NS_INT16SZ;
   naptr_record->preference = ns_get16(rdata);
   rdata += NS_INT16SZ;

   /*
   ** Flags field
   */
   if((naptr_record->flags = get_rdata_str(rdata,edata,&rdchars))==NULL) {
      free_naptr_record(naptr_record);
      return NULL;
   }
   rdata += rdchars;

   /*
   ** Service field
   */
   if((naptr_record->service = get_rdata_str(rdata,edata,&rdchars))==NULL) {
      free_naptr_record(naptr_record);
      return NULL;
   }
   rdata += rdchars;

   /*
   ** RegExp field
   */
   if((naptr_record->regexp = get_rdata_str(rdata,edata,&rdchars))==NULL) {
      free_naptr_record(naptr_record);
      return NULL;
   }
   rdata += rdchars;

   /*
   ** Replacement field
   **  Note: dn_expand() sets the first character to '\0' for the root,
   **        we are going to set it back to '.'
   */
   if(dn_expand(message,message+message_len,rdata,buf,NS_MAXDNAME)==-1) {
      free_naptr_record(naptr_record);
      return NULL;
   }
   if(buf[0]=='\0') {
      buf[0]='.';
      buf[1]='\0';
   }
   naptr_record->replacement = strdup(buf);

   return naptr_record;
}

 

/*
** get_rdata_str()
**  Return the next string from the given raw response record.
*/
static char *get_rdata_str(const u_char *rdata,
                           const u_char *edata,
                           size_t *rdata_read)
{
   size_t n = *rdata;
   char *buffer = NULL;

   *rdata_read=0;
   if (rdata + n <= edata) {
      if((buffer=malloc(n+1))==NULL) return NULL;
      strncpy(buffer,(char *)++rdata,n);
      *(buffer+n)='\0';
      *rdata_read=n+1;
   }
   return buffer;
}



/*
**  insertion_sort()
**   Given a new NAPTR_record, insert into array of pointers, sorted by
**   order/preference.
**  returns 1 if successful, 0 if max_records is reached.
*/
static int insertion_sort(NAPTR_record **records,
                          NAPTR_record *new_record,
                          int valid_records,
                          int max_records) {
   int i,j;

   /* Reached maximum number of records */
   if(valid_records>=max_records) return 0;

   if(valid_records==0) {
      /* Initial case */
      records[0]=new_record;
   } else {
      /*
      ** We're going to search backwards from the given sorted list
      ** until we find that the right position for the new record.
      */
      for(i=valid_records-1; i>=0; i--) {
         if(comp_records(new_record,records[i])>=0) break;
      }
      i++;

      /* Shift this part of array forward */
      for(j=valid_records;j>i;j--) records[j]=records[j-1];

      /* Insert new record */
      records[i]=new_record;
   }
   return 1;
}



/*
**  comp_records()
**   Compare the order & preference between two NAPTR_records
**    -1 = NAPTR record 1 has lower order/preference _value_ than record 2
**     0 = equivalent
**    +1 = NAPTR record 1 has higher order/preference _value_ than record 2
*/
static signed int comp_records(NAPTR_record *rec1, NAPTR_record *rec2) {
    return ( (rec1->order > rec2->order) ? 1 :
             (rec1->order < rec2->order) ? -1 :
             (rec1->preference > rec2->preference) ? 1 :
             ((rec1->preference < rec2->preference) ? -1 : 0) );
}
