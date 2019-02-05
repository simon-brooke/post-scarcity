/*
 * map.h
 *
 * An immutable hashmap in vector space.
 *
 * (c) 2019 Simon Brooke <simon@journeyman.cc>
 * Licensed under GPL version 2.0, or, at your option, any later version.
 */

#ifndef __psse_map_h
#define __psse_map_h

#include "consspaceobject.h"
#include "conspage.h"

/**
 * macros for the tag of a mutable map.
 */
#define MAPTAG "IMAP"
#define MAPTV 1346456905

/**
 * Number of buckets in a single tier map.
 */
#define BUCKETSINMAP 256

/**
 * Maximum number of entries in an association-list bucket.
 */
#define MAXENTRIESINASSOC 16

/**
 * true if this vector_space_object is a map, else false.
 */
#define mapp( vso) (((struct vector_space_object *)vso)->header.tag.value == MAPTV)

/**
 * The vector-space payload of a map object. Essentially a vector of
 * `BUCKETSINMAP` + 1 `cons_pointer`s, but the first one is considered
 * special.
 */
struct map_payload {
    /**
     * There is a default hash function, which is used if `hash_function` is
     * `nil` (which it normally should be); and keywords will probably carry
     * their own hash values. But it will be possible to override the hash
     * function by putting a function of one argument returning an integer
     * here. */
    struct cons_pointer hash_function;

    /**
     * Obviously the number of buckets in a map is a trade off, and this may need
     * tuning - or it may even be necessary to have different sized base maps. The
     * idea here is that the value of a bucket is
     *
     * 1. `nil`; or
     * 2. an association list; or
     * 3. a map.
     *
     * All buckets are initially `nil`. Adding a value to a `nil` bucket returns
     * a map with a new bucket in the form of an assoc list. Subsequent additions
     * cons new key/value pairs onto the assoc list, until there are
     * `MAXENTRIESINASSOC` pairs, at which point if a further value is added to
     * the same bucket the bucket returned will be in the form of a second level
     * map. My plan is that buckets the first level map will be indexed on the
     * first sixteen bits of the hash value, those in the second on the second
     * sixteen, and, potentially, so on.
     */
    struct cons_pointer buckets[BUCKETSINMAP];
};

uint32_t get_hash_32(struct cons_pointer f, struct cons_pointer key);

struct map_payload *get_map_payload( struct cons_pointer pointer );

struct cons_pointer bind_in_map( struct cons_pointer parent,
                                struct cons_pointer key,
                                struct cons_pointer value);

struct cons_pointer keys( struct cons_pointer store);

struct cons_pointer merge_into_map( struct cons_pointer parent,
                                   struct cons_pointer to_merge);

struct cons_pointer assoc_in_map( struct cons_pointer map,
                                  struct cons_pointer key);

void dump_map( URL_FILE * output, struct cons_pointer map_pointer );

#endif
