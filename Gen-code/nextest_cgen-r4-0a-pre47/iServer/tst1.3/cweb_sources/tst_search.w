@*Searching for keys with |tst_search()|.
@(tst_search.c@>=
#include "tst.h"
#include <stdio.h>
#include <stdlib.h>

void *tst_search(unsigned char *key, struct tst *tst)
{
   struct node *current_node;
   int key_index;

   @<Fail if |key| is NULL@>
   @<Return NULL if head is NULL@>
   @<Initialize |current_node|, |key_index|, start search loop and return NULL on failure@>
}

@
Here we check for the NULL key, which is not allowed.
@<Fail if |key| is NULL@>=
if(key[0] == 0)
   return NULL;

@
Here we simply check the head node to see if it is NULL. If it is, then
we know that the key cannot exist in the tree so we return NULL to
indicate failure.
@<Return NULL if head is NULL@>=
if(tst->head[(int)key[0]] == NULL)
   return NULL;

@
Here we set |current_node| node to the head node and set our index to
1. The loop runs until we hit a NULL node, in which case we return NULL
to indicate failure, otherwise, we return the data stored in the
terminating node.
@<Initialize |current_node|, |key_index|, start search loop and return NULL on failure@>=
current_node = tst->head[(int)key[0]];
key_index = 1;

while (current_node != NULL)
{
   if(key[key_index] == current_node->value)
   {
      if(current_node->value == 0)
         return current_node->middle;
      else
      {
         current_node = current_node->middle;
         key_index++;
         continue;
      }
   }
   else if( ((current_node->value == 0) && (key[key_index] < 64)) ||
      ((current_node->value != 0) && (key[key_index] <
      current_node->value)) )
   {
      current_node = current_node->left;
      continue;
   }
   else
   {
      current_node = current_node->right;
      continue;
   }
}
return NULL;
