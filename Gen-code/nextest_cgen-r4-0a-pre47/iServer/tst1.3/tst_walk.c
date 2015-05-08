#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include "tst.h"
#include "tst_iterator.h"

tst_iterator *
tst_iterator_init(tst_iterator *tsti, struct tst *tst)
{
	memset(tsti, 0, sizeof(tst_iterator));
	tsti->tst = tst;

	return tsti;
}

// search for key starting from current node down
void *tst_search_for_key(unsigned char key, struct node *current_node)
{
   while (current_node != NULL)
   {
      if(toascii(key) == current_node->value)
      {
         return current_node->middle;
      }
      else if( ((current_node->value == 0) && (toascii(key) < 64)) ||
         ((current_node->value != 0) && (toascii(key) < current_node->value)) )
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

void *tst_searchall(tst_iterator *tsti, unsigned char *key)
{
	struct tst *tst;
	void *data = NULL;

	if (tsti == NULL) return NULL;

	if (key == NULL) return TST_NULL_KEY;

	tst = tsti->tst;	
   
	if (!tsti->init)
	{
		if(toascii(key[0]) == 0)
		{
			return NULL;
		}
   
		if(tst->head[toascii(key[0])] == NULL)
		{
			return NULL;
		}
   
		tsti->current_node = tst->head[toascii(key[0])];
		tsti->key_index = 1;
		tsti->init = 1;

		// We have the first index matched
		// See if you can match a null at or below the current node
		// If the next index is 0, then code below will take care of it.
		if (toascii(key[tsti->key_index]) != 0)
		{
			data = tst_search_for_key(0, tsti->current_node);
		}
	}

   	while (!data && tsti->current_node != NULL)
   	{
    	if (tsti->current_node->value == 0)
      	{
			// pattern in tree has terminated, and none are left. return this one
            data = tsti->current_node->middle;

    		if (toascii(key[tsti->key_index]) == 0)
			{
				tsti->current_node = NULL;
				return data;
			}

			// if key is not finished yet, then we need to find 
			// a new current node. Note that the first "if" check does
			// not apply (below)
			// data must not be re-assigned after this point
      	}

		// set up the iterator, so that when we return,
		// we have something to go on
    	if (toascii(key[tsti->key_index]) == tsti->current_node->value)
      	{
			// We know that neither the pattern nor the key have terminated
            tsti->current_node = tsti->current_node->middle;
            tsti->key_index++;

			// See if you can match a null at or below the current node
			// If the key does not terminate at this point, we want to similate that
			if (toascii(key[tsti->key_index]) != 0)
			{
				data = tst_search_for_key(0, tsti->current_node);
			}

            continue;
      	}
      	else if( ((tsti->current_node->value == 0) && 
				(toascii(key[tsti->key_index]) < 64)) ||
         		((tsti->current_node->value != 0) && (toascii(key[tsti->key_index]) <
         		tsti->current_node->value)) )
      	{
         	tsti->current_node = tsti->current_node->left;
         	continue;
      	}
      	else
      	{
         	tsti->current_node = tsti->current_node->right;
         	continue;
      	}
   	}

   	return data;
}

