/**
 *  payloads/integer.h
 *
 *  An integer.
 *
 *  (c) 2026 Simon Brooke <simon@journeyman.cc>
 *  Licensed under GPL version 2.0, or, at your option, any later version.
 */

#ifndef __psse_payloads_integer_h
#define __psse_payloads_integer_h

#include <stdint.h>

/**
 * @brief An integer .
 *
 * Integers can in principal store a 128 bit value, but in practice we'll start
 * promoting them to bignums when they pass the 64 bit barrier. However, that's
 * in the Lisp layer, not the substrate.
 */
struct integer_payload {
    __int128_t value;
};

struct pso_pointer make_integer( int64_t value );

#endif
