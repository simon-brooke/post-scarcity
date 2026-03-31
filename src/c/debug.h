/**
 *  debug.h
 *
 *  Post Scarcity Software Environment: entry point.
 *
 *  Print debugging output.
 *
 *
 *  (c) 2026 Simon Brooke <simon@journeyman.cc>
 *  Licensed under GPL version 2.0, or, at your option, any later version.
 */

#ifndef __psse_debug_h
#define __psse_debug_h
#include <ctype.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>

/*
 * wide characters
 */
#include <wchar.h>
#include <wctype.h>

#include "memory/pointer.h"

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

/**
 * @brief Print messages about equality tests.
 * 
 * Flag interpretation for the value of `verbosity`, defined in `debug.c`, q.v.
 */
#define DEBUG_EQUAL 512

/**
 * @brief Verbosity (and content) of debugging output
 *
 * Interpreted as a sequence of topic-specific flags, see above.
 */
extern int verbosity;

void debug_print( wchar_t *message, int level, int indent );

void debug_print_object( struct pso_pointer object, int level, int indent );

void debug_dump_object( struct pso_pointer object, int level, int indent );

void debug_print_128bit( __int128_t n, int level );

void debug_println( int level );

void debug_printf( int level, int indent, wchar_t *format, ... );

#endif
