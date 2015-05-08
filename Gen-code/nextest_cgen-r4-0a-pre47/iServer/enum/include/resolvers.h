/****************************************************************************
* resolvers.h
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

#ifndef _RESOLVERS_H
#define _RESOLVERS_H

#define MAX_ROOT_DOMAINS  4

extern int errno;

typedef struct naptr_rec {
   char *domain;
   unsigned long ttl;
   unsigned int order;
   unsigned int preference;
   char *flags;
   char *service;
   char *regexp;
   char *replacement;
} NAPTR_record;


typedef struct enum_rec {
   char *domain;
   unsigned int order;
   unsigned int preference;
   char *service;
   char *uri;
} ENUM_record;

int naptr_resolve(char *,char **,NAPTR_record **,int);
void free_naptr_record(NAPTR_record *);

int enum_resolve(char *,char *,char **,ENUM_record **,int);
void free_enum_record(ENUM_record *);

#endif /* _RESOLVERS_H */
