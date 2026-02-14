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

#define replace_integer_i(p,i) {struct cons_pointer __p = acquire_integer(i,NIL); release_integer(p); p = __p;}
#define replace_integer_p(p,q) {struct cons_pointer __p = p; release_integer( p);  p = q;}

struct cons_pointer make_integer( int64_t value, struct cons_pointer more );

struct cons_pointer acquire_integer( int64_t value, struct cons_pointer more );

void release_integer( struct cons_pointer p );

struct cons_pointer add_integers( struct cons_pointer a,
                                  struct cons_pointer b );

struct cons_pointer multiply_integers( struct cons_pointer a,
                                       struct cons_pointer b );

struct cons_pointer integer_to_string( struct cons_pointer int_pointer,
                                       int base );

bool equal_integer_integer( struct cons_pointer a, struct cons_pointer b );

bool equal_integer_real( struct cons_pointer a, struct cons_pointer b );

#endif
