#include <stdio.h>
#include <stdlib.h>
#include "tst.h"

void *(*_tst_malloc)(size_t) = SHM_Malloc;
void (*_tst_free)(void *) = SHM_Free;
void *(*_tst_realloc)(void *, size_t) = SHM_Realloc;

#ifdef _TST_PRIV_
int
tst_mem_init(int mtype)
{
	if (mtype)
	{
		_tst_malloc = malloc;
		_tst_free = free;
		_tst_realloc = realloc;
	}
	else
	{
		_tst_malloc = SHM_Malloc;
		_tst_free = SHM_Free;
		_tst_realloc = SHM_Realloc;
	}
}
#endif

struct tst *tst_init(int width)
{
   struct tst *tst;
   struct node *current_node;
   int i;

if((tst = (struct tst *) calloc(1, sizeof(struct tst))) == NULL)
   return NULL;

// nextone
memset(tst, 0, sizeof(struct tst));

if ((tst->node_lines = (struct node_lines *) calloc(1, sizeof(struct node_lines))) == NULL)
{
   free(tst);
   return NULL;
}

// nextone
memset(tst->node_lines, 0, sizeof(struct node_lines));

tst->node_line_width = width;
tst->node_lines->next = NULL;
if ((tst->node_lines->node_line = (struct node *) calloc(width, sizeof(struct node))) == NULL)
{
   free(tst->node_lines);
   free(tst);
   return NULL;
}

// nextone
memset(tst->node_lines->node_line, 0, width*sizeof(struct node));

current_node = tst->node_lines->node_line;
tst->free_list = current_node;
for (i = 1; i < width; i++)
{
   current_node->middle = &(tst->node_lines->node_line[i]);
   current_node = current_node->middle;
}
current_node->middle = NULL;
return tst;
}

