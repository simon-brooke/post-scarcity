/**
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

#include "equal.h"
#include "conspage.h"
#include "consspaceobject.h"
#include "equal.h"

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
 * return the value associated with this key in this store. In the current 
 * implementation a store is just an assoc list, but in future it might be a
 * namespace, a regularity or a homogeneity.
 * TODO: This function ultimately belongs in lispops.
 */
struct cons_pointer assoc( struct cons_pointer key, struct cons_pointer store) {
  struct cons_pointer result = NIL;
  
  if ( consp( store)) {
    struct cons_space_object* cell_store = &pointer2cell( store);

    if ( consp( cell_store->payload.cons.car)) {
      struct cons_space_object* binding =
	&pointer2cell( cell_store->payload.cons.car);

      if ( eq( key, binding->payload.cons.car)) {
	result = binding->payload.cons.cdr;
      }
    }
    /* top-level objects on an assoc list ought to be conses (i.e. each
     * successive car should be a cons), but there's no need to throw a
     * wobbly if it isn't. */

    if ( nilp( result)) {
      result = assoc( key, cell_store->payload.cons.cdr);
    }
  }

  return result;
}


/** 
 * Internal workings of internedp, q.v. Not intended to be called from anywhere
 * else. Note that this is VERY similar to assoc, but returns the car (key) of
 * the binding rather than the cdr (value).
 */
struct cons_pointer __internedp( struct cons_pointer key,
				 struct cons_pointer store) {
  struct cons_pointer result = NIL;

  if ( consp( store)) {
    struct cons_space_object* cell_store = &pointer2cell( store);

    if ( consp( cell_store->payload.cons.car)) {
      struct cons_space_object* binding =
	&pointer2cell( cell_store->payload.cons.car);

      if ( equal( key, binding->payload.cons.car)) {
	result = binding->payload.cons.car;
      }
    }
    /* top-level objects on an assoc list ought to be conses (i.e. each
     * successive car should be a cons), but there's no need to throw a
     * wobbly if it isn't. */

    if ( nilp( result)) {
      result = assoc( key, cell_store->payload.cons.cdr);
    }
  }

  return result;  
}


/**
 * Return the canonical version of this key if ut is present as a key in this 
 * enviroment, defaulting to the oblist if no environment is passed. Key is
 * expected to be a string.
 */
struct cons_pointer internedp( struct cons_pointer key,
			       struct cons_pointer environment) {
  struct cons_pointer result = NIL;

  if ( stringp( key)) {
    if ( nilp( environment)) {
      result = __internedp( key, oblist);
    } else {
      result = __internedp( key, environment);
    }
  }
  
  return result;
}


/**
 * Return a new key/value store containing all the key/value pairs in this store
 * with this key/value pair added to the front.
 */
struct cons_pointer bind( struct cons_pointer key, struct cons_pointer value,
			    struct cons_pointer store) {
  return make_cons( make_cons( key, value), store);
}


/**
 * Binds this key to this value in the global oblist, but doesn't affect the 
 * current environment. May not be useful except in bootstrapping (and even 
 * there it may not be especially useful).
 */
struct cons_pointer deep_bind( struct cons_pointer key, struct cons_pointer value) {
  oblist = bind( key, value, oblist);
  return oblist;
}


/**
 * Ensure that a canonical copy of this key is bound in this environment, and
 * return that canonical copy. If there is currently no such binding, create one
 * with the value NIL.
 */
struct cons_pointer intern( struct cons_pointer key,
			    struct cons_pointer environment) {
  struct cons_pointer result = environment;
  struct cons_pointer canonical = internedp( key, environment);

  if ( nilp( canonical)) {
    /* not currently bound */
    result = bind( key, NIL, environment);
  }

  return result;
}
