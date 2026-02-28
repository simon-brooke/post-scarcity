/*
 * intern.c
 *
 * For now this implements an oblist and shallow binding; local environments can
 * be consed onto the front of the oblist. Later, this won't do; bindings will happen
 * in namespaces, which will probably be implemented as hash tables.
 *
 * Doctrine is that cons cells are immutable, and life is a lot more simple if they are;
 * so when a symbol is rebound in the master oblist, what in fact we do is construct
 * a new oblist without the previous binding but with the new binding. Anything which,
 * prior to this action, held a pointer to the old oblist (as all current threads'
 * environments must do) continues to hold a pointer to the old oblist, and consequently
 * doesn't see the change. This is probably good but does mean you cannot use bindings
 * on the oblist to signal between threads.
 *
 * (c) 2017 Simon Brooke <simon@journeyman.cc>
 * Licensed under GPL version 2.0, or, at your option, any later version.
 */

#include <stdbool.h>
#include <string.h>
/*
 * wide characters
 */
#include <wchar.h>
#include <wctype.h>

#include "authorise.h"
#include "debug.h"
#include "io/io.h"
#include "memory/conspage.h"
#include "memory/consspaceobject.h"
#include "memory/hashmap.h"
#include "ops/equal.h"
#include "ops/intern.h"
#include "ops/lispops.h"
// #include "print.h"

/**
 * @brief The global object list/or, to put it differently, the root namespace.
 * What is added to this during system setup is 'global', that is,
 * visible to all sessions/threads. What is added during a session/thread is local to
 * that session/thread (because shallow binding). There must be some way for a user to
 * make the contents of their own environment persistent between threads but I don't
 * know what it is yet. At some stage there must be a way to rebind deep values so
 * they're visible to all users/threads, but again I don't yet have any idea how
 * that will work.
 */
struct cons_pointer oblist = NIL;

/**
 * @brief the symbol `NIL`, which is special!
 * 
 */
struct cons_pointer privileged_symbol_nil = NIL;

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
 * Make a hashmap with this number of buckets, using this `hash_fn`. If 
 * `hash_fn` is `NIL`, use the standard hash funtion.
 */
struct cons_pointer make_hashmap( uint32_t n_buckets,
                                  struct cons_pointer hash_fn,
                                  struct cons_pointer write_acl ) {
    struct cons_pointer result = make_vso( HASHTV,
                                           ( sizeof( struct cons_pointer ) *
                                             ( n_buckets + 2 ) ) +
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

/**
 * Copy all key/value pairs in this association list `assoc` into this hashmap `mapp`. If
 * current user is authorised to write to this hashmap, modifies the hashmap and
 * returns it; if not, clones the hashmap, modifies the clone, and returns that.
 */
struct cons_pointer hashmap_put_all( struct cons_pointer mapp,
                                     struct cons_pointer assoc ) {
    // TODO: if current user has write access to this hashmap
    if ( hashmapp( mapp ) ) {
        struct vector_space_object *map = pointer_to_vso( mapp );

        if ( consp( assoc ) ) {
            for ( struct cons_pointer pair = c_car( assoc ); !nilp( pair );
                  pair = c_car( assoc ) ) {
                /* TODO: this is really hammering the memory management system, because
                 * it will make a new lone for every key/value pair added. Fix. */
                if ( consp( pair ) ) {
                    mapp = hashmap_put( mapp, c_car( pair ), c_cdr( pair ) );
                } else if ( hashmapp( pair ) ) {
                    hashmap_put_all( mapp, pair );
                } else {
                    hashmap_put( mapp, pair, TRUE );
                }
                assoc = c_cdr( assoc );
            }
        } else if ( hashmapp( assoc ) ) {
            for ( struct cons_pointer keys = hashmap_keys( assoc );
                  !nilp( keys ); keys = c_cdr( keys ) ) {
                struct cons_pointer key = c_car( keys );
                hashmap_put( mapp, key, hashmap_get( assoc, key ) );
            }
        }
    }

    return mapp;
}

/** Get a value from a hashmap. 
  *
  * Note that this is here, rather than in memory/hashmap.c, because it is 
  * closely tied in with c_assoc, q.v.
  */
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
 * If this `ptr` is a pointer to a hashmap, return a new identical hashmap; 
 * else return an exception. 
 */
struct cons_pointer clone_hashmap( struct cons_pointer ptr ) {
    struct cons_pointer result = NIL;

    if ( truep( authorised( ptr, NIL ) ) ) {
        if ( hashmapp( ptr ) ) {
            struct vector_space_object const *from = pointer_to_vso( ptr );

            if ( from != NULL ) {
                struct hashmap_payload from_pl = from->payload.hashmap;
                result =
                    make_hashmap( from_pl.n_buckets, from_pl.hash_fn,
                                  from_pl.write_acl );
                struct vector_space_object const *to =
                    pointer_to_vso( result );
                struct hashmap_payload to_pl = to->payload.hashmap;

                for ( int i = 0; i < to_pl.n_buckets; i++ ) {
                    to_pl.buckets[i] = from_pl.buckets[i];
                    inc_ref( to_pl.buckets[i] );
                }
            }
        }
    } else {
        result =
            make_exception( c_string_to_lisp_string
                            ( L"Arg to `clone_hashmap` must "
                              L"be a readable hashmap.`" ), NIL );
    }

    return result;
}

// (keys set let quote read equal *out* *log* oblist cons source cond close meta mapcar negative? open subtract eval nλ *in* *sink* cdr set! reverse slurp try assoc eq add list time car t *prompt* absolute append apply divide exception get-hash hashmap inspect metadata multiply print put! put-all! read-char repl throw type + * - / = lambda λ nlambda progn)

/**
 * If this key is lexically identical to a key in this store, return the key
 * from the store (so that later when we want to retrieve a value, an eq test
 * will work); otherwise return NIL.
 */
struct cons_pointer interned( struct cons_pointer key,
                              struct cons_pointer store ) {
    struct cons_pointer result = NIL;

    if ( symbolp( key ) || keywordp( key ) ) {
        struct cons_space_object *cell = &pointer2cell( store );

        switch ( cell->tag.value ) {
            case CONSTV:
                for ( struct cons_pointer next = store;
                      nilp( result ) && consp( next );
                      next = c_cdr( next) ) {
                    if ( !nilp( next ) ) {
                        // struct cons_space_object entry =
                        //     pointer2cell( c_car( next) );

                        if ( equal( key, c_car(next) ) ) {
                            result = key;
                        }
                    }
                }
                break;
            case VECTORPOINTTV:
                if ( hashmapp( store ) || namespacep( store ) ) {
                    // get the right hash bucket and recursively call interned on that.
                    struct vector_space_object *map = pointer_to_vso( store );
                    uint32_t bucket_no =
                        get_hash( key ) % map->payload.hashmap.n_buckets;

                    result =
                        interned( key,
                                  map->payload.hashmap.buckets[bucket_no] );
                } else {
                    result =
                        throw_exception( make_cons
                                         ( c_string_to_lisp_string
                                           ( L"Unexpected store type: " ),
                                           c_type( store ) ), NIL );
                }
                break;
            default:
                result =
                    throw_exception( make_cons
                                     ( c_string_to_lisp_string
                                       ( L"Unexpected store type: " ),
                                       c_type( store ) ), NIL );
                break;
        }
    } else {
        result =
            throw_exception( make_cons
                             ( c_string_to_lisp_string
                               ( L"Unexpected key type: " ), c_type( key ) ),
                             NIL );
    }

    return result;
}

/**
 * @brief Implementation of `interned?` in C: predicate wrapped around interned.
 * 
 * @param key the key to search for.
 * @param store the store to search in.
 * @return struct cons_pointer `t` if the key was found, else `nil`.
 */
struct cons_pointer internedp( struct cons_pointer key,
                               struct cons_pointer store ) {
    return nilp( interned( key, store ) ) ? NIL : TRUE;
}

/**
 * Implementation of assoc in C. Like interned?, the final implementation will
 * deal with stores which can be association lists or hashtables or hybrids of
 * the two, but that will almost certainly be implemented in lisp.
 *
 * If this key is lexically identical to a key in this store, return the value
 * of that key from the store; otherwise return NIL.
 */
struct cons_pointer c_assoc( struct cons_pointer key,
                             struct cons_pointer store ) {
    struct cons_pointer result = NIL;

    if ( !nilp( key ) ) {
        if ( consp( store ) ) {
            for ( struct cons_pointer next = store;
                  nilp( result ) && ( consp( next ) || hashmapp( next ) );
                  next = pointer2cell( next ).payload.cons.cdr ) {
                if ( consp( next ) ) {
// #ifdef DEBUG
//                     debug_print( L"\nc_assoc; key is `", DEBUG_BIND );
//                     debug_print_object( key, DEBUG_BIND );
//                     debug_print( L"`\n", DEBUG_BIND );
// #endif

                    struct cons_pointer entry_ptr = c_car( next );
                    struct cons_space_object entry = pointer2cell( entry_ptr );

                    switch ( entry.tag.value ) {
                        case CONSTV:
                            if ( equal( key, entry.payload.cons.car ) ) {
                                result = entry.payload.cons.cdr;
                            }
                            break;
                        case VECTORPOINTTV:
                            result = hashmap_get( entry_ptr, key );
                            break;
                        default:
                            throw_exception( c_append
                                             ( c_string_to_lisp_string
                                               ( L"Store entry is of unknown type: " ),
                                               c_type( entry_ptr ) ), NIL );
                    }

// #ifdef DEBUG
//                     debug_print( L"c_assoc `", DEBUG_BIND );
//                     debug_print_object( key, DEBUG_BIND );
//                     debug_print( L"` returning: ", DEBUG_BIND );
//                     debug_print_object( result, DEBUG_BIND );
//                     debug_println( DEBUG_BIND );
// #endif
                }
            }
        } else if ( hashmapp( store ) || namespacep( store ) ) {
            result = hashmap_get( store, key );
        } else if ( !nilp( store ) ) {
// #ifdef DEBUG        
//             debug_print( L"c_assoc; store is of unknown type `", DEBUG_BIND );
//             debug_printf( DEBUG_BIND, L"%4.4s", (char *)pointer2cell(key).tag.bytes);
//             debug_print( L"`\n", DEBUG_BIND );
// #endif
            result =
                throw_exception( c_append
                                 ( c_string_to_lisp_string
                                   ( L"Store is of unknown type: " ),
                                   c_type( store ) ), NIL );
        }
    }

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
    if ( hashmapp( mapp ) && !nilp( key ) ) {
        struct vector_space_object *map = pointer_to_vso( mapp );

        if ( nilp( authorised( mapp, map->payload.hashmap.write_acl ) ) ) {
            mapp = clone_hashmap( mapp );
            map = pointer_to_vso( mapp );
        }
        uint32_t bucket_no = get_hash( key ) % map->payload.hashmap.n_buckets;

        // TODO: if there are too many values in the bucket, rehash the whole 
        // hashmap to a bigger number of buckets, and return that.

        map->payload.hashmap.buckets[bucket_no] =
            make_cons( make_cons( key, val ),
                       map->payload.hashmap.buckets[bucket_no] );
    }

    return mapp;
}

    /**
     * Return a new key/value store containing all the key/value pairs in this
     * store with this key/value pair added to the front.
     */
struct cons_pointer set( struct cons_pointer key, struct cons_pointer value,
                         struct cons_pointer store ) {
    struct cons_pointer result = NIL;

#ifdef DEBUG
    bool deep = eq( store, oblist);
    debug_print_binding( key, value, deep, DEBUG_BIND );

    if ( deep ) {
        debug_printf( DEBUG_BIND, L"\t-> %4.4s\n",
                      pointer2cell( store ).payload.vectorp.tag.bytes );
    }
#endif
    if ( nilp( value ) ) {
        result = store;
    } else if ( nilp( store ) || consp( store ) ) {
        result = make_cons( make_cons( key, value ), store );
    } else if ( hashmapp( store ) ) {
        result = hashmap_put( store, key, value );
    }

    return result;
}

/**
 * @brief Binds this `key` to this `value` in the global oblist, and returns the `key`. 
 */
struct cons_pointer
deep_bind( struct cons_pointer key, struct cons_pointer value ) {
    debug_print( L"Entering deep_bind\n", DEBUG_BIND );

    struct cons_pointer old = oblist;

    oblist = set( key, value, oblist );

    // The oblist is not now an assoc list, and I don't think it will be again.
    // if ( consp( oblist ) ) {
    //     inc_ref( oblist );
    //     dec_ref( old );
    // }

    debug_print( L"deep_bind returning ", DEBUG_BIND );
    debug_print_object( key, DEBUG_BIND );
    debug_println( DEBUG_BIND );

    return key;
}

/**
 * Ensure that a canonical copy of this key is bound in this environment, and
 * return that canonical copy. If there is currently no such binding, create one
 * with the value TRUE.
 */
struct cons_pointer
intern( struct cons_pointer key, struct cons_pointer environment ) {
    struct cons_pointer result = environment;
    struct cons_pointer canonical = internedp( key, environment );
    if ( nilp( canonical ) ) {
        /*
         * not currently bound. TODO: should this bind to NIL?
         */
        result = set( key, TRUE, environment );
    }

    return result;
}
