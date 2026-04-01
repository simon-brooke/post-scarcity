/**
 *  payloads/exception.c
 *
 *  An exception; required three pointers, so use object of size class 3.
 *
 *  (c) 2026 Simon Brooke <simon@journeyman.cc>
 *  Licensed under GPL version 2.0, or, at your option, any later version.
 */


#include "memory/node.h"
#include "memory/pointer.h"
#include "memory/pso.h"
#include "memory/pso4.h"
#include "memory/tags.h"

#include "payloads/exception.h"

struct pso_pointer make_exception( struct pso_pointer message,
		struct pso_pointer frame_pointer, struct pso_pointer cause) {
    // TODO: not yet implemented
    return nil;
}

/**
 * @brief When an exception is freed, all its pointers must be decremented.
 *
 * Lisp calling conventions; one expected arg, the pointer to the object to
 * be destroyed.
 */
struct pso_pointer destroy_exception( struct pso_pointer fp,
		struct pso_pointer env) {
	if (stackp(fp)) {
		struct pso4 *frame = pointer_to_pso4( fp);
		struct pso_pointer p = frame->payload.stack_frame.arg[0];

		// TODO: decrement every pointer indicated by an exception.
	}

	return nil;
}
