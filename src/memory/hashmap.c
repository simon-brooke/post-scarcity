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
#include "memory/vectorspace.h"

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
 * Make a hashmap with this number of buckets, using this `hash_fn`. If 
 * `hash_fn` is `NIL`, use the standard hash funtion.
 */
struct cons_pointer make_hashmap( uint32_t n_buckets,
                                  struct cons_pointer hash_fn ) {
  struct cons_pointer result =
      make_vso( HASHTAG, ( sizeof( struct cons_pointer ) * ( n_buckets + 1 ) ) +
                             ( sizeof( uint32_t ) * 2 ) );

  struct hashmap_payload *payload =
      (struct hashmap_payload *)&pointer_to_vso( result )->payload;

  payload->hash_fn = hash_fn;
  payload->n_buckets = n_buckets;
  for ( int i = 0; i < n_buckets; i++ ) {
    payload->buckets[i] = NIL;
  }

  return result;
}

/**
 * If this `ptr` is a pointer to a hashmap, return a new identical hashmap; 
 * else return `NIL`. TODO: should return an exception.
 */
struct cons_pointer clone_hashmap(struct cons_pointer ptr) {
    struct cons_pointer result = NIL;

    if (hashmapp(ptr)) {
        struct vector_space_object *from = pointer_to_vso( ptr );

      if ( from != NULL ) {
          struct hashmap_payload *from_pl = (struct hashmap_payload*)from->payload;
          result = make_hashmap( from_pl->n_buckets, from_pl->hash_fn);
          struct vector_space_object *to = pointer_to_vso(result);
          struct hashmap_payload *to_pl = (struct hashmap_payload*)to->payload;

          for (int i = 0; i < to_pl->n_buckets; i++) {
              to_pl->buckets[i] = from_pl->buckets[i];
              inc_ref(to_pl->buckets[i]);
          }
      }
    }

    return result;
}

