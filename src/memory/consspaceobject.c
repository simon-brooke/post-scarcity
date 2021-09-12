/*
 *  consspaceobject.c
 *
 *  Structures common to all cons space objects.
 *
 *
 *  (c) 2017 Simon Brooke <simon@journeyman.cc>
 *  Licensed under GPL version 2.0, or, at your option, any later version.
 */

#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
/*
 * wide characters
 */
#include <wchar.h>
#include <wctype.h>

#include "authorise.h"
#include "memory/conspage.h"
#include "memory/consspaceobject.h"
#include "debug.h"
#include "ops/intern.h"
#include "io/print.h"
#include "memory/stack.h"
#include "memory/vectorspace.h"

/**
 * True if the value of the tag on the cell at this `pointer` is this `value`,
 * or, if the tag of the cell is `VECP`, if the value of the tag of the
 * vectorspace object indicated by the cell is this `value`, else false.
 */
bool check_tag( struct cons_pointer pointer, uint32_t value ) {
    bool result = false;

    struct cons_space_object cell = pointer2cell( pointer );
    result = cell.tag.value == value;

    if ( result == false ) {
        if ( cell.tag.value == VECTORPOINTTV ) {
            struct vector_space_object *vec = pointer_to_vso( pointer );

            if ( vec != NULL ) {
                result = vec->header.tag.value == value;
            }
        }
    }

    return result;
}

/**
 * increment the reference count of the object at this cons pointer.
 *
 * You can't roll over the reference count. Once it hits the maximum
 * value you cannot increment further.
 * 
 * Returns the `pointer`.
 */
struct cons_pointer inc_ref( struct cons_pointer pointer ) {
    struct cons_space_object *cell = &pointer2cell( pointer );

    if ( cell->count < MAXREFERENCE ) {
        cell->count++;
    }

    return pointer;
}

/**
 * Decrement the reference count of the object at this cons pointer.
 *
 * If a count has reached MAXREFERENCE it cannot be decremented.
 * If a count is decremented to zero the cell should be freed.
 * 
 * Returns the `pointer`, or, if the cell has been freed, NIL.
 */
struct cons_pointer dec_ref( struct cons_pointer pointer ) {
    struct cons_space_object *cell = &pointer2cell( pointer );

    if ( cell->count > 0 ) {
        cell->count--;

        if ( cell->count == 0 ) {
            free_cell( pointer );
            pointer = NIL;
        }
    }

    return pointer;
}


/**
 * Get the Lisp type of the single argument.
 * @param pointer a pointer to the object whose type is requested.
 * @return As a Lisp string, the tag of the object which is at that pointer.
 */
struct cons_pointer c_type( struct cons_pointer pointer ) {
    struct cons_pointer result = NIL;
    struct cons_space_object cell = pointer2cell( pointer );

    if ( strncmp( ( char * ) &cell.tag.bytes, VECTORPOINTTAG, TAGLENGTH ) ==
         0 ) {
        struct vector_space_object *vec = pointer_to_vso( pointer );

        for ( int i = TAGLENGTH - 1; i >= 0; i-- ) {
            result =
                make_string( ( wchar_t ) vec->header.tag.bytes[i], result );
        }
    } else {
        for ( int i = TAGLENGTH - 1; i >= 0; i-- ) {
            result = make_string( ( wchar_t ) cell.tag.bytes[i], result );
        }
    }

    return result;
}

/**
 * Implementation of car in C. If arg is not a cons, or the current user is not
 * authorised to read it, does not error but returns nil.
 */
struct cons_pointer c_car( struct cons_pointer arg ) {
    struct cons_pointer result = NIL;

    if ( truep( authorised( arg, NIL ) ) && consp( arg ) ) {
        result = pointer2cell( arg ).payload.cons.car;
    }

    return result;
}

/**
 * Implementation of cdr in C. If arg is not a sequence, or the current user is
 * not authorised to read it,does not error but returns nil.
 */
struct cons_pointer c_cdr( struct cons_pointer arg ) {
    struct cons_pointer result = NIL;

    if ( truep( authorised( arg, NIL ) ) ) {
        struct cons_space_object *cell = &pointer2cell( arg );

        switch ( cell->tag.value ) {
            case CONSTV:
                result = cell->payload.cons.cdr;
                break;
            case KEYTV:
            case STRINGTV:
            case SYMBOLTV:
                result = cell->payload.string.cdr;
                break;
        }
    }

    return result;
}

/**
 * Implementation of `length` in C. If arg is not a cons, does not error but returns 0.
 */
int c_length( struct cons_pointer arg ) {
    int result = 0;

    for ( struct cons_pointer c = arg; !nilp( c ); c = c_cdr( c ) ) {
        result++;
    }

    return result;
}


/**
 * Construct a cons cell from this pair of pointers.
 */
struct cons_pointer make_cons( struct cons_pointer car,
                               struct cons_pointer cdr ) {
    struct cons_pointer pointer = NIL;

    pointer = allocate_cell( CONSTV );

    struct cons_space_object *cell = &pointer2cell( pointer );

    inc_ref( car );
    inc_ref( cdr );
    cell->payload.cons.car = car;
    cell->payload.cons.cdr = cdr;

    return pointer;
}

/**
 * Construct an exception cell.
 * @param message should be a lisp string describing the problem, but actually any cons pointer will do;
 * @param frame_pointer should be the pointer to the frame in which the exception occurred.
 */
struct cons_pointer make_exception( struct cons_pointer message,
                                    struct cons_pointer frame_pointer ) {
    struct cons_pointer result = NIL;
    struct cons_pointer pointer = allocate_cell( EXCEPTIONTV );
    struct cons_space_object *cell = &pointer2cell( pointer );

    inc_ref( message );
    inc_ref( frame_pointer );
    cell->payload.exception.payload = message;
    cell->payload.exception.frame = frame_pointer;

    result = pointer;

    return result;
}


/**
 * Construct a cell which points to an executable Lisp function.
 */
struct cons_pointer
make_function( struct cons_pointer meta, struct cons_pointer ( *executable )
                ( struct stack_frame *,
                  struct cons_pointer, struct cons_pointer ) ) {
    struct cons_pointer pointer = allocate_cell( FUNCTIONTV );
    struct cons_space_object *cell = &pointer2cell( pointer );
    inc_ref( meta );

    cell->payload.function.meta = meta;
    cell->payload.function.executable = executable;

    return pointer;
}

/**
 * Construct a lambda (interpretable source) cell
 */
struct cons_pointer make_lambda( struct cons_pointer args,
                                 struct cons_pointer body ) {
    struct cons_pointer pointer = allocate_cell( LAMBDATV );
    struct cons_space_object *cell = &pointer2cell( pointer );

    inc_ref( pointer );         /* this is a hack; I don't know why it's necessary to do this, but if I don't the cell gets freed */

    inc_ref( args );
    inc_ref( body );
    cell->payload.lambda.args = args;
    cell->payload.lambda.body = body;

    return pointer;
}

/**
 * Construct an nlambda (interpretable source) cell; to a
 * lambda as a special form is to a function.
 */
struct cons_pointer make_nlambda( struct cons_pointer args,
                                  struct cons_pointer body ) {
    struct cons_pointer pointer = allocate_cell( NLAMBDATV );

    inc_ref( pointer );         /* this is a hack; I don't know why it's necessary to do this, but if I don't the cell gets freed */

    struct cons_space_object *cell = &pointer2cell( pointer );
    inc_ref( args );
    inc_ref( body );
    cell->payload.lambda.args = args;
    cell->payload.lambda.body = body;

    return pointer;
}

/**
 * Return a hash value for this string like thing.
 * 
 * What's important here is that two strings with the same characters in the
 * same order should have the same hash value, even if one was created using
 * `"foobar"` and the other by `(append "foo" "bar")`. I *think* this function 
 * has that property. I doubt that it's the most efficient hash function to 
 * have that property.
 * 
 * returns 0 for things which are not string like.
 */
uint32_t calculate_hash( wint_t c, struct cons_pointer ptr ) {
    struct cons_space_object *cell = &pointer2cell( ptr );
    uint32_t result = 0;

    switch ( cell->tag.value ) {
        case KEYTV:
        case STRINGTV:
        case SYMBOLTV:
            if ( nilp( cell->payload.string.cdr ) ) {
                result = ( uint32_t ) c;
            } else {
                result = ( ( uint32_t ) c *
                           cell->payload.string.hash ) & 0xffffffff;
            }
            break;
    }

    return result;
}

/**
 * Construct a string from this character (which later will be UTF) and
 * this tail. A string is implemented as a flat list of cells each of which
 * has one character and a pointer to the next; in the last cell the
 * pointer to next is NIL.
 */
struct cons_pointer
make_string_like_thing( wint_t c, struct cons_pointer tail, uint32_t tag ) {
    struct cons_pointer pointer = NIL;

    if ( check_tag( tail, tag ) || check_tag( tail, NILTV ) ) {
        pointer = allocate_cell( tag );
        struct cons_space_object *cell = &pointer2cell( pointer );

        inc_ref( tail );
        cell->payload.string.character = c;
        cell->payload.string.cdr.page = tail.page;
        /* \todo There's a problem here. Sometimes the offsets on
         * strings are quite massively off. Fix is probably
         * cell->payload.string.cdr = tail */
        cell->payload.string.cdr.offset = tail.offset;

        cell->payload.string.hash = calculate_hash( c, tail );
    } else {
        // \todo should throw an exception!
        debug_printf( DEBUG_ALLOC,
                      L"Warning: only NIL and %4.4s can be prepended to %4.4s\n",
                      tag, tag );
    }

    return pointer;
}

/**
 * Construct a string from the character `c` and this `tail`. A string is
 * implemented as a flat list of cells each of which has one character and a
 * pointer to the next; in the last cell the pointer to next is NIL.
 *
 * @param c the character to add (prepend);
 * @param tail the string which is being built.
 */
struct cons_pointer make_string( wint_t c, struct cons_pointer tail ) {
    return make_string_like_thing( c, tail, STRINGTV );
}

/**
 * Construct a symbol or keyword from the character `c` and this `tail`.
 * Each is internally identical to a string except for having a different tag.
 *
 * @param c the character to add (prepend);
 * @param tail the symbol which is being built.
 * @param tag the tag to use: expected to be "SYMB" or "KEYW"
 */
struct cons_pointer make_symbol_or_key( wint_t c, struct cons_pointer tail,
                                        uint32_t tag ) {
    struct cons_pointer result = make_string_like_thing( c, tail, tag );

    if ( tag == KEYTV ) {
        struct cons_pointer r = internedp( result, oblist );

        if ( nilp( r ) ) {
            intern( result, oblist );
        } else {
            result = r;
        }
    }

    return result;
}

/**
 * Construct a cell which points to an executable Lisp special form.
 */
struct cons_pointer
make_special( struct cons_pointer meta, struct cons_pointer ( *executable )
               ( struct stack_frame * frame,
                 struct cons_pointer, struct cons_pointer env ) ) {
    struct cons_pointer pointer = allocate_cell( SPECIALTV );
    struct cons_space_object *cell = &pointer2cell( pointer );
    inc_ref( meta );

    cell->payload.special.meta = meta;
    cell->payload.special.executable = executable;

    return pointer;
}

/**
 * Construct a cell which points to a stream open for reading.
 * @param input the C stream to wrap.
 * @param metadata a pointer to an associaton containing metadata on the stream.
 * @return a pointer to the new read stream.
 */
struct cons_pointer make_read_stream( URL_FILE * input,
                                      struct cons_pointer metadata ) {
    struct cons_pointer pointer = allocate_cell( READTV );
    struct cons_space_object *cell = &pointer2cell( pointer );

    cell->payload.stream.stream = input;
    cell->payload.stream.meta = metadata;

    return pointer;
}

/**
 * Construct a cell which points to a stream open for writing.
 * @param output the C stream to wrap.
 * @param metadata a pointer to an associaton containing metadata on the stream.
 * @return a pointer to the new read stream.
 */
struct cons_pointer make_write_stream( URL_FILE * output,
                                       struct cons_pointer metadata ) {
    struct cons_pointer pointer = allocate_cell( WRITETV );
    struct cons_space_object *cell = &pointer2cell( pointer );

    cell->payload.stream.stream = output;
    cell->payload.stream.meta = metadata;

    return pointer;
}

/**
 * Return a lisp keyword representation of this wide character string. In keywords,
 * I am accepting only lower case characters and numbers.
 */
struct cons_pointer c_string_to_lisp_keyword( wchar_t *symbol ) {
    struct cons_pointer result = NIL;

    for ( int i = wcslen( symbol ) - 1; i >= 0; i-- ) {
        wchar_t c = towlower( symbol[i] );

        if ( iswalnum( c ) || c == L'-' ) {
            result = make_keyword( c, result );
        }
    }

    return result;
}

/**
 * Return a lisp string representation of this wide character string.
 */
struct cons_pointer c_string_to_lisp_string( wchar_t *string ) {
    struct cons_pointer result = NIL;

    for ( int i = wcslen( string ) - 1; i >= 0; i-- ) {
        if ( iswprint( string[i] ) && string[i] != '"' ) {
            result = make_string( string[i], result );
        }
    }

    return result;
}

/**
 * Return a lisp symbol representation of this wide character string.
 */
struct cons_pointer c_string_to_lisp_symbol( wchar_t *symbol ) {
    struct cons_pointer result = NIL;

    for ( int i = wcslen( symbol ); i > 0; i-- ) {
        result = make_symbol( symbol[i - 1], result );
    }

    return result;
}
