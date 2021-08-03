/*
 * hashmap.c
 *
 * Basic implementation of a hashmap.
 *
 * (c) 2021 Simon Brooke <simon@journeyman.cc>
 * Licensed under GPL version 2.0, or, at your option, any later version.
 */

#include "arith/integer.h"
#include "memory/consspaceobject.h"
#include "memory/hashmap.h"

/**
 * Get the hash value for the cell indicated by this `ptr`; currently only
 * implemented for string like things.
 */
uint32_t get_hash(struct cons_pointer ptr)
{
    struct cons_space_object *cell = &pointer2cell(ptr);
    uint32_t result = 0;

    switch (cell->tag.value)
    {
    case KEYTV:
    case STRINGTV:
    case SYMBOLTV:
        result = cell->payload.string.hash;
    default:
        // TODO: Not Yet Implemented
        result = 0;
    }

    return result;
}

/**
 * A lisp function signature conforming wrapper around get_hash, q.v..
 */
struct cons_pointer lisp_get_hash(struct stack_frame *frame,
                                  struct cons_pointer frame_pointer,
                                  struct cons_pointer env)
{
    return make_integer(get_hash(frame->arg[0]), NIL);
}

/**
 * Make a hashmap with this number of buckets.
 */
struct cons_pointer make_hashmap( uint32_t n_buckets) {
    struct cons_pointer result = make_vso(HASHTAG,
    (sizeof(struct cons_pointer) * (n_buckets + 1)) +
    (sizeof(uint32_t) * 2));

    //  TODO: fill in the payload!

    struct hashmap_payload *payload  = 
        (struct hashmap_payload *) &pointer_to_vso(result)->payload;

    return result;
}