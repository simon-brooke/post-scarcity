/**
 *  payloads/lambda.h
 *
 *  A lambda cell.
 *
 *  (c) 2026 Simon Brooke <simon@journeyman.cc>
 *  Licensed under GPL version 2.0, or, at your option, any later version.
 */

#ifndef __psse_payloads_lambda_h
#define __psse_payloads_lambda_h

#include "memory/pointer.h"

/**
 * @brief Tag for lambda cell. Lambdas are the interpretable (source) versions of functions.
 * \see FUNCTIONTAG.
 */
#define LAMBDATAG  "LMD"

/**
 * @brief payload for lambda and nlambda cells.
 */
struct lambda_payload {
    /** the arument list */
    struct pso_pointer args;
    /** the body of the function to be applied to the arguments. */
    struct pso_pointer body;
};


#endif
