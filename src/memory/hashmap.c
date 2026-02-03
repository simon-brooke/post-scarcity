/*
 * hashmap.c
 *
 * Basic implementation of a hashmap.
 *
 * (c) 2021 Simon Brooke <simon@journeyman.cc>
 * Licensed under GPL version 2.0, or, at your option, any later version.
 */

#include "arith/integer.h"
#include "arith/peano.h"
#include "authorise.h"
#include "debug.h"
#include "ops/intern.h"
#include "io/print.h"
#include "memory/conspage.h"
#include "memory/consspaceobject.h"
#include "memory/hashmap.h"
#include "memory/vectorspace.h"


/**
 * A lisp function signature conforming wrapper around get_hash, q.v..
 */
struct cons_pointer lisp_get_hash( struct stack_frame *frame,
                                   struct cons_pointer frame_pointer,
                                   struct cons_pointer env ) {
    return make_integer( get_hash( frame->arg[0] ), NIL );
}

/**
 * Lisp funtion of up to four args (all optional), where
 * 
 * first is expected to be an integer, the number of buckets, or nil;
 * second is expected to be a hashing function, or nil;
 * third is expected to be an assocable, or nil;
 * fourth is a list of user tokens, to be used as a write ACL, or nil.
 */
struct cons_pointer lisp_make_hashmap( struct stack_frame *frame,
                                       struct cons_pointer frame_pointer,
                                       struct cons_pointer env ) {
    uint32_t n = DFLT_HASHMAP_BUCKETS;
    struct cons_pointer hash_fn = NIL;
    struct cons_pointer result = NIL;

    if ( frame->args > 0 ) {
        if ( integerp( frame->arg[0] ) ) {
            n = to_long_int( frame->arg[0] ) % UINT32_MAX;
        } else if ( !nilp( frame->arg[0] ) ) {
            result =
                make_exception( c_string_to_lisp_string
                                ( L"First arg to `hashmap`, if passed, must "
                                  L"be an integer or `nil`.`" ), NIL );
        }
    }
    if ( frame->args > 1 ) {
        if ( functionp( frame->arg[1])) {
        hash_fn = frame->arg[1];
        } else if ( nilp(frame->arg[1])){
            /* that's allowed */
        } else {
           result =
                make_exception( c_string_to_lisp_string
                                ( L"Second arg to `hashmap`, if passed, must "
                                  L"be a function or `nil`.`" ), NIL );
        }
    }

    if ( nilp( result ) ) {
        /* if there are fewer than 4 args, then arg[3] ought to be nil anyway, which
         * is fine */
        result = make_hashmap( n, hash_fn, frame->arg[3] );
        struct vector_space_object *map = pointer_to_vso( result );

        if ( frame->args > 2 &&
             truep( authorised( result, map->payload.hashmap.write_acl ) ) ) {
            // then arg[2] ought to be an assoc list which we should iterate down
            // populating the hashmap.
            for ( struct cons_pointer cursor = frame->arg[2]; !nilp( cursor );
                  cursor = c_cdr( cursor ) ) {
                struct cons_pointer pair = c_car( cursor );
                struct cons_pointer key = c_car( pair );
                struct cons_pointer val = c_cdr( pair );

                uint32_t bucket_no =
                    get_hash( key ) % ( ( struct hashmap_payload * )
                                        &( map->payload ) )->n_buckets;

                map->payload.hashmap.buckets[bucket_no] =
                    inc_ref( make_cons( make_cons( key, val ),
                                        map->payload.hashmap.
                                        buckets[bucket_no] ) );
            }
        }
    }

    // TODO: I am not sure this is right! We do not inc_ref a string when
    // we make it.
    inc_ref(result);
    return result;
}

/**
 * Expects `frame->arg[1]` to be a hashmap or namespace; `frame->arg[2]` to be
 * a string-like-thing (perhaps necessarily a keyword); frame->arg[3] to be 
 * any value. If
 * current user is authorised to write to this hashmap, modifies the hashmap and
 * returns it; if not, clones the hashmap, modifies the clone, and returns that.
 */
struct cons_pointer lisp_hashmap_put( struct stack_frame *frame,
                                      struct cons_pointer frame_pointer,
                                      struct cons_pointer env ) {
    // TODO: if current user has write access to this hashmap

    struct cons_pointer mapp = frame->arg[0];
    struct cons_pointer key = frame->arg[1];
    struct cons_pointer val = frame->arg[2];

    struct cons_pointer result = hashmap_put( mapp, key, val );
    struct cons_space_object *cell = &pointer2cell( result);
    // if (cell->count <= 1) {
    //     inc_ref( result); // TODO: I DO NOT BELIEVE this is the right place! 
    // }
    return result;

    // TODO: else clone and return clone.
}

/**
 * Lisp function expecting two arguments, a hashmap and an assoc list. Copies all
 * key/value pairs from the assoc list into the map.
 */
struct cons_pointer lisp_hashmap_put_all( struct stack_frame *frame,
                                          struct cons_pointer frame_pointer,
                                          struct cons_pointer env ) {
    return hashmap_put_all( frame->arg[0], frame->arg[1] );
}

struct cons_pointer lisp_hashmap_keys( struct stack_frame *frame,
                                       struct cons_pointer frame_pointer,
                                       struct cons_pointer env ) {
    return hashmap_keys( frame->arg[0] );
}

void dump_map( URL_FILE * output, struct cons_pointer pointer ) {
    struct hashmap_payload *payload =
        &pointer_to_vso( pointer )->payload.hashmap;
    url_fwprintf( output, L"Hashmap with %d buckets:\n", payload->n_buckets );
    url_fwprintf( output, L"\tHash function: " );
    print( output, payload->hash_fn );
    url_fwprintf( output, L"\n\tWrite ACL: " );
    print( output, payload->write_acl );
    url_fwprintf( output, L"\n\tBuckets:" );
    for ( int i = 0; i < payload->n_buckets; i++ ) {
        url_fwprintf( output, L"\n\t\t[%d]: ", i );
        print( output, payload->buckets[i] );
    }
    url_fwprintf( output, L"\n" );
}
