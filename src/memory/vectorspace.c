/*
 *  vectorspace.c
 *
 *  Structures common to all vector space objects.
 *
 *
 *  (c) 2017 Simon Brooke <simon@journeyman.cc>
 *  Licensed under GPL version 2.0, or, at your option, any later version.
 */

#include <math.h>
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
#include "vectorspace.h"


/**
 * make a cons-space object which points to the vector space object
 * with this `tag` at this `address`.
 * NOTE that `tag` should be the vector-space tag of the particular type of
 * vector-space object, NOT `VECTORPOINTTAG`.
 */
struct cons_pointer make_vec_pointer( struct vector_space_object *address ) {
    debug_print( L"Entered make_vec_pointer\n", DEBUG_ALLOC );
    struct cons_pointer pointer = allocate_cell( VECTORPOINTTAG );
    struct cons_space_object *cell = &pointer2cell( pointer );
    debug_printf( DEBUG_ALLOC,
                  L"make_vec_pointer: tag written, about to set pointer address to %p\n",
                  address );
    cell->payload.vectorp.address = address;
    debug_printf( DEBUG_ALLOC,
                  L"make_vec_pointer: all good, returning pointer to %p\n",
                  cell->payload.vectorp.address );

    debug_dump_object( pointer, DEBUG_ALLOC );

    return pointer;
}

/**
 * allocate a vector space object with this `payload_size` and `tag`,
 * and return a `cons_pointer` which points to an object whigh points to it.
 * NOTE that `tag` should be the vector-space tag of the particular type of
 * vector-space object, NOT `VECTORPOINTTAG`.
 * Returns NIL if the vector could not be allocated due to memory exhaustion.
 */
struct cons_pointer make_vso( char *tag, uint64_t payload_size ) {
    debug_print( L"Entered make_vso\n", DEBUG_ALLOC );
    struct cons_pointer result = NIL;
    int64_t total_size = sizeof( struct vector_space_header ) + payload_size;

    /* Pad size to 64 bit words. This is intended to promote access efficiancy
     * on 64 bit machines but may just be voodoo coding */
    uint64_t padded = ceil( ( total_size * 8.0 ) / 8.0 );
    debug_print( L"make_vso: about to malloc\n", DEBUG_ALLOC );
    struct vector_space_object *vso = malloc( padded );

    if ( vso != NULL ) {
        memset( vso, 0, padded );
        debug_printf( DEBUG_ALLOC,
                      L"make_vso: about to write tag '%s' into vso at %p\n",
                      tag, vso );
        strncpy( &vso->header.tag.bytes[0], tag, TAGLENGTH );
        result = make_vec_pointer( vso );
        debug_dump_object( result, DEBUG_ALLOC );
        vso->header.vecp = result;
        // memcpy(vso->header.vecp, result, sizeof(struct cons_pointer));

        vso->header.size = payload_size;

#ifdef DEBUG
        debug_printf( DEBUG_ALLOC,
                      L"Allocated vector-space object of type %4.4s, total size %ld, payload size %ld, at address %p, payload address %p\n",
                      &vso->header.tag.bytes, total_size, vso->header.size,
                      vso, &vso->payload );
        if ( padded != total_size ) {
            debug_printf( DEBUG_ALLOC, L"\t\tPadded from %d to %d\n",
                          total_size, padded );
        }
#endif
    }
#ifdef DEBUG
    debug_printf( DEBUG_ALLOC,
                  L"make_vso: all good, returning pointer to %p\n",
                  pointer2cell( result ).payload.vectorp.address );
#endif

    return result;
}
