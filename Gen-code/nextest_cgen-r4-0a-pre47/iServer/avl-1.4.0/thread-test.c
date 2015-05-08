/* libavl - manipulates AVL trees.
   Copyright (C) 1998, 1999 Free Software Foundation, Inc.

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public License as
   published by the Free Software Foundation; either version 2 of the
   License, or (at your option) any later version.

   This program is distributed in the hope that it will be useful, but
   WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA
   02111-1307, USA.

   The author may be contacted at <pfaffben@pilot.msu.edu> on the
   Internet, or as Ben Pfaff, 12167 Airport Rd, DeWitt MI 48820, USA
   through more mundane means. */

/* This is file thread-test.c in libavl. */

#if HAVE_CONFIG_H
#include <config.h>
#endif
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "avl.h"
#include "avlt.h"
#include "avltr.h"

#if __GNUC__ >= 2
#define unused __attribute__ ((unused))
#else
#define unused
#endif

/* Compare two integers A and B and return a strcmp()-type result. */
int
compare_ints (const void *a, const void *b, void unused *param)
{
  return ((int) a) - ((int) b);
}

/* Arrange the N elements of ARRAY in random order. */
void
shuffle (int *array, int n)
{
  int i;
  
  for (i = 0; i < n; i++)
    {
      int j = i + rand () % (n - i);
      int t = array[j];
      array[j] = array[i];
      array[i] = t;
    }
}


/* Simple stress test procedure for the AVL tree threading/unthreading
   routines. */
#define TREE_SIZE 1024
#define N_ITERATIONS 16
int
main (int argc, char **argv)
{
  int array[TREE_SIZE];
  int seed;
  int iteration;
  
  if (argc == 2)
    seed = atoi (argv[1]);
  else
    seed = time (0) * 257 % 32768;

  fputs ("Testing threading and unthreading...\n", stdout);
  for (iteration = 1; iteration <= N_ITERATIONS; iteration++)
    {
      avl_tree *tree;
      avl_traverser trav = AVL_TRAVERSER_INIT;
      void **nodep = NULL;
      void *node;
      int i;
      
      printf ("Iteration %4d/%4d: seed=%5d", iteration, N_ITERATIONS, seed);
      fflush (stdout);
      
      srand (seed++);
      
      for (i = 0; i < TREE_SIZE; i++)
	array[i] = i + 1;
      shuffle (array, TREE_SIZE);
      
      tree = avl_create (compare_ints, NULL);
      for (i = 0; i < TREE_SIZE; i++)
	avl_force_insert (tree, (void *) (array[i]));

      shuffle (array, TREE_SIZE);
      for (i = 0; i < TREE_SIZE; i++)
	{
	  avlt_tree *t;
	  avltr_tree *tr;

	  avl_delete (tree, (void *) (array[i]));
      
	  while ((node = avl_traverse (tree, &trav)) != NULL);
      
	  t = avlt_thread (tree);
	  while ((nodep = avlt_next (t, nodep)) != NULL);
	  while ((nodep = avlt_prev (t, nodep)) != NULL);
	  avlt_unthread (t);

	  tr = avltr_thread (tree);
	  while ((nodep = avltr_next (tr, nodep)) != NULL);
	  avltr_unthread (tr);
	  
	  while ((node = avl_traverse (tree, &trav)) != NULL);

	  if (i % 128 == 0)
	    {
	      putchar ('.');
	      fflush (stdout);
	    }
	}
      fputs (" good.\n", stdout);
      
      avl_destroy (tree, NULL);
    }
  
  return 0;
}

/*
  Local variables:
  compile-command: "gcc -W -Wall -I. -o ./thread-test thread-test.c avl.c avlt.c avltr.c"
  End:
*/
