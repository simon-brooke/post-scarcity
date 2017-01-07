/**
 * integer.c
 *
 * functions for integer cells.
 *
 * (c) 2017 Simon Brooke <simon@journeyman.cc>
 * Licensed under GPL version 2.0, or, at your option, any later version.
 */

#include "conspage.h"
#include "consspaceobject.h"
#include "read.h"

/**
 * Allocate an integer cell representing this value and return a cons pointer to it.
 */
struct cons_pointer make_integer( int value) {
  struct cons_pointer result = allocate_cell( INTEGERTAG);
  struct cons_space_object* cell = &pointer2cell(result);
  cell->payload.integer.value = value;

  return result;
}
  
