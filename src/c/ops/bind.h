/**
 *  ops/bind.h
 *
 *  Post Scarcity Software Environment: bind.
 *
 *  Test for pointer binduality.
 *
 *  (c) 2026 Simon Brooke <simon@journeyman.cc>
 *  Licensed under GPL version 2.0, or, at your option, any later version.
 */

#ifndef __psse_ops_bind_h
#define __psse_ops_bind_h
#include <stdbool.h>

#include "memory/pointer.h"

struct pso_pointer bind( struct pso_pointer frame_pointer,
                            struct pso_pointer env);

struct pso_pointer c_bind( struct pso_pointer key,
		struct pso_pointer value,
		struct pso_pointer store);

#endif
