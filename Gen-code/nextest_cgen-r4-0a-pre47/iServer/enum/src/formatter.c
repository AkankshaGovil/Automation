/****************************************************************************
* formatter.c
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
#include "formatter.h"


/**
 * Normalizes a given telephone number as specified in SIP rfc2543
 * to plain digit string.
 * Returns NULL if any illegal characters are encountered.
 *  Illegal characters are any characters other than digits, '.',
 *  '-', '+', '(', or ')'.
 */
char *reformat_e164(char *e164) {
   size_t e164_len=strlen(e164);
   char *ret_p=malloc(e164_len+1);
   char *new_p=ret_p;
   char *curr_p=e164;
   char *end_p;

   if(e164==NULL || ret_p==NULL) return NULL;
   end_p=e164+e164_len;

   while(curr_p<end_p && *curr_p!='\0') {
      if(isdigit((int)*curr_p))
         *new_p++=*curr_p;
      else
         if(strpbrk(curr_p,"+-().")!=curr_p) return NULL;
      curr_p++;
   }
   *new_p='\0';
   return ret_p;
}

/**
 * format_e164_fqd()
 * Create a fully-qualified domain name given an E.164 number and
 * domain name.
 * 
 */
char *format_e164_fqd(char *prefix, /* prefix string (ie. "ldap.tcp" or NULL)*/
                      char *e164,   /* E.164 string (ie. "5551212")   */
                      char *domain) /* domain string (ie. "tel")      */
{
   int prefix_len, e164_len;
   size_t return_len;
   char *curr_p, *ret_p, *new_p;

   if(e164==NULL || domain==NULL) return NULL;

   e164_len=strlen(e164);
   prefix_len=(prefix==NULL) ? 0 : strlen(prefix);

   /* Allocate memory for return value. */
   return_len=(e164_len<<1)+prefix_len+strlen(domain)+1;
   if((ret_p=(char *)malloc(return_len))==NULL) return NULL;
   memset(ret_p,0,return_len);

   /* Copy prefix string, append '.' if required. */
   new_p=ret_p+prefix_len;  /* position after end of string */
   if(prefix_len!=0) {
      strncpy(ret_p,prefix,prefix_len);
      if(*(new_p-1)!='.') *new_p++='.';
   }

   /* Reverse E.164 string, put '.' between each digit */
   for(curr_p=e164+e164_len-1; curr_p>=e164; --curr_p,++new_p){
      *new_p=*curr_p;
      *(++new_p)='.';
   }
   strcpy(new_p,domain);
   return ret_p;
}
