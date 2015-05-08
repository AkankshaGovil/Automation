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

/* This is file rb.c in libavl. */

#if HAVE_CONFIG_H
#include <config.h>
#endif
#if SELF_TEST 
#include <limits.h>
#include <time.h>
#endif
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include "rb.h"

#if !PSPP && !__GCC__
#define inline
#endif

#if __GNUC__ >= 2
#define unused __attribute__ ((unused))
#else
#define unused
#endif

void *SHM_Malloc(size_t);
void SHM_Free(void *);
void *SHM_Realloc(void *, size_t);
void *malloc(size_t);
void free(void *);
void *realloc(void *, size_t);

void *(*_rb_malloc)(size_t) = SHM_Malloc;
void (*_rb_free)(void *) = SHM_Free;
void *(*_rb_realloc)(void *, size_t) = SHM_Realloc;

#define malloc(n)	_rb_malloc(n)
#define free(ptr)	_rb_free(ptr)
#define realloc(ptr, size)	_rb_realloc(ptr, size)

int
rb_init(int mtype)
{
	if (mtype)
	{
		_rb_malloc = malloc;
		_rb_free = free;
		_rb_realloc = realloc;
	}
	else
	{
		_rb_malloc = SHM_Malloc;
		_rb_free = SHM_Free;
		_rb_realloc = SHM_Realloc;
	}
}

#ifdef HAVE_XMALLOC
void *xmalloc (size_t);
#else /* !HAVE_XMALLOC */
/* Allocates SIZE bytes of space using malloc().  Aborts if out of
   memory. */
static void *
xmalloc (size_t size)
{
  void *vp;

  if (size == 0)
    return NULL;
  vp = malloc (size);

  assert (vp != NULL);
  if (vp == NULL)
    {
      fprintf (stderr, "virtual memory exhausted\n");
      exit (EXIT_FAILURE);
    }
  return vp;
}
#endif /* !HAVE_XMALLOC */

/* Creates a red-black tree in arena OWNER (which can be NULL).  The
   arena is owned by the caller, not by the red-black tree.  CMP is a
   order function for the data to be stored in the tree.  PARAM is
   arbitrary data that becomes an argument to the comparison
   function. */
rb_tree *
rb_create (MAYBE_ARENA int cmp, int inscmp, void *param)
{
  rb_tree *tree;

  tree = xmalloc (sizeof (rb_tree));

  tree->root.link[0] = NULL;
  tree->root.link[1] = NULL; 
  tree->cmp = cmp;
  tree->inscmp = inscmp;
  tree->count = 0;
  tree->param = param;

  return tree;
}

/* Destroy tree TREE.  Function FREE_FUNC is called for every node in
   the tree as it is destroyed.  

   No effect if the tree has an arena owner and free_func is NULL.
   The caller owns the arena and must destroy it itself.

   Do not attempt to reuse the tree after it has been freed.  Create a
   new one.  */
void
rb_destroy (rb_tree *tree, avl_node_func free_func)
{
  assert (tree != NULL);
  
  {
    /* Uses Knuth's Algorithm 2.3.1T as modified in exercise 13
       (postorder traversal). */
      
      /* T1. */
    rb_node *an[RB_MAX_HEIGHT];	/* Stack A: nodes. */
    unsigned long ab = 0;		/* Stack A: bits. */
    int ap = 0;			/* Stack A: height. */
    rb_node *p = tree->root.link[0];

    for (;;)
      {
	/* T2. */
	while (p != NULL)
	  {
	    /* T3. */
	    ab &= ~(1ul << ap);
	    an[ap++] = p;
	    p = p->link[0];
	  }

	/* T4. */
	for (;;)
	  {
	    if (ap == 0)
	      goto done;

	    p = an[--ap];
	    if ((ab & (1ul << ap)) == 0)
	      {
		ab |= (1ul << ap++);
		p = p->link[1];
		break;
	      }
      
	    if (free_func)
	      free_func (p->data, tree->param);
	    free (p);
	  }
      }
  }

 done:
  free (tree);
}

/* rb_destroy() with FREE_FUNC hardcoded as free(). */
void
rb_free (rb_tree *tree)
{
  rb_destroy (tree, (avl_node_func) free);
}

/* Return the number of nodes in TREE. */
int
rb_count (const rb_tree *tree)
{
  assert (tree != NULL);
  return tree->count;
}

/* Copy the contents of TREE to a new tree in arena OWNER.  If COPY is
   non-NULL, then each data item is passed to function COPY, and the
   return values are inserted into the new tree; otherwise, the items
   are copied verbatim from the old tree to the new tree.  Returns the
   new tree. */
rb_tree *
rb_copy (MAYBE_ARENA const rb_tree *tree, avl_copy_func copy)
{
  /* This is a combination of Knuth's Algorithm 2.3.1C (copying a
     binary tree) and Algorithm 2.3.1T as modified by exercise 12
     (preorder traversal). */

  rb_tree *new_tree;

  /* PT1. */
  const rb_node *pa[RB_MAX_HEIGHT];	/* Stack PA: nodes. */
  const rb_node **pp = pa;		/* Stack PA: stack pointer. */
  const rb_node *p = &tree->root;
  
  /* QT1. */
  rb_node *qa[RB_MAX_HEIGHT];	/* Stack QA: nodes. */
  rb_node **qp = qa;		/* Stack QA: stack pointer. */
  rb_node *q;
  
  assert (tree != NULL);
  new_tree = rb_create (tree->cmp, tree->inscmp, tree->param);
  new_tree->count = tree->count;
  q = &new_tree->root;

  for (;;)
    {
      /* C4. */
      if (p->link[0] != NULL)
	{
	  rb_node *r = xmalloc (sizeof (rb_node));
	  r->link[0] = r->link[1] = NULL;
	  q->link[0] = r;
	}

      /* C5: Find preorder successors of P and Q.  */
      goto start;
      for (;;)
	{
	  /* PT2. */
	  while (p != NULL)
	    {
	      goto escape;
	    start:
	      /* PT3. */
	      *pp++ = p;
	      *qp++ = q;
	      p = p->link[0];
	      q = q->link[0];
	    }
      
	  /* PT4. */
	  if (pp == pa)
	    {
	      assert (qp == qa);
	      return new_tree;
	    }
	      
	  p = *--pp;
	  q = *--qp;

	  /* PT5. */
	  p = p->link[1];
	  q = q->link[1];
	}
    escape:

      /* C2. */
      if (p->link[1])
	{
	  rb_node *r = xmalloc (sizeof (rb_node));
	  r->link[0] = r->link[1] = NULL;
	  q->link[1] = r;
	}

      /* C3. */
      q->color = p->color;
      if (copy == NULL)
	q->data = p->data;
      else
	q->data = copy (p->data, tree->param);
    }
}

/* Walk tree TREE in inorder, calling WALK_FUNC at each node.  Passes
   PARAM to WALK_FUNC.  */
void
rb_walk (const rb_tree *tree, avl_node_func walk_func, void *param)
{
  /* Uses Knuth's algorithm 2.3.1T (inorder traversal). */
  assert (tree && walk_func);
  
  {
    /* T1. */
    const rb_node *an[RB_MAX_HEIGHT];	/* Stack A: nodes. */
    const rb_node **ap = an;		/* Stack A: stack pointer. */
    const rb_node *p = tree->root.link[0];

    for (;;)
      {
	/* T2. */
	while (p != NULL)
	  {
	    /* T3. */
	    *ap++ = p;
	    p = p->link[0];
	  }
      
	/* T4. */
	if (ap == an)
	  return;
	p = *--ap;

	/* T5. */
	walk_func (p->data, param);
	p = p->link[1];
      }
  }
}

/* Each call to this function for a given TREE and TRAV return the
   next item in the tree in inorder.  Initialize the first element of
   TRAV (init) to 0 before calling the first time.  Returns NULL when
   out of elements.  */
void *
rb_traverse (const rb_tree *tree, rb_traverser *trav)
{
  assert (tree && trav);

  /* Uses Knuth's algorithm 2.3.1T (inorder traversal). */
  if (trav->init == 0)
    {
      /* T1. */
      trav->init = 1;
      trav->nstack = 0;
      trav->p = tree->root.link[0];
    }
  else
    /* T5. */
    trav->p = trav->p->link[1];

  for (;;)
    {
      /* T2. */
      while (trav->p != NULL)
	{
	  /* T3. */
	  trav->stack[trav->nstack++] = trav->p;
	  trav->p = trav->p->link[0];
	}
      
      /* T4. */
      if (trav->nstack == 0)
	{
	  trav->init = 0;
	  return NULL;
	}
      trav->p = trav->stack[--trav->nstack];

      /* T5. */
      return trav->p->data;
    }
}

/* Search TREE for an item matching ITEM.  If found, returns a pointer
   to the address of the item.  If none is found, ITEM is inserted
   into the tree, and a pointer to the address of ITEM is returned.
   In either case, the pointer returned can be changed by the caller,
   or the returned data item can be directly edited, but the key data
   in the item must not be changed. */
void **
rb_probe (rb_tree *tree, void *item)
{
  /* Algorithm based on RB-Insert from section 14.3 of _Introduction
     to Algorithms_, Cormen et al., MIT Press 1990, ISBN
     0-262-03141-8. */

  rb_node *ap[RB_MAX_HEIGHT];		/* Stack A: Nodes. */
  char ad[RB_MAX_HEIGHT];		/* Stack A: Directions. */
  int ak = 1;				/* Stack A: Pointer. */

  rb_node *t, *x, *y, *n;
  
  assert (tree != NULL);
  t = &tree->root;
  x = t->link[0];

  if (x == NULL)
    {
      tree->count++;
      assert (tree->count == 1);
      x = t->link[0] = xmalloc (sizeof (rb_node));
      x->data = item;
      x->link[0] = x->link[1] = NULL;
      x->color = RB_BLACK;
      return &x->data;
    }

  ad[0] = 0;
  ap[0] = &tree->root;

  for (;;)
    {
      int diff = CacheinscmpArray[tree->inscmp] (item, x->data, tree->param);

      if (diff < 0)
	{
	  ap[ak] = x;
	  ad[ak++] = 0;
	  y = x->link[0];
	  if (y == NULL)
	    {
	      n = x = x->link[0] = xmalloc (sizeof (rb_node));
	      break;
	    }
	}
      else if (diff > 0)
	{
	  ap[ak] = x;
	  ad[ak++] = 1;
	  y = x->link[1];
	  if (y == NULL)
	    {
	      n = x = x->link[1] = xmalloc (sizeof (rb_node));
	      break;
	    }
	}
      else
	return &x->data;

      x = y;
    }
  
  tree->count++;
  x->data = item;
  x->link[0] = x->link[1] = NULL;
  x->color = RB_RED;

  for (;;)
    {
      if (ak < 3 || ap[ak - 1]->color != RB_RED)
	break;
      
      if (ad[ak - 2] == 0)
	{
	  y = ap[ak - 2]->link[1];
	  if (y != NULL && y->color == RB_RED)
	    {
	      /* Case 1. */
	      ap[ak - 1]->color = y->color = RB_BLACK;
	      ap[ak - 2]->color = RB_RED;
	      ak -= 2;
	    }
	  else
	    {
	      if (ad[ak - 1] == 1)
		{
		  /* Case 2. */
		  x = ap[ak - 1];
		  y = x->link[1];
		  x->link[1] = y->link[0];
		  y->link[0] = x;
		  ap[ak - 2]->link[0] = y;
		}
	      else
		y = ap[ak - 1];

	      /* Case 3. */
	      x = ap[ak - 2];
	      x->color = RB_RED;
	      y->color = RB_BLACK;

	      x->link[0] = y->link[1];
	      y->link[1] = x;
	      ap[ak - 3]->link[(int) ad[ak - 3]] = y;

	      break;
	    }
	}
      else
	{
	  y = ap[ak - 2]->link[0];
	  if (y != NULL && y->color == RB_RED)
	    {
	      /* Case 1. */
	      ap[ak - 1]->color = y->color = RB_BLACK;
	      ap[ak - 2]->color = RB_RED;
	      ak -= 2;
	    }
	  else
	    {
	      if (ad[ak - 1] == 0)
		{
		  /* Case 2. */
		  x = ap[ak - 1];
		  y = x->link[0];
		  x->link[0] = y->link[1];
		  y->link[1] = x;
		  ap[ak - 2]->link[1] = y;
		}
	      else
		y = ap[ak - 1];

	      /* Case 3. */
	      x = ap[ak - 2];
	      x->color = RB_RED;
	      y->color = RB_BLACK;

	      x->link[1] = y->link[0];
	      y->link[0] = x;
	      ap[ak - 3]->link[(int) ad[ak - 3]] = y;
	      break;
	    }
	}
    }

  tree->root.link[0]->color = RB_BLACK;
  
  return &n->data;
}
  
/* Search TREE for an item matching ITEM, and return it if found. */
void *
rb_find (const rb_tree *tree, const void *item)
{
  const rb_node *p;

  assert (tree != NULL);
  for (p = tree->root.link[0]; p; )
    {
      int diff = CachecmpArray[tree->cmp] (item, p->data, tree->param);

      if (diff < 0)
	p = p->link[0];
      else if (diff > 0)
	p = p->link[1];
      else
	return p->data;
    }

  return NULL;
}

/* Search TREE for an item close to the value of ITEM, and return it.
   This function will return a null pointer only if TREE is empty. */
void *
rb_find_close (const rb_tree *tree, const void *item)
{
  const rb_node *p;

  assert (tree != NULL);
  p = tree->root.link[0];
  if (p == NULL)
    return NULL;
  
  for (;;)
    {
      int diff = CachecmpArray[tree->cmp] (item, p->data, tree->param);
      int t;

      if (diff < 0)
	t = 0;
      else if (diff > 0)
	t = 1;
      else
	return p->data;

      if (p->link[t])
	p = p->link[t];
      else
	return p->data;
    }
}

/* Searches red-black tree TREE for an item matching ITEM.  If found,
   the item is removed from the tree and the actual item found is
   returned to the caller.  If no item matching ITEM exists in the
   tree, returns NULL. */
void *
rb_delete (rb_tree *tree, const void *item)
{
  /* Algorithm based on RB-Delete and RB-Delete-Fixup from section
     14.4 of _Introduction to Algorithms_, Cormen et al., MIT Press
     1990, ISBN 0-262-03141-8. */

  rb_node *pa[RB_MAX_HEIGHT];		/* Stack P: Nodes. */
  char a[RB_MAX_HEIGHT];		/* Stack P: Bits. */
  int k = 1;				/* Stack P: Pointer. */
  
  rb_node *w, *x, *y, *z;

  assert (tree != NULL);

  a[0] = 0;
  pa[0] = &tree->root;
  z = tree->root.link[0];
  for (;;)
    {
      int diff;

      if (z == NULL)
	return NULL;

      diff = CachecmpArray[tree->cmp] (item, z->data, tree->param);
      if (diff == 0)
	break;

      pa[k] = z;
      if (diff < 0)
	{
	  z = z->link[0];
	  a[k] = 0;
	}
      else if (diff > 0)
	{
	  z = z->link[1];
	  a[k] = 1;
	}
      k++;
    }
  tree->count--;
  
  item = z->data;

  /* RB-Delete: Line 1. */
  if (z->link[0] == NULL || z->link[1] == NULL)
    {
      /* Line 2. */
      y = z;

      /* Lines 4-6. */
      if (y->link[0] != NULL)
	x = y->link[0];
      else
	x = y->link[1];

      pa[k - 1]->link[(int) a[k - 1]] = x;
    }
  else
    {
      pa[k] = z;
      a[k++] = 1;

      /* Line 3. */
      y = z->link[1];
      while (y->link[0])
	{
	  pa[k] = y;
	  a[k++] = 0;
	  y = y->link[0];
	}

      /* Lines 4-6. */
      x = y->link[1];

      /* Lines 13-15. */
      z->data = y->data;
      pa[k - 1]->link[(int) a[k - 1]] = x;
    }

  /* Line 16. */
  if (y->color == RB_RED)
    {
      free (y);
      return (void *) item;
    }

  free (y);

  /* Numbers below are line numbers from RB-Delete-Fixup. */
  while (k > 1 && (x == NULL || x->color == RB_BLACK))			/* 1 */
    {
      if (a[k - 1] == 0)						/* 2 */
	{
	  w = pa[k - 1]->link[1];					/* 3 */

	  if (w->color == RB_RED)					/* 4 */
	    {
	      /* Case 1. */
	      w->color = RB_BLACK;					/* 5 */
	      pa[k - 1]->color = RB_RED;				/* 6 */

	      pa[k - 1]->link[1] = w->link[0];				/* 7 */
	      w->link[0] = pa[k - 1];
	      pa[k - 2]->link[(int) a[k - 2]] = w;

	      pa[k] = pa[k - 1];
	      a[k] = 0;
	      pa[k - 1] = w;
	      k++;

	      w = pa[k - 1]->link[1];					/* 8 */
	    }

	  if ((w->link[0] == NULL || w->link[0]->color == RB_BLACK)	/* 9 */
	      && (w->link[1] == NULL || w->link[1]->color == RB_BLACK))
	    {
	      /* Case 2. */
	      w->color = RB_RED;				       /* 10 */

	      x = pa[k - 1];					       /* 11 */
	      k--;
	    }
	  else
	    {
	      if (w->link[1] == NULL || w->link[1]->color == RB_BLACK) /* 12 */
		{
		  /* Case 3. */
		  w->link[0]->color = RB_BLACK;			       /* 13 */
		  w->color = RB_RED;				       /* 14 */

		  y = w->link[0];				       /* 15 */
		  w->link[0] = y->link[1];
		  y->link[1] = w;

		  w = pa[k - 1]->link[1] = y;			       /* 16 */
		}

	      /* Case 4. */
	      w->color = pa[k - 1]->color;			       /* 17 */
	      pa[k - 1]->color = RB_BLACK;			       /* 18 */
	      w->link[1]->color = RB_BLACK;			       /* 19 */

	      pa[k - 1]->link[1] = w->link[0];			       /* 20 */
	      w->link[0] = pa[k - 1];
	      pa[k - 2]->link[(int) a[k - 2]] = w;

	      x = tree->root.link[0];				       /* 21 */
	      break;
	    }
	}
      else
	{
	  w = pa[k - 1]->link[0];
	  if (w->color == RB_RED)
	    {
	      /* Case 1. */
	      w->color = RB_BLACK;
	      pa[k - 1]->color = RB_RED;

	      pa[k - 1]->link[0] = w->link[1];
	      w->link[1] = pa[k - 1];
	      pa[k - 2]->link[(int) a[k - 2]] = w;

	      pa[k] = pa[k - 1];
	      a[k] = 1;
	      pa[k - 1] = w;
	      k++;

	      w = pa[k - 1]->link[0];
	    }

	  if ((w->link[0] == NULL || w->link[0]->color == RB_BLACK)
	      && (w->link[1] == NULL || w->link[1]->color == RB_BLACK))
	    {
	      /* Case 2. */
	      w->color = RB_RED;
	      x = pa[k - 1];
	      k--;
	    }
	  else
	    {
	      if (w->link[0] == NULL || w->link[0]->color == RB_BLACK)
		{
		  /* Case 3. */
		  w->link[1]->color = RB_BLACK;
		  w->color = RB_RED;

		  y = w->link[1];
		  w->link[1] = y->link[0];
		  y->link[0] = w;

		  w = pa[k - 1]->link[0] = y;
		}
	      
	      /* Case 4. */
	      w->color = pa[k - 1]->color;
	      pa[k - 1]->color = RB_BLACK;
	      w->link[0]->color = RB_BLACK;

	      pa[k - 1]->link[0] = w->link[1];
	      w->link[1] = pa[k - 1];
	      pa[k - 2]->link[(int) a[k - 2]] = w;

	      x = tree->root.link[0];
	      break;
	    }
	}
    }

  if (x != NULL)
    x->color = RB_BLACK;					       /* 23 */
  
  return (void *) item;
}

/* Inserts ITEM into TREE.  Returns NULL if the item was inserted,
   otherwise a pointer to the duplicate item. */
void *
rb_insert (rb_tree *tree, void *item)
{
  void **p;
  
  assert (tree != NULL);
  
  p = rb_probe (tree, item);
  return (*p == item) ? NULL : *p;
}

/* If ITEM does not exist in TREE, inserts it and returns NULL.  If a
   matching item does exist, it is replaced by ITEM and the item
   replaced is returned.  The caller is responsible for freeing the
   item returned. */
void *
rb_replace (rb_tree *tree, void *item)
{
  void **p;

  assert (tree != NULL);
  
  p = rb_probe (tree, item);
  if (*p == item)
    return NULL;
  else
    {
      void *r = *p;
      *p = item;
      return r;
    }
}

/* Delete ITEM from TREE when you know that ITEM must be in TREE.  For
   debugging purposes. */
void *
(rb_force_delete) (rb_tree *tree, void *item)
{
  void *found = rb_delete (tree, item);
  assert (found != NULL);
  return found;
}

#if SELF_TEST

/* Used to flag delayed aborting. */
int done = 0;

/* Print the structure of node NODE of a red-black tree, which is
   LEVEL levels from the top of the tree.  Uses different delimiters
   to visually distinguish levels. */
void
print_structure (rb_node *node, int level)
{
  char lc[] = "([{<`/";
  char rc[] = ")]}>'\\";

  assert (level <= 10);
  
  if (node == NULL)
    {
      printf (" nil");
      fflush (stdout);
      return;
    }
  printf (" %c%d%c",
	  lc[level % 6], (int) node->data,
	  node->color == RB_BLACK ? 'b' : 'r');
  fflush (stdout);
  if (node->link[0] || node->link[1])
    print_structure (node->link[0], level + 1);
  if (node->link[1])
    print_structure (node->link[1], level + 1);
  printf ("%c", rc[level % 6]);
  fflush (stdout);
}

/* Compare two integers A and B and return a strcmp()-type result. */
int
compare_ints (const void *a, const void *b, void *param unused)
{
  return ((int) a) - ((int) b);
}

/* Print the value of integer A. */
void
print_int (void *a, void *param unused)
{
  printf (" %d", (int) a);
}

/* Linearly print contents of TREE. */
void
print_contents (rb_tree *tree)
{
  rb_walk (tree, print_int, NULL);
  printf ("\n");
}

/* Examine NODE in a red-black tree.  *COUNT is increased by the
   number of nodes in the tree, including the current one.  Returns
   the number of black nodes (including this node) in a path from this
   node to any leaf. */
int
recurse_tree (rb_node *node, int *count, int ge, int le)
{
  if (node) 
    {
      const int d = (int) node->data;
      int nl = 1;
      int nr = 1;
      
      (*count)++;

      if (!(d >= ge) || !(d <= le))
	{
	  printf (" Node %d is out of order in the tree.\n", d);
	  done = 1;
	}

      if (node->link[0])
	nl = recurse_tree (node->link[0], count, ge, d - 1);
      if (node->link[1])
	nr = recurse_tree (node->link[1], count, d + 1, le);
	  
      if (node->color != RB_RED && node->color != RB_BLACK)
	{
	  printf (" Node %d is neither red nor black (%d).\n", d, node->color);
	  done = 1;
	}

      if (node->color == RB_RED
	  && node->link[0] && node->link[0]->color == RB_RED)
	{
	  printf (" Red node %d has red left child %d\n",
		  d, (int) node->link[0]->data);
	  done = 1;
	}
      
      if (node->color == RB_RED
	  && node->link[1] && node->link[1]->color == RB_RED)
	{
	  printf (" Red node %d has red right child %d\n",
		  d, (int) node->link[1]->data);
	  done = 1;
	}
      
      if (nl != nr)
	{
	  printf (" Node %d has two different black-heights: left bh=%d, "
		  "right bh=%d\n", d, nl, nr);
	  done = 1;
	}

      return (node->color == RB_BLACK) + nl;
    }
  else return 1;
}

/* Check that everything about TREE is kosher. */
void
verify_tree (rb_tree *tree)
{
  int count = 0;
  recurse_tree (tree->root.link[0], &count, INT_MIN, INT_MAX);
  if (count != tree->count)
    {
      printf (" Tree has %d nodes, but tree count is %d.\n",
	      count, tree->count);
      done = 1;
    }
  if (done)
    abort ();
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

/* Compares red-black trees rooted at A and B, making sure that they
   are identical. */
void
compare_trees (rb_node *a, rb_node *b)
{
  if (a == NULL || b == NULL)
    {
      assert (a == NULL && b == NULL);
      return;
    }
  if (a->data != b->data || a->color != b->color
      || ((a->link[0] != NULL) ^ (b->link[0] != NULL))
      || ((a->link[1] != NULL) ^ (b->link[1] != NULL)))
    {
      printf (" Copied nodes differ: %d b=%d a->color=%d b->color=%d a:",
	      (int) a->data, (int) b->data, a->color, b->color);
      if (a->link[0])
	printf ("l");
      if (a->link[1])
	printf ("r");
      printf (" b:");
      if (b->link[0])
	printf ("l");
      if (b->link[1])
	printf ("r");
      printf ("\n");
      abort ();
    }
  if (a->link[0] != NULL)
    compare_trees (a->link[0], b->link[0]);
  if (a->link[1] != NULL)
    compare_trees (a->link[1], b->link[1]);
}

/* Simple stress test procedure for the red-black tree routines.  Does
   the following:

   * Generate a random number seed.  By default this is generated from
   the current time.  You can also pass a seed value on the command
   line if you want to test the same case.  The seed value is
   displayed.

   * Create a tree and insert the integers from 0 up to TREE_SIZE - 1
   into it, in random order.  Verify the tree structure after each
   insertion.
   
   * Remove each integer from the tree, in a different random order.
   After each deletion, verify the tree structure; also, make a copy
   of the tree into a new tree, verify the copy and compare it to the
   original, then destroy the copy.

   * Destroy the tree, increment the random seed value, and start over.

   If you make any modifications to the red-black tree routines, then
   you might want to insert some calls to print_structure() at
   strategic places in order to be able to see what's really going on.
   Also, memory debuggers like Checker or Purify are very handy. */
#define TREE_SIZE 16
#define N_ITERATIONS 1024
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

  fputs ("Testing rb...\n", stdout);
  
  for (iteration = 1; iteration <= N_ITERATIONS; iteration++)
    {
      rb_tree *tree;
      int i;
      
      printf ("Iteration %4d/%4d: seed=%5d", iteration, N_ITERATIONS, seed);
      fflush (stdout);
      
      srand (seed++);

      for (i = 0; i < TREE_SIZE; i++)
	array[i] = i;
      shuffle (array, TREE_SIZE);
      
      tree = rb_create (compare_ints, NULL);
      for (i = 0; i < TREE_SIZE; i++)
	rb_force_insert (tree, (void *) (array[i]));
      verify_tree (tree);

      shuffle (array, TREE_SIZE);
      for (i = 0; i < TREE_SIZE; i++)
	{
	  rb_tree *copy;

	  rb_delete (tree, (void *) (array[i]));
	  verify_tree (tree);

	  copy = rb_copy (tree, NULL);
	  verify_tree (copy);
	  compare_trees (tree->root.link[0], copy->root.link[0]);
	  rb_destroy (copy, NULL);

	  if (i % 128 == 0)
	    {
	      putchar ('.');
	      fflush (stdout);
	    }
	}
      fputs (" good.\n", stdout);

      rb_destroy (tree, NULL);
    }
  
  return 0;
}
#endif /* SELF_TEST */

/*
  Local variables:
  compile-command: "gcc -DSELF_TEST=1 -W -Wall -I. -o ./rb-test rb.c"
  End:
*/
