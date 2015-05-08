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


#include <rvcommon.h>
#include "buffer.h"

BOOL buffAddToBuffer(BUFFER* to, BUFFER* from, UINT32 offset)
{
    if (from->length + offset <= to->length)
    {
        memcpy((UINT8*)to->buffer + offset, from->buffer, from->length);

        return TRUE;
    }

    return FALSE;
}

BOOL buffValid(BUFFER *buff, UINT32 size)
{
    return (size <= buff->length  &&  buff->buffer);
}

BUFFER  buffCreate(void* data,int size)
{
    BUFFER buff;
    buff.buffer = (unsigned char*)data;
    buff.length = size;
    return buff;
}
#ifdef __cplusplus
}
#endif



