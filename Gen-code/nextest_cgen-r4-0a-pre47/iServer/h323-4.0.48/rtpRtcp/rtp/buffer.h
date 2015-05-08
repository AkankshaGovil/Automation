#ifdef __cplusplus
extern "C" {
#endif



/*

NOTICE:
This document contains information that is proprietary to RADVISION LTD..
No part of this publication may be reproduced in any form whatsoever without
written prior approval by RADVISION LTD..

RADVISION LTD. reserves the right to revise this publication and make changes
without obligation to notify any person of such revisions or changes.

*/

typedef struct
{
    UINT32 length;
    BYTE*  buffer;
} BUFFER;

BOOL buffAddToBuffer(BUFFER* to,BUFFER* from, UINT32 offset);
BOOL buffValid(BUFFER* buff,UINT32 size);
BUFFER buffCreate(void* data,int size);
#ifdef __cplusplus
}
#endif



