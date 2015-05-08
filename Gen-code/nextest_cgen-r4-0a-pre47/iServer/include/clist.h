#ifndef _clist_h_
#define _clist_h_

typedef struct _list {
     struct _list *prev, *next;
} ListEntry;

#define List(elem) ((ListEntry *)(elem))
#define Listg(elem, offt) ((ListEntry *)(((char *)(elem))+(offt)))

/* list insert and delete macros */
// These macros assume that the prev and next pointers are
// on the top of the list and elem structures
#define ListInsert(list, elem) do { register ListEntry *rlist = (ListEntry *)(list), *relem = (ListEntry *)(elem); (((rlist)->next = (((relem)->next = (rlist)->next)->prev = relem))->prev = rlist); } while (0)

#define ListDelete(elem) do { register ListEntry *relem = (ListEntry *)(elem); ((((relem)->prev)->next = (relem)->next)->prev = (relem)->prev); } while (0)

#define ListInitElem(elem) do { register ListEntry *relem = (ListEntry *)(elem); relem->next = relem->prev = relem; } while (0)

#define ListIsSingle(elem) ((((ListEntry *)(elem))->prev == (ListEntry *)(elem)) && (((ListEntry *)(elem))->next == (ListEntry *)(elem)))

// Generic macros, which can be used when prev and next are not the first rntries in
// the elem structure
#define ListgInsert(list, elem, offt) do { register ListEntry *rlist = Listg(list, offt), *relem = Listg(elem, offt), *rlisto = (ListEntry *)(list), *relemo = (ListEntry *)(elem); (Listg(((rlist)->next = (Listg(((relem)->next = (rlist)->next), offt)->prev = List(relemo))), offt)->prev = List(rlisto)); } while (0)

#define ListgDelete(elem, offt) do { register ListEntry *relem = Listg(elem, offt); (Listg((Listg((relem)->prev, offt)->next = (relem)->next), offt)->prev = (relem)->prev); } while (0)

#define ListgInitElem(elem, offt) do { register ListEntry *relem = Listg(elem, offt), *relemo = (ListEntry *)(elem); relem->next = relem->prev = List(relemo); } while (0)
#define ListgResetElem(elem, offt) do { register ListEntry *relem = Listg(elem, offt); relem->next = relem->prev = NULL; } while (0)

#define ListgIsSingle(elem, offt) ((Listg(elem, offt)->prev == (ListEntry *)(elem)) && (Listg(elem, offt)->next == (ListEntry *)(elem)))

/**************************** CLIST cache API ******************/

void *list_get(ListEntry *list, int poffset, void *item, int cachecmp);

void *list_insert(ListEntry **list, int poffset, void *item, int cacheinscmp);

void *list_delete(ListEntry **list, int poffset, void *item, int cachecmp);

void *list_reset(ListEntry **list, int poffset);

#endif /* _clist_h_ */
