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
#include "conspage.h"

#ifndef __stack_h
#define __stack_h

/**
 * set a register in a stack frame. Alwaye use this macro to do so,
 • because that way we can be sure the inc_ref happens!
 */
#define set_reg(frame,register,value)frame->arg[register]=value; inc_ref(value)


/**
 * Make an empty stack frame, and return it.
 * @param previous the current top-of-stack;
 * @param env the environment in which evaluation happens.
 * @return the new frame.
 */
struct stack_frame *make_empty_frame( struct stack_frame *previous,
                                      struct cons_pointer env );

struct stack_frame *make_stack_frame( struct stack_frame *previous,
                                      struct cons_pointer args,
                                      struct cons_pointer env,
                                      struct cons_pointer *exception );
void free_stack_frame( struct stack_frame *frame );

/**
 * Dump a stackframe to this stream for debugging
 * @param output the stream
 * @param frame the frame
 */
void dump_frame( FILE * output, struct stack_frame *frame );

struct cons_pointer fetch_arg( struct stack_frame *frame, unsigned int n );

/**
 * A 'special' frame is exactly like a normal stack frame except that the
 * arguments are unevaluated.
 * @param previous the previous stack frame;
 * @param args a list of the arguments to be stored in this stack frame;
 * @param env the execution environment;
 * @return a new special frame.
 */
struct stack_frame *make_special_frame( struct stack_frame *previous,
                                        struct cons_pointer args,
                                        struct cons_pointer env );

/*
 * struct stack_frame is defined in consspaceobject.h to break circularity
 * TODO: refactor.
 */

#endif
