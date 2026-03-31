/*
 * io.c
 *
 * Communication between PSSE and the outside world, via libcurl. NOTE
 * that this file destructively changes metadata on URL connections,
 * because the metadata is not available until the stream has been read
 * from. It would be better to find a workaround!
 *
 * (c) 2019 Simon Brooke <simon@journeyman.cc>
 * Licensed under GPL version 2.0, or, at your option, any later version.
 */

#include <grp.h>
#include <langinfo.h>
#include <pwd.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <uuid/uuid.h>
/*
 * wide characters
 */
#include <wchar.h>
#include <wctype.h>

#include <curl/curl.h>

//#include "arith/integer.h"
#include "debug.h"
#include "io/fopen.h"
#include "io/io.h"

#include "memory/node.h"
#include "memory/pointer.h"
#include "memory/pso2.h"
#include "memory/pso4.h"
#include "memory/tags.h"

// #include "ops/intern.h"
// #include "ops/lispops.h"

#include "ops/stack_ops.h"
#include "ops/string_ops.h"
#include "ops/truth.h"

#include "payloads/cons.h"
#include "payloads/exception.h"
#include "payloads/integer.h"
#include "payloads/stack.h"

#include "utils.h"

/**
 * The sharing hub for all connections. TODO: Ultimately this probably doesn't
 * work for a multi-user environment and we will need one sharing hub for each
 * user, or else we will need to not share at least cookies and ssl sessions.
 */
CURLSH *io_share;

/**
 * @brief bound to the Lisp string representing C_IO_IN in initialisation.
 */
struct pso_pointer lisp_io_in;
/**
 * @brief bound to the Lisp string representing C_IO_OUT in initialisation.
 */
struct pso_pointer lisp_io_out;


/**
 * Allow a one-character unget facility. This may not be enough - we may need
 * to allocate a buffer.
 */
wint_t ungotten = 0;

/**
 * Initialise the I/O subsystem.
 *
 * @return 0 on success; any other value means failure.
 */
int io_init(  ) {
    int result = curl_global_init( CURL_GLOBAL_SSL );

    io_share = curl_share_init(  );

    if ( result == 0 ) {
        curl_share_setopt( io_share, CURLSHOPT_SHARE, CURL_LOCK_DATA_CONNECT );
        curl_share_setopt( io_share, CURLSHOPT_SHARE, CURL_LOCK_DATA_COOKIE );
        curl_share_setopt( io_share, CURLSHOPT_SHARE, CURL_LOCK_DATA_DNS );
        curl_share_setopt( io_share, CURLSHOPT_SHARE,
                           CURL_LOCK_DATA_SSL_SESSION );
        curl_share_setopt( io_share, CURLSHOPT_SHARE, CURL_LOCK_DATA_PSL );
    }

    return result;
}

/**
 * Convert this lisp string-like-thing (also works for symbols, and, later
 * keywords) into a UTF-8 string. NOTE that the returned value has been
 * malloced and must be freed. TODO: candidate to moving into a utilities
 * file.
 *
 * @param s the lisp string or symbol;
 * @return the c string.
 */
char *lisp_string_to_c_string( struct pso_pointer s ) {
    char *result = NULL;

    if ( stringp( s ) || symbolp( s ) ) {
        int len = 0;

        for ( struct pso_pointer c = s; !nilp( c );
              c = cdr(c)) {
            len++;
        }

        wchar_t *buffer = calloc( len + 1, sizeof( wchar_t ) );
        /* worst case, one wide char = four utf bytes */
        result = calloc( ( len * 4 ) + 1, sizeof( char ) );

        int i = 0;
        for ( struct pso_pointer c = s; !nilp( c );
              c = cdr(c)) {
            buffer[i++] = pointer_to_object( c )->payload.string.character;
        }

        wcstombs( result, buffer, len );
        free( buffer );
    }

    debug_print( L"lisp_string_to_c_string( ", DEBUG_IO, 0 );
    debug_print_object( s, DEBUG_IO , 0);
    debug_printf( DEBUG_IO, 0, L") => '%s'\n", result );

    return result;
}


/**
 * given this file handle f, return a new url_file handle wrapping it.
 *
 * @param f the file to be wrapped;
 * @return the new handle, or null if no such handle could be allocated.
 */
URL_FILE *file_to_url_file( FILE *f ) {
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
wint_t url_fgetwc( URL_FILE *input ) {
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

                    debug_print( L"url_fgetwc: about to call url_fgets\n",
                                 DEBUG_IO, 0 );
                    url_fgets( cbuff, 2, input );
                    debug_print( L"url_fgetwc: back from url_fgets\n",
                                 DEBUG_IO, 0 );
                    int c = ( int ) cbuff[0];
                    // TODO: risk of reading off cbuff?
                    debug_printf( DEBUG_IO, 0,
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
                    if ( c <= 0xf7 ) {
                        count = 1;
                    } else if ( c >= 0xc2 && c <= 0xdf ) {
                        count = 2;
                    } else if ( c >= 0xe0 && c <= 0xef ) {
                        count = 3;
                    } else if ( c >= 0xf0 && c <= 0xff ) {
                        count = 4;
                    }

                    if ( count > 1 ) {
                        url_fgets( ( char * ) &cbuff[1], count, input );
                    }
                    mbstowcs( wbuff, cbuff, 2 );  //(char *)(&input->buffer[input->buffer_pos]), 1 );
                    result = wbuff[0];

                    free( wbuff );
                    free( cbuff );
                }
                break;
            case CFTYPE_NONE:
                break;
        }
    }

    debug_printf( DEBUG_IO, 0, L"url_fgetwc returning %d (%C)\n", result,
                  result );
    return result;
}

wint_t url_ungetwc( wint_t wc, URL_FILE *input ) {
    wint_t result = -1;

    switch ( input->type ) {
        case CFTYPE_FILE:
            fwide( input->handle.file, 1 ); /* wide characters */
            result = ungetwc( wc, input->handle.file ); /* passthrough */
            break;

        case CFTYPE_CURL:{
                ungotten = wc;
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
 * @param frame my stack frame.
 * @param frame_pointer a pointer to my stack frame.
 * @param env my environment.
 * @return T if the stream was successfully closed, else nil.
 */
struct pso_pointer
lisp_close( struct pso4 *frame, struct pso_pointer frame_pointer,
            struct pso_pointer env ) {
    struct pso_pointer result = nil;

    if ( readp( fetch_arg( frame, 0) ) || writep( fetch_arg( frame, 0) ) ) {
        if ( url_fclose( pointer_to_object( fetch_arg( frame, 0) )->payload.stream.stream )
             == 0 ) {
            result = t;
        }
    }

    return result;
}

struct pso_pointer add_meta_integer( struct pso_pointer meta, wchar_t *key,
                                      long int value ) {
    return
        cons( cons
                   ( c_string_to_lisp_keyword( key ),
                     make_integer( value ) ), meta );
}

struct pso_pointer add_meta_string( struct pso_pointer meta, wchar_t *key,
                                     char *value ) {
    value = trim( value );
    wchar_t buffer[strlen( value ) + 1];
    mbstowcs( buffer, value, strlen( value ) + 1 );

    return cons( cons( c_string_to_lisp_keyword( key ),
                                 c_string_to_lisp_string( buffer ) ), meta );
}

struct pso_pointer add_meta_time( struct pso_pointer meta, wchar_t *key,
                                   time_t *value ) {
    /* I don't yet have a concept of a date-time object, which is a
     * bit of an oversight! */
    char datestring[256];

    strftime( datestring,
              sizeof( datestring ),
              nl_langinfo( D_T_FMT ), localtime( value ) );

    return add_meta_string( meta, key, datestring );
}

/**
 * Callback to assemble metadata for a URL stream. This is naughty because
 * it modifies data, but it's really the only way to create metadata.
 */
static size_t write_meta_callback( char *string, size_t size, size_t nmemb,
                                   struct pso_pointer stream ) {
    struct pso2 *cell = pointer_to_object( stream );

    // TODO: reimplement

    /* make a copy of the string that we can destructively change */
    // char *s = calloc( strlen( string ), sizeof( char ) );

    // strcpy( s, string );

    // if ( check_tag( cell, READTV) ||
    //      check_tag( cell, WRITETV) ) {
    //     int offset = index_of( ':', s );

    //     if ( offset != -1 ) {
    //         s[offset] = ( char ) 0;
    //         char *name = trim( s );
    //         char *value = trim( &s[++offset] );
    //         wchar_t wname[strlen( name )];

    //         mbstowcs( wname, name, strlen( name ) + 1 );

    //         cell->payload.stream.meta =
    //             add_meta_string( cell->payload.stream.meta, wname, value );

    //         debug_printf( DEBUG_IO,
    //                       L"write_meta_callback: added header '%s': value '%s'\n",
    //                       name, value );
    //     } else if ( strncmp( "HTTP", s, 4 ) == 0 ) {
    //         int offset = index_of( ' ', s );
    //         char *value = trim( &s[offset] );

    //         cell->payload.stream.meta =
    //             add_meta_integer( add_meta_string
    //                               ( cell->payload.stream.meta, L"status",
    //                                 value ), L"status-code", strtol( value,
    //                                                                  NULL,
    //                                                                  10 ) );

    //         debug_printf( DEBUG_IO,
    //                       L"write_meta_callback: added header 'status': value '%s'\n",
    //                       value );
    //     } else {
    //         debug_printf( DEBUG_IO,
    //                       L"write_meta_callback: header passed with no colon: '%s'\n",
    //                       s );
    //     }
    // } else {
    //     debug_print
    //         ( L"Pointer passed to write_meta_callback did not point to a stream: ",
    //           DEBUG_IO );
    //     debug_dump_object( stream, DEBUG_IO );
    // }

    // free( s );
    return 0; // strlen( string );
}

void collect_meta( struct pso_pointer stream, char *url ) {
    struct pso2 *cell = pointer_to_object( stream );
    URL_FILE *s = pointer_to_object( stream )->payload.stream.stream;
    struct pso_pointer meta =
        add_meta_string( cell->payload.stream.meta, L"url", url );
    struct stat statbuf;
    int result = stat( url, &statbuf );
    struct passwd *pwd;
    struct group *grp;

    switch ( s->type ) {
        case CFTYPE_NONE:
            break;
        case CFTYPE_FILE:
            if ( result == 0 ) {
                if ( ( pwd = getpwuid( statbuf.st_uid ) ) != NULL ) {
                    meta = add_meta_string( meta, L"owner", pwd->pw_name );
                } else {
                    meta = add_meta_integer( meta, L"owner", statbuf.st_uid );
                }

                if ( ( grp = getgrgid( statbuf.st_gid ) ) != NULL ) {
                    meta = add_meta_string( meta, L"group", grp->gr_name );
                } else {
                    meta = add_meta_integer( meta, L"group", statbuf.st_gid );
                }

                meta =
                    add_meta_integer( meta, L"size",
                                      ( intmax_t ) statbuf.st_size );

                meta = add_meta_time( meta, L"modified", &statbuf.st_mtime );
            }
            break;
        case CFTYPE_CURL:
            curl_easy_setopt( s->handle.curl, CURLOPT_VERBOSE, 1L );
            curl_easy_setopt( s->handle.curl, CURLOPT_HEADERFUNCTION,
                              write_meta_callback );
            curl_easy_setopt( s->handle.curl, CURLOPT_HEADERDATA, stream );
            break;
    }

    /* this is destructive change before the cell is released into the
     * wild, and consequently permissible, just. */
    cell->payload.stream.meta = meta;
}

/**
 * Resutn the current default input, or of `inputp` is false, output stream from
 * this `env`ironment.
 */
struct pso_pointer get_default_stream( bool inputp, struct pso_pointer env ) {
    struct pso_pointer result = nil;
    // struct pso_pointer stream_name = inputp ? lisp_io_in : lisp_io_out;

    // result = c_assoc( stream_name, env );

    return result;
}


/**
 * Function: return a stream open on the URL indicated by the first argument;
 * if a second argument is present and is non-nil, open it for writing. At
 * present, further arguments are ignored and there is no mechanism to open
 * to append, or error if the URL is faulty or indicates an unavailable
 * resource.
 *
 * * (open url)
 *
 * @param frame my stack frame.
 * @param frame_pointer a pointer to my stack frame.
 * @param env my environment.
 * @return a string of one character, namely the next available character
 * on my stream, if any, else nil.
 */
struct pso_pointer
lisp_open( struct pso4 *frame, struct pso_pointer frame_pointer,
           struct pso_pointer env ) {
    struct pso_pointer result = nil;

    // if ( stringp( fetch_arg( frame, 0) ) ) {
    //     char *url = lisp_string_to_c_string( fetch_arg( frame, 0) );

    //     if ( nilp( fetch_arg( frame, 1) ) ) {
    //         URL_FILE *stream = url_fopen( url, "r" );

    //         debug_printf( DEBUG_IO, 0, 
    //                       L"lisp_open: stream @ %ld, stream type = %d, stream handle = %ld\n",
    //                       ( long int ) &stream, ( int ) stream->type,
    //                       ( long int ) stream->handle.file );

    //         switch ( stream->type ) {
    //             case CFTYPE_NONE:
    //                 return
    //                     make_exception( c_string_to_lisp_string
    //                                     ( L"Could not open stream" ),
    //                                     frame_pointer , nil );
    //                 break;
    //             case CFTYPE_FILE:
    //                 if ( stream->handle.file == NULL ) {
    //                     return
    //                         make_exception( c_string_to_lisp_string
    //                                         ( L"Could not open file" ),
    //                                         frame_pointer , nil);
    //                 }
    //                 break;
    //             case CFTYPE_CURL:
    //                 /* can't tell whether a URL is bad without reading it */
    //                 break;
    //         }

    //         result = make_read_stream( stream, nil );
    //     } else {
    //         // TODO: anything more complex is a problem for another day.
    //         URL_FILE *stream = url_fopen( url, "w" );
    //         result = make_write_stream( stream, nil );
    //     }

    //     if ( pointer_to_object( result )->payload.stream.stream == NULL ) {
    //         result = nil;
    //     } else {
    //         collect_meta( result, url );
    //     }

    //     free( url );
    // }

    return result;
}

/**
 * Function: return the next character from the stream indicated by arg 0;
 * further arguments are ignored.
 *
 * * (read-char stream)
 *
 * @param frame my stack frame.
 * @param frame_pointer a pointer to my stack frame.
 * @param env my environment.
 * @return a string of one character, namely the next available character
 * on my stream, if any, else nil.
 */
struct pso_pointer
lisp_read_char( struct pso4 *frame, struct pso_pointer frame_pointer,
                struct pso_pointer env ) {
    struct pso_pointer result = nil;

    if ( readp( fetch_arg( frame, 0) ) ) {
        result =
            make_string( url_fgetwc
                         ( pointer_to_object( fetch_arg( frame, 0) )->payload.
                           stream.stream ), nil );
    }

    return result;
}

/**
 * Function: return a string representing all characters from the stream
 * indicated by arg 0; further arguments are ignored.
 *
 * TODO: it should be possible to optionally pass a string URL to this function,
 *
 * * (slurp stream)
 *
 * @param frame my stack frame.
 * @param frame_pointer a pointer to my stack frame.
 * @param env my environment.
 * @return a string of one character, namely the next available character
 * on my stream, if any, else nil.
 */
struct pso_pointer
lisp_slurp( struct pso4 *frame, struct pso_pointer frame_pointer,
            struct pso_pointer env ) {
    struct pso_pointer result = nil;

    if ( readp( fetch_arg( frame, 0) ) ) {
        URL_FILE *stream = pointer_to_object( fetch_arg( frame, 0) )->payload.stream.stream;
        struct pso_pointer cursor = make_string( url_fgetwc( stream ), nil );
        result = cursor;

        for ( wint_t c = url_fgetwc( stream ); !url_feof( stream ) && c != 0;
              c = url_fgetwc( stream ) ) {
            debug_print( L"slurp: cursor is: ", DEBUG_IO, 0);
            debug_dump_object( cursor, DEBUG_IO, 0 );
            debug_print( L"; result is: ", DEBUG_IO, 0 );
            debug_dump_object( result, DEBUG_IO, 0 );
            debug_println( DEBUG_IO );

            struct pso2 *cell = pointer_to_object( cursor );
            cursor = make_string( ( wchar_t ) c, nil );
            cell->payload.string.cdr = cursor;
        }
    }

    return result;
}
