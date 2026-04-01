/**
 *  ops/reverse.h
 *
 *  Post Scarcity Software Environment: reverse.
 *
 *  Reverse a sequence.
 *
 *  (c) 2026 Simon Brooke <simon@journeyman.cc>
 *  Licensed under GPL version 2.0, or, at your option, any later version.
 */

#ifndef __psse_ops_reverse_h
#define __psse_ops_reverse_h

#include <stdbool.h>

#include "memory/pointer.h"

struct pso_pointer reverse( struct pso_pointer sequence );

#endif
