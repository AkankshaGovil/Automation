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

/* This is file rb.h in libavl. */

#if !rb_h
#define rb_h 1

/* The default maximum height of 32 allows for red-black trees having
   between 65,535 and 4,294,967,295 nodes, depending on order of
   insertion.  You may change this compile-time constant as you
   wish. */
#ifndef RB_MAX_HEIGHT
#define RB_MAX_HEIGHT	32
#endif

/* Node colors. */
enum
  {
    RB_BLACK,
    RB_RED,
  };

/* Structure for a node in a red-black tree. */
typedef struct rb_node
  {
    void *data;			/* Pointer to data. */
    struct rb_node *link[2];	/* Subtrees. */
    signed char color;		/* Color. */
    char cache;			/* Used during insertion. */
    signed char pad[2];		/* Unused.  Reserved for threaded trees. */
  }
rb_node;

/* Used for traversing a red-black tree. */
typedef struct rb_traverser
  {
    int init;			/* Initialized? */
    int nstack;			/* Top of stack. */
    const rb_node *p;		/* Used for traversal. */
    const rb_node *stack[RB_MAX_HEIGHT];/* Descended trees. */
  }
rb_traverser;

/* Initializer for rb_traverser. */
#define RB_TRAVERSER_INIT {0}

/* Function types. */
#if !AVL_FUNC_TYPES
#define AVL_FUNC_TYPES 1
typedef int (*avl_comparison_func) (const void *a, const void *b, void *param);
typedef void (*avl_node_func) (void *data, void *param);
typedef void *(*avl_copy_func) (void *data, void *param);
#endif

/* Structure which holds information about a red-black tree. */
typedef struct rb_tree
  {
#if PSPP
    struct arena **owner;	/* Arena to store nodes. */
#endif
    rb_node root;		/* Tree root node. */
    int cmp;	/* Used to compare keys. */
    int inscmp;	/* Used to compare keys. */
    int count;			/* Number of nodes in the tree. */
    void *param;		/* Arbitary user data. */
  }
rb_tree;

#if PSPP
#define MAYBE_ARENA struct arena **owner,
#else
#define MAYBE_ARENA /* nothing */
#endif

/* General functions. */
rb_tree *rb_create (MAYBE_ARENA int, int, void *param);
int rb_init (int mtype);
void rb_destroy (rb_tree *, avl_node_func);
void rb_free (rb_tree *);
int rb_count (const rb_tree *);
rb_tree *rb_copy (MAYBE_ARENA const rb_tree *, avl_copy_func);

/* Walk the tree. */
void rb_walk (const rb_tree *, avl_node_func, void *param);
void *rb_traverse (const rb_tree *, rb_traverser *);
#define rb_init_traverser(TRAVERSER) ((TRAVERSER)->init = 0)

/* Search for a given item. */
void **rb_probe (rb_tree *, void *);
void *rb_delete (rb_tree *, const void *);
void *rb_find (const rb_tree *, const void *);
void *rb_find_close (const rb_tree *, const void *);

#if __GCC__ >= 2
extern inline void *
rb_insert (rb_tree *tree, void *item)
{
  void **p = rb_probe (tree, item);
  return (*p == item) ? NULL : *p;
}

extern inline void *
rb_replace (rb_tree *tree, void *item)
{
  void **p = rb_probe (tree, item);
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
void *rb_insert (rb_tree *tree, void *item);
void *rb_replace (rb_tree *tree, void *item);
#endif /* not gcc */

/* Easy assertions on insertion & deletion. */
#ifndef NDEBUG
#define rb_force_insert(A, B)			\
	do					\
	  {					\
            void *r = rb_insert (A, B);		\
	    assert (r == NULL);			\
	  }					\
	while (0)
void *rb_force_delete (rb_tree *, void *);
#else
#define rb_force_insert(A, B)			\
	rb_insert (A, B)
#define rb_force_delete(A, B)			\
	rb_delete (A, B)
#endif

extern avl_comparison_func CacheinscmpArray[];
extern avl_comparison_func CachecmpArray[];

#endif /* rb_h */
