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
struct cons_pointer makeinteger( int value) {
  struct cons_pointer result = allocatecell( INTEGERTAG);
  struct cons_space_object cell = conspages[result.page]->cell[result.offset];
  cell.payload.integer.value = value;

  return result;
}
  
