#include <stdlib.h>
#include "srvrlog.h"
#include "bits.h"
#include "ipc.h"
#include "mem.h"
#include "handles.h"
#include <malloc.h>

/************************ RHANDLES ********************************/

ResolveHandle *
GisAllocRHandle(void)
{
	 ResolveHandle *rhandle = 
		  (ResolveHandle *)malloc(sizeof(ResolveHandle));

	 memset(rhandle, 0, sizeof(ResolveHandle));

	 return rhandle;
}

void
GisFreeRHandle(void *ptr)
{
	 ResolveHandle *r = (ResolveHandle *)ptr;

	 if (r == NULL)
	 {
		  return;
	 }

	 free(r);
}

