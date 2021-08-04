/**
 *  vectorspace.h
 *
 *  Declarations common to all vector space objects.
 *
 *
 *  (c) 2017 Simon Brooke <simon@journeyman.cc>
 *  Licensed under GPL version 2.0, or, at your option, any later version.
 */

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
/*
 * wide characters
 */
#include <wchar.h>
#include <wctype.h>

#include "consspaceobject.h"

#ifndef __vectorspace_h
#define __vectorspace_h

/*
 * part of the implementation structure of a namespace.
 */
#define HASHTAG "HASH"
#define HASHTV 0

#define hashmapp(conspoint)((check_tag(conspoint,HASHTAG)))

/*
 * a namespace (i.e. a binding of names to values, implemented as a hashmap)
 */
#define NAMESPACETAG "NMSP"
#define NAMESPACETV 0

#define namespacep(conspoint)(check_tag(conspoint,NAMESPACETAG))

/*
 * a vector of cons pointers.
 */
#define VECTORTAG "VECT"
#define VECTORTV 0

#define vectorp(conspoint)(check_tag(conspoint,VECTORTAG))

/**
 * given a pointer to a vector space object, return the object.
 */
#define pointer_to_vso(pointer)((vectorpointp(pointer)? (struct vector_space_object *) pointer2cell(pointer).payload.vectorp.address : (struct vector_space_object *) NULL))

/**
 * given a vector space object, return its canonical pointer.
 */
#define vso_get_vecp(vso)((((vector_space_object)vso)->header.vecp))

struct cons_pointer make_vso( char *tag, uint64_t payload_size );

/**
 * the header which forms the start of every vector space object.
 */
struct vector_space_header {
    /** the tag (type) of this vector-space object. */
    union {
        /** the tag considered as bytes. */
        char bytes[TAGLENGTH];
        /** the tag considered as a number */
        uint32_t value;
    } tag;
    /** back pointer to the vector pointer which uniquely points to this vso */
    struct cons_pointer vecp;
    /** the size of my payload, in bytes */
    uint64_t size;
};

/** a vector_space_object is just a vector_space_header followed by a
 * lump of bytes; what we deem to be in there is a function of the tag,
 * and at this stage we don't have a good picture of what these may be.
 *
 * \see stack_frame for an example payload;
 * \see make_empty_frame for an example of how to initialise and use one.
 */
struct vector_space_object {
    /** the header of this object */
    struct vector_space_header header;
    /** we'll malloc `size` bytes for payload, `payload` is just the first of these.
     * \todo this is almost certainly not idiomatic C. */
    char payload;
};

#endif
