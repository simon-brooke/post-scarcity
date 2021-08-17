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
#include "intern.h"
#include "io/print.h"
#include "memory/conspage.h"
#include "memory/consspaceobject.h"
#include "memory/hashmap.h"
#include "memory/vectorspace.h"

/**
 * Return a hash value for the structure indicated by `ptr` such that if
 * `x`,`y` are two separate structures whose print representation is the same
 * then `(sxhash x)` and `(sxhash y)` will always be equal.
 */
uint32_t sxhash( struct cons_pointer ptr ) {
    // TODO: Not Yet Implemented
    /* TODO: should look at the implementation of Common Lisp sxhash?
     * My current implementation of `print` only addresses URL_FILE
     * streams. It would be better if it also addressed strings but
     * currently it doesn't. Creating a print string of the structure
     * and taking the hash of that would be one simple (but not necessarily
     * cheap) solution.
     */
    /* TODO: sbcl's implementation of `sxhash` is in src/compiler/sxhash.lisp
     * and is EXTREMELY complex, and essentially has a different dispatch for
     * every type of object. It's likely we need to do the same.
     */
    return 0;
}

/**
 * Get the hash value for the cell indicated by this `ptr`; currently only
 * implemented for string like things and integers.
 */
uint32_t get_hash( struct cons_pointer ptr ) {
    struct cons_space_object *cell = &pointer2cell( ptr );
    uint32_t result = 0;

    switch ( cell->tag.value ) {
        case INTEGERTV:
            /* Note that we're only hashing on the least significant word of an
             * integer. */
            result = cell->payload.integer.value & 0xffffffff;
            break;
        case KEYTV:
        case STRINGTV:
        case SYMBOLTV:
            result = cell->payload.string.hash;
            break;
        case TRUETV:
            result = 1;         // arbitrarily
            break;
        default:
            result = sxhash( ptr );
            break;
    }

    return result;
}

/**
 * Free the hashmap indicated by this `pointer`.
 */
void free_hashmap( struct cons_pointer pointer ) {
    struct cons_space_object *cell = &pointer2cell( pointer );

    if ( hashmapp( pointer ) ) {
        struct vector_space_object *vso = cell->payload.vectorp.address;

        dec_ref( vso->payload.hashmap.hash_fn );
        dec_ref( vso->payload.hashmap.write_acl );

        for ( int i = 0; i < vso->payload.hashmap.n_buckets; i++ ) {
            if ( !nilp( vso->payload.hashmap.buckets[i] ) ) {
                debug_printf( DEBUG_ALLOC,
                              L"Decrementing bucket [%d] of hashmap at 0x%lx\n",
                              i, cell->payload.vectorp.address );
                dec_ref( vso->payload.hashmap.buckets[i] );
            }
        }
    } else {
        debug_printf( DEBUG_ALLOC, L"Non-hashmap passed to `free_hashmap`\n" );
    }
}

/**
 * A lisp function signature conforming wrapper around get_hash, q.v..
 */
struct cons_pointer lisp_get_hash( struct stack_frame *frame,
                                   struct cons_pointer frame_pointer,
                                   struct cons_pointer env ) {
    return make_integer( get_hash( frame->arg[0] ), NIL );
}

/**
 * Make a hashmap with this number of buckets, using this `hash_fn`. If 
 * `hash_fn` is `NIL`, use the standard hash funtion.
 */
struct cons_pointer make_hashmap( uint32_t n_buckets,
                                  struct cons_pointer hash_fn,
                                  struct cons_pointer write_acl ) {
    struct cons_pointer result =
        make_vso( HASHTV,
                  ( sizeof( struct cons_pointer ) * ( n_buckets + 1 ) ) +
                  ( sizeof( uint32_t ) * 2 ) );

    struct hashmap_payload *payload =
        ( struct hashmap_payload * ) &pointer_to_vso( result )->payload;

    payload->hash_fn = inc_ref( hash_fn );
    payload->write_acl = inc_ref( write_acl );

    payload->n_buckets = n_buckets;
    for ( int i = 0; i < n_buckets; i++ ) {
        payload->buckets[i] = NIL;
    }

    return result;
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
        hash_fn = frame->arg[1];
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
                    get_hash( key ) %
                    ( ( struct hashmap_payload * ) &( map->payload ) )->
                    n_buckets;

                map->payload.hashmap.buckets[bucket_no] =
                    inc_ref( make_cons( make_cons( key, val ),
                                        map->payload.hashmap.
                                        buckets[bucket_no] ) );
            }
        }
    }

    return result;
}



/**
 * If this `ptr` is a pointer to a hashmap, return a new identical hashmap; 
 * else return `NIL`. TODO: should return an exception if ptr is not a 
 * readable hashmap.
 */
struct cons_pointer clone_hashmap( struct cons_pointer ptr ) {
    struct cons_pointer result = NIL;

    if ( truep( authorised( ptr, NIL ) ) ) {
        if ( hashmapp( ptr ) ) {
            struct vector_space_object *from = pointer_to_vso( ptr );

            if ( from != NULL ) {
                struct hashmap_payload from_pl = from->payload.hashmap;
                result =
                    make_hashmap( from_pl.n_buckets, from_pl.hash_fn,
                                  from_pl.write_acl );
                struct vector_space_object *to = pointer_to_vso( result );
                struct hashmap_payload to_pl = to->payload.hashmap;

                for ( int i = 0; i < to_pl.n_buckets; i++ ) {
                    to_pl.buckets[i] = from_pl.buckets[i];
                    inc_ref( to_pl.buckets[i] );
                }
            }
        }
    }
    // TODO: else exception?

    return result;
}

/**
 * Store this `val` as the value of this `key` in this hashmap `mapp`. If
 * current user is authorised to write to this hashmap, modifies the hashmap and
 * returns it; if not, clones the hashmap, modifies the clone, and returns that.
 */
struct cons_pointer hashmap_put( struct cons_pointer mapp,
                                 struct cons_pointer key,
                                 struct cons_pointer val ) {
    // TODO: if current user has write access to this hashmap
    if ( hashmapp( mapp ) && !nilp( key ) ) {
        struct vector_space_object *map = pointer_to_vso( mapp );

        if ( nilp( authorised( mapp, map->payload.hashmap.write_acl ) ) ) {
            mapp = clone_hashmap( mapp );
            map = pointer_to_vso( mapp );
        }
        uint32_t bucket_no = get_hash( key ) % map->payload.hashmap.n_buckets;

        map->payload.hashmap.buckets[bucket_no] =
            inc_ref( make_cons( make_cons( key, val ),
                                map->payload.hashmap.buckets[bucket_no] ) );
    }

    return mapp;
}

struct cons_pointer hashmap_get( struct cons_pointer mapp,
                                 struct cons_pointer key ) {
    struct cons_pointer result = NIL;
    if ( hashmapp( mapp ) && truep( authorised( mapp, NIL ) ) && !nilp( key ) ) {
        struct vector_space_object *map = pointer_to_vso( mapp );
        uint32_t bucket_no = get_hash( key ) % map->payload.hashmap.n_buckets;

        result = c_assoc( key, map->payload.hashmap.buckets[bucket_no] );
    }

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
    struct cons_pointer mapp = frame->arg[0];
    struct cons_pointer key = frame->arg[1];
    struct cons_pointer val = frame->arg[2];

    return hashmap_put( mapp, key, val );
}

/**
 * Copy all key/value pairs in this association list `assoc` into this hashmap `mapp`. If
 * current user is authorised to write to this hashmap, modifies the hashmap and
 * returns it; if not, clones the hashmap, modifies the clone, and returns that.
 */
struct cons_pointer hashmap_put_all( struct cons_pointer mapp,
                                     struct cons_pointer assoc ) {
    // TODO: if current user has write access to this hashmap
    if ( hashmapp( mapp ) && !nilp( assoc ) ) {
        struct vector_space_object *map = pointer_to_vso( mapp );

        if ( hashmapp( mapp ) && consp( assoc ) ) {
            for ( struct cons_pointer pair = c_car( assoc ); !nilp( pair );
                  pair = c_car( assoc ) ) {
                /* TODO: this is really hammering the memory management system, because
                 * it will make a new lone for every key/value pair added. Fix. */
                mapp = hashmap_put( mapp, c_car( pair ), c_cdr( pair ) );
            }
        }
    }

    return mapp;
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

/**
 * return a flat list of all the keys in the hashmap indicated by `map`.
 */
struct cons_pointer hashmap_keys( struct cons_pointer mapp ) {
    struct cons_pointer result = NIL;
    if ( hashmapp( mapp ) && truep( authorised( mapp, NIL ) ) ) {
        struct vector_space_object *map = pointer_to_vso( mapp );

        for ( int i = 0; i < map->payload.hashmap.n_buckets; i++ ) {
            for ( struct cons_pointer c = map->payload.hashmap.buckets[i];
                  !nilp( c ); c = c_cdr( c ) ) {
                result = make_cons( c_car( c_car( c ) ), result );
            }

        }
    }

    return result;
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
