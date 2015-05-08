#ifndef _pef_h_
#define _pef_h_

PefHeader *
PEF10_GetFrame(int type, void **ptr, int payload_len);

char *
PEF10_EncryptPayload(SA *sa, char *payload, int plen);
     
char *
PEF10_DecryptPayload(SA *sa, char *payload, int plen);

PktHeader *
EncryptPayload(SA *sa, PktHeader *hdr, int offset, int size);
     
PktHeader *
DecryptPayload(SA *sa, PktHeader *hdr, int offset, int size);

#endif /* _pef_h_ */
