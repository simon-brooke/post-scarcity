/**
 * intern.h
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


#ifndef __intern_h
#define __intern_h

extern struct cons_pointer oblist;

/**
 * return the value associated with this key in this store. In the current 
 * implementation a store is just an assoc list, but in future it might be a
 * namespace, a regularity or a homogeneity.
 */
struct cons_pointer c_assoc( struct cons_pointer key, struct cons_pointer store);

/**
 * Return true if this key is present as a key in this enviroment, defaulting to
 * the oblist if no environment is passed.
 */
struct cons_pointer internedp( struct cons_pointer key,
			       struct cons_pointer environment);

/**
 * Return a new key/value store containing all the key/value pairs in this store
 * with this key/value pair added to the front.
 */
struct cons_pointer bind( struct cons_pointer key, struct cons_pointer value,
			  struct cons_pointer store);

/**
 * Binds this key to this value in the global oblist, but doesn't affect the 
 * current environment. May not be useful except in bootstrapping (and even 
 * there it may not be especially useful).
 */
struct cons_pointer deep_bind( struct cons_pointer key, struct cons_pointer value);

/**
 * Ensure that a canonical copy of this key is bound in this environment, and
 * return that canonical copy. If there is currently no such binding, create one
 * with the value NIL.
 */
struct cons_pointer intern( struct cons_pointer key,
			    struct cons_pointer environment);

#endif
