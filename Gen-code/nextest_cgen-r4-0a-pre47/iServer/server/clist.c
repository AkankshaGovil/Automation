#include "clist.h"
#include "cache.h"
/*
 * Code based on clist.h macros for cache
 */

void *
list_get(ListEntry *list, int poffset, void *item, int cachecmp)
{
	ListEntry *walker;

	if (!list || !item) return NULL;

	walker = list;

	do
	{
		// If cmp fn is not defined, we make this get first
		if ((cachecmp == -1) || !CachecmpArray[cachecmp] (item, walker, list))
		{
			return walker;
		}
		walker = Listg(walker, poffset)->next;
	} while (walker != list);

	return(NULL);
}

// returns duplicate if one found and NULL if insert was successful
void *
list_insert(ListEntry **list, int poffset, void *item, int cacheinscmp)
{
	ListEntry *walker, *last = NULL;
	int diff;

	if (!list || !item) return NULL;

	// Initialize the element
	ListgInitElem(item, poffset);

	if (!(walker = *list))
	{
		*list = item;
		return NULL;
	}

	do
	{
		if (cacheinscmp == -1)
		{
			// Treat this as an insert in the end
			diff = -1;
		}
		else
		{
			diff = CacheinscmpArray[cacheinscmp] (item, walker, *list);
		}

		if (!diff)
		{
			// duplicate found
			return walker;
		}
		
		if (diff < 0) last = walker;
		else break;

		walker = Listg(walker, poffset)->next;
	} while (walker != *list);

	if (last == NULL)
	{
		// first entry needs to be modified
		ListgInsert(Listg((*list), poffset)->prev, item, poffset);
		*list = item;
	}
	else
	{
		ListgInsert(last, item, poffset);
	}

	return NULL;
}

// returns deleted item if one found and NULL if not found
// note that we cannot ensure that the item belonged to this list
// caller must have sufficient logic to make sure of that
void *
list_delete(ListEntry **list, int poffset, void *item, int cachecmp)
{
	ListEntry *last = NULL;
	int diff;

	if (!list || !*list || !item) return NULL;

	if (ListgIsSingle(item, poffset))
	{
		if (*list == item)
		{
			*list = NULL;
			return item;
		}
		else
	 	{
			// item being deleted is not in this list
			return NULL;
		}
	}
	else
	{
		// Delete item, check to see if it is the first
		if (item == *list)
		{
			// first
			*list = Listg(*list, poffset)->next;
		}

		ListgDelete(item, poffset);
	}

	return item;
}

// resets every element of the list
void *
list_reset(ListEntry **list, int poffset)
{
	ListEntry *walker, *last = NULL;
	int diff;

	if (!list) return NULL;

	if (!(walker = *list))
	{
		return NULL;
	}

	do
	{
		last = walker;
		walker = Listg(walker, poffset)->next;
		ListgInitElem(last, poffset);
	} while (walker != *list);

	*list = NULL;

	return NULL;
}

