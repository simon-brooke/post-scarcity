/**
 *  payloads/special.h
 *
 *  A special form.
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

// /**
//  * @brief Payload of a special form cell. 
//  *
//  * Currently identical to the payload of a function cell.
//  * \see function_payload
//  */
// struct special_payload {
//     /**
//      * pointer to the source from which the special form was compiled, or NIL
//      * if it is a primitive.
//      */
//     struct pso_pointer meta;
//     /**  pointer to a function which takes a cons pointer (representing
//      * its argument list) and a cons pointer (representing its environment) and a
//      * stack frame (representing the previous stack frame) as arguments and returns
//      * a cons pointer (representing its result). */
//     struct pso_pointer ( *executable ) ( struct pso4*,
//                                           struct pso_pointer,
//                                           struct pso_pointer );
// };

#endif
