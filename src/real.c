/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

#include "conspage.h"
#include "consspaceobject.h"
#include "read.h"

/**
 * Allocate a real number cell representing this value and return a cons 
 * pointer to it.
 * @param value the value to wrap;
 * @return a real number cell wrapping this value.
 */
struct cons_pointer make_real( long double value) {
  struct cons_pointer result = allocate_cell( REALTAG);
  struct cons_space_object* cell = &pointer2cell(result);
  cell->payload.real.value = value;

  return result;
}
  
