/*
 * psse_time.h
 *
 * Bare bones of PSSE time. See issue #16.
 *
 * (c) 2019 Simon Brooke <simon@journeyman.cc>
 * Licensed under GPL version 2.0, or, at your option, any later version.
 */

#ifndef __psse_time_h
#define __psse_time_h

#define _GNU_SOURCE
#include "consspaceobject.h"

struct cons_pointer lisp_time( struct stack_frame *frame,
                               struct cons_pointer frame_pointer,
                               struct cons_pointer env );
struct cons_pointer time_to_string( struct cons_pointer pointer );

#endif
