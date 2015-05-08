#include <stdio.h>
#include <stdlib.h>
#include "tst.h"

int tst_grow_node_free_list(struct tst *tst)
{
   struct node *current_node;
   struct node_lines *new_line;
   int i;

   
   if((new_line = (struct node_lines *) malloc(sizeof(struct node_lines))) == NULL)
      return TST_ERROR;
   
// nextone
memset(new_line, 0, sizeof(struct node_lines));

   if((new_line->node_line = (struct node *)
   calloc(tst->node_line_width, sizeof(struct node))) == NULL)
   {
      free(new_line);
      return TST_ERROR;
   }
   else
   {
// nextone
memset(new_line->node_line, 0, tst->node_line_width*sizeof(struct node));

      new_line->next = tst->node_lines;
      tst->node_lines = new_line;
   }
   
   current_node = tst->node_lines->node_line;
   tst->free_list = current_node;
   for (i = 1; i < tst->node_line_width; i++)
   {
      current_node->middle = &(tst->node_lines->node_line[i]);
      current_node = current_node->middle;
   }
   current_node->middle = NULL;
   return 1;
}

