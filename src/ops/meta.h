/*
 * meta.h
 *
 * Get metadata from a cell which has it.
 *
 * (c) 2019 Simon Brooke <simon@journeyman.cc>
 * Licensed under GPL version 2.0, or, at your option, any later version.
 */

#ifndef __psse_meta_h
#define __psse_meta_h


struct cons_pointer lisp_metadata( struct stack_frame *frame, struct cons_pointer frame_pointer,
                                  struct cons_pointer env ) ;

#endif
