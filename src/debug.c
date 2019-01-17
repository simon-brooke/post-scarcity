/**
 * debug.c
 *
 * Better debug log messages.
 *
 * (c) 2017 Simon Brooke <simon@journeyman.cc>
 * Licensed under GPL version 2.0, or, at your option, any later version.
 */

#include <ctype.h>
#include <stdarg.h>
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
        fwide( stderr, 1 );
        fputws( message, stderr );
    }
#endif
}

/**
 * stolen from https://stackoverflow.com/questions/11656241/how-to-print-uint128-t-number-using-gcc
 */
void debug_print_128bit( __int128_t n, int level ) {
  #ifdef DEBUG
  if ( level & verbosity ) {
    if (n == 0) {
      fwprintf(stderr, L"0");
    } else {
      char str[40] = {0}; // log10(1 << 128) + '\0'
      char *s = str + sizeof(str) - 1; // start at the end
      while (n != 0) {
        if (s == str) return; // never happens

        *--s = "0123456789"[n % 10]; // save last digit
        n /= 10;                     // drop it
      }
      fwprintf(stderr, L"%s", s);
    }
  }
  #endif
}

/**
 * print a line feed to stderr, if `verbosity` matches `level`.
 * `verbosity is a set of flags, see debug_print.h; so you can
 * turn debugging on for only one part of the system.
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
 * `wprintf` adapted for the debug logging system. Print to stderr only
 * `verbosity` matches `level`. All other arguments as for `wprintf`.
 */
void debug_printf( int level, wchar_t *format, ... ) {
#ifdef DEBUG
    if ( level & verbosity ) {
        fwide( stderr, 1 );
        va_list( args );
        va_start( args, format );
        vfwprintf( stderr, format, args );
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
        fwide( stderr, 1 );
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
        fwide( stderr, 1 );
        dump_object( stderr, pointer );
    }
#endif
}
