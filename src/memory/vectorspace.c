/*
 *  vectorspace.c
 *
 *  Structures common to all vector space objects.
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
#include "vectorspace.h"


/**
 * make a cons-space object which points to the vector space object
 * with this `tag` at this `address`.
 * NOTE that `tag` should be the vector-space tag of the particular type of
 * vector-space object, NOT `VECTORPOINTTAG`.
 */
struct cons_pointer make_vec_pointer( char *tag, uint64_t address ) {
    struct cons_pointer pointer = allocate_cell( VECTORPOINTTAG );
    struct cons_space_object cell = pointer2cell( pointer );

    strncpy( &cell.payload.vectorp.tag.bytes[0], tag, 4 );
    cell.payload.vectorp.address = address;

    return pointer;
}

/**
 * allocate a vector space object with this `payload_size` and `tag`,
 * and return a `cons_pointer` which points to an object whigh points to it.
 * NOTE that `tag` should be the vector-space tag of the particular type of
 * vector-space object, NOT `VECTORPOINTTAG`.
 */
struct cons_pointer make_vso( char *tag, long int payload_size ) {
    struct cons_pointer result = NIL;
    long int total_size = sizeof( struct vector_space_header ) + payload_size;

    struct vector_space_header *vso = malloc( total_size );

    if ( vso != NULL ) {
        strncpy( &vso->tag.bytes[0], tag, TAGLENGTH );
        vso->vecp = make_vec_pointer( tag, ( uint64_t ) vso );
        vso->size = payload_size;

#ifdef DEBUG
        fwprintf( stderr,
                  L"Allocated vector-space object of type %s, total size %ld, payload size %ld\n",
                  tag, total_size, payload_size );
#endif

        result = vso->vecp;
    }

    return result;
}
