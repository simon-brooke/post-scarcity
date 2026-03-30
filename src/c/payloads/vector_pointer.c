/**
 *  payloads/vector_pointer.c
 *
 *  A pointer to an object in vector space.
 *
 *  (c) 2026 Simon Brooke <simon@journeyman.cc>
 *  Licensed under GPL version 2.0, or, at your option, any later version.
 */

#include <stdbool.h>

#include "memory/pointer.h"
#include "memory/pso.h"
#include "payloads/vector_pointer.h"

bool vectorpointp( struct pso_pointer p) {
	return (get_tag_value( p) == VECTORPOINTTV);
}
