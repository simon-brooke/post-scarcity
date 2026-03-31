/**
 *  ops/eq.h
 *
 *  Post Scarcity Software Environment: eq.
 *
 *  Test for pointer equality.
 *
 *  (c) 2026 Simon Brooke <simon@journeyman.cc>
 *  Licensed under GPL version 2.0, or, at your option, any later version.
 */

#ifndef __psse_ops_eq_h
#define __psse_ops_eq_h
#include <stdbool.h>

#include "memory/pointer.h"
#include "memory/pso4.h"

bool eq( struct pso_pointer a, struct pso_pointer b );

struct pso_pointer lisp_eq( struct pso4 *frame,
                            struct pso_pointer frame_pointer,
                            struct pso_pointer env );

bool equal( struct pso_pointer a, struct pso_pointer b);
#endif
