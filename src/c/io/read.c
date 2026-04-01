/**
 *  read.c
 *
 *  Read basic Lisp objects..This is :bootstrap layer print; it needs to be
 *  able to read characters, symbols, integers, lists and dotted pairs. I
 *  don't think it needs to be able to read anything else. It must, however,
 *  take a readtable as argument and expand reader macros.
 *
 *
 * (c) 2017 Simon Brooke <simon@journeyman.cc>
 * Licensed under GPL version 2.0, or, at your option, any later version.
 */

#include <math.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>

/*
 * wide characters
 */
#include <wchar.h>
#include <wctype.h>

#include "debug.h"

#include "io/io.h"
#include "io/read.h"
#include "memory/node.h"
#include "memory/pointer.h"
#include "memory/pso.h"
#include "memory/pso2.h"
#include "memory/tags.h"

#include "payloads/integer.h"
#include "payloads/read_stream.h"

#include "ops/assoc.h"
#include "ops/reverse.h"
#include "ops/stack_ops.h"
#include "ops/string_ops.h"
#include "ops/truth.h"

// TODO: what I've copied from 0.0.6 is *wierdly* over-complex for just now.
// I think I'm going to essentially delete all this and start again. We need
// to be able to despatch on readttables, and the initial readtable functions
// don't need to be written in Lisp.
//
// In the long run a readtable ought to be a hashtable, but for now an assoc
// list will do.
//
// A readtable function is a Lisp function so needs the stackframe and the
// environment. Other arguments (including the output stream) should be passed
// in the argument, so I think the first arg in the frame is the character read;
// the next is the input stream; the next is the readtable, if any.

/*
 * for the time being things which may be read are: 
 * * integers
 * * lists 
 * * atoms
 * * dotted pairs
 */

/**
 * An example wrapper function while I work out how I'm going to do this.
 *
 * For this and all other `read` functions unless documented otherwise, the
 * arguments in the frame are expected to be:
 *
 * 0. The input stream to read from;
 * 1. The read table currently in use;
 * 2. The character most recently read from that stream.
 */
struct pso_pointer read_example( struct pso_pointer frame_pointer,
                            struct pso_pointer env) {
	struct pso4 *frame = pointer_to_pso4( frame_pointer);
	struct pso_pointer stream = fetch_arg( frame, 0);
	struct pso_pointer readtable = fetch_arg( frame, 1);
	struct pso_pointer character = fetch_arg( frame, 2);
	struct pso_pointer result = nil;

	return result;
}

/**
 * @brief Read one integer from the stream and return it.
 *
 * For this and all other `read` functions unless documented otherwise, the
 * arguments in the frame are expected to be:
 *
 * 0. The input stream to read from;
 * 1. The read table currently in use;
 * 2. The character most recently read from that stream.
 */
struct pso_pointer read_number( struct pso_pointer frame_pointer,
                            struct pso_pointer env) {
	struct pso4 *frame = pointer_to_pso4( frame_pointer);
	struct pso_pointer stream = fetch_arg( frame, 0);
	struct pso_pointer readtable = fetch_arg( frame, 1);
	struct pso_pointer character = fetch_arg( frame, 2);
	struct pso_pointer result = nil;

	int base = 10;
	// TODO: should check for *read-base* in the environment
	int64_t value = 0;

	if (readp(stream)) {
		if (nilp( character)) {
			character = get_character( stream);
		}
		wchar_t c = nilp(character) ? 0 :
				pointer_to_object( character)->payload.character.character;

		URL_FILE * input = pointer_to_object(stream)->payload.stream.stream;
		for ( ; iswdigit( c );
				  c = url_fgetwc( input ) ){
			value = (value * base) + ((int)c - (int)L'0');
		}

		url_ungetwc( c, input);
		result = make_integer( value);
	} // else exception?

	return result;
}

struct pso_pointer read_symbol( struct pso_pointer frame_pointer,
                            struct pso_pointer env) {
	struct pso4 *frame = pointer_to_pso4( frame_pointer);
	struct pso_pointer stream = fetch_arg( frame, 0);
	struct pso_pointer readtable = fetch_arg( frame, 1);
	struct pso_pointer character = fetch_arg( frame, 2);
	struct pso_pointer result = nil;

	if (readp(stream)) {
		if (nilp( character)) {
			character = get_character( stream);
		}

		wchar_t c = nilp(character) ? 0 :
				pointer_to_object( character)->payload.character.character;

		URL_FILE * input = pointer_to_object(stream)->payload.stream.stream;
		for ( ; iswalnum( c );
				  c = url_fgetwc( input ) ){
			result = make_string_like_thing(c, result, SYMBOLTAG);
		}

		url_ungetwc( c, input);
		result = reverse( result);
	}

	return result;
}

/**
 * @brief Read the next object on the input stream indicated by this stack
 * frame, and return a pso_pointer to the object read.
 *
 * For this and all other `read` functions unless documented otherwise, the
 * arguments in the frame are expected to be:
 *
 * 0. The input stream to read from;
 * 1. The read table currently in use;
 * 2. The character most recently read from that stream.
 */
struct pso_pointer read( struct pso_pointer frame_pointer,
                          struct pso_pointer env ) {
	struct pso4 *frame = pointer_to_pso4( frame_pointer);
	struct pso_pointer stream = fetch_arg( frame, 0);
	struct pso_pointer readtable = fetch_arg( frame, 1);
	struct pso_pointer character = fetch_arg( frame, 2);

	struct pso_pointer result = nil;

	if (nilp(stream)) {
		stream = make_read_stream( file_to_url_file(stdin), nil);
	}

	if (nilp( readtable)) {
		// TODO: check for the value of `*read-table*` in the environment and
		// use that.
	}

	if (nilp( character)) {
		character = get_character( stream);
	}

	struct pso_pointer readmacro = assoc(character, readtable);

	if (!nilp( readmacro)) {
		// invoke the read macro on the stream
	} else if (readp( stream) && characterp(character)) {
		wchar_t c = pointer_to_object( character)->payload.character.character;
		URL_FILE * input = pointer_to_object(stream)->payload.stream.stream;

        switch ( c ) {
            case ';':
                for ( c = url_fgetwc( input ); c != '\n';
                      c = url_fgetwc( input ) );
                /* skip all characters from semi-colon to the end of the line */
                break;
            case EOF:
//                result = throw_exception( c_string_to_lisp_symbol( L"read" ),
//                                          c_string_to_lisp_string
//                                          ( L"End of input while reading" ),
//                                          frame_pointer );
                break;
            default:
               struct pso_pointer next = make_frame( frame_pointer, stream, readtable, make_character(c));
               inc_ref( next);
               if ( iswdigit( c ) ) {
                    result =
                        read_number( next, env );
                } else if ( iswalpha( c ) ) {
                    result = read_symbol( next, env );
                } else {
//                    result =
//                        throw_exception( c_string_to_lisp_symbol( L"read" ),
//                                         make_cons( c_string_to_lisp_string
//                                                    ( L"Unrecognised start of input character" ),
//                                                    make_string( c, NIL ) ),
//                                         frame_pointer );
                }
               dec_ref( next);
                break;
        }
	}

	return result;
}
