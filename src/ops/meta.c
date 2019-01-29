/*
 * meta.c
 *
 * Get metadata from a cell which has it.
 *
 * (c) 2019 Simon Brooke <simon@journeyman.cc>
 * Licensed under GPL version 2.0, or, at your option, any later version.
 */

#include "conspage.h"
#include "debug.h"

/**
 * Function: get metadata describing my first argument.
 *
 * * (metadata any)
 *
 * @return a pointer to the metadata of my first argument, or nil if none.
 */
struct cons_pointer lisp_metadata( struct stack_frame *frame, struct cons_pointer frame_pointer,
                                  struct cons_pointer env ) {
  debug_print(L"lisp_metadata: entered\n", DEBUG_EVAL);
  debug_dump_object(frame->arg[0], DEBUG_EVAL);
  struct cons_pointer result = NIL;
  struct cons_space_object cell = pointer2cell(frame->arg[0]);

  switch( cell.tag.value) {
    case FUNCTIONTV:
    result = cell.payload.function.meta;
    break;
    case SPECIALTV:
    result = cell.payload.special.meta;
    break;
    case READTV:
    case WRITETV:
    result = cell.payload.special.meta;
    break;
  }

  return make_cons(
    make_cons(
      c_string_to_lisp_keyword( L"type"),
      c_type(frame->arg[0])),
    result);

//  return result;
}
