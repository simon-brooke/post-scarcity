/**
 *  ops/assoc.h
 *
 *  Post Scarcity Software Environment: assoc.
 *
 *  Search a store for the value associated with a key.
 *
 *  (c) 2026 Simon Brooke <simon@journeyman.cc>
 *  Licensed under GPL version 2.0, or, at your option, any later version.
 */

#ifndef __psse_ops_assoc_h
#define __psse_ops_assoc_h

#include <stdbool.h>

#include "memory/pointer.h"

struct cons_pointer search( struct pso_pointer key,
                            struct pso_pointer store, bool return_key );

struct pso_pointer assoc( struct pso_pointer key, struct pso_pointer store );

struct pso_pointer interned( struct pso_pointer key,
                             struct pso_pointer store );

bool internedp( struct pso_pointer key, struct pso_pointer store );
#endif
