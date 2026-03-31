/**
 *  ops/assoc.c
 *
 *  Post Scarcity Software Environment: assoc.
 *
 *  Search a store for the value associated with a key.
 *
 *  (c) 2026 Simon Brooke <simon@journeyman.cc>
 *  Licensed under GPL version 2.0, or, at your option, any later version.
 */

#include <stdbool.h>

#include "memory/node.h"
#include "memory/pointer.h"
#include "memory/pso2.h"
#include "memory/tags.h"

#include "payloads/cons.h"

#include "ops/eq.h"
#include "ops/truth.h"

/**
 * @brief: fundamental search function; only knows about association lists
 *
 * @param key a pointer indicating the key to search for;
 * @param store a pointer indicating the store to search;
 * @param return_key if a binding is found for `key` in `store`, if true
 * 			return the key found in the store, else return the value
 *
 * @return nil if no binding for `key` is found in `store`; otherwise, if
 * 			`return_key` is true, return the key from the store; else
 * 			return the binding.
 */
struct pso_pointer search( struct pso_pointer key,
                                  struct pso_pointer store,
                                  bool return_key ) {
	struct pso_pointer result = nil;
	bool found = false;

	if (consp( store)) {
		for ( struct pso_pointer cursor = store;
				consp( store) && found == false;
				cursor = cdr( cursor)) {
			struct pso_pointer pair = car( cursor);

			if (consp(pair) && equal(car(pair), key)) {
				found = true;
				result = return_key ? car(pair) : cdr( pair);
			}
		}
	}

	return result;
}

/**
 * @prief: bootstap layer assoc; only knows about association lists.
 *
 * @param key a pointer indicating the key to search for;
 * @param store a pointer indicating the store to search;
 *
 * @return a pointer to the value of the key in the store, or nil if not found
 */
struct pso_pointer assoc( struct pso_pointer key, struct pso_pointer store) {
	return search( key, store, false);
}

/**
 * @prief: bootstap layer interned; only knows about association lists.
 *
 * @param key a pointer indicating the key to search for;
 * @param store a pointer indicating the store to search;
 *
 * @return a pointer to the copy of the key in the store, or nil if not found.
 */
struct pso_pointer interned(struct pso_pointer key, struct pso_pointer store) {
	return search( key, store, true);
}

/**
 * @prief: bootstap layer interned; only knows about association lists.
 *
 * @param key a pointer indicating the key to search for;
 * @param store a pointer indicating the store to search;
 *
 * @return `true` if a pointer the key was found in the store..
 */
bool internedp(struct pso_pointer key, struct pso_pointer store) {
	return !nilp( search( key, store, true));
}
