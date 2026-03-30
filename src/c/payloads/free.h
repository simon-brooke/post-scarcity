/**
 *  payloads/free.h
 *
 *  An unassigned object, on a freelist; may be of any size class.
 *
 *  (c) 2026 Simon Brooke <simon@journeyman.cc>
 *  Licensed under GPL version 2.0, or, at your option, any later version.
 */

#ifndef __psse_payloads_free_h
#define __psse_payloads_free_h

#include "memory/pointer.h"

/**
 * @brief An unassigned object, on a freelist; may be of any size class.
 * 
 */
struct free_payload {
  /** the next object on the free list for my size class */
    struct pso_pointer next;
};


#endif
