
/*

NOTICE:
This document contains information that is proprietary to RADVISION LTD..
No part of this publication may be reproduced in any form whatsoever without
written prior approval by RADVISION LTD..

RADVISION LTD. reserves the right to revise this publication and make changes
without obligation to notify any person of such revisions or changes.

*/

#ifndef RASPARAMS_H
#define RASPARAMS_H

#ifdef __cplusplus
extern "C" {
#endif


#include <cmiras.h>
#include <rasdef.h>



/************************************************************************
 * rasGetParam
 * purpose: Get a parameter about the RAS transaction
 * input  : ras     - RAS instance we're using
 *          hsRas   - Stack's handle for the RAS transaction
 *          trStage - The transaction stage the parameters
 *          param   - Type of the RAS parameter
 *          index   - If the parameter has several instances, the index
 *                    that identifies the specific instance (0-based).
 *                    0 otherwise.
 *          value   - If the parameter value is a structure, the value
 *                    represents the length of the parameter.
 * output : value   - For a simple integer - the parameter's value.
 *                    For a structure - the length of the parameter.
 *          svalue  - For a structure - svalue represents the parameter
 *                    itself. Can be set to NULL if we're only interested
 *                    in its length.
 * return : If an error occurs, the function returns a negative value.
 *          If no error occurs, the function returns a non-negative value.
 ************************************************************************/
int rasGetParam(
    IN  rasModule*       ras,
    IN  HRAS             hsRas,
    IN  cmRASTrStage     trStage,
    IN  cmRASParam       param,
    IN  int              index,
    IN  OUT INT32*       value,
    IN  char*            svalue);


/************************************************************************
 * rasSetParam
 * purpose: Get a parameter about the RAS transaction
 * input  : ras     - RAS instance we're using
 *          hsRas   - Stack's handle for the RAS transaction
 *          trStage - The transaction stage the parameters
 *          param   - Type of the RAS parameter
 *          index   - If the parameter has several instances, the index
 *                    that identifies the specific instance (0-based).
 *                    0 otherwise.
 *          value   - For a simple integer - the parameter's value.
 *                    For a structure - the length of the parameter.
 *          svalue  - For a structure - svalue represents the parameter
 *                    itself.
 * return : If an error occurs, the function returns a negative value.
 *          If no error occurs, the function returns a non-negative value.
 ************************************************************************/
int rasSetParam(
    IN  rasModule*      ras,
    IN  HRAS            hsRas,
    IN  cmRASTrStage    trStage,
    IN  cmRASParam      param,
    IN  int             index,
    IN  INT32           value,
    IN  char*           svalue);


/************************************************************************
 * rasGetNumOfParams
 * purpose: Returns the number of params in sequences on the property
 *          tree.
 * input  : ras     - RAS instance we're using
 *          hsRas   - Stack's handle for the RAS transaction
 *          trStage - The transaction stage the parameters
 *          param   - Type of the RAS parameter
 * output : none
 * return : Number of params in sequence on the property tree on success
 *          Negative value on failure
 ************************************************************************/
int rasGetNumOfParams(
    IN  rasModule*      ras,
    IN  HRAS            hsRas,
    IN  cmRASTrStage    trStage,
    IN  cmRASParam      param);




#ifdef __cplusplus
}
#endif

#endif  /* RASPARAMS_H */

