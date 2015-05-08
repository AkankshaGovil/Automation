/*:file:version:date: "%n    V.%v;  %f"
 * "TAVLINIT.C    V.12;  27-Apr-91,12:07:04"
 *
 *  Purpose: Initialize threaded AVL tree. Must be called before tree
 *           can be used.
 *
 *  Released to the PUBLIC DOMAIN
 *
 *               author:    Bert C. Hughes
 *                          200 N.Saratoga
 *                          St.Paul, MN 55104
 *                          Compuserve 71211,577
 */

#include "tavlpriv.h"

TAVL_treeptr tavl_init(                         /* user supplied functions: */
                     int (*compare)(const void *, const void *,void *),/* compares identifiers */
                     int (*insCompare)(const void *,const void *,void *),    /* returns for comparison while inserting*/
                    void *(*make_item)(const void *), /* create copy of item */
                    void (*free_item)(void *),      /* frees node's data */
                    void *(*copy_item)(void *, void *),
                    void *(*alloc)(size_t),
                    void (*dealloc)(void *),
		    void *cmpParam
                    )
{
    TAVL_treeptr tree = (*alloc)(sizeof(TAVL_TREE));

    if (tree)   {
        if ((tree->head = (*alloc)(sizeof(TAVL_NODE))) != NULL) {
            tree->cmp = compare;
            tree->Inscompare = insCompare;
            tree->make_item = make_item;
            tree->free_item = free_item;
            tree->copy_item = copy_item;
            tree->alloc = alloc;
            tree->dealloc = dealloc;
            tree->head->bf = 0;
            tree->head->Lbit = THREAD;
            tree->head->Rbit = LINK;
            tree->head->dataptr = NULL;
            tree->head->Lptr = tree->head;
            tree->head->Rptr = tree->head;
	    tree->cmpParam = cmpParam;
        }
        else {
            (*dealloc)(tree);
            tree = NULL;
        }
    }
    return tree;
}
