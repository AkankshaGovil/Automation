#ifndef _RESUTILS_H_
#define _RESUTILS_H_

#include "bits.h"
#include "ipc.h"

int GwPortAvailCall(PhoNode *phonode, int ncalls, int callSource);
int CPMatchPattern (char *inpattern, char **outpattern, int *hasMatch, int *hasRejectAll,
	       	int *hasReject, char *phone);
int CPAnalyzePattern (char *dest, int *reject, char **rdest);
int CPAnalyzePattern (char *dest, int *reject, char **rdest);
int GwCompare (struct gw_match_set *g1, struct gw_match_set *g2, int *reason);
int GwRouteCompare (struct gw_match_set *g1, struct gw_match_set *g2, int *reason);
int RouteCompareSet (route_match_set *old, route_match_set *new);
int GwUpdateCP (char *cpname);
int GwUpdateCR (char *crname);
int ctInfoAvailCall (CacheTableInfo *ctInfo, int ncalls, int callSource);
int GeneratePatternInstance (char *pat, char *vars, char *newpat, int patlen);
// Utilities to replace the original ANI with an ANI from a file
int ReplaceANI(char *phonenum, char *filename);

#endif /* _RESUTILS_H_ */
