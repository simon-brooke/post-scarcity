/**
 *  memory/destroy.h
 *
 *  Despatcher for destructor functions when objects are freed.
 *
 *  (c) 2026 Simon Brooke <simon@journeyman.cc>
 *  Licensed under GPL version 2.0, or, at your option, any later version.
 */

#ifndef __psse_memory_destroy_h
#define __psse_memory_destroy_h

#include "memory/pointer.h"

struct pso_pointer destroy( struct pso_pointer p );

#endif
