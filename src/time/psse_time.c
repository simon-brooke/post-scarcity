/*
 * psse_time.c
 *
 * Bare bones of PSSE time. See issue #16.
 *
 * (c) 2019 Simon Brooke <simon@journeyman.cc>
 * Licensed under GPL version 2.0, or, at your option, any later version.
 */

#include <stdlib.h>
#include <string.h>
#include <time.h>
/*
 * wide characters
 */
#include <wchar.h>
#include <wctype.h>

#include "conspage.h"
#include "consspaceobject.h"
#include "integer.h"
#include "psse_time.h"
#define _GNU_SOURCE

#define seconds_per_year 31557600L

/**
 * PSSE Lisp epoch is 14 Bn years, or 441,806,400,000,000,000 seconds, before
 * the UNIX epoch; the value in microseconds will break the C reader.
 */
unsigned __int128 epoch_offset = ((__int128)(seconds_per_year * 1000000000L) *
                                  (__int128)(14L * 1000000000L));

/**
 * Return the UNIX time value which represents this time, if it falls within
 * the period representable in UNIX time, or zero otherwise.
 */
long int lisp_time_to_unix_time(struct cons_pointer t) {
    long int result = 0;

    if (timep( t)) {
        unsigned __int128 value = pointer2cell(t).payload.time.value;

        if (value > epoch_offset) { // \todo && value < UNIX time rollover
            result = ((value - epoch_offset) / 1000000000);
        }
    }

    return result;
}

unsigned __int128 unix_time_to_lisp_time( time_t t) {
    unsigned __int128 result = epoch_offset + (t * 1000000000);

    return result;
}

struct cons_pointer make_time( struct cons_pointer integer_or_nil) {
    struct cons_pointer pointer = allocate_cell( TIMETV );
    struct cons_space_object *cell = &pointer2cell( pointer );

    if (integerp(integer_or_nil)) {
        cell->payload.time.value = pointer2cell(integer_or_nil).payload.integer.value;
        // \todo: if integer is a bignum, deal with it.
    } else {
        cell->payload.time.value = unix_time_to_lisp_time( time(NULL));
    }

    return pointer;
}

/**
 * Function; return a time representation of the first argument in the frame;
 * further arguments are ignored.
 *
 * * (time integer_or_nil)
 *
 * @param frame my stack_frame.
 * @param frame_pointer a pointer to my stack_frame.
 * @param env my environment.
 * @return a lisp time; if `integer_or_nil` is an integer, return a time which
 * is that number of microseconds after the notional big bang; else the current
 * time.
 */
struct cons_pointer lisp_time( struct stack_frame *frame, struct cons_pointer frame_pointer,
           struct cons_pointer env ) {
    return make_time( frame->arg[0]);
}

/**
 * This is temporary, for bootstrapping.
 */
struct cons_pointer time_to_string( struct cons_pointer pointer) {
    struct cons_pointer result = NIL;
    long int t = lisp_time_to_unix_time(pointer);

    if ( t != 0) {
        char * bytes = ctime(&t);
        int l = strlen(bytes) + 1;
        wchar_t buffer[ l];

        mbstowcs( buffer, bytes, l);
        result = c_string_to_lisp_string( buffer);
    }

    return result;
}
