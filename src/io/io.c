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
 * Allow a one-character unget facility. This may not be enough - we may need
 * to allocate a buffer.
 */
wint_t ungotten = 0;

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

    debug_print( L"lisp_string_to_c_string( ", DEBUG_IO );
    debug_print_object( s, DEBUG_IO );
    debug_printf( DEBUG_IO, L") => '%s'\n", result );

    return result;
}


/**
 * given this file handle f, return a new url_file handle wrapping it.
 *
 * @param f the file to be wrapped;
 * @return the new handle, or null if no such handle could be allocated.
 */
URL_FILE *file_to_url_file( FILE * f ) {
    URL_FILE *result = ( URL_FILE * ) malloc( sizeof( URL_FILE ) );

    if ( result != NULL ) {
        result->type = CFTYPE_FILE, result->handle.file = f;
    }

    return result;
}


/**
 * get one wide character from the buffer.
 *
 * @param file the stream to read from;
 * @return the next wide character on the stream, or zero if no more.
 */
wint_t url_fgetwc( URL_FILE * input ) {
    wint_t result = -1;

    if ( ungotten != 0 ) {
        /* TODO: not thread safe */
        result = ungotten;
        ungotten = 0;
    } else {
        switch ( input->type ) {
            case CFTYPE_FILE:
                fwide( input->handle.file, 1 ); /* wide characters */
                result = fgetwc( input->handle.file );  /* passthrough */
                break;

            case CFTYPE_CURL:{
                    char *cbuff =
                        calloc( sizeof( wchar_t ) + 2, sizeof( char ) );
                    wchar_t *wbuff = calloc( 2, sizeof( wchar_t ) );

                    size_t count = 0;

                    debug_print( L"url_fgetwc: about to call url_fgets\n", DEBUG_IO );
                    url_fgets( cbuff, 2, input );
                    debug_print( L"url_fgetwc: back from url_fgets\n", DEBUG_IO );
                    int c = ( int ) cbuff[0];
                    debug_printf( DEBUG_IO,
                                 L"url_fgetwc: cbuff is '%s'; (first) character = %d (%c)\n",
                                 cbuff, c, c & 0xf7 );
                    /* The value of each individual byte indicates its UTF-8 function, as follows:
                     *
                     * 00 to 7F hex (0 to 127): first and only byte of a sequence.
                     * 80 to BF hex (128 to 191): continuing byte in a multi-byte sequence.
                     * C2 to DF hex (194 to 223): first byte of a two-byte sequence.
                     * E0 to EF hex (224 to 239): first byte of a three-byte sequence.
                     * F0 to FF hex (240 to 255): first byte of a four-byte sequence.
                     */
                    if ( c <= 0x07 ) {
                        count = 1;
                    } else if ( c >= '0xc2' && c <= '0xdf' ) {
                        count = 2;
                    } else if ( c >= '0xe0' && c <= '0xef' ) {
                        count = 3;
                    } else if ( c >= '0xf0' && c <= '0xff' ) {
                        count = 4;
                    }

                    if ( count > 1 ) {
                        url_fgets( (char *)&cbuff[1], count, input );
                    }
                    mbstowcs( wbuff, cbuff, 1 );  //(char *)(&input->buffer[input->buffer_pos]), 1 );
                    result = wbuff[0];

                    free( wbuff );
                    free( cbuff );
                }
                break;
            case CFTYPE_NONE:
                break;
        }
    }

    debug_printf( DEBUG_IO, L"url_fgetwc returning %d (%C)\n", result,
                  result );
    return result;
}

wint_t url_ungetwc( wint_t wc, URL_FILE * input ) {
    wint_t result = -1;

    switch ( input->type ) {
        case CFTYPE_FILE:
            fwide( input->handle.file, 1 ); /* wide characters */
            result = ungetwc( wc, input->handle.file ); /* passthrough */
            break;

        case CFTYPE_CURL:{
                ungotten = wc;
//                wchar_t *wbuff = calloc( 2, sizeof( wchar_t ) );
//                char *cbuff = calloc( 5, sizeof( char ) );
//
//                wbuff[0] = wc;
//                result = wcstombs( cbuff, wbuff, 1 );
//
//                input->buffer_pos -= strlen( cbuff );
//
//                free( cbuff );
//                free( wbuff );
//
//                result = result > 0 ? wc : result;
                break;
        case CFTYPE_NONE:
                break;
            }
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

        if ( pointer2cell( result ).payload.stream.stream == NULL ) {
            result = NIL;
        }
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

    if ( readp( frame->arg[0] ) ) {
        URL_FILE *stream = pointer2cell( frame->arg[0] ).payload.stream.stream;
        struct cons_pointer cursor = make_string( url_fgetwc( stream ), NIL );
        result = cursor;

        for ( wint_t c = url_fgetwc( stream ); !url_feof( stream ) && c != 0;
              c = url_fgetwc( stream ) ) {
            debug_print( L"slurp: cursor is: ", DEBUG_IO );
            debug_dump_object( cursor, DEBUG_IO );
            debug_print( L"; result is: ", DEBUG_IO );
            debug_dump_object( result, DEBUG_IO );
            debug_println( DEBUG_IO );

            struct cons_space_object *cell = &pointer2cell( cursor );
            cursor = make_string( ( wchar_t ) c, NIL );
            cell->payload.string.cdr = cursor;
        }
    }

    return result;
}
