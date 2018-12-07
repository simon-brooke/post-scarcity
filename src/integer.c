/*
 * integer.c
 *
 * functions for integer cells.
 *
 * (c) 2017 Simon Brooke <simon@journeyman.cc>
 * Licensed under GPL version 2.0, or, at your option, any later version.
 */

#define _GNU_SOURCE
#include <math.h>
#include <stdio.h>

#include "conspage.h"
#include "consspaceobject.h"
#include "read.h"

/**
 * return the numeric value of this cell, as a C primitive double, not
 * as a cons-space object. Cell may in principle be any kind of number,
 * but only integers and reals are so far implemented.
 */
long double numeric_value( struct cons_pointer pointer ) {
    double result = NAN;
    struct cons_space_object *cell = &pointer2cell( pointer );

    if ( integerp( pointer ) ) {
        result = cell->payload.integer.value * 1.0;
    } else if ( realp( pointer ) ) {
        result = cell->payload.real.value;
    }

    return result;
}

/**
 * Allocate an integer cell representing this value and return a cons pointer to it.
 */
struct cons_pointer make_integer( long int value ) {
    struct cons_pointer result = allocate_cell( INTEGERTAG );
    struct cons_space_object *cell = &pointer2cell( result );
    cell->payload.integer.value = value;

    dump_object( stderr, result );

    return result;
}
