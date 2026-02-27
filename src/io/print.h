/**
 * print.h
 *
 * First pass at a printer, for bootstrapping.
 *
 *
 * (c) 2017 Simon Brooke <simon@journeyman.cc>
 * Licensed under GPL version 2.0, or, at your option, any later version.
 */

#include <ctype.h>
#include <stdio.h>

#include "io/fopen.h"

#ifndef __print_h
#define __print_h

struct cons_pointer print( URL_FILE * output, struct cons_pointer pointer );
void println( URL_FILE * output );

struct cons_pointer lisp_print( struct stack_frame *frame,
                                struct cons_pointer frame_pointer,
                                struct cons_pointer env );
struct cons_pointer lisp_println( struct stack_frame *frame,
                                  struct cons_pointer frame_pointer,
                                  struct cons_pointer env );


#endif
