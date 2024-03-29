/*
 * integer.h
 *
 * functions for integer cells.
 *
 *
 * (c) 2017 Simon Brooke <simon@journeyman.cc>
 * Licensed under GPL version 2.0, or, at your option, any later version.
 */

#ifndef __integer_h
#define __integer_h

#include <stdbool.h>
#include <stdint.h>

struct cons_pointer make_integer( int64_t value, struct cons_pointer more );

struct cons_pointer add_integers( struct cons_pointer a,
                                  struct cons_pointer b );

struct cons_pointer multiply_integers( struct cons_pointer a,
                                       struct cons_pointer b );

struct cons_pointer integer_to_string( struct cons_pointer int_pointer,
                                       int base );

bool equal_integer_integer( struct cons_pointer a, struct cons_pointer b );

bool equal_integer_real( struct cons_pointer a, struct cons_pointer b );

#endif
