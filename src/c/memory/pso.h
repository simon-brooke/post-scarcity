/**
 *  memory/pso.h
 *
 *  Paged space objects.
 *
 *  (c) 2026 Simon Brooke <simon@journeyman.cc>
 *  Licensed under GPL version 2.0, or, at your option, any later version.
 */

#ifndef __psse_memory_pso_h
#define __psse_memory_pso_h

#include <stdint.h>

#include "memory/header.h"
#include "memory/pointer.h"

struct pso_pointer allocate( char *tag, uint8_t size_class );

struct pso_pointer dec_ref( struct pso_pointer pointer );

struct pso_pointer inc_ref( struct pso_pointer pointer );

struct pso_pointer lock_object( struct pso_pointer pointer );

struct pso_pointer free_object( struct pso_pointer p );

#endif
