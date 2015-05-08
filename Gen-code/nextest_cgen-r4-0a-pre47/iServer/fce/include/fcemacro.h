/*
 * Copyright (c) 2003 NExtone Communications, Inc.
 * All rights reserved.
 */
#ifndef _FCEMACRO_H_
#define _FCEMACRO_H_

/******************************************************************************
**
**  This file contains helpful definitions and macros used in FCE
**
*******************************************************************************
*/

/*
**    Handy MACROs
*/

/* Macro to detect if the firewall is an NSF firewall */
#define IS_LOCAL_FW                                           \
	(    (strcasecmp((fceConfigFwName), "NSF") == 0)            \
      || (strcasecmp((fceConfigFwName), "ipfilter") == 0)       \
      || (strcasecmp((fceConfigFwName), "HKNIFE") == 0) )

#define IS_FW_DISABLED 										\
	( (strcasecmp((fceConfigFwName), "none") == 0) )

#define IS_HKNIFE_FW                                        \
	( (strcasecmp((fceConfigFwName), "HKNIFE") == 0) )

#endif /* _FCEMACRO_H_ */

 
