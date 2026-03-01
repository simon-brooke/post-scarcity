/*
 * debug.h
 *
 * Better debug log messages.
 *
 * (c) 2017 Simon Brooke <simon@journeyman.cc>
 * Licensed under GPL version 2.0, or, at your option, any later version.
 */

#include <ctype.h>
#include <stdbool.h>
#include <stdio.h>

#include "memory/consspaceobject.h"

#ifndef __debug_print_h
#define __debug_print_h

/**
 * @brief Print messages debugging memory allocation. 
 *
 * Flag interpretation for the value of `verbosity`, defined in `debug.c`, q.v.
 */
#define DEBUG_ALLOC 1

/**
 * @brief Print messages debugging arithmetic operations.
 * 
 * Flag interpretation for the value of `verbosity`, defined in `debug.c`, q.v.
 */
#define DEBUG_ARITH 2

/**
 * @brief Print messages debugging symbol binding.
 * 
 * Flag interpretation for the value of `verbosity`, defined in `debug.c`, q.v.
 */
#define DEBUG_BIND 4

/**
 * @brief Print messages debugging bootstrapping and teardown.
 * 
 * Flag interpretation for the value of `verbosity`, defined in `debug.c`, q.v.
 */
#define DEBUG_BOOTSTRAP 8

/**
 * @brief Print messages debugging evaluation.
 * 
 * Flag interpretation for the value of `verbosity`, defined in `debug.c`, q.v.
 */
#define DEBUG_EVAL 16

/**
 * @brief Print messages debugging input/output operations.
 * 
 * Flag interpretation for the value of `verbosity`, defined in `debug.c`, q.v.
 */
#define DEBUG_IO 32

/**
 * @brief Print messages debugging lambda functions (interpretation).
 * 
 * Flag interpretation for the value of `verbosity`, defined in `debug.c`, q.v.
 */
#define DEBUG_LAMBDA 64

/**
 * @brief Print messages debugging the read eval print loop.
 * 
 * Flag interpretation for the value of `verbosity`, defined in `debug.c`, q.v.
 */
#define DEBUG_REPL 128

/**
 * @brief Print messages debugging stack operations.
 * 
 * Flag interpretation for the value of `verbosity`, defined in `debug.c`, q.v.
 */
#define DEBUG_STACK 256

extern int verbosity;

void debug_print_exception( struct cons_pointer ex_ptr );
void debug_print( wchar_t *message, int level );
void debug_print_128bit( __int128_t n, int level );
void debug_println( int level );
void debug_printf( int level, wchar_t *format, ... );
void debug_print_object( struct cons_pointer pointer, int level );
void debug_dump_object( struct cons_pointer pointer, int level );
void debug_print_binding( struct cons_pointer key, struct cons_pointer val,
                          bool deep, int level );

#endif
