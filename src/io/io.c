/*
 * io.c
 *
 * Communication between PSSE and the outside world, via libcurl.
 *
 * (c) 2019 Simon Brooke <simon@journeyman.cc>
 * Licensed under GPL version 2.0, or, at your option, any later version.
 */

#include <stdlib.h>

#include "conspage.h"
#include "consspaceobject.h"
#include "debug.h"
#include "fopen.h"
#include "lispops.h"

/**
 * Convert this lisp string-like-thing (also works for symbols, and, later
 * keywords) into a UTF-8 string. NOTE that the returned value has been
 * malloced and must be freed. TODO: candidate to moving into a utilities
 * file.
 *
 * @param s the lisp string or symbol;
 * @return the c string.
 */
char *lisp_string_to_c_string( struct cons_pointer s ) {
    char *result = NULL;

    if ( stringp( s ) || symbolp( s ) ) {
        int len = 0;

        for ( struct cons_pointer c = s; !nilp( c );
              c = pointer2cell( c ).payload.string.cdr ) {
            len++;
        }

        wchar_t *buffer = calloc( len + 1, sizeof( wchar_t ) );
        /* worst case, one wide char = four utf bytes */
        result = calloc( ( len * 4 ) + 1, sizeof( char ) );

        int i = 0;
        for ( struct cons_pointer c = s; !nilp( c );
              c = pointer2cell( c ).payload.string.cdr ) {
            buffer[i++] = pointer2cell( c ).payload.string.character;
        }

        wcstombs( result, buffer, len );
        free( buffer );
    }

    return result;
}

/**
 * Function, sort-of: close the file indicated by my first arg, and return
 * nil. If the first arg is not a stream, does nothing. All other args are
 * ignored.
 *
 * * (close stream)
 *
 * @param frame my stack_frame.
 * @param frame_pointer a pointer to my stack_frame.
 * @param env my environment.
 * @return T if the stream was successfully closed, else NIL.
 */
struct cons_pointer
lisp_close( struct stack_frame *frame, struct cons_pointer frame_pointer,
            struct cons_pointer env ) {
    struct cons_pointer result = NIL;

    if ( readp( frame->arg[0] ) || writep( frame->arg[0] ) ) {
        if ( url_fclose( pointer2cell( frame->arg[0] ).payload.stream.stream )
             == 0 ) {
            result = TRUE;
        }
    }

    return result;
}

/**
 * Function: return a stream open on the URL indicated by the first argument;
 * if a second argument is present and is non-nil, open it for reading. At
 * present, further arguments are ignored and there is no mechanism to open
 * to append, or error if the URL is faulty or indicates an unavailable
 * resource.
 *
 * * (read-char stream)
 *
 * @param frame my stack_frame.
 * @param frame_pointer a pointer to my stack_frame.
 * @param env my environment.
 * @return a string of one character, namely the next available character
 * on my stream, if any, else NIL.
 */
struct cons_pointer
lisp_open( struct stack_frame *frame, struct cons_pointer frame_pointer,
           struct cons_pointer env ) {
    struct cons_pointer result = NIL;

    if ( stringp( frame->arg[0] ) ) {
        char *url = lisp_string_to_c_string( frame->arg[0] );

        if ( nilp( frame->arg[1] ) ) {
            result = make_read_stream( url_fopen( url, "r" ) );
        } else {
            // TODO: anything more complex is a problem for another day.
            result = make_write_stream( url_fopen( url, "w" ) );
        }

        free( url );
    }

    return result;
}

/**
 * Function: return the next character from the stream indicated by arg 0;
 * further arguments are ignored.
 *
 * * (read-char stream)
 *
 * @param frame my stack_frame.
 * @param frame_pointer a pointer to my stack_frame.
 * @param env my environment.
 * @return a string of one character, namely the next available character
 * on my stream, if any, else NIL.
 */
struct cons_pointer
lisp_read_char( struct stack_frame *frame, struct cons_pointer frame_pointer,
                struct cons_pointer env ) {
    struct cons_pointer result = NIL;

    if ( readp( frame->arg[0] ) ) {
        result =
            make_string( url_fgetwc
                         ( pointer2cell( frame->arg[0] ).payload.stream.
                           stream ), NIL );
    }

    return result;
}

/**
 * Function: return a string representing all characters from the stream
 * indicated by arg 0; further arguments are ignored.
 *
 * * (slurp stream)
 *
 * @param frame my stack_frame.
 * @param frame_pointer a pointer to my stack_frame.
 * @param env my environment.
 * @return a string of one character, namely the next available character
 * on my stream, if any, else NIL.
 */
struct cons_pointer
lisp_slurp( struct stack_frame *frame, struct cons_pointer frame_pointer,
            struct cons_pointer env ) {
    struct cons_pointer result = NIL;
    struct cons_pointer cdr = NIL;

    if ( readp( frame->arg[0] ) ) {
        URL_FILE *stream = pointer2cell( frame->arg[0] ).payload.stream.stream;

        for ( wint_t c = url_fgetwc( stream ); c != -1;
              c = url_fgetwc( stream ) ) {
            cdr = make_string( ( ( wchar_t ) c ), cdr );

            if ( nilp( result ) ) {
                result = cdr;
            }
        }
    }

    return result;
}
