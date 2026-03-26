/**
 *  memory/memory.h
 *
 *  The memory management subsystem.
 *
 *  (c) 2026 Simon Brooke <simon@journeyman.cc>
 *  Licensed under GPL version 2.0, or, at your option, any later version.
 */

#ifndef __psse_memory_memory_h
#define __psse_memory_memory_h

#include "memory/pointer.h"

int initialise_memory();

extern struct pso_pointer out_of_memory_exception;

#endif
