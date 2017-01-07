/**
 * equal.h
 *
 * Checks for shallow and deep equality
 *
 * (c) 2017 Simon Brooke <simon@journeyman.cc>
 * Licensed under GPL version 2.0, or, at your option, any later version.
 */

#include <math.h>
#include <stdbool.h>

#ifndef __equal_h
#define __equal_h

/**
 * Shallow, and thus cheap, equality: true if these two objects are 
 * the same object, else false.
 */
bool eq( struct cons_pointer a, struct cons_pointer b);

/**
 * Deep, and thus expensive, equality: true if these two objects have
 * identical structure, else false.
 */
bool equal( struct cons_pointer a, struct cons_pointer b);

#endif
