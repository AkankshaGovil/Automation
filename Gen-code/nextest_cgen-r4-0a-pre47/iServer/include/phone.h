#ifndef _phone_h_
#define _phone_h_

int phoneMatch(void *x, void *opaque);

/* Used by Finds...*/
int phoneLookup(void *x, void *opaque);

int phoneExactMatch(void *x, void *opaque);

int checkPhoneField(NetoidInfoEntry *info, char **sphone);

int gPhoneMatch(void *x, void *opaque);
int gVpnPhoneMatch(void *x, void *opaque);

#endif /* _phone_h_ */
