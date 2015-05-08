
#ifndef _phonode_h_
#define _phonode_h_

#include "ipc.h"
#include "bits.h"
#include "serverdb.h"

PhoNode *PhoNodeDup(PhoNode *phonode);
PhoNode *NewPhoNode(void);

int htonPhonode(PhoNode *phonodep);

int ntohPhonode(PhoNode *phonodep);

int PhoNodeCmp(PhoNode *n1, PhoNode *n2);

int PrintPhoNode(PhoNode *node);

extern int InitPhonodeFromInfoEntry (InfoEntry *info, PhoNode *phonode);

#endif /* _phonode_h_ */
