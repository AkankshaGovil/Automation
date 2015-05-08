/******************************************************************************

NOTICE:
This document contains information that is proprietary to RADVISION LTD..
No part of this publication may be reproduced in any form whatsoever without
written prior approval by RADVISION LTD..

RADVISION LTD. reserves the right to revise this publication and make changes
without obligation to notify any person of such revisions or changes.

******************************************************************************/

/*
    LList.h

    Authors:
        Assen Stoyanov (softa@biscom.net)

    Created:
        31 May 2000

    Overview:
        Linked lists (for internal use only!)

    Revised:

    Bugs:

    ToDo:
*/
#ifndef _LList_h_
#define _LList_h_

#ifdef _cplusplus
extern "C" {
#endif /* _cplusplus*/

#define VOLATILE

/* Calculate the address of the base of the structure given its type, and an
 address of a field within the structure.*/
#ifndef CONTAINING_RECORD
#define CONTAINING_RECORD(address, type, field) \
    ((type *)((char*)(address) - (char*)(&((type *)0)->field)))
#endif

/*  Doubly linked list structure.  Can be used as either a list head, or
  as link words.*/

#ifndef UNDER_CE
typedef struct _LIST_ENTRY {
   struct _LIST_ENTRY* VOLATILE Flink;
   struct _LIST_ENTRY* VOLATILE Blink;
} LIST_ENTRY, *PLIST_ENTRY;
#endif

#define LIST LIST_ENTRY

#define InitializeListHead(ListHead) (\
    (ListHead)->Flink = (ListHead)->Blink = (ListHead))

#define IsListEmpty(ListHead) \
    ((ListHead)->Flink == (ListHead))

#define GetLastList(ListHead) \
    (IsListEmpty(ListHead) ? NULL : (ListHead)->Blink)

#define RemoveHeadList(ListHead) \
    (ListHead)->Flink;\
    {RemoveEntryList((ListHead)->Flink)}

#define RemoveEntryList(Entry) {\
    PLIST_ENTRY _EX_Blink;\
    PLIST_ENTRY _EX_Flink;\
    _EX_Flink = (Entry)->Flink;\
    _EX_Blink = (Entry)->Blink;\
    _EX_Blink->Flink = _EX_Flink;\
    _EX_Flink->Blink = _EX_Blink;\
    }

#define InsertTailList(ListHead,Entry) {\
    PLIST_ENTRY _EX_Blink;\
    PLIST_ENTRY _EX_ListHead;\
    _EX_ListHead = (ListHead);\
    _EX_Blink = _EX_ListHead->Blink;\
    (Entry)->Flink = _EX_ListHead;\
    (Entry)->Blink = _EX_Blink;\
    _EX_Blink->Flink = (Entry);\
    _EX_ListHead->Blink = (Entry);\
    }

#define InsertHeadList(ListHead,Entry) {\
    PLIST_ENTRY _EX_Flink;\
    PLIST_ENTRY _EX_ListHead;\
    _EX_ListHead = (ListHead);\
    _EX_Flink = _EX_ListHead->Flink;\
    (Entry)->Flink = _EX_Flink;\
    (Entry)->Blink = _EX_ListHead;\
    _EX_Flink->Blink = (Entry);\
    _EX_ListHead->Flink = (Entry);\
    }

#ifdef _cplusplus
}
#endif /* _cplusplus */

#endif /* _LList_h_*/
