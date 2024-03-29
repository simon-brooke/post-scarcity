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
#define DEBUG_ARITH 2
#define DEBUG_BIND 4
#define DEBUG_BOOTSTRAP 8
#define DEBUG_EVAL 16
#define DEBUG_IO 32
#define DEBUG_LAMBDA 64
#define DEBUG_REPL 128
#define DEBUG_STACK 256

extern int verbosity;

void debug_print( wchar_t *message, int level );
void debug_print_128bit( __int128_t n, int level );
void debug_println( int level );
void debug_printf( int level, wchar_t *format, ... );
void debug_print_object( struct cons_pointer pointer, int level );
void debug_dump_object( struct cons_pointer pointer, int level );

#endif
