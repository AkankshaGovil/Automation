#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include "tst.h"

void *tst_search(unsigned char *key, struct tst *tst)
{
   struct node *current_node;
   int key_index;

   if(key == NULL)
      return TST_NULL_KEY;
   
   if(toascii(key[0]) == 0)
      return NULL;
   
   if(tst->head[toascii(key[0])] == NULL)
      return NULL;
   
   current_node = tst->head[toascii(key[0])];
   key_index = 1;
   
   while (current_node != NULL)
   {
      if(toascii(key[key_index]) == current_node->value)
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
      else if( ((current_node->value == 0) && (toascii(key[key_index]) < 64)) ||
         ((current_node->value != 0) && (toascii(key[key_index]) <
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
}

