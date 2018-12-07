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
#include "print.h"
#include "stack.h"

/**
 * Check that the tag on the cell at this pointer is this tag
 */
int check_tag( struct cons_pointer pointer, char *tag ) {
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

    if ( cell->count <= MAXREFERENCE ) {
        cell->count--;

        if ( cell->count == 0 ) {
            free_cell( pointer );
        }
    }
}

void dump_string_cell( FILE * output, wchar_t *prefix,
                       struct cons_pointer pointer ) {
    struct cons_space_object cell = pointer2cell( pointer );
    if ( cell.payload.string.character == 0 ) {
        fwprintf( output,
                  L"\t\t%ls cell: termination; next at page %d offset %d, count %u\n",
                  prefix,
                  cell.payload.string.cdr.page, cell.payload.string.cdr.offset,
                  cell.count );
    } else {
        fwprintf( output,
                  L"\t\t%ls cell: character '%lc' (%d) next at page %d offset %d, count %u\n",
                  prefix,
                  ( wint_t ) cell.payload.string.character,
                  cell.payload.string.character,
                  cell.payload.string.cdr.page,
                  cell.payload.string.cdr.offset, cell.count );
        fwprintf( output, L"\t\t value: " );
        print( output, pointer );
        fwprintf( output, L"\n" );
    }
}

/**
 * dump the object at this cons_pointer to this output stream.
 */
void dump_object( FILE * output, struct cons_pointer pointer ) {
    struct cons_space_object cell = pointer2cell( pointer );
    fwprintf( output,
              L"\tDumping %c%c%c%c (%d) at page %d, offset %d count %u\n",
              cell.tag.bytes[0],
              cell.tag.bytes[1],
              cell.tag.bytes[2],
              cell.tag.bytes[3],
              cell.tag.value, pointer.page, pointer.offset, cell.count );

    switch ( cell.tag.value ) {
    case CONSTV:
        fwprintf( output,
                  L"\t\tCons cell: car at page %d offset %d, cdr at page %d offset %d, count %u\n",
                  cell.payload.cons.car.page,
                  cell.payload.cons.car.offset,
                  cell.payload.cons.cdr.page,
                  cell.payload.cons.cdr.offset, cell.count );
        break;
    case EXCEPTIONTV:
        fwprintf( output, L"\t\tException cell: " );
        print( output, cell.payload.exception.message );
        fwprintf( output, L"\n" );
        /* TODO: dump the stack trace */
        for ( struct stack_frame * frame = cell.payload.exception.frame;
              frame != NULL; frame = frame->previous ) {
            dump_frame( output, frame );
        }
        break;
    case FREETV:
        fwprintf( output, L"\t\tFree cell: next at page %d offset %d\n",
                  cell.payload.cons.cdr.page, cell.payload.cons.cdr.offset );
        break;
    case INTEGERTV:
        fwprintf( output,
                  L"\t\tInteger cell: value %ld, count %u\n",
                  cell.payload.integer.value, cell.count );
        break;
    case READTV:
        fwprintf( output, L"\t\tInput stream\n" );
    case REALTV:
        fwprintf( output, L"\t\tReal cell: value %Lf, count %u\n",
                  cell.payload.real.value, cell.count );
        break;
    case STRINGTV:
        dump_string_cell( output, L"String", pointer );
        break;
    case SYMBOLTV:
        dump_string_cell( output, L"Symbol", pointer );
        break;
    }
}

/**
 * Construct a cons cell from this pair of pointers.
 */
struct cons_pointer make_cons( struct cons_pointer car,
                               struct cons_pointer cdr ) {
    struct cons_pointer pointer = NIL;

    pointer = allocate_cell( CONSTAG );

    struct cons_space_object *cell =
        &conspages[pointer.page]->cell[pointer.offset];

    inc_ref( car );
    inc_ref( cdr );
    cell->payload.cons.car = car;
    cell->payload.cons.cdr = cdr;

    return pointer;
}

/**
 * Construct an exception cell.
 * @param message should be a lisp string describing the problem, but actually any cons pointer will do;
 * @param frame should be the frame in which the exception occurred.
 */
struct cons_pointer make_exception( struct cons_pointer message,
                                    struct stack_frame *frame ) {
    struct cons_pointer pointer = allocate_cell( EXCEPTIONTAG );
    struct cons_space_object *cell = &pointer2cell( pointer );

    cell->payload.exception.message = message;
    cell->payload.exception.frame = frame;

    return pointer;
}


/**
 * Construct a cell which points to an executable Lisp special form.
 */
struct cons_pointer
make_function( struct cons_pointer src, struct cons_pointer ( *executable )
                ( struct stack_frame *, struct cons_pointer ) ) {
    struct cons_pointer pointer = allocate_cell( FUNCTIONTAG );
    struct cons_space_object *cell = &pointer2cell( pointer );

    cell->payload.function.source = src;
    cell->payload.function.executable = executable;

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
        /* TODO: There's a problem here. Sometimes the offsets on
         * strings are quite massively off. */
        cell->payload.string.cdr.offset = tail.offset;
    } else {
        fwprintf( stderr,
                  L"Warning: only NIL and %s can be appended to %s\n",
                  tag, tag );
    }

    return pointer;
}

/**
 * Construct a string from this character and
 * this tail. A string is implemented as a flat list of cells each of which
 * has one character and a pointer to the next; in the last cell the
 * pointer to next is NIL.
 */
struct cons_pointer make_string( wint_t c, struct cons_pointer tail ) {
    return make_string_like_thing( c, tail, STRINGTAG );
}

/**
 * Construct a symbol from this character and this tail.
 */
struct cons_pointer make_symbol( wint_t c, struct cons_pointer tail ) {
    return make_string_like_thing( c, tail, SYMBOLTAG );
}

/**
 * Construct a cell which points to an executable Lisp special form.
 */
struct cons_pointer
make_special( struct cons_pointer src, struct cons_pointer ( *executable )
               ( struct stack_frame * frame, struct cons_pointer env ) ) {
    struct cons_pointer pointer = allocate_cell( SPECIALTAG );
    struct cons_space_object *cell = &pointer2cell( pointer );

    cell->payload.special.source = src;
    cell->payload.special.executable = executable;

    return pointer;
}

/**
 * Construct a cell which points to a stream open for reading.
 * @param input the C stream to wrap.
 */
struct cons_pointer make_read_stream( FILE * input ) {
    struct cons_pointer pointer = allocate_cell( READTAG );
    struct cons_space_object *cell = &pointer2cell( pointer );

    cell->payload.stream.stream = input;

    return pointer;
}

/**
 * Construct a cell which points to a stream open for writeing.
 * @param output the C stream to wrap.
 */
struct cons_pointer make_write_stream( FILE * output ) {
    struct cons_pointer pointer = allocate_cell( WRITETAG );
    struct cons_space_object *cell = &pointer2cell( pointer );

    cell->payload.stream.stream = output;

    return pointer;
}

/**
 * Return a lisp string representation of this old skool ASCII string.
 */
struct cons_pointer c_string_to_lisp_string( char *string ) {
    struct cons_pointer result = NIL;

    for ( int i = strlen( string ); i > 0; i-- ) {
        result = make_string( ( wint_t ) string[i - 1], result );
    }

    return result;
}

/**
 * Return a lisp symbol representation of this old skool ASCII string.
 */
struct cons_pointer c_string_to_lisp_symbol( char *symbol ) {
    struct cons_pointer result = NIL;

    for ( int i = strlen( symbol ); i > 0; i-- ) {
        result = make_symbol( ( wint_t ) symbol[i - 1], result );
    }

    return result;
}
