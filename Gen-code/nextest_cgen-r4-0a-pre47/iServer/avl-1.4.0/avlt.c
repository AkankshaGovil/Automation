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

/* This is file avlt.c in libavl. */

#if HAVE_CONFIG_H
#include <config.h>
#endif
#if SELF_TEST 
#include <limits.h>
#include <time.h>
#endif
#include <stdio.h>
#include <stddef.h>
#include <stdlib.h>
#include <assert.h>
#include "avlt.h"

/* Tag types. */
#define PLUS +1
#define MINUS -1

#if !__GCC__ && !defined (inline)
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

void *(*_avlt_malloc)(size_t) = SHM_Malloc;
void (*_avlt_free)(void *) = SHM_Free;
void *(*_avlt_realloc)(void *, size_t) = SHM_Realloc;

#define malloc(n)	_avlt_malloc(n)
#define free(ptr)	_avlt_free(ptr)
#define realloc(ptr, size)	_avlt_realloc(ptr, size)

int
avlt_init(int mtype)
{
	if (mtype)
	{
		_avlt_malloc = malloc;
		_avlt_free = free;
		_avlt_realloc = realloc;
	}
	else
	{
		_avlt_malloc = SHM_Malloc;
		_avlt_free = SHM_Free;
		_avlt_realloc = SHM_Realloc;
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

/* Creates an AVL tree in arena OWNER (which can be NULL).  The arena
   is owned by the caller, not by the AVL tree.  CMP is a order
   function for the data to be stored in the tree.  PARAM is arbitrary
   data that becomes an argument to the comparison function. */
avlt_tree *
avlt_create (int cmp, int inscmp, void *param)
{
  avlt_tree *tree;

  tree = xmalloc (sizeof (avlt_tree));

  tree->root.link[0] = tree->root.link[1] = &tree->root;
  tree->root.tag[0] = MINUS;
  tree->root.tag[1] = PLUS;
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
avlt_destroy (avlt_tree *tree, avl_node_func free_func)
{
  assert (tree != NULL);
  
  if (tree->root.link[0] != &tree->root)
    {
      /* Uses Knuth's Algorithm 2.3.1T as modified in exercise 13
	 (postorder traversal). */
      
      /* T1. */
      avlt_node *an[AVL_MAX_HEIGHT];	/* Stack A: nodes. */
      char ab[AVL_MAX_HEIGHT];		/* Stack A: bits. */
      int ap = 0;			/* Stack A: height. */
      avlt_node *p = tree->root.link[0];

      for (;;)
	{
	  /* T2. */
	  for (;;)
	    {
	      /* T3. */
	      ab[ap] = 0;
	      an[ap++] = p;
	      if (p->tag[0] == MINUS)
		break;
	      p = p->link[0];
	    }

	  /* T4. */
	  for (;;)
	    {
	      if (ap == 0)
		goto done;

	      p = an[--ap];
	      if (ab[ap] == 0)
		{
		  ab[ap++] = 1;
		  if (p->tag[1] == MINUS)
		    continue;
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

/* avlt_destroy() with FREE_FUNC hardcoded as free(). */
void
avlt_free (avlt_tree *tree)
{
  avlt_destroy (tree, (avl_node_func) free);
}

/* Return the number of nodes in TREE. */
int
avlt_count (const avlt_tree *tree)
{
  assert (tree != NULL);
  return tree->count;
}

/* Copy the contents of TREE to a new tree in arena OWNER.  If COPY is
   non-NULL, then each data item is passed to function COPY, and the
   return values are inserted into the new tree; otherwise, the items
   are copied verbatim from the old tree to the new tree.  Returns the
   new tree. */
avlt_tree *
avlt_copy (const avlt_tree *tree, avl_copy_func copy)
{
  /* Knuth's Algorithm 2.3.1C (copying a binary tree).  Additionally
     uses Algorithm 2.3.1I (insertion into a threaded binary tree) and
     Algorithm 2.3.1 exercise 17 (preorder successor in threaded
     binary tree). */

  avlt_tree *new_tree;

  const avlt_node *p;
  avlt_node *q;
  
  assert (tree != NULL);
  new_tree = avlt_create (tree->cmp, tree->inscmp, tree->param);
  new_tree->count = tree->count;
  p = &tree->root;
  if (p->link[0] == p)
    return new_tree;
  q = &new_tree->root;

  for (;;)
    {
      /* C4.  This is Algorithm 2.3.1I modified for insertion to the
         left.  Step I2 is not necessary. */
      if (p->tag[0] == PLUS)
	{
	  avlt_node *r = xmalloc (sizeof (avlt_node));

	  r->link[0] = q->link[0];
	  r->tag[0] = q->tag[0];
	  q->link[0] = r;
	  q->tag[0] = PLUS;
	  r->link[1] = q;
	  r->tag[1] = MINUS;
	}

      /* C5: Find preorder successors of P and Q.  This is Algorithm
         2.3.1 exercise 17 but applies its actions to Q as well as
         P. */
      if (p->tag[0] == PLUS)
	{
	  p = p->link[0];
	  q = q->link[0];
	}
      else
	{
	  while (p->tag[1] == MINUS)
	    {
	      p = p->link[1];
	      q = q->link[1];
	    }
	  p = p->link[1];
	  q = q->link[1];
	}

      /* C6. */
      if (p == &tree->root)
	{
	  assert (q == &new_tree->root);
	  return new_tree;
	}
      
      /* C2.  This is Algorithm 2.3.1I.  Step I2 is not necessary. */
      if (p->tag[1] == PLUS)
	{
	  avlt_node *r = xmalloc (sizeof (avlt_node));

	  r->link[1] = q->link[1];
	  r->tag[1] = q->tag[1];
	  q->link[1] = r;
	  q->tag[1] = PLUS;
	  r->link[0] = q;
	  r->tag[0] = MINUS;
	}

      /* C3. */
      q->bal = p->bal;
      if (copy == NULL)
	q->data = p->data;
      else
	q->data = copy (p->data, tree->param);
    }
}

/* Threads the unthreaded AVL tree TREE in-place, and returns TREE cast to
   avlt_tree *. */
avlt_tree *
avlt_thread (struct avl_tree *_tree)
{
  /* Uses Knuth's Algorithm 2.3.1 exercise 30 (thread an unthreaded
     tree, with Algorithm 2.3.1T (inorder traversal) for computing
     Q$. */

  avlt_tree *tree = (avlt_tree *) _tree;

  /* Algorithm T's variables. */
  avlt_node *an[AVL_MAX_HEIGHT];	/* Stack A: nodes. */
  avlt_node **ap;			/* Stack A: stack pointer. */
  avlt_node *tp;			/* P. */

  /* Algorithm L's variables. */
  avlt_node *p, *q;

  assert (tree != NULL);

  /* T1. */
  ap = an;
  tp = tree->root.link[0];

  /* L1. */
  q = &tree->root;
  q->link[1] = q;

  for (;;)
    {
      /* L2. */
      {
	/* T2. */
	while (tp != NULL)
	  {
	    /* T3. */
	    *ap++ = tp;
	    tp = tp->link[0];
	  }
      
	/* T4.  Modified to visit HEAD after fully traversing the
           tree. */
	if (ap == an)
	  tp = &tree->root;
	else
	  tp = *--ap;

	/* T5: Visit P. */
	p = tp;
      }

      /* L3. */
      if (q->link[1] == NULL)
	{
	  q->link[1] = p;
	  q->tag[1] = MINUS;
	}
      else
	q->tag[1] = PLUS;
  
      if (p->link[0] == NULL)
	{
	  p->link[0] = q;
	  p->tag[0] = MINUS;
	}
      else
	p->tag[0] = PLUS;

      /* L4. */
      if (p == &tree->root)
	return tree;
      q = p;

      /* T5: Next. */
      tp = tp->link[1];
    }
}

/* Unthreads the threaded tree TREE in-place, and returns TREE cast to
   avl_tree *. */
struct avl_tree *
avlt_unthread (avlt_tree *tree)
{
  /* Uses Knuth's Algorithm 2.3.1T as modified in exercise 13
     (postorder traversal). */
      
  /* T1. */
  avlt_node *an[AVL_MAX_HEIGHT];	/* Stack A: nodes. */
  char ab[AVL_MAX_HEIGHT];		/* Stack A: bits. */
  int ap = 0;				/* Stack A: height. */
  avlt_node *p;

  assert (tree != NULL);
  p = tree->root.link[0];
  if (p != &tree->root)
    for (;;)
      {
	/* T2. */
	for (;;)
	  {
	    /* T3. */
	    ab[ap] = 0;
	    an[ap++] = p;
	    if (p->tag[0] == MINUS)
	      break;
	    p = p->link[0];
	  }

	/* T4. */
	for (;;)
	  {
	    if (ap == 0)
	      goto done;

	    p = an[--ap];
	    if (ab[ap] == 0)
	      {
		ab[ap++] = 1;
		if (p->tag[1] == MINUS)
		  continue;
		p = p->link[1];
		break;
	      }
      
	    if (p->tag[0] == MINUS)
	      p->link[0] = NULL;
	    if (p->tag[1] == MINUS)
	      p->link[1] = NULL;
	  }
      }
  else
    tree->root.link[0] = NULL;

 done:
  tree->root.link[1] = NULL;
  return (struct avl_tree *) tree;
}

/* Walk tree TREE in inorder, calling WALK_FUNC at each node.  Passes
   PARAM to WALK_FUNC.  */
void
avlt_walk (const avlt_tree *tree, avl_node_func walk_func, void *param)
{
  const avlt_node *p;

  /* Uses Knuth's algorithm 2.3.1D (threaded inorder successor). */
  assert (tree && walk_func);

  p = &tree->root;
  for (;;)
    {
      if (p->tag[1] == MINUS)
	p = p->link[1];
      else
	{
	  p = p->link[1];
	  while (p->tag[0] == PLUS)
	    p = p->link[0];
	}

      if (p == &tree->root)
	return;

      walk_func (p->data, param);
    }
}

/* Each call to this function for a given TREE and TRAV return the
   next item in the tree in inorder.  Initialize the first element of
   TRAV (init) to 0 before calling the first time.  Returns NULL when
   out of elements.  */
void *
avlt_traverse (const avlt_tree *tree, avlt_traverser *trav)
{
  const avlt_node *p;
  
  assert (tree && trav);

  if (trav->init == 0)
    {
      p = &tree->root;
      trav->init = 1;
    }
  else
    p = trav->p;

  /* Knuth's Algorithm 2.3.1S (threaded inorder successor). */
  if (p->tag[1] == MINUS)
    p = p->link[1];
  else
    {
      p = p->link[1];
      while (p->tag[0] == PLUS)
	p = p->link[0];
    }

  if (p == &tree->root)
    {
      trav->init = 0;
      return NULL;
    }
  else
    {
      trav->p = p;
      return (void *) p->data;
    }
}

/* Given ITEM, a pointer to a data item in TREE (or NULL), returns a
   pointer to the next item in the tree in comparison order, or NULL
   if ITEM is the last item. */
void **
avlt_next (const avlt_tree *tree, void **item)
{
  const avlt_node *p;

  assert (tree != NULL);
  if (item == NULL)
    p = &tree->root;
  else
    p = (avlt_node *) (((char *) item) - offsetof (avlt_node, data));

  /* Knuth's Algorithm 2.3.1S (threaded inorder successor). */
  if (p->tag[1] == MINUS)
    p = p->link[1];
  else
    {
      p = p->link[1];
      while (p->tag[0] == PLUS)
	p = p->link[0];
    }

  if (p == &tree->root)
    return NULL;

  return (void **) &p->data;
}

/* Given ITEM, a pointer to a data item in TREE (or NULL), returns a
   pointer to the previous item in the tree in comparison order, or
   NULL if ITEM is the first item. */
void **
avlt_prev (const avlt_tree *tree, void **item)
{
  const avlt_node *p;

  assert (tree != NULL);
  if (item == NULL)
    {
      /* Find node with greatest value. */
      p = tree->root.link[0];
      if (p == &tree->root)
	return NULL;
      while (p->tag[1] == PLUS)
	p = p->link[1];
    }
  else
    {
      p = (avlt_node *) (((char *) item) - offsetof (avlt_node, data));

      /* Knuth's Algorithm 2.3.1S (threaded inorder successor)
	 modified to find the predecessor node. */
      if (p->tag[0] == MINUS)
	p = p->link[0];
      else
	{
	  assert (p->tag[0] == PLUS);
	  p = p->link[0];
	  while (p->tag[1] == PLUS)
	    p = p->link[1];
	}
    }

  if (p == &tree->root)
    return NULL;

  return (void **) &p->data;
}

/* Search TREE for an item matching ITEM.  If found, returns a pointer
   to the address of the item.  If none is found, ITEM is inserted
   into the tree, and a pointer to the address of ITEM is returned.
   In either case, the pointer returned can be changed by the caller,
   or the returned data item can be directly edited, but the key data
   in the item must not be changed. */
void **
avlt_probe (avlt_tree *tree, void *item)
{
  /* Uses Knuth's Algorithm 6.2.3A (balanced tree search and
     insertion), modified for a threaded binary tree.  Caches results
     of comparisons.  In empirical tests this eliminates about 25% of
     the comparisons seen under random insertions.  */

  /* A1. */
  avlt_node *t;
  avlt_node *s, *p, *q, *r;

  assert (tree != NULL);
  t = &tree->root;
  s = p = t->link[0];

  if (t->tag[0] == MINUS)
    {
      tree->count++;
      assert (tree->count == 1);
      t->tag[0] = PLUS;
      q = t->link[0] = xmalloc (sizeof (avlt_node));
      q->data = item;
      q->link[0] = q->link[1] = t;
      q->tag[0] = q->tag[1] = MINUS;
      q->bal = 0;
      return &q->data;
    }

  for (;;)
    {
      /* A2. */
      int diff = CacheinscmpArray[tree->inscmp] (item, p->data, tree->param);

      /* A3. */
      if (diff < 0)
	{
	  p->cache = 0;
	  q = p->link[0];
	  if (p->tag[0] == MINUS)
	    {
	      q = xmalloc (sizeof (avlt_node));
	      q->link[0] = p->link[0];
	      q->tag[0] = p->tag[0];
	      p->link[0] = q;
	      p->tag[0] = PLUS;
	      q->link[1] = p;
	      q->tag[1] = MINUS;
	      break;
	    }
	}
      /* A4. */
      else if (diff > 0)
	{
	  p->cache = 1;
	  q = p->link[1];
	  if (p->tag[1] == MINUS)
	    {
	      q = xmalloc (sizeof (avlt_node));
	      q->link[1] = p->link[1];
	      q->tag[1] = p->tag[1];
	      p->link[1] = q;
	      p->tag[1] = PLUS;
	      q->link[0] = p;
	      q->tag[0] = MINUS;
	      break;
	    }
	}
      else
	/* A2. */
	return &p->data;

      /* A3, A4. */
      if (q->bal != 0)
	t = p, s = q;
      p = q;
    }
  
  /* A5. */
  tree->count++;
  q->data = item;
  q->bal = 0;
  
  /* A6. */
  r = p = s->link[(int) s->cache];
  while (p != q)
    {
      p->bal = p->cache * 2 - 1;
      p = p->link[(int) p->cache];
    }

  /* A7. */
  if (s->cache == 0)
    {
      /* a = -1. */
      if (s->bal == 0)
	{
	  s->bal = -1;
	  return &q->data;
	}
      else if (s->bal == +1)
	{
	  s->bal = 0;
	  return &q->data;
	}
      
      assert (s->bal == -1);
      if (r->bal == -1)
	{
	  /* A8. */
	  p = r;
	  s->link[0] = r->link[1];
	  s->tag[0] = r->tag[1];
	  r->link[1] = s;
	  r->tag[1] = PLUS;
	  if (s->link[0] == s)
	    {
	      s->link[0] = r;
	      s->tag[0] = MINUS;
	    }
	  r->tag[0] = r->tag[1] = PLUS;
	  s->bal = r->bal = 0;
	}
      else
	{
	  /* A9. */
	  assert (r->bal == +1);
	  p = r->link[1];
	  r->link[1] = p->link[0];
	  p->link[0] = r;
	  s->link[0] = p->link[1];
	  p->link[1] = s;
	  if (p->bal == -1)
	    s->bal = 1, r->bal = 0;
	  else if (p->bal == 0)
	    s->bal = r->bal = 0;
	  else 
	    {
	      assert (p->bal == +1);
	      s->bal = 0, r->bal = -1;
	    }
	  p->bal = 0;
	  p->tag[0] = p->tag[1] = PLUS;
	  if (s->link[0] == s)
	    {
	      s->link[0] = p;
	      s->tag[0] = MINUS;
	    }
	  if (r->link[1] == r)
	    {
	      r->link[1] = p;
	      r->tag[1] = MINUS;
	    }
	}
    }
  else
    {
      /* a == +1. */
      if (s->bal == 0)
	{
	  s->bal = 1;
	  return &q->data;
	}
      else if (s->bal == -1)
	{
	  s->bal = 0;
	  return &q->data;
	}

      assert (s->bal == +1);
      if (r->bal == +1)
	{
	  /* A8. */
	  p = r;
	  s->link[1] = r->link[0];
	  s->tag[1] = r->tag[0];
	  r->link[0] = s;
	  r->tag[0] = PLUS;
	  if (s->link[1] == s)
	    {
	      s->link[1] = r;
	      s->tag[1] = MINUS;
	    }
	  s->bal = r->bal = 0;
	}
      else
	{
	  /* A9. */
	  assert (r->bal == -1);
	  p = r->link[0];
	  r->link[0] = p->link[1];
	  p->link[1] = r;
	  s->link[1] = p->link[0];
	  p->link[0] = s;
	  if (p->bal == +1)
	    s->bal = -1, r->bal = 0;
	  else if (p->bal == 0)
	    s->bal = r->bal = 0;
	  else 
	    {
	      assert (p->bal == -1);
	      s->bal = 0, r->bal = 1;
	    }
	  p->tag[0] = p->tag[1] = PLUS;
	  if (s->link[1] == s)
	    {
	      s->link[1] = p;
	      s->tag[1] = MINUS;
	    }
	  if (r->link[0] == r)
	    {
	      r->link[0] = p;
	      r->tag[0] = MINUS;
	    }
	  p->bal = 0;
	}
    }
		
  /* A10. */
  if (t != &tree->root && s == t->link[1])
    t->link[1] = p;
  else
    t->link[0] = p;

  return &q->data;
}
  
/* Search TREE for an item matching ITEM, and return a pointer to it
   if found. */
void **
avlt_find (const avlt_tree *tree, const void *item)
{
  const avlt_node *p;

  assert (tree != NULL);
  if (tree->root.tag[0] == MINUS)
    /* Tree is empty. */
    return NULL;

  p = tree->root.link[0];
  for (;;)
    {
      int diff = CachecmpArray[tree->cmp] (item, p->data, tree->param);
      int t;

      /* A3. */
      if (diff < 0)
	t = 0;
      else if (diff > 0)
	t = 1;
      else
	return (void **) &p->data;

      if (p->tag[t] == PLUS)
	p = p->link[t];
      else
	return NULL;
    }
}

/* Search TREE for an item close to the value of ITEM, and return it.
   This function will return a null pointer only if TREE is empty. */
void **
avlt_find_close (const avlt_tree *tree, const void *item)
{
  const avlt_node *p;

  assert (tree != NULL);
  if (tree->root.tag[0] == MINUS)
    /* Tree is empty. */
    return NULL;

  p = tree->root.link[0];
  for (;;)
    {
      int diff = CachecmpArray[tree->cmp] (item, p->data, tree->param);
      int t;

      /* A3. */
      if (diff < 0)
	t = 0;
      else if (diff > 0)
	t = 1;
      else
	return (void **) &p->data;

      if (p->tag[t] == PLUS)
	p = p->link[t];
      else
	return (void **) &p->data;
    }
}

/* Searches AVL tree TREE for an item matching ITEM.  If found, the
   item is removed from the tree and the actual item found is returned
   to the caller.  If no item matching ITEM exists in the tree,
   returns NULL. */
void *
avlt_delete (avlt_tree *tree, const void *item)
{
  /* Uses my Algorithm DT, which can be found at
     http://www.msu.edu/user/pfaffben/avl.  Algorithm DT is based on
     Knuth's Algorithms 6.2.2D (Tree deletion), 6.2.3A (Balanced tree
     search and insertion), 2.3.1I (Insertion into a threaded binary
     trees), and the notes on pages 465-466 of Vol. 3. */

  /* D1. */
  avlt_node *pa[AVL_MAX_HEIGHT];	/* Stack P: Nodes. */
  unsigned char a[AVL_MAX_HEIGHT];	/* Stack P: Bits. */
  int k = 1;				/* Stack P: Pointer. */
  
  avlt_node *p;

  assert (tree != NULL);

  if (tree->root.tag[0] == MINUS)
    /* Empty tree. */
    return NULL;

  a[0] = 0;
  pa[0] = &tree->root;
  p = tree->root.link[0];
  for (;;)
    {
      /* D2. */
      int diff = CachecmpArray[tree->cmp] (item, p->data, tree->param);

      if (diff == 0)
	break;

      /* D3, D4. */
      pa[k] = p;
      if (diff < 0)
	{
	  if (p->tag[0] == PLUS)
	    {
	      p = p->link[0];
	      a[k] = 0;
	    }
	  else
	    return NULL;
	}
      else if (diff > 0)
	{
	  if (p->tag[1] == PLUS)
	    {
	      p = p->link[1];
	      a[k] = 1;
	    }
	  else
	    return NULL;
	}

      k++;
    }
  tree->count--;
  
  item = p->data;

  {
    avlt_node *t = p;
    avlt_node **q = &pa[k - 1]->link[(int) a[k - 1]];

    /* D5. */
    if (t->tag[1] == MINUS)
      {
	if (t->tag[0] == PLUS)
	  {
	    avlt_node *const x = t->link[0];

	    *q = x;
	    (*q)->bal = 0;
	    if (x->tag[1] == MINUS)
	      {
		if (a[k - 1] == 1)
		  x->link[1] = t->link[1];
		else
		  x->link[1] = pa[k - 1];
	      }
	  }
	else
	  {
	    *q = t->link[a[k - 1]];
	    pa[k - 1]->tag[a[k - 1]] = MINUS;
	  }
      }
    else
      {
	/* D6. */
	avlt_node *r = t->link[1];
	if (r->tag[0] == MINUS)
	  {
	    r->link[0] = t->link[0];
	    r->tag[0] = t->tag[0];
	    r->bal = t->bal;
	    if (r->tag[0] == PLUS)
	      {
		avlt_node *s = r->link[0];
		while (s->tag[1] == PLUS)
		  s = s->link[1];
		assert (s->tag[1] == MINUS);
		s->link[1] = r;
	      }
	    *q = r;
	    a[k] = 1;
	    pa[k++] = r;
	  }
	else
	  {
	    /* D7. */
	    avlt_node *s = r->link[0];

	    a[k] = 1;
	    pa[k++] = t;

	    a[k] = 0;
	    pa[k++] = r;
	    
	    /* D8. */
	    while (s->tag[0] != MINUS)
	      {
		r = s;
		s = r->link[0];
		a[k] = 0;
		pa[k++] = r;
	      }

	    /* D9. */
	    t->data = s->data;
	    if (s->tag[1] == MINUS)
	      {
		r->tag[0] = MINUS;
		r->link[0] = t;
	      }
	    else
	      {
		r->link[0] = s->link[1];
		if (s->link[1]->tag[0] == MINUS)
		  s->link[1]->link[0] = t;
	      }
	    p = s;
	  }
      }
  }

  free (p);

  assert (k > 0);
  /* D10. */
  while (--k)
    {
      avlt_node *const s = pa[k];

      if (a[k] == 0)
	{
	  avlt_node *const r = s->link[1];
	  
	  /* D10. */
	  if (s->bal == -1)
	    {
	      s->bal = 0;
	      continue;
	    }
	  else if (s->bal == 0)
	    {
	      s->bal = +1;
	      break;
	    }

	  assert (s->bal == +1);
	  if (s->tag[1] == MINUS || r->bal == 0)
	    {
	      /* D11. */
	      s->link[1] = r->link[0];
	      r->link[0] = s;
	      r->bal = -1;
	      pa[k - 1]->link[(int) a[k - 1]] = r;
	      break;
	    }
	  else if (r->bal == +1)
	    {
	      /* D12. */
	      if (PLUS == (s->tag[1] = r->tag[0]))
		s->link[1] = r->link[0];
	      r->link[0] = s;
	      r->tag[0] = PLUS;
	      s->bal = r->bal = 0;
	      pa[k - 1]->link[a[k - 1]] = r;
	    }
	  else 
	    {
	      /* D13. */
	      assert (r->bal == -1);
	      p = r->link[0];
	      if (PLUS == (r->tag[0] = p->tag[1]))
		r->link[0] = p->link[1];
	      p->link[1] = r;
	      p->tag[1] = PLUS;
	      if (MINUS == (s->tag[1] = p->tag[0]))
		s->link[1] = p;
	      else
		s->link[1] = p->link[0];
	      p->link[0] = s;
	      p->tag[0] = PLUS;
	      if (p->bal == +1)
		s->bal = -1, r->bal = 0;
	      else if (p->bal == 0)
		s->bal = r->bal = 0;
	      else
		{
		  assert (p->bal == -1);
		  s->bal = 0, r->bal = +1;
		}
	      p->bal = 0;
	      pa[k - 1]->link[(int) a[k - 1]] = p;
	      pa[k - 1]->tag[(int) a[k - 1]] = PLUS;
	    }
	}
      else
	{
	  avlt_node *const r = s->link[0];
	  
	  /* D10. */
	  if (s->bal == +1)
	    {
	      s->bal = 0;
	      continue;
	    }
	  else if (s->bal == 0)
	    {
	      s->bal = -1;
	      break;
	    }

	  assert (s->bal == -1);
	  if (s->tag[0] == MINUS || r->bal == 0)
	    {
	      /* D11. */
	      s->link[0] = r->link[1];
	      r->link[1] = s;
	      r->bal = +1;
	      pa[k - 1]->link[(int) a[k - 1]] = r;
	      break;
	    }
	  else if (r->bal == -1)
	    {
	      /* D12. */
	      if (PLUS == (s->tag[0] = r->tag[1]))
		s->link[0] = r->link[1];
	      r->link[1] = s;
	      r->tag[1] = PLUS;
	      s->bal = r->bal = 0;
	      pa[k - 1]->link[a[k - 1]] = r;
	    }
	  else 
	    {
	      /* D13. */
	      assert (r->bal == +1);
	      p = r->link[1];
	      if (PLUS == (r->tag[1] = p->tag[0]))
		r->link[1] = p->link[0];
	      p->link[0] = r;
	      p->tag[0] = PLUS;
	      if (MINUS == (s->tag[0] = p->tag[1]))
		s->link[0] = p;
	      else
		s->link[0] = p->link[1];
	      p->link[1] = s;
	      p->tag[1] = PLUS;
	      if (p->bal == -1)
		s->bal = +1, r->bal = 0;
	      else if (p->bal == 0)
		s->bal = r->bal = 0;
	      else
		{
		  assert (p->bal == +1);
		  s->bal = 0, r->bal = -1;
		}
	      p->bal = 0;
	      pa[k - 1]->link[(int) a[k - 1]] = p;
	      pa[k - 1]->tag[(int) a[k - 1]] = PLUS;
	    }
	}
    }
      
  return (void *) item;
}

/* Inserts ITEM into TREE.  Returns NULL if the item was inserted,
   otherwise a pointer to the duplicate item. */
void *
avlt_insert (avlt_tree *tree, void *item)
{
  void **p;
  
  assert (tree != NULL);
  
  p = avlt_probe (tree, item);
  return (*p == item) ? NULL : *p;
}

/* If ITEM does not exist in TREE, inserts it and returns NULL.  If a
   matching item does exist, it is replaced by ITEM and the item
   replaced is returned.  The caller is responsible for freeing the
   item returned. */
void *
avlt_replace (avlt_tree *tree, void *item)
{
  void **p;

  assert (tree != NULL);
  
  p = avlt_probe (tree, item);
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
(avlt_force_delete) (avlt_tree *tree, void *item)
{
  void *found = avlt_delete (tree, item);
  assert (found != NULL);
  return found;
}

#if SELF_TEST

/* Size of the tree used for testing. */
#define TREE_SIZE 1024	

/* Used to flag delayed aborting. */
int done = 0;

/* Count the number of nodes in TREE below and including NODE. */
int
count (avlt_tree *tree, avlt_node *node)
{
  int n = 1;
  if (node->tag[0] == PLUS)
    n += count (tree, node->link[0]);
  if (node->tag[1] == PLUS)
    n += count (tree, node->link[1]);
  return n;
}

/* Print the structure of node NODE of an avl tree, which is LEVEL
   levels from the top of the tree.  Uses different delimiters to
   visually distinguish levels. */
void
print_structure (avlt_tree *tree, avlt_node *node, int level)
{
  char lc[] = "([{<`";
  char rc[] = ")]}>'";

  assert (node != NULL);
  if (level >= 10)
    {
      printf ("Too deep, giving up.\n");
      done = 1;
      return;
    }
  if (node == &tree->root)
    {
      printf (" root");
      return;
    }
  printf (" %c%d", lc[level % 5], (int) node->data);
  fflush (stdout);

  {
    int i;

    for (i = 0; i <= 1; i++)
      {
	if (node->tag[i] == PLUS)
	  print_structure (tree, node->link[i], level + 1);
	else if (node->link[i] != &tree->root)
	  printf (" :%d", (int) node->link[i]->data);
	else
	  printf (" :r");
	fflush (stdout);
      }
  }
  printf ("%c", rc[level % 5]);
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
print_contents (avlt_tree *tree)
{
  avlt_walk (tree, print_int, NULL);
  printf ("\n");
}

/* Examine NODE in a avl tree.  *COUNT is increased by the number of
   nodes in the tree, including the current one.  If the node is the
   root of the tree, PARENT should be INT_MIN, otherwise it should be
   the parent node value.  DIR is the direction that the current node
   is linked from the parent: -1 for left child, +1 for right child;
   it is not used if PARENT is INT_MIN.  Returns the height of the
   tree rooted at NODE. */
int
recurse_tree (avlt_tree *tree, avlt_node *node, int *count, int parent,
	      int dir, unsigned char *nodes, unsigned char *threads)
{
  if (node != &tree->root) 
    {
      int d = (int) node->data;
      int nl = 0;
      int nr = 0;

      (*count)++;

      assert (d >= 0 && d < TREE_SIZE);
      if (nodes[d / 8] & (1 << (d % 8)))
	{
	  printf (" Arrived at node %d by two different paths.\n", d);
	  done = 1;
	}
      else
	nodes[d / 8] |= 1 << (d % 8);

      if (node->tag[0] == PLUS)
	nl = recurse_tree (tree, node->link[0], count, d, -1, nodes, threads);
      else if (node->link[0] != &tree->root)
	{
	  int dl = (int) node->link[0]->data;
	  assert (dl >= 0 && dl < TREE_SIZE);
	  threads[dl / 8] |= 1 << (dl % 8);
	}

      if (node->tag[1] == PLUS)
	nr = recurse_tree (tree, node->link[1], count, d, 1, nodes, threads);
      else if (node->link[1] != &tree->root)
	{
	  int dr = (int) node->link[1]->data;
	  assert (dr >= 0 && dr < TREE_SIZE);
	  threads[dr / 8] |= 1 << (dr % 8);
	}

      if (nr - nl != node->bal)
	{
	  printf (" Node %d has incorrect balance: right height=%d, "
		  "left height=%d, difference=%d, but balance factor=%d.\n",
		  d, nr, nl, nr - nl, node->bal);
	  done = 1;
	}
      
      if (node->bal < -1 || node->bal > 1)
	{
	  printf (" Node %d has invalid balance factor %d.\n",
		  d, node->bal);
	  done = 1;
	}
      
      if (parent != INT_MIN)
	{
	  assert (dir == -1 || dir == +1);
	  if (dir == -1 && d > parent)
	    {
	      printf (" Node %d is smaller than its left child %d.\n",
		    parent, d);
	      done = 1;
	    }
	  else if (dir == +1 && d < parent)
	    {
	      printf (" Node %d is larger than its right child %d.\n",
		      parent, d);
	      done = 1;
	    }
	}
      assert (node->bal >= -1 && node->bal <= 1);
      return 1 + (nl > nr ? nl : nr);
    }
  else return 0;
}

/* Check that everything about TREE is kosher. */
void
verify_tree (avlt_tree *tree)
{
  {
    unsigned char nodes[(TREE_SIZE + 7) / 8];
    unsigned char threads[(TREE_SIZE + 7) / 8];

    int count = 0;
    int i;
    
    memset (nodes, 0, (TREE_SIZE + 7) / 8);
    memset (threads, 0, (TREE_SIZE + 7) / 8);
  
    recurse_tree (tree, tree->root.link[0], &count, INT_MIN, 0, nodes,
		  threads);
    
    if (count != tree->count)
      {
	printf (" Tree should have %d nodes, but tree count by recursive "
		"descent is %d.\n", tree->count, count);
	done = 1;
      }

    for (i = 0; i < TREE_SIZE; i++)
      {
	int thread = threads[i / 8] & (1 << (i % 8));
	int node = nodes[i / 8] & (1 << (i % 8));

	if (thread && !node)
	  {
	    printf (" A thread leads to ``node'' %d, which is "
		    "not in the tree.", i);
	    done = 1;
	  }
      }
  }

  /* Check right threads. */
  {
    int count = 0;
    int last = INT_MIN;
    void **data = NULL;
  
    while (NULL != (data = avlt_next (tree, data)))
      {
	if (((int) *data) < last)
	  {
	    printf (" Misordered right threads.\n");
	    abort ();
	  }
	else if (((int) *data) == last)
	  {
	    printf (" Loop in right threads detected on %d.\n", last);
	    abort ();
	  }
	last = (int) *data;
	count++;
      }
    if (count != tree->count)
      {
	printf (" Tree should have %d nodes, but tree count by right threads "
		"is %d.\n", tree->count, count);
	done = 1;
      }
  }

  /* Check left threads. */
  {
    int count = 0;
    int last = INT_MAX;
    void **data = NULL;

    while (NULL != (data = avlt_prev (tree, data)))
      {
	if (((int) *data) > last)
	  {
	    printf (" Misordered left threads.\n");
	    abort ();
	  }
	else if (((int) *data) == last)
	  {
	    printf (" Loop in left threads detected on %d.\n", last);
	    abort ();
	  }
	last = (int) *data;
	count++;
      }
    if (count != tree->count)
      {
	printf (" Tree should have %d nodes, but tree count by left threads "
		"is %d.\n", tree->count, count);
	done = 1;
      }
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

/* Compares avl trees rooted at A and B, making sure that they are
   identical. */
void
compare_trees (avlt_node *a, avlt_node *b)
{
  int diff = 0;
  
  assert (a && b);
  
  /* Separating these conditions makes it easier to pinpoint bad data
     under a memory debugger like Checker because each test is a
     separate statement. */
  diff |= a->data != b->data;
  diff |= a->bal != b->bal;
  diff |= ((a->tag[0] == PLUS) ^ (b->tag[0] == PLUS));
  diff |= ((a->tag[1] == PLUS) ^ (b->tag[1] == PLUS));
  if (diff)
    {
      printf (" Copied nodes differ: %d b=%d a->bal=%d b->bal=%d a:",
	      (int) a->data, (int) b->data, a->bal, b->bal);
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
  if (a->tag[0] == PLUS)
    compare_trees (a->link[0], b->link[0]);
  if (a->tag[1] == PLUS)
    compare_trees (a->link[1], b->link[1]);
}

/* Simple stress test procedure for the AVL tree routines.  Does the
   following:

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

   If you make any modifications to the avl tree routines, then you
   might want to insert some calls to print_structure() at strategic
   places in order to be able to see what's really going on.  Also,
   memory debuggers like Checker or Purify are very handy. */
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

  fputs ("Testing avlt...\n", stdout);
  
  for (iteration = 1; iteration <= N_ITERATIONS; iteration++)
    {
      avlt_tree *tree;
      int i;
      
      printf ("Iteration %4d/%4d: seed=%5d", iteration, N_ITERATIONS, seed);
      fflush (stdout);
      
      srand (seed++);

      for (i = 0; i < TREE_SIZE; i++)
	array[i] = i;
      shuffle (array, TREE_SIZE);
      
      tree = avlt_create (compare_ints, NULL);
      for (i = 0; i < TREE_SIZE; i++)
	avlt_force_insert (tree, (void *) (array[i]));
      verify_tree (tree);
      
      shuffle (array, TREE_SIZE);
      for (i = 0; i < TREE_SIZE; i++)
	{
	  avlt_tree *copy;

	  avlt_delete (tree, (void *) (array[i]));
	  verify_tree (tree);

	  copy = avlt_copy (tree, NULL);
	  verify_tree (copy);
	  if (tree->root.link[0] != &tree->root)
	    compare_trees (tree->root.link[0], copy->root.link[0]);
	  else if (copy->root.link[1] != &copy->root)
	    printf (" Empty tree results in nonempty copy.\n"), abort ();
	  avlt_destroy (copy, NULL);

	  if (i % 128 == 0)
	    {
	      putchar ('.');
	      fflush (stdout);
	    }
	}
      fputs (" good.\n", stdout);

      avlt_destroy (tree, NULL);
    }
  
  return 0;
}
#endif /* SELF_TEST */

/*
  Local variables:
  compile-command: "gcc -DSELF_TEST=1 -W -Wall -I. -o ./avlt-test avlt.c"
  End:
*/

