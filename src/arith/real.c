/*
 * real.c
 *
 * functions for real number cells.
 *
 * (c) 2017 Simon Brooke <simon@journeyman.cc>
 * Licensed under GPL version 2.0, or, at your option, any later version.
 */

#include "memory/conspage.h"
#include "memory/consspaceobject.h"
#include "debug.h"
#include "io/read.h"

/**
 * Allocate a real number cell representing this value and return a cons
 * pointer to it.
 * @param value the value to wrap;
 * @return a real number cell wrapping this value.
 */
struct cons_pointer make_real( long double value ) {
    struct cons_pointer result = allocate_cell( REALTV );
    struct cons_space_object *cell = &pointer2cell( result );
    cell->payload.real.value = value;

    debug_dump_object( result, DEBUG_ARITH );

    return result;
}
