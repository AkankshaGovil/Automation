/****************************************************************************
* naptr_client.c
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
#include "formatter.h"
#include "resolvers.h"

#define MAX_RESULTS 20

int main(int argc, char **argv) {
   NAPTR_record *results[MAX_RESULTS];
   char **root_domains=NULL;
   char *client_default_domains[]={"trial.e164.com.",NULL,NULL,NULL};

   int i;
   int ret;

   /*
   ** Check command-line
   */
   if(argc<2) {
      fprintf(stderr,"usage: %s <E.164> [root domains (up to 4)] \n",argv[0]);
      exit(1);
   }
   root_domains=(argc>2) ? argv+2 : client_default_domains; 

   /*
   ** Do the queries
   */
   ret = naptr_resolve(argv[1], root_domains, results, MAX_RESULTS);

   /*
   ** Display results
   */
   for(i=0;i<ret;i++) {
      printf("Result #%d\n domain = %s\n ttl = %lu\n order = %u\n pref = %u\n flags = %s\n service = %s\n regexp = %s\n replacement = %s\n\n",
             i+1,
             results[i]->domain,
             results[i]->ttl,
             results[i]->order,
             results[i]->preference,
             results[i]->flags,
             results[i]->service,
             results[i]->regexp,
             results[i]->replacement);
      free_naptr_record(results[i]);
   }

   printf("Total number of results:%d\n",ret);

   exit(0);
}


