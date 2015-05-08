
#ifndef _list_h_
#define _list_h_


typedef struct _list_
{
	void 		*  item;
	struct _list_ 	* next;
	struct _list_ 	* prev;
	struct _listhead_ * head;
} ListStruct, *List;


typedef struct _listhead_
{
	int	nitems;
	int	status;
	ListStruct	list;
	ListStruct	* begin;	/* first elem */
	ListStruct	* end;		/* last one */
} ListHead;


List listInit (void);
List listDup (List, size_t);
int listDestroy (List list);
int listDestroyNotItem (List list);
int listReset (List list);
int listAddItem (List list, void * item);
int listDeleteItem (List list, void * item);
int listStatus (List list);
int listItems (List list);
void * listGetEndItem (List list);
void * listGetFirstItem (List list);
void * listGetPrevItem (List list, void *item);
void * listGetNextItem (List list, void *item);
void * listMatchItem (List list, 
	       int (*match)(void *item));
ListStruct * SearchList (List list, void * item);
ListStruct * SearchListforMatch (List list, void * item, 
			int (*match)(const void *, const void *));
ListStruct * ListGetNext (List list);
int listApply(List list, int (*fn)(void *arg, void *item), void *arg);
void * listDeleteFirstItem (List list);
void * ListGetItem (List list, int i);
void * listSearchItem (List list, 
	       int (*match)(void *item, void *arg), void *arg);

extern int listAddItemInFront (List list, void *item);
int listDeleteListEntry (List list, List ptr);
	


#endif /* _list_h_ */
