/**
 *  memory/pso4.h
 *
 *  Paged space object of size class 4, 16 words total, 14 words payload.
 *
 *  (c) 2026 Simon Brooke <simon@journeyman.cc>
 *  Licensed under GPL version 2.0, or, at your option, any later version.
 */

#include "memory/pointer.h"
#include "memory/pso.h"
#include "memory/pso2.h"
#include "memory/pso4.h"

struct pso4* pointer_to_pso4( struct pso_pointer p) {
	struct pso4* result = (struct pso4*)pointer_to_object_of_size_class( p, 4);
}
