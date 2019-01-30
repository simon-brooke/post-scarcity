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

#include "conspage.h"
#include "consspaceobject.h"
#include "debug.h"
#include "intern.h"
#include "print.h"
#include "stack.h"

/**
 * True if the tag on the cell at this `pointer` is this `tag`, else false.
 */
bool check_tag( struct cons_pointer pointer, char *tag ) {
    struct cons_space_object cell = pointer2cell( pointer );
    return strncmp( &cell.tag.bytes[0], tag, TAGLENGTH ) == 0;
}

/**
 * increment the reference count of the object at this cons pointer.
 *
 * You can't roll over the reference count. Once it hits the maximum
 * value you cannot increment further.
 */
void inc_ref( struct cons_pointer pointer ) {
    struct cons_space_object *cell = &pointer2cell( pointer );

    if ( cell->count < MAXREFERENCE ) {
        cell->count++;
    }
}

/**
 * Decrement the reference count of the object at this cons pointer.
 *
 * If a count has reached MAXREFERENCE it cannot be decremented.
 * If a count is decremented to zero the cell should be freed.
 */
void dec_ref( struct cons_pointer pointer ) {
    struct cons_space_object *cell = &pointer2cell( pointer );

    if ( cell->count > 0 ) {
        cell->count--;

        if ( cell->count == 0 ) {
            free_cell( pointer );
        }
    }
}


/**
 * Get the Lisp type of the single argument.
 * @param pointer a pointer to the object whose type is requested.
 * @return As a Lisp string, the tag of the object which is at that pointer.
 */
struct cons_pointer c_type( struct cons_pointer pointer ) {
    struct cons_pointer result = NIL;
    struct cons_space_object cell = pointer2cell( pointer );

    for ( int i = TAGLENGTH - 1; i >= 0; i-- ) {
        result = make_string( ( wchar_t ) cell.tag.bytes[i], result );
    }

    return result;
}

/**
 * Implementation of car in C. If arg is not a cons, does not error but returns nil.
 */
struct cons_pointer c_car( struct cons_pointer arg ) {
    struct cons_pointer result = NIL;

    if ( consp( arg ) ) {
        result = pointer2cell( arg ).payload.cons.car;
    }

    return result;
}

/**
 * Implementation of cdr in C. If arg is not a cons, does not error but returns nil.
 */
struct cons_pointer c_cdr( struct cons_pointer arg ) {
    struct cons_pointer result = NIL;

    if ( consp( arg ) || stringp( arg ) || symbolp( arg ) ) {
        result = pointer2cell( arg ).payload.cons.cdr;
    }

    return result;
}

/**
 * Construct a cons cell from this pair of pointers.
 */
struct cons_pointer make_cons( struct cons_pointer car,
                               struct cons_pointer cdr ) {
    struct cons_pointer pointer = NIL;

    pointer = allocate_cell( CONSTAG );

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
    struct cons_pointer pointer = allocate_cell( EXCEPTIONTAG );
    struct cons_space_object *cell = &pointer2cell( pointer );

    inc_ref( message );
    inc_ref( frame_pointer );
    cell->payload.exception.message = message;
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
    struct cons_pointer pointer = allocate_cell( FUNCTIONTAG );
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
    struct cons_pointer pointer = allocate_cell( LAMBDATAG );
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
    struct cons_pointer pointer = allocate_cell( NLAMBDATAG );

    inc_ref( pointer );         /* this is a hack; I don't know why it's necessary to do this, but if I don't the cell gets freed */

    struct cons_space_object *cell = &pointer2cell( pointer );
    inc_ref( args );
    inc_ref( body );
    cell->payload.lambda.args = args;
    cell->payload.lambda.body = body;

    return pointer;
}

/**
 * Construct a string from this character (which later will be UTF) and
 * this tail. A string is implemented as a flat list of cells each of which
 * has one character and a pointer to the next; in the last cell the
 * pointer to next is NIL.
 */
struct cons_pointer
make_string_like_thing( wint_t c, struct cons_pointer tail, char *tag ) {
    struct cons_pointer pointer = NIL;

    if ( check_tag( tail, tag ) || check_tag( tail, NILTAG ) ) {
        pointer = allocate_cell( tag );
        struct cons_space_object *cell = &pointer2cell( pointer );

        inc_ref( tail );
        cell->payload.string.character = c;
        cell->payload.string.cdr.page = tail.page;
        /* \todo There's a problem here. Sometimes the offsets on
         * strings are quite massively off. Fix is probably
         * cell->payload.string.cdr = tsil */
        cell->payload.string.cdr.offset = tail.offset;
    } else {
        // \todo should throw an exception!
        debug_printf( DEBUG_ALLOC,
                      L"Warning: only NIL and %s can be prepended to %s\n",
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
    return make_string_like_thing( c, tail, STRINGTAG );
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
                                        char *tag ) {
    struct cons_pointer result = make_string_like_thing( c, tail, tag );

    if ( strncmp( tag, KEYTAG, 4 ) == 0 ) {
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
    struct cons_pointer pointer = allocate_cell( SPECIALTAG );
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
    struct cons_pointer pointer = allocate_cell( READTAG );
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
    struct cons_pointer pointer = allocate_cell( WRITETAG );
    struct cons_space_object *cell = &pointer2cell( pointer );

    cell->payload.stream.stream = output;
    cell->payload.stream.meta = metadata;

    return pointer;
}

/**
 * Return a lisp keyword representation of this wide character string.
 */
struct cons_pointer c_string_to_lisp_keyword( wchar_t *symbol ) {
    struct cons_pointer result = NIL;

    for (int i = 0; symbol[i] != '\0'; i++) {
        if(iswalpha(symbol[i] && !iswlower(symbol[i]))) {
            symbol[i] = towlower(symbol[i]);
        }
    }

    for ( int i = wcslen( symbol ); i > 0; i-- ) {
        result = make_keyword( symbol[i - 1], result );
    }

    return result;
}

/**
 * Return a lisp string representation of this wide character string.
 */
struct cons_pointer c_string_to_lisp_string( wchar_t *string ) {
    struct cons_pointer result = NIL;

    for ( int i = wcslen( string ); i > 0; i-- ) {
        result = make_string( string[i - 1], result );
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
