
/*
 * io.h
 *
 * Communication between PSSE and the outside world, via libcurl.
 *
 * (c) 2019 Simon Brooke <simon@journeyman.cc>
 * Licensed under GPL version 2.0, or, at your option, any later version.
 */

#ifndef __psse_io_h
#define __psse_io_h

struct cons_pointer
lisp_close( struct stack_frame *frame, struct cons_pointer frame_pointer,
            struct cons_pointer env );
struct cons_pointer
lisp_open( struct stack_frame *frame, struct cons_pointer frame_pointer,
           struct cons_pointer env );
struct cons_pointer
lisp_read_char( struct stack_frame *frame, struct cons_pointer frame_pointer,
                struct cons_pointer env );
struct cons_pointer
lisp_slurp( struct stack_frame *frame, struct cons_pointer frame_pointer,
            struct cons_pointer env );


#endif
