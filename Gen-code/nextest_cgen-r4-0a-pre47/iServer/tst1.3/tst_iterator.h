#ifndef _tst_iterator_h_
#define  _tst_iterator_h_ 

typedef struct
{
	int 		key_index;		// depicts the no of chars matched, if 
								// non-null value was returned - basically
								// indicates depth of current match/mismatch

	int			init;			// internal field
	struct tst 	*tst;			// internal field
   	struct node *current_node;	// internal field

} tst_iterator;

tst_iterator *
tst_iterator_init(tst_iterator *tsti, struct tst *tst);

void *tst_searchall(tst_iterator *tsti, unsigned char *key);

#endif /* _tst_iterator_h_ */
