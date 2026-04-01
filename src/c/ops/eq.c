/**
 *  ops/eq.c
 *
 *  Post Scarcity Software Environment: eq.
 *
 *  Test for pointer equality; bootstrap level tests for object equality.
 *
 *  (c) 2026 Simon Brooke <simon@journeyman.cc>
 *  Licensed under GPL version 2.0, or, at your option, any later version.
 */

#include "memory/memory.h"
#include "memory/node.h"
#include "memory/pointer.h"
#include "memory/pso2.h"
#include "memory/tags.h"

#include "payloads/cons.h"
#include "payloads/integer.h"
#include "payloads/stack.h"
#include "ops/stack_ops.h"
#include "ops/truth.h"

/**
 * @brief Function; do these two pointers point to the same object? 
 *
 * Shallow, cheap equality.
 *
 * TODO: if either of these pointers points to a cache cell, then what
 * we need to check is the cached value, which is not so cheap. Ouch!
 *
 * @param a a pointer;
 * @param b another pointer;
 * @return `true` if they are the same, else `false`
 */
bool eq( struct pso_pointer a, struct pso_pointer b ) {
    return ( a.node == b.node && a.page == b.page && a.offset == b.offset );
}

bool equal( struct pso_pointer a, struct pso_pointer b) {
	bool result = false;

	if ( eq( a, b)) {
		result = true;
	} else if ( get_tag_value(a) == get_tag_value(b)) {
		struct pso2 *oa = pointer_to_object(a);
		struct pso2 *ob = pointer_to_object(b);

		switch ( get_tag_value(a)) {
		case CHARACTERTV :
			result = (oa->payload.character.character == ob->payload.character.character);
			break;
		case CONSTV :
			result = (equal( car(a), car(b)) && equal( cdr(a), cdr(b)));
			break;
		case INTEGERTV :
			result = (oa->payload.integer.value
					==
					ob->payload.integer.value);
			break;
		case KEYTV:
		case STRINGTV :
		case SYMBOLTV :
			while (result == false && !nilp(a) && !nilp(b)) {
				if (pointer_to_object(a)->payload.string.character ==
						pointer_to_object(b)->payload.string.character) {
					a = cdr(a);
					b = cdr(b);
				}
			}
			result = nilp(a) && nilp(b);
			break;
		}
	}

	return result;
}


/**
 * Function; do all arguments to this finction point to the same object? 
 *
 * Shallow, cheap equality.
 *
 * * (eq? args...)
 *
 * @param frame my stack_frame.
 * @param frame_pointer a pointer to my stack_frame.
 * @param env my environment (ignored).
 * @return `t` if all args are pointers to the same object, else `nil`;
 */
struct pso_pointer lisp_eq( struct pso4 *frame,
                            struct pso_pointer frame_pointer,
                            struct pso_pointer env ) {
    struct pso_pointer result = t;

    if ( frame->payload.stack_frame.args > 1 ) {
        for ( int b = 1;
              ( truep( result ) ) && ( b < frame->payload.stack_frame.args );
              b++ ) {
            result =
                eq( fetch_arg( frame, 0 ), fetch_arg( frame, b ) ) ? t : nil;
        }
    }

    return result;
}


