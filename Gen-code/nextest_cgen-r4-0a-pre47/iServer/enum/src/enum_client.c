/****************************************************************************
* enum_client.c
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
   ENUM_record *results[MAX_RESULTS];
   char **root_domains=NULL;
   char *client_default_domains[]={"trial.e164.com.",NULL,NULL,NULL};
   char *filter=NULL;
   int i;
   int ret;

   /*
   ** Check command-line
   */
   if(argc<3) {
      fprintf(stderr,"usage: %s <E.164> <filter|\"all\"> [root domains (up to 4)] \n",argv[0]);
      exit(1);
   }
   if(strcmp(*(argv+2),"all")!=0 && strcmp(*(argv+2),"ALL")!=0)
     filter=*(argv+2);
   root_domains=(argc>3) ? argv+3 : client_default_domains; 

   /*
   ** Do the queries
   */
   ret = enum_resolve(argv[1], filter, root_domains, results, MAX_RESULTS);

   /*
   ** Display results
   */
   for(i=0;i<ret;i++) {
      printf("Result #%d\n domain = %s\n order = %u\n pref = %u\n service = %s\n uri = %s\n\n",
             (i+1),
             results[i]->domain,
             results[i]->order,
             results[i]->preference,
             results[i]->service,
             results[i]->uri);
      free_enum_record(results[i]);
   }

   printf("Total number of results:%d\n",ret);
   exit(0);
}


