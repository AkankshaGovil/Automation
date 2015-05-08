#ifndef _tstpriv_h_
#define  _tstpriv_h_ 

void *SHM_Malloc(size_t);
void SHM_Free(void *);
void *SHM_Realloc(void *, size_t);

extern void *(*_tst_malloc)(size_t);
extern void (*_tst_free)(void *);
extern void *(*_tst_realloc)(void *, size_t);

#define malloc(n)	_tst_malloc(n)
#define calloc(n, sz)	_tst_malloc((n)*(sz))
#define free(ptr)	_tst_free(ptr)
#define realloc(ptr, size)	_tst_realloc(ptr, size)

#endif /* _tstpriv_h_ */
