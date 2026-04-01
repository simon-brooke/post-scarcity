/**
 *  memory/free.c
 *
 *  Centralised point for despatching free methods to types.
 *
 *  TODO: In the long run, we need a type for tags, which defines a constructor
 *  and a free method, along with the minimum and maximum size classes
 *  allowable for that tag; and we need a namespace in which tags are
 *  canonically stored, probably ::system:tags, in which the tag is bound to
 *  the type record describing it. And this all needs to work in Lisp, not
 *  in the substrate.
 *
 *  (c) 2026 Simon Brooke <simon@journeyman.cc>
 *  Licensed under GPL version 2.0, or, at your option, any later version.
 */

#include "memory/node.h"
#include "memory/pointer.h"
#include "memory/pso.h"
#include "memory/tags.h"

#include "payloads/cons.h"
#include "payloads/exception.h"
#include "payloads/stack.h"
#include "payloads/psse_string.h"

/**
 * @brief Despatch destroy message to the handler for the type of the
 * object indicated by `p`, if there is one. What the destroy handler
 * 		needs to do is dec_ref all the objects pointed to by it.
 *
 * 		The handler has 0.1.0 lisp calling convention, since
 * 		1. we should be able to write destroy handlers in Lisp; and
 * 		2. in the long run this whole system should be rewritten in Lisp.
 *
 * 		The handler returns `nil` on success, an exception pointer on
 * 		failure. This function returns that exception pointer. How we
 * 		handle that exception pointer I simply don't know yet.
 */
struct pso_pointer destroy( struct pso_pointer p) {
	struct pso_pointer result = nil;
	struct pso_pointer f = make_frame( nil, p);
	inc_ref( f);

	switch (get_tag_value(p)) {
	case CONSTV: destroy_cons(f, nil); break;
	case EXCEPTIONTV: destroy_exception(f, nil); break;
	case KEYTV :
	case STRINGTV:
	case SYMBOLTV: destroy_string(f, nil); break;
	case STACKTV: destroy_stack_frame(f, nil); break;
	// TODO: others.
	}

	dec_ref(f);
	return result;
}

