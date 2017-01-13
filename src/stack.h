/**
 * stack.h
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

#include "consspaceobject.h"

#ifndef __stack_h
#define __stack_h

/* number of local variables stored in a stack frame */
#define locals_in_frame 8

struct stack_frame* make_stack_frame(struct stack_frame* previous);
void free_stack_frame( struct stack_frame* frame);
struct cons_pointer fetch_local( struct stack_frame* frame, unsigned int n);

struct stack_frame {
  struct stack_frame* previous;         /* the previous frame */
  struct cons_pointer local[locals_in_frame];
                                        /* first 8 local variable bindings */
  struct cons_pointer more;             /* list of any further local 
					 * variable bindings */
  struct cons_pointer function;         /* the function to be called */
};

#endif
