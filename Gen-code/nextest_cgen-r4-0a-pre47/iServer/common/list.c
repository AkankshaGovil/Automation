
/*
 * File: list.c
 *
 * Description:
 *	Provides a doubly linked "List" data structure.
 *	The doubly linked List handles opaque data structures.
 */

#include <stdio.h>
#include <memory.h>
#include <strings.h>
#include "list.h"
#include "srvrlog.h"
#include <malloc.h>

List 
listInit (void)
{
	ListHead * lh;

	lh = (ListHead *) malloc (sizeof(ListHead));
	if (!lh)
		return ((List)NULL);

	lh->nitems = 0;
	lh->status  = 0;
	memset (&lh->list, 0, sizeof(ListStruct));

	lh->begin = lh->end = (List)&lh->list;

	lh->list.head  = lh;
	lh->list.item = 0;

	return ((List)&(lh->list));
}

int
listDestroy (List list)
{
	List	end;
	List	prev;
	ListHead * lh;

	if (! list)
		return -1;

	lh = list->head;
	end = lh->end;
	prev = end;

	/* Walk backwards from the end */
	while (end)
	{
		prev = end->prev;

		/* Fix for the hole in the zeroth item */
		if (prev)
		{
			if (end->item)
				free(end->item);

			free (end);
		}
		end = prev;
	}

	/* Free the list head itself */
	free (lh);

	return 0;
}

int
listDestroyNotItem (List list)
{
	List	end;
	List	prev;
	ListHead * lh;

	if (! list)
		return -1;

	lh = list->head;
	end = lh->end;
	prev = end;

	/* Walk backwards from the end */
	while (end)
	{
		prev = end->prev;

		/* Fix for the hole in the zeroth item */
		if (prev)
			free (end);
		end = prev;
	}

	/* Free the list head itself */
	free (lh);

	return 0;
}

/* Note that the following function destroys the
 * items as well.
 */

int
listReset (List list)
{
	List	end;
	List	prev;
	ListHead * lh;

	if (! list)
		return -1;

	lh = list->head;
	end = lh->end;
	prev = end;

	/* Walk backwards from the end */
	while (end)
	{
		prev = end->prev;
		/* Fix for the hole in the zeroth item */
		if (prev)
		{
			if (end->item)
				free(end->item);

			free (end);
		}
		else
		{
			break;
		}

		end = prev;
	}

	lh->nitems = 0;
	lh->begin = lh->end = list;
	lh->list.next = 0;

	return 0;
}

int 
listAddItem (List list, void * item)
{
	ListHead  *lh = list->head;
	List      end;
	ListStruct * new;

	new = (ListStruct *) malloc (sizeof(ListStruct));
	if (!new)
	{
		/* Out of memory */
		return -1;
	}

	/* Zero it out */
	bzero (new, sizeof(ListStruct));

	end = lh->end;

	/* Copy the item in */
	new->item = item;

	new->prev = end;
	end->next =  new;
	new->next = NULL;

	/* Save off the new node */
	lh->end = new;

	/* Add to the count */
	lh->nitems++;
	return 0;
}

int 
listAddItemInFront (List list, void * item)
{
	ListHead  *lh = list->head;
	List      begin;
	ListStruct * new;

	if (lh->nitems == 0)
	{
		// same as adding in the end
		return listAddItem(list, item);
	}

	// We are guaranteed to have at least one element

	new = (ListStruct *) malloc (sizeof(ListStruct));
	if (!new)
	{
		/* Out of memory */
		return -1;
	}

	/* Zero it out */
	bzero (new, sizeof(ListStruct));

	begin = lh->begin->next;

	if (begin == NULL)
	{
		// This is an error
		NETERROR(MINIT,
			("List corrupted nitems=%d, first elmt not assigned\n",
			lh->nitems));	
		return 0;
	}

	/* Copy the item in */
	new->item = item;

	new->prev = lh->begin;
	new->next = begin;
	begin->prev =  new;

	/* Save off the new node */
	lh->begin->next = new;

	/* Add to the count */
	lh->nitems++;
	return 0;
}

/* inserts item in before "cur" list elem */
int 
listInsertItem (List list, void * item, ListStruct *cur)
{
	ListHead  *lh = list->head;
	ListStruct * new;

	new = (ListStruct *) malloc (sizeof(ListStruct));
	if (!new)
	{
		/* Out of memory */
		return -1;
	}

	/* Zero it out */
	bzero (new, sizeof(ListStruct));

	/* Copy the item in */
	new->item = item;

	new->prev = cur->prev;
	new->next = cur;

	cur->prev->next = new;
	cur->prev = new;

	/* Add to the count */
	lh->nitems++;
	return 0;
}

void * 
listMatchItem (List list, 
	       int (*match)(void *item))
{
     ListHead * lh = list->head;
     ListStruct * ptr = lh->begin;
     int	found = 0;
     
     while (ptr)
     {
	  if (match(ptr->item))
	  {
	       found = 1;
	       break;
	  }
	  
	  ptr = ptr->next;
     }
     
     if (found)
	  return (ptr->item);
     else
	  return (NULL);
}

void * 
listSearchItem (List list, 
	       int (*match)(void *item, void *arg), void *arg)
{
     ListHead * lh = list->head;
     ListStruct * ptr = lh->begin;
     int	found = 0;
     
     while (ptr)
     {
	  if (match(ptr->item, arg))
	  {
	       found = 1;
	       break;
	  }
	  
	  ptr = ptr->next;
     }
     
     if (found)
	  return (ptr->item);
     else
	  return (NULL);
}

int 
listDeleteItem (List list, void * item)
{
	ListHead  *lh = list->head;
	List	ptr;
	List	prev;
	List	next;
	ptr = SearchList (list, item);

	if (!ptr)
		return -1;

	if (ptr == lh->begin)
	{
		printf("Attempted delete of list head\n");
		return -1;
	}

	prev = ptr->prev;
	next = ptr->next;

	prev->next = next;
	if (next)
	{
	     next->prev = prev;
	}
	
	if (lh->end == ptr)
	{
	     lh->end = prev;
	}
	
	if (lh->begin == ptr)
	{
	}

	free (ptr);

	lh->nitems --;

	return 0;
}

int 
listDeleteListEntry (List list, List ptr)
/* Item not getting freed */
{
	ListHead  *lh = list->head;
	List	prev;
	List	next;

	if (!ptr)
		return -1;

	prev = ptr->prev;
	next = ptr->next;

	prev->next = next;
	if (next)
	{
	     next->prev = prev;
	}
	
	if (lh->end == ptr)
	{
	     lh->end = prev;
	}
	
	if (lh->begin == ptr)
	{
	}

	free (ptr);

	lh->nitems --;

	return 0;
}


void *
listGetPrevItem(List list, void *item)
{
     List ptr;

     if (item == 0)
     {
	 return 0;
     }

     /* Search the item, return the previous item */
     ptr = SearchList(list, item);

     if (!ptr)
     {
	  return 0;
     }
     
     return ptr->prev->item;
}
     


void *
listGetNextItem(List list, void *item)
{
     List ptr;

     /* Search the item, return the previous item */
     ptr = SearchList(list, item);

     if (!ptr || !ptr->next)
     {
	  return 0;
     }
     
     return ptr->next->item;
}


void *
listGetEndItem (List list)
{
	ListStruct * end = list->head->end;

	if (end)
		return ((void *)end->item);
	else 
		return ((void *) NULL);
}



void *
listGetFirstItem (List list)
{
     ListHead * lh;
     ListStruct * ptr;
     
	 if (list == NULL)
	 {
		return NULL;
	 }

     lh = list->head;
     ptr = lh->begin;
     
     if (ptr->next)
     	return ptr->next->item;
     return 0;
}

int 
listStatus (List list)
{
	ListHead * lh = list->head;

	return (lh->status);
}


int
listItems (List list)
{
	ListHead * lh = list->head;

	return (lh->nitems);
}


ListStruct *
ListGetNext (List list)
{
	return list->next;
}
	
ListStruct *
SearchListforMatch (List list, void * item, 
	int (*match)(const void *, const void *))
{
	ListHead *lh = list->head;
	ListStruct *ptr = lh->begin;
	int	found = 0;

	while (ptr) {
		if (match(ptr->item,item)) {
			found = 1;
			break;
		}

		ptr = ptr->next;
	}

	if (found)
		return (ptr);
	else
		return (NULL);
}

ListStruct *
SearchList (List list, void * item)
{
	ListHead * lh = list->head;
	ListStruct * ptr = lh->begin;
	int	found = 0;

	while (ptr)
	{
		if (ptr->item == item)
		{
			found = 1;
			break;
		}

		ptr = ptr->next;
	}

	if (found)
		return (ptr);
	else
		return (NULL);
}

// items start from 1, 0 is undefined
void *
ListGetItem (List list, int i)
{
	ListHead * lh = list->head;
	ListStruct * ptr = lh->begin;
	int	found = 0, j = 0;

	while (ptr && (j++ != i))
	{
		ptr = ptr->next;
	}

	if (ptr) return (ptr->item);
	else return NULL;
}

void *
listDeleteFirstItem (List list)
{
     ListHead * lh;
     ListStruct * ptr;
	 void 		* item;
     
	 if (list == NULL)
	 {
		return NULL;
	 }

	 if (item = listGetFirstItem (list))
	 {
	 	listDeleteItem (list, item);
	 }

	 return item;
#if 0
     lh = list->head;
     ptr = lh->begin;
     
     if (ptr->next)
	 {
     	item = ptr->next->item;
     	ptr->next->item = NULL;
		listDeleteListEntry (list,ptr->next);
     	return item;
	 }
     return 0;
#endif
}
