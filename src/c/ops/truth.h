/**
 *  ops/truth.h
 *
 *  Post Scarcity Software Environment: truth functions.
 *
 *  Tests for truth.
 *
 *  (c) 2026 Simon Brooke <simon@journeyman.cc>
 *  Licensed under GPL version 2.0, or, at your option, any later version.
 */

#ifndef __psse_ops_truth_h
#define __psse_ops_truth_h
#include <stdbool.h>

#include "memory/pointer.h"
#include "memory/pso4.h"

bool nilp( struct pso_pointer p );

struct pso_pointer lisp_nilp( struct pso4 *frame,
                            struct pso_pointer frame_pointer,
                            struct pso_pointer env );

bool not( struct pso_pointer p );

struct pso_pointer lisp_not( struct pso4 *frame,
                            struct pso_pointer frame_pointer,
                            struct pso_pointer env );

bool truep( struct pso_pointer p );

struct pso_pointer lisp_truep( struct pso4 *frame,
                            struct pso_pointer frame_pointer,
                            struct pso_pointer env );

#endif
