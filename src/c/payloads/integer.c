/**
 *  payloads/integer.c
 *
 *  An integer. Doctrine here is that we are not implementing bignum arithmetic in
 *  the bootstrap layer; an integer is, for now, just a 64 bit integer.
 *
 *  (c) 2026 Simon Brooke <simon@journeyman.cc>
 *  Licensed under GPL version 2.0, or, at your option, any later version.
 */

#include <stdint.h>

#include "memory/node.h"
#include "memory/pointer.h"
#include "memory/pso.h"
#include "memory/pso2.h"
#include "memory/tags.h"

#include "debug.h"

/**
 * Allocate an integer cell representing this `value` and return a pso_pointer to it.
 * @param value an integer value;
 * @param more `nil`, or a pointer to the more significant cell(s) of this number.
 * *NOTE* that if `more` is not `nil`, `value` *must not* exceed `MAX_INTEGER`.
 */
struct pso_pointer make_integer( int64_t value ) {
    struct pso_pointer result = nil;
    debug_print( L"Entering make_integer\n", DEBUG_ALLOC, 0 );

    result = allocate( INTEGERTAG, 2 );
    struct pso2 *cell = pointer_to_object( result );
    cell->payload.integer.value = value;

    debug_print( L"make_integer: returning\n", DEBUG_ALLOC, 0 );
    debug_dump_object( result, DEBUG_ALLOC, 0 );

    return result;
}
