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

#include <rvinternal.h>
#include <stdio.h>

#include <tls.h>
#include <log.h>
#include <msg.h>
#include <ms.h>


#undef msaPrintFormat



RVAPI void RVCALLCONV msaPrintFormat(int type,const char*line,...)
{
#ifndef NOLOGSUPPORT
    if (logIsSelected((RVHLOG)type, RV_INFO))
    {
        va_list v;
        char* buf = msGetPrintBuffer();

        va_start(v, line);
        vsprintf(buf, line, v);
        va_end(v);

        logPrintFormat((RVHLOG)type, RV_INFO, "%s", buf);
    }
#else
	if (type || line);
#endif
}

#ifdef __cplusplus
}
#endif



