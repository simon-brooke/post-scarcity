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
                 * it will make a new clone for every key/value pair added. Fix. */
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
                hashmap_put( mapp, key, hashmap_get( assoc, key, false ) );
            }
        }
    }

    return mapp;
}

/** Get a value from a hashmap. 
  *
  * Note that this is here, rather than in memory/hashmap.c, because it is 
  * closely tied in with search_store, q.v.
  */
struct cons_pointer hashmap_get( struct cons_pointer mapp,
                                 struct cons_pointer key, bool return_key ) {
#ifdef DEBUG
    debug_print( L"\nhashmap_get: key is `", DEBUG_BIND );
    debug_print_object( key, DEBUG_BIND );
    debug_print( L"`; store of type `", DEBUG_BIND );
    debug_print_object( c_type( mapp ), DEBUG_BIND );
    debug_printf( DEBUG_BIND, L"`; returning `%s`.\n",
                  return_key ? "key" : "value" );
#endif

    struct cons_pointer result = NIL;
    if ( hashmapp( mapp ) && truep( authorised( mapp, NIL ) ) && !nilp( key ) ) {
        struct vector_space_object *map = pointer_to_vso( mapp );
        uint32_t bucket_no = get_hash( key ) % map->payload.hashmap.n_buckets;

        result =
            search_store( key, map->payload.hashmap.buckets[bucket_no],
                          return_key );
    }
#ifdef DEBUG
    debug_print( L"\nhashmap_get returning: `", DEBUG_BIND );
    debug_print_object( result, DEBUG_BIND );
    debug_print( L"`\n", DEBUG_BIND );
#endif

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

/**
 * @brief `(search-store key store return-key?)` Search this `store` for this
 * a key lexically identical to this `key`. 
 *
 * If found, then, if `return-key?` is non-nil, return the copy found in the 
 * `store`, else return the value associated with it.
 *
 * At this stage the following structures are legal stores:
 * 1. an association list comprising (key . value) dotted pairs;
 * 2. a hashmap;
 * 3. a namespace (which for these purposes is identical to a hashmap);
 * 4. a hybrid list comprising both (key . value) pairs and hashmaps as first
 *    level items;
 * 5. such a hybrid list, but where the last CDR pointer is to a hashmap
 *    rather than to a cons sell or to `nil`.
 *
 * This is over-complex and type 5 should be disallowed, but it will do for 
 * now.
 */
struct cons_pointer search_store( struct cons_pointer key,
                                  struct cons_pointer store,
                                  bool return_key ) {
    struct cons_pointer result = NIL;

#ifdef DEBUG
    debug_print( L"\nsearch_store; key is `", DEBUG_BIND );
    debug_print_object( key, DEBUG_BIND );
    debug_print( L"`; store of type `", DEBUG_BIND );
    debug_print_object( c_type( store ), DEBUG_BIND );
    debug_printf( DEBUG_BIND, L"`; returning `%s`.\n",
                  return_key ? "key" : "value" );
#endif

    switch ( get_tag_value( key ) ) {
        case SYMBOLTV:
        case KEYTV:
            struct cons_space_object *store_cell = &pointer2cell( store );

            switch ( get_tag_value( store ) ) {
                case CONSTV:
                    for ( struct cons_pointer cursor = store;
                          nilp( result ) && ( consp( cursor )
                                              || hashmapp( cursor ) );
                          cursor = pointer2cell( cursor ).payload.cons.cdr ) {
                        switch ( get_tag_value( cursor ) ) {
                            case CONSTV:
                                struct cons_pointer entry_ptr =
                                    c_car( cursor );

                                switch ( get_tag_value( entry_ptr ) ) {
                                    case CONSTV:
                                        if ( equal( key, c_car( entry_ptr ) ) ) {
                                            result =
                                                return_key ? c_car( entry_ptr )
                                                : c_cdr( entry_ptr );
                                            break;
                                        }
                                        break;
                                    case HASHTV:
                                    case NAMESPACETV:
                                        // TODO: I think this should be impossible, and we should maybe
                                        // throw an exception.
                                        result =
                                            hashmap_get( entry_ptr, key,
                                                         return_key );
                                        break;
                                    default:
                                        result =
                                            throw_exception
                                            ( c_string_to_lisp_symbol
                                              ( L"search-store (entry)" ),
                                              make_cons
                                              ( c_string_to_lisp_string
                                                ( L"Unexpected store type: " ),
                                                c_type( c_car( entry_ptr ) ) ),
                                              NIL );

                                }
                                break;
                            case HASHTV:
                            case NAMESPACETV:
                                debug_print
                                    ( L"\n\tHashmap as top-level value in list",
                                      DEBUG_BIND );
                                result =
                                    hashmap_get( cursor, key, return_key );
                                break;
                            default:
                                result =
                                    throw_exception( c_string_to_lisp_symbol
                                                     ( L"search-store (cursor)" ),
                                                     make_cons
                                                     ( c_string_to_lisp_string
                                                       ( L"Unexpected store type: " ),
                                                       c_type( cursor ) ),
                                                     NIL );
                        }
                    }
                    break;
                case HASHTV:
                case NAMESPACETV:
                    result = hashmap_get( store, key, return_key );
                    break;
                default:
                    result =
                        throw_exception( c_string_to_lisp_symbol
                                         ( L"search-store (store)" ),
                                         make_cons( c_string_to_lisp_string
                                                    ( L"Unexpected store type: " ),
                                                    c_type( store ) ), NIL );
                    break;
            }
            break;
        case EXCEPTIONTV:
            result =
                throw_exception( c_string_to_lisp_symbol
                                 ( L"search-store (exception)" ),
                                 make_cons( c_string_to_lisp_string
                                            ( L"Unexpected key type: " ),
                                            c_type( key ) ), NIL );

            break;
        default:
            result =
                throw_exception( c_string_to_lisp_symbol
                                 ( L"search-store (key)" ),
                                 make_cons( c_string_to_lisp_string
                                            ( L"Unexpected key type: " ),
                                            c_type( key ) ), NIL );
    }

    debug_print( L"search-store: returning `", DEBUG_BIND );
    debug_print_object( result, DEBUG_BIND );
    debug_print( L"`\n", DEBUG_BIND );

    return result;
}

struct cons_pointer interned( struct cons_pointer key,
                              struct cons_pointer store ) {
    return search_store( key, store, true );
}

/**
 * @brief Implementation of `interned?` in C.
 * 
 * @param key the key to search for.
 * @param store the store to search in.
 * @return struct cons_pointer `t` if the key was found, else `nil`.
 */
struct cons_pointer internedp( struct cons_pointer key,
                               struct cons_pointer store ) {
    struct cons_pointer result = NIL;

    if ( consp( store ) ) {
        for ( struct cons_pointer pair = c_car( store ); eq( result, NIL) && !nilp( pair );
                pair = c_car( store ) ) {
            if ( consp( pair ) ) {
                if ( equal( c_car( pair), key)) {
                    // yes, this should be `eq`, but if symbols are correctly 
                    // interned this will work efficiently, and if not it will
                    // still work.
                    result = TRUE; 
                }
            } else if ( hashmapp( pair ) ) {
                result=internedp( key, pair); 
            } 
            
            store = c_cdr( store );
        }
    } else if ( hashmapp( store ) ) {
        struct vector_space_object *map = pointer_to_vso( store );

        for ( int i = 0; i < map->payload.hashmap.n_buckets; i++ ) {
            for ( struct cons_pointer c = map->payload.hashmap.buckets[i];
                  !nilp( c ); c = c_cdr( c ) ) {
                result = internedp( key, c);
            }
        }
    }

    return result;
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
    return search_store( key, store, false );
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

    debug_print( L"hashmap_put:\n", DEBUG_BIND );
    debug_dump_object( mapp, DEBUG_BIND );

    return mapp;
}

/**
 * If this store is modifiable, add this key value pair to it. Otherwise,
 * return a new key/value store containing all the key/value pairs in this
 * store with this key/value pair added to the front.
 */
struct cons_pointer set( struct cons_pointer key, struct cons_pointer value,
                         struct cons_pointer store ) {
    struct cons_pointer result = NIL;

#ifdef DEBUG
    bool deep = eq( store, oblist );
    debug_print_binding( key, value, deep, DEBUG_BIND );

    if ( deep ) {
        debug_printf( DEBUG_BIND, L"\t-> %4.4s\n",
                      pointer2cell( store ).payload.vectorp.tag.bytes );
    }
#endif
    if ( nilp( store ) || consp( store ) ) {
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

    oblist = set( key, value, oblist );

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
         * not currently bound. TODO: this should bind to NIL?
         */
        result = set( key, TRUE, environment );
    }

    return result;
}
