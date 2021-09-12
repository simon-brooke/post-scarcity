/*
 * conspage.c
 *
 * Setup and tear down cons pages, and (FOR NOW) do primitive
 * allocation/deallocation of cells.
 * NOTE THAT before we go multi-threaded, these functions must be
 * aggressively
 * thread safe.
 *
 * (c) 2017 Simon Brooke <simon@journeyman.cc>
 * Licensed under GPL version 2.0, or, at your option, any later version.
 */

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "memory/consspaceobject.h"
#include "memory/conspage.h"
#include "debug.h"
#include "memory/dump.h"
#include "memory/stack.h"
#include "memory/vectorspace.h"

/**
 * Flag indicating whether conspage initialisation has been done.
 */
bool conspageinitihasbeencalled = false;

/**
 * keep track of total cells allocated and freed to check for leakage.
 */
uint64_t total_cells_allocated = 0;
uint64_t total_cells_freed = 0;

/**
 * the number of cons pages which have thus far been initialised.
 */
int initialised_cons_pages = 0;

/**
 * The (global) pointer to the (global) freelist. Not sure whether this ultimately
 * belongs in this file.
 */
struct cons_pointer freelist = NIL;

/**
 * An array of pointers to cons pages.
 */
struct cons_page *conspages[NCONSPAGES];

/**
 * Make a cons page. Initialise all cells and prepend each to the freelist;
 * if `initialised_cons_pages` is zero, do not prepend cells 0 and 1 to the
 * freelist but initialise them as NIL and T respectively.
 * \todo we ought to handle cons space exhaustion more gracefully than just
 * crashing; should probably return an exception instead, although obviously
 * that exception would have to have been pre-built.
 */
void make_cons_page(  ) {
    struct cons_page *result = malloc( sizeof( struct cons_page ) );

    if ( result != NULL ) {
        conspages[initialised_cons_pages] = result;

        for ( int i = 0; i < CONSPAGESIZE; i++ ) {
            struct cons_space_object *cell =
                &conspages[initialised_cons_pages]->cell[i];
            if ( initialised_cons_pages == 0 && i < 2 ) {
                switch ( i ) {
                    case 0:
                        /*
                         * initialise cell as NIL
                         */
                        strncpy( &cell->tag.bytes[0], NILTAG, TAGLENGTH );
                        cell->count = MAXREFERENCE;
                        cell->payload.free.car = NIL;
                        cell->payload.free.cdr = NIL;
                        debug_printf( DEBUG_ALLOC,
                                      L"Allocated special cell NIL\n" );
                        break;
                    case 1:
                        /*
                         * initialise cell as T
                         */
                        strncpy( &cell->tag.bytes[0], TRUETAG, TAGLENGTH );
                        cell->count = MAXREFERENCE;
                        cell->payload.free.car = ( struct cons_pointer ) {
                            0, 1
                        };
                        cell->payload.free.cdr = ( struct cons_pointer ) {
                            0, 1
                        };
                        debug_printf( DEBUG_ALLOC,
                                      L"Allocated special cell T\n" );
                        break;
                }
            } else {
                /*
                 * otherwise, standard initialisation
                 */
                strncpy( &cell->tag.bytes[0], FREETAG, TAGLENGTH );
                cell->payload.free.car = NIL;
                cell->payload.free.cdr = freelist;
                freelist.page = initialised_cons_pages;
                freelist.offset = i;
            }
        }

        initialised_cons_pages++;
    } else {
        debug_printf( DEBUG_ALLOC,
                      L"FATAL: Failed to allocate memory for cons page %d\n",
                      initialised_cons_pages );
        exit( 1 );
    }

}

/**
 * dump the allocated pages to this `output` stream.
 */
void dump_pages( URL_FILE * output ) {
    for ( int i = 0; i < initialised_cons_pages; i++ ) {
        url_fwprintf( output, L"\nDUMPING PAGE %d\n", i );

        for ( int j = 0; j < CONSPAGESIZE; j++ ) {
            dump_object( output, ( struct cons_pointer ) {
                         i, j
                         } );
        }
    }
}

/**
 * Frees the cell at the specified `pointer`; for all the types of cons-space
 * object which point to other cons-space objects, cascade the decrement.
 * Dangerous, primitive, low level.
 *
 * @pointer the cell to free
 */
void free_cell( struct cons_pointer pointer ) {
    struct cons_space_object *cell = &pointer2cell( pointer );

    debug_printf( DEBUG_ALLOC, L"Freeing cell " );
    debug_dump_object( pointer, DEBUG_ALLOC );

    if ( !check_tag( pointer, FREETV ) ) {
        if ( cell->count == 0 ) {
            switch ( cell->tag.value ) {
                case CONSTV:
                    dec_ref( cell->payload.cons.car );
                    dec_ref( cell->payload.cons.cdr );
                    break;
                case EXCEPTIONTV:
                    dec_ref( cell->payload.exception.payload );
                    dec_ref( cell->payload.exception.frame );
                    break;
                case FUNCTIONTV:
                    dec_ref( cell->payload.function.meta );
                    break;
                case INTEGERTV:
                    dec_ref( cell->payload.integer.more );
                    break;
                case LAMBDATV:
                case NLAMBDATV:
                    dec_ref( cell->payload.lambda.args );
                    dec_ref( cell->payload.lambda.body );
                    break;
                case RATIOTV:
                    dec_ref( cell->payload.ratio.dividend );
                    dec_ref( cell->payload.ratio.divisor );
                    break;
                case READTV:
                case WRITETV:
                    dec_ref( cell->payload.stream.meta );
                    url_fclose( cell->payload.stream.stream );
                    break;
                case SPECIALTV:
                    dec_ref( cell->payload.special.meta );
                    break;
                case STRINGTV:
                case SYMBOLTV:
                    dec_ref( cell->payload.string.cdr );
                    break;
                case VECTORPOINTTV:
                    free_vso( pointer );
                    break;
            }

            strncpy( &cell->tag.bytes[0], FREETAG, TAGLENGTH );
            cell->payload.free.car = NIL;
            cell->payload.free.cdr = freelist;
            freelist = pointer;
            total_cells_freed++;
        } else {
            debug_printf( DEBUG_ALLOC,
                          L"ERROR: Attempt to free cell with %d dangling references at page %d, offset %d\n",
                          cell->count, pointer.page, pointer.offset );
        }
    } else {
        debug_printf( DEBUG_ALLOC,
                      L"ERROR: Attempt to free cell which is already FREE at page %d, offset %d\n",
                      pointer.page, pointer.offset );
    }
}

/**
 * Allocates a cell with the specified `tag`. Dangerous, primitive, low
 * level.
 *
 * @param tag the tag of the cell to allocate - must be a valid cons space tag.
 * @return the cons pointer which refers to the cell allocated.
 * \todo handle the case where another cons_page cannot be allocated;
 * return an exception. Which, as we cannot create such an exception when
 * cons space is exhausted, means we must construct it at init time.
 */
struct cons_pointer allocate_cell( uint32_t tag ) {
    struct cons_pointer result = freelist;


    if ( result.page == NIL.page && result.offset == NIL.offset ) {
        make_cons_page(  );
        result = allocate_cell( tag );
    } else {
        struct cons_space_object *cell = &pointer2cell( result );

        if ( strncmp( &cell->tag.bytes[0], FREETAG, TAGLENGTH ) == 0 ) {
            freelist = cell->payload.free.cdr;

            cell->tag.value = tag;

            cell->count = 0;
            cell->payload.cons.car = NIL;
            cell->payload.cons.cdr = NIL;

            total_cells_allocated++;

            debug_printf( DEBUG_ALLOC,
                          L"Allocated cell of type '%4.4s' at %d, %d \n", tag,
                          result.page, result.offset );
        } else {
            debug_printf( DEBUG_ALLOC, L"WARNING: Allocating non-free cell!" );
        }
    }

    return result;
}

/**
 * initialise the cons page system; to be called exactly once during startup.
 */
void initialise_cons_pages(  ) {
    if ( conspageinitihasbeencalled == false ) {
        for ( int i = 0; i < NCONSPAGES; i++ ) {
            conspages[i] = ( struct cons_page * ) NULL;
        }

        make_cons_page(  );
        conspageinitihasbeencalled = true;
    } else {
        debug_printf( DEBUG_ALLOC,
                      L"WARNING: initialise_cons_pages() called a second or subsequent time\n" );
    }
}

void summarise_allocation(  ) {
    fwprintf( stderr,
              L"Allocation summary: allocated %lld; deallocated %lld.\n",
              total_cells_allocated, total_cells_freed );
}
