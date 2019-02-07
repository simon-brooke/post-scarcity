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

#include "conspage.h"
#include "consspaceobject.h"
#include "debug.h"
#include "equal.h"
#include "lispops.h"
#include "map.h"
#include "print.h"

/**
 * The global object list/or, to put it differently, the root namespace.
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
 * Implementation of interned? in C. The final implementation if interned? will
 * deal with stores which can be association lists or hashtables or hybrids of
 * the two, but that will almost certainly be implemented in lisp.
 *
 * If this key is lexically identical to a key in this store, return the key
 * from the store (so that later when we want to retrieve a value, an eq test
 * will work); otherwise return NIL.
 */
struct cons_pointer
internedp( struct cons_pointer key, struct cons_pointer store ) {
    struct cons_pointer result = NIL;

    if ( symbolp( key ) || keywordp( key ) ) {
        for ( struct cons_pointer next = store;
              nilp( result ) && consp( next );
              next = pointer2cell( next ).payload.cons.cdr ) {
            struct cons_space_object entry =
                pointer2cell( pointer2cell( next ).payload.cons.car );

            debug_print( L"Internedp: checking whether `", DEBUG_BIND );
            debug_print_object( key, DEBUG_BIND );
            debug_print( L"` equals `", DEBUG_BIND );
            debug_print_object( entry.payload.cons.car, DEBUG_BIND );
            debug_print( L"`\n", DEBUG_BIND );

            if ( equal( key, entry.payload.cons.car ) ) {
                result = entry.payload.cons.car;
            }
        }
    } else {
        debug_print( L"`", DEBUG_BIND );
        debug_print_object( key, DEBUG_BIND );
        debug_print( L"` is a ", DEBUG_BIND );
        debug_print_object( c_type( key ), DEBUG_BIND );
        debug_print( L", not a KEYW or SYMB", DEBUG_BIND );
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
    struct cons_pointer result = NIL;

    debug_print( L"c_assoc; key is `", DEBUG_BIND);
    debug_print_object( key, DEBUG_BIND);
    debug_print( L"`\n", DEBUG_BIND);

    if (consp(store)) {
        for ( struct cons_pointer next = store;
             consp( next ); next = pointer2cell( next ).payload.cons.cdr ) {
            struct cons_space_object entry =
                pointer2cell( pointer2cell( next ).payload.cons.car );

            if ( equal( key, entry.payload.cons.car ) ) {
                result = entry.payload.cons.cdr;
                break;
            }
        }
    } else if (vectorpointp( store)) {
        result = assoc_in_map( key, store);
    }

    debug_print( L"c_assoc returning ", DEBUG_BIND);
    debug_print_object( result, DEBUG_BIND);
    debug_println( DEBUG_BIND);

    return result;
}

/**
 * Return a new key/value store containing all the key/value pairs in this store
 * with this key/value pair added to the front.
 */
struct cons_pointer
    set( struct cons_pointer key, struct cons_pointer value,
        struct cons_pointer store ) {
    struct cons_pointer result = NIL;

    debug_print( L"set: binding `", DEBUG_BIND );
    debug_print_object( key, DEBUG_BIND );
    debug_print( L"` to `", DEBUG_BIND );
    debug_print_object( value, DEBUG_BIND );
    debug_print( L"` in store ", DEBUG_BIND );
    debug_dump_object( store, DEBUG_BIND);
    debug_println( DEBUG_BIND );

    if (nilp( store) || consp(store)) {
        result = make_cons( make_cons( key, value ), store );
    } else if (vectorpointp( store)) {
        result = bind_in_map( store, key, value);
    }

    debug_print( L"set returning ", DEBUG_BIND);
    debug_print_object( result, DEBUG_BIND);
    debug_println( DEBUG_BIND);

    return result;
}

/**
 * Binds this key to this value in the global oblist, but doesn't affect the
 * current environment. May not be useful except in bootstrapping (and even
 * there it may not be especially useful).
 */
struct cons_pointer
deep_bind( struct cons_pointer key, struct cons_pointer value ) {
    debug_print( L"Entering deep_bind\n", DEBUG_BIND );
    struct cons_pointer old = oblist;

    debug_print( L"deep_bind: binding `", DEBUG_BIND );
    debug_print_object( key, DEBUG_BIND );
    debug_print( L"` to ", DEBUG_BIND );
    debug_print_object( value, DEBUG_BIND );
    debug_println( DEBUG_BIND );

    oblist = set( key, value, oblist );
    inc_ref( oblist );
    dec_ref( old );

    debug_print( L"deep_bind returning ", DEBUG_BIND );
    debug_print_object( oblist, DEBUG_BIND );
    debug_println( DEBUG_BIND );

    return oblist;
}

/**
 * Ensure that a canonical copy of this key is bound in this environment, and
 * return that canonical copy. If there is currently no such binding, create one
 * with the value NIL.
 */
struct cons_pointer
intern( struct cons_pointer key, struct cons_pointer environment ) {
    struct cons_pointer result = environment;
    struct cons_pointer canonical = internedp( key, environment );
    if ( nilp( canonical ) ) {
        /*
         * not currently bound
         */
        result = set( key, NIL, environment );
    }

    return result;
}
