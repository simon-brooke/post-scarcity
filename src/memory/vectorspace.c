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
#include "hashmap.h"
#include "stack.h"
#include "vectorspace.h"


/**
 * Make a cons_space_object which points to the vector_space_object
 * with this `tag` at this `address`.
 *
 * @address the address of the vector_space_object to point to.
 * @tag the vector-space tag of the particular type of vector-space object,
 * NOT `VECTORPOINTTV`.
 *
 * @return a cons_pointer to the object, or NIL if the object could not be
 * allocated due to memory exhaustion.
 */
struct cons_pointer make_vec_pointer( struct vector_space_object *address,
                                      uint32_t tag ) {
    debug_print( L"Entered make_vec_pointer\n", DEBUG_ALLOC );
    struct cons_pointer pointer = allocate_cell( VECTORPOINTTV );
    struct cons_space_object *cell = &pointer2cell( pointer );

    debug_printf( DEBUG_ALLOC,
                  L"make_vec_pointer: tag written, about to set pointer address to %p\n",
                  address );

    cell->payload.vectorp.address = address;
    cell->payload.vectorp.tag.value = tag;

    debug_printf( DEBUG_ALLOC,
                  L"make_vec_pointer: all good, returning pointer to %p\n",
                  cell->payload.vectorp.address );

    debug_dump_object( pointer, DEBUG_ALLOC );

    return pointer;
}

/**
 * Allocate a vector space object with this `payload_size` and `tag`,
 * and return a `cons_pointer` which points to an object whigh points to it.
 *
 * @tag the vector-space tag of the particular type of vector-space object,
 * NOT `VECTORPOINTTAG`.
 * @payload_size the size of the payload required, in bytes.
 *
 * @return a cons_pointer to the object, or NIL if the object could not be
 * allocated due to memory exhaustion.
 */
struct cons_pointer make_vso( uint32_t tag, uint64_t payload_size ) {
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
        vso->header.tag.value = tag;
        result = make_vec_pointer( vso, tag );
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

/** for vector space pointers, free the actual vector-space
 * object. Dangerous! */

void free_vso( struct cons_pointer pointer ) {
    struct cons_space_object cell = pointer2cell( pointer );

    debug_printf( DEBUG_ALLOC, L"About to free vector-space object at 0x%lx\n",
                  cell.payload.vectorp.address );
    struct vector_space_object *vso = cell.payload.vectorp.address;

    switch ( vso->header.tag.value ) {
        case HASHTV:
            free_hashmap( pointer );
            break;
        case STACKFRAMETV:
            free_stack_frame( get_stack_frame( pointer ) );
            break;
    }

//  free( (void *)cell.payload.vectorp.address );
    debug_printf( DEBUG_ALLOC, L"Freed vector-space object at 0x%lx\n",
                  cell.payload.vectorp.address );
}

// bool check_vso_tag( struct cons_pointer pointer, char * tag) {
//     bool result = false;

//     if (check_tag(pointer, VECTORPOINTTAG)) {
//         struct vector_space_object * vso = pointer_to_vso(pointer);
//         result = strncmp( vso->header.tag.bytes[0], tag, TAGLENGTH);
//     }

//     return result;
// }
