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
#include "equal.h"
#include "lispops.h"
#include "print.h"

/**
 * The object list. What is added to this during system setup is 'global', that is, 
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

    if (symbolp(key)) {
        for ( struct cons_pointer next = store;
              nilp( result ) && consp( next );
              next = pointer2cell( next ).payload.cons.cdr ) {
            struct cons_space_object entry =
                pointer2cell( pointer2cell( next ).payload.cons.car );

            fputws( L"Internedp: checking whether `", stderr);
            print(stderr, key);
            fputws( L"` equals `", stderr);
            print( stderr, entry.payload.cons.car);
            fputws( L"`\n", stderr);
            
            if ( equal( key, entry.payload.cons.car ) ) {
                result = entry.payload.cons.car;
            }
        }
    } else {
        fputws(L"`", stderr);
        print( stderr, key );
        fputws( L"` is a ", stderr);
        print( stderr, c_type( key));
        fputws( L", not a SYMB", stderr);
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

    for ( struct cons_pointer next = store;
          consp( next ); next = pointer2cell( next ).payload.cons.cdr ) {
        struct cons_space_object entry =
            pointer2cell( pointer2cell( next ).payload.cons.car );

        if ( equal( key, entry.payload.cons.car ) ) {
            result = entry.payload.cons.cdr;
            break;
        }
    }

    return result;
}

/**
 * Return a new key/value store containing all the key/value pairs in this store
 * with this key/value pair added to the front.
 */
struct cons_pointer
bind( struct cons_pointer key, struct cons_pointer value,
      struct cons_pointer store ) {
    return make_cons( make_cons( key, value ), store );
}

/**
 * Binds this key to this value in the global oblist, but doesn't affect the 
 * current environment. May not be useful except in bootstrapping (and even 
 * there it may not be especially useful).
 */
struct cons_pointer
deep_bind( struct cons_pointer key, struct cons_pointer value ) {
    oblist = bind( key, value, oblist );
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
        result = bind( key, NIL, environment );
    }

    return result;
}
