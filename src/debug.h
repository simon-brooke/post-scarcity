/**
 * debug.h
 *
 * Better debug log messages.
 *
 * (c) 2017 Simon Brooke <simon@journeyman.cc>
 * Licensed under GPL version 2.0, or, at your option, any later version.
 */

#include <ctype.h>
#include <stdio.h>

#ifndef __debug_print_h
#define __debug_print_h

#define DEBUG_ALLOC 1
#define DEBUG_STACK 2
#define DEBUG_ARITH 4
#define DEBUG_EVAL 8
#define DEBUG_LAMBDA 16
#define DEBUG_BOOTSTRAP 32
#define DEBUG_IO 64

extern int verbosity;
void debug_print( wchar_t *message, int level );
void debug_print_object( struct cons_pointer pointer, int level );
void debug_dump_object( struct cons_pointer pointer, int level );
#endif
