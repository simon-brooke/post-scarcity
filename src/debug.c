/**
 * debug.c
 *
 * Better debug log messages.
 *
 * (c) 2017 Simon Brooke <simon@journeyman.cc>
 * Licensed under GPL version 2.0, or, at your option, any later version.
 */

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
/*
 * wide characters
 */
#include <wchar.h>
#include <wctype.h>

#include "consspaceobject.h"
#include "debug.h"
#include "dump.h"
#include "print.h"

/**
 * the controlling flags for `debug_print`; set in `init.c`, q.v.
 */
int verbosity = 0;

/**
 * print this debug `message` to stderr, if `verbosity` matches `level`.
 * `verbosity is a set of flags, see debug_print.h; so you can
 * turn debugging on for only one part of the system.
 */
void debug_print( wchar_t *message, int level ) {
#ifdef DEBUG
    if ( level & verbosity ) {
        fputws( message, stderr );
    }
#endif
}

/**
 * print the object indicated by this `pointer` to stderr, if `verbosity`
 * matches `level`.`verbosity is a set of flags, see debug_print.h; so you can
 * turn debugging on for only one part of the system.
 */
void debug_print_object( struct cons_pointer pointer, int level ) {
#ifdef DEBUG
    if ( level & verbosity ) {
        print( stderr, pointer );
    }
#endif
}

/**
 * Like `dump_object`, q.v., but protected by the verbosity mechanism.
 */
void debug_dump_object( struct cons_pointer pointer, int level ) {
#ifdef DEBUG
    if ( level & verbosity ) {
        dump_object( stderr, pointer );
    }
#endif
}
