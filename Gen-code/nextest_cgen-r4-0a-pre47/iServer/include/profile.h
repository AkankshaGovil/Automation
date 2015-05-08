#ifndef _profile_h_
#define _profile_h_

/*
 * profile.h
 * Contains definitions for profile databse
 */

typedef enum {
     Db_eProfileCall = 0,
} DbProfileType;
	
typedef struct {
     NetoidSNKey snkey;
     unsigned short profileType;
     unsigned short seqNo;
} DbProfileCallKey;
  
typedef Profile DbProfileCall;

void *
DbProfileGet(DB db, DbProfileType type, char *key, int keylen, void *handle);

AlStatus
DbProfileStore(DB db, void *profile, DbProfileType type, char *key, int keylen);

AlStatus
DbProfileDelete(DB db, DbProfileType type, char *key, int keylen);

#endif /* _profile_h_ */
