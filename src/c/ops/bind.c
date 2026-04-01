/**
 *  ops/bind.c
 *
 *  Post Scarcity Software Environment: bind.
 *
 *  Add a binding for a key/value pair to a store -- at this stage, just an
 *  association list.
 *
 *  (c) 2026 Simon Brooke <simon@journeyman.cc>
 *  Licensed under GPL version 2.0, or, at your option, any later version.
 */

#include "memory/node.h"
#include "memory/pointer.h"
#include "memory/pso.h"
#include "memory/pso4.h"
#include "memory/tags.h"

#include "ops/stack_ops.h"

#include "payloads/cons.h"
#include "payloads/stack.h"

struct pso_pointer bind( struct pso_pointer frame_pointer,
                            struct pso_pointer env) {
	struct pso4 *frame = pointer_to_pso4( frame_pointer);
	struct pso_pointer key = fetch_arg( frame, 0);
	struct pso_pointer value = fetch_arg( frame, 1);
	struct pso_pointer store = fetch_arg( frame, 2);

	return cons( cons(key, value), store);
}

struct pso_pointer c_bind( struct pso_pointer key,
		struct pso_pointer value,
		struct pso_pointer store) {
	struct pso_pointer result = nil;
	struct pso_pointer next = make_frame( nil, key, value, store);
	inc_ref( next);
	result = bind( next, nil);
	dec_ref( next);

	return result;
}

