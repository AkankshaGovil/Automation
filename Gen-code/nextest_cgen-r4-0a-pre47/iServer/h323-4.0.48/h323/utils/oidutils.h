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



typedef enum
{
    numberForm, /* 0 2 3 */
    nameForm, /* itu-t 2 3 */
    nameAndNumberForm /* itu-t(0) 2 3 */
} form;



int oidEncodeOID(
         OUT int oidSize,
         OUT char* oid,
         IN char*buff
         );


int oidDecodeOID(
         IN  int oidSize,
         IN  char* oid,
         OUT int buffSize,
         OUT char* buff,
         IN  form f
         );

#ifdef __cplusplus
}
#endif



