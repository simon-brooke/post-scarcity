/**
 *  payloads/special.h
 *
 *  A special form.
 *
 *  No payload here; it would be identical to function_payload, q.v., so
 *  use that instead. 
 *
 *  (c) 2026 Simon Brooke <simon@journeyman.cc>
 *  Licensed under GPL version 2.0, or, at your option, any later version.
 */

#ifndef __psse_payloads_special_h
#define __psse_payloads_special_h

#include "memory/pointer.h"
#include "memory/pso4.h"

/**
 * A special form - one whose arguments are not pre-evaluated but passed as
 * provided.
 * \see NLAMBDATAG.
 */
#define SPECIALTAG  "SFM"
#define SPECIALTV   5064275

#endif
