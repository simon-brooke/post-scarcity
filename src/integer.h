/**
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

/**
 * Allocate an integer cell representing this value and return a cons pointer to it.
 */
struct cons_pointer makeinteger( int value);

#endif
