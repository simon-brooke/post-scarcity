/**
 *  payloads/exception.c
 *
 *  An exception; required three pointers, so use object of size class 3.
 *
 *  (c) 2026 Simon Brooke <simon@journeyman.cc>
 *  Licensed under GPL version 2.0, or, at your option, any later version.
 */


#include "memory/node.h"
#include "memory/pointer.h"
#include "memory/pso.h"

#include "payloads/exception.h"

struct pso_pointer make_exception( struct pso_pointer message, struct pso_pointer frame_pointer, struct pso_pointer cause) {
    // TODO: not yet implemented
    return nil;
}