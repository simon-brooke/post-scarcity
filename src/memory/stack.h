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
 * macros for the tag of a stack frame.
 */
#define STACKFRAMETAG "STAK"
#define STACKFRAMETV 1262572627

/**
 * is this vector-space object a stack frame?
 */
#define stackframep(vso)(((struct vector_space_object *)vso)->header.tag.value == STACKFRAMETV)

/**
 * set a register in a stack frame. Alwaye use this macro to do so,
 • because that way we can be sure the inc_ref happens!
 */
//#define set_reg(frame,register,value){frame->arg[register]=value; inc_ref(value);}

void set_reg(struct stack_frame * frame, int reg, struct cons_pointer value);

struct stack_frame *get_stack_frame( struct cons_pointer pointer );

struct cons_pointer make_empty_frame( struct cons_pointer previous );

struct cons_pointer make_stack_frame( struct cons_pointer previous,
                                      struct cons_pointer args,
                                      struct cons_pointer env );

void free_stack_frame( struct stack_frame *frame );

void dump_frame( FILE * output, struct cons_pointer pointer );

void dump_stack_trace( FILE * output, struct cons_pointer frame_pointer );

struct cons_pointer fetch_arg( struct stack_frame *frame, unsigned int n );

struct cons_pointer make_special_frame( struct cons_pointer previous,
                                        struct cons_pointer args,
                                        struct cons_pointer env );

/*
 * struct stack_frame is defined in consspaceobject.h to break circularity
 * TODO: refactor.
 */

#endif
