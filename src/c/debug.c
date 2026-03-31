/**
 *  debug.c
 *
 *  Post Scarcity Software Environment: debugging messages.
 *
 *  Print debugging output.
 *
 *
 *  (c) 2026 Simon Brooke <simon@journeyman.cc>
 *  Licensed under GPL version 2.0, or, at your option, any later version.
 */

#include <stdarg.h>

#include "debug.h"

int verbosity = 0;


/**
 * @brief print this debug `message` to stderr, if `verbosity` matches `level`.
 *
 * `verbosity` is a set of flags, see debug_print.h; so you can
 * turn debugging on for only one part of the system.
 *
 * NOTE THAT: contrary to behaviour in the 0.0.X prototypes, a line feed is
 * always printed before a debug_print message. Hopefully this will result
 * in clearer formatting.
 *
 * @param message The message to be printed, in *wide* (32 bit) characters. 
 * @param level a mask for `verbosity`. If a bitwise and of `verbosity` and 
 * `level`  is non-zero, print this `message`, else don't.
 * @param indent print `indent` spaces before the message.
 */
void debug_print( wchar_t *message, int level, int indent ) {
#ifdef DEBUG
    if ( level & verbosity ) {
        fwide( stderr, 1 );
        fputws( L"\n", stderr );
        for ( int i = 0; i < indent; i++ ) {
            fputws( L" ", stderr );
        }
        fputws( message, stderr );
    }
#endif
}

void debug_print_object( struct pso_pointer object, int level, int indent ) {
    // TODO: not yet implemented
}

void debug_dump_object( struct pso_pointer object, int level, int indent ) {
    // TODO: not yet implemented
}

/**
 * @brief print a 128 bit integer value to stderr, if `verbosity` matches `level`.
 *
 * `verbosity` is a set of flags, see debug_print.h; so you can
 * turn debugging on for only one part of the system.
 *
 * stolen from https://stackoverflow.com/questions/11656241/how-to-print-uint128-t-number-using-gcc
 *
 * @param n the large integer to print.
 * @param level  a mask for `verbosity`. If a bitwise and of `verbosity` and 
 * `level`  is non-zero, print this `message`, else don't.
 */
void debug_print_128bit( __int128_t n, int level ) {
#ifdef DEBUG
    if ( level & verbosity ) {
        if ( n == 0 ) {
            fwprintf( stderr, L"0" );
        } else {
            char str[40] = { 0 }; // log10(1 << 128) + '\0'
            char *s = str + sizeof( str ) - 1;  // start at the end
            while ( n != 0 ) {
                if ( s == str )
                    return;     // never happens

                *--s = "0123456789"[n % 10];  // save last digit
                n /= 10;        // drop it
            }
            fwprintf( stderr, L"%s", s );
        }
    }
#endif
}

/**
 * @brief print a line feed to stderr, if `verbosity` matches `level`.
 *
 * `verbosity` is a set of flags, see debug_print.h; so you can
 * turn debugging on for only one part of the system.
 *
 *  @param level a mask for `verbosity`. If a bitwise and of `verbosity` and 
 * `level`  is non-zero, print this `message`, else don't.
 */
void debug_println( int level ) {
#ifdef DEBUG
    if ( level & verbosity ) {
        fwide( stderr, 1 );
        fputws( L"\n", stderr );
    }
#endif
}


/**
 * @brief `wprintf` adapted for the debug logging system. 
 *
 * Print to stderr only if `verbosity` matches `level`. All other arguments
 * as for `wprintf`.
 *
 *  @param level a mask for `verbosity`. If a bitwise and of `verbosity` and 
 * `level`  is non-zero, print this `message`, else don't.
 * @param indent print `indent` spaces before the message.
 * @param format Format string in *wide characters*, but otherwise as used by
 * `printf` and friends.
 * 
 * Remaining arguments should match the slots in the format string.
 */
void debug_printf( int level, int indent, wchar_t *format, ... ) {
#ifdef DEBUG
    if ( level & verbosity ) {
        fwide( stderr, 1 );
        fputws( L"\n", stderr );
        for ( int i = 0; i < indent; i++ ) {
            fputws( L" ", stderr );
        }
        va_list( args );
        va_start( args, format );
        vfwprintf( stderr, format, args );
    }
#endif
}

// debug_dump_object, debug_print_binding, debug_print_exception, debug_print_object,
// not yet implemented but probably will be.
