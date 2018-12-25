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

/*
 * a namespace (i.e. a binding of names to values, implemented as a hashmap)
 */
#define NAMESPACETAG "NMSP"
#define NAMESPACETV 0

/*
 * a stack frame.
 */
#define STACKFRAMETAG "STAK"
#define STACKFRAMETV
/*
 * a vector of cons pointers.
 */
#define VECTORTAG "VECT"
#define VECTORTV 0

#define pointer_to_vso(pointer)(vectorpointp(pointer)? pointer2cell(pointer).payload.vectorp.address : 0)

struct cons_pointer make_vso( char *tag, int64_t payload_size );

struct vector_space_header {
    union {
        char bytes[TAGLENGTH];  /* the tag (type) of the
                                 * vector-space object this cell
                                 * points to, considered as bytes.
                                 * NOTE that the vector space object
                                 * should itself have the identical
                                 * tag. */
        uint32_t value;         /* the tag considered as a number */
    } tag;
    struct cons_pointer vecp;   /* back pointer to the vector pointer
                                 * which uniquely points to this vso */
    uint64_t size;              /* the size of my payload, in bytes */
    char mark;                  /* mark bit for marking/sweeping the
                                 * heap (not in this version) */
    char payload;               /* we'll malloc `size` bytes for payload,
                                 * `payload` is just the first of these.
                                 * TODO: this is almost certainly not
                                 * idiomatic C. */
};

#endif
