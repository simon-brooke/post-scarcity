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
#include "vectorspace.h"


/**
 * make a cons-space object which points to the vector space object
 * with this `tag` at this `address`.
 * NOTE that `tag` should be the vector-space tag of the particular type of
 * vector-space object, NOT `VECTORPOINTTAG`.
 */
struct cons_pointer make_vec_pointer( char *tag,
                                      struct vector_space_object *address ) {
    fputws( L"Entered make_vec_pointer\n", stderr );
    struct cons_pointer pointer = allocate_cell( VECTORPOINTTAG );
    struct cons_space_object cell = pointer2cell( pointer );
    fwprintf( stderr,
              L"make_vec_pointer: allocated cell, about to write tag '%s'\n",
              tag );
    strncpy( &cell.payload.vectorp.tag.bytes[0], tag, 4 );
    fputws( L"make_vec_pointer: tag written, about to set pointer address\n",
            stderr );
    cell.payload.vectorp.address = address;
    fputws( L"make_vec_pointer: all good, returning\n", stderr );

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
    fputws( L"Entered make_vso\n", stderr );
    struct cons_pointer result = NIL;
    int64_t total_size = sizeof( struct vector_space_header ) + payload_size;

    /* Pad size to 64 bit words. This is intended to promote access efficiancy
     * on 64 bit machines but may just be voodoo coding */
    uint64_t padded = ceil( ( total_size * 8.0 ) / 8.0 );
    fputws( L"make_vso: about to malloc\n", stderr );
    struct vector_space_object *vso = malloc( padded );

    if ( vso != NULL ) {
        fwprintf( stderr, L"make_vso: about to write tag '%s'\n", tag );
        strncpy( &vso->header.tag.bytes[0], tag, TAGLENGTH );
        vso->header.vecp = make_vec_pointer( tag, vso );
        vso->header.size = payload_size;

#ifdef DEBUG
        fwprintf( stderr,
                  L"Allocated vector-space object of type %4.4s, total size %ld, payload size %ld\n",
                  tag, total_size, payload_size );
        if ( padded != total_size ) {
            fwprintf( stderr, L"\t\tPadded from %d to %d\n",
                      total_size, padded );
        }
#endif

        result = vso->header.vecp;
    }
    fputws( L"make_vso: all good, returning\n", stderr );

    return result;
}
