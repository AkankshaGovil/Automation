#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <netinet/in.h>
#include "ipc.h"
#include "blowfish.h"
#include "rc4.h"
#include "isakmp.h"
#include <malloc.h>

#define DEBUG_SEC

PefHeader *
PEF10_GetFrame(int type, void **ptr, int payload_len)
{
	int len = payload_len + sizeof(PefHeader);
	PefHeader *buf = (PefHeader *)malloc(len);
	
	memset(buf, 0, len);
	buf->type = PKT_PEF;
	buf->totalLen = len;
	buf->next_payload = type;

	*ptr = buf + 1;
	
	return buf;
}

char *
PEF10_EncryptPayload(SA *sa, char *payload, int plen)
{
     char *buf;

     printf("Encrypting payload using BF\n");

#ifdef DEBUG_SEC
     if (plen % 8)
     {
	  /* Cry... */
	  printf("Payload is not a multiple of 8 bytes, rejecting...\n");
	  return( (char*) 0 );
     }
#endif     

     buf = payload;

#if 0
     BF_ecb_encrypt(buf, buf, &sa->symmkey, BF_ENCRYPT);
#endif
     RC4(&sa->symmkey, plen, (unsigned char *)buf, (unsigned char *)buf);
 
     printf("finished encryption. encrypted len is %d bytes\n", 
	    plen);

     return buf;
}
     
char *
PEF10_DecryptPayload(SA *sa, char *payload, int plen)
{
     char *buf;

     printf("Decrypting payload using BF\n");

     if (plen % 8)
     {
	  printf("invalid packet len the data len is %d bytes\n", 
		 plen);
	  return 0;
     }
     
     buf = payload;

#if 0
     BF_ecb_encrypt(buf, buf, &sa->symmkey, BF_DECRYPT);
#endif
     RC4(&sa->symmkey, plen, (unsigned char *)buf, (unsigned char *)buf);
 
     printf("finished decryption. decrypted len is %d bytes\n", 
	    plen);

     return buf;
}

/* Start encrypting at offset, and encrypt size bytes.
 * If stuff is not the required size, pad it,
 * and change the length stuff inside header.
 */
PktHeader *
EncryptPayload(SA *sa, PktHeader *hdr, int offset, int size)
{
     PktHeader *new;
     char *buf;

     printf("Encrypting payload using BF\n");
     new = hdr;

     if (size % 8)
     {
	  int newlen;

	  printf("packet must be padded, the data len is %d bytes\n", 
		 size);
	  /* Now new is really new ;-) */
	  size = ((size/8) + 1)*8;
	  newlen = size + offset;
	  new = (PktHeader *)malloc(newlen);
	  memcpy(new, hdr, ntohl(hdr->totalLen));
	  
	  /* remaining should be set to zeros */
	  memset((char *)new + ntohl(hdr->totalLen), 0, 
		 newlen - ntohl(hdr->totalLen)); /* New len - old len */

	  hdr->totalLen = htonl(newlen);
	  free(hdr);
     }
     
     buf = (char *)new + offset;

#if 0
     BF_ecb_encrypt(buf, buf, &sa->symmkey, BF_ENCRYPT);
#endif
     RC4(&sa->symmkey, size, (unsigned char *)buf, (unsigned char *)buf);
 
     printf("finished encryption. encrypted len is %d bytes\n", 
	    size);

     return new;
}
     
PktHeader *
DecryptPayload(SA *sa, PktHeader *hdr, int offset, int size)
{
     PefHeader *new;
     char *buf;

     /* We may need to adjust the length of the payload etc...
      */
     printf("Decrypting payload using BF\n");
     new = (PefHeader *)hdr;

     if (size % 8)
     {
	  printf("invalid packet len the data len is %d bytes\n", 
		 size);
	  return 0;
     }
     
     buf = (char *)new + offset;

#if 0
     BF_ecb_encrypt(buf, buf, &sa->symmkey, BF_DECRYPT);
#endif
     RC4(&sa->symmkey, size, (unsigned char *)buf, (unsigned char *)buf);
 
     printf("finished decryption. decrypted len is %d bytes\n", 
	    size);

     return (PktHeader *)new;
}

int
htonl_pefhdr(PefHeader *hdr)
{
	hdr->type = htonl(hdr->type);
	hdr->totalLen = htonl(hdr->totalLen);
	hdr->next_payload = htonl(hdr->next_payload);
	return( 0 );
}

int
ntohl_pefhdr(PefHeader *hdr)
{
	hdr->type = ntohl(hdr->type);
	hdr->totalLen = ntohl(hdr->totalLen);
	hdr->next_payload = ntohl(hdr->next_payload);
	return( 0 );
}

int
htonl_pkthdr(PktHeader *hdr)
{
        hdr->type = htonl(hdr->type);
        hdr->totalLen = htonl(hdr->totalLen);
		return( 0 );
}

int
ntohl_pkthdr(PktHeader *hdr)
{
        hdr->type = ntohl(hdr->type);
        hdr->totalLen = ntohl(hdr->totalLen);
		return( 0 );
}

