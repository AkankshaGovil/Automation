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

/* This is file avlt.h in libavl. */

#if !avlt_h
#define avlt_h 1

/* The default maximum height of 32 allows for AVL trees having
   between 5,704,880 and 4,294,967,295 nodes, depending on order of
   insertion.  You may change this compile-time constant as you
   wish. */
#ifndef AVL_MAX_HEIGHT
#define AVL_MAX_HEIGHT	32
#endif

/* Structure for a node in a threaded AVL tree. */
typedef struct avlt_node
  {
    void *data;			/* Pointer to data. */
    struct avlt_node *link[2];	/* Subtrees or threads. */
    signed char bal;		/* Balance factor. */
    char cache;			/* Used during insertion. */
    signed char tag[2];		/* Left and right thread tags. */
  }
avlt_node;

/* Used for traversing a threaded AVL tree. */
typedef struct avlt_traverser
  {
    int init;			/* Initialized? */
    const avlt_node *p;		/* Last node returned. */
  }
avlt_traverser;

/* Initializer for avlt_traverser. */
#define AVLT_TRAVERSER_INIT {0}

/* Function types. */
#if !AVL_FUNC_TYPES
#define AVL_FUNC_TYPES 1
typedef int (*avl_comparison_func) (const void *a, const void *b, void *param);
typedef void (*avl_node_func) (void *data, void *param);
typedef void *(*avl_copy_func) (void *data, void *param);
#endif

/* Structure which holds information about a threaded AVL tree. */
typedef struct avlt_tree
  {
    avlt_node root;		/* Tree root node. */
    int cmp;	/* Used to compare keys. */
    int inscmp;	/* Used to compare keys. */
    int count;			/* Number of nodes in the tree. */
    void *param;		/* Arbitary user data. */
  }
avlt_tree;

/* General functions. */
avlt_tree *avlt_create (int, int, void *param);
int avlt_init (int mtype);
void avlt_destroy (avlt_tree *, avl_node_func);
void avlt_free (avlt_tree *);
int avlt_count (const avlt_tree *);
avlt_tree *avlt_copy (const avlt_tree *, avl_copy_func);
struct avl_tree;
avlt_tree *avlt_thread (struct avl_tree *);
struct avl_tree *avlt_unthread (avlt_tree *);

/* Walk the tree. */
void avlt_walk (const avlt_tree *, avl_node_func, void *param);
void *avlt_traverse (const avlt_tree *, avlt_traverser *);
#define avlt_init_traverser(TRAVERSER) ((TRAVERSER)->init = 0)
void **avlt_next (const avlt_tree *tree, void **item);
void **avlt_prev (const avlt_tree *tree, void **item);

/* Search for a given item. */
void **avlt_probe (avlt_tree *, void *);
void *avlt_delete (avlt_tree *, const void *);
void **avlt_find (const avlt_tree *, const void *);
void **avlt_find_close (const avlt_tree *, const void *);

#if __GCC__ >= 2
extern inline void *
avlt_insert (avlt_tree *tree, void *item)
{
  void **p = avlt_probe (tree, item);
  return (*p == item) ? NULL : *p;
}

extern inline void *
avlt_replace (avlt_tree *tree, void *item)
{
  void **p = avlt_probe (tree, item);
  if (*p == item)
    return NULL;
  else
    {
      void *r = *p;
      *p = item;
      return r;
    }
}
#else /* not gcc */
void *avlt_insert (avlt_tree *tree, void *item);
void *avlt_replace (avlt_tree *tree, void *item);
#endif /* not gcc */

/* Easy assertions on insertion & deletion. */
#ifndef NDEBUG
#define avlt_force_insert(A, B)			\
	do					\
	  {					\
            void *r = avlt_insert (A, B);	\
	    assert (r == NULL);			\
	  }					\
	while (0)
void *avlt_force_delete (avlt_tree *, void *);
#else
#define avlt_force_insert(A, B)			\
	avlt_insert (A, B)
#define avlt_force_delete(A, B)			\
	avlt_delete (A, B)
#endif

extern avl_comparison_func CacheinscmpArray[];
extern avl_comparison_func CachecmpArray[];

#endif /* avlt_h */
