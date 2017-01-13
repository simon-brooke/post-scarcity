/**
 * stack.c
 *
 * The Lisp evaluation stack.
 *
 * Stack frames could be implemented in cons space; indeed, the stack
 * could simply be an assoc list consed onto the front of the environment. 
 * But such a stack would be costly to search. The design sketched here, 
 * with stack frames as special objects, SHOULD be substantially more 
 * efficient, but does imply we need to generalise the idea of cons pages
 * with freelists to a more general 'equal sized object pages', so that
 * allocating/freeing stack frames can be more efficient.
 * 
 * Stack frames are not yet a first class object; they have no VECP pointer
 * in cons space.
 *
 * (c) 2017 Simon Brooke <simon@journeyman.cc>
 * Licensed under GPL version 2.0, or, at your option, any later version.
 */

#include <stdlib.h>

#include "consspaceobject.h"
#include "conspage.h"
#include "stack.h"

/**
 * Allocate a new stack frame with its previous pointer set to this value
 */
struct stack_frame* make_stack_frame(struct stack_frame* previous) {
  /* TODO: later, pop a frame off a free-list of stack frames */
  struct stack_frame* result = malloc( sizeof( struct stack_frame));

  result->previous = previous;

  /* clearing the frame with memset would probably be slightly quicker, but
   * this is clear. */
  result->more = NIL;
  result->function = NIL;

  for ( int i = 0; i < locals_in_frame; i++) {
    result->local[i] = NIL;
  }

  return result;
}

/**
 * Free this stack frame.
 */
void free_stack_frame( struct stack_frame* frame) {
  /* TODO: later, push it back on the stack-frame freelist */
  free( frame);
}

/**
 * Fetch a pointer to the value of the local variable at this index.
 */
struct cons_pointer fetch_local( struct stack_frame* frame, unsigned int index) {
  struct cons_pointer result = NIL;
  
  if ( index < locals_in_frame) {
    result = frame->local[ index];
  } else {
    struct cons_pointer p = frame->more;
    
    for ( int i = locals_in_frame; i < index; i++) {
      p = pointer2cell( p).payload.cons.cdr;
    }

    result = pointer2cell( p).payload.cons.car;
  }

  return result;
}
