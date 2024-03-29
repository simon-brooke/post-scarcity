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
#include "hashmap.h"

#ifndef __vectorspace_h
#define __vectorspace_h

/*
 * part of the implementation structure of a namespace.
 */
#define HASHTAG "HASH"
#define HASHTV 1213415752

#define hashmapp(conspoint)((check_tag(conspoint,HASHTV)))

/*
 * a namespace (i.e. a binding of names to values, implemented as a hashmap)
 * TODO: but note that a namespace is now essentially a hashmap with a write ACL
 * whose name is interned.
 */
#define NAMESPACETAG "NMSP"
#define NAMESPACETV 1347636558

#define namespacep(conspoint)(check_tag(conspoint,NAMESPACETV))

/*
 * a vector of cons pointers.
 */
#define VECTORTAG "VECT"
#define VECTORTV 1413694806

#define vectorp(conspoint)(check_tag(conspoint,VECTORTV))

/**
 * given a pointer to a vector space object, return the object.
 */
#define pointer_to_vso(pointer)((vectorpointp(pointer)? (struct vector_space_object *) pointer2cell(pointer).payload.vectorp.address : (struct vector_space_object *) NULL))

/**
 * given a vector space object, return its canonical pointer.
 */
#define vso_get_vecp(vso)((((vector_space_object)vso)->header.vecp))

struct cons_pointer make_vso( uint32_t tag, uint64_t payload_size );

void free_vso( struct cons_pointer pointer );

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

/**
 * The payload of a hashmap. The number of buckets is assigned at run-time,
 * and is stored in n_buckets. Each bucket is something ASSOC can consume:
 * i.e. either an assoc list or a further hashmap.
 */
struct hashmap_payload {
    struct cons_pointer hash_fn;  /* function for hashing values in this hashmap, or `NIL` to use
                                     the default hashing function */
    struct cons_pointer write_acl;  /* it seems to me that it is likely that the
                                     * principal difference between a hashmap and a
                                     * namespace is that a hashmap has a write ACL
                                     * of `NIL`, meaning not writeable by anyone */
    uint32_t n_buckets;         /* number of hash buckets */
    uint32_t unused;            /* for word alignment and possible later expansion */
    struct cons_pointer buckets[];  /* actual hash buckets, which should be `NIL`
                                     * or assoc lists or (possibly) further hashmaps. */
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
    union {
        /** the payload considered as bytes */
        char bytes;
        struct hashmap_payload hashmap;
    } payload;
};

#endif
