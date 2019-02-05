/*
 * map.c
 *
 * An immutable hashmap in vector space.
 *
 * (c) 2019 Simon Brooke <simon@journeyman.cc>
 * Licensed under GPL version 2.0, or, at your option, any later version.
 */

#include <stdlib.h>

#include "consspaceobject.h"
#include "conspage.h"
#include "debug.h"
#include "dump.h"
#include "fopen.h"
#include "intern.h"
#include "lookup3.h"
#include "map.h"
#include "print.h"
#include "vectorspace.h"

/* \todo: a lot of this will be inherited by namespaces, regularities and
 * homogeneities. Exactly how I don't yet know. */

/**
 * Get a hash value for this key.
 */
uint32_t get_hash_32(struct cons_pointer f, struct cons_pointer key) {
    uint32_t result = 0;
    int l =  c_length(key);

    if (keywordp(key) || stringp(key)) {
        if ( l > 0) {
            uint32_t buffer[l];

            if (!nilp(f)) {
                fputws(L"Custom hashing functions are not yet implemented.\n", stderr);
            }
            for (int i = 0; i < l; i++) {
                buffer[i] = (uint32_t)pointer2cell(key).payload.string.character;
            }

            result = hashword( buffer, l, 0);
        }
    } else {
        fputws(L"Hashing is thud far implemented only for keys and strings.\n", stderr);
    }

    return result;
}

/**
 * get the actual map object from this `pointer`, or NULL if
 * `pointer` is not a map pointer.
 */
struct map_payload *get_map_payload( struct cons_pointer pointer ) {
    struct map_payload *result = NULL;
    struct vector_space_object *vso =
        pointer2cell( pointer ).payload.vectorp.address;

    if (vectorpointp(pointer) && mapp( vso ) ) {
        result = ( struct map_payload * ) &( vso->payload );
        debug_printf( DEBUG_STACK,
                      L"get_map_payload: all good, returning %p\n", result );
    } else {
        debug_print( L"get_map_payload: fail, returning NULL\n", DEBUG_STACK );
    }

    return result;
}


/**
 * Make an empty immutable map, and return it.
 *
 * @param hash_function a pointer to a function of one argument, which
 * returns an integer; or (more usually) `nil`.
 * @return the new map, or NULL if memory is exhausted.
 */
struct cons_pointer make_empty_map( struct cons_pointer hash_function ) {
    debug_print( L"Entering make_empty_map\n", DEBUG_ALLOC );
    struct cons_pointer result =
        make_vso( MAPTAG, sizeof( struct map_payload ) );

    if ( !nilp( result ) ) {
        struct map_payload *payload = get_map_payload( result );

        payload->hash_function = functionp( hash_function) ? hash_function : NIL;
        inc_ref(hash_function);

        for ( int i = 0; i < BUCKETSINMAP; i++) {
            payload->buckets[i] = NIL;
        }
    }

    return result;
}


struct cons_pointer make_duplicate_map( struct cons_pointer parent) {
    struct cons_pointer result = NIL;
    struct map_payload * parent_payload = get_map_payload(parent);

    if (parent_payload != NULL) {
        result =
            make_vso( MAPTAG, sizeof( struct map_payload ) );

        if ( !nilp( result ) ) {
            struct map_payload *payload = get_map_payload( result );

            payload->hash_function = parent_payload->hash_function;
            inc_ref(payload->hash_function);

            for ( int i = 0; i < BUCKETSINMAP; i++) {
                payload->buckets[i] = parent_payload->buckets[i];
                inc_ref(payload->buckets[i]);
            }
        }
    }

    return result;
}


struct cons_pointer bind_in_map( struct cons_pointer parent,
                                struct cons_pointer key,
                                struct cons_pointer value) {
    struct cons_pointer result = make_duplicate_map(parent);

    if ( !nilp( result)) {
        struct map_payload * payload = get_map_payload( result );
        int bucket = get_hash_32(payload->hash_function, key) % BUCKETSINMAP;

        payload->buckets[bucket] = make_cons(
            make_cons(key, value), payload->buckets[bucket]);

        inc_ref(payload->buckets[bucket]);
    }

    return result;
}


struct cons_pointer keys( struct cons_pointer store) {
    struct cons_pointer result = NIL;

    struct cons_space_object cell = pointer2cell( store );

    switch (pointer2cell( store ).tag.value) {
        case CONSTV:
        for (struct cons_pointer c = store; !nilp(c); c = c_cdr(c)) {
            result = make_cons( c_car( c_car( c)), result);
        }
        break;
        case VECTORPOINTTV: {
            struct vector_space_object *vso =
                pointer2cell( store ).payload.vectorp.address;

            if ( mapp( vso ) ) {
                struct map_payload * payload = get_map_payload( result );

                for (int bucket = 0; bucket < BUCKETSINMAP; bucket++) {
                    for (struct cons_pointer c = payload->buckets[bucket];
                         !nilp(c); c = c_cdr(c)) {
                        result = make_cons( c_car( c_car( c)), result);
                    }
                }
            }
        }
        break;
    }

    return result;
}

/**
 * Return a new map which represents the merger of `to_merge` into
 * `parent`. `parent` must be a map, but `to_merge` may be a map or
 * an assoc list.
 *
 * @param parent a map;
 * @param to_merge an association from which key/value pairs will be merged.
 * @result a new map, containing all key/value pairs from `to_merge`
 * together with those key/value pairs from `parent` whose keys did not
 * collide.
 */
struct cons_pointer merge_into_map( struct cons_pointer parent,
                                   struct cons_pointer to_merge) {
    struct cons_pointer result = make_duplicate_map(parent);

    if (!nilp(result)) {
        struct map_payload *payload = get_map_payload( result );
        for (struct cons_pointer c = keys(to_merge);
             !nilp(c); c = c_cdr(c)) {
            struct cons_pointer key = c_car( c);
            int bucket = get_hash_32(payload->hash_function, key) % BUCKETSINMAP;

            payload->buckets[bucket] = make_cons(
                make_cons( key, c_assoc( key, to_merge)),
                payload->buckets[bucket]);
        }
    }

    return result;
}


struct cons_pointer assoc_in_map( struct cons_pointer map,
                                  struct cons_pointer key) {
    struct cons_pointer result = NIL;
    struct map_payload *payload = get_map_payload( map );

    if (payload != NULL) {
        int bucket = get_hash_32(payload->hash_function, key) % BUCKETSINMAP;

        result = c_assoc(key, payload->buckets[bucket]);
    }

    return result;
}

/**
 * Dump a map to this stream for debugging
 * @param output the stream
 * @param map_pointer the pointer to the frame
 */
void dump_map( URL_FILE * output, struct cons_pointer map_pointer ) {
    struct vector_space_object *vso =
        pointer2cell( map_pointer ).payload.vectorp.address;

    if (vectorpointp(map_pointer) && mapp( vso ) ) {
        struct map_payload *payload = get_map_payload( map_pointer );

        if ( payload != NULL ) {
            url_fputws( L"Immutable map; hash function:", output );

            if (nilp(payload->hash_function)) {
                url_fputws( L"default", output);
            } else {
                dump_object( output, payload->hash_function);
            }

            for (int i = 0; i < BUCKETSINMAP; i++) {
                url_fwprintf(output, L"\n\tBucket %d: ", i);
                print( output, payload->buckets[i]);
            }
        }
    }
}

