/*

NOTICE:
This document contains information that is proprietary to RADVISION LTD..
No part of this publication may be reproduced in any form whatsoever without
written prior approval by RADVISION LTD..

RADVISION LTD. reserves the right to revise this publication and make changes
without obligation to notify any person of such revisions or changes.

*/

#ifndef _CMLOG_H
#define _CMLOG_H

#ifdef __cplusplus
extern "C" {
#endif

#include <cmintr.h>
#include <ci.h>
#include <pvaltree.h>
#include <log.h>


/************************************************************************
 * cmiCreateLog
 * purpose: Create a log manager for use by a stack instance
 *          This function reads the information from a configuration
 *          manager and initializes the log manager with it.
 * input  : app     - Stack's application handle
 * output : none
 * return : Log manager handle on success
 *          NULL on failure
 ************************************************************************/
RVHLOGMGR RVCALLCONV cmiCreateLog(IN cmElem* app);


/************************************************************************
 * pvtLoadFromConfig
 * purpose: Convert information from a configuration instance (HCFG) to
 *          a value tree of a given syntax.
 * input  : hCfg        - Configuration instance handle
 *          hVal        - Value tree handle
 *          name        - Name of root from configuration to convert
 *          rootNodeId  - Syntax of configuration
 *          log         - Log handle to use for debug messages
 * output : none
 * return : Root ID of configuration created on success
 *          Negative value on failure
 ************************************************************************/
int pvtLoadFromConfig(HCFG hCfg, HPVT hVal, char* name, int rootNodeId, RVHLOG log);


/************************************************************************
 * cmGetLogMgr
 * purpose: Get the log manager instance used
 * input  : hApp     - Stack instance handle
 * output : none
 * return : Log manager used on success
 *          NULL on failure
 ************************************************************************/
RVAPI RVHLOGMGR RVCALLCONV cmGetLogMgr(IN  HAPP        hApp);


#ifdef __cplusplus
}
#endif

#endif  /* _CMLOG_H */

