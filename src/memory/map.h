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
 * The vector-space payload of a map object.
 */
struct map_payload {
    /**
     * There is a default hash function, which is used if `hash_function` is
     * `nil` (which it normally should be); and keywords will probably carry
     * their own hash values. But it will be possible to override the hash
     * function by putting a function of one argument returning an integer
     * here. */
    struct cons_pointer hash_function = NIL;

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

#endif
